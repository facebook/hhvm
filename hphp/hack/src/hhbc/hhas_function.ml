(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

type t = {
  function_attributes        : Hhas_attribute.t list;
  function_name              : Hhbc_id.Function.t;
  function_body              : Hhas_body.t;
  function_span              : Hhas_pos.span;
  function_is_async          : bool;
  function_is_generator      : bool;
  function_is_pair_generator : bool;
  function_is_top            : bool;
  function_no_injection      : bool;
  function_inout_wrapper     : bool;
  function_is_return_by_ref  : bool;
  function_is_interceptable  : bool;
  function_is_memoize_impl   : bool;
}

let make
  function_attributes
  function_name
  function_body
  function_span
  function_is_async
  function_is_generator
  function_is_pair_generator
  function_is_top
  function_no_injection
  function_inout_wrapper
  function_is_return_by_ref
  function_is_interceptable
  function_is_memoize_impl =
  {
    function_attributes;
    function_name;
    function_body;
    function_span;
    function_is_async;
    function_is_generator;
    function_is_pair_generator;
    function_is_top;
    function_no_injection;
    function_inout_wrapper;
    function_is_return_by_ref;
    function_is_interceptable;
    function_is_memoize_impl;
  }

let attributes f = f.function_attributes
let name f = f.function_name
let body f = f.function_body
let span f = f.function_span
let is_async f = f.function_is_async
let is_generator f = f.function_is_generator
let is_pair_generator f = f.function_is_pair_generator
let is_top f = f.function_is_top
let no_injection f = f.function_no_injection
let inout_wrapper f = f.function_inout_wrapper
let is_return_by_ref f = f.function_is_return_by_ref
let is_interceptable f = f.function_is_interceptable
let is_memoize_impl f = f.function_is_memoize_impl

let with_body f function_body = { f with function_body }
let params f = f.function_body.Hhas_body.body_params
let return_type f = f.function_body.Hhas_body.body_return_type
