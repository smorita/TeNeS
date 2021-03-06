.. highlight:: none

Input file for ``tense_simple`` 
---------------------------------

-  File format is
   `TOML <https://github.com/toml-lang/toml/blob/master/versions/ja/toml-v0.5.0.md>`__
   format.
-  The input file has five sections : ``model``, ``parameter``, ``lattice``, ``observable``, ``correlation`` .

-  In future, the file will be split by specifying a file name.

``parameter`` section
==========================

The contents of the ``parameter`` section are copied directly to the ``parameter`` section of the input file of ``tenes``.
You can also specify the imaginary time step size for the imaginary time evolution operator in simple and full updates in the subsections ``simple_update``, ``full_update``.

.. csv-table::
   :header: "Name", "Description", "Type", "Default"
   :widths: 30, 30, 10, 10

   ``tau``, imaginary time step in the imaginary time evolution operator, Real, 0.01

The following parameters are common to the ``tenes`` input file.

``parameter.tensor``
~~~~~~~~~~~~~~~~~~~~

.. csv-table::
   :header: "Name", "Description", "Type", "Default"
   :widths: 30, 30, 10, 10

   ``D``,        "The virtual bond dimensions of the central tensor",          Integer, 2
   ``CHI``,      "The virtual bond dimensions of the angular transfer matrix", Integer, 4
   ``save_dir``, "Directory to write optimized tensors",                       String,  \"\"
   ``load_dir``, "Directory to read initial tensor",                           String,  \"\"


- ``save_dir``

  - Store optimized tensors below this directory.
  - When it is empty, the tensors are not saved.

- ``load_dir``

  - Read various tensors from below this directory.
  - Must be the same degree of parallelism as when saved.
  - Not read if it is empty.

``parameter.simple_update``
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. csv-table::
   :header: "Name", "Description", "Type", "Default"
   :widths: 30, 30, 10, 10

   ``num_step``,      "Number of simple updates",                                            Integer, 0
   ``lambda_cutoff``, "cutoff of the mean field to be considered zero in the simple update", Real,    1e-12

``parameter.full_update``
~~~~~~~~~~~~~~~~~~~~~~~~~

.. csv-table::
   :header: "Name", "Description", "Type", "Default"
   :widths: 30, 30, 10, 10

   ``num_step``,            "Number of full updates",                                                                                      Integer, 0
   ``env_cutoff``,          "Cutoff of singular values to be considered as zero when computing environment through full updates",          Real,    1e-12
   ``inverse_precision``,   "Cutoff of singular values to be considered as zero when computing the pseudoinverse matrix with full update", Real,    1e-12
   ``convergence_epsilon``, "Convergence criteria for truncation optimization with full update",                                           Real,    1e-12
   ``iteration_max``,       "Maximum iteration number for truncation optimization on full updates",                                        Integer, 1000
   ``gauge_fix``,           "Whether the tensor gauge is fixed",                                                                           Boolean, true
   ``fastfullupdate``,      "Whether the Fast full update is adopted",                                                                     Boolean, true

``parameter.ctm``
~~~~~~~~~~~~~~~~~

.. csv-table::
   :header: "Name", "Description", "Type", "Default"
   :widths: 30, 30, 10, 10

   ``projector_cutoff``,         "Cutoff of singular values to be considered as zero when computing CTM projectors",                          Real,    1e-12
   ``convergence_epsilon``,      "CTM convergence criteria",                                                                                  Real,    1e-10
   ``iteration_max``,            "Maximum iteration number of convergence for CTM",                                                           Integer, 100
   ``projector_corner``,         "Whether to use only the 1/4 corner tensor in the CTM projector calculation",                                Boolean, true
   ``use_rsvd``,                 "Whether to replace SVD with Random SVD",                                                                    Boolean, false
   ``rsvd_oversampling_factor``, "Ratio of the number of the oversampled elements to that of the obtained elements in the Random SVD method", Real,    2.0


``parameter.random``
~~~~~~~~~~~~~~~~~~~~~

.. csv-table::
   :header: "Name", "Description", "Type", "Default"
   :widths: 30, 30, 10, 10

   ``seed``, "Seed of the pseudo-random number generator used to initialize the tensor", Integer, 11

Each MPI process has the own seed as ``seed`` plus the process ID (MPI rank).

Example
~~~~~~~

