(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Decl_defs

module Dep = Typing_deps.Dep

type env = {
  mode : FileInfo.mode;
  droot : Typing_deps.Dep.variant option;
  decl_tcopt : TypecheckerOptions.t;
}

let mode env = env.mode

let add_wclass env x =
  let dep = Dep.Class x in
  Option.iter env.droot (fun root -> Typing_deps.add_idep root dep);
  ()

let add_extends_dependency env x =
  Option.iter env.droot begin fun root ->
    let dep = Dep.Class x in
    Typing_deps.add_idep root (Dep.Extends x);
    Typing_deps.add_idep root dep;
  end;
  ()

let get_class_dep env x =
  add_wclass env x;
  add_extends_dependency env x;
  Decl_heap.Classes.get x

let get_construct env class_ =
  add_wclass env class_.dc_name;
  let dep = Dep.Cstr (class_.dc_name) in
  Option.iter env.droot (fun root -> Typing_deps.add_idep root dep);
  class_.dc_construct
