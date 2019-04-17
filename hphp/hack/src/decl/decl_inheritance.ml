(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Decl_defs
open Decl_subst
open Nast
open Reordered_argument_collections
open Shallow_decl_defs
open Typing_defs

module LSTable = Lazy_string_table
module Reason = Typing_reason

type inherited_members = {
  props : class_elt LSTable.t;
  sprops : class_elt LSTable.t;
  methods : class_elt LSTable.t;
  smethods : class_elt LSTable.t;
  all_inherited_methods : class_elt list LSTable.t;
  all_inherited_smethods : class_elt list LSTable.t;
  construct: (class_elt option * consistent_kind) Lazy.t;
}

(** [inheritable_elt] is a representation internal to Decl_inheritance which is
    used for both methods and properties (members represented using
    {!Typing_defs.class_elt}). Tagging these members with
    [should_chown_privates] allows us to write {!filter_or_chown_privates}. *)
type inheritable_elt = {
  id: string;
  should_chown_privates: bool;
  elt: class_elt;
}

let method_redeclaration_to_shallow_method smr =
  {
    sm_abstract = smr.smr_abstract;
    sm_final = smr.smr_final;
    sm_memoizelsb = false;
    sm_name = smr.smr_name;
    sm_override = false;
    sm_reactivity = None;
    sm_type = smr.smr_type;
    sm_visibility = smr.smr_visibility;
  }

let redecl_list_to_method_seq redecls =
  redecls
  |> Sequence.of_list
  |> Sequence.map ~f:method_redeclaration_to_shallow_method

let base_visibility origin_class_name = function
  | Public -> Vpublic
  | Private -> Vprivate origin_class_name
  | Protected -> Vprotected origin_class_name

let ft_to_ty ft = Reason.Rwitness ft.ft_pos, Tfun ft

let shallow_method_to_class_elt child_class mro subst meth : class_elt =
  let visibility = base_visibility mro.mro_name meth.sm_visibility in
  let ty = lazy begin
    let ty = ft_to_ty meth.sm_type in
    if child_class = mro.mro_name then ty else
    Decl_instantiate.instantiate subst ty
  end in
  {
    ce_abstract = meth.sm_abstract;
    ce_final = meth.sm_final;
    ce_is_xhp_attr = false;
    ce_const = false;
    ce_lateinit = false;
    ce_override = meth.sm_override;
    ce_lsb = false;
    ce_memoizelsb = meth.sm_memoizelsb;
    ce_synthesized = mro.mro_synthesized;
    ce_visibility = visibility;
    ce_origin = mro.mro_name;
    ce_type = ty;
  }

let shallow_method_to_ielt child_class mro subst meth : inheritable_elt =
  {
    id = snd meth.sm_name;
    should_chown_privates = mro.mro_copy_private_members;
    elt = shallow_method_to_class_elt child_class mro subst meth;
  }

let shallow_prop_to_ielt child_class mro subst prop : inheritable_elt =
  let visibility = base_visibility mro.mro_name prop.sp_visibility in
  let ty = lazy begin
    let ty = match prop.sp_type with
      | None -> Reason.Rwitness (fst prop.sp_name), Tany
      | Some ty -> ty
    in
    if child_class = mro.mro_name then ty else
    Decl_instantiate.instantiate subst ty
  end in
  {
    id = snd prop.sp_name;
    should_chown_privates = mro.mro_copy_private_members;
    elt = {
      ce_abstract = false;
      ce_final = true;
      ce_is_xhp_attr = prop.sp_is_xhp_attr;
      ce_const = prop.sp_const;
      ce_lateinit = prop.sp_lateinit;
      ce_override = false;
      ce_lsb = prop.sp_lsb;
      ce_memoizelsb = false;
      ce_synthesized = false;
      ce_visibility = visibility;
      ce_origin = mro.mro_name;
      ce_type = ty;
    }
  }

(** Given a linearization, pair each MRO element with its shallow class and its
    type parameter substitutions. Drop MRO elements for which we cannot find a
    shallow class (this should only happen in partial-mode files when the
    assume_php setting is enabled). *)
let get_shallow_classes_and_substs
    (lin: linearization)
    : (mro_element * shallow_class * decl subst) Sequence.t =
  Sequence.filter_map lin begin fun mro ->
    match Shallow_classes_heap.get mro.mro_name with
    | None -> None
    | Some cls ->
      let subst = Decl_subst.make cls.sc_tparams mro.mro_type_args in
      Some (mro, cls, subst)
  end

(** Return a linearization suitable for method lookup, where ancestors are
    included in the linearization only if the child class inherits their
    methods. *)
let filter_for_method_lookup lin =
  Sequence.filter lin ~f:(fun (mro, _, _) ->
    not mro.mro_xhp_attrs_only && not mro.mro_consts_only)

(** Return a linearization suitable for property lookup, where ancestors are
    included in the linearization only if the child class inherits their
    properties or XHP attributes. *)
let filter_for_prop_lookup lin =
  Sequence.filter lin ~f:(fun (mro, _, _) -> not mro.mro_consts_only)

module SPairSet = Reordered_argument_set(Caml.Set.Make(struct
  type t = string * string
  (* Equivalent to polymorphic compare; written explicitly for perf reasons *)
  let compare (a1, a2) (b1, b2) =
    let r = String.compare a1 b1 in
    if r <> 0 then r else String.compare a2 b2
end))

(** Given a linearization filtered for method lookup, return a [Sequence.t]
    containing each method (as an {!inheritable_elt}) in linearization order,
    with method redeclarations already handled (that is, methods arising from a
    redeclaration appear in the sequence as regular methods, and trait methods
    which were redeclared do not appear in the sequence). *)
let method_ielts ~static child_class_name lin =
  Sequence.unfold ~init:(SPairSet.empty, lin) ~f:begin fun (removed, seq) ->
    match Sequence.next seq with
    | None -> None
    | Some ((mro, cls, subst), rest) ->
      let methods = if static then cls.sc_static_methods else cls.sc_methods in
      let redecls = cls.sc_method_redeclarations in
      let redecls = List.filter ~f:(fun x -> x.smr_static = static) redecls in
      let methods_from_redecls = redecl_list_to_method_seq redecls in
      let cid = mro.mro_name in
      let methods_seq =
        Sequence.append (Sequence.of_list methods) methods_from_redecls
        |> Sequence.filter ~f:(fun sm -> not (SPairSet.mem removed (cid, snd sm.sm_name)))
        |> Sequence.map ~f:(shallow_method_to_ielt child_class_name mro subst)
      in
      (* "Remove" all trait methods which were redeclared. If we encounter any
         of these trait methods later in the linearization, just ignore them. *)
      let removed = List.fold redecls ~init:removed ~f:begin fun acc mr ->
        let _, trait, _ = Decl_utils.unwrap_class_hint mr.smr_trait in
        let _, trait_method = mr.smr_method in
        SPairSet.add acc (trait, trait_method)
      end in
      Some (methods_seq, (removed, rest))
  end
  |> Sequence.concat

(** Given a linearization filtered for property lookup, return a [Sequence.t]
    emitting each property (as an {!inheritable_elt}) in linearization order. *)
let prop_ielts ~static child_class_name lin =
  lin
  |> Sequence.map ~f:begin fun (mro, cls, subst) ->
    (if static then cls.sc_sprops else cls.sc_props)
    |> Sequence.of_list
    |> Sequence.map ~f:(shallow_prop_to_ielt child_class_name mro subst)
  end
  |> Sequence.concat

(** Return true if the element is private and not marked with the __LSB
    attribute. Private elements are not inherited by child classes and are
    namespaced to the containing class--if B extends A, then A may define a
    method A::foo and B may define a method B::foo, and they both will exist in
    the hierarchy and be callable at runtime (which method is invoked depends on
    the caller).

    The __LSB attribute can be applied to properties only. LSB properties are
    (effectively) implicitly cloned into every subclass. This means that in the
    typechecker, we want to avoid filtering them out in subclasses, so we treat
    them as non-private here. *)
let is_private elt =
  match elt.ce_visibility with
  | Vprivate _ when elt.ce_lsb -> false
  | Vprivate _ -> true
  | Vpublic | Vprotected _ -> false

let chown_private child_class_name ancestor_sig =
  let ce_visibility =
    match ancestor_sig.ce_visibility with
    | Vprivate _ -> Vprivate child_class_name
    | _ -> ancestor_sig.ce_visibility
  in
  { ancestor_sig with ce_visibility }

(** Remove private members not visible to [child_class_name] from the sequence.
    Mark private trait members as private to [child_class_name] instead. *)
let filter_or_chown_privates
    (child_class_name: string)
    (lin: inheritable_elt Sequence.t)
    : (string * class_elt) Sequence.t =
  Sequence.filter_map lin begin fun {id; should_chown_privates; elt} ->
    let ancestor_name = elt.ce_origin in
    let is_private_and_inherited =
      ancestor_name <> child_class_name && is_private elt
    in
    if is_private_and_inherited && not should_chown_privates
    then None
    else
      if is_private_and_inherited && should_chown_privates
      then Some (id, chown_private child_class_name elt)
      else Some (id, elt)
  end

(* NB: Update [is_elt_canonical] below when changing this function. *)
let should_use_ancestor_sig child_class_name descendant_sig ancestor_sig =
  (* Any member directly declared in the child class overrides ancestor members,
     even if it is abstract and an ancestor member is concrete. *)
  child_class_name <> descendant_sig.ce_origin &&
  (* Otherwise, concrete members take priority over abstract members. *)
  not ancestor_sig.ce_abstract && descendant_sig.ce_abstract

(* NB: Update [is_elt_canonical] below when changing this function. *)
let update_descendant_vis descendant_sig ancestor_sig =
  let descendant_vis = descendant_sig.ce_visibility in
  let ancestor_vis = ancestor_sig.ce_visibility in
  (* If both are protected, use the ancestor's origin. *)
  match descendant_vis, ancestor_vis with
  | Vprotected _, Vprotected _ ->
    { descendant_sig with ce_visibility = ancestor_vis }
  | _ -> descendant_sig

(* NB: Update [is_elt_canonical] below when changing this function. *)
let merge_elts child_class_name ~earlier:descendant_sig ~later:ancestor_sig =
  if should_use_ancestor_sig child_class_name descendant_sig ancestor_sig
  then ancestor_sig
  else update_descendant_vis descendant_sig ancestor_sig

(** NB: The correctness of is_elt_canonical depends upon the implementation of
    [merge] (and therefore also its helpers [should_use_ancestor_sig] and
    [update_descendant_vis]) above. We cannot consider any protected member
    canonical because [merge] may replace a descendant element via
    [update_descendent_vis]. Otherwise, we must only return [true] when
    [should_use_ancestor_sig] would *always* return [false] when comparing the
    given element against *any* possible ancestor version. *)
let is_elt_canonical child_class_name elt =
  match elt.ce_visibility with
  | Vprotected _ -> false
  | _ -> child_class_name = elt.ce_origin || not elt.ce_abstract

let make_elt_cache class_name seq =
  LSTable.make seq
    ~is_canonical:(is_elt_canonical class_name)
    ~merge:(merge_elts class_name)

let constructor_elt child_class_name (mro, cls, subst) =
  let consistent =
    if cls.sc_final then FinalClass else
    let consistent_attr_present =
      Attributes.mem
        SN.UserAttributes.uaConsistentConstruct
        cls.sc_user_attributes
    in
    if consistent_attr_present then ConsistentConstruct else Inconsistent
  in
  let elt =
    Option.map cls.sc_constructor
      ~f:(shallow_method_to_class_elt child_class_name mro subst)
  in
  elt, consistent

let fold_constructors child_class_name acc ancestor =
  let descendant_cstr, descendant_consist = acc in
  let ancestor_cstr, ancestor_consist = ancestor in
  let cstr =
    match descendant_cstr, ancestor_cstr with
    | None, _ -> ancestor_cstr
    | Some d, Some a when should_use_ancestor_sig child_class_name d a ->
      ancestor_cstr
    | _ -> descendant_cstr
  in
  cstr, Decl_utils.coalesce_consistent ancestor_consist descendant_consist

let get_all_methods ~static class_name lin =
  lin
  |> filter_for_method_lookup
  |> method_ielts ~static class_name
  |> filter_or_chown_privates class_name
  |> Sequence.memoize

let props_cache ~static class_name lin =
  lin
  |> filter_for_prop_lookup
  |> prop_ielts ~static class_name
  |> filter_or_chown_privates class_name
  |> make_elt_cache class_name

let construct class_name lin =
  lazy begin
    lin
    |> filter_for_method_lookup
    |> Sequence.map ~f:(constructor_elt class_name)
    |> Sequence.fold ~init:(None, Inconsistent) ~f:(fold_constructors class_name)
  end

let make class_name =
  let lin =
    Decl_linearize.get_linearization class_name
    |> get_shallow_classes_and_substs
    |> Sequence.memoize
  in

  let all_methods = get_all_methods class_name lin ~static:false in
  let all_smethods = get_all_methods class_name lin ~static:true in

  let methods = make_elt_cache class_name all_methods in
  let smethods = make_elt_cache class_name all_smethods in

  let make_inheritance_cache seq =
    let is_canonical _ = false in
    let merge ~earlier ~later = later @ earlier in
    seq
    |> Sequence.map ~f:(fun (id, x) -> id, [x])
    |> LSTable.make ~is_canonical ~merge
  in
  let all_inherited_methods = make_inheritance_cache all_methods in
  let all_inherited_smethods = make_inheritance_cache all_smethods in
  {
    props = props_cache class_name lin ~static:false;
    sprops = props_cache class_name lin ~static:true;
    methods;
    smethods;
    all_inherited_methods;
    all_inherited_smethods;
    construct = construct class_name lin;
  }
