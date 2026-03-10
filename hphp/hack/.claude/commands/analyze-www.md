---
description: "Analyze code patterns across WWW using a TAST logger. Uses hh_distc for fast distributed type checking (~20 min). Example: /analyze-www How are as type assertions used?"
---

# Analyze WWW Code Patterns

You are answering a question about code patterns in the WWW (Hack) codebase by
building and running a TAST (Typed Abstract Syntax Tree) logger analysis.

**Question to answer:** $ARGUMENTS

## Overview

You will:
1. Design a TAST logger to collect data about the pattern
2. Implement: shared types library, logger, summarizer, orchestrator, tests
3. Build, test, lint
4. Commit and submit a draft diff
5. Run on `~/www` via hh_distc (~20 min), monitoring every 5 minutes
6. Analyze results and report with pastry links

## Prerequisites

- WWW should be clean - warn the user if `sl status` in WWW shows changes but continue anyway.
If unclean: make sure to include a note in your commit messages that www wasn't clean, with sl status output.
- `sl id` to get the commit hash of the www you are analyzing. you will need this later in your commit messages.
- Work independently. The interaction flow is user makes a single query and gets back a stack of diffs with detailled stats and examples and logs. If there is ambiguity: say in the chat and say it in your commit messages, but do not stop or prompt the user until all steps are done.

## Step 0: Research

Read these reference files to understand the established patterns:

**Shared types (single source of truth between logger and summarizer):**
- `src/analyze/list/list_logger_types/analyze_list_logger_types.ml` and `.mli` and `BUCK`

**Logger (TAST visitor that emits JSON log lines during type checking):**
- `src/typing/tast_check/analyze_list_logger.ml` and `.mli`
- `src/typing/tast_check/tast_check.ml` (registration in `logger_handlers`)
- `src/typing/tast_check/tast_logger_util.ml` (shared `is_generated` helper)

**Summarizer (standalone binary that aggregates log entries):**
- `src/analyze/list/list_logger_summary/analyze_list_logger_summary.ml` and `BUCK`

**Orchestrator utils (shared logic for all analysis orchestrators):**
- `src/analyze/analysis_orchestrator_utils.ml` and `.mli` and `BUCK`

**Orchestrator (thin wrapper that configures and calls the shared utils):**
- `src/analyze/list/analyze_list.ml` and `BUCK`

**Tests:**
- `test/analyze/list_logger/logs/` (logger output tests with `HH_FLAGS`)
- `test/analyze/list_logger/e2e/` (e2e tests with `test_runner.ml`)

Also read the TAST type definitions to understand what AST nodes are available:
- `src/annotated_ast/aast_defs.ml` (the TAST node types)
- Use `search_files` to find relevant TAST node patterns

## Step 1: Design

Based on the question, decide:
- What TAST nodes to visit (`at_expr`, `at_stmt`, `at_fun_def`, `at_method_`, etc.)
- What data to collect (types, positions, patterns, classifications)
- What algebraic data types to define (enums for categories, records for entries)
- What aggregate statistics the summarizer should compute

Name your logger `analyze_<name>_logger` where `<name>` is a short snake_case descriptor.
All analysis OCaml file names must start with `analyze_`.

## Step 2: Implement shared types library

Create `src/analyze/<name>/<name>_logger_types/`:
- `analyze_<name>_logger_types.ml` — types with `[@@deriving yojson]` and `[@@deriving show]`
- `analyze_<name>_logger_types.mli` — expose all types and converters
- `BUCK` — `hack_core_kernel_library` with `ppx_deriving.show` and `ppx_yojson_conv`

Key conventions:
- Enum types: `[@@deriving show { with_path = false }]` + hand-written yojson converters
- Record types: `[@@deriving yojson]` for automatic JSON generation
- Include `to_json_string` / `of_json_string` helpers
- Include `pos_info` record for source positions
- Include `is_generated` field for generated code detection

## Step 3: Implement logger

