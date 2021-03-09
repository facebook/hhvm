(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Abstracts over the particular shared memory implementation used for decl
   information.
 *)

type class_members = {
  m_properties: Typing_defs.decl_ty SMap.t;
  m_static_properties: Typing_defs.decl_ty SMap.t;
  m_methods: Typing_defs.fun_elt SMap.t;
  m_static_methods: Typing_defs.fun_elt SMap.t;
  m_constructor: Typing_defs.fun_elt option;
}

type class_entries = Decl_defs.decl_class_type * class_members option

module ClassEltKey : SharedMem.UserKeyType with type t = string * string

type decl_store = {
  add_class: string -> Decl_defs.decl_class_type -> unit;
  get_class: string -> Decl_defs.decl_class_type option;
  add_prop: ClassEltKey.t -> Typing_defs.decl_ty -> unit;
  get_prop: ClassEltKey.t -> Typing_defs.decl_ty option;
  add_static_prop: ClassEltKey.t -> Typing_defs.decl_ty -> unit;
  get_static_prop: ClassEltKey.t -> Typing_defs.decl_ty option;
  add_method: ClassEltKey.t -> Typing_defs.fun_elt -> unit;
  get_method: ClassEltKey.t -> Typing_defs.fun_elt option;
  add_static_method: ClassEltKey.t -> Typing_defs.fun_elt -> unit;
  get_static_method: ClassEltKey.t -> Typing_defs.fun_elt option;
  add_constructor: string -> Typing_defs.fun_elt -> unit;
  get_constructor: string -> Typing_defs.fun_elt option;
  add_fun: string -> Typing_defs.fun_elt -> unit;
  get_fun: string -> Typing_defs.fun_elt option;
  add_typedef: string -> Typing_defs.typedef_type -> unit;
  get_typedef: string -> Typing_defs.typedef_type option;
  add_recorddef: string -> Typing_defs.record_def_type -> unit;
  get_recorddef: string -> Typing_defs.record_def_type option;
  add_gconst: string -> Typing_defs.const_decl -> unit;
  get_gconst: string -> Typing_defs.const_decl option;
  push_local_changes: unit -> unit;
  pop_local_changes: unit -> unit;
}

val get : unit -> decl_store

val set : decl_store -> unit
