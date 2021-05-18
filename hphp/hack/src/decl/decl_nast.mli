(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val fun_decl_in_env :
  Decl_env.env -> is_lambda:bool -> Nast.fun_ -> Typing_defs.fun_elt

val fun_naming_and_decl :
  Provider_context.t -> Nast.fun_def -> string * Typing_defs.fun_elt

val record_def_naming_and_decl :
  Provider_context.t -> Nast.record_def -> string * Typing_defs.record_def_type

val typedef_naming_and_decl :
  Provider_context.t -> Nast.typedef -> string * Typing_defs.typedef_type

val const_naming_and_decl :
  Provider_context.t -> Nast.gconst -> string * Typing_defs.const_decl
