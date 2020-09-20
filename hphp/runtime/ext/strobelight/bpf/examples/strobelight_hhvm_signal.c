BPF_HASH(hhvm_pids, uint64_t, uint32_t);

// This is meant to be hooked up to a perf event
// It records the time when the triggering signal is fired
int on_event(void* ctx) {
  uint64_t pid_tgid = bpf_get_current_pid_tgid();
  uint64_t pid = pid_tgid >> 32;

  uint32_t* is_hhvm = hhvm_pids.lookup(&pid);
  if (is_hhvm) {
    // Let USDT know thread id of collected traces
    bpf_trace_printk("Sig sent %d\n", pid);
    bpf_send_signal_thread(HHVM_TRACING_SIGNUM);
  }
  return 0;
}
