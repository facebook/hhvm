(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include module type of struct
  include Full_fidelity_ast_types
end

type lifted_awaits [@@deriving show]

(**
 * The `env` of the lowerer is "full request." It provides all the settings the
 * lowerer needs to produce an AST.
 *)
type env [@@deriving show]

val make_env (* Optional parts *) :
  ?codegen:bool ->
  ?php5_compat_mode:bool ->
  ?elaborate_namespaces:bool ->
  ?include_line_comments:bool ->
  ?keep_errors:bool ->
  ?ignore_pos:bool ->
  ?quick_mode:bool ->
  ?show_all_errors:bool ->
  ?lower_coroutines:bool ->
  ?fail_open:bool ->
  ?parser_options:ParserOptions.t ->
  ?is_hh_file:bool ->
  ?hacksperimental:bool (* Required parts *) ->
  Relative_path.t ->
  env

type 'a result_ = {
  fi_mode: FileInfo.mode;
  is_hh_file: bool;
  ast: 'a;
  content: string;
  file: Relative_path.t;
  comments: (Pos.t * Prim_defs.comment) list;
}
[@@deriving show]

type rust_result = (Pos.t, unit, unit, unit) Aast.program result_

(**
 * A `result` contains some information from the original `env`; the information
 * that is typically required later. This is still quite ad-hoc and should be
 * redesigned properly at some point.
 *)

val from_text_rust : env -> Full_fidelity_source_text.t -> rust_result

val from_text_with_legacy : env -> string -> Parser_return.t

val from_text_with_legacy_and_cst :
  env ->
  Full_fidelity_source_text.t ->
  PositionedSyntaxTree.t * Parser_return.t

val from_text_to_custom_aast :
  env ->
  Full_fidelity_source_text.t ->
  (Ast_defs.pos -> 'ex) ->
  'fb ->
  'en ->
  'hi ->
  ('ex, 'fb, 'en, 'hi) Aast.program * bool

(* Only for hh_single_compile at the moment. *)
val from_text_to_empty_tast :
  env -> Full_fidelity_source_text.t -> Tast.program * bool

(**
 * Here only for backward compatibility. Consider these deprecated.
 *)

val from_file_with_legacy : env -> Parser_return.t

val defensive_program :
  ?hacksperimental:bool ->
  ?quick:bool ->
  ?show_all_errors:bool ->
  ?fail_open:bool ->
  ?keep_errors:bool ->
  ?elaborate_namespaces:bool ->
  ?include_line_comments:bool ->
  ParserOptions.t ->
  Relative_path.t ->
  string ->
  Parser_return.t

val defensive_from_file_with_default_popt :
  ?quick:bool -> ?show_all_errors:bool -> Relative_path.t -> Parser_return.t

val defensive_from_file :
  ?quick:bool ->
  ?show_all_errors:bool ->
  ParserOptions.t ->
  Relative_path.t ->
  Parser_return.t

val defensive_program_with_default_popt :
  ?hacksperimental:bool ->
  ?quick:bool ->
  ?show_all_errors:bool ->
  ?fail_open:bool ->
  ?elaborate_namespaces:bool ->
  Relative_path.t ->
  string ->
  Parser_return.t

val scour_comments_and_add_fixmes :
  env ->
  Full_fidelity_source_text.t ->
  Full_fidelity_positioned_syntax.t ->
  (Pos.t * Prim_defs.comment) list
