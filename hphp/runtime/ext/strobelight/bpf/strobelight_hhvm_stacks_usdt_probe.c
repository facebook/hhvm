// Copyright 2004-present Facebook. All Rights Reserved.

// Reference Client for HHVM's hack_stack USDT tracepoint
//
// This reads data from the stack data provided by the tracepoint.
//
// It records each frames data in a nomralization map, and creates a
// stack of normalized IDs to pass through a perf buffer

BPF_PERF_OUTPUT(hack_samples);
BPF_PERCPU_ARRAY(hack_state, hack_state_t, 1);
BPF_HASH(hack_symbols_map, hack_symbol_t, uint32_t);

// To avoid duplicate ids, every CPU needs to use a different ID space
// when inserting into the hashmap. NUM_CPUS is defined at compile
// time and passed through -D flag.
static inline __attribute__((__always_inline__)) int64_t get_symbol_id(
    hack_state_t* state) {
  int32_t* symbol_id_ptr = hack_symbols_map.lookup(&state->sym);
  if (symbol_id_ptr) {
    return *symbol_id_ptr;
  }
  // the symbol is new, bump the counter
  int32_t symbol_id = state->symbol_counter * NUM_CPUS + state->cur_cpu;
  state->symbol_counter++;
  hack_symbols_map.update(&state->sym, &symbol_id);
  return symbol_id;
}

// Hook me up to HHVM's hack_stack tracepoint
int on_hhvm_event_hook(struct pt_regs* ctx) {
  uint32_t zero = 0;
  hack_state_t* state = hack_state.lookup(&zero);
  if (!state) {
    // this will never happen
    return 0;
  }

  state->cur_cpu = bpf_get_smp_processor_id();

  // Must be same size as backtrace_t.len
  uint32_t num_frames;
  void* addr;

  uint64_t pid_tgid = bpf_get_current_pid_tgid();
  uint32_t tid = (uint32_t)(pid_tgid);
  uint32_t pid = (uint32_t)(pid_tgid >> 32);
  state->sample.pid = pid;
  state->sample.tid = tid;

  bpf_usdt_readarg(1, ctx, &addr);

  bpf_probe_read(&num_frames, sizeof(num_frames), addr);
  hack_symbol_t* syms = addr + sizeof(num_frames);

#pragma unroll
  for (int i = 0; i < HHVM_TRACING_MAX_STACK_FRAMES; i++) {
    bpf_probe_read(&state->sym, sizeof(hack_symbol_t), &syms[i]);
    state->sample.stack[i] = get_symbol_id(state);
  }

  state->sample.stack_len = num_frames;
  hack_samples.perf_submit(ctx, &state->sample, sizeof(hack_sample_t));

  return 0;
}
