(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


(*****************************************************************************)
(* Module dealing with inheritance.
 * When we want to declare a new class, we first have to retrieve all the
 * types that were inherited from their parents.
 *)
(*****************************************************************************)

open Hh_core
open Decl_defs
open Nast
open Typing_defs

module Inst = Decl_instantiate

(*****************************************************************************)
(* This is what we are trying to produce for a given class. *)
(*****************************************************************************)

type inherited = {
  ih_substs   : subst_context SMap.t;
  ih_cstr     : element option * bool (* consistency required *);
  ih_consts   : class_const SMap.t ;
  ih_typeconsts : typeconst_type SMap.t ;
  ih_props    : element SMap.t ;
  ih_sprops   : element SMap.t ;
  ih_methods  : element SMap.t ;
  ih_smethods : element SMap.t ;
}

let empty = {
  ih_substs   = SMap.empty;
  ih_cstr     = None, false;
  ih_consts   = SMap.empty;
  ih_typeconsts = SMap.empty;
  ih_props    = SMap.empty;
  ih_sprops   = SMap.empty;
  ih_methods  = SMap.empty;
  ih_smethods = SMap.empty;
}

(*****************************************************************************)
(* Functions used to merge an additional inherited class to the types
 * we already inherited.
 *)
(*****************************************************************************)

let should_keep_old_sig sig_ old_sig =
  (not (old_sig.elt_abstract) && sig_.elt_abstract)
  || (old_sig.elt_abstract = sig_.elt_abstract
     && not (old_sig.elt_synthesized) && sig_.elt_synthesized)

let add_method name sig_ methods =
  match SMap.get name methods with
  | None ->
    (* The method didn't exist so far, let's add it *)
    SMap.add name sig_ methods
  | Some old_sig ->
    if should_keep_old_sig sig_ old_sig
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
    then methods

    (* Otherwise, we *are* overwriting a method definition. This is
     * OK when a naming conflict is parent class vs trait (trait
     * wins!), but not really OK when the naming conflict is trait vs
     * trait (we rely on HHVM to catch the error at runtime) *)
    else SMap.add name {sig_ with elt_override = false} methods

let add_methods methods' acc =
  SMap.fold add_method methods' acc

let add_const name const acc =
  match SMap.get name acc with
  | None ->
    SMap.add name const acc
  | Some existing_const ->
    match (const.cc_abstract, existing_const.cc_abstract) with
    | true, true ->
      SMap.add name const acc
    | true, _ ->
      acc
    | _, _ ->
      SMap.add name const acc

let add_members members acc =
  SMap.fold SMap.add members acc

let is_abstract_typeconst x = x.ttc_type = None

let can_override_typeconst x =
  (is_abstract_typeconst x) || x.ttc_constraint <> None

let add_typeconst name sig_ typeconsts =
  match SMap.get name typeconsts with
  | None ->
      (* The type constant didn't exist so far, let's add it *)
      SMap.add name sig_ typeconsts
  (* This covers the following case
   *
   * interface I1 { abstract const type T; }
   * interface I2 { const type T = int; }
   *
   * class C implements I1, I2 {}
   *
   * Then C::T == I2::T since I2::T is not abstract
   *)
  | Some old_sig
    when not (is_abstract_typeconst old_sig) && (is_abstract_typeconst sig_) ->
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
  | Some old_sig
    when not (can_override_typeconst old_sig) && (can_override_typeconst sig_) ->
      typeconsts
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
  | _ ->
      SMap.add name sig_ typeconsts

let add_constructor (cstr, cstr_consist) (acc, acc_consist) =
  let ce = match cstr, acc with
    | None, _ -> acc
    | Some ce, Some acce when should_keep_old_sig ce acce ->
      acc
    | _ -> cstr
  in ce, cstr_consist || acc_consist

let add_inherited inherited acc = {
  ih_substs = SMap.merge begin fun _ sub old_sub ->
    match sub, old_sub with
    | None, None -> None
    | Some s, None | None, Some s -> Some s
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
    | Some s, Some old_s
      when old_s.sc_from_req_extends && (not s.sc_from_req_extends) ->
      Some s
    | Some _, Some old_s -> Some old_s
  end acc.ih_substs inherited.ih_substs;
  ih_cstr     = add_constructor inherited.ih_cstr acc.ih_cstr;
  ih_consts   = SMap.fold add_const inherited.ih_consts acc.ih_consts;
  ih_typeconsts =
    SMap.fold add_typeconst inherited.ih_typeconsts acc.ih_typeconsts;
  ih_props    = add_members inherited.ih_props acc.ih_props;
  ih_sprops   = add_members inherited.ih_sprops acc.ih_sprops;
  ih_methods  = add_methods inherited.ih_methods acc.ih_methods;
  ih_smethods = add_methods inherited.ih_smethods acc.ih_smethods;
}

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let check_arity pos class_name class_type class_parameters =
  let arity = List.length class_type.dc_tparams in
  if List.length class_parameters <> arity
  then Errors.class_arity pos class_type.dc_pos class_name arity;
  ()

let make_substitution pos class_name class_type class_parameters =
  check_arity pos class_name class_type class_parameters;
  Inst.make_subst class_type.dc_tparams class_parameters

let mark_as_synthesized inh =
  let mark_elt elt = { elt with elt_synthesized = true } in
  { inh with
    ih_substs = SMap.map begin fun sc ->
      { sc with sc_from_req_extends = true }
      end inh.ih_substs;
    ih_cstr     = (Option.map (fst inh.ih_cstr) mark_elt), (snd inh.ih_cstr);
    ih_props    = SMap.map mark_elt inh.ih_props;
    ih_sprops   = SMap.map mark_elt inh.ih_sprops;
    ih_methods  = SMap.map mark_elt inh.ih_methods;
    ih_smethods = SMap.map mark_elt inh.ih_smethods;
  }

(*****************************************************************************)
(* Code filtering the private members (useful for inheritance) *)
(*****************************************************************************)

let filter_privates class_type =
  let is_not_private _ elt = match elt.elt_visibility with
    | Vprivate _ -> false
    | Vpublic | Vprotected _ -> true
  in
  {
    class_type with
    dc_props = SMap.filter is_not_private class_type.dc_props;
    dc_sprops = SMap.filter is_not_private class_type.dc_sprops;
    dc_methods = SMap.filter is_not_private class_type.dc_methods;
    dc_smethods = SMap.filter is_not_private class_type.dc_smethods;
  }

let chown_privates owner class_type =
  let chown_private elt = match elt.elt_visibility with
    | Vprivate _ -> {elt with elt_visibility = Vprivate owner}
    | Vpublic | Vprotected _ -> elt
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

let inherit_hack_class env c p class_name class_type argl =
  let subst = make_substitution p class_name class_type argl in
  let class_type =
    match class_type.dc_kind with
    | Ast.Ctrait ->
        (* Change the private visibility to point to the inheriting class *)
        chown_privates (snd c.c_name) class_type
    | Ast.Cnormal | Ast.Cabstract | Ast.Cinterface ->
        filter_privates class_type
    | Ast.Cenum -> class_type
  in
  let typeconsts = SMap.map (Inst.instantiate_typeconst subst)
    class_type.dc_typeconsts in
  let consts = SMap.map (Inst.instantiate_cc subst) class_type.dc_consts in
  let props    = class_type.dc_props in
  let sprops   = class_type.dc_sprops in
  let methods = class_type.dc_methods in
  let smethods = class_type.dc_smethods in
  let cstr     = Decl_env.get_construct env class_type in
  let subst_ctx = {
    sc_subst = subst;
    sc_class_context = snd c.c_name;
    sc_from_req_extends = false;
  } in
  let substs = SMap.add class_name subst_ctx class_type.dc_substs in
  let result = {
    ih_substs   = substs;
    ih_cstr     = cstr;
    ih_consts   = consts;
    ih_typeconsts = typeconsts;
    ih_props    = props;
    ih_sprops   = sprops;
    ih_methods  = methods;
    ih_smethods = smethods;
  } in
  result

(* mostly copy paste of inherit_hack_class *)
let inherit_hack_class_constants_only p class_name class_type argl =
  let subst = make_substitution p class_name class_type argl in
  let instantiate = SMap.map (Inst.instantiate_cc subst) in
  let consts  = instantiate class_type.dc_consts in
  let typeconsts = SMap.map (Inst.instantiate_typeconst subst)
    class_type.dc_typeconsts in
  let result = { empty with
    ih_consts   = consts;
    ih_typeconsts = typeconsts;
  } in
  result

(* This logic deals with importing XHP attributes from an XHP class
   via the "attribute :foo;" syntax. *)
let inherit_hack_xhp_attrs_only class_type =
  (* Filter out properties that are not XHP attributes *)
  let props =
    SMap.fold begin fun name prop acc ->
      if prop.elt_is_xhp_attr then SMap.add name prop acc else acc
    end class_type.dc_props SMap.empty in
  let result = { empty with ih_props = props; } in
  result

(*****************************************************************************)

let from_class env c hint =
  let pos, class_name, class_params = Decl_utils.unwrap_class_hint hint in
  let class_params = List.map class_params (Decl_hint.hint env) in
  let class_type = Decl_env.get_class_dep env class_name in
  match class_type with
  | None ->
    (* The class lives in PHP, we don't know anything about it *)
    empty
  | Some class_ ->
    (* The class lives in Hack *)
    inherit_hack_class env c pos class_name class_ class_params

(* mostly copy paste of from_class *)
let from_class_constants_only env hint =
  let pos, class_name, class_params = Decl_utils.unwrap_class_hint hint in
  let class_params = List.map class_params (Decl_hint.hint env) in
  let class_type = Decl_env.get_class_dep env class_name in
  match class_type with
  | None ->
    (* The class lives in PHP, we don't know anything about it *)
    empty
  | Some class_ ->
    (* The class lives in Hack *)
    inherit_hack_class_constants_only pos class_name class_ class_params

let from_class_xhp_attrs_only env hint =
  let _pos, class_name, _class_params = Decl_utils.unwrap_class_hint hint in
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
    match c.c_kind with
      | Ast.Cabstract -> c.c_implements @ c.c_extends
      | Ast.Ctrait -> c.c_implements @ c.c_extends @ c.c_req_implements
      | _ -> c.c_extends
  in
  let inherited_l = List.map extends (from_class env c) in
  List.fold_right ~f:add_inherited inherited_l ~init:empty

let from_requirements env c acc reqs =
  let inherited = from_class env c reqs in
  let inherited = mark_as_synthesized inherited in
  add_inherited inherited acc

let from_trait env c acc uses =
  let inherited = from_class env c uses in
  add_inherited inherited acc

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
  let acc = List.fold_left ~f:(from_requirements env c)
    ~init:acc c.c_req_extends in
  (* ... are overridden with those inherited from used traits *)
  let acc = List.fold_left ~f:(from_trait env c) ~init:acc c.c_uses in
  let acc = List.fold_left ~f:(from_xhp_attr_use env)
    ~init:acc c.c_xhp_attr_uses in
  (* todo: what about the same constant defined in different interfaces
   * we implement? We should forbid and say "constant already defined".
   * to julien: where is the logic that check for duplicated things?
   * todo: improve constant handling, see task #2487051
   *)
  let acc = List.fold_left ~f:(from_interface_constants env)
    ~init:acc c.c_req_implements in
  List.fold_left ~f:(from_interface_constants env) ~init:acc c.c_implements
