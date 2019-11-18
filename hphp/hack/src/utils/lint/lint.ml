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
      message = spf "Please use '%s' instead of '%s'" lower cst;
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
    (spf "Use `%s {...}` instead of `new %s(...)`" coll coll)

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
    ( "The field '"
    ^ name
    ^ "' is required to exist in the shape. Consider using a subscript-expression instead, such as $myshape['"
    ^ name
    ^ "']" )
