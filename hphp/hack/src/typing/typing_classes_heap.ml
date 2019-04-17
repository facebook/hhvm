(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Decl_inheritance
open Typing_defs
open Shallow_decl_defs

module Attrs = Attributes
module LSTable = Lazy_string_table
module SN = Naming_special_names

type lazy_class_type = {
  sc: shallow_class;
  c: class_type;
  ih: inherited_members;
  ancestors: decl ty LSTable.t;
}

type class_type_variant =
  | Lazy of lazy_class_type
  | Eager of class_type

type t = class_type_variant

let make_lazy_class_type class_name sc c =
  let ancestors = Decl_ancestors.ancestors_cache class_name in
  let get_ancestor = LSTable.get ancestors in
  let inherited_members = Decl_inheritance.make class_name get_ancestor in
  {
    sc;
    c;
    ih = inherited_members;
    ancestors;
  }

let shallow_decl_enabled () =
  TypecheckerOptions.shallow_class_decl (GlobalNamingOptions.get ())

module Classes = struct
  module Cache = SharedMem.LocalCache (StringKey) (struct
    type t = class_type_variant
    let prefix = Prefix.make()
    let description = "ClassType"
  end)

  type key = StringKey.t
  type t = class_type_variant

  let get class_name =
    match Cache.get class_name with
    | Some t -> Some t
    | None ->
      try
        let get_eager_class_type class_name =
          Decl_class.to_class_type @@
          match Decl_heap.Classes.get class_name with
          | Some dc -> dc
          | None ->
            match Naming_table.Types.get_pos class_name with
            | Some (_, Naming_table.TTypedef) | None -> raise Exit
            | Some (pos, Naming_table.TClass) ->
              let file = FileInfo.get_pos_filename pos in
              Errors.run_in_decl_mode file
                (fun () -> Decl.declare_class_in_file file class_name);
              match Decl_heap.Classes.get class_name with
              | Some dc -> dc
              | None -> raise Exit
        in
        (* We don't want to fetch the shallow_class if shallow_class_decl is not
           enabled--this would frequently involve a re-parse, which would result
           in a huge perf penalty. We also want to avoid computing the folded
           decl of the class and all its ancestors when shallow_class_decl is
           enabled. We maintain these invariants in this module by only ever
           constructing [Eager] or [Lazy] classes, depending on whether
           shallow_class_decl is enabled. *)
        let class_type_variant =
          if shallow_decl_enabled ()
          then
            match Shallow_classes_heap.get class_name with
            | None -> raise Exit
            | Some sc ->
              let c = get_eager_class_type class_name in
              let lazy_class_type = make_lazy_class_type class_name sc c in
              Lazy lazy_class_type
          else
            let class_type = get_eager_class_type class_name in
            Eager class_type
        in
        Cache.add class_name class_type_variant;
        Some class_type_variant
      (* If we raise Exit, then the class does not exist. *)
      with Exit -> None

  let find_unsafe key =
    match get key with
    | None -> raise Caml.Not_found
    | Some x -> x

  let mem key =
    match get key with
    | None -> false
    | Some _ -> true
end

let members_fully_known t =
  match t with
  | Lazy lc -> lc.c.tc_members_fully_known
  | Eager c -> c.tc_members_fully_known

let abstract t =
  match t with
  | Lazy lc ->
    begin match lc.sc.sc_kind with
    | Ast.Cabstract | Ast.Cinterface | Ast.Ctrait | Ast.Cenum -> true
    | _ -> false
    end
  | Eager c ->
    c.tc_abstract

let final t =
  match t with
  | Lazy lc -> lc.sc.sc_final
  | Eager c -> c.tc_final

let const t =
  match t with
  | Lazy lc -> Attrs.mem SN.UserAttributes.uaConst lc.sc.sc_user_attributes
  | Eager c -> c.tc_const

let ppl t =
  match t with
  | Lazy lc -> Attrs.mem SN.UserAttributes.uaProbabilisticModel lc.sc.sc_user_attributes
  | Eager c -> c.tc_ppl

let kind t =
  match t with
  | Lazy lc -> lc.sc.sc_kind
  | Eager c -> c.tc_kind

let is_xhp t =
  match t with
  | Lazy lc -> lc.sc.sc_is_xhp
  | Eager c -> c.tc_is_xhp

let is_disposable t =
  match t with
  | Lazy lc -> lc.c.tc_is_disposable
  | Eager c -> c.tc_is_disposable

let name t =
  match t with
  | Lazy lc -> snd lc.sc.sc_name
  | Eager c -> c.tc_name

let pos t =
  match t with
  | Lazy lc -> fst lc.sc.sc_name
  | Eager c -> c.tc_pos

let tparams t =
  match t with
  | Lazy lc -> lc.sc.sc_tparams
  | Eager c -> c.tc_tparams

let construct t =
  match t with
  | Lazy lc -> Lazy.force lc.ih.construct
  | Eager c -> c.tc_construct

let enum_type t =
  match t with
  | Lazy lc -> lc.sc.sc_enum_type
  | Eager c -> c.tc_enum_type

let decl_errors t =
  match t with
  | Lazy lc -> lc.c.tc_decl_errors
  | Eager c -> c.tc_decl_errors

let sort_by_key seq =
  seq
  |> Sequence.to_list_rev
  |> List.sort ~compare:(fun (a, _) (b, _) -> String.compare a b)
  |> Sequence.of_list

let get_ancestor t ancestor =
  match t with
  | Lazy lc -> LSTable.get lc.ancestors ancestor
  | Eager c -> SMap.get ancestor c.tc_ancestors

let has_ancestor t ancestor =
  match t with
  | Lazy lc -> LSTable.mem lc.ancestors ancestor
  | Eager c -> SMap.mem ancestor c.tc_ancestors

let need_init t =
  match t with
  | Lazy _ ->
    begin match fst (construct t) with
    | None -> false
    | Some ce -> not ce.ce_abstract
    end
  | Eager c ->
    c.tc_need_init

(* We cannot invoke [Typing_deferred_members.class_] here because it would be a
   dependency cycle. Instead, we raise an exception. We should remove this
   function (along with [Decl_init_check]) altogether when we delete legacy
   class declaration, since [Typing_deferred_members] makes it obsolete. *)
let deferred_init_members t =
  match t with
  | Lazy _ -> failwith "shallow_class_decl is enabled"
  | Eager c -> c.tc_deferred_init_members

let requires_ancestor t ancestor =
  match t with
  | Lazy lc -> SSet.mem ancestor lc.c.tc_req_ancestors_extends
  | Eager c -> SSet.mem ancestor c.tc_req_ancestors_extends

let extends t ancestor =
  match t with
  | Lazy lc -> SSet.mem ancestor lc.c.tc_extends
  | Eager c -> SSet.mem ancestor c.tc_extends

let all_ancestors t =
  match t with
  | Lazy lc -> LSTable.to_seq lc.ancestors |> sort_by_key
  | Eager c -> SMap.bindings c.tc_ancestors |> Sequence.of_list

let all_ancestor_names t =
  match t with
  | Lazy _ -> Sequence.map (all_ancestors t) fst
  | Eager c -> SMap.ordered_keys c.tc_ancestors |> Sequence.of_list

let all_ancestor_reqs t =
  match t with
  | Lazy lc -> Sequence.of_list lc.c.tc_req_ancestors
  | Eager c -> Sequence.of_list c.tc_req_ancestors

let all_ancestor_req_names t =
  match t with
  | Lazy lc -> Sequence.of_list (SSet.elements lc.c.tc_req_ancestors_extends)
  | Eager c -> Sequence.of_list (SSet.elements c.tc_req_ancestors_extends)

let all_extends_ancestors t =
  match t with
  | Lazy lc -> Sequence.of_list (SSet.elements lc.c.tc_extends)
  | Eager c -> Sequence.of_list (SSet.elements c.tc_extends)

let get_const t id =
  match t with
  | Lazy lc -> LSTable.get lc.ih.consts id
  | Eager c -> SMap.get id c.tc_consts

let get_typeconst t id =
  match t with
  | Lazy lc -> LSTable.get lc.ih.typeconsts id
  | Eager c -> SMap.get id c.tc_typeconsts

let get_prop t id =
  match t with
  | Lazy lc -> LSTable.get lc.ih.props id
  | Eager c -> SMap.get id c.tc_props

let get_sprop t id =
  match t with
  | Lazy lc -> LSTable.get lc.ih.sprops id
  | Eager c -> SMap.get id c.tc_sprops

let get_method t id =
  match t with
  | Lazy lc -> LSTable.get lc.ih.methods id
  | Eager c -> SMap.get id c.tc_methods

let get_smethod t id =
  match t with
  | Lazy lc -> LSTable.get lc.ih.smethods id
  | Eager c -> SMap.get id c.tc_smethods

let has_const t id =
  match t with
  | Lazy lc -> LSTable.mem lc.ih.consts id
  | Eager _ -> Option.is_some (get_const t id)

let has_typeconst t id =
  match t with
  | Lazy lc -> LSTable.mem lc.ih.typeconsts id
  | Eager _ -> Option.is_some (get_typeconst t id)

let has_prop t id =
  match t with
  | Lazy lc -> LSTable.mem lc.ih.props id
  | Eager _ -> Option.is_some (get_prop t id)

let has_sprop t id =
  match t with
  | Lazy lc -> LSTable.mem lc.ih.sprops id
  | Eager _ -> Option.is_some (get_sprop t id)

let has_method t id =
  match t with
  | Lazy lc -> LSTable.mem lc.ih.methods id
  | Eager _ -> Option.is_some (get_method t id)

let has_smethod t id =
  match t with
  | Lazy lc -> LSTable.mem lc.ih.smethods id
  | Eager _ -> Option.is_some (get_smethod t id)

let consts t =
  match t with
  | Lazy lc -> LSTable.to_seq lc.ih.consts |> sort_by_key
  | Eager c -> Sequence.of_list (SMap.bindings c.tc_consts)

let typeconsts t =
  match t with
  | Lazy lc -> LSTable.to_seq lc.ih.typeconsts |> sort_by_key
  | Eager c -> Sequence.of_list (SMap.bindings c.tc_typeconsts)

let props t =
  match t with
  | Lazy lc -> LSTable.to_seq lc.ih.props |> sort_by_key
  | Eager c -> Sequence.of_list (SMap.bindings c.tc_props)

let sprops t =
  match t with
  | Lazy lc -> LSTable.to_seq lc.ih.sprops |> sort_by_key
  | Eager c -> Sequence.of_list (SMap.bindings c.tc_sprops)

let methods t =
  match t with
  | Lazy lc -> LSTable.to_seq lc.ih.methods |> sort_by_key
  | Eager c -> Sequence.of_list (SMap.bindings c.tc_methods)

let smethods t =
  match t with
  | Lazy lc -> LSTable.to_seq lc.ih.smethods |> sort_by_key
  | Eager c -> Sequence.of_list (SMap.bindings c.tc_smethods)

let get_all cache id = LSTable.get cache id |> Option.value ~default:[]

let all_inherited_methods t id =
  match t with
  | Lazy lc -> get_all lc.ih.all_inherited_methods id
  | Eager _ -> failwith "shallow_class_decl is disabled"

let all_inherited_smethods t id =
  match t with
  | Lazy lc -> get_all lc.ih.all_inherited_smethods id
  | Eager _ -> failwith "shallow_class_decl is disabled"

let shallow_decl t =
  match t with
  | Lazy lc -> lc.sc
  | Eager _ -> failwith "shallow_class_decl is disabled"
