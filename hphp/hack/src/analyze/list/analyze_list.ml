(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let () =
  Analysis_orchestrator_utils.run
    {
      logger_name = "list_logger";
      log_line_prefix = "@list_logger:";
      summary_arg_name = "--list-logger-summary";
      temp_file_prefix = "list_analysis_";
      analysis_display_name = "List analysis";
    }
