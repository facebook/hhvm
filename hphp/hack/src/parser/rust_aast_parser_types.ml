(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type env = {
  codegen: bool;
  php5_compat_mode: bool;
  elaborate_namespaces: bool;
  include_line_comments: bool;
  keep_errors: bool;
  quick_mode: bool;
  (* Show errors even in quick mode. Does not override keep_errors. Hotfix
   * until we can properly set up saved states to surface parse errors during
   * typechecking properly. *)
  show_all_errors: bool;
  fail_open: bool;
  parser_options: ParserOptions.t;
}

type 'aast result_ = {
  file_mode: FileInfo.mode;
  scoured_comments: Scoured_comments.t;
  aast: ('aast, string) Stdlib.result;
  lowpri_errors: (Pos.t * string) list;
  syntax_errors: Full_fidelity_syntax_error.t list;
  errors: Errors.error list;
  lint_errors: Pos.t Lint.t list;
}

type result = (Pos.t, unit, unit, unit) Aast.program result_

type tast_result = Tast.program result_

type error =
  | NotAHackFile
  | ParserFatal of Full_fidelity_syntax_error.t * Pos.t
  | Other of string