::

    [parameter]
    [parameter.tensor]
    D  = 4     # tensor_dim
    CHI  = 16  # env_dim

    [parameter.simple_update]
    num_step = 1000

    [parameter.full_update]
    num_step = 1

    [parameter.ctm]
    iteration_max = 5


``lattice`` section
==========================

Specify the lattice information.
Square lattice and honeycomb lattice are defined as lattice types.

.. csv-table::
   :header: "Name", "Description", "Type", "Default"
   :widths: 30, 30, 20, 20

   ``type``, "Lattice name (square lattice or honeycomb lattice)", String, --
   ``L_sub``, "Unit cell size", Integer or a list of two integers, --
   ``initial``, "Initial state", String, "random"
   ``noise``, "Noise of initial components", Real, 1e-2


When a list of two integers is passed as ``L_sub``, the first element gives the value of ``Lx`` and the second one does ``Ly``.
If ``L_sub`` is an integer, Both ``Lx`` and ``Ly`` will have the same value.
A list of three or more elements causes an error.

Sites in a unit cell are indexed starting from 0.
These are arranged in order from the x direction.

Sites in a unit cell of ``L_sub = [2,3]`` are arranged as follows::

 y
 ^     4 5
 |     2 3
 .->x  0 1

``initial`` ane ``noise`` specifies an initial state.
``initial`` can take the following value: ``"ferro", "antiferro", "random"`` .
``noise`` is an amplitude of a noise.

Square lattice
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

There are two types of bond, horizontal (0) and vertical (1) (corresponding to ``-`` and ``|`` in the below figure).

The unit cell for ``L_sub = 2`` is given as follows::

 0   1
 |   |
 2 - 3 - 2
 |   | 
 0 - 1 - 0


Honeycomb lattice
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Unit cell size (Each element of ``L_sub``) must be an even number.

There are 3 types of bonds: x, y, and z (corresponding to ``-``, ``~``, ``|``  in the below figure).
Each site with an even index has a rightward (x), a leftward (y), and an upward (z) bonds and
each site with an odd index has a leftward (x), a rightward (y), and a bottomward (z) bonds.

The unit cell for ``L_sub = 2`` is given as follows::

 0   1
     |
 2 ~ 3 - 2
 |   
 0 - 1 ~ 0


``model`` section
==========================

Specify the type of the model.
Only the Spin system can be spcified in ver. 0.1.

.. csv-table::
   :header: "Name", "Description", "Type"
   :widths: 30, 30, 15

   ``type``, "The type of the model", String


Spin system
~~~~~~~~~~~~~~~~~~~~~

Spin system

.. math ::

 \mathcal{H} = \sum_{\langle ij \rangle}\left[\sum_\alpha^{x,y,z} J^\alpha_{ij} S^\alpha_i S^\alpha_j + B \left(\vec{S}_i\cdot\vec{S}_j\right)^2 \right] - \sum_i \left[ h S^z_i + \Gamma S^x_i - D \left(S^z_i\right)^2 \right]

.. csv-table::
   :header: "Name", "Description", "Type", "Default"
   :widths: 30, 30, 10, 10

   ``S``, "Magnituide of the local spin", Real, 0.5
   ``Jx``, "The x component of the exchange interaction J", Real or a list of Real, 1.0
   ``Jy``, "The y component of the exchange interaction J", Real or a list of Real, 1.0
   ``Jz``,"The z component of the exchange interaction J", Real or a list of Real, 1.0
   ``BQ``, "Biquadratic interaction B", Real or a list of Real, 0.0
   ``h``, "longitudinal magnetic field h", Real, 0.0
   ``G``, "Transverse magnetic field
 :math:`\Gamma` ", Real, 0.0
   ``D``, "On-site spin anisotropy D", Real, 0.0


By providing a list of exchange and biquadratic interactions, we can vary the magnitude of the interaction for each type of lattice bond.
If the number of elements in the list is less than the type of lattice bond, the remainder is filled in with the last element specified.


``observable`` section
==========================

By default, the local physical quantities used for physical quantities measurements: :math:`S^z`  and :math:`S^x` .
Measurements of more detailed physical quantities can be made by overwriting the format common to the input file of ``tenes``. For details, See ``observable`` section :doc:`expert_format`.

``correlation`` section
==========================

For ``tenes_simple`` , correlation functions :math:`C = \langle A(0)B(r)\rangle` are not calculated by default.
For calculating correlation functions, they have to be specified in the same file format as the input file of ``tenes``.
For details, See ``correlation`` section :doc:`expert_format`.
