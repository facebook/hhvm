(**
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
  -> ?disallow_elvis_space:bool
  -> ?fail_open:bool
  -> ?parser_options:ParserOptions.t
  -> ?fi_mode:FileInfo.mode
  -> ?is_hh_file:bool
  -> ?stats:Stats_container.t
  -> ?hacksperimental:bool
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
  } [@@deriving show]

module WithPositionedSyntax : functor (Syntax : Positioned_syntax_sig.PositionedSyntax_S) -> sig

  val lower
    :  env
    -> source_text:Full_fidelity_source_text.t
    -> script:Syntax.t
    -> result

end (* WithPositionedSyntax *)

val parse_text
  :  env
  -> Full_fidelity_source_text.t
  -> (FileInfo.file_type *
      FileInfo.mode option *
      PositionedSyntaxTree.t)
val lower_tree
  :  env
  -> Full_fidelity_source_text.t
  -> FileInfo.file_type
  -> FileInfo.mode option
  -> PositionedSyntaxTree.t
  -> result
val from_text : env -> Full_fidelity_source_text.t -> result
val from_file : env -> result

(**
 * Here only for backward compatibility. Consider these deprecated.
 *)
val from_text_with_legacy : env -> string -> Parser_hack.parser_return
val from_file_with_legacy : env -> Parser_hack.parser_return
val legacy_compliant_parse_defensively :
  Relative_path.t -> bool -> GlobalOptions.t -> string -> Parser_hack.parser_return
