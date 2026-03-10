Usage: /analyze-www YOUR_QUESTION_HERE to ask a question about WWW
and get back detailled stats and code examples in ~2 hours.

Uses your local checkout of WWW, so make sure WWW is up to date and clean.

Hint: Tell Claude to send you a GChat when the analysis is ready.

implementation:
../../.claude/commands/analyze-www.md

Creates a tast logger, log-summarizer, and orchestrator
and runs on all-WWW.

Structure:
- analysis_orchestrator_utils.ml: shared orchestration logic (arg parsing,
  server management, log extraction, pastry upload) used by all analyses
- <name>/: per-analysis directory with a thin orchestrator, types, and summarizer
