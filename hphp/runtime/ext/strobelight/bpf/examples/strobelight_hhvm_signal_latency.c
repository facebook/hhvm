// Measures the latency between sending a signal and
// the signal handler firing

BPF_HASH(hhvm_pids, uint64_t, uint32_t);
BPF_HASH(send_time, uint64_t, uint64_t);
BPF_HISTOGRAM(dist, uint64_t);

// This is meant to be hooked up to a perf event
// It records the time when the triggering signal is fired
int on_event(void* ctx) {
  uint64_t pid_tgid = bpf_get_current_pid_tgid();
  uint64_t pid = pid_tgid >> 32;

  uint32_t* is_hhvm = hhvm_pids.lookup(&pid);
  if (is_hhvm) {
    // Let USDT know thread id of collected traces
    uint64_t ktime_ns = bpf_ktime_get_ns();
    bpf_send_signal_thread(HHVM_TRACING_SIGNUM);
    send_time.insert(&pid, &ktime_ns);
  }
  return 0;
}

// This is meant to be hooked up to the USDT hhvm_surprise which
// indicates when all of the surprise flags have just been set.  The
// latency between this time and the time in on_event is critical and
// must be small
int on_hhvm_signal_handler(struct pt_regs* ctx) {
  uint64_t latency_us = 0;
  uint64_t ktime_ns = bpf_ktime_get_ns();

  uint64_t pid_tgid = bpf_get_current_pid_tgid();
  uint64_t pid = pid_tgid >> 32;

  uint64_t* start_time_ns = send_time.lookup(&pid);
  if (!start_time_ns) {
    // this will never happen
    return 0;
  }

  latency_us = (ktime_ns - *start_time_ns) / 1000;
  bpf_trace_printk("SIGNAL BY %d HANDLED IN %lluus\n", pid, latency_us);
  dist.increment(bpf_log2l(latency_us));

  send_time.delete(&pid);
  return 0;
}
