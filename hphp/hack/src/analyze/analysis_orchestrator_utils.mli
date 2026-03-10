(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Shared utilities for TAST logger analysis orchestrators.

    Each analysis (e.g. list, shape_merge) has its own orchestrator binary
    that configures an [analysis_config] and calls [run]. The orchestrator
    handles type checking with the logger enabled via hh_distc (distributed
    type checking), extracting log lines, invoking the summarizer, and
    optionally uploading results to Pastry.

    TAST logger output at level 1 goes to worker stdout, which hh_distc
    captures from remote workers and prints. The orchestrator redirects
    hh_distc stdout to a temp file and extracts logger lines from it. *)

(** Configuration that distinguishes one analysis from another. *)
type analysis_config = {
  logger_name: string;
      (** Key in log_levels, e.g. "list_logger" or "shape_merge" *)
  log_line_prefix: string;
      (** Prefix used in log output, e.g. "@list_logger:" or "@shape_merge:" *)
  summary_arg_name: string;
      (** CLI flag for the summary binary path, e.g. "--list-logger-summary" *)
  temp_file_prefix: string;
      (** Prefix for temp files, e.g. "list_analysis_" or "shape_merge_analysis_" *)
  analysis_display_name: string;
      (** Human-readable name for messages, e.g. "List analysis" or "Shape merge analysis" *)
}

(** Run the full orchestration: type-check with logger via hh_distc,
    extract log lines, summarize, optionally upload to Pastry. *)
val run : analysis_config -> unit
