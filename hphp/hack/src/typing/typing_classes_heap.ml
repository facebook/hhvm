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
  lin_members: Decl_defs.mro_element list;
  lin_ancestors: Decl_defs.mro_element list;
  ih: inherited_members;
  ancestors: decl_ty LSTable.t;  (** Types of parents, interfaces, and traits *)
  req_ancestor_names: unit LSTable.t;
  all_requirements: (Pos_or_decl.t * decl_ty) Sequence.t;
}

type eager_members = {
  props: class_elt String.Table.t;
  static_props: class_elt String.Table.t;
  methods: class_elt String.Table.t;
  static_methods: class_elt String.Table.t;
  construct: Provider_context.t option -> class_elt option * consistent_kind;
}

(** class_t:
This type is an abstraction layer over shallow vs folded decl,
and provides a view of classes which includes all
inherited members and their types.

In legacy folded decl, that view is constructed by merging a single
heap entry for the folded class with many entries for the types
of each of its members (those member entries are looked up lazily,
as needed).

In shallow decl, that view is constructed even more lazily,
by iterating over the shallow representation of the class
and its ancestors one at a time. *)
type class_t =
  | Lazy of shallow_class * (lazy_class_type Lazy.t[@opaque])
  | Eager of Decl_defs.decl_class_type * (eager_members[@opaque])
[@@deriving show]

let make_lazy_class_type ctx class_name =
  match Shallow_classes_provider.get ctx class_name with
  | None -> None
  | Some sc ->
    let remainder =
      lazy
        (let { Decl_linearize.lin_members; lin_ancestors } =
           Decl_linearize.get_linearizations ctx class_name
         in
         let lin_members_seq = Sequence.of_list lin_members in
         let lin_ancestors_seq = Sequence.of_list lin_ancestors in
         let lin_ancestors_drop_one =
           Sequence.drop_eagerly lin_ancestors_seq 1
         in
         let is_canonical _ = true in
         let merge ~earlier ~later:_ = earlier in
         let ancestors =
           LSTable.make
             (Decl_ancestors.all_ancestors ~lin_ancestors_drop_one)
             ~is_canonical
             ~merge
         in
         {
           lin_members;
           lin_ancestors;
           ih =
             Decl_inheritance.make ctx class_name lin_members_seq (fun x ->
                 LSTable.get ancestors x);
           ancestors;
           req_ancestor_names =
             LSTable.make
               (Decl_ancestors.req_ancestor_names ~lin_members:lin_members_seq)
               ~is_canonical
               ~merge;
           all_requirements =
             Decl_ancestors.all_requirements ~lin_members:lin_members_seq;
         })
    in
    Some (Lazy (sc, remainder))

let make_eager_class_decl decl =
  Eager
    ( decl,
      {
        methods = String.Table.create ();
        static_methods = String.Table.create ();
        props = String.Table.create ();
        static_props = String.Table.create ();
        construct =
          (let cstr_ref = ref None in
           fun ctx ->
             match !cstr_ref with
             | Some cstr -> cstr
             | None ->
               let cstr =
                 Decl_class.lookup_constructor_lazy
                   ctx
                   ~child_class_name:decl.Decl_defs.dc_name
                   decl.Decl_defs.dc_substs
                   decl.Decl_defs.dc_construct
               in
               cstr_ref := Some cstr;
               cstr);
      } )

let make_eager_class_type ctx class_name declare_folded_class_in_file =
  match Decl_store.((get ()).get_class class_name) with
  | Some decl -> Some (make_eager_class_decl decl)
  | None ->
    begin
      match Naming_provider.get_class_path ctx class_name with
      | None -> None
      | Some file ->
        Deferred_decl.raise_if_should_defer ();
        (* declare_folded_class_in_file actual reads from Decl_heap.Classes.get
         * like what we do above, which makes our test redundant but cleaner.
         * It also writes into Decl_heap.Classes and other Decl_heaps. *)
        let (decl, _) = declare_folded_class_in_file ctx file class_name in
        Some (make_eager_class_decl decl)
    end

let get
    (ctx : Provider_context.t)
    (class_name : string)
    declare_folded_class_in_file : class_t option =
  (* Fetches either the [Lazy] class (if shallow decls are enabled)
   * or the [Eager] class (otherwise).
   * Note: Eager will always read+write to shmem Decl_heaps.
   * Lazy will solely go through the ctx provider. *)
  if TypecheckerOptions.shallow_class_decl (Provider_context.get_tcopt ctx) then
    make_lazy_class_type ctx class_name
  else
    make_eager_class_type ctx class_name declare_folded_class_in_file

