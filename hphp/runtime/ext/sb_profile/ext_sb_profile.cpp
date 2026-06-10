#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/vm/jit/prof-data-serialize.h"

namespace HPHP {

OptString HHVM_FUNCTION(sb_profile_ser,
                     const OptString& sb_root,
                     const OptString& prof_path) {
  auto const status = jit::serializeSBProfData(sb_root.data(), prof_path.data());
  return OptString{status.c_str()};
}

OptString HHVM_FUNCTION(sb_profile_deser,
                     const OptString& sb_root,
                     const OptString& prof_path,
                     int64_t warmup) {
  auto const flags = static_cast<jit::SBWarmupFlags>(warmup);
  auto const status =
    jit::deserializeSBProfData(sb_root.data(), prof_path.data(), flags);
  return OptString{status.c_str()};
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
