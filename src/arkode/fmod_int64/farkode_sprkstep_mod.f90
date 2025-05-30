! This file was automatically generated by SWIG (http://www.swig.org).
! Version 4.0.0
!
! Do not make changes to this file unless you know what you are doing--modify
! the SWIG interface file instead.

! ---------------------------------------------------------------
! Programmer(s): Auto-generated by swig.
! ---------------------------------------------------------------
! SUNDIALS Copyright Start
! Copyright (c) 2002-2025, Lawrence Livermore National Security
! and Southern Methodist University.
! All rights reserved.
!
! See the top-level LICENSE and NOTICE files for details.
!
! SPDX-License-Identifier: BSD-3-Clause
! SUNDIALS Copyright End
! ---------------------------------------------------------------

module farkode_sprkstep_mod
 use, intrinsic :: ISO_C_BINDING
 use farkode_mod
 use fsundials_core_mod
 implicit none
 private

 ! DECLARATION CONSTRUCTS
 integer(C_INT), parameter, public :: SPRKSTEP_DEFAULT_1 = ARKODE_SPRK_EULER_1_1
 integer(C_INT), parameter, public :: SPRKSTEP_DEFAULT_2 = ARKODE_SPRK_LEAPFROG_2_2
 integer(C_INT), parameter, public :: SPRKSTEP_DEFAULT_3 = ARKODE_SPRK_MCLACHLAN_3_3
 integer(C_INT), parameter, public :: SPRKSTEP_DEFAULT_4 = ARKODE_SPRK_MCLACHLAN_4_4
 integer(C_INT), parameter, public :: SPRKSTEP_DEFAULT_5 = ARKODE_SPRK_MCLACHLAN_5_6
 integer(C_INT), parameter, public :: SPRKSTEP_DEFAULT_6 = ARKODE_SPRK_YOSHIDA_6_8
 integer(C_INT), parameter, public :: SPRKSTEP_DEFAULT_8 = ARKODE_SPRK_SUZUKI_UMENO_8_16
 integer(C_INT), parameter, public :: SPRKSTEP_DEFAULT_10 = ARKODE_SPRK_SOFRONIOU_10_36
 public :: FSPRKStepCreate
 public :: FSPRKStepReInit
 public :: FSPRKStepSetUseCompensatedSums
 public :: FSPRKStepSetMethod
 type, bind(C) :: SwigArrayWrapper
  type(C_PTR), public :: data = C_NULL_PTR
  integer(C_SIZE_T), public :: size = 0
 end type
 public :: FSPRKStepSetMethodName
 public :: FSPRKStepGetCurrentMethod
 public :: FSPRKStepReset
 public :: FSPRKStepRootInit
 public :: FSPRKStepSetRootDirection
 public :: FSPRKStepSetNoInactiveRootWarn
 public :: FSPRKStepSetDefaults
 public :: FSPRKStepSetOrder
 public :: FSPRKStepSetInterpolantType
 public :: FSPRKStepSetInterpolantDegree
 public :: FSPRKStepSetMaxNumSteps
 public :: FSPRKStepSetStopTime
 public :: FSPRKStepSetFixedStep
 public :: FSPRKStepSetUserData
 public :: FSPRKStepSetPostprocessStepFn
 public :: FSPRKStepSetPostprocessStageFn
 public :: FSPRKStepEvolve
 public :: FSPRKStepGetDky
 public :: FSPRKStepGetReturnFlagName
 public :: FSPRKStepGetCurrentState
 public :: FSPRKStepGetCurrentStep
 public :: FSPRKStepGetCurrentTime
 public :: FSPRKStepGetLastStep
 public :: FSPRKStepGetNumStepAttempts
 public :: FSPRKStepGetNumSteps
 public :: FSPRKStepGetRootInfo
 public :: FSPRKStepGetUserData
 public :: FSPRKStepPrintAllStats
 public :: FSPRKStepWriteParameters
 public :: FSPRKStepGetStepStats
 public :: FSPRKStepFree
 public :: FSPRKStepGetNumRhsEvals

! WRAPPER DECLARATIONS
interface
function swigc_FSPRKStepCreate(farg1, farg2, farg3, farg4, farg5) &
bind(C, name="_wrap_FSPRKStepCreate") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_FUNPTR), value :: farg1
type(C_FUNPTR), value :: farg2
real(C_DOUBLE), intent(in) :: farg3
type(C_PTR), value :: farg4
type(C_PTR), value :: farg5
type(C_PTR) :: fresult
end function

