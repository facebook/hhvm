(**
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
  ns_fun_uses: string SMap.t;
  ns_const_uses: string SMap.t;
  ns_name: string option;
  ns_popt: ParserOptions.t;
}

let empty popt = {
  ns_ns_uses = SMap.empty;
  ns_class_uses = SMap.empty;
  ns_fun_uses = SMap.empty;
  ns_const_uses = SMap.empty;
  ns_name = None;
  ns_popt = popt;
}

let empty_with_default_popt =
  empty ParserOptions.default

let is_global_namespace env =
  Option.is_none env.ns_name