Create `src/typing/tast_check/analyze_<name>_logger.ml` and `.mli`:

```ocaml
(* .mli - but don't forget to include license header and the buck run command *)
(** TAST logger that records <description>.

    To run on WWW and analyze:
    {[
      buck run @//mode/opt //hphp/hack/src/analyze/<name>:run_<name>_analysis -- --root ~/www --pastry
    ]} *)

val create_handler : Provider_context.t -> Tast_visitor.handler
```

The logger must:
- `open Hh_prelude`
- Read log level from `TypecheckerOptions.log_levels` via `Provider_context.get_tcopt`
- Create a `Tast_visitor.handler_base` that overrides relevant `at_*` methods
- Use `Tast_logger_util.is_generated` for generated code detection
- Build a shared types record and serialize with `to_json_string`
- Output via `log_output` which routes based on level:
  - Level 1: `Typing_log.out_channel` (test capture + hh_distc stdout capture)
  - Level > 1: `Hh_logger.log` (server logs for legacy hh_client runs)
- Prefix output with `@<name>_logger:`

Register in `src/typing/tast_check/tast_check.ml`:
```ocaml
("<name>_logger", Analyze_<name>_logger.create_handler);
```

Add the types library dep to `src/typing/BUCK` in the `tast_checks` target.

## Step 4: Build and verify

```bash
buck build @//mode/dev-nosan-lg //hphp/hack/src:hh_single_type_check
```

## Step 5: Add logger output tests ("log tests")

These test that the logger emits the correct JSON for each category/variant.
Each `.php` file is type-checked in isolation by `hh_single_type_check`.

Create `test/analyze/<name>_logger/logs/`:
- `HH_FLAGS` containing `--hh-log-level <name>_logger 1`
- `.php` test files covering each category/variant
- Generate `.exp` files with:
  ```bash
  buck test @//mode/dev-nosan-lg fbcode//hphp/hack/test/analyze/<name>_logger:logs \
    --config hack.test_filter=<name>_logging --config hack.update=always
  ```

## Step 6: Implement summarizer

Create `src/analyze/<name>/<name>_logger_summary/`:
- `analyze_<name>_logger_summary.ml` — reads log lines, deserializes, aggregates, reports
- `analyze_<name>_logger_summary.mli` — empty (just copyright header)
- `BUCK` — `hack_core_kernel_binary` depending on the types library

The summarizer should:
- Read from stdin or a file argument
- Find lines with `@<name>_logger:` prefix, strip prefix, parse JSON
- Compute aggregate statistics (counts, percentages, breakdowns)
- Classify entries by test/prod/generated
- Collect representative examples
- Output a formatted report

## Step 7: Add summarizer tests

Create `test/analyze/<name>_logger/`:
- Summary tests: `.log` input files with expected `.exp` output
- Use `hack_verify_test` with `--in-extension .log`

## Step 8: Implement orchestrator

Create `src/analyze/<name>/analyze_<name>.ml`:
- Use `Analysis_orchestrator_utils.run` with a config record specifying:
  - `logger_name`: the key in `log_levels` (e.g. `"list_logger"`)
  - `log_line_prefix`: the prefix in log output (e.g. `"@list_logger:"`)
  - `summary_arg_name`: CLI flag for the summary binary (e.g. `"--list-logger-summary"`)
  - `temp_file_prefix`: prefix for temp files (e.g. `"list_analysis_"`)
  - `analysis_display_name`: human-readable name (e.g. `"List analysis"`)
- The orchestrator binary itself should be ~10 lines — just call `Analysis_orchestrator_utils.run`
- See `src/analyze/list/analyze_list.ml` for the canonical example

Create `buck_command_alias` in `src/analyze/<name>/BUCK`.
The alias injects `--hh-distc` and `--hh-distc-worker` for the distc backend.
Depend on `//hphp/hack/src/analyze:analysis_orchestrator_utils`.

