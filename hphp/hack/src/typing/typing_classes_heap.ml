(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Decl_inheritance
open Shallow_decl_defs
open Typing_defs
module Attrs = Attributes
module LSTable = Lazy_string_table
module SN = Naming_special_names

type lazy_class_type = {
  sc: shallow_class;
  ih: inherited_members;
  ancestors: decl_ty LSTable.t;
  parents_and_traits: unit LSTable.t;
  members_fully_known: bool Lazy.t;
  req_ancestor_names: unit LSTable.t;
  all_requirements: (Pos.t * decl_ty) list Lazy.t;
}

type class_type_variant =
  | Lazy of lazy_class_type
  | Eager of class_type

let make_lazy_class_type class_name sc =
  let Decl_ancestors.
        {
          ancestors;
          parents_and_traits;
          members_fully_known;
          req_ancestor_names;
          all_requirements;
        } =
    Decl_ancestors.make class_name
  in
  let get_ancestor = LSTable.get ancestors in
  let inherited_members = Decl_inheritance.make class_name get_ancestor in
  {
    sc;
    ih = inherited_members;
    ancestors;
    parents_and_traits;
    members_fully_known;
    req_ancestor_names;
    all_requirements;
  }

let shallow_decl_enabled () =
  TypecheckerOptions.shallow_class_decl (GlobalNamingOptions.get ())

let defer_threshold () =
  TypecheckerOptions.defer_class_declaration_threshold
    (GlobalNamingOptions.get ())

module Classes = struct
  module Cache =
    SharedMem.LocalCache
      (StringKey)
      (struct
        type t = class_type_variant

        let prefix = Prefix.make ()

        let description = "ClassType"
      end)

  type key = StringKey.t

  type t = class_type_variant

  let compute_class_decl ~(use_cache : bool) (class_name : string) : t option =
    try
      let get_eager_class_type class_name =
        Decl_class.to_class_type
        @@
        let cached =
          if use_cache then
            Decl_heap.Classes.get class_name
          else
            None
        in
        match cached with
        | Some dc -> dc
        | None ->
          (match Naming_table.Types.get_pos class_name with
          | Some (_, Naming_table.TTypedef)
          | None ->
            raise Exit
          | Some (pos, Naming_table.TClass) ->
            let file = FileInfo.get_pos_filename pos in
            Option.iter (defer_threshold ()) ~f:(fun threshold ->
                Deferred_decl.should_defer ~d:file ~threshold);
            let class_type =
              Errors.run_in_decl_mode file (fun () ->
                  Decl.declare_class_in_file file class_name)
            in
            (match class_type with
            | Some class_type -> class_type
            | None ->
              failwith
                ("No class returned for get_eager_class_type on " ^ class_name)))
      in
      (* We don't want to fetch the shallow_class if shallow_class_decl is not
         enabled--this would frequently involve a re-parse, which would result
         in a huge perf penalty. We also want to avoid computing the folded
         decl of the class and all its ancestors when shallow_class_decl is
         enabled. We maintain these invariants in this module by only ever
         constructing [Eager] or [Lazy] classes, depending on whether
         shallow_class_decl is enabled. *)
      let class_type_variant =
        if shallow_decl_enabled () then
          match Shallow_classes_heap.get class_name with
          | None -> raise Exit
          | Some sc -> Lazy (make_lazy_class_type class_name sc)
        else
          Eager (get_eager_class_type class_name)
      in
      if use_cache then Cache.add class_name class_type_variant;
      Some class_type_variant
      (* If we raise Exit, then the class does not exist. *)
    with
    | Deferred_decl.Defer d ->
      Deferred_decl.add ~d;
      None
    | Exit -> None

  let get class_name =
    match Cache.get class_name with
    | Some t -> Some t
    | None -> compute_class_decl ~use_cache:true class_name

  let find_unsafe key =
    match get key with
    | None -> raise Caml.Not_found
    | Some x -> x

  let mem key =
    match get key with
    | None -> false
    | Some _ -> true
end

module Api = struct
  type t = class_type_variant

  let members_fully_known t =
    match t with
    | Lazy lc -> Lazy.force lc.members_fully_known
    | Eager c -> c.tc_members_fully_known

  let abstract t =
    match t with
    | Lazy lc ->
      begin
        match lc.sc.sc_kind with
        | Ast_defs.Cabstract
        | Ast_defs.Cinterface
        | Ast_defs.Ctrait
        | Ast_defs.Cenum ->
          true
        | _ -> false
      end
    | Eager c -> c.tc_abstract

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
    | Lazy lc ->
      Attrs.mem SN.UserAttributes.uaProbabilisticModel lc.sc.sc_user_attributes
    | Eager c -> c.tc_ppl

  let kind t =
    match t with
    | Lazy lc -> lc.sc.sc_kind
    | Eager c -> c.tc_kind

  let is_xhp t =
    match t with
    | Lazy lc -> lc.sc.sc_is_xhp
    | Eager c -> c.tc_is_xhp

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

  let where_constraints t =
    match t with
    | Lazy lc -> lc.sc.sc_where_constraints
    | Eager c -> c.tc_where_constraints

  let construct t =
    match t with
    | Lazy lc -> Lazy.force lc.ih.construct
    | Eager c -> c.tc_construct

  let enum_type t =
    match t with
    | Lazy lc -> lc.sc.sc_enum_type
    | Eager c -> c.tc_enum_type

  let get_sealed_whitelist sc =
    Aast.(
      match Attrs.find SN.UserAttributes.uaSealed sc.sc_user_attributes with
      | None -> None
      | Some { ua_params; _ } ->
        let add_class_name names param =
          match param with
          | (_, Class_const ((_, CI (_, cls)), (_, name)))
            when name = SN.Members.mClass ->
            SSet.add cls names
          | _ -> names
        in
        Some (List.fold_left ua_params ~f:add_class_name ~init:SSet.empty))

  let sealed_whitelist t =
    match t with
    | Lazy lc -> get_sealed_whitelist lc.sc
    | Eager c -> c.tc_sealed_whitelist

  let decl_errors t =
    match t with
    | Lazy lc -> Some lc.sc.sc_decl_errors
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
      begin
        match fst (construct t) with
        | None -> false
        | Some ce -> not ce.ce_abstract
      end
    | Eager c -> c.tc_need_init

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
    | Lazy lc -> LSTable.mem lc.req_ancestor_names ancestor
    | Eager c -> SSet.mem ancestor c.tc_req_ancestors_extends

  let extends t ancestor =
    match t with
    | Lazy lc -> LSTable.mem lc.parents_and_traits ancestor
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
    | Lazy lc -> Sequence.of_list (Lazy.force lc.all_requirements)
    | Eager c -> Sequence.of_list c.tc_req_ancestors

  let all_ancestor_req_names t =
    match t with
    | Lazy lc -> LSTable.to_seq lc.req_ancestor_names |> Sequence.map ~f:fst
    | Eager c -> Sequence.of_list (SSet.elements c.tc_req_ancestors_extends)

  let all_extends_ancestors t =
    match t with
    | Lazy lc -> LSTable.to_seq lc.parents_and_traits |> Sequence.map ~f:fst
    | Eager c -> Sequence.of_list (SSet.elements c.tc_extends)

  let all_where_constraints_on_this t =
    List.filter
      ~f:(fun c ->
        match c with
        | ((_, Typing_defs.Tthis), _, _)
        | (_, _, (_, Typing_defs.Tthis)) ->
          true
        | _ -> false)
      (where_constraints t)

  let upper_bounds_on_this_from_constraints t =
    List.filter_map
      ~f:(fun c ->
        match c with
        | ((_, Typing_defs.Tthis), Ast_defs.Constraint_as, ty)
        | ((_, Typing_defs.Tthis), Ast_defs.Constraint_eq, ty)
        | (ty, Ast_defs.Constraint_eq, (_, Typing_defs.Tthis))
        | (ty, Ast_defs.Constraint_super, (_, Typing_defs.Tthis)) ->
          Some ty
        | _ -> None)
      (where_constraints t)
    |> Sequence.of_list

  (* get upper bounds on `this` from the where constraints as well as
   * requirements *)
  let upper_bounds_on_this t =
    Sequence.map ~f:(fun req -> snd req) (all_ancestor_reqs t)
    |> Sequence.append (upper_bounds_on_this_from_constraints t)

  let has_upper_bounds_on_this_from_constraints t =
    not (Sequence.is_empty (upper_bounds_on_this_from_constraints t))

  (* get lower bounds on `this` from the where constraints *)
  let lower_bounds_on_this_from_constraints t =
    List.filter_map
      ~f:(fun c ->
        match c with
        | ((_, Typing_defs.Tthis), Ast_defs.Constraint_super, ty)
        | ((_, Typing_defs.Tthis), Ast_defs.Constraint_eq, ty)
        | (ty, Ast_defs.Constraint_eq, (_, Typing_defs.Tthis))
        | (ty, Ast_defs.Constraint_as, (_, Typing_defs.Tthis)) ->
          Some ty
        | _ -> None)
      (where_constraints t)
    |> Sequence.of_list

  let lower_bounds_on_this = lower_bounds_on_this_from_constraints

  let has_lower_bounds_on_this_from_constraints t =
    not (Sequence.is_empty (lower_bounds_on_this_from_constraints t))

  let is_disposable_class_name class_name =
    class_name = SN.Classes.cIDisposable
    || class_name = SN.Classes.cIAsyncDisposable

  let is_disposable t =
    match t with
    | Lazy _ ->
      is_disposable_class_name (name t)
      || Sequence.exists (all_ancestor_names t) is_disposable_class_name
      || Sequence.exists (all_ancestor_req_names t) is_disposable_class_name
    | Eager c -> c.tc_is_disposable

  let get_const t id =
    match t with
    | Lazy lc -> LSTable.get lc.ih.consts id
    | Eager c -> SMap.get id c.tc_consts

  let get_typeconst t id =
    match t with
    | Lazy lc -> LSTable.get lc.ih.typeconsts id
    | Eager c -> SMap.get id c.tc_typeconsts

  let get_pu_enum t id =
    match t with
    | Lazy lc -> LSTable.get lc.ih.pu_enums id
    | Eager c -> SMap.get id c.tc_pu_enums

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

  let pu_enums t =
    match t with
    | Lazy lc -> LSTable.to_seq lc.ih.pu_enums |> sort_by_key
    | Eager c -> Sequence.of_list (SMap.bindings c.tc_pu_enums)

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
end

let compute_class_decl_no_cache = Classes.compute_class_decl ~use_cache:false
