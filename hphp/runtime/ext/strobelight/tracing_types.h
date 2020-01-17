#ifndef incl_HPHP_TRACING_TYPES_H
#define incl_HPHP_TRACING_TYPES_H

// THIS FILE SHOULD NOT INCLUDE ANY OTHER HEADERS

namespace HPHP {
namespace strobelight {

// Should be a signal number in the real-time range
constexpr uint32_t kSignumAll = 42;
constexpr uint32_t kSignumCurrent = 43;

constexpr int32_t kFileNameMax = 128;
constexpr int32_t kClassNameMax = 128;
constexpr int32_t kFunctionMax = 128;
constexpr int32_t kMaxStackframes = 200;

// https://secure.php.net/manual/en/function.debug-backtrace.php
// PHP backtraces typically includes some other fields as well.
// We are not yet adding them for our tracing purposes yet.
struct backtrace_frame_t {
  uint32_t line;
  char file_name[kFileNameMax];
  char class_name[kClassNameMax];
  char function[kFunctionMax];
};

struct backtrace_t {
  uint32_t len;
  backtrace_frame_t frames[kMaxStackframes];
};

}
}

#endif
