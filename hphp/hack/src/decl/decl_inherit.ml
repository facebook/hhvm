(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Module dealing with inheritance.
 * When we want to declare a new class, we first have to retrieve all the
 * types that were inherited from their parents.
 *)
(*****************************************************************************)

open Hh_prelude
open Option.Monad_infix
open Decl_defs
open Shallow_decl_defs
open Typing_defs
module Inst = Decl_instantiate

(*****************************************************************************)
(* This is what we are trying to produce for a given class. *)
(*****************************************************************************)

type inherited = {
  ih_substs: subst_context SMap.t;
  ih_cstr: (element * fun_elt option) option * consistent_kind;
  ih_consts: class_const SMap.t;
  ih_typeconsts: typeconst_type SMap.t;
  ih_props: (element * decl_ty option) SMap.t;
  ih_sprops: (element * decl_ty option) SMap.t;
  ih_methods: (element * fun_elt option) SMap.t;
  ih_smethods: (element * fun_elt option) SMap.t;
}

let empty =
  {
    ih_substs = SMap.empty;
    ih_cstr = (None, Inconsistent);
    ih_consts = SMap.empty;
    ih_typeconsts = SMap.empty;
    ih_props = SMap.empty;
    ih_sprops = SMap.empty;
    ih_methods = SMap.empty;
    ih_smethods = SMap.empty;
  }

(*****************************************************************************)
(* Functions used to merge an additional inherited class to the types
 * we already inherited.
 *)
(*****************************************************************************)

let should_keep_old_sig (sig_, _) (old_sig, _) =
  ((not (get_elt_abstract old_sig)) && get_elt_abstract sig_)
  || Bool.equal (get_elt_abstract old_sig) (get_elt_abstract sig_)
     && (not (get_elt_synthesized old_sig))
     && get_elt_synthesized sig_

let add_method name sig_ methods =
  match SMap.find_opt name methods with
  | None ->
    (* The method didn't exist so far, let's add it *)
    SMap.add name sig_ methods
  | Some old_sig ->
    if
      should_keep_old_sig sig_ old_sig
      (* The then-branch of this if is encountered when the method being
       * added shouldn't *actually* be added. When's that?
       * In isolation, we can say that
       *   - We don't want to override a concrete method with
       *     an abstract one.
       *   - We don't want to override a method that's actually
       *     implemented by the programmer with one that's "synthetic",
       *     e.g. arising merely from a require-extends declaration in
       *     a trait.
       * When these two considerations conflict, we give precedence to
       * abstractness for determining priority of the method.
       *)
    then
      methods
    (* Otherwise, we *are* overwriting a method definition. This is
     * OK when a naming conflict is parent class vs trait (trait
     * wins!), but not really OK when the naming conflict is trait vs
     * trait (we rely on HHVM to catch the error at runtime) *)
    else
      let sig_ =
        Tuple.T2.map_fst sig_ ~f:(fun elt -> set_elt_override elt false)
      in
      SMap.add name sig_ methods

let add_methods methods' acc = SMap.fold add_method methods' acc

let add_const name const acc =
  match SMap.find_opt name acc with
  | None -> SMap.add name const acc
  | Some existing_const ->
    (match
       ( const.cc_synthesized,
         existing_const.cc_synthesized,
         const.cc_abstract,
         existing_const.cc_abstract )
     with
    | (true, false, _, _) ->
      (* Don't replace a constant with a synthesized constant. This
         covers the following case:

         class HasFoo { abstract const int FOO; }
         trait T { require extends Foo; }
         class Child extends HasFoo {
            use T;
         }

         In this case, Child still doesn't have a value for the FOO
         constant. *)
      acc
    | (_, _, CCAbstract false, CCAbstract true)
    | (_, _, CCAbstract _, CCConcrete) ->
      (* Don't replace a concrete constant with an abstract constant
           found later in the MRO.*)
      acc
    | (_, _, _, _) -> SMap.add name const acc)

let add_members members acc = SMap.fold SMap.add members acc