module ApiShallow = struct
  let shallow_decl (decl, t, _ctx) =
    (* Looks only at the immediate shallow decl *)
    Decl_counters.count_subdecl decl Decl_counters.Shallow_decl @@ fun () ->
    match t with
    | Lazy (sc, _lc) -> sc
    | Eager _ -> failwith "shallow_class_decl is disabled"

  let abstract (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Abstract @@ fun () ->
    match t with
    | Lazy (sc, _lc) ->
      begin
        match sc.sc_kind with
        | Ast_defs.Cclass k -> Ast_defs.is_abstract k
        | Ast_defs.Cenum_class k -> Ast_defs.is_abstract k
        | Ast_defs.Cinterface
        | Ast_defs.Ctrait
        | Ast_defs.Cenum ->
          true
      end
    | Eager (c, _) -> c.Decl_defs.dc_abstract

  let final (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Final @@ fun () ->
    match t with
    | Lazy (sc, _lc) -> sc.sc_final
    | Eager (c, _) -> c.Decl_defs.dc_final

  let const (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Const @@ fun () ->
    match t with
    | Lazy (sc, _lc) ->
      Attrs.mem SN.UserAttributes.uaConst sc.sc_user_attributes
    | Eager (c, _) -> c.Decl_defs.dc_const

  let kind (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Kind @@ fun () ->
    match t with
    | Lazy (sc, _lc) -> sc.sc_kind
    | Eager (c, _) -> c.Decl_defs.dc_kind

  let is_xhp (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Is_xhp @@ fun () ->
    match t with
    | Lazy (sc, _lc) -> sc.sc_is_xhp
    | Eager (c, _) -> c.Decl_defs.dc_is_xhp

  let name (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Name @@ fun () ->
    match t with
    | Lazy (sc, _lc) -> snd sc.sc_name
    | Eager (c, _) -> c.Decl_defs.dc_name

  let pos (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Pos @@ fun () ->
    match t with
    | Lazy (sc, _lc) -> fst sc.sc_name
    | Eager (c, _) -> c.Decl_defs.dc_pos

  let tparams (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Tparams @@ fun () ->
    match t with
    | Lazy (sc, _lc) -> sc.sc_tparams
    | Eager (c, _) -> c.Decl_defs.dc_tparams

  let where_constraints (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Where_constraints
    @@ fun () ->
    match t with
    | Lazy (sc, _lc) -> sc.sc_where_constraints
    | Eager (c, _) -> c.Decl_defs.dc_where_constraints

  let enum_type (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Enum_type @@ fun () ->
    match t with
    | Lazy (sc, _lc) -> sc.sc_enum_type
    | Eager (c, _) -> c.Decl_defs.dc_enum_type

  let xhp_enum_values (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Xhp_enum_values @@ fun () ->
    match t with
    | Lazy (sc, _lc) -> sc.sc_xhp_enum_values
    | Eager (c, _) -> c.Decl_defs.dc_xhp_enum_values

  let sealed_whitelist (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Sealed_whitelist @@ fun () ->
    let get_sealed_whitelist sc =
      match Attrs.find SN.UserAttributes.uaSealed sc.sc_user_attributes with
      | None -> None
      | Some { ua_classname_params; _ } ->
        Some (SSet.of_list ua_classname_params)
    in
    match t with
    | Lazy (sc, _lc) -> get_sealed_whitelist sc
    | Eager (c, _) -> c.Decl_defs.dc_sealed_whitelist

  let get_module (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Module @@ fun () ->
    match t with
    | Lazy (sc, _lc) -> Option.map sc.sc_module ~f:snd
    | Eager (c, _) -> Option.map c.Decl_defs.dc_module ~f:snd

  let internal (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Internal @@ fun () ->
    match t with
    | Lazy (sc, _lc) ->
      Attrs.mem SN.UserAttributes.uaInternal sc.sc_user_attributes
    | Eager (c, _) -> c.Decl_defs.dc_internal

  let decl_errors (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Decl_errors @@ fun () ->
    match t with
    | Lazy _ -> None
    | Eager (c, _) -> c.Decl_defs.dc_decl_errors

  let get_support_dynamic_type (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Support_dynamic_type
    @@ fun () ->
    match t with
    | Lazy (sc, _lc) -> sc.sc_support_dynamic_type
    | Eager (c, _) -> c.Decl_defs.dc_support_dynamic_type

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
    match t with
    | Lazy (_sc, lc) -> Lazy.force (Lazy.force lc).ih.construct
    | Eager (_, members) -> members.construct ctx

  let need_init (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Need_init @@ fun () ->
    match t with
    | Lazy (_sc, lc) ->
      let (constructor, _) = Lazy.force (Lazy.force lc).ih.construct in
      (match constructor with
      | None -> false
      | Some ce -> not (get_ce_abstract ce))
    | Eager (c, _) -> c.Decl_defs.dc_need_init

  let get_ancestor (decl, t, _ctx) ancestor =
    Decl_counters.count_subdecl decl (Decl_counters.Get_ancestor ancestor)
    @@ fun () ->
    match t with
    | Lazy (_sc, lc) -> LSTable.get (Lazy.force lc).ancestors ancestor
    | Eager (c, _) -> SMap.find_opt ancestor c.Decl_defs.dc_ancestors

  let has_ancestor (decl, t, _ctx) ancestor =
    Decl_counters.count_subdecl decl (Decl_counters.Has_ancestor ancestor)
    @@ fun () ->
    match t with
    | Lazy (_sc, lc) -> LSTable.mem (Lazy.force lc).ancestors ancestor
    | Eager (c, _) -> SMap.mem ancestor c.Decl_defs.dc_ancestors

  let requires_ancestor (decl, t, _ctx) ancestor =
    Decl_counters.count_subdecl decl (Decl_counters.Requires_ancestor ancestor)
    @@ fun () ->
    match t with
    | Lazy (_sc, lc) -> LSTable.mem (Lazy.force lc).req_ancestor_names ancestor
    | Eager (c, _) -> SSet.mem ancestor c.Decl_defs.dc_req_ancestors_extends

  let get_const (decl, t, _ctx) id =
    Decl_counters.count_subdecl decl (Decl_counters.Get_const id) @@ fun () ->
    match t with
    | Lazy (_sc, lc) -> LSTable.get (Lazy.force lc).ih.consts id
    | Eager (c, _) -> SMap.find_opt id c.Decl_defs.dc_consts

  let has_const (decl, t, _ctx) id =
    Decl_counters.count_subdecl decl (Decl_counters.Has_const id) @@ fun () ->
    match t with
    | Lazy (_sc, lc) -> LSTable.mem (Lazy.force lc).ih.consts id
    | Eager (c, _) -> SMap.mem id c.Decl_defs.dc_consts

  let get_typeconst (decl, t, _ctx) id =
    Decl_counters.count_subdecl decl (Decl_counters.Get_typeconst id)
    @@ fun () ->
    match t with
    | Lazy (_sc, lc) -> LSTable.get (Lazy.force lc).ih.typeconsts id
    | Eager (c, _) -> SMap.find_opt id c.Decl_defs.dc_typeconsts

  let has_typeconst (decl, t, _ctx) id =
    Decl_counters.count_subdecl decl (Decl_counters.Has_typeconst id)
    @@ fun () ->
    match t with
    | Lazy (_sc, lc) -> LSTable.mem (Lazy.force lc).ih.typeconsts id
    | Eager (c, _) -> SMap.mem id c.Decl_defs.dc_typeconsts

  let get_typeconst_enforceability (decl, t, _ctx) id =
    Decl_counters.count_subdecl
      decl
      (Decl_counters.Get_typeconst_enforceability id)
    @@ fun () ->
    match t with
    | Lazy (_sc, lc) ->
      LSTable.get (Lazy.force lc).ih.typeconst_enforceability id
    | Eager (c, _) ->
      Option.map (SMap.find_opt id c.Decl_defs.dc_typeconsts) ~f:(fun t ->
          t.ttc_enforceable)

  let get_prop (decl, t, ctx) id =
    Decl_counters.count_subdecl decl (Decl_counters.Get_prop id) @@ fun () ->
    match t with
    | Lazy (_sc, lc) -> LSTable.get (Lazy.force lc).ih.props id
    | Eager (c, members) ->
      (match String.Table.find members.props id with
      | Some _ as elt_opt -> elt_opt
      | None ->
        (match SMap.find_opt id c.Decl_defs.dc_props with
        | None -> None
        | Some elt ->
          let elt = Decl_class.lookup_property_type_lazy ctx c id elt in
          String.Table.add_exn members.props ~key:id ~data:elt;
          Some elt))

  let has_prop (decl, t, _ctx) id =
    Decl_counters.count_subdecl decl (Decl_counters.Has_prop id) @@ fun () ->
    match t with
    | Lazy (_sc, lc) -> LSTable.mem (Lazy.force lc).ih.props id
    | Eager (c, _) -> SMap.mem id c.Decl_defs.dc_props

  let get_sprop (decl, t, ctx) id =
    Decl_counters.count_subdecl decl (Decl_counters.Get_sprop id) @@ fun () ->
    match t with
    | Lazy (_sc, lc) -> LSTable.get (Lazy.force lc).ih.sprops id
    | Eager (c, members) ->
      (match String.Table.find members.static_props id with
      | Some _ as elt_opt -> elt_opt
      | None ->
        (match SMap.find_opt id c.Decl_defs.dc_sprops with
        | None -> None
        | Some elt ->
          let elt = Decl_class.lookup_static_property_type_lazy ctx c id elt in
          String.Table.add_exn members.static_props ~key:id ~data:elt;
          Some elt))

  let has_sprop (decl, t, _ctx) id =
    Decl_counters.count_subdecl decl (Decl_counters.Has_sprop id) @@ fun () ->
    match t with
    | Lazy (_sc, lc) -> LSTable.mem (Lazy.force lc).ih.sprops id
    | Eager (c, _) -> SMap.mem id c.Decl_defs.dc_sprops

  let get_method (decl, t, ctx) id =
    Decl_counters.count_subdecl decl (Decl_counters.Get_method id) @@ fun () ->
    match t with
    | Lazy (_sc, lc) -> LSTable.get (Lazy.force lc).ih.methods id
    | Eager (c, members) ->
      (match String.Table.find members.methods id with
      | Some _ as elt_opt -> elt_opt
      | None ->
        (match SMap.find_opt id c.Decl_defs.dc_methods with
        | None -> None
        | Some elt ->
          let elt = Decl_class.lookup_method_type_lazy ctx c id elt in
          String.Table.add_exn members.methods ~key:id ~data:elt;
          Some elt))

  let has_method (decl, t, _ctx) id =
    Decl_counters.count_subdecl decl (Decl_counters.Has_method id) @@ fun () ->
    match t with
    | Lazy (_sc, lc) -> LSTable.mem (Lazy.force lc).ih.methods id
    | Eager (c, _) -> SMap.mem id c.Decl_defs.dc_methods

  let get_smethod (decl, t, ctx) id =
    Decl_counters.count_subdecl decl (Decl_counters.Get_smethod id) @@ fun () ->
    match t with
    | Lazy (_sc, lc) -> LSTable.get (Lazy.force lc).ih.smethods id
    | Eager (c, members) ->
      (match String.Table.find members.static_methods id with
      | Some _ as elt_opt -> elt_opt
      | None ->
        (match SMap.find_opt id c.Decl_defs.dc_smethods with
        | None -> None
        | Some elt ->
          let elt = Decl_class.lookup_static_method_type_lazy ctx c id elt in
          String.Table.add_exn members.static_methods ~key:id ~data:elt;
          Some elt))

  let has_smethod (decl, t, _ctx) id =
    Decl_counters.count_subdecl decl (Decl_counters.Has_smethod id) @@ fun () ->
    match t with
    | Lazy (_sc, lc) -> LSTable.mem (Lazy.force lc).ih.smethods id
    | Eager (c, _) -> SMap.mem id c.Decl_defs.dc_smethods

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
  let linearization (decl, t, _ctx) kind : Decl_defs.mro_element list =
    Decl_counters.count_subdecl decl Decl_counters.Linearization @@ fun () ->
    match (t, kind) with
    | (Lazy (_sc, lc), Decl_defs.Member_resolution) ->
      (Lazy.force lc).lin_members
    | (Lazy (_sc, lc), Decl_defs.Ancestor_types) ->
      (Lazy.force lc).lin_ancestors
    | (Eager _, _) -> failwith "shallow_class_decl is disabled"

  let all_ancestor_req_names (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.All_ancestor_req_names
    @@ fun () ->
    (* The two below will traverse ancestors in different orders.
       But if the typechecker discovers errors in different order, no matter. *)
    match t with
    | Lazy (_sc, lc) ->
      LSTable.to_list (Lazy.force lc).req_ancestor_names |> List.map ~f:fst
    | Eager (c, _) -> SSet.elements c.Decl_defs.dc_req_ancestors_extends

  let all_ancestors (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.All_ancestors @@ fun () ->
    match t with
    | Lazy (_sc, lc) -> LSTable.to_list (Lazy.force lc).ancestors
    | Eager (c, _) -> SMap.bindings c.Decl_defs.dc_ancestors

  let all_ancestor_names (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.All_ancestors @@ fun () ->
    match t with
    | Lazy (_sc, lc) ->
      List.map (LSTable.to_list (Lazy.force lc).ancestors) ~f:fst
    | Eager (c, _) -> SMap.ordered_keys c.Decl_defs.dc_ancestors

  let all_ancestor_reqs (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.All_ancestor_reqs
    @@ fun () ->
    match t with
    | Lazy (_sc, lc) -> (Lazy.force lc).all_requirements |> Sequence.to_list
    | Eager (c, _) -> c.Decl_defs.dc_req_ancestors

  let upper_bounds_on_this t =
    (* tally is already done by all_ancestors and upper_bounds *)
    List.map ~f:(fun req -> snd req) (all_ancestor_reqs t)
    |> List.append (ApiShallow.upper_bounds_on_this_from_constraints t)

  let consts (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Consts @@ fun () ->
    match t with
    | Lazy (_sc, lc) -> LSTable.to_list (Lazy.force lc).ih.consts
    | Eager (c, _) -> SMap.bindings c.Decl_defs.dc_consts

  let typeconsts (decl, t, _ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Typeconsts @@ fun () ->
    match t with
    | Lazy (_sc, lc) -> LSTable.to_list (Lazy.force lc).ih.typeconsts
    | Eager (c, _) -> SMap.bindings c.Decl_defs.dc_typeconsts

  let props (decl, t, ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Props @@ fun () ->
    match t with
    | Lazy (_sc, lc) -> LSTable.to_list (Lazy.force lc).ih.props
    | Eager (c, _) ->
      SMap.bindings c.Decl_defs.dc_props
      |> List.map ~f:(fun (id, elt) ->
             (id, Decl_class.lookup_property_type_lazy ctx c id elt))

  let sprops (decl, t, ctx) =
    Decl_counters.count_subdecl decl Decl_counters.SProps @@ fun () ->
    match t with
    | Lazy (_sc, lc) -> LSTable.to_list (Lazy.force lc).ih.sprops
    | Eager (c, _) ->
      SMap.bindings c.Decl_defs.dc_sprops
      |> List.map ~f:(fun (id, elt) ->
             (id, Decl_class.lookup_static_property_type_lazy ctx c id elt))

  let methods (decl, t, ctx) =
    Decl_counters.count_subdecl decl Decl_counters.Methods @@ fun () ->
    match t with
    | Lazy (_sc, lc) -> LSTable.to_list (Lazy.force lc).ih.methods
    | Eager (c, _) ->
      SMap.bindings c.Decl_defs.dc_methods
      |> List.map ~f:(fun (id, elt) ->
             (id, Decl_class.lookup_method_type_lazy ctx c id elt))

  let smethods (decl, t, ctx) =
    Decl_counters.count_subdecl decl Decl_counters.SMethods @@ fun () ->
    match t with
    | Lazy (_sc, lc) -> LSTable.to_list (Lazy.force lc).ih.smethods
    | Eager (c, _) ->
      SMap.bindings c.Decl_defs.dc_smethods
      |> List.map ~f:(fun (id, elt) ->
             (id, Decl_class.lookup_static_method_type_lazy ctx c id elt))

  let all_inherited_methods (decl, t, _ctx) id =
    Decl_counters.count_subdecl decl Decl_counters.All_inherited_methods
    @@ fun () ->
    match t with
    | Lazy (_sc, lc) ->
      LSTable.get (Lazy.force lc).ih.all_inherited_methods id
      |> Option.value ~default:[]
    | Eager _ -> failwith "shallow_class_decl is disabled"

  let all_inherited_smethods (decl, t, _ctx) id =
    Decl_counters.count_subdecl decl Decl_counters.All_inherited_smethods
    @@ fun () ->
    match t with
    | Lazy (_sc, lc) ->
      LSTable.get (Lazy.force lc).ih.all_inherited_smethods id
      |> Option.value ~default:[]
    | Eager _ -> failwith "shallow_class_decl is disabled"
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

  (* We cannot invoke [Typing_deferred_members.class_] here because it would be a
     dependency cycle. Instead, we raise an exception. We should remove this
     function (along with [Decl_init_check]) altogether when we delete legacy
     class declaration, since [Typing_deferred_members] makes it obsolete. *)
  let deferred_init_members (decl, t, _ctx) =
    (* Not applicable to shallow decl *)
    Decl_counters.count_subdecl decl Decl_counters.Deferred_init_members
    @@ fun () ->
    match t with
    | Lazy _ -> failwith "shallow_class_decl is enabled"
    | Eager (c, _) -> c.Decl_defs.dc_deferred_init_members

  let valid_newable_class cls =
    if Ast_defs.is_c_class (kind cls) then
      final cls
      || not (equal_consistent_kind (snd (construct cls)) Inconsistent)
    (* There is currently a bug with interfaces that allows constructors to change
     * their signature, so they are not considered here. TODO: T41093452 *)
    else
      false
end
