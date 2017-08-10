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
  function_name          : Hhbc_id.Function.t;
  function_body          : Hhas_body.t;
  function_span          : Hhas_pos.span;
  function_is_async      : bool;
  function_is_generator      : bool;
  function_is_pair_generator : bool;
  function_is_top            : bool;
}

let make
  function_attributes
  function_name
  function_body
  function_span
  function_is_async
  function_is_generator
  function_is_pair_generator
  function_is_top =
  {
    function_attributes;
    function_name;
    function_body;
    function_span;
    function_is_async;
    function_is_generator;
    function_is_pair_generator;
    function_is_top;
  }

let attributes f = f.function_attributes
let name f = f.function_name
let body f = f.function_body
let span f = f.function_span
let is_async f = f.function_is_async
let is_generator f = f.function_is_generator
let is_pair_generator f = f.function_is_pair_generator
let is_top f = f.function_is_top
let with_name f function_name = { f with function_name }
let with_body f function_body = { f with function_body }
let with_is_async f function_is_async = { f with function_is_async }
let params f = f.function_body.Hhas_body.body_params
let return_type f = f.function_body.Hhas_body.body_return_type