function swigc_FSPRKStepReInit(farg1, farg2, farg3, farg4, farg5) &
bind(C, name="_wrap_FSPRKStepReInit") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
type(C_FUNPTR), value :: farg2
type(C_FUNPTR), value :: farg3
real(C_DOUBLE), intent(in) :: farg4
type(C_PTR), value :: farg5
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepSetUseCompensatedSums(farg1, farg2) &
bind(C, name="_wrap_FSPRKStepSetUseCompensatedSums") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
integer(C_INT), intent(in) :: farg2
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepSetMethod(farg1, farg2) &
bind(C, name="_wrap_FSPRKStepSetMethod") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
type(C_PTR), value :: farg2
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepSetMethodName(farg1, farg2) &
bind(C, name="_wrap_FSPRKStepSetMethodName") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
import :: swigarraywrapper
type(C_PTR), value :: farg1
type(SwigArrayWrapper) :: farg2
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepGetCurrentMethod(farg1, farg2) &
bind(C, name="_wrap_FSPRKStepGetCurrentMethod") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
type(C_PTR), value :: farg2
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepReset(farg1, farg2, farg3) &
bind(C, name="_wrap_FSPRKStepReset") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
real(C_DOUBLE), intent(in) :: farg2
type(C_PTR), value :: farg3
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepRootInit(farg1, farg2, farg3) &
bind(C, name="_wrap_FSPRKStepRootInit") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
integer(C_INT), intent(in) :: farg2
type(C_FUNPTR), value :: farg3
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepSetRootDirection(farg1, farg2) &
bind(C, name="_wrap_FSPRKStepSetRootDirection") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
type(C_PTR), value :: farg2
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepSetNoInactiveRootWarn(farg1) &
bind(C, name="_wrap_FSPRKStepSetNoInactiveRootWarn") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepSetDefaults(farg1) &
bind(C, name="_wrap_FSPRKStepSetDefaults") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepSetOrder(farg1, farg2) &
bind(C, name="_wrap_FSPRKStepSetOrder") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
integer(C_INT), intent(in) :: farg2
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepSetInterpolantType(farg1, farg2) &
bind(C, name="_wrap_FSPRKStepSetInterpolantType") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
integer(C_INT), intent(in) :: farg2
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepSetInterpolantDegree(farg1, farg2) &
bind(C, name="_wrap_FSPRKStepSetInterpolantDegree") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
integer(C_INT), intent(in) :: farg2
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepSetMaxNumSteps(farg1, farg2) &
bind(C, name="_wrap_FSPRKStepSetMaxNumSteps") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
integer(C_LONG), intent(in) :: farg2
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepSetStopTime(farg1, farg2) &
bind(C, name="_wrap_FSPRKStepSetStopTime") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
real(C_DOUBLE), intent(in) :: farg2
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepSetFixedStep(farg1, farg2) &
bind(C, name="_wrap_FSPRKStepSetFixedStep") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
real(C_DOUBLE), intent(in) :: farg2
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepSetUserData(farg1, farg2) &
bind(C, name="_wrap_FSPRKStepSetUserData") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
type(C_PTR), value :: farg2
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepSetPostprocessStepFn(farg1, farg2) &
bind(C, name="_wrap_FSPRKStepSetPostprocessStepFn") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
type(C_FUNPTR), value :: farg2
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepSetPostprocessStageFn(farg1, farg2) &
bind(C, name="_wrap_FSPRKStepSetPostprocessStageFn") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
type(C_FUNPTR), value :: farg2
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepEvolve(farg1, farg2, farg3, farg4, farg5) &
bind(C, name="_wrap_FSPRKStepEvolve") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
real(C_DOUBLE), intent(in) :: farg2
type(C_PTR), value :: farg3
type(C_PTR), value :: farg4
integer(C_INT), intent(in) :: farg5
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepGetDky(farg1, farg2, farg3, farg4) &
bind(C, name="_wrap_FSPRKStepGetDky") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
real(C_DOUBLE), intent(in) :: farg2
integer(C_INT), intent(in) :: farg3
type(C_PTR), value :: farg4
integer(C_INT) :: fresult
end function

 subroutine SWIG_free(cptr) &
  bind(C, name="free")
 use, intrinsic :: ISO_C_BINDING
 type(C_PTR), value :: cptr
