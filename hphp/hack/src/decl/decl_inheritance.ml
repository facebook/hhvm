(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Decl_defs
open Decl_subst
open Reordered_argument_collections
open Shallow_decl_defs
open Typing_defs
module DTT = Decl_to_typing
module LSTable = Lazy_string_table
module SN = Naming_special_names

type inherited_members = {
  consts: class_const LSTable.t;
  typeconsts: typeconst_type LSTable.t;
  props: class_elt LSTable.t;
  sprops: class_elt LSTable.t;
  methods: class_elt LSTable.t;
  smethods: class_elt LSTable.t;
  all_inherited_methods: class_elt list LSTable.t;
  all_inherited_smethods: class_elt list LSTable.t;
  typeconst_enforceability: (Pos_or_decl.t * bool) LSTable.t;
  construct: (class_elt option * consistent_kind) Lazy.t;
}

(** Are we targeting a particular member or all members. Different sequences
 * will be generated depending on the [target] *)
type target =
  | SingleMember of string
  | AllMembers

(** Given a linearization, pair each MRO element with its shallow class and its
    type parameter substitutions. Drop MRO elements for which we cannot find a
    shallow class (this should only happen in partial-mode files when the
    assume_php setting is enabled). *)
let get_shallow_classes_and_substs
    ~target (ctx : Provider_context.t) (lin : linearization) :
    (mro_element * shallow_class * decl_subst) Sequence.t =
  let lin =
    match target with
    | AllMembers -> lin
    | SingleMember member_name ->
      (* If we are targeting a specific member we utilize the member filters to
       * avoid loading [shallow_class_decl]s that do not contain a member with
       * the given name. *)
      let hash = BloomFilter.hash member_name in
      Sequence.filter lin ~f:(fun mro ->
          match Shallow_classes_provider.get_member_filter ctx mro.mro_name with
          | None -> true
          | Some filter -> BloomFilter.mem filter hash)
  in
  Sequence.filter_map lin (fun mro ->
      match Shallow_classes_provider.get ctx mro.mro_name with
      | None -> None
      | Some cls ->
        let subst = Decl_subst.make_decl cls.sc_tparams mro.mro_type_args in
        Some (mro, cls, subst))

(** Return a linearization suitable for method lookup, where ancestors are
    included in the linearization only if the child class inherits their
    methods. *)
let filter_for_method_lookup lin =
  Sequence.filter lin ~f:(fun mro ->
      (not (is_set mro_xhp_attrs_only mro.mro_flags))
      && not (is_set mro_consts_only mro.mro_flags))

(** Return a linearization suitable for property lookup, where ancestors are
    included in the linearization only if the child class inherits their
    properties or XHP attributes. *)
let filter_for_prop_lookup lin =
  Sequence.filter lin ~f:(fun mro -> not (is_set mro_consts_only mro.mro_flags))

(** Return a linearization suitable for lookup of class constants and type
    constants, where ancestors are included in the linearization only if the
    child class inherits their constants. *)
let filter_for_const_lookup lin =
  Sequence.filter lin ~f:(fun mro ->
      not (is_set mro_xhp_attrs_only mro.mro_flags))

module SPairSet = Reordered_argument_set (Caml.Set.Make (struct
  type t = string * string

  (* Equivalent to polymorphic compare; written explicitly for perf reasons *)
  let compare (a1, a2) (b1, b2) =
    let r = String.compare a1 b1 in
    if Int.( <> ) r 0 then
      r
    else
      String.compare a2 b2
end))

let filter_target ~target ~f list =
  match target with
  | AllMembers -> list
  | SingleMember s ->
    list |> List.find ~f:(fun x -> String.equal s (f x)) |> Option.to_list

(** Given a linearization filtered for method lookup, return a [Sequence.t]
    containing each method (as an {!tagged_elt}) in linearization order, with
    method redeclarations already handled (that is, methods arising from a
    redeclaration appear in the sequence as regular methods, and trait methods
    which were redeclared do not appear in the sequence). *)
let methods ~target ~static child_class_name lin =
  Sequence.unfold ~init:(SPairSet.empty, lin) ~f:(fun (removed, seq) ->
      match Sequence.next seq with
      | None -> None
      | Some ((mro, cls, subst), rest) ->
        let methods =
          if static then
            cls.sc_static_methods
          else
            cls.sc_methods
        in
        let cid = mro.mro_name in
        let methods_seq =
          methods
          |> filter_target ~target ~f:(fun { sm_name = (_, n); _ } -> n)
          |> Sequence.of_list
          |> Sequence.filter ~f:(fun sm ->
                 not (SPairSet.mem removed (cid, snd sm.sm_name)))
          |> Sequence.map
               ~f:(DTT.shallow_method_to_telt child_class_name mro subst)
        in
        Some (methods_seq, (removed, rest)))
  |> Sequence.concat

(** Given a linearization filtered for property lookup, return a [Sequence.t]
    emitting each property (as an {!tagged_elt}) in linearization order. *)
let props ~target ~static child_class_name lin =
  lin
  |> Sequence.map ~f:(fun (mro, cls, subst) ->
         ( if static then
           cls.sc_sprops
         else
           cls.sc_props )
         |> filter_target ~target ~f:(fun { sp_name = (_, n); _ } -> n)
         |> Sequence.of_list
         |> Sequence.map
              ~f:(DTT.shallow_prop_to_telt child_class_name mro subst))
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
  | Vprivate _ when get_ce_lsb elt -> false
  | Vprivate _ -> true
  | Vprotected _
  | Vpublic ->
    false

let is_private_or_protected elt =
  match elt.ce_visibility with
  | Vprivate _
  | Vprotected _
    when get_ce_lsb elt ->
    false
  | Vprivate _
  | Vprotected _ ->
    true
  | Vpublic -> false

let chown_private_or_protected child_class_name ancestor_sig =
  let ce_visibility =
    match ancestor_sig.ce_visibility with
    | Vprivate _ -> Vprivate child_class_name
    | Vprotected _ when not (get_ce_synthesized ancestor_sig) ->
      Vprotected child_class_name
    | _ -> ancestor_sig.ce_visibility
  in
  { ancestor_sig with ce_visibility }

(** Remove private members not visible to [child_class_name] from the sequence.
    Mark private/protected trait members as private/protected to [child_class_name] instead. *)
let filter_or_chown_privates
    (child_class_name : string) (lin : DTT.tagged_elt Sequence.t) :
    (string * class_elt) Sequence.t =
  Sequence.filter_map lin (fun DTT.{ id; inherit_when_private; elt } ->
      let ancestor_name = elt.ce_origin in
      let is_inherited = String.( <> ) ancestor_name child_class_name in
      if is_private elt && is_inherited && not inherit_when_private then
        None
      else if
        is_private_or_protected elt && is_inherited && inherit_when_private
      then
        Some (id, chown_private_or_protected child_class_name elt)
      else
        Some (id, elt))

(* NB: Update [is_elt_canonical] below when changing this function. *)
let should_use_ancestor_sig child_class_name descendant_sig ancestor_sig =
  (* Any member directly declared in the child class overrides ancestor members,
     even if it is abstract and an ancestor member is concrete. *)
  String.( <> ) child_class_name descendant_sig.ce_origin
  (* Otherwise, concrete members take priority over abstract members. *)
  && (not (get_ce_abstract ancestor_sig))
  && get_ce_abstract descendant_sig

(* NB: Update [is_elt_canonical] below when changing this function. *)
let update_descendant_vis descendant_sig ancestor_sig =
  let descendant_vis = descendant_sig.ce_visibility in
  let ancestor_vis = ancestor_sig.ce_visibility in
  (* If both are protected, use the ancestor's origin. *)
  match (descendant_vis, ancestor_vis) with
  | (Vprotected _, Vprotected _) ->
    { descendant_sig with ce_visibility = ancestor_vis }
  | _ -> descendant_sig

(* NB: Update [is_elt_canonical] below when changing this function. *)
let merge_elts child_class_name ~earlier:descendant_sig ~later:ancestor_sig =
  if should_use_ancestor_sig child_class_name descendant_sig ancestor_sig then
    ancestor_sig
  else
    update_descendant_vis descendant_sig ancestor_sig

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
  | _ ->
    String.equal child_class_name elt.ce_origin || not (get_ce_abstract elt)

let make_inheritance_cache seq =
  let is_canonical _ = false in
  let merge ~earlier ~later = later @ earlier in
  let seq target = seq target |> Sequence.map ~f:(fun (id, x) -> (id, [x])) in
  let get_single_seq target =
    seq (SingleMember target) |> Sequence.map ~f:snd
  in
  LSTable.make (seq AllMembers) ~get_single_seq ~is_canonical ~merge

let make_elt_cache class_name seq =
  let get_single_seq target =
    seq (SingleMember target) |> Sequence.map ~f:snd
  in
  LSTable.make
    (seq AllMembers)
    ~get_single_seq
    ~is_canonical:(is_elt_canonical class_name)
    ~merge:(merge_elts class_name)

(** Given a linearization filtered for const lookup, return a [Sequence.t]
    emitting each constant in linearization order. *)
let consts ~target ctx child_class_name get_typeconst get_ancestor lin =
  (* If a class extends the legacy [Enum] class, we give all of the constants
     in the class the type of the Enum, as a convenience. Modern Hack enums
     should replace the legacy Enum class, but perhaps they cannot do so
     without allowing subtyping (as legacy enums do).

     Detecting the legacy Enum as an ancestor is the reason that we need access
     to [get_ancestor] here. We then invoke [Decl_enum.rewrite_class_consts]
     below if the class does indeed extend [Enum].

     The [classname_const] here is the implicit constant [C::class], which has
     type [classname<C>]. *)
  let (classname_const, enum_kind) =
    match Shallow_classes_provider.get ctx child_class_name with
    | None -> (Sequence.empty, lazy None)
    | Some cls ->
      ( Sequence.singleton (DTT.classname_const cls.sc_name),
        lazy
          (Decl_enum.enum_kind
             cls.sc_name
             cls.sc_enum_type
             Option.(
               get_typeconst Naming_special_names.FB.tInner >>= fun t ->
               t.ttc_type)
             get_ancestor) )
  in
  let consts_and_typeconst_structures =
    lin
    |> Sequence.map ~f:(fun (mro, cls, subst) ->
           let consts =
             cls.sc_consts
             |> filter_target ~target ~f:(fun { scc_name = (_, n); _ } -> n)
             |> Sequence.of_list
             |> Sequence.map
                  ~f:
                    (DTT.shallow_const_to_class_const
                       child_class_name
                       mro
                       subst)
           in
           (* Each concrete type constant implicitly defines a class constant of the
         same name with that type's TypeStructure. *)
           let typeconst_structures =
             cls.sc_typeconsts
             |> filter_target ~target ~f:(fun { stc_name = (_, n); _ } -> n)
             |> Sequence.of_list
             |> Sequence.map ~f:(DTT.typeconst_structure mro (snd cls.sc_name))
           in
           Sequence.append consts typeconst_structures)
    |> Sequence.concat
    |> Decl_enum.rewrite_class_consts enum_kind
  in
  match target with
  | SingleMember const_name when String.equal const_name SN.Members.mClass ->
    classname_const
  | SingleMember _ -> consts_and_typeconst_structures
  | AllMembers ->
    Sequence.append classname_const consts_and_typeconst_structures

let make_consts_cache class_name lin =
  let get_single_seq target =
    lin (SingleMember target) |> Sequence.map ~f:snd
  in
  LSTable.make
    (lin AllMembers)
    ~get_single_seq
    ~is_canonical:(fun cc ->
      String.equal cc.cc_origin class_name || not cc.cc_abstract)
    ~merge:
      begin
        fun ~earlier ~later ->
        if String.equal earlier.cc_origin class_name then
          earlier
        else if not earlier.cc_abstract then
          earlier
        else if not later.cc_abstract then
          later
        else
          earlier
      end

(** Given a linearization filtered for const lookup, return a [Sequence.t]
    emitting each type constant in linearization order. *)
let typeconsts ~target child_class_name lin =
  lin
  |> Sequence.map ~f:(fun (mro, cls, subst) ->
         cls.sc_typeconsts
         |> filter_target ~target ~f:(fun { stc_name = (_, n); _ } -> n)
         |> Sequence.of_list
         |> Sequence.map
              ~f:
                (DTT.shallow_typeconst_to_typeconst_type
                   child_class_name
                   mro
                   subst))
  |> Sequence.concat

let make_typeconst_cache class_name lin =
  let get_single_seq target =
    lin (SingleMember target) |> Sequence.map ~f:snd
  in
  LSTable.make
    (lin AllMembers)
    ~get_single_seq
    ~is_canonical:(fun t ->
      String.equal t.ttc_origin class_name
      || equal_typeconst_abstract_kind t.ttc_abstract TCConcrete
         && not t.ttc_concretized)
    ~merge:
      begin
        fun ~earlier:descendant_tc ~later:ancestor_tc ->
        match (descendant_tc.ttc_abstract, ancestor_tc.ttc_abstract) with
        (* This covers the following case:
         *
         * interface I1 { abstract const type T; }
         * interface I2 { const type T = int; }
         *
         * class C implements I2, I1 {}
         *
         * Then C::T == I2::T since I2::T is not abstract.
         *)
        | (TCAbstract _, (TCConcrete | TCPartiallyAbstract)) -> ancestor_tc
        (* NB: The following comment (written in D1825740) claims that this arm is
         necessary to cover the example it describes. But this example does not
         exercise this arm--the interface appears earlier in the linearization
         than the abstract class, so the descendant typeconst is the concrete
         one. Furthermore, returning [descendant_tc] rather than [ancestor_tc]
         from this arm does not cause any of our typecheck tests to fail.

         I have left it to avoid possibly introducing a subtle behavioral change
         compared to eager decl. We are planning to remove partially-abstract
         type constants in any case, so this arm will be removed soon.
      *)
        (* This covers the following case:
         *
         * abstract class P { const type T as arraykey = arraykey; }
         * interface I { const type T = int; }
         *
         * class C extends P implements I {}
         *
         * Then C::T == I::T since P::T has a constraint and thus can be
         * overridden by its child, while I::T cannot be overridden.
         *)
        | (TCPartiallyAbstract, TCConcrete) -> ancestor_tc
        (* This covers the following case
         *
         * interface I {
         *   abstract const type T as arraykey;
         * }
         *
         * abstract class A {
         *   abstract const type T as arraykey = string;
         * }
         *
         * final class C extends A implements I {}
         *
         * C::T must come from A, not I, as A provides the default that will synthesize
         * into a concrete type constant in C.
         *)
        | (TCAbstract None, TCAbstract (Some _)) -> ancestor_tc
        (* When a type constant is declared in multiple parents we need to make a
         * subtle choice of what type we inherit. For example, in:
         *
         * interface I1 { abstract const type t as Container<int>; }
         * interface I2 { abstract const type t as KeyedContainer<int, int>; }
         * abstract class C implements I1, I2 {}
         *
         * Depending on the order the interfaces are declared, we may report an
         * error. Since this could be confusing there is special logic in
         * Typing_extends that checks for this potentially ambiguous situation
         * and requires the user to explicitly declare T in C.
         *)
        | (TCAbstract _, TCAbstract _)
        | (TCPartiallyAbstract, (TCAbstract _ | TCPartiallyAbstract))
        | (TCConcrete, (TCAbstract _ | TCPartiallyAbstract)) ->
          descendant_tc
        | (TCConcrete, TCConcrete) ->
          if descendant_tc.ttc_concretized && not ancestor_tc.ttc_concretized
          then
            ancestor_tc
          else
            descendant_tc
      end

let make_typeconst_enforceability_cache lin =
  let get_single_seq target =
    lin (SingleMember target)
    |> Sequence.map ~f:(fun (_name, ttc) -> ttc.ttc_enforceable)
  in
  LSTable.make
    ( lin AllMembers
    |> Sequence.map ~f:(fun (name, ttc) -> (name, ttc.ttc_enforceable)) )
    ~get_single_seq
    ~is_canonical:(fun (_pos, enforceable) -> enforceable)
    ~merge:(fun ~earlier ~later ->
      if snd later then
        later
      else
        earlier)

let constructor_elt child_class_name (mro, cls, subst) =
  let consistent = Decl_utils.consistent_construct_kind cls in
  let elt =
    Option.map
      cls.sc_constructor
      ~f:(DTT.shallow_method_to_class_elt child_class_name mro subst)
  in
  (elt, consistent)

let fold_constructors child_class_name acc ancestor =
  let (descendant_cstr, descendant_consist) = acc in
  let (ancestor_cstr, ancestor_consist) = ancestor in
  let cstr =
    match (descendant_cstr, ancestor_cstr) with
    | (None, _) -> ancestor_cstr
    | (Some d, Some a) when should_use_ancestor_sig child_class_name d a ->
      ancestor_cstr
    | _ -> descendant_cstr
  in
  (cstr, Decl_utils.coalesce_consistent ancestor_consist descendant_consist)

let get_all_methods ~static ctx class_name lin target =
  lin
  |> filter_for_method_lookup
  |> get_shallow_classes_and_substs ~target ctx
  |> methods ~target ~static class_name
  |> filter_or_chown_privates class_name

let props_cache ~static ctx class_name lin =
  (fun target ->
    lin
    |> filter_for_prop_lookup
    |> get_shallow_classes_and_substs ~target ctx
    |> props ~target ~static class_name
    |> filter_or_chown_privates class_name)
  |> make_elt_cache class_name

let consts_cache ctx class_name get_typeconst get_ancestor lin =
  (fun target ->
    lin
    |> filter_for_const_lookup
    |> get_shallow_classes_and_substs ~target ctx
    |> consts ~target ctx class_name get_typeconst get_ancestor)
  |> make_consts_cache class_name

let get_all_typeconsts ctx class_name lin target =
  lin
  |> filter_for_const_lookup
  |> get_shallow_classes_and_substs ~target ctx
  |> typeconsts ~target class_name

let construct ctx class_name lin =
  lazy
    ( lin
    |> filter_for_method_lookup
    |> get_shallow_classes_and_substs
         ~target:(SingleMember SN.Members.__construct)
         ctx
    |> Sequence.map ~f:(constructor_elt class_name)
    |> Sequence.fold
         ~init:(None, Inconsistent)
         ~f:(fold_constructors class_name) )

let make ctx class_name lin get_ancestor =
  let all_methods = get_all_methods ctx class_name lin ~static:false in
  let all_smethods = get_all_methods ctx class_name lin ~static:true in
  let all_typeconsts = get_all_typeconsts ctx class_name lin in
  let typeconsts = make_typeconst_cache class_name all_typeconsts in
  let typeconst_enforceability =
    make_typeconst_enforceability_cache all_typeconsts
  in
  {
    consts =
      consts_cache ctx class_name (LSTable.get typeconsts) get_ancestor lin;
    typeconsts;
    props = props_cache ctx class_name lin ~static:false;
    sprops = props_cache ctx class_name lin ~static:true;
    methods = make_elt_cache class_name all_methods;
    smethods = make_elt_cache class_name all_smethods;
    all_inherited_methods = make_inheritance_cache all_methods;
    all_inherited_smethods = make_inheritance_cache all_smethods;
    typeconst_enforceability;
    construct = construct ctx class_name lin;
  }
