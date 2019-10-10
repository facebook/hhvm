#ifndef incl_HPHP_RUNTIME_TRACING_H
#define incl_HPHP_RUNTIME_TRACING_H

// THIS FILE SHOULD NOT INCLUDE ANY OTHER HEADERS

namespace HPHP {
namespace tracing {

// Should be a signal number in the real-time range
constexpr uint32_t kSignum = 42;

#define HHVM_TRACING_MAX_MESSAGE_BUFFER_SIZE 0x4000

constexpr int32_t kFileNameMax = 512;
constexpr int32_t kClassNameMax = 128;
constexpr int32_t kFunctionMax = 128;

// https://secure.php.net/manual/en/function.debug-backtrace.php
// PHP backtraces typically includes some other fields as well.
// We are not yet adding them for our tracing purposes yet.
struct backtrace_frame_t {
  uint32_t line;
  char file_name[kFileNameMax];
  char class_name[kClassNameMax];
  char function[kFunctionMax];
};

constexpr uint32_t kMaxStackframes = 200;

struct backtrace_t {
  uint64_t len;
  backtrace_frame_t frames[kMaxStackframes];
};

constexpr uint32_t kFrameStructSize = sizeof(struct backtrace_frame_t);

}
}

#endif
