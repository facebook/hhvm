
#include <runtime/base/hphp.h>
#include <system/gen/sys/scalar_arrays_remap.h>


namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

StaticArray s_sys_sa00000000;
VarNR s_sys_sva00000000;
StaticArray s_sys_sa4a9657b8;

void SystemScalarArrays::initializeNamed() {
  s_sys_sa00000000 = ssa_[0];
  new (&s_sys_sva00000000) VarNR(s_sys_sa00000000);
  s_sys_sa4a9657b8 = ssa_[1];
}

///////////////////////////////////////////////////////////////////////////////
}