let add_typeconst name sig_ typeconsts =
  match SMap.find_opt name typeconsts with
  | None ->
    (* The type constant didn't exist so far, let's add it *)
    SMap.add name sig_ typeconsts
  | Some old_sig ->
    let typeconsts =
      (* Boolean OR for the second element of the tuple. If some typeconst in
         some ancestor was enforceable, then the child class' typeconst will be
         enforceable too, even if we didn't take that ancestor typeconst. *)
      if snd sig_.ttc_enforceable && not (snd old_sig.ttc_enforceable) then
        SMap.add
          name
          { old_sig with ttc_enforceable = sig_.ttc_enforceable }
          typeconsts
      else
        typeconsts
    in
    (match (old_sig.ttc_kind, sig_.ttc_kind) with
    (* This covers the following case
     *
     * interface I1 { abstract const type T; }
     * interface I2 { const type T = int; }
     *
     * class C implements I1, I2 {}
     *
     * Then C::T == I2::T since I2::T is not abstract
     *)
    | (TCConcrete _, TCAbstract _)
    | (TCPartiallyAbstract _, TCAbstract _) ->
      typeconsts
    (* This covers the following case
     *
     * abstract P { const type T as arraykey = arraykey; }
     * interface I { const type T = int; }
     *
     * class C extends P implements I {}
     *
     * Then C::T == I::T since P::T has a constraint and thus can be overridden
     * by it's child, while I::T cannot be overridden.
     *)
    | (TCConcrete _, TCPartiallyAbstract _) -> typeconsts
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
    | ( TCAbstract { atc_default = Some _; _ },
        TCAbstract { atc_default = None; _ } ) ->
      typeconsts
    | (_, _) ->
      (* When a type constant is declared in multiple parents we need to make a
       * subtle choice of what type we inherit. For example in:
       *
       * interface I1 { abstract const type t as Container<int>; }
       * interface I2 { abstract const type t as KeyedContainer<int, int>; }
       * abstract class C implements I1, I2 {}
       *
       * Depending on the order the interfaces are declared, we may report an error.
       * Since this could be confusing there is special logic in Typing_extends that
       * checks for this potentially ambiguous situation and warns the programmer to
       * explicitly declare T in C.
       *)
      let sig_ =
        (* Boolean OR for the second element of the tuple. If a typeconst we
           already inherited from some other ancestor was enforceable, then the
           one we inherit here will be enforceable too. *)
        if snd old_sig.ttc_enforceable && not (snd sig_.ttc_enforceable) then
          { sig_ with ttc_enforceable = old_sig.ttc_enforceable }
        else
          sig_
      in
      SMap.add name sig_ typeconsts)

let add_constructor (cstr, cstr_consist) (acc, acc_consist) =
  let ce =
    match (cstr, acc) with
    | (None, _) -> acc
    | (Some ce, Some acce) when should_keep_old_sig ce acce -> acc
    | _ -> cstr
  in
  (ce, Decl_utils.coalesce_consistent acc_consist cstr_consist)

let add_inherited inherited acc =
  {
    ih_substs =
      SMap.merge
        begin
          fun _ sub old_sub ->
          match (sub, old_sub) with
          | (None, None) -> None
          | (Some s, None)
          | (None, Some s) ->
            Some s
          (* If the old subst_contexts came via required extends then we want to use
           * the substitutios from the actual extends instead. I.e.
           *
           * class Base<+T> {}
           *
           * trait MyTrait { require extends Base<mixed>; }
           *
           * class Child extends Base<int> { use MyTrait; }
           *
           * Here the subst_context (MyTrait/[T -> mixed]) should be overrode by
           * (Child/[T -> int]), because it's the actual extension of class Base.
           *)
          | (Some s, Some old_s)
            when old_s.sc_from_req_extends && not s.sc_from_req_extends ->
            Some s
          | (Some _, Some old_s) -> Some old_s
        end
        acc.ih_substs
        inherited.ih_substs;
    ih_cstr = add_constructor inherited.ih_cstr acc.ih_cstr;
    ih_consts = SMap.fold add_const inherited.ih_consts acc.ih_consts;
    ih_typeconsts =
      SMap.fold add_typeconst inherited.ih_typeconsts acc.ih_typeconsts;
    ih_props = add_members inherited.ih_props acc.ih_props;
    ih_sprops = add_members inherited.ih_sprops acc.ih_sprops;
    ih_methods = add_methods inherited.ih_methods acc.ih_methods;
    ih_smethods = add_methods inherited.ih_smethods acc.ih_smethods;
  }

let collapse_trait_inherited methods smethods acc =
  let collapse_methods name sigs acc =
    (* fold_right because when traits get considered in order
     * T1, T2, T3, the list will be built up as [T3::f; T2::f; T1::f],
     * so this way we still call add_method in the declared order *)
    List.fold_right sigs ~f:(add_method name) ~init:acc
  in
  {
    acc with
    ih_methods = SMap.fold collapse_methods methods acc.ih_methods;
    ih_smethods = SMap.fold collapse_methods smethods acc.ih_smethods;
  }

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let make_substitution class_type class_parameters =
  Inst.make_subst class_type.dc_tparams class_parameters

let mark_as_synthesized inh =
  let mark_elt elt =
    Tuple.T2.map_fst elt ~f:(fun elt -> set_elt_synthesized elt true)
  in
  {
    ih_substs =
      SMap.map
        begin
          fun sc ->
          { sc with sc_from_req_extends = true }
        end
        inh.ih_substs;
    ih_cstr = (Option.map (fst inh.ih_cstr) ~f:mark_elt, snd inh.ih_cstr);
    ih_props = SMap.map mark_elt inh.ih_props;
    ih_sprops = SMap.map mark_elt inh.ih_sprops;
    ih_methods = SMap.map mark_elt inh.ih_methods;
    ih_smethods = SMap.map mark_elt inh.ih_smethods;
    ih_typeconsts =
      SMap.map
        (fun const -> { const with ttc_synthesized = true })
        inh.ih_typeconsts;
    ih_consts =
      SMap.map (fun const -> { const with cc_synthesized = true }) inh.ih_consts;
  }

(*****************************************************************************)
(* Code filtering the private members (useful for inheritance) *)
(*****************************************************************************)

let filter_privates class_type =
  let is_not_private _ elt =
    match elt.elt_visibility with
    | Vprivate _ when get_elt_lsb elt -> true
    | Vprivate _ -> false
    | Vpublic
    | Vprotected _
    | Vinternal _ ->
      true
  in
  {
    class_type with
    dc_props = SMap.filter is_not_private class_type.dc_props;
    dc_sprops = SMap.filter is_not_private class_type.dc_sprops;
    dc_methods = SMap.filter is_not_private class_type.dc_methods;
    dc_smethods = SMap.filter is_not_private class_type.dc_smethods;
  }

let chown_private_and_protected owner class_type =
  let chown elt =
    match elt.elt_visibility with
    | Vprivate _ -> { elt with elt_visibility = Vprivate owner }
    (* Update protected member that's included via a `use` statement, unless
     * it's synthesized, in which case its owner will be the one specified
     * in `requires extends` or `requires implements`
     *)
    | Vprotected _ when not (get_elt_synthesized elt) ->
      { elt with elt_visibility = Vprotected owner }
    | Vpublic
    | Vprotected _
    | Vinternal _ ->
      elt
  in
  {
    class_type with
    dc_props = SMap.map chown class_type.dc_props;
    dc_sprops = SMap.map chown class_type.dc_sprops;
    dc_methods = SMap.map chown class_type.dc_methods;
    dc_smethods = SMap.map chown class_type.dc_smethods;
  }

(*****************************************************************************)
(* Builds the inherited type when the class lives in Hack *)
(*****************************************************************************)

let pair_with_heap_entries :
    type heap_entry.
    element SMap.t ->
    heap_entry SMap.t option ->
    (element * heap_entry option) SMap.t =
 fun elts heap_entries ->
  SMap.mapi (fun name elt -> (elt, heap_entries >>= SMap.find_opt name)) elts

let inherit_hack_class
    env
    child
    parent_name
    parent
    argl
    (parent_members : Decl_store.class_members option) : inherited =
  let subst = make_substitution parent argl in
  let parent =
    match parent.dc_kind with
    | Ast_defs.Ctrait ->
      (* Change the private/protected visibility to point to the inheriting class *)
      chown_private_and_protected (snd child.sc_name) parent
    | Ast_defs.Cnormal
    | Ast_defs.Cabstract
    | Ast_defs.Cinterface ->
      filter_privates parent
    | Ast_defs.Cenum -> parent
  in
  let typeconsts =
    SMap.map (Inst.instantiate_typeconst_type subst) parent.dc_typeconsts
  in
  let consts = SMap.map (Inst.instantiate_cc subst) parent.dc_consts in
  let (cstr, constructor_consistency) = Decl_env.get_construct env parent in
  let subst_ctx =
    {
      sc_subst = subst;
      sc_class_context = snd child.sc_name;
      sc_from_req_extends = false;
    }
  in
  let substs = SMap.add parent_name subst_ctx parent.dc_substs in
  let result =
    {
      ih_substs = substs;
      ih_cstr =
        ( Option.map cstr ~f:(fun cstr ->
              let constructor_heap_value =
                parent_members >>= fun m -> m.Decl_store.m_constructor
              in
              (cstr, constructor_heap_value)),
          constructor_consistency );
      ih_consts = consts;
      ih_typeconsts = typeconsts;
      ih_props =
        pair_with_heap_entries
          parent.dc_props
          (parent_members >>| fun m -> m.Decl_store.m_properties);
      ih_sprops =
        pair_with_heap_entries
          parent.dc_sprops
          (parent_members >>| fun m -> m.Decl_store.m_static_properties);
      ih_methods =
        pair_with_heap_entries
          parent.dc_methods
          (parent_members >>| fun m -> m.Decl_store.m_methods);
      ih_smethods =
        pair_with_heap_entries
          parent.dc_smethods
          (parent_members >>| fun m -> m.Decl_store.m_static_methods);
    }
  in
  result

(* mostly copy paste of inherit_hack_class *)
let inherit_hack_class_constants_only class_type argl _parent_members =
  let subst = make_substitution class_type argl in
  let instantiate = SMap.map (Inst.instantiate_cc subst) in
  let consts = instantiate class_type.dc_consts in
  let typeconsts =
    SMap.map (Inst.instantiate_typeconst_type subst) class_type.dc_typeconsts
  in
  let result = { empty with ih_consts = consts; ih_typeconsts = typeconsts } in
  result

(* This logic deals with importing XHP attributes from an XHP class
   via the "attribute :foo;" syntax. *)
let inherit_hack_xhp_attrs_only class_type members =
  (* Filter out properties that are not XHP attributes *)
  let props =
    SMap.fold
      begin
        fun name prop acc ->
        if Option.is_some (get_elt_xhp_attr prop) then
          SMap.add name prop acc
        else
          acc
      end
      class_type.dc_props
      SMap.empty
  in
  let result =
    {
      empty with
      ih_props =
        pair_with_heap_entries
          props
          (members >>| fun m -> m.Decl_store.m_properties);
    }
  in
  result

(*****************************************************************************)

(** Return all the heap entries for a class names, i.e. the entry from the class heap
    and optionally all the entries from the property and method heaps.
    [classes] acts as a cache for entries we already have at hand.
    Also add dependency to that class. *)
let heap_entries env class_name (classes : Decl_store.class_entries SMap.t) :
    Decl_store.class_entries option =
  match SMap.find_opt class_name classes with
  | Some (class_, _) as heap_entries ->
    if not (Pos_or_decl.is_hhi class_.dc_pos) then
      Decl_env.add_extends_dependency env class_name;
    heap_entries
  | None ->
    (match Decl_store.((get ()).get_class class_name) with
    | None ->
      Decl_env.add_extends_dependency env class_name;
      None
    | Some class_ ->
      if not (Pos_or_decl.is_hhi class_.dc_pos) then
        Decl_env.add_extends_dependency env class_name;
      Some (class_, None))

(* Include definitions inherited from a class (extends) or a trait (use)
 * or requires extends
 *)
let from_class env c (parents : Decl_store.class_entries SMap.t) parent_ty :
    inherited =
  let (_, (_, parent_name), parent_class_params) =
    Decl_utils.unwrap_class_type parent_ty
  in
  match heap_entries env parent_name parents with
  | None ->
    (* The class lives in PHP, we don't know anything about it *)
    empty
  | Some (class_, parent_members) ->
    (* The class lives in Hack *)
    inherit_hack_class
      env
      c
      parent_name
      class_
      parent_class_params
      parent_members

let from_class_constants_only env (parents : Decl_store.class_entries SMap.t) ty
    =
  let (_, (_, class_name), class_params) = Decl_utils.unwrap_class_type ty in
  match heap_entries env class_name parents with
  | None ->
    (* The class lives in PHP, we don't know anything about it *)
    empty
  | Some (class_, parent_members) ->
    (* The class lives in Hack *)
    inherit_hack_class_constants_only class_ class_params parent_members

let from_class_xhp_attrs_only env (parents : Decl_store.class_entries SMap.t) ty
    =
  let (_, (_pos, class_name), _class_params) =
    Decl_utils.unwrap_class_type ty
  in
  match heap_entries env class_name parents with
  | None ->
    (* The class lives in PHP, we don't know anything about it *)
    empty
  | Some (class_, parent_members) ->
    (* The class lives in Hack *)
    inherit_hack_xhp_attrs_only class_ parent_members

let from_parent env c (parents : Decl_store.class_entries SMap.t) =
  let extends =
    (* In an abstract class or a trait, we assume the interfaces
     * will be implemented in the future, so we take them as
     * part of the class (as requested by dependency injection implementers)
     *)
    match c.sc_kind with
    | Ast_defs.Cabstract -> c.sc_implements @ c.sc_extends
    | Ast_defs.Ctrait -> c.sc_implements @ c.sc_extends @ c.sc_req_implements
    | _ -> c.sc_extends
  in
  let inherited_l = List.map extends ~f:(from_class env c parents) in
  List.fold_right ~f:add_inherited inherited_l ~init:empty

let from_requirements env c parents acc reqs =
  let inherited = from_class env c parents reqs in
  let inherited = mark_as_synthesized inherited in
  add_inherited inherited acc

let from_trait env c parents (acc, methods, smethods) uses =
  let ({ ih_methods; ih_smethods; _ } as inherited) =
    from_class env c parents uses
  in
  let inherited =
    { inherited with ih_methods = SMap.empty; ih_smethods = SMap.empty }
  in
  let extend_methods name sig_ methods =
    let sigs = Option.value ~default:[] (SMap.find_opt name methods) in
    SMap.add name (sig_ :: sigs) methods
  in
  let methods = SMap.fold extend_methods ih_methods methods in
  let smethods = SMap.fold extend_methods ih_smethods smethods in
  (add_inherited inherited acc, methods, smethods)

let from_xhp_attr_use env (parents : Decl_store.class_entries SMap.t) acc uses =
  let inherited = from_class_xhp_attrs_only env parents uses in
  add_inherited inherited acc

let from_interface_constants
    env (parents : Decl_store.class_entries SMap.t) acc impls =
  let inherited = from_class_constants_only env parents impls in
  add_inherited inherited acc

(*****************************************************************************)
(* The API to the outside *)
(*****************************************************************************)

let make env c ~cache:(parents : Decl_store.class_entries SMap.t) =
  (* members inherited from parent class ... *)
  let (acc : inherited) = from_parent env c parents in
  let acc =
    List.fold_left
      ~f:(from_requirements env c parents)
      ~init:acc
      c.sc_req_extends
  in
  (* ... are overridden with those inherited from used traits *)
  let (acc, methods, smethods) =
    List.fold_left
      ~f:(from_trait env c parents)
      ~init:(acc, SMap.empty, SMap.empty)
      c.sc_uses
  in
  let acc = collapse_trait_inherited methods smethods acc in
  let acc =
    List.fold_left
      ~f:(from_xhp_attr_use env parents)
      ~init:acc
      c.sc_xhp_attr_uses
  in
  (* todo: what about the same constant defined in different interfaces
   * we implement? We should forbid and say "constant already defined".
   * to julien: where is the logic that check for duplicated things?
   * todo: improve constant handling, see task #2487051
   *)
  let acc =
    List.fold_left
      ~f:(from_interface_constants env parents)
      ~init:acc
      c.sc_req_implements
  in
  let included_enums =
    match c.sc_enum_type with
    | None -> []
    | Some enum_type -> enum_type.te_includes
  in
  let acc =
    List.fold_left
      ~f:(from_interface_constants env parents)
      ~init:acc
      included_enums
  in
  List.fold_left
    ~f:(from_interface_constants env parents)
    ~init:acc
    c.sc_implements
