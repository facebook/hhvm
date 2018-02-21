(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(**
 * Explicitly exporting the definition of `Syntax` used in the lowerer. This
 * should
 *)
module Syntax = Full_fidelity_editable_positioned_syntax

(**
 * The `env` of the lowerer is "full request." It provides all the settings the
 * lowerer needs to produce an AST.
 *)
type env

val make_env
  (* Optional parts *)
  :  ?codegen:bool
  -> ?systemlib_compat_mode:bool
  -> ?php5_compat_mode:bool
  -> ?elaborate_namespaces:bool
  -> ?include_line_comments:bool
  -> ?keep_errors:bool
  -> ?ignore_pos:bool
  -> ?quick_mode:bool
  -> ?lower_coroutines:bool
  -> ?enable_hh_syntax:bool
  -> ?parser_options:GlobalOptions.t
  -> ?fi_mode:FileInfo.mode
  -> ?is_hh_file:bool
  -> ?stats:Stats_container.t
  (* Required parts *)
  -> Relative_path.t
  -> env

(**
 * A `result` contains some information from the original `env`; the information
 * that is typically required later. This is still quite ad-hoc and should be
 * redesigned properly at some point.
 *)
type result =
  { fi_mode    : FileInfo.mode
  ; is_hh_file : bool
  ; ast        : Ast.program
  ; content    : string
  ; file       : Relative_path.t
  ; comments   : (Pos.t * Prim_defs.comment) list
  }
val lower
  :  env
  -> source_text:Syntax.SourceText.t
  -> script:Syntax.t
  -> result
val from_text : env -> Syntax.SourceText.t -> result
val from_file : env -> result

(**
 * Here only for backward compatibility. Consider these deprecated.
 *)
val from_text_with_legacy : env -> string -> Parser_hack.parser_return
val from_file_with_legacy : env -> Parser_hack.parser_return
