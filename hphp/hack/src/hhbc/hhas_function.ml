(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

type t = {
  function_attributes    : Hhas_attribute.t list;
  function_name          : Litstr.id;
  function_params        : Hhas_param.t list;
  function_return_type   : Hhas_type_info.t option;
  function_body          : Hhbc_ast.instruct list;
  function_decl_vars     : string list; (* Actually local_id list *)
  function_num_iters     : int;
  function_num_cls_ref_slots : int;
  function_is_async      : bool;
  function_is_generator  : bool;
  function_is_pair_generator : bool;
}

let make
  function_attributes
  function_name
  function_params
  function_return_type
  function_body
  function_decl_vars
  function_num_iters
  function_num_cls_ref_slots
  function_is_async
  function_is_generator
  function_is_pair_generator =
  {
    function_attributes;
    function_name;
    function_params;
    function_return_type;
    function_body;
    function_decl_vars;
    function_num_iters;
    function_num_cls_ref_slots;
    function_is_async;
    function_is_generator;
    function_is_pair_generator;
  }

let attributes f = f.function_attributes
let name f = f.function_name
let params f = f.function_params
let return_type f = f.function_return_type
let body f = f.function_body
let decl_vars f = f.function_decl_vars
let num_iters f = f.function_num_iters
let num_cls_ref_slots f = f.function_num_cls_ref_slots
let is_async f = f.function_is_async
let is_generator f = f.function_is_generator
let is_pair_generator f = f.function_is_pair_generator
let with_name f function_name = { f with function_name }
let with_body f function_body = { f with function_body }
