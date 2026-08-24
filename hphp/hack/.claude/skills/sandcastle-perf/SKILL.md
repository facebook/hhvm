---
name: sandcastle-perf
description: "Run or monitor Sandcastle hh_perf jobs for Hack performance benchmarking."
user-invocable: true
disable-model-invocation: true
argument-hint: "<run or job to start or monitor>"
metadata:
  strict: true
  oncalls:
    - hack
---

Read and follow `.llms/skills/sandcastle-perf/SKILL.md` exactly.
Treat `$ARGUMENTS` as the requested Sandcastle performance task.
