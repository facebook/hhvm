(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
let find ~entry pos ctx =
  let variable_actions =
    match Inline_variable.find ~entry pos ctx with
    | [] -> Extract_variable.find ~entry pos ctx
    | actions -> actions
  in
  Override_method.find ~entry pos ctx
  @ variable_actions
  @ Inline_method.find ~entry pos ctx
  @ Extract_method.find ~entry pos ctx
  @ Extract_classish.find ~entry pos ctx
  @ Extract_shape_type.find ~entry pos ctx
  @ Flip_around_comma.find ~entry pos ctx
  @ Add_local_type_hint.find ~entry pos ctx
  @ Add_doc_comment.find ~entry pos ctx
