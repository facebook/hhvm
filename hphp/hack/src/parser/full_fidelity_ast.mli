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
  ?quick_mode:bool ->
  ?show_all_errors:bool ->
  ?lower_coroutines:bool ->
  ?fail_open:bool ->
  ?parser_options:ParserOptions.t (* Required parts *) ->
  ?disable_global_state_mutation:bool ->
  Relative_path.t ->
  env

val from_source_text_with_legacy :
  env -> Full_fidelity_source_text.t -> Parser_return.t

val from_text_with_legacy : env -> string -> Parser_return.t

val parse_text :
  env ->
  Full_fidelity_source_text.t ->
  FileInfo.mode option * PositionedSyntaxTree.t

(* Only for hh_single_compile at the moment. *)
val from_text_to_empty_tast :
  env -> Full_fidelity_source_text.t -> Rust_aast_parser_types.tast_result

(**
 * Here only for backward compatibility. Consider these deprecated.
 *)

val from_file_with_legacy : env -> Parser_return.t

val defensive_program :
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
  ?quick:bool ->
  ?show_all_errors:bool ->
  ?fail_open:bool ->
  ?elaborate_namespaces:bool ->
  Relative_path.t ->
  string ->
  Parser_return.t

(*
  from_text_rust are only used for testing
*)
val from_text_rust :
  env -> Full_fidelity_source_text.t -> Rust_aast_parser_types.result

val aast_to_tast : (Pos.t, unit, unit, unit) Aast.program -> Tast.program

val tast_to_aast : Tast.program -> (Pos.t, unit, unit, unit) Aast.program
