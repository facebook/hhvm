(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* TODO(jakebailey): Can/should this be done with the direct decl parser? *)
val lambda_decl_in_env : Decl_env.env -> Nast.fun_ -> Typing_defs.fun_elt

(** DEPRECATED: When TypecheckerOptions.use_direct_decl_parser is enabled, parse
    the source text using the direct decl parser (or ast_and_decl_parser, if the
    AST is also needed) instead of transforming the AST to a decl. *)
val fun_naming_and_decl_DEPRECATED :
  Provider_context.t -> Nast.fun_def -> string * Typing_defs.fun_elt

(** DEPRECATED: When TypecheckerOptions.use_direct_decl_parser is enabled, parse
    the source text using the direct decl parser (or ast_and_decl_parser, if the
    AST is also needed) instead of transforming the AST to a decl. *)
val record_def_naming_and_decl_DEPRECATED :
  Provider_context.t ->
  Nast.record_def ->
  Ast_defs.id option ->
  string * Typing_defs.record_def_type

(** DEPRECATED: When TypecheckerOptions.use_direct_decl_parser is enabled, parse
    the source text using the direct decl parser (or ast_and_decl_parser, if the
    AST is also needed) instead of transforming the AST to a decl. *)
val typedef_naming_and_decl_DEPRECATED :
  Provider_context.t -> Nast.typedef -> string * Typing_defs.typedef_type

(** DEPRECATED: When TypecheckerOptions.use_direct_decl_parser is enabled, parse
    the source text using the direct decl parser (or ast_and_decl_parser, if the
    AST is also needed) instead of transforming the AST to a decl. *)
val const_naming_and_decl_DEPRECATED :
  Provider_context.t -> Nast.gconst -> string * Typing_defs.const_decl
