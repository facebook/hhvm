(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type env = {
  ns_ns_uses: string SMap.t;
  ns_class_uses: string SMap.t;
  ns_fun_uses: string SMap.t;
  ns_const_uses: string SMap.t;
  ns_name: string option;
  ns_auto_ns_map: (string * string) list;
  ns_is_codegen: bool;
  ns_disable_xhp_element_mangling: bool;
}
[@@deriving eq, hash, show, ord]

val empty_with_default : env

val empty_from_env : env -> env

val empty_from_popt : ParserOptions.t -> env

val is_global_namespace : env -> bool
