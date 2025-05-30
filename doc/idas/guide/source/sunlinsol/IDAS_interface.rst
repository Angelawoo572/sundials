.. ----------------------------------------------------------------
   SUNDIALS Copyright Start
   Copyright (c) 2002-2025, Lawrence Livermore National Security
   and Southern Methodist University.
   All rights reserved.

   See the top-level LICENSE and NOTICE files for details.

   SPDX-License-Identifier: BSD-3-Clause
   SUNDIALS Copyright End
   ----------------------------------------------------------------

.. _SUNLinSol.IDAS:

IDAS SUNLinearSolver interface
==============================

:numref:`SUNLinSol.IDAS.Table` below lists the ``SUNLinearSolver`` module linear solver
functions used within the IDALS interface. As with the ``SUNMatrix`` module, we
emphasize that the IDA user does not need to know detailed usage of linear
solver functions by the IDA code modules in order to use IDA. The
information is presented as an implementation detail for the interested reader.

The linear solver functions listed below are marked with 'x' to
indicate that they are required, or with :math:`\dagger` to indicate that
they are only called if they are non-``NULL`` in the ``SUNLinearSolver``
implementation that is being used. Note:

#. Although IDALS does not call ``SUNLinSolLastFlag``
   directly, this routine is available for users to query linear solver
   issues directly.

#. Although IDALS does not call ``SUNLinSolFree``
   directly, this routine should be available for users to call when
   cleaning up from a simulation.

.. _SUNLinSol.IDAS.Table:
.. table:: List of linear solver function usage in the IDALS interface

   +----------------------------------------+-----------------+-----------------+------------------+
   |                                        |     DIRECT      |    ITERATIVE    | MATRIX_ITERATIVE |
   +========================================+=================+=================+==================+
   | :c:func:`SUNLinSolGetType`             | x               | x               | x                |
   +----------------------------------------+-----------------+-----------------+------------------+
   | :c:func:`SUNLinSolSetATimes`           | :math:`\dagger` | x               | :math:`\dagger`  |
   +----------------------------------------+-----------------+-----------------+------------------+
   | :c:func:`SUNLinSolSetPreconditioner`   | :math:`\dagger` | :math:`\dagger` | :math:`\dagger`  |
   +----------------------------------------+-----------------+-----------------+------------------+
   | :c:func:`SUNLinSolSetScalingVectors`   | :math:`\dagger` | :math:`\dagger` | :math:`\dagger`  |
   +----------------------------------------+-----------------+-----------------+------------------+
   | :c:func:`SUNLinSolInitialize`          | x               | x               | x                |
   +----------------------------------------+-----------------+-----------------+------------------+
   | :c:func:`SUNLinSolSetup`               | x               | x               | x                |
   +----------------------------------------+-----------------+-----------------+------------------+
   | :c:func:`SUNLinSolSolve`               | x               | x               | x                |
   +----------------------------------------+-----------------+-----------------+------------------+
   | :c:func:`SUNLinSolNumIters`            |                 | x               | x                |
   +----------------------------------------+-----------------+-----------------+------------------+
   | :c:func:`SUNLinSolResid`               |                 | x               | x                |
   +----------------------------------------+-----------------+-----------------+------------------+
   | :math:`^1` :c:func:`SUNLinSolLastFlag` |                 |                 |                  |
   +----------------------------------------+-----------------+-----------------+------------------+
   | :math:`^2`\ :c:func:`SUNLinSolFree`    |                 |                 |                  |
   +----------------------------------------+-----------------+-----------------+------------------+
   | :c:func:`SUNLinSolSpace`               | :math:`\dagger` | :math:`\dagger` | :math:`\dagger`  |
   +----------------------------------------+-----------------+-----------------+------------------+

Since there are a wide range of potential ``SUNLinearSolver`` use cases, the following
subsections describe some details of the IDALS interface, in the case that
interested users wish to develop custom ``SUNLinearSolver`` modules.

.. _SUNLinSol.IDAS.Lagged:

Lagged matrix information
-------------------------

If the ``SUNLinearSolver`` object self-identifies as having type
``SUNLINEARSOLVER_DIRECT`` or ``SUNLINEARSOLVER_MATRIX_ITERATIVE``, then the
``SUNLinearSolver`` object solves a linear system *defined* by a ``SUNMatrix``
object. IDALS will update the matrix information infrequently according to the
strategies outlined in :numref:`IDAS.Mathematics`. To this end, we
differentiate between the *desired* linear system :math:`Jx=b` with
:math:`J = \left(\dfrac{\partial F}{\partial y}-c_j \dfrac{\partial F}{\partial\dot{y}}\right)`,
and the *actual* linear system :math:`\bar{J}\bar{x}=b` with

