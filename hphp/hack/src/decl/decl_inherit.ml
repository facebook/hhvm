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
open Decl_defs
open Shallow_decl_defs
open Typing_defs
module Inst = Decl_instantiate

(*****************************************************************************)
(* This is what we are trying to produce for a given class. *)
(*****************************************************************************)

type inherited = {
  ih_substs: subst_context SMap.t;
  ih_cstr: element option * consistent_kind;
  ih_consts: class_const SMap.t;
  ih_typeconsts: typeconst_type SMap.t;
  ih_pu_enums: pu_enum_type SMap.t;
  ih_props: element SMap.t;
  ih_sprops: element SMap.t;
  ih_methods: element SMap.t;
  ih_smethods: element SMap.t;
}

let empty =
  {
    ih_substs = SMap.empty;
    ih_cstr = (None, Inconsistent);
    ih_consts = SMap.empty;
    ih_typeconsts = SMap.empty;
    ih_pu_enums = SMap.empty;
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

let should_keep_old_sig sig_ old_sig =
  ((not old_sig.elt_abstract) && sig_.elt_abstract)
  || Bool.equal old_sig.elt_abstract sig_.elt_abstract
     && (not old_sig.elt_synthesized)
     && sig_.elt_synthesized

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
      SMap.add name { sig_ with elt_override = false } methods

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
    | (_, _, true, false) ->
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
    (match (old_sig.ttc_abstract, sig_.ttc_abstract) with
    (* This covers the following case
     *
     * interface I1 { abstract const type T; }
     * interface I2 { const type T = int; }
     *
     * class C implements I1, I2 {}
     *
     * Then C::T == I2::T since I2::T is not abstract
     *)
    | (TCConcrete, TCAbstract _)
    | (TCPartiallyAbstract, TCAbstract _) ->
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
    | (TCConcrete, TCPartiallyAbstract) -> typeconsts
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
    | (TCAbstract (Some _), TCAbstract None) -> typeconsts
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
      SMap.add name sig_ typeconsts)

let add_pu_enum name pu pu_enums =
  (* TODO(T36532263) deal with multiple inheritance *)
  SMap.add name pu pu_enums

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
    ih_pu_enums = SMap.fold add_pu_enum inherited.ih_pu_enums acc.ih_pu_enums;
    ih_props = add_members inherited.ih_props acc.ih_props;
    ih_sprops = add_members inherited.ih_sprops acc.ih_sprops;
    ih_methods = add_methods inherited.ih_methods acc.ih_methods;
    ih_smethods = add_methods inherited.ih_smethods acc.ih_smethods;
  }

let remove_trait_redeclared (methods, smethods) m =
  let (pos, trait, _) = Decl_utils.unwrap_class_hint m.smr_trait in
  let (_, trait_method) = m.smr_method in
  let remove_from map =
    match SMap.find_opt trait_method map with
    | Some decls ->
      let decls =
        List.filter ~f:(fun d -> String.( <> ) d.elt_origin trait) decls
      in
      SMap.add trait_method decls map
    | None ->
      Errors.redeclaring_missing_method pos trait_method;
      map
  in
  if m.smr_static then
    (methods, remove_from smethods)
  else
    (remove_from methods, smethods)

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
  let mark_elt elt = { elt with elt_synthesized = true } in
  {
    inh with
    ih_substs =
      SMap.map
        begin
          fun sc ->
          { sc with sc_from_req_extends = true }
        end
        inh.ih_substs;
    ih_cstr = (Option.map (fst inh.ih_cstr) mark_elt, snd inh.ih_cstr);
    ih_props = SMap.map mark_elt inh.ih_props;
    ih_sprops = SMap.map mark_elt inh.ih_sprops;
    ih_methods = SMap.map mark_elt inh.ih_methods;
    ih_smethods = SMap.map mark_elt inh.ih_smethods;
    ih_consts =
      SMap.map (fun const -> { const with cc_synthesized = true }) inh.ih_consts;
  }

(*****************************************************************************)
(* Code filtering the private members (useful for inheritance) *)
(*****************************************************************************)

let filter_privates class_type =
  let is_not_private _ elt =
    match elt.elt_visibility with
    | Vprivate _ when elt.elt_lsb -> true
    | Vprivate _ -> false
    | Vpublic
    | Vprotected _ ->
      true
  in
  {
    class_type with
    dc_props = SMap.filter is_not_private class_type.dc_props;
    dc_sprops = SMap.filter is_not_private class_type.dc_sprops;
    dc_methods = SMap.filter is_not_private class_type.dc_methods;
    dc_smethods = SMap.filter is_not_private class_type.dc_smethods;
  }

let chown_privates owner class_type =
  let chown_private elt =
    match elt.elt_visibility with
    | Vprivate _ -> { elt with elt_visibility = Vprivate owner }
    | Vpublic
    | Vprotected _ ->
      elt
  in
  {
    class_type with
    dc_props = SMap.map chown_private class_type.dc_props;
    dc_sprops = SMap.map chown_private class_type.dc_sprops;
    dc_methods = SMap.map chown_private class_type.dc_methods;
    dc_smethods = SMap.map chown_private class_type.dc_smethods;
  }

(*****************************************************************************)
(* Builds the inherited type when the class lives in Hack *)
(*****************************************************************************)

