#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/vm/jit/prof-data-serialize.h"

namespace HPHP {

String HHVM_FUNCTION(sb_profile_ser,
                     const String& sb_root,
                     const String& prof_path) {
  auto const status = jit::serializeSBProfData(sb_root.data(), prof_path.data());
  return String{status.c_str()};
}

String HHVM_FUNCTION(sb_profile_deser,
                     const String& sb_root,
                     const String& prof_path) {
  auto const status = jit::deserializeSBProfData(sb_root.data(), prof_path.data());
  return String{status.c_str()};
}

static struct SbProfileExtension final : Extension {
  SbProfileExtension()
    : Extension("sb_profile", NO_EXTENSION_VERSION_YET, "hphp_hphpi") {}
  void moduleRegisterNative() override {
    HHVM_FE(sb_profile_ser);
    HHVM_FE(sb_profile_deser);
  }
} s_sb_profile_extension;
}
