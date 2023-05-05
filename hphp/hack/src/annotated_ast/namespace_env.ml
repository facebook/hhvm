(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type env = {
  ns_ns_uses: string SMap.t; [@opaque]
  ns_class_uses: string SMap.t; [@opaque]
  ns_fun_uses: string SMap.t; [@opaque]
  ns_const_uses: string SMap.t; [@opaque]
  ns_name: string option;
  ns_auto_ns_map: (string * string) list;
  ns_is_codegen: bool;
  ns_disable_xhp_element_mangling: bool;
}
[@@deriving eq, hash, show, ord]

let hh_autoimport_map_of_list ids =
  List.map ids ~f:(fun id -> (id, "HH\\" ^ id)) |> SMap.of_list

let default_class_uses = hh_autoimport_map_of_list Hh_autoimport.types

let default_fun_uses = hh_autoimport_map_of_list Hh_autoimport.funcs

let default_const_uses = hh_autoimport_map_of_list Hh_autoimport.consts

let default_ns_uses = hh_autoimport_map_of_list Hh_autoimport.namespaces

let empty auto_ns_map is_codegen disable_xhp_element_mangling =
  {
    ns_ns_uses = SMap.union (SMap.of_list auto_ns_map) default_ns_uses;
    ns_class_uses = default_class_uses;
    ns_fun_uses = default_fun_uses;
    ns_const_uses = default_const_uses;
    ns_name = None;
    ns_auto_ns_map = auto_ns_map;
    ns_is_codegen = is_codegen;
    ns_disable_xhp_element_mangling = disable_xhp_element_mangling;
  }

let empty_with_default =
  let popt = ParserOptions.default in
  let auto_ns_map = ParserOptions.auto_namespace_map popt in
  let codegen = ParserOptions.codegen popt in
  let disable_xhp_element_mangling =
    ParserOptions.disable_xhp_element_mangling popt
  in
  empty auto_ns_map codegen disable_xhp_element_mangling

let empty_from_env env =
  empty env.ns_auto_ns_map env.ns_is_codegen env.ns_disable_xhp_element_mangling

let empty_from_popt popt =
  empty
    (ParserOptions.auto_namespace_map popt)
    (ParserOptions.codegen popt)
    (ParserOptions.disable_xhp_element_mangling popt)

let is_global_namespace env = Option.is_none env.ns_name
