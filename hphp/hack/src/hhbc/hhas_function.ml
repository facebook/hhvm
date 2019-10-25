(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  function_attributes: Hhas_attribute.t list;
  function_name: Hhbc_id.Function.t;
  function_body: Hhas_body.t;
  function_span: Hhas_pos.span;
  function_is_async: bool;
  function_is_generator: bool;
  function_is_pair_generator: bool;
  function_hoisted: Closure_convert.hoist_kind;
  function_no_injection: bool;
  function_is_interceptable: bool;
  function_is_memoize_impl: bool;
  function_rx_level: Rx.t;
  function_rx_disabled: bool;
}

let make
    function_attributes
    function_name
    function_body
    function_span
    function_is_async
    function_is_generator
    function_is_pair_generator
    function_hoisted
    function_no_injection
    function_is_interceptable
    function_is_memoize_impl
    function_rx_level
    function_rx_disabled =
  {
    function_attributes;
    function_name;
    function_body;
    function_span;
    function_is_async;
    function_is_generator;
    function_is_pair_generator;
    function_hoisted;
    function_no_injection;
    function_is_interceptable;
    function_is_memoize_impl;
    function_rx_level;
    function_rx_disabled;
  }

let attributes f = f.function_attributes

let name f = f.function_name

let body f = f.function_body

let span f = f.function_span

let is_async f = f.function_is_async

let is_generator f = f.function_is_generator

let is_pair_generator f = f.function_is_pair_generator

let is_top f =
  match f.function_hoisted with
  | Closure_convert.TopLevel -> true
  | Closure_convert.Hoisted -> false

let no_injection f = f.function_no_injection

let is_interceptable f = f.function_is_interceptable

let is_memoize_impl f = f.function_is_memoize_impl

let rx_level f = f.function_rx_level

let rx_disabled f = f.function_rx_disabled

let with_body f function_body = { f with function_body }

let params f = f.function_body.Hhas_body.body_params

let return_type f = f.function_body.Hhas_body.body_return_type
