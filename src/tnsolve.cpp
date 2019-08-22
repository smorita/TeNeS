#define _USE_MATH_DEFINES
#include <random>
#include <sys/stat.h>
#include <complex>

#include <mptensor/complex.hpp>
#include <mptensor/rsvd.hpp>
#include <mptensor/tensor.hpp>

#include "Lattice.hpp"
#include "PEPS_Basics.hpp"
#include "PEPS_Parameters.hpp"
#include "Square_lattice_CTM.hpp"
#include "edge.hpp"

#include "tnsolve.hpp"


using namespace mptensor;

template <class ptensor>
void Initialize_Tensors(std::vector<ptensor> &Tn, Lattice lattice, int ldof) {
  int D = Tn[0].shape()[0];

  std::vector<double> ran(D * D * D * D * ldof);
  std::mt19937 gen(11);
  std::uniform_real_distribution<double> dist(-0.01, 0.01);
  for (int i = 0; i < D * D * D * D * ldof; i++) {
    ran[i] = dist(gen);
  }
  int nr;

  Index index;
  for (int i = 0; i < lattice.N_UNIT; ++i) {
    for (int n = 0; n < Tn[i].local_size(); ++n) {
      index = Tn[i].global_index(n);
      if (index == Index(0, 0, 0, 0, 0)) {
        Tn[i].set_value(index, 1.0);
      } else {
        nr = index[0] + index[1] * D + index[2] * D * D + index[3] * D * D * D +
             index[4] * D * D * D * D;
        Tn[i].set_value(index, ran[nr]);
      }
    }
  }
}