end subroutine
function swigc_FSPRKStepGetReturnFlagName(farg1) &
bind(C, name="_wrap_FSPRKStepGetReturnFlagName") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
import :: swigarraywrapper
integer(C_LONG), intent(in) :: farg1
type(SwigArrayWrapper) :: fresult
end function

function swigc_FSPRKStepGetCurrentState(farg1, farg2) &
bind(C, name="_wrap_FSPRKStepGetCurrentState") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
type(C_PTR), value :: farg2
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepGetCurrentStep(farg1, farg2) &
bind(C, name="_wrap_FSPRKStepGetCurrentStep") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
type(C_PTR), value :: farg2
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepGetCurrentTime(farg1, farg2) &
bind(C, name="_wrap_FSPRKStepGetCurrentTime") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
type(C_PTR), value :: farg2
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepGetLastStep(farg1, farg2) &
bind(C, name="_wrap_FSPRKStepGetLastStep") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
type(C_PTR), value :: farg2
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepGetNumStepAttempts(farg1, farg2) &
bind(C, name="_wrap_FSPRKStepGetNumStepAttempts") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
type(C_PTR), value :: farg2
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepGetNumSteps(farg1, farg2) &
bind(C, name="_wrap_FSPRKStepGetNumSteps") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
type(C_PTR), value :: farg2
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepGetRootInfo(farg1, farg2) &
bind(C, name="_wrap_FSPRKStepGetRootInfo") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
type(C_PTR), value :: farg2
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepGetUserData(farg1, farg2) &
bind(C, name="_wrap_FSPRKStepGetUserData") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
type(C_PTR), value :: farg2
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepPrintAllStats(farg1, farg2, farg3) &
bind(C, name="_wrap_FSPRKStepPrintAllStats") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
type(C_PTR), value :: farg2
integer(C_INT), intent(in) :: farg3
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepWriteParameters(farg1, farg2) &
bind(C, name="_wrap_FSPRKStepWriteParameters") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
type(C_PTR), value :: farg2
integer(C_INT) :: fresult
end function

function swigc_FSPRKStepGetStepStats(farg1, farg2, farg3, farg4, farg5, farg6) &
bind(C, name="_wrap_FSPRKStepGetStepStats") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
type(C_PTR), value :: farg2
type(C_PTR), value :: farg3
type(C_PTR), value :: farg4
type(C_PTR), value :: farg5
type(C_PTR), value :: farg6
integer(C_INT) :: fresult
end function

subroutine swigc_FSPRKStepFree(farg1) &
bind(C, name="_wrap_FSPRKStepFree")
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
end subroutine

function swigc_FSPRKStepGetNumRhsEvals(farg1, farg2, farg3) &
bind(C, name="_wrap_FSPRKStepGetNumRhsEvals") &
result(fresult)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), value :: farg1
type(C_PTR), value :: farg2
type(C_PTR), value :: farg3
integer(C_INT) :: fresult
end function

end interface


contains
 ! MODULE SUBPROGRAMS
function FSPRKStepCreate(f1, f2, t0, y0, sunctx) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
type(C_PTR) :: swig_result
type(C_FUNPTR), intent(in), value :: f1
type(C_FUNPTR), intent(in), value :: f2
real(C_DOUBLE), intent(in) :: t0
type(N_Vector), target, intent(inout) :: y0
type(C_PTR) :: sunctx
type(C_PTR) :: fresult 
type(C_FUNPTR) :: farg1 
type(C_FUNPTR) :: farg2 
real(C_DOUBLE) :: farg3 
type(C_PTR) :: farg4 
type(C_PTR) :: farg5 

