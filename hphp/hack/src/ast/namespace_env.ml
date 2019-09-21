(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

type env = {
  ns_ns_uses: string SMap.t;
  ns_class_uses: string SMap.t;
  ns_record_def_uses: string SMap.t;
  ns_fun_uses: string SMap.t;
  ns_const_uses: string SMap.t;
  ns_name: string option;
  ns_auto_ns_map: (string * string) list;
  ns_is_codegen: bool;
}
[@@deriving show]

let empty auto_ns_map is_code_gen =
  {
    ns_ns_uses = SMap.empty;
    ns_class_uses = SMap.empty;
    ns_record_def_uses = SMap.empty;
    ns_fun_uses = SMap.empty;
    ns_const_uses = SMap.empty;
    ns_name = None;
    ns_auto_ns_map = auto_ns_map;
    ns_is_codegen = is_code_gen;
  }

let empty_with_default =
  let popt = ParserOptions.default in
  let auto_ns_map = ParserOptions.auto_namespace_map popt in
  let codegen = ParserOptions.codegen popt in
  empty auto_ns_map codegen

let empty_from_env env = empty env.ns_auto_ns_map env.ns_is_codegen

let empty_from_popt popt =
  empty (ParserOptions.auto_namespace_map popt) (ParserOptions.codegen popt)

let is_global_namespace env = Option.is_none env.ns_name
