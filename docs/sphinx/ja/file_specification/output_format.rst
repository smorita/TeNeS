.. highlight:: none

出力ファイル
---------------------------------

計算結果は ``output`` ディレクトリ以下に保存されます。


``parameters.dat``
=====================

``parameter`` と ``lattice`` のパラメータが出力されます。

``energy.dat``
==============
サイトあたりのエネルギーが出力されます。

``site_obs.dat``
=================


-  サイト演算子の期待値 :math:`\langle\hat{A}^\alpha_i\rangle` が出力されます。
-  各行4列からなります。

   1. 演算子のインデックス :math:`\alpha`
   2. サイトのインデックス :math:`i`
   3. 期待値の実部 :math:`\mathrm{Re}\langle\hat{A}^\alpha_i\rangle`
   4. 期待値の虚部 :math:`\mathrm{Im}\langle\hat{A}^\alpha_i\rangle`

例
~~

::

    # $1: op_index
    # $2: site_index
    # $3: real
    # $4: imag

    0 0 1.92549465249573365e-02 0.00000000000000000e+00
    0 1 -1.92620814130195529e-02 0.00000000000000000e+00
    0 2 -1.95243093055922252e-02 0.00000000000000000e+00
    0 3 1.91619477632061150e-02 0.00000000000000000e+00
    1 0 4.07206063348768799e-01 0.00000000000000000e+00
    1 1 -4.07243511737157671e-01 0.00000000000000000e+00
    1 2 -4.07255967738734126e-01 0.00000000000000000e+00
    1 3 4.07308918791554009e-01 0.00000000000000000e+00

``neighbor_obs.dat``
======================

-  サイト演算子の最近接相関 :math:`\langle\hat{A}^\alpha_i \hat{A}^\alpha_j\rangle` が出力されます。
-  各行5列からなります。

   1. 演算子のインデックス :math:`\alpha`
   2. サイトのインデックス :math:`i`
   3. サイトのインデックス :math:`j`
   4. 期待値の実部 :math:`\mathrm{Re}\langle\hat{A}^\alpha_i\hat{A}^\alpha_j\rangle`
   5. 期待値の虚部 :math:`\mathrm{Im}\langle\hat{A}^\alpha_i\hat{A}^\alpha_j\rangle`

::

    # $1: op_index
    # $2: source_site
    # $3: target_site
    # $4: real
    # $5: imag

    0 0 1 -7.05927615064968900e-02 0.00000000000000000e+00
    0 0 2 -7.27068456430051274e-02 0.00000000000000000e+00
    0 1 0 -7.13284385957392297e-02 0.00000000000000000e+00
    0 1 3 -7.19523349256113581e-02 0.00000000000000000e+00
    0 2 3 -7.12610364895483045e-02 0.00000000000000000e+00
    0 2 0 -7.19731507561011952e-02 0.00000000000000000e+00
    0 3 2 -7.05633558230210067e-02 0.00000000000000000e+00
    0 3 1 -7.26803750807340498e-02 0.00000000000000000e+00
    1 0 1 -1.85942869237103348e-01 0.00000000000000000e+00
    1 0 2 -1.87164731677545187e-01 0.00000000000000000e+00
    1 1 0 -1.86360382550076586e-01 0.00000000000000000e+00
    1 1 3 -1.86768451086366694e-01 0.00000000000000000e+00
    1 2 3 -1.86384181909805935e-01 0.00000000000000000e+00
    1 2 0 -1.86747576732693515e-01 0.00000000000000000e+00
    1 3 2 -1.85975089525013598e-01 0.00000000000000000e+00
    1 3 1 -1.87196522916879049e-01 0.00000000000000000e+00

``correlation.dat``
=====================

-  相関関数 :math:`C^{\alpha \beta}_{ij}(x,y) \equiv \langle \hat{A}^\alpha_i(0,0) \hat{A}^\beta_j(x,y) \rangle` が出力されます。
-  各行8列から構成されます。

   1. 左演算子のインデックス :math:`\alpha`
   2. 左演算子のサイトインデックス :math:`i`
   3. 右演算子のインデックス :math:`\beta`
   4. 右演算子のサイトインデックス :math:`j`
   5. 右演算子のユニットセルオフセット (x) :math:`x`
   6. 右演算子のユニットセルオフセット (y) :math:`y`
   7. 演算子の実部 :math:`\mathrm{Re}C^{\alpha \beta}_{ij}(x,y)`
   8. 演算子の虚部 :math:`\mathrm{Im}C^{\alpha \beta}_{ij}(x,y)`

例
~~

::

    # $1: left_op
    # $2: left_site
    # $3: right_op
    # $4: right_site
    # $5: offset_x
    # $6: offset_y
    # $7: real
    # $8: imag

    0 0 0 1 0 0 -7.05927615064967928e-02 0.00000000000000000e+00 
    0 0 0 0 1 0 1.19668843226761017e-02 0.00000000000000000e+00 
    0 0 0 1 1 0 -2.43086229320005863e-03 0.00000000000000000e+00 
    0 0 0 0 2 0 7.42729194528496308e-04 0.00000000000000000e+00 
    0 0 0 1 2 0 -4.38794819416885419e-04 0.00000000000000000e+00 
    0 0 0 2 0 0 -7.27068456430051135e-02 0.00000000000000000e+00 
    0 0 0 0 0 1 1.23339845746621279e-02 0.00000000000000000e+00 
    0 0 0 2 0 1 -2.50111186244407349e-03 0.00000000000000000e+00 
    0 0 0 0 0 2 7.54607806587391516e-04 0.00000000000000000e+00 
    0 0 0 2 0 2 -4.47734559969679546e-04 0.00000000000000000e+00 
    1 0 1 1 0 0 -1.85942869237103237e-01 0.00000000000000000e+00 
    （中略）
    1 3 1 1 0 3 -1.65874245891461547e-01 0.00000000000000000e+00

``time.dat``
=====================

計算時間が出力されます。
