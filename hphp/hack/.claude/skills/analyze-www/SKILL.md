---
name: analyze-www
description: "Analyze code patterns across WWW using a TAST logger. Uses hh_distc for fast distributed type checking (~20 min)."
user-invocable: true
disable-model-invocation: true
argument-hint: "<question about WWW code patterns>"
metadata:
  strict: true
  oncalls:
    - hack
---

Read and follow `.llms/skills/analyze-www/SKILL.md` exactly.
Treat `$ARGUMENTS` as the question to analyze.
