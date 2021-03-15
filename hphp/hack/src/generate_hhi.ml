(*
 * Copyright (c) 2016-present , Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * Usage: hh_parse --generate-hhi [FILE]
 *
 * Generates a .hhi file from a .hack or .php file; intended for use in the
 * build system for a typechecked systemlib.
 *)

open Hh_prelude
module Syntax = Full_fidelity_editable_syntax
module SourceText = Full_fidelity_source_text
module Rewriter = Full_fidelity_rewriter.WithSyntax (Syntax)
module Token = Full_fidelity_editable_token
module TokenKind = Full_fidelity_token_kind

let go editable =
  let without_bodies =
    Rewriter.rewrite_pre_and_stop
      (fun inner ->
        match Syntax.syntax inner with
        | Syntax.MarkupSuffix _ ->
          (* remove `<?hh` line if present *)
          Rewriter.Replace (Syntax.make_missing SourceText.empty 0)
        | Syntax.FunctionDeclaration f ->
          (* remove function bodies *)
          Rewriter.Replace
            (Syntax.make_function_declaration
               f.function_attribute_spec
               f.function_declaration_header
               (* replace body *)
               (Syntax.make_token (Token.create TokenKind.Semicolon ";" [] [])))
        | Syntax.MethodishDeclaration m ->
          (* remove method bodies *)
          Rewriter.Replace
            (Syntax.make_methodish_declaration
               m.methodish_attribute
               m.methodish_function_decl_header
               (* no body *)
               (Syntax.make_missing SourceText.empty 0)
               (* but always a semicolon, even if not abstract *)
               (Syntax.make_token (Token.create TokenKind.Semicolon ";" [] [])))
        | _ -> Rewriter.Keep)
      editable
  in
  let text = Syntax.text without_bodies in
  "<?hh\n// @" ^ "generated from implementation\n\n" ^ text