farg1 = f1
farg2 = f2
farg3 = t0
farg4 = c_loc(y0)
farg5 = sunctx
fresult = swigc_FSPRKStepCreate(farg1, farg2, farg3, farg4, farg5)
swig_result = fresult
end function

function FSPRKStepReInit(arkode_mem, f1, f2, t0, y0) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
type(C_FUNPTR), intent(in), value :: f1
type(C_FUNPTR), intent(in), value :: f2
real(C_DOUBLE), intent(in) :: t0
type(N_Vector), target, intent(inout) :: y0
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
type(C_FUNPTR) :: farg2 
type(C_FUNPTR) :: farg3 
real(C_DOUBLE) :: farg4 
type(C_PTR) :: farg5 

farg1 = arkode_mem
farg2 = f1
farg3 = f2
farg4 = t0
farg5 = c_loc(y0)
fresult = swigc_FSPRKStepReInit(farg1, farg2, farg3, farg4, farg5)
swig_result = fresult
end function

function FSPRKStepSetUseCompensatedSums(arkode_mem, onoff) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
integer(C_INT), intent(in) :: onoff
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
integer(C_INT) :: farg2 

farg1 = arkode_mem
farg2 = onoff
fresult = swigc_FSPRKStepSetUseCompensatedSums(farg1, farg2)
swig_result = fresult
end function

function FSPRKStepSetMethod(arkode_mem, sprk_storage) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
type(C_PTR) :: sprk_storage
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
type(C_PTR) :: farg2 

farg1 = arkode_mem
farg2 = sprk_storage
fresult = swigc_FSPRKStepSetMethod(farg1, farg2)
swig_result = fresult
end function


subroutine SWIG_string_to_chararray(string, chars, wrap)
  use, intrinsic :: ISO_C_BINDING
  character(kind=C_CHAR, len=*), intent(IN) :: string
  character(kind=C_CHAR), dimension(:), target, allocatable, intent(OUT) :: chars
  type(SwigArrayWrapper), intent(OUT) :: wrap
  integer :: i

  allocate(character(kind=C_CHAR) :: chars(len(string) + 1))
  do i=1,len(string)
    chars(i) = string(i:i)
  end do
  i = len(string) + 1
  chars(i) = C_NULL_CHAR ! C string compatibility
  wrap%data = c_loc(chars)
  wrap%size = len(string)
end subroutine

function FSPRKStepSetMethodName(arkode_mem, method) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
character(kind=C_CHAR, len=*), target :: method
character(kind=C_CHAR), dimension(:), allocatable, target :: farg2_chars
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
type(SwigArrayWrapper) :: farg2 

farg1 = arkode_mem
call SWIG_string_to_chararray(method, farg2_chars, farg2)
fresult = swigc_FSPRKStepSetMethodName(farg1, farg2)
swig_result = fresult
end function

function FSPRKStepGetCurrentMethod(arkode_mem, sprk_storage) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
type(C_PTR), target, intent(inout) :: sprk_storage
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
type(C_PTR) :: farg2 

farg1 = arkode_mem
farg2 = c_loc(sprk_storage)
fresult = swigc_FSPRKStepGetCurrentMethod(farg1, farg2)
swig_result = fresult
end function

function FSPRKStepReset(arkode_mem, tr, yr) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
real(C_DOUBLE), intent(in) :: tr
type(N_Vector), target, intent(inout) :: yr
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
real(C_DOUBLE) :: farg2 
type(C_PTR) :: farg3 

farg1 = arkode_mem
farg2 = tr
farg3 = c_loc(yr)
fresult = swigc_FSPRKStepReset(farg1, farg2, farg3)
swig_result = fresult
end function

function FSPRKStepRootInit(arkode_mem, nrtfn, g) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
integer(C_INT), intent(in) :: nrtfn
type(C_FUNPTR), intent(in), value :: g
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
integer(C_INT) :: farg2 
type(C_FUNPTR) :: farg3 

