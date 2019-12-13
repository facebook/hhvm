// Copyright 2004-present Facebook. All Rights Reserved.

#ifdef __cplusplus
// Define based on values in tracing_types.h if it is being included in C++
#define HHVM_TRACING_MAX_STACK_FRAMES HPHP::tracing::kMaxStackframes
#define HHVM_TRACING_FILE_NAME_MAX HPHP::tracing::kFileNameMax
#define HHVM_TRACING_CLASS_NAME_MAX HPHP::tracing::kClassNameMax
#define HHVM_TRACING_FUNCTION_MAX HPHP::tracing::kFunctionMax
#endif

// identical to backtrace_frame_t
typedef struct {
  uint32_t line;
  char file_name[HHVM_TRACING_FILE_NAME_MAX];
  char class_name[HHVM_TRACING_CLASS_NAME_MAX];
  char function[HHVM_TRACING_FUNCTION_MAX];
} hack_symbol_t;

// Data sent to the client through the perf buffer
typedef struct {
  uint32_t pid;
  uint32_t tid;
  int32_t stack_len;
  int32_t stack[HHVM_TRACING_MAX_STACK_FRAMES];
} hack_sample_t;

// The set of temp variables per-cpu
typedef struct {
  uint64_t cur_cpu;
  int64_t symbol_counter;
  hack_symbol_t sym;
  hack_sample_t sample;
} hack_state_t;
