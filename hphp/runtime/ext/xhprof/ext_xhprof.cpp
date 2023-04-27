#include "hphp/runtime/ext/xhprof/ext_xhprof.h"
#include "hphp/runtime/ext/hotprofiler/ext_hotprofiler.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/base/array-iterator.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

void HHVM_FUNCTION(fb_setprofile,
  const Variant& callback,
  int64_t flags,
  ArrayArg functions
) {
  if (RequestInfo::s_requestInfo->m_profiler != nullptr) {
    // phpprof is enabled, don't let PHP code override it
    return;
  }
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

void HHVM_FUNCTION(xhprof_frame_begin, const String& name) {
  Profiler *prof = RequestInfo::s_requestInfo->m_profiler;
  if (prof) {
    s_profiler_factory->cacheString(name);
    prof->beginFrame(name.data());
  }
}

void HHVM_FUNCTION(xhprof_frame_end) {
  Profiler *prof = RequestInfo::s_requestInfo->m_profiler;
  if (prof) {
    prof->endFrame(nullptr, nullptr);
  }
}

void HHVM_FUNCTION(xhprof_enable, int64_t flags/* = 0 */,
                                  const Array& args /* = null_array */) {
  if (!RuntimeOption::EnableHotProfiler) {
    raise_warning("The runtime option Stats.EnableHotProfiler must be on to "
                  "use xhprof.");
    return;
  }

  if (flags & TrackVtsc) {
    flags |= TrackCPU;
  }

  if (flags & XhpTrace) {
    s_profiler_factory->start(ProfilerKind::Trace, flags);
  } else if (flags & Memo) {
    flags = 0;  /* flags are not used by MemoProfiler::MemoProfiler */
    s_profiler_factory->start(ProfilerKind::Memo, flags);
  } else if (flags & External) {
    for (ArrayIter iter(args); iter; ++iter) {
      if ((int)iter.first().toInt64() == 0) {
         flags = (int)iter.second().toInt64();
      }
    }
    s_profiler_factory->start(ProfilerKind::External, flags);
  } else {
    s_profiler_factory->start(ProfilerKind::Hierarchical, flags);
  }
}

Variant HHVM_FUNCTION(xhprof_disable) {
  return s_profiler_factory->stop();
}

void HHVM_FUNCTION(xhprof_network_enable) {
  ServerStats::StartNetworkProfile();
}

Variant HHVM_FUNCTION(xhprof_network_disable) {
  return ServerStats::EndNetworkProfile();
}

void HHVM_FUNCTION(xhprof_sample_enable) {
  if (RuntimeOption::EnableHotProfiler) {
    s_profiler_factory->start(ProfilerKind::Sample, 0);
  } else {
    raise_warning("The runtime option Stats.EnableHotProfiler must be on to "
                  "use xhprof.");
  }
}

Variant HHVM_FUNCTION(xhprof_sample_disable) {
  return s_profiler_factory->stop();
}

/////////////////////////////////////////////////////////////////////////////

struct XHProfExtension : Extension {
  XHProfExtension(): Extension("xhprof", "0.9.4") {}

  void moduleInit() override {
    HHVM_RC_INT(XHPROF_FLAGS_NO_BUILTINS, NoTrackBuiltins);
    HHVM_RC_INT(XHPROF_FLAGS_CPU, TrackCPU);
    HHVM_RC_INT(XHPROF_FLAGS_MEMORY, TrackMemory);
    HHVM_RC_INT(XHPROF_FLAGS_VTSC, TrackVtsc);
    HHVM_RC_INT(XHPROF_FLAGS_TRACE, XhpTrace);
    HHVM_RC_INT(XHPROF_FLAGS_MEASURE_XHPROF_DISABLE, MeasureXhprofDisable);
    HHVM_RC_INT(XHPROF_FLAGS_MALLOC, TrackMalloc);
    HHVM_RC_INT(XHPROF_FLAGS_I_HAVE_INFINITE_MEMORY, IHaveInfiniteMemory);
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
    HHVM_FE(xhprof_frame_begin);
    HHVM_FE(xhprof_frame_end);
    HHVM_FE(xhprof_enable);
    HHVM_FE(xhprof_disable);
    HHVM_FE(xhprof_network_enable);
    HHVM_FE(xhprof_network_disable);
    HHVM_FE(xhprof_sample_enable);
    HHVM_FE(xhprof_sample_disable);

    loadSystemlib();
  }
} s_xhprof_extension;

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
