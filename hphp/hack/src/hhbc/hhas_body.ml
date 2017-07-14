(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

(* Everything needed to describe a function, method, or main body *)
type t = {
  body_instrs            : Instruction_sequence.t;
  body_decl_vars         : string list; (* Actually local_id list *)
  body_num_iters         : int;
  body_num_cls_ref_slots : int;
  body_is_memoize_wrapper: bool;
  body_params            : Hhas_param.t list;
  body_return_type       : Hhas_type_info.t option;
  body_static_inits      : string list;
  body_doc_comment       : string option;
}

let make
  instrs
  decl_vars
  num_iters
  num_cls_ref_slots
  is_memoize_wrapper
  params
  return_type
  static_inits
  doc_comment =
  {
    body_instrs = instrs;
    body_decl_vars = decl_vars;
    body_num_iters = num_iters;
    body_num_cls_ref_slots = num_cls_ref_slots;
    body_is_memoize_wrapper = is_memoize_wrapper;
    body_params = params;
    body_return_type = return_type;
    body_static_inits = static_inits;
    body_doc_comment = doc_comment;
  }

let params body = body.body_params
let instrs body = body.body_instrs
let decl_vars body = body.body_decl_vars
let num_iters body = body.body_num_iters
let num_cls_ref_slots body = body.body_num_cls_ref_slots
let return_type body = body.body_return_type
let is_memoize_wrapper body = body.body_is_memoize_wrapper
let with_instrs body instrs = { body with body_instrs = instrs }
let with_decl_vars body decl_vars = { body with body_decl_vars = decl_vars }
let static_inits body = body.body_static_inits
let doc_comment body = body.body_doc_comment
