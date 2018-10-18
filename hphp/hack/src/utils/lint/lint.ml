(**
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

let internal_error pos msg =
  add 0 Lint_error pos ("Internal error: "^msg)

let lowercase_constant pos cst =
  let lower = String.lowercase_ascii cst in
  add Codes.lowercase_constant Lint_warning pos
    (spf "Please use '%s' instead of '%s'" lower cst)

let use_collection_literal pos coll =
  let coll = Utils.strip_ns coll in
  add Codes.use_collection_literal Lint_warning pos
    (spf "Use `%s {...}` instead of `new %s(...)`" coll coll)

let static_string ?(no_consts=false) pos =
  add Codes.static_string Lint_warning pos begin
    if no_consts
    then
      "This should be a string literal so that lint can analyze it."
    else
      "This should be a string literal or string constant so that lint can "^
      "analyze it."
  end

let shape_idx_access_required_field field_pos name =
  add Codes.shape_idx_required_field Lint_warning field_pos
    ("The field '"^name^"' is required to exist in the shape. Consider using a \
    subscript-expression instead, such as $myshape['"^name^"']")
