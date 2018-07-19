(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
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
  ns_popt: ParserOptions.t;
  ns_auto_namespace_map: (string * string) list;
}

let empty popt = {
  ns_ns_uses = SMap.empty;
  ns_class_uses = SMap.empty;
  ns_fun_uses = SMap.empty;
  ns_const_uses = SMap.empty;
  ns_name = None;
  ns_popt = popt;
  ns_auto_namespace_map = ParserOptions.auto_namespace_map popt;
}

let empty_with_default_popt =
  empty ParserOptions.default

let is_global_namespace env =
  Option.is_none env.ns_name

let with_auto_namespace_map env ns_auto_namespace_map =
  { env with ns_auto_namespace_map }