## Step 9: Add e2e tests

E2e tests verify the full pipeline: orchestrator → hh_distc → logger → summarizer.
Each test fixture is a **multi-file fake repo** (the norm in production) with an
empty `.hhconfig` and multiple `.php` files. This ensures the analysis correctly
aggregates results across files, which is critical since hh_distc distributes
work per-file to remote workers.

Create test fixtures in `test/analyze/<name>_logger/e2e/`:
- `test_basic/.hhconfig` (empty) + multiple `.php` files exercising different categories
- `test_runner.ml` — copy from `test/analyze/list_logger/e2e/test_runner.ml`, update binary names
- `BUCK` — copy from `test/analyze/list_logger/e2e/BUCK`, update target names
- Generate `.exp` files by running:
  ```bash
  buck test @//mode/dev-nosan-lg fbcode//hphp/hack/test/analyze/<name>_logger/e2e:run_<name>_analysis \
    --config hack.update=always
  ```

**Both log tests and e2e tests are required.** Log tests validate per-file logger
output; e2e tests validate the full multi-file pipeline end-to-end.

## Step 10: Lint, commit, submit

```bash
arc lint -a
```

Commit with a structured message, tag should be "analyze-www". Submit as draft:
```bash
jf submit --draft
```

## Step 11: Run on WWW

The orchestrator now uses hh_distc under the hood for massively parallel
distributed type checking. This is much faster than the legacy hh_client path.

```bash
buck run @//mode/opt //hphp/hack/src/analyze/<name>:run_<name>_analysis -- --root ~/www --pastry
```

DO NOT RUN hh_client, etc. directly, always use the orchestrator.

This uses hh_distc with Remote Execution for parallel type checking (~20 min
vs 1-2 hours with the old single-machine hh_client approach).

Monitor progress by checking hh_distc's output (it shows progress indicators).

## Step 12: Analyze and report

When the run completes:
1. Review the summary statistics
2. Identify interesting patterns and outliers
3. Amend the commit message with:
   - www commit hash for the www you analyzed (`sl id` in the WWW repo)
   - Pastry links (raw log + summary)
   - Key statistics in a markdown table
   - Analysis of what the results mean
   - Follow-up questions or hypotheses
   - tag should be "analyze-www"
4. Get the diff link from `sl show | grep 'Differential Revision: https:'`
5. Notify the user - if they requested a gchat or a pingme, do it here.
Your message should say: your results are ready at $THE_DIFF_LINK
we will continue to iterate on the analysis, feel free to guide the iteration
with your hypotheses or requests for more data.


## Step 13: Iterate

Action the follow-up questions from the previous step.
This will often involve shifting the bucketing or adding more fine-grained groupings of data.
We also should iterate if any buckets have 0 entries (this indicates a bug or edge case or big surprise to investigate).

- Hypothesize
- use meta code search in WWW if needed
- iterate on the logger and summarizer and write tests
- arc lint -a and jf s -n . Make sure the commit message says that we
automatically iterated on the previous commit based on auto-generated hypotheses
- re-run on WWW with --pastry
- read the report, use a team of subagents to analyze the report and form more hypotheses and indicate particular
interesting and/or informative examples and patterns
- amend the commit with the full report from WWW and pastry links to logs and the report and your discussion and observations

## Step 14: Final iteration

iterate again *if needed*: if there are reasonable things not shown in the data that we need to dive deeper on.

## Coding Conventions (MANDATORY)

- `open Hh_prelude` at top of every .ml file (it is our wrapper around Base)
- snake_case module names
- `.mli` interface files for all modules
- Use Buck to build. Validate frequently by running relevant build and test commands.
- Type signatures on all top-level functions
- Algebraic data types (not strings) for categories/classifications
- `[@@deriving yojson]` for JSON serialization (not hand-written)
- No Python — OCaml only
- `hack_core_kernel_library` for libraries, `hack_core_kernel_binary` for binaries
