#include "hphp/runtime/ext/xhprof/ext_xhprof.h"
#include "hphp/runtime/ext/hotprofiler/ext_hotprofiler.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/util/vdso.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

void HHVM_FUNCTION(fb_setprofile, const Variant& callback) {
  if (ThreadInfo::s_threadInfo->m_profiler != nullptr) {
    // phpprof is enabled, don't let PHP code override it
    return;
  }
  g_context->m_setprofileCallback = callback;
  if (callback.isNull()) {
    HPHP::EventHook::Disable();
  } else {
    HPHP::EventHook::Enable();
  }
}

void HHVM_FUNCTION(xhprof_frame_begin, const String& name) {
  Profiler *prof = ThreadInfo::s_threadInfo->m_profiler;
  if (prof) {
    s_profiler_factory->cacheString(name);
    prof->beginFrame(name.data());
  }
}

void HHVM_FUNCTION(xhprof_frame_end) {
  Profiler *prof = ThreadInfo::s_threadInfo->m_profiler;
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

  bool missingClockGetTimeNS = true;
#ifdef CLOCK_THREAD_CPUTIME_ID
  missingClockGetTimeNS = Vdso::ClockGetTimeNS(CLOCK_THREAD_CPUTIME_ID) == -1;
#endif

  if (missingClockGetTimeNS) {
    // Both TrackVtsc and TrackCPU mean "do CPU time profiling".
    //
    // TrackVtsc means: require clock_gettime, or else no data.
    // TrackCPU means: prefer clock_gettime, but fall back to getrusage.
    flags &= ~TrackVtsc;
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
      if (iter.first().toInt32() == 0) {
         flags = iter.second().toInt32();
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

const StaticString
  s_XHPROF_FLAGS_NO_BUILTINS("XHPROF_FLAGS_NO_BUILTINS"),
  s_XHPROF_FLAGS_CPU("XHPROF_FLAGS_CPU"),
  s_XHPROF_FLAGS_MEMORY("XHPROF_FLAGS_MEMORY"),
  s_XHPROF_FLAGS_VTSC("XHPROF_FLAGS_VTSC"),
  s_XHPROF_FLAGS_TRACE("XHPROF_FLAGS_TRACE"),
  s_XHPROF_FLAGS_MEASURE_XHPROF_DISABLE("XHPROF_FLAGS_MEASURE_XHPROF_DISABLE"),
  s_XHPROF_FLAGS_MALLOC("XHPROF_FLAGS_MALLOC"),
  s_XHPROF_FLAGS_I_HAVE_INFINITE_MEMORY("XHPROF_FLAGS_I_HAVE_INFINITE_MEMORY");

class XHProfExtension : public Extension {
 public:
  XHProfExtension(): Extension("xhprof", "0.9.4") {}

  void moduleInit() override {
#define XHPROFCNS(n,v) Native::registerConstant<KindOfInt64> (s_##n.get(), v)
    XHPROFCNS(XHPROF_FLAGS_NO_BUILTINS, NoTrackBuiltins);
    XHPROFCNS(XHPROF_FLAGS_CPU, TrackCPU);
    XHPROFCNS(XHPROF_FLAGS_MEMORY, TrackMemory);
    XHPROFCNS(XHPROF_FLAGS_VTSC, TrackVtsc);
    XHPROFCNS(XHPROF_FLAGS_TRACE, XhpTrace);
    XHPROFCNS(XHPROF_FLAGS_MEASURE_XHPROF_DISABLE, MeasureXhprofDisable);
    XHPROFCNS(XHPROF_FLAGS_MALLOC, TrackMalloc);
    XHPROFCNS(XHPROF_FLAGS_I_HAVE_INFINITE_MEMORY, IHaveInfiniteMemory);
#undef XHPROFCNS

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
