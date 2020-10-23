(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Decl_inheritance
open Shallow_decl_defs
open Typing_defs
module Attrs = Typing_defs.Attributes
module LSTable = Lazy_string_table
module SN = Naming_special_names

type lazy_class_type = {
  sc: shallow_class;
  ih: inherited_members;
  ancestors: decl_ty LSTable.t;
  parents_and_traits: unit LSTable.t;
  members_fully_known: bool Lazy.t;
  req_ancestor_names: unit LSTable.t;
  all_requirements: (Pos.t * decl_ty) Sequence.t;
}

type class_type_variant =
  | Lazy of lazy_class_type
  | Eager of class_type

let make_lazy_class_type ctx decl_counter class_name =
  match Shallow_classes_provider.get ctx class_name with
  | None -> None
  | Some sc ->
    let Decl_ancestors.
          {
            ancestors;
            parents_and_traits;
            members_fully_known;
            req_ancestor_names;
            all_requirements;
          } =
      Decl_ancestors.make ctx class_name
    in
    let get_ancestor = LSTable.get ancestors in
    let inherited_members = Decl_inheritance.make ctx class_name get_ancestor in
    Some
      ( decl_counter,
        Lazy
          {
            sc;
            ih = inherited_members;
            ancestors;
            parents_and_traits;
            members_fully_known;
            req_ancestor_names;
            all_requirements;
          } )

let make_eager_class_type ctx decl_counter class_name =
  match Decl_heap.Classes.get class_name with
  | Some decl -> Some (decl_counter, Eager (Decl_class.to_class_type decl))
  | None ->
    begin
      match Naming_provider.get_type_path_and_kind ctx class_name with
      | Some (_, Naming_types.TTypedef)
      | Some (_, Naming_types.TRecordDef)
      | None ->
        None
      | Some (file, Naming_types.TClass) ->
        Deferred_decl.raise_if_should_defer ~d:file;
        (* declare_class_in_file actual reads from Decl_heap.Classes.get
        like what we do above, which makes our test redundant but cleaner.
        It also writes into Decl_heap.Classes and other Decl_heaps. *)
        let decl =
          Errors.run_in_decl_mode file (fun () ->
              Decl.declare_class_in_file ~sh:SharedMem.Uses ctx file class_name)
        in
        Deferred_decl.increment_counter ();
        Some
          ( decl_counter,
            Eager (Decl_class.to_class_type (Option.value_exn decl)) )
    end

module Classes = struct
  (** This module is an abstraction layer over shallow vs folded decl,
      and provides a view of classes which includes all
      inherited members and their types.

      In legacy folded decl, that view is constructed by merging a single
      heap entry for the folded class with many entries for the types
      of each of its members (those member entries are looked up lazily,
      as needed).

      In shallow decl, that view is constructed even more lazily,
      by iterating over the shallow representation of the class
      and its ancestors one at a time. *)

  (** This cache caches the result of full class computations
      (the class merged with all its inherited members.)  *)
  module Cache =
    SharedMem.LocalCache
      (StringKey)
      (struct
        type t = class_type_variant

        let prefix = Prefix.make ()

        let description = "Decl_Typing_ClassType"
      end)
      (struct
        let capacity = 1000
      end)

  type key = StringKey.t

  (** This type "t" what all subdecl accessors operate upon, e.g. "get method foo
  of the specified class_type_variant" or "get all smethods of the specified
  class_type_variant". We also pass in a "decl option". This provides context
  about how the specified class_type_variant was fetched in the first place.
  It's used solely for telemetry, so that telemetry about subdecl accessors
  can be easily correlated with how the original class_type_variant was fetched. *)
  type t = Decl_counters.decl option * class_type_variant

  let get_no_local_cache_impl
      (ctx : Provider_context.t)
      (decl_counter : Decl_counters.decl option)
      (class_name : string) :
      (Decl_counters.decl option * class_type_variant) option =
    (* Fetches either the [Lazy] class (if shallow decls are enabled)
    or the [Eager] class (otherwise).
    Note: Eager will always read+write to shmem Decl_heaps.
    Lazy will solely go through the ctx provider. *)
    try
      if TypecheckerOptions.shallow_class_decl (Provider_context.get_tcopt ctx)
      then
        make_lazy_class_type ctx decl_counter class_name
      else
        make_eager_class_type ctx decl_counter class_name
    with Deferred_decl.Defer d ->
      Deferred_decl.add_deferment ~d;
      None

  let get_no_local_cache (ctx : Provider_context.t) (class_name : string) :
      t option =
    Decl_counters.count_decl
      Decl_counters.Class_no_local_cache
      class_name
      (fun decl_counter -> get_no_local_cache_impl ctx decl_counter class_name)

  let get_impl ctx decl_counter class_name =
    match Cache.get class_name with
    | Some t -> Some (decl_counter, t)
    | None ->
      begin
        match get_no_local_cache_impl ctx decl_counter class_name with
        | None -> None
        | Some (decl_counter, class_type_variant) ->
          Cache.add class_name class_type_variant;
          Some (decl_counter, class_type_variant)
      end

  let get ctx class_name =
    Decl_counters.count_decl Decl_counters.Class class_name (fun decl_counter ->
        get_impl ctx decl_counter class_name)
end

module ApiShallow = struct
  let shallow_decl (decl, t) =
    (* Looks only at the immediate shallow decl *)
    Decl_counters.count_subdecl decl Decl_counters.Shallow_decl @@ fun () ->
    match t with
    | Lazy lc -> lc.sc
    | Eager _ -> failwith "shallow_class_decl is disabled"

  let abstract (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.Abstract @@ fun () ->
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

  let final (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.Final @@ fun () ->
    match t with
    | Lazy lc -> lc.sc.sc_final
    | Eager c -> c.tc_final

  let const (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.Const @@ fun () ->
    match t with
    | Lazy lc -> Attrs.mem SN.UserAttributes.uaConst lc.sc.sc_user_attributes
    | Eager c -> c.tc_const

  let kind (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.Kind @@ fun () ->
    match t with
    | Lazy lc -> lc.sc.sc_kind
    | Eager c -> c.tc_kind

  let is_xhp (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.Is_xhp @@ fun () ->
    match t with
    | Lazy lc -> lc.sc.sc_is_xhp
    | Eager c -> c.tc_is_xhp

  let name (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.Name @@ fun () ->
    match t with
    | Lazy lc -> snd lc.sc.sc_name
    | Eager c -> c.tc_name

  let pos (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.Pos @@ fun () ->
    match t with
    | Lazy lc -> fst lc.sc.sc_name
    | Eager c -> c.tc_pos

  let tparams (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.Tparams @@ fun () ->
    match t with
    | Lazy lc -> lc.sc.sc_tparams
    | Eager c -> c.tc_tparams

  let where_constraints (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.Where_constraints
    @@ fun () ->
    match t with
    | Lazy lc -> lc.sc.sc_where_constraints
    | Eager c -> c.tc_where_constraints

  let enum_type (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.Enum_type @@ fun () ->
    match t with
    | Lazy lc -> lc.sc.sc_enum_type
    | Eager c -> c.tc_enum_type

  let sealed_whitelist (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.Sealed_whitelist @@ fun () ->
    let get_sealed_whitelist sc =
      match Attrs.find SN.UserAttributes.uaSealed sc.sc_user_attributes with
      | None -> None
      | Some { ua_classname_params; _ } ->
        Some (SSet.of_list ua_classname_params)
    in
    match t with
    | Lazy lc -> get_sealed_whitelist lc.sc
    | Eager c -> c.tc_sealed_whitelist

  let decl_errors (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.Decl_errors @@ fun () ->
    match t with
    | Lazy _ -> None
    | Eager c -> c.tc_decl_errors

  let all_where_constraints_on_this t =
    (* tally is already done by where_constraints *)
    List.filter
      ~f:(fun (l, _, r) ->
        match (get_node l, get_node r) with
        | (Tthis, _)
        | (_, Tthis) ->
          true
        | _ -> false)
      (where_constraints t)

  let upper_bounds_on_this_from_constraints t =
    (* tally is already done by where_constraints *)
    List.filter_map
      ~f:(fun (l, c, r) ->
        match (get_node l, c, get_node r) with
        | (Tthis, Ast_defs.Constraint_as, _)
        | (Tthis, Ast_defs.Constraint_eq, _) ->
          Some r
        | (_, Ast_defs.Constraint_eq, Tthis)
        | (_, Ast_defs.Constraint_super, Tthis) ->
          Some l
        | _ -> None)
      (where_constraints t)

  let has_upper_bounds_on_this_from_constraints t =
    (* tally is already done by upper_bounds_on_this *)
    not (List.is_empty (upper_bounds_on_this_from_constraints t))

  (* get lower bounds on `this` from the where constraints *)
  let lower_bounds_on_this_from_constraints t =
    (* tally is already done by where_constraint *)
    List.filter_map
      ~f:(fun (l, c, r) ->
        match (get_node l, c, get_node r) with
        | (Tthis, Ast_defs.Constraint_super, _)
        | (Tthis, Ast_defs.Constraint_eq, _) ->
          Some r
        | (_, Ast_defs.Constraint_eq, Tthis)
        | (_, Ast_defs.Constraint_as, Tthis) ->
          Some l
        | _ -> None)
      (where_constraints t)

  let has_lower_bounds_on_this_from_constraints t =
    (* tally is already done by lower_bounds_on_this *)
    not (List.is_empty (lower_bounds_on_this_from_constraints t))

  let lower_bounds_on_this t =
    (* tally is done inside the following method *)
    lower_bounds_on_this_from_constraints t
end

module ApiLazy = struct
  let construct (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.Construct @@ fun () ->
    match t with
    | Lazy lc -> Lazy.force lc.ih.construct
    | Eager c -> c.tc_construct

  let need_init (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.Need_init @@ fun () ->
    match t with
    | Lazy lc ->
      let (constructor, _) = Lazy.force lc.ih.construct in
      (match constructor with
      | None -> false
      | Some ce -> not (get_ce_abstract ce))
    | Eager c -> c.tc_need_init

  let get_ancestor (decl, t) ancestor =
    Decl_counters.count_subdecl decl (Decl_counters.Get_ancestor ancestor)
    @@ fun () ->
    match t with
    | Lazy lc -> LSTable.get lc.ancestors ancestor
    | Eager c -> SMap.find_opt ancestor c.tc_ancestors

  let has_ancestor (decl, t) ancestor =
    Decl_counters.count_subdecl decl (Decl_counters.Has_ancestor ancestor)
    @@ fun () ->
    match t with
    | Lazy lc -> LSTable.mem lc.ancestors ancestor
    | Eager c -> SMap.mem ancestor c.tc_ancestors

  let requires_ancestor (decl, t) ancestor =
    Decl_counters.count_subdecl decl (Decl_counters.Requires_ancestor ancestor)
    @@ fun () ->
    match t with
    | Lazy lc -> LSTable.mem lc.req_ancestor_names ancestor
    | Eager c -> SSet.mem ancestor c.tc_req_ancestors_extends

  let extends (decl, t) ancestor =
    Decl_counters.count_subdecl decl (Decl_counters.Extends ancestor)
    @@ fun () ->
    match t with
    | Lazy lc -> LSTable.mem lc.parents_and_traits ancestor
    | Eager c -> SSet.mem ancestor c.tc_extends

  let get_const (decl, t) id =
    Decl_counters.count_subdecl decl (Decl_counters.Get_const id) @@ fun () ->
    match t with
    | Lazy lc -> LSTable.get lc.ih.consts id
    | Eager c -> SMap.find_opt id c.tc_consts

  let has_const (decl, t) id =
    Decl_counters.count_subdecl decl (Decl_counters.Has_const id) @@ fun () ->
    match t with
    | Lazy lc -> LSTable.mem lc.ih.consts id
    | Eager c -> SMap.mem id c.tc_consts

  let get_typeconst (decl, t) id =
    Decl_counters.count_subdecl decl (Decl_counters.Get_typeconst id)
    @@ fun () ->
    match t with
    | Lazy lc -> LSTable.get lc.ih.typeconsts id
    | Eager c -> SMap.find_opt id c.tc_typeconsts

  let has_typeconst (decl, t) id =
    Decl_counters.count_subdecl decl (Decl_counters.Has_typeconst id)
    @@ fun () ->
    match t with
    | Lazy lc -> LSTable.mem lc.ih.typeconsts id
    | Eager c -> SMap.mem id c.tc_typeconsts

  let get_prop (decl, t) id =
    Decl_counters.count_subdecl decl (Decl_counters.Get_prop id) @@ fun () ->
    match t with
    | Lazy lc -> LSTable.get lc.ih.props id
    | Eager c -> SMap.find_opt id c.tc_props

  let has_prop (decl, t) id =
    Decl_counters.count_subdecl decl (Decl_counters.Has_prop id) @@ fun () ->
    match t with
    | Lazy lc -> LSTable.mem lc.ih.props id
    | Eager c -> SMap.mem id c.tc_props

  let get_sprop (decl, t) id =
    Decl_counters.count_subdecl decl (Decl_counters.Get_sprop id) @@ fun () ->
    match t with
    | Lazy lc -> LSTable.get lc.ih.sprops id
    | Eager c -> SMap.find_opt id c.tc_sprops

  let has_sprop (decl, t) id =
    Decl_counters.count_subdecl decl (Decl_counters.Has_sprop id) @@ fun () ->
    match t with
    | Lazy lc -> LSTable.mem lc.ih.sprops id
    | Eager c -> SMap.mem id c.tc_sprops

  let get_method (decl, t) id =
    Decl_counters.count_subdecl decl (Decl_counters.Get_method id) @@ fun () ->
    match t with
    | Lazy lc -> LSTable.get lc.ih.methods id
    | Eager c -> SMap.find_opt id c.tc_methods

  let has_method (decl, t) id =
    Decl_counters.count_subdecl decl (Decl_counters.Has_method id) @@ fun () ->
    match t with
    | Lazy lc -> LSTable.mem lc.ih.methods id
    | Eager c -> SMap.mem id c.tc_methods

  let get_smethod (decl, t) id =
    Decl_counters.count_subdecl decl (Decl_counters.Get_smethod id) @@ fun () ->
    match t with
    | Lazy lc -> LSTable.get lc.ih.smethods id
    | Eager c -> SMap.find_opt id c.tc_smethods

  let has_smethod (decl, t) id =
    Decl_counters.count_subdecl decl (Decl_counters.Has_smethod id) @@ fun () ->
    match t with
    | Lazy lc -> LSTable.mem lc.ih.smethods id
    | Eager c -> SMap.mem id c.tc_smethods

  let get_any_method ~is_static cls id =
    (* tally is already done inside the following three methods *)
    if String.equal id SN.Members.__construct then
      fst (construct cls)
    else if is_static then
      get_smethod cls id
    else
      get_method cls id
end

module ApiEager = struct
  let members_fully_known (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.Members_fully_known
    @@ fun () ->
    match t with
    | Lazy lc -> Lazy.force lc.members_fully_known
    | Eager c -> c.tc_members_fully_known

  let all_ancestor_req_names (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.All_ancestor_req_names
    @@ fun () ->
    (* The two below will traverse ancestors in different orders.
    But if the typechecker discovers errors in different order, no matter. *)
    match t with
    | Lazy lc -> LSTable.to_list lc.req_ancestor_names |> List.map ~f:fst
    | Eager c -> SSet.elements c.tc_req_ancestors_extends

  let all_extends_ancestors (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.All_extends_ancestors
    @@ fun () ->
    match t with
    | Lazy lc -> LSTable.to_list lc.parents_and_traits |> List.map ~f:fst
    | Eager c -> SSet.elements c.tc_extends

  let all_ancestors (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.All_ancestors @@ fun () ->
    match t with
    | Lazy lc -> LSTable.to_list lc.ancestors
    | Eager c -> SMap.bindings c.tc_ancestors

  let all_ancestor_names (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.All_ancestors @@ fun () ->
    match t with
    | Lazy lc -> List.map (LSTable.to_list lc.ancestors) fst
    | Eager c -> SMap.ordered_keys c.tc_ancestors

  let all_ancestor_reqs (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.All_ancestor_reqs
    @@ fun () ->
    match t with
    | Lazy lc -> lc.all_requirements |> Sequence.to_list
    | Eager c -> c.tc_req_ancestors

  let upper_bounds_on_this t =
    (* tally is already done by all_ancestors and upper_bounds *)
    List.map ~f:(fun req -> snd req) (all_ancestor_reqs t)
    |> List.append (ApiShallow.upper_bounds_on_this_from_constraints t)

  let is_disposable (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.Is_disposable @@ fun () ->
    let is_disposable_class_name class_name =
      String.equal class_name SN.Classes.cIDisposable
      || String.equal class_name SN.Classes.cIAsyncDisposable
    in
    match t with
    | Lazy lc ->
      let all_ancestor_names = List.map (LSTable.to_list lc.ancestors) fst in
      let all_ancestor_req_names =
        LSTable.to_list lc.req_ancestor_names |> List.map ~f:fst
      in
      is_disposable_class_name (snd lc.sc.sc_name)
      || List.exists all_ancestor_names is_disposable_class_name
      || List.exists all_ancestor_req_names is_disposable_class_name
    | Eager c -> c.tc_is_disposable

  let get_pu_enum (decl, t) id =
    Decl_counters.count_subdecl decl (Decl_counters.Get_pu_enum id) @@ fun () ->
    match t with
    | Lazy lc -> LSTable.get lc.ih.pu_enums id
    | Eager c -> SMap.find_opt id c.tc_pu_enums

  let consts (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.Consts @@ fun () ->
    match t with
    | Lazy lc -> LSTable.to_list lc.ih.consts
    | Eager c -> SMap.bindings c.tc_consts

  let typeconsts (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.Typeconsts @@ fun () ->
    match t with
    | Lazy lc -> LSTable.to_list lc.ih.typeconsts
    | Eager c -> SMap.bindings c.tc_typeconsts

  let pu_enums (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.Pu_enums @@ fun () ->
    match t with
    | Lazy lc -> LSTable.to_list lc.ih.pu_enums
    | Eager c -> SMap.bindings c.tc_pu_enums

  let props (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.Props @@ fun () ->
    match t with
    | Lazy lc -> LSTable.to_list lc.ih.props
    | Eager c -> SMap.bindings c.tc_props

  let sprops (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.SProps @@ fun () ->
    match t with
    | Lazy lc -> LSTable.to_list lc.ih.sprops
    | Eager c -> SMap.bindings c.tc_sprops

  let methods (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.Methods @@ fun () ->
    match t with
    | Lazy lc -> LSTable.to_list lc.ih.methods
    | Eager c -> SMap.bindings c.tc_methods

  let smethods (decl, t) =
    Decl_counters.count_subdecl decl Decl_counters.SMethods @@ fun () ->
    match t with
    | Lazy lc -> LSTable.to_list lc.ih.smethods
    | Eager c -> SMap.bindings c.tc_smethods

  let all_inherited_methods (decl, t) id =
    Decl_counters.count_subdecl decl Decl_counters.All_inherited_methods
    @@ fun () ->
    match t with
    | Lazy lc ->
      LSTable.get lc.ih.all_inherited_methods id |> Option.value ~default:[]
    | Eager _ -> failwith "shallow_class_decl is disabled"

  let all_inherited_smethods (decl, t) id =
    Decl_counters.count_subdecl decl Decl_counters.All_inherited_smethods
    @@ fun () ->
    match t with
    | Lazy lc ->
      LSTable.get lc.ih.all_inherited_smethods id |> Option.value ~default:[]
    | Eager _ -> failwith "shallow_class_decl is disabled"
end

module Api = struct
  type t = Decl_counters.decl option * class_type_variant

  include ApiShallow
  include ApiLazy
  include ApiEager

  (* We cannot invoke [Typing_deferred_members.class_] here because it would be a
     dependency cycle. Instead, we raise an exception. We should remove this
     function (along with [Decl_init_check]) altogether when we delete legacy
     class declaration, since [Typing_deferred_members] makes it obsolete. *)
  let deferred_init_members (decl, t) =
    (* Not applicable to shallow decl *)
    Decl_counters.count_subdecl decl Decl_counters.Deferred_init_members
    @@ fun () ->
    match t with
    | Lazy _ -> failwith "shallow_class_decl is enabled"
    | Eager c -> c.tc_deferred_init_members
end
