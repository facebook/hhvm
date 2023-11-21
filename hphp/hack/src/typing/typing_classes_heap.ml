(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
open Typing_class_types
module SN = Naming_special_names

type class_t = Typing_class_types.class_t [@@deriving show]

let make_eager_class_decl decl =
  ( decl,
    {
      methods = String.Table.create ();
      static_methods = String.Table.create ();
      props = String.Table.create ();
      static_props = String.Table.create ();
      construct = ref None;
    } )

let make_eager_class_type ctx class_name declare_folded_class =
  match Decl_store.((get ()).get_class class_name) with
  | Some decl -> Some (make_eager_class_decl decl)
  | None -> begin
    match Naming_provider.get_type_kind ctx class_name with
    | None -> None
    | Some Naming_types.TTypedef -> None
    | Some Naming_types.TClass ->
      Deferred_decl.raise_if_should_defer ();
      (* declare_folded_class_in_file actual reads from Decl_heap.Classes.get
       * like what we do above, which makes our test redundant but cleaner.
       * It also writes into Decl_heap.Classes and other Decl_heaps. *)
      let (decl, _) = declare_folded_class ctx class_name in
      Some (make_eager_class_decl decl)
  end

let get (ctx : Provider_context.t) (class_name : string) declare_folded_class :
    class_t option =
  make_eager_class_type ctx class_name declare_folded_class

module ApiShallow = struct
  let abstract (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Abstract @@ fun () ->
    let (c, _) = t in
    c.Decl_defs.dc_abstract

  let final (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Final @@ fun () ->
    let (c, _) = t in
    c.Decl_defs.dc_final

  let const (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Const @@ fun () ->
    let (c, _) = t in
    c.Decl_defs.dc_const

  let kind (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Kind @@ fun () ->
    let (c, _) = t in
    c.Decl_defs.dc_kind

  let is_xhp (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Is_xhp @@ fun () ->
    let (c, _) = t in
    c.Decl_defs.dc_is_xhp

  let name (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Name @@ fun () ->
    let (c, _) = t in
    c.Decl_defs.dc_name

  let pos (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Pos @@ fun () ->
    let (c, _) = t in
    c.Decl_defs.dc_pos

  let tparams (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Tparams @@ fun () ->
    let (c, _) = t in
    c.Decl_defs.dc_tparams

  let where_constraints (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Where_constraints
    @@ fun () ->
    let (c, _) = t in
    c.Decl_defs.dc_where_constraints

  let enum_type (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Enum_type @@ fun () ->
    let (c, _) = t in
    c.Decl_defs.dc_enum_type

  let xhp_enum_values (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Xhp_enum_values @@ fun () ->
    let (c, _) = t in
    c.Decl_defs.dc_xhp_enum_values

  let xhp_marked_empty (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Xhp_marked_empty @@ fun () ->
    let (c, _) = t in
    c.Decl_defs.dc_xhp_marked_empty

  let sealed_whitelist (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Sealed_whitelist @@ fun () ->
    let (c, _) = t in
    c.Decl_defs.dc_sealed_whitelist

  let get_docs_url (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Docs_url @@ fun () ->
    let (c, _) = t in
    c.Decl_defs.dc_docs_url

  let get_module (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Module @@ fun () ->
    let (c, _) = t in
    Option.map c.Decl_defs.dc_module ~f:snd

  let internal (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Internal @@ fun () ->
    let (c, _) = t in
    c.Decl_defs.dc_internal

  let is_module_level_trait (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.IsModuleLevelTrait
    @@ fun () ->
    let (c, _) = t in
    c.Decl_defs.dc_is_module_level_trait

  let decl_errors (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Decl_errors @@ fun () ->
    let (c, _) = t in
    c.Decl_defs.dc_decl_errors

  let get_support_dynamic_type (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Support_dynamic_type
    @@ fun () ->
    let (c, _) = t in
    c.Decl_defs.dc_support_dynamic_type

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
  let construct (decl, t, ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Construct @@ fun () ->
    let (cls, members) = t in
    match !(members.construct) with
    | Some x -> x
    | None ->
      let x =
        Decl_class.lookup_constructor_lazy
          ctx
          ~child_class_name:cls.Decl_defs.dc_name
          cls.Decl_defs.dc_substs
          cls.Decl_defs.dc_construct
      in
      members.construct := Some x;
      x

  let need_init (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Need_init @@ fun () ->
    let (c, _) = t in
    c.Decl_defs.dc_need_init

  let get_ancestor (decl, t, _ctx) ancestor =
    Decl_counters.count_subdecl decl (Decl_counters.Get_ancestor ancestor)
    @@ fun () ->
    let (c, _) = t in
    SMap.find_opt ancestor c.Decl_defs.dc_ancestors

  let has_ancestor (decl, t, _ctx) ancestor =
    Decl_counters.count_subdecl decl (Decl_counters.Has_ancestor ancestor)
    @@ fun () ->
    let (c, _) = t in
    SMap.mem ancestor c.Decl_defs.dc_ancestors

  let requires_ancestor (decl, t, _ctx) ancestor =
    Decl_counters.count_subdecl decl (Decl_counters.Requires_ancestor ancestor)
    @@ fun () ->
    let (c, _) = t in
    SSet.mem ancestor c.Decl_defs.dc_req_ancestors_extends

  let get_const (decl, t, _ctx) id =
    Decl_counters.count_subdecl decl (Decl_counters.Get_const id) @@ fun () ->
    let (c, _) = t in
    SMap.find_opt id c.Decl_defs.dc_consts

  let has_const (decl, t, _ctx) id =
    Decl_counters.count_subdecl decl (Decl_counters.Has_const id) @@ fun () ->
    let (c, _) = t in
    SMap.mem id c.Decl_defs.dc_consts

  let get_typeconst (decl, t, _ctx) id =
    Decl_counters.count_subdecl decl (Decl_counters.Get_typeconst id)
    @@ fun () ->
    let (c, _) = t in
    SMap.find_opt id c.Decl_defs.dc_typeconsts

  let has_typeconst (decl, t, _ctx) id =
    Decl_counters.count_subdecl decl (Decl_counters.Has_typeconst id)
    @@ fun () ->
    let (c, _) = t in
    SMap.mem id c.Decl_defs.dc_typeconsts

  let get_typeconst_enforceability (decl, t, _ctx) id =
    Decl_counters.count_subdecl
      decl
      (Decl_counters.Get_typeconst_enforceability id)
    @@ fun () ->
    let (c, _) = t in
    Option.map (SMap.find_opt id c.Decl_defs.dc_typeconsts) ~f:(fun t ->
        t.ttc_enforceable)

  let get_prop (decl, t, ctx) id =
    Decl_counters.count_subdecl decl (Decl_counters.Get_prop id) @@ fun () ->
    let (c, members) = t in
    match Hashtbl.find members.props id with
    | Some _ as elt_opt -> elt_opt
    | None ->
      (match SMap.find_opt id c.Decl_defs.dc_props with
      | None -> None
      | Some elt ->
        let elt = Decl_class.lookup_property_type_lazy ctx c id elt in
        Hashtbl.add_exn members.props ~key:id ~data:elt;
        Some elt)

  let has_prop (decl, t, _ctx) id =
    Decl_counters.count_subdecl decl (Decl_counters.Has_prop id) @@ fun () ->
    let (c, _) = t in
    SMap.mem id c.Decl_defs.dc_props

  let get_sprop (decl, t, ctx) id =
    Decl_counters.count_subdecl decl (Decl_counters.Get_sprop id) @@ fun () ->
    let (c, members) = t in
    match Hashtbl.find members.static_props id with
    | Some _ as elt_opt -> elt_opt
    | None ->
      (match SMap.find_opt id c.Decl_defs.dc_sprops with
      | None -> None
      | Some elt ->
        let elt = Decl_class.lookup_static_property_type_lazy ctx c id elt in
        Hashtbl.add_exn members.static_props ~key:id ~data:elt;
        Some elt)

  let has_sprop (decl, t, _ctx) id =
    Decl_counters.count_subdecl decl (Decl_counters.Has_sprop id) @@ fun () ->
    let (c, _) = t in
    SMap.mem id c.Decl_defs.dc_sprops

  let get_method (decl, t, ctx) id =
    Decl_counters.count_subdecl decl (Decl_counters.Get_method id) @@ fun () ->
    let (c, members) = t in
    match Hashtbl.find members.methods id with
    | Some _ as elt_opt -> elt_opt
    | None ->
      (match SMap.find_opt id c.Decl_defs.dc_methods with
      | None -> None
      | Some elt ->
        let elt = Decl_class.lookup_method_type_lazy ctx c id elt in
        Hashtbl.add_exn members.methods ~key:id ~data:elt;
        Some elt)

  let has_method (decl, t, _ctx) id =
    Decl_counters.count_subdecl decl (Decl_counters.Has_method id) @@ fun () ->
    let (c, _) = t in
    SMap.mem id c.Decl_defs.dc_methods

  let get_smethod (decl, t, ctx) id =
    Decl_counters.count_subdecl decl (Decl_counters.Get_smethod id) @@ fun () ->
    let (c, members) = t in
    match Hashtbl.find members.static_methods id with
    | Some _ as elt_opt -> elt_opt
    | None ->
      (match SMap.find_opt id c.Decl_defs.dc_smethods with
      | None -> None
      | Some elt ->
        let elt = Decl_class.lookup_static_method_type_lazy ctx c id elt in
        Hashtbl.add_exn members.static_methods ~key:id ~data:elt;
        Some elt)

  let has_smethod (decl, t, _ctx) id =
    Decl_counters.count_subdecl decl (Decl_counters.Has_smethod id) @@ fun () ->
    let (c, _) = t in
    SMap.mem id c.Decl_defs.dc_smethods

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
  let all_ancestor_req_names (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.All_ancestor_req_names
    @@ fun () ->
    (* The two below will traverse ancestors in different orders.
       But if the typechecker discovers errors in different order, no matter. *)
    let (c, _) = t in
    SSet.elements c.Decl_defs.dc_req_ancestors_extends

  let all_ancestors (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.All_ancestors @@ fun () ->
    let (c, _) = t in
    SMap.bindings c.Decl_defs.dc_ancestors

  let all_ancestor_names (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.All_ancestors @@ fun () ->
    let (c, _) = t in
    SMap.ordered_keys c.Decl_defs.dc_ancestors

  let all_ancestor_reqs (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.All_ancestor_reqs
    @@ fun () ->
    let (c, _) = t in
    c.Decl_defs.dc_req_ancestors

  let all_ancestor_req_class_requirements (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.All_ancestor_reqs
    @@ fun () ->
    let (c, _) = t in
    c.Decl_defs.dc_req_class_ancestors

  let upper_bounds_on_this t =
    (* tally is already done by all_ancestors and upper_bounds *)
    List.map ~f:(fun req -> snd req) (all_ancestor_reqs t)
    |> List.append (List.map ~f:snd (all_ancestor_req_class_requirements t))
    |> List.append (ApiShallow.upper_bounds_on_this_from_constraints t)

  let consts (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Consts @@ fun () ->
    let (c, _) = t in
    SMap.bindings c.Decl_defs.dc_consts

  let typeconsts (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Typeconsts @@ fun () ->
    let (c, _) = t in
    SMap.bindings c.Decl_defs.dc_typeconsts

  let props (decl, t, ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Props @@ fun () ->
    let (c, _) = t in
    SMap.bindings c.Decl_defs.dc_props
    |> List.map ~f:(fun (id, elt) ->
           (id, Decl_class.lookup_property_type_lazy ctx c id elt))

  let sprops (decl, t, ctx) =
    Decl_counters.count_subdecl decl Decl_counters.SProps @@ fun () ->
    let (c, _) = t in
    SMap.bindings c.Decl_defs.dc_sprops
    |> List.map ~f:(fun (id, elt) ->
           (id, Decl_class.lookup_static_property_type_lazy ctx c id elt))

  let methods (decl, t, ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Methods @@ fun () ->
    let (c, _) = t in
    SMap.bindings c.Decl_defs.dc_methods
    |> List.map ~f:(fun (id, elt) ->
           (id, Decl_class.lookup_method_type_lazy ctx c id elt))

  let smethods (decl, t, ctx) =
    Decl_counters.count_subdecl decl Decl_counters.SMethods @@ fun () ->
    let (c, _) = t in
    SMap.bindings c.Decl_defs.dc_smethods
    |> List.map ~f:(fun (id, elt) ->
           (id, Decl_class.lookup_static_method_type_lazy ctx c id elt))

  let overridden_method (decl, t, ctx) ~method_name ~is_static ~get_class =
    let open Option.Monad_infix in
    Decl_counters.count_subdecl decl Decl_counters.Overridden_method
    @@ fun () ->
    ctx >>= fun ctx ->
    let get_method (ty : decl_ty) : class_elt option =
      let (_, (_, class_name), _) = Decl_utils.unwrap_class_type ty in
      get_class ctx class_name >>= fun cls ->
      ApiLazy.get_any_method ~is_static cls method_name
    in
    let (cls, _members) = t in
    Decl_provider_internals.get_shallow_class ctx cls.Decl_defs.dc_name
    >>= Decl_inherit.find_overridden_method ~get_method
end

module Api = struct
  type t =
    (Decl_counters.decl option[@opaque])
    * class_t
    * (Provider_context.t[@opaque]) option
  [@@deriving show]

  include ApiShallow
  include ApiLazy
  include ApiEager

  let deferred_init_members (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Deferred_init_members
    @@ fun () ->
    let (c, _) = t in
    c.Decl_defs.dc_deferred_init_members

  let valid_newable_class cls =
    if Ast_defs.is_c_class (kind cls) then
      final cls
      || not (equal_consistent_kind (snd (construct cls)) Inconsistent)
    (* There is currently a bug with interfaces that allows constructors to change
     * their signature, so they are not considered here. TODO: T41093452 *)
    else
      false
end

let get_class_with_cache ctx class_name decl_cache declare_folded_class =
  Provider_backend.Decl_cache.find_or_add
    decl_cache
    ~key:(Provider_backend.Decl_cache_entry.Class_decl class_name)
    ~default:(fun () -> get ctx class_name declare_folded_class)