farg1 = arkode_mem
farg2 = nrtfn
farg3 = g
fresult = swigc_FSPRKStepRootInit(farg1, farg2, farg3)
swig_result = fresult
end function

function FSPRKStepSetRootDirection(arkode_mem, rootdir) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
integer(C_INT), dimension(*), target, intent(inout) :: rootdir
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
type(C_PTR) :: farg2 

farg1 = arkode_mem
farg2 = c_loc(rootdir(1))
fresult = swigc_FSPRKStepSetRootDirection(farg1, farg2)
swig_result = fresult
end function

function FSPRKStepSetNoInactiveRootWarn(arkode_mem) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 

farg1 = arkode_mem
fresult = swigc_FSPRKStepSetNoInactiveRootWarn(farg1)
swig_result = fresult
end function

function FSPRKStepSetDefaults(arkode_mem) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 

farg1 = arkode_mem
fresult = swigc_FSPRKStepSetDefaults(farg1)
swig_result = fresult
end function

function FSPRKStepSetOrder(arkode_mem, maxord) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
integer(C_INT), intent(in) :: maxord
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
integer(C_INT) :: farg2 

farg1 = arkode_mem
farg2 = maxord
fresult = swigc_FSPRKStepSetOrder(farg1, farg2)
swig_result = fresult
end function

function FSPRKStepSetInterpolantType(arkode_mem, itype) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
integer(C_INT), intent(in) :: itype
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
integer(C_INT) :: farg2 

farg1 = arkode_mem
farg2 = itype
fresult = swigc_FSPRKStepSetInterpolantType(farg1, farg2)
swig_result = fresult
end function

function FSPRKStepSetInterpolantDegree(arkode_mem, degree) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
integer(C_INT), intent(in) :: degree
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
integer(C_INT) :: farg2 

farg1 = arkode_mem
farg2 = degree
fresult = swigc_FSPRKStepSetInterpolantDegree(farg1, farg2)
swig_result = fresult
end function

function FSPRKStepSetMaxNumSteps(arkode_mem, mxsteps) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
integer(C_LONG), intent(in) :: mxsteps
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
integer(C_LONG) :: farg2 

farg1 = arkode_mem
farg2 = mxsteps
fresult = swigc_FSPRKStepSetMaxNumSteps(farg1, farg2)
swig_result = fresult
end function

function FSPRKStepSetStopTime(arkode_mem, tstop) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
real(C_DOUBLE), intent(in) :: tstop
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
real(C_DOUBLE) :: farg2 

farg1 = arkode_mem
farg2 = tstop
fresult = swigc_FSPRKStepSetStopTime(farg1, farg2)
swig_result = fresult
end function

function FSPRKStepSetFixedStep(arkode_mem, hfixed) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
real(C_DOUBLE), intent(in) :: hfixed
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
real(C_DOUBLE) :: farg2 

farg1 = arkode_mem
farg2 = hfixed
fresult = swigc_FSPRKStepSetFixedStep(farg1, farg2)
swig_result = fresult
end function

function FSPRKStepSetUserData(arkode_mem, user_data) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
type(C_PTR) :: user_data
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
type(C_PTR) :: farg2 

farg1 = arkode_mem
farg2 = user_data
fresult = swigc_FSPRKStepSetUserData(farg1, farg2)
swig_result = fresult
end function

function FSPRKStepSetPostprocessStepFn(arkode_mem, processstep) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
type(C_FUNPTR), intent(in), value :: processstep
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
type(C_FUNPTR) :: farg2 

farg1 = arkode_mem
farg2 = processstep
fresult = swigc_FSPRKStepSetPostprocessStepFn(farg1, farg2)
swig_result = fresult
end function

function FSPRKStepSetPostprocessStageFn(arkode_mem, processstage) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
type(C_FUNPTR), intent(in), value :: processstage
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
type(C_FUNPTR) :: farg2 

farg1 = arkode_mem
farg2 = processstage
fresult = swigc_FSPRKStepSetPostprocessStageFn(farg1, farg2)
swig_result = fresult
end function

