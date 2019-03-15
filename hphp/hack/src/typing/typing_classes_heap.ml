(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs

type t = class_type

module Classes = struct
  module Cache = SharedMem.LocalCache (StringKey) (struct
    type t = class_type
    let prefix = Prefix.make()
    let description = "ClassType"
  end)

  type key = StringKey.t
  type t = class_type

  let get key =
    match Cache.get key with
    | Some c -> Some c
    | None ->
      match Decl_heap.Classes.get key with
      | Some c ->
        let class_type = Decl_class.to_class_type c in
        Cache.add key class_type;
        Some class_type
      | None ->
        None

  let find_unsafe key =
    match get key with
    | None -> raise Caml.Not_found
    | Some x -> x

  let mem key =
    match get key with
    | None -> false
    | Some _ -> true
end

let need_init cls = cls.tc_need_init
let members_fully_known cls = cls.tc_members_fully_known
let abstract cls = cls.tc_abstract
let final cls = cls.tc_final
let const cls = cls.tc_const
let deferred_init_members cls = cls.tc_deferred_init_members
let kind cls = cls.tc_kind
let is_xhp cls = cls.tc_is_xhp
let is_disposable cls = cls.tc_is_disposable
let name cls = cls.tc_name
let pos cls = cls.tc_pos
let tparams cls = cls.tc_tparams
let construct cls = cls.tc_construct
let enum_type cls = cls.tc_enum_type
let decl_errors cls = cls.tc_decl_errors

let get_ancestor cls ancestor = SMap.get ancestor cls.tc_ancestors

let has_ancestor cls ancestor =
  SMap.mem ancestor cls.tc_ancestors
let requires_ancestor cls ancestor =
  SSet.mem ancestor cls.tc_req_ancestors_extends
let extends cls ancestor =
  SSet.mem ancestor cls.tc_extends

let all_ancestors cls =
  Sequence.of_list (SMap.bindings cls.tc_ancestors)
let all_ancestor_names cls =
  Sequence.of_list (SMap.ordered_keys cls.tc_ancestors)
let all_ancestor_reqs cls =
  Sequence.of_list cls.tc_req_ancestors
let all_ancestor_req_names cls =
  Sequence.of_list (SSet.elements cls.tc_req_ancestors_extends)
let all_extends_ancestors cls =
  Sequence.of_list (SSet.elements cls.tc_extends)

let get_const cls id     = SMap.get id cls.tc_consts
let get_typeconst cls id = SMap.get id cls.tc_typeconsts
let get_prop cls id      = SMap.get id cls.tc_props
let get_sprop cls id     = SMap.get id cls.tc_sprops
let get_method cls id    = SMap.get id cls.tc_methods
let get_smethod cls id   = SMap.get id cls.tc_smethods

let has_const cls id     = Option.is_some (get_const cls id)
let has_typeconst cls id = Option.is_some (get_typeconst cls id)
let has_prop cls id      = Option.is_some (get_prop cls id)
let has_sprop cls id     = Option.is_some (get_sprop cls id)
let has_method cls id    = Option.is_some (get_method cls id)
let has_smethod cls id   = Option.is_some (get_smethod cls id)

let consts cls     = Sequence.of_list (SMap.bindings cls.tc_consts)
let typeconsts cls = Sequence.of_list (SMap.bindings cls.tc_typeconsts)
let props cls      = Sequence.of_list (SMap.bindings cls.tc_props)
let sprops cls     = Sequence.of_list (SMap.bindings cls.tc_sprops)
let methods cls    = Sequence.of_list (SMap.bindings cls.tc_methods)
let smethods cls   = Sequence.of_list (SMap.bindings cls.tc_smethods)