.. math::

   \bar{J} = \dfrac{\partial \bar{F}}{\partial y}
   - \bar{c}_j \dfrac{\partial \bar{F}}{\partial\dot{y}},

where the overlines indicate the lagged versions of these numbers and matrices.

Since IDALS updates the ``SUNMatrix`` objects infrequently and it is likely that
:math:`c_j\ne\bar{c}_j`, then typically :math:`J\ne\bar{J}`. Thus after calling
the ``SUNLinearSolver``-provided ``SUNLinSolSolve`` routine, we test whether
:math:`\dfrac{c_j}{\bar{c}_j} \ne 1`, and if this is
the case we scale the solution :math:`\bar{x}` to correct the linear system
solution :math:`x` via

.. math::
   :label: IDAS_rescaling

   x = \frac{2}{1 + c_j / \bar{c}_j} \bar{x}.

The motivation for this selection of the scaling factor :math:`c = 2/(1 + c_j/\bar{c}_j)`
is discussed in detail in :cite:p:`BBH:89,Hin:00`. In short, if we consider a stationary
iteration for the linear system as consisting of a solve with :math:`\bar{J}`
followed by scaling by :math:`c`, then for a linear constant-coefficient
problem, the error in the solution vector will be reduced at each iteration by
the error matrix :math:`E = I - c \bar{J}^{-1} J`, with a convergence rate given
by the spectral radius of :math:`E`. Assuming that stiff systems have a spectrum
spread widely over the left half-plane, :math:`c` is chosen to minimize the
magnitude of the eigenvalues of :math:`E`.

.. _SUNLinSol.IDAS.Iterative.Tolerance:

Iterative linear solver tolerance
---------------------------------

If the ``SUNLinearSolver`` object self-identifies as having type
``SUNLINEARSOLVER_ITERATIVE`` or
``SUNLINEARSOLVER_MATRIX_ITERATIVE`` then IDALS will set the input
tolerance ``delta`` as described in :numref:`IDAS.Mathematics.ivp_sol`. However, if the
iterative linear solver does not support scaling matrices (i.e., the
``SUNLinSolSetScalingVectors`` routine is ``NULL``), then IDALS will attempt
to adjust the linear solver tolerance to account for this lack of functionality.
To this end, the following assumptions are made:

#. All solution components have similar magnitude; hence the error
   weight vector :math:`W` used in the WRMS norm (see :numref:`IDAS.Mathematics.ivp_sol`)
   should satisfy the assumption

   .. math:: W_i \approx W_{mean},\quad \text{for}\quad i=0,\ldots,n-1.

#. The ``SUNLinearSolver`` object uses a standard 2-norm to measure
   convergence.

Since IDA uses identical left and right scaling matrices,
:math:`S_1 = S_2 = S = \operatorname{diag}(W)`, then the linear
solver convergence requirement is converted as follows
(using the notation from equations :eq:`eq:transformed_linear_system` -- :eq:`eq:transformed_linear_system_components`):

.. math::

   \begin{aligned}
     &\left\| \tilde{b} - \tilde{A} \tilde{x} \right\|_2  <  \text{tol}\\
     \Leftrightarrow \quad & \left\| S P_1^{-1} b - S P_1^{-1} A x \right\|_2  <  \text{tol}\\
     \Leftrightarrow \quad & \sum_{i=0}^{n-1} \left[W_i \left(P_1^{-1} (b - A x)\right)_i\right]^2  <  \text{tol}^2\\
     \Leftrightarrow \quad & W_{mean}^2 \sum_{i=0}^{n-1} \left[\left(P_1^{-1} (b - A x)\right)_i\right]^2  <  \text{tol}^2\\
     \Leftrightarrow \quad & \sum_{i=0}^{n-1} \left[\left(P_1^{-1} (b - A x)\right)_i\right]^2  <  \left(\frac{\text{tol}}{W_{mean}}\right)^2\\
     \Leftrightarrow \quad & \left\| P_1^{-1} (b - A x)\right\|_2  <  \frac{\text{tol}}{W_{mean}}\end{aligned}

Therefore the tolerance scaling factor

.. math:: W_{mean} = \|W\|_2 / \sqrt{n}

is computed and the scaled tolerance ``delta``\ :math:`= \text{tol} / W_{mean}` is
supplied to the ``SUNLinearSolver`` object.
