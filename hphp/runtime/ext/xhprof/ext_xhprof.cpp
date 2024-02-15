#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/base/array-iterator.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

void HHVM_FUNCTION(fb_setprofile,
  const Variant& callback,
  int64_t flags,
  ArrayArg functions
) {
  g_context->m_setprofileCallback = callback;
  g_context->m_setprofileFlags = flags;
  g_context->m_setprofileFunctions.clear();
  g_context->m_setprofileFunctions.reserve(functions->size());
  IterateV(
    functions.get(),
    [&](TypedValue tv) -> bool {
      if (isStringType(type(tv))) {
        g_context->m_setprofileFunctions.emplace(String(val(tv).pstr));
      }
      return false;
    }
  );
  if (callback.isNull()) {
    HPHP::EventHook::Disable();
  } else {
    HPHP::EventHook::Enable();
  }
}

/////////////////////////////////////////////////////////////////////////////

struct XHProfExtension : Extension {
  XHProfExtension(): Extension("xhprof", "0.9.4", NO_ONCALL_YET) {}

  void moduleRegisterNative() override {
    HHVM_RC_INT(SETPROFILE_FLAGS_ENTERS, EventHook::ProfileEnters);
    HHVM_RC_INT(SETPROFILE_FLAGS_EXITS, EventHook::ProfileExits);
    HHVM_RC_INT(SETPROFILE_FLAGS_DEFAULT, EventHook::ProfileDefault);
    HHVM_RC_INT(SETPROFILE_FLAGS_FRAME_PTRS, EventHook::ProfileFramePointers);
    HHVM_RC_INT(SETPROFILE_FLAGS_CTORS, EventHook::ProfileConstructors);
    HHVM_RC_INT(SETPROFILE_FLAGS_RESUME_AWARE, EventHook::ProfileResumeAware);
    HHVM_RC_INT(SETPROFILE_FLAGS_THIS_OBJECT__MAY_BREAK,
                EventHook::ProfileThisObject);
    HHVM_RC_INT(SETPROFILE_FLAGS_FILE_LINE, EventHook::ProfileFileLine);

    HHVM_FE(fb_setprofile);
  }
} s_xhprof_extension;

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
