
#include <runtime/base/hphp.h>
#include <sys/system_globals.h>
#include <sys/literal_strings.h>

namespace hphp_impl_starter {}

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

StaticArray SystemScalarArrays::ssa_[2];

void SystemScalarArrays::initialize() {
  ssa_[0] = StaticArray(ArrayData::Create());
  ssa_[0].setEvalScalar();
  ssa_[1] = StaticArray(ArrayInit(9).add(NAMSTR(s_sys_ss558db28e, "bool"), 1LL, true).add(NAMSTR(s_sys_ss336538f4, "boolean"), 1LL, true).add(NAMSTR(s_sys_ss128a1ef0, "int"), 1LL, true).add(NAMSTR(s_sys_ss362ff05d, "integer"), 1LL, true).add(NAMSTR(s_sys_ss2da7d86e, "real"), 1LL, true).add(NAMSTR(s_sys_ss2ea2d57a, "double"), 1LL, true).add(NAMSTR(s_sys_ss119bffe4, "float"), 1LL, true).add(NAMSTR(s_sys_ss69ad4382, "string"), 1LL, true).add(NAMSTR(s_sys_ss0c04b960, "array"), 1LL, true).create());
  ssa_[1].setEvalScalar();
  SystemScalarArrays::initializeNamed();
}

///////////////////////////////////////////////////////////////////////////////
}