template <class ptensor>
int tnsolve(MPI_Comm comm,
            PEPS_Parameters peps_parameters,
            Lattice lattice,
            Edges simple_edges,
            Edges full_edges,
            std::vector<ptensor> hams,
            std::vector<ptensor> lops
    ){
  int mpisize, mpirank;

  /* MPI initialization */
  // MPI_Init(&argc, &argv);
  MPI_Comm_rank(comm, &mpirank);
  MPI_Comm_size(comm, &mpisize);

  // for measure time
  double time_simple_update = 0.0;
  double time_full_update = 0.0;
  double time_env = 0.0;
  double time_obs = 0.0;
  double start_time;

  std::cout << std::setprecision(12);

  constexpr int nleg = 4;

  peps_parameters.Bcast(comm);
  lattice.Bcast(comm);

  // output debug or warning info only from process 0
  if (mpirank != 0) {
    peps_parameters.Debug_flag = false;
    peps_parameters.Warning_flag = false;
  }

  if (mpirank == 0) {
    // folder check
    struct stat status;
    if (stat("output_data", &status) != 0) {
      mkdir("output_data", 0755);
    }

    peps_parameters.save("output_data/output_params.dat");
    lattice.save_append("output_data/output_params.dat");
    std::ofstream ofs;
    ofs.open("output_data/output_params.dat", std::ios::out | std::ios::app);
    ofs << std::endl;
    ofs.close();
  }

  // set seed for randomized svd
  random_tensor::set_seed(11 + mpirank);

  // for convenience//
  int D = peps_parameters.D;
  int CHI = peps_parameters.CHI;

  int LX = lattice.LX;
  int LY = lattice.LY;
  int N_UNIT = lattice.N_UNIT;
  int ldof = lops.begin()->shape()[0];

  std::clog << "Start initialize tensors" << std::endl;

  // Tensors
  std::vector<ptensor> Tn(N_UNIT, ptensor(Shape(D, D, D, D, ldof)));
  std::vector<ptensor>
      eTt(N_UNIT, ptensor(Shape(CHI, CHI, D, D))),
      eTr(N_UNIT, ptensor(Shape(CHI, CHI, D, D))),
      eTb(N_UNIT, ptensor(Shape(CHI, CHI, D, D))),
      eTl(N_UNIT, ptensor(Shape(CHI, CHI, D, D)));
  std::vector<ptensor> 
      C1(N_UNIT, ptensor(Shape(CHI, CHI))),
      C2(N_UNIT, ptensor(Shape(CHI, CHI))),
      C3(N_UNIT, ptensor(Shape(CHI, CHI))),
      C4(N_UNIT, ptensor(Shape(CHI, CHI)));

  std::vector<std::vector<std::vector<double> > > lambda_tensor(
      N_UNIT, std::vector<std::vector<double> >(nleg, std::vector<double>(D)));

  Initialize_Tensors(Tn, lattice, ldof);
  for (int i1 = 0; i1 < N_UNIT; ++i1) {
    for (int i2 = 0; i2 < nleg; ++i2) {
      for (int i3 = 0; i3 < D; ++i3) {
        lambda_tensor[i1][i2][i3] = 1.0;
      }
    }
  }

  ptensor Tn1_new, Tn2_new;
  std::vector<double> lambda_c;

  std::vector<ptensor> ops;
  for(auto Ham: hams){
    ptensor U = EvolutionaryTensor(Ham, peps_parameters.tau_simple);
    ptensor op12 = transpose(reshape(U, Shape(ldof, ldof, ldof, ldof)), Axes(2, 3, 0, 1));
    ops.push_back(op12);
  }

  std::clog << "Start simple update" << std::endl;

  // simple update
  start_time = MPI_Wtime();
  for (int int_tau = 0; int_tau < peps_parameters.num_simple_step; ++int_tau) {
    for(auto ed: simple_edges){
      const int source = ed.source_site;
      const int target = ed.target_site;
      const int source_leg = ed.source_leg;
      const int target_leg = ed.target_leg;
      Simple_update_bond(Tn[source], Tn[target],
          lambda_tensor[source], lambda_tensor[target],
          ops[ed.op_id], source_leg, peps_parameters,
          Tn1_new, Tn2_new, lambda_c
          );
      lambda_tensor[source][source_leg] = lambda_c;
      lambda_tensor[target][target_leg] = lambda_c;
      Tn[source] = Tn1_new;
      Tn[target] = Tn2_new;
    }
  }
  time_simple_update += MPI_Wtime() - start_time;
  // done simple update

  std::clog << "Start full update" << std::endl;

  // Start full update
  if (peps_parameters.num_full_step > 0) {
    ops.clear();
    for(auto Ham: hams){
      ptensor U = EvolutionaryTensor(Ham, peps_parameters.tau_full);
      ptensor op12 = transpose(reshape(U, Shape(ldof, ldof, ldof, ldof)), Axes(2, 3, 0, 1));
      ops.push_back(op12);
    }
    // Environment
    start_time = MPI_Wtime();
    Calc_CTM_Environment(C1, C2, C3, C4, eTt, eTr, eTb, eTl, Tn,
        peps_parameters, lattice);
    time_env += MPI_Wtime() - start_time;
  }

  start_time = MPI_Wtime();
  for (int int_tau = 0; int_tau < peps_parameters.num_full_step; ++int_tau) {

    for(auto ed: full_edges){
      const int source = ed.source_site;
      const int target = ed.target_site;

      if(ed.is_horizontal()){
        /*
         *  C1 t t' C2'
         *  l  T T' r'
         *  C4 b b' C3'
         */
        Full_update_bond(C1[source], C2[target], C3[target], C4[source],
            eTt[source], eTt[target], eTr[target], // t  t' r'
            eTb[target], eTb[source], eTl[source], // b' b  l
            Tn[source], Tn[target],
            ops[ed.op_id], ed.source_leg, peps_parameters,
            Tn1_new, Tn2_new);
      }else{
        /*
         * C1  t C2
         *  l  T r
         *  l' T'  r'
         * C4' b  C3'
         *
         *   |
         *   | rotate
         *   V
         * 
         *  C2 r r' C3'
         *  t  T T' b'
         *  C4 l l' C1'
         */
        Full_update_bond(C2[source], C3[target], C1[target], C4[source],
            eTr[source], eTr[target], eTb[target],
            eTl[target], eTl[source], eTt[source],
            Tn[source], Tn[target],
            ops[ed.op_id], ed.source_leg, peps_parameters,
            Tn1_new, Tn2_new);
      }
      Tn[source] = Tn1_new;
      Tn[target] = Tn2_new;

      if(peps_parameters.Full_Use_FFU){
        if(ed.is_horizontal()){
          const int source_x = source % LX;
          const int target_x = target % LX;
          Left_move(C1, C2, C3, C4, eTt, eTr, eTb, eTl, Tn,
              source_x, peps_parameters, lattice);
          Right_move(C1, C2, C3, C4, eTt, eTr, eTb, eTl, Tn,
              target_x, peps_parameters, lattice);
        }else{
          const int source_y = source / LX;
          const int target_y = target / LX;
          Top_move(C1, C2, C3, C4, eTt, eTr, eTb, eTl, Tn,
              source_y, peps_parameters, lattice);
          Bottom_move(C1, C2, C3, C4, eTt, eTr, eTb, eTl, Tn,
              target_y, peps_parameters, lattice);
        }
      }else{
        Calc_CTM_Environment(C1, C2, C3, C4, eTt, eTr, eTb, eTl, Tn,
            peps_parameters, lattice);
      }
    }
  }
  time_full_update += MPI_Wtime() - start_time;
  // done full update

  std::clog << "Start calculating observables" << std::endl;

  start_time = MPI_Wtime();
  Calc_CTM_Environment(C1, C2, C3, C4, eTt, eTr, eTb, eTl, Tn,
      peps_parameters, lattice);
  time_env += MPI_Wtime() - start_time;

  ptensor op_identity(Shape(ldof, ldof));
  // initialize
  for(int i=0; i<ldof; ++i){
    for(int j=0; j<i; ++j)
      op_identity.set_value(Index(i, j), 0.0);
    op_identity.set_value(Index(i, i), 1.0);
    for(int j=i+1; j<ldof; ++j)
      op_identity.set_value(Index(i, j), 0.0);
  }

  std::vector<ptensor> hamtensors;
  for(int i=0; i<hams.size(); ++i){
    hamtensors.push_back(transpose(reshape(hams[i], Shape(ldof, ldof, ldof, ldof)), Axes(2, 3, 0, 1)));
  }

  const int nlops = lops.size();
  ptensor local_obs(Shape(nlops, N_UNIT));
  ptensor neighbor_obs(Shape(nlops, N_UNIT, 2));  // vertical and horizontal
  
  for (int ilops=0; ilops<nlops; ++ilops){
    for (int i = 0; i < N_UNIT; ++i) {
      local_obs.set_value(Index(ilops, i), 0.0);
      neighbor_obs.set_value(Index(ilops, i, 0), 0.0);
      neighbor_obs.set_value(Index(ilops, i, 1), 0.0);
    }
  }

  start_time = MPI_Wtime();
  for (int i = 0; i < N_UNIT; ++i) {
    double norm = Contract_one_site(C1[i], C2[i], C3[i], C4[i],
                             eTt[i], eTr[i],
                             eTb[i], eTl[i], Tn[i], op_identity);
    for(int ilops=0; ilops<nlops; ++ilops){
      double val = Contract_one_site(C1[i], C2[i], C3[i], C4[i],
                                     eTt[i], eTr[i], eTb[i], eTl[i],
                                     Tn[i], lops[ilops]) / norm;
      local_obs.set_value(Index(ilops, i), val);
    }
    /*
    if (peps_parameters.Debug_flag) {
      std::cout << i << " "
      << " " << norm << " " << mz[i] << " " << mx[i] << std::endl;
    }
    */
  }

  double energy=0.0;
  for(auto ed: simple_edges){
    // const int source = ed.source_site;
    // const int target = ed.target_site;
    const int source = ed.source_site;
    const int target = ed.target_site;
    if(ed.is_horizontal()){
      const double local_norm = Contract_two_sites_horizontal(
                   C1[source], C2[target], C3[target], C4[source],
                   eTt[source], eTt[target], eTr[target], eTb[target], eTb[source], eTl[source],
                   Tn[source], Tn[target], op_identity, op_identity);
      energy += Contract_two_sites_horizontal_op12(
                   C1[source], C2[target], C3[target], C4[source],
                   eTt[source], eTt[target], eTr[target], eTb[target], eTb[source], eTl[source],
                   Tn[source], Tn[target], hamtensors[ed.op_id]) / local_norm;
      for(int ilops=0; ilops<nlops; ++ilops){
        double val = 0.0;
        neighbor_obs.get_value(Index(ilops, source, 0), val);
        val += Contract_two_sites_horizontal(
                     C1[source], C2[target], C3[target], C4[source],
                     eTt[source], eTt[target], eTr[target], eTb[target], eTb[source], eTl[source],
                     Tn[source], Tn[target], lops[ilops], lops[ilops]) / local_norm;
        neighbor_obs.set_value(Index(ilops, source, 0), val);
      }
    }else{
      const double local_norm = Contract_two_sites_vertical(
                   C1[source], C2[source], C3[target], C4[target],
                   eTt[source], eTr[source], eTr[target], eTb[target], eTl[target], eTl[source],
                   Tn[source], Tn[target], op_identity, op_identity);
      energy += Contract_two_sites_vertical_op12(
                   C1[source], C2[source], C3[target], C4[target],
                   eTt[source], eTr[source], eTr[target], eTb[target], eTl[target], eTl[source],
                   Tn[source], Tn[target], hamtensors[ed.op_id]) / local_norm;
      for(int ilops=0; ilops<nlops; ++ilops){
        double val = 0.0;
        neighbor_obs.get_value(Index(ilops, source, 1), val);
        val += Contract_two_sites_vertical(
                     C1[source], C2[source], C3[target], C4[target],
                     eTt[source], eTr[source], eTr[target], eTb[target], eTl[target], eTl[source],
                     Tn[source], Tn[target], lops[ilops], lops[ilops]);
        neighbor_obs.set_value(Index(ilops, source, 1), val);
      }
    }
  }
  time_obs += MPI_Wtime() - start_time;

  if (mpirank == 0) {
    std::cout << "Energy = " << energy / N_UNIT << std::endl;

    for(int ilops=0; ilops<nlops; ++ilops){
      double sum=0.0;
      for(int i=0; i<N_UNIT; ++i){
        double val=0.0;
        local_obs.get_value(Index(ilops, i), val);
        sum += val;
      }
      std::cout << "Local operator " << ilops << " = "
                << sum/N_UNIT << std::endl;
    }
    for(int ilops=0; ilops<nlops; ++ilops){
      double sum=0.0;
      for(int i=0; i<N_UNIT; ++i){
        double val=0.0;
        neighbor_obs.get_value(Index(ilops, i, 0), val);
        sum += val;
        neighbor_obs.get_value(Index(ilops, i, 1), val);
        sum += val;
      }
      std::cout << "Nearest Neighbor " << ilops << " = "
                << 0.5*sum/N_UNIT << std::endl;
    }
  }

  if (mpirank == 0) {
    std::cout << "time simple update = " << time_simple_update << std::endl;
    std::cout << "time full update   = " << time_full_update << std::endl;
    std::cout << "time environmnent  = " << time_env << std::endl;
    std::cout << "time observable    = " << time_obs << std::endl;
  }
  return 0;
}

// template specialization
using d_tensor = mptensor::Tensor<mptensor::scalapack::Matrix, double>;
template
int tnsolve<d_tensor>(MPI_Comm comm,
                      PEPS_Parameters peps_parameters,
                      Lattice lattice,
                      Edges simple_edges,
                      Edges full_edges,
                      std::vector<d_tensor> hams,
                      std::vector<d_tensor> lops
    );
/*
using c_tensor = mptensor::Tensor<mptensor::scalapack::Matrix, std::complex<double>>;
template
int tnsolve<c_tensor>(MPI_Comm comm,
                      PEPS_Parameters peps_parameters,
                      Lattice lattice,
                      Edges simple_edges,
                      Edges full_edges,
                      std::vector<c_tensor> hams
    );
    */