let inherit_hack_class env c class_name class_type argl =
  let subst = make_substitution class_type argl in
  let class_type =
    match class_type.dc_kind with
    | Ast_defs.Ctrait ->
      (* Change the private visibility to point to the inheriting class *)
      chown_privates (snd c.sc_name) class_type
    | Ast_defs.Cnormal
    | Ast_defs.Cabstract
    | Ast_defs.Cinterface ->
      filter_privates class_type
    | Ast_defs.Cenum -> class_type
  in
  let typeconsts =
    SMap.map (Inst.instantiate_typeconst subst) class_type.dc_typeconsts
  in
  let pu_enums = class_type.dc_pu_enums in
  let consts = SMap.map (Inst.instantiate_cc subst) class_type.dc_consts in
  let props = class_type.dc_props in
  let sprops = class_type.dc_sprops in
  let methods = class_type.dc_methods in
  let smethods = class_type.dc_smethods in
  let cstr = Decl_env.get_construct env class_type in
  let subst_ctx =
    {
      sc_subst = subst;
      sc_class_context = snd c.sc_name;
      sc_from_req_extends = false;
    }
  in
  let substs = SMap.add class_name subst_ctx class_type.dc_substs in
  let result =
    {
      ih_substs = substs;
      ih_cstr = cstr;
      ih_consts = consts;
      ih_typeconsts = typeconsts;
      ih_pu_enums = pu_enums;
      ih_props = props;
      ih_sprops = sprops;
      ih_methods = methods;
      ih_smethods = smethods;
    }
  in
  result

(* mostly copy paste of inherit_hack_class *)
let inherit_hack_class_constants_only class_type argl =
  let subst = make_substitution class_type argl in
  let instantiate = SMap.map (Inst.instantiate_cc subst) in
  let consts = instantiate class_type.dc_consts in
  let typeconsts =
    SMap.map (Inst.instantiate_typeconst subst) class_type.dc_typeconsts
  in
  let result = { empty with ih_consts = consts; ih_typeconsts = typeconsts } in
  result

(* This logic deals with importing XHP attributes from an XHP class
   via the "attribute :foo;" syntax. *)
let inherit_hack_xhp_attrs_only class_type =
  (* Filter out properties that are not XHP attributes *)
  let props =
    SMap.fold
      begin
        fun name prop acc ->
        if Option.is_some prop.elt_xhp_attr then
          SMap.add name prop acc
        else
          acc
      end
      class_type.dc_props
      SMap.empty
  in
  let result = { empty with ih_props = props } in
  result

(*****************************************************************************)

let from_class env c ty =
  let (_, (_, class_name), class_params) = Decl_utils.unwrap_class_type ty in
  let class_type = Decl_env.get_class_dep env class_name in
  match class_type with
  | None ->
    (* The class lives in PHP, we don't know anything about it *)
    empty
  | Some class_ ->
    (* The class lives in Hack *)
    inherit_hack_class env c class_name class_ class_params

(* mostly copy paste of from_class *)
let from_class_constants_only env ty =
  let (_, (_, class_name), class_params) = Decl_utils.unwrap_class_type ty in
  let class_type = Decl_env.get_class_dep env class_name in
  match class_type with
  | None ->
    (* The class lives in PHP, we don't know anything about it *)
    empty
  | Some class_ ->
    (* The class lives in Hack *)
    inherit_hack_class_constants_only class_ class_params

let from_class_xhp_attrs_only env ty =
  let (_, (_pos, class_name), _class_params) =
    Decl_utils.unwrap_class_type ty
  in
  let class_type = Decl_env.get_class_dep env class_name in
  match class_type with
  | None ->
    (* The class lives in PHP, we don't know anything about it *)
    empty
  | Some class_ ->
    (* The class lives in Hack *)
    inherit_hack_xhp_attrs_only class_

let from_parent env c =
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
  let inherited_l = List.map extends (from_class env c) in
  List.fold_right ~f:add_inherited inherited_l ~init:empty

let from_requirements env c acc reqs =
  let inherited = from_class env c reqs in
  let inherited = mark_as_synthesized inherited in
  add_inherited inherited acc

let from_trait env c (acc, methods, smethods) uses =
  let ({ ih_methods; ih_smethods; _ } as inherited) = from_class env c uses in
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

let from_xhp_attr_use env acc uses =
  let inherited = from_class_xhp_attrs_only env uses in
  add_inherited inherited acc

let from_interface_constants env acc impls =
  let inherited = from_class_constants_only env impls in
  add_inherited inherited acc

(*****************************************************************************)
(* The API to the outside *)
(*****************************************************************************)

let make env c =
  (* members inherited from parent class ... *)
  let acc = from_parent env c in
  let acc =
    List.fold_left ~f:(from_requirements env c) ~init:acc c.sc_req_extends
  in
  (* ... are overridden with those inherited from used traits *)
  let (acc, methods, smethods) =
    List.fold_left
      ~f:(from_trait env c)
      ~init:(acc, SMap.empty, SMap.empty)
      c.sc_uses
  in
  let (methods, smethods) =
    List.fold_left
      ~f:remove_trait_redeclared
      ~init:(methods, smethods)
      c.sc_method_redeclarations
  in
  let acc = collapse_trait_inherited methods smethods acc in
  let acc =
    List.fold_left ~f:(from_xhp_attr_use env) ~init:acc c.sc_xhp_attr_uses
  in
  (* todo: what about the same constant defined in different interfaces
   * we implement? We should forbid and say "constant already defined".
   * to julien: where is the logic that check for duplicated things?
   * todo: improve constant handling, see task #2487051
   *)
  let acc =
    List.fold_left
      ~f:(from_interface_constants env)
      ~init:acc
      c.sc_req_implements
  in
  List.fold_left ~f:(from_interface_constants env) ~init:acc c.sc_implements
