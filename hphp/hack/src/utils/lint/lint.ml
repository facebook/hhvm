(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include Lint_core
module Codes = Lint_codes

let spf = Printf.sprintf

let internal_error pos msg = add 0 Lint_error pos ("Internal error: " ^ msg)

let add_lint lint =
  add
    ~bypass_changed_lines:lint.bypass_changed_lines
    ~autofix:lint.autofix
    lint.code
    lint.severity
    lint.pos
    lint.message

let mk_lowercase_constant pos cst =
  let lower = String.lowercase_ascii cst in
  Lint_core.
    {
      code = Codes.to_enum Codes.LowercaseConstant;
      severity = Lint_warning;
      pos;
      message =
        spf
          "Please use %s instead of %s"
          (Markdown_lite.md_codify lower)
          (Markdown_lite.md_codify cst);
      bypass_changed_lines = false;
      autofix = ("", "");
    }

let lowercase_constant pos cst = add_lint (mk_lowercase_constant pos cst)

let use_collection_literal pos coll =
  let coll = Utils.strip_ns coll in
  add
    (Codes.to_enum Codes.UseCollectionLiteral)
    Lint_warning
    pos
    ( "Use "
    ^ Markdown_lite.md_codify (coll ^ " {...}")
    ^ " instead of "
    ^ Markdown_lite.md_codify ("new " ^ coll ^ "(...)") )

let static_string ?(no_consts = false) pos =
  add
    (Codes.to_enum Codes.StaticString)
    Lint_warning
    pos
    begin
      if no_consts then
        "This should be a string literal so that lint can analyze it."
      else
        "This should be a string literal or string constant so that lint can "
        ^ "analyze it."
    end

let shape_idx_access_required_field field_pos name =
  add
    (Codes.to_enum Codes.ShapeIdxRequiredField)
    Lint_warning
    field_pos
    ( "The field "
    ^ Markdown_lite.md_codify name
    ^ " is required to exist in the shape. Consider using a subscript-expression instead, such as "
    ^ Markdown_lite.md_codify ("$myshape['" ^ name ^ "']") )

let opt_closed_shape_idx_missing_field method_name field_pos =
  let msg =
    match method_name with
    | Some method_name ->
      "You are calling "
      ^ Markdown_lite.md_codify ("Shapes::" ^ method_name ^ "()")
      ^ " on a field known to not exist, with a closed optional shape."
    | None ->
      "You are indexing a closed optional shape with"
      ^ " a field known to not exist."
  in
  add
    (Codes.to_enum Codes.OptClosedShapeIdxMissingField)
    Lint_error
    field_pos
    msg