function FSPRKStepEvolve(arkode_mem, tout, yout, tret, itask) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
real(C_DOUBLE), intent(in) :: tout
type(N_Vector), target, intent(inout) :: yout
real(C_DOUBLE), dimension(*), target, intent(inout) :: tret
integer(C_INT), intent(in) :: itask
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
real(C_DOUBLE) :: farg2 
type(C_PTR) :: farg3 
type(C_PTR) :: farg4 
integer(C_INT) :: farg5 

farg1 = arkode_mem
farg2 = tout
farg3 = c_loc(yout)
farg4 = c_loc(tret(1))
farg5 = itask
fresult = swigc_FSPRKStepEvolve(farg1, farg2, farg3, farg4, farg5)
swig_result = fresult
end function

function FSPRKStepGetDky(arkode_mem, t, k, dky) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
real(C_DOUBLE), intent(in) :: t
integer(C_INT), intent(in) :: k
type(N_Vector), target, intent(inout) :: dky
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
real(C_DOUBLE) :: farg2 
integer(C_INT) :: farg3 
type(C_PTR) :: farg4 

farg1 = arkode_mem
farg2 = t
farg3 = k
farg4 = c_loc(dky)
fresult = swigc_FSPRKStepGetDky(farg1, farg2, farg3, farg4)
swig_result = fresult
end function


subroutine SWIG_chararray_to_string(wrap, string)
  use, intrinsic :: ISO_C_BINDING
  type(SwigArrayWrapper), intent(IN) :: wrap
  character(kind=C_CHAR, len=:), allocatable, intent(OUT) :: string
  character(kind=C_CHAR), dimension(:), pointer :: chars
  integer(kind=C_SIZE_T) :: i
  call c_f_pointer(wrap%data, chars, [wrap%size])
  allocate(character(kind=C_CHAR, len=wrap%size) :: string)
  do i=1, wrap%size
    string(i:i) = chars(i)
  end do
end subroutine

function FSPRKStepGetReturnFlagName(flag) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
character(kind=C_CHAR, len=:), allocatable :: swig_result
integer(C_LONG), intent(in) :: flag
type(SwigArrayWrapper) :: fresult 
integer(C_LONG) :: farg1 

farg1 = flag
fresult = swigc_FSPRKStepGetReturnFlagName(farg1)
call SWIG_chararray_to_string(fresult, swig_result)
if (.false.) call SWIG_free(fresult%data)
end function

function FSPRKStepGetCurrentState(arkode_mem, state) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
type(C_PTR) :: state
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
type(C_PTR) :: farg2 

farg1 = arkode_mem
farg2 = state
fresult = swigc_FSPRKStepGetCurrentState(farg1, farg2)
swig_result = fresult
end function

function FSPRKStepGetCurrentStep(arkode_mem, hcur) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
real(C_DOUBLE), dimension(*), target, intent(inout) :: hcur
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
type(C_PTR) :: farg2 

farg1 = arkode_mem
farg2 = c_loc(hcur(1))
fresult = swigc_FSPRKStepGetCurrentStep(farg1, farg2)
swig_result = fresult
end function

function FSPRKStepGetCurrentTime(arkode_mem, tcur) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
real(C_DOUBLE), dimension(*), target, intent(inout) :: tcur
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
type(C_PTR) :: farg2 

farg1 = arkode_mem
farg2 = c_loc(tcur(1))
fresult = swigc_FSPRKStepGetCurrentTime(farg1, farg2)
swig_result = fresult
end function

function FSPRKStepGetLastStep(arkode_mem, hlast) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
real(C_DOUBLE), dimension(*), target, intent(inout) :: hlast
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
type(C_PTR) :: farg2 

farg1 = arkode_mem
farg2 = c_loc(hlast(1))
fresult = swigc_FSPRKStepGetLastStep(farg1, farg2)
swig_result = fresult
end function

function FSPRKStepGetNumStepAttempts(arkode_mem, step_attempts) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
integer(C_LONG), dimension(*), target, intent(inout) :: step_attempts
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
type(C_PTR) :: farg2 

farg1 = arkode_mem
farg2 = c_loc(step_attempts(1))
fresult = swigc_FSPRKStepGetNumStepAttempts(farg1, farg2)
swig_result = fresult
end function

function FSPRKStepGetNumSteps(arkode_mem, nsteps) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
integer(C_LONG), dimension(*), target, intent(inout) :: nsteps
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
type(C_PTR) :: farg2 

farg1 = arkode_mem
farg2 = c_loc(nsteps(1))
fresult = swigc_FSPRKStepGetNumSteps(farg1, farg2)
swig_result = fresult
end function

function FSPRKStepGetRootInfo(arkode_mem, rootsfound) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
integer(C_INT), dimension(*), target, intent(inout) :: rootsfound
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
type(C_PTR) :: farg2 

farg1 = arkode_mem
farg2 = c_loc(rootsfound(1))
fresult = swigc_FSPRKStepGetRootInfo(farg1, farg2)
swig_result = fresult
end function

function FSPRKStepGetUserData(arkode_mem, user_data) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
type(C_PTR), target, intent(inout) :: user_data
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
type(C_PTR) :: farg2 

farg1 = arkode_mem
farg2 = c_loc(user_data)
fresult = swigc_FSPRKStepGetUserData(farg1, farg2)
swig_result = fresult
end function

function FSPRKStepPrintAllStats(arkode_mem, outfile, fmt) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
type(C_PTR) :: outfile
integer(SUNOutputFormat), intent(in) :: fmt
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
type(C_PTR) :: farg2 
integer(C_INT) :: farg3 

farg1 = arkode_mem
farg2 = outfile
farg3 = fmt
fresult = swigc_FSPRKStepPrintAllStats(farg1, farg2, farg3)
swig_result = fresult
end function

function FSPRKStepWriteParameters(arkode_mem, fp) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
type(C_PTR) :: fp
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
type(C_PTR) :: farg2 

farg1 = arkode_mem
farg2 = fp
fresult = swigc_FSPRKStepWriteParameters(farg1, farg2)
swig_result = fresult
end function

function FSPRKStepGetStepStats(arkode_mem, nsteps, hinused, hlast, hcur, tcur) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
integer(C_LONG), dimension(*), target, intent(inout) :: nsteps
real(C_DOUBLE), dimension(*), target, intent(inout) :: hinused
real(C_DOUBLE), dimension(*), target, intent(inout) :: hlast
real(C_DOUBLE), dimension(*), target, intent(inout) :: hcur
real(C_DOUBLE), dimension(*), target, intent(inout) :: tcur
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
type(C_PTR) :: farg2 
type(C_PTR) :: farg3 
type(C_PTR) :: farg4 
type(C_PTR) :: farg5 
type(C_PTR) :: farg6 

farg1 = arkode_mem
farg2 = c_loc(nsteps(1))
farg3 = c_loc(hinused(1))
farg4 = c_loc(hlast(1))
farg5 = c_loc(hcur(1))
farg6 = c_loc(tcur(1))
fresult = swigc_FSPRKStepGetStepStats(farg1, farg2, farg3, farg4, farg5, farg6)
swig_result = fresult
end function

subroutine FSPRKStepFree(arkode_mem)
use, intrinsic :: ISO_C_BINDING
type(C_PTR), target, intent(inout) :: arkode_mem
type(C_PTR) :: farg1 

farg1 = c_loc(arkode_mem)
call swigc_FSPRKStepFree(farg1)
end subroutine

function FSPRKStepGetNumRhsEvals(arkode_mem, nf1, nf2) &
result(swig_result)
use, intrinsic :: ISO_C_BINDING
integer(C_INT) :: swig_result
type(C_PTR) :: arkode_mem
integer(C_LONG), dimension(*), target, intent(inout) :: nf1
integer(C_LONG), dimension(*), target, intent(inout) :: nf2
integer(C_INT) :: fresult 
type(C_PTR) :: farg1 
type(C_PTR) :: farg2 
type(C_PTR) :: farg3 

farg1 = arkode_mem
farg2 = c_loc(nf1(1))
farg3 = c_loc(nf2(1))
fresult = swigc_FSPRKStepGetNumRhsEvals(farg1, farg2, farg3)
swig_result = fresult
end function


end module
