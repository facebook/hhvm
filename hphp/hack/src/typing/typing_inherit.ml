(**
 * Copyright (c) 2014, Facebook, Inc.
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

open Utils
open Nast
open Typing_defs
open Silent

module Env = Typing_env
module Inst = Typing_instantiate
type env = Env.env

(*****************************************************************************)
(* This is what we are trying to produce for a given class. *)
(*****************************************************************************)

type inherited = {
    ih_cstr     : class_elt option ;
    ih_consts   : class_elt SMap.t ;
    ih_cvars    : class_elt SMap.t ;
    ih_scvars   : class_elt SMap.t ;
    ih_methods  : class_elt SMap.t ;
    ih_smethods : class_elt SMap.t ;
  }

let empty = {
  ih_cstr     = None;
  ih_consts   = SMap.empty;
  ih_cvars    = SMap.empty;
  ih_scvars   = SMap.empty;
  ih_methods  = SMap.empty;
  ih_smethods = SMap.empty;
}

let desugar ih =
  ih.ih_cstr,
  ih.ih_consts,
  ih.ih_cvars,
  ih.ih_scvars,
  ih.ih_methods,
  ih.ih_smethods

(*****************************************************************************)
(* Functions used to merge an additional inherited class to the types
 * we already inherited.
 *)
(*****************************************************************************)

let is_abstract_method x =
  match x.ce_type with
  | _, Tfun x when x.ft_abstract -> true
  | _ ->  false

let add_method name sig_ methods =
  match SMap.get name methods with
  | None ->
      (* The method didn't exist so far, let's add it *)
      SMap.add name sig_ methods
  | Some old_sig ->
      if not (is_abstract_method old_sig) && is_abstract_method sig_
      (* The method was defined was not abstract, and this one is
       * abstract. We don't want to override a concrete method with
       * an abstract one.
       *)
      then methods
      (* We're overwriting a method definition. This is OK when a
       * naming conflict is parent class vs trait (trait wins!), but
       * not really OK when the naming conflict is trait vs trait
       * (we rely on HHVM to catch the error at runtime) *)
      else SMap.add name {sig_ with ce_override = false} methods

let add_methods methods' acc =
  SMap.fold add_method methods' acc

let add_members members acc =
  SMap.fold SMap.add members acc

let add_constructor cstr acc =
  match cstr with None -> acc | _ -> cstr

let add_inherited inherited acc = {
  ih_cstr     = add_constructor inherited.ih_cstr acc.ih_cstr;
  ih_consts   = add_members inherited.ih_consts acc.ih_consts;
  ih_cvars    = add_members inherited.ih_cvars acc.ih_cvars;
  ih_scvars   = add_members inherited.ih_scvars acc.ih_scvars;
  ih_methods  = add_methods inherited.ih_methods acc.ih_methods;
  ih_smethods = add_methods inherited.ih_smethods acc.ih_smethods;
}

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let desugar_class_hint = function
  | (_, Happly ((pos, class_name), type_parameters)) ->
      pos, class_name, type_parameters
  | _ -> assert false

let check_arity pos class_name class_type class_parameters =
  let arity = List.length class_type.tc_tparams in
  if List.length class_parameters <> arity && not !is_silent_mode
  then error pos ("The class "^class_name^" expects "^soi arity^" arguments");
  ()

let make_substitution ?this:(this=None) pos class_name class_type class_parameters =
  check_arity pos class_name class_type class_parameters;
  match this with
  | None -> Inst.make_subst class_type.tc_tparams class_parameters
  | Some this ->
      let this_ty = (fst this, Tgeneric ("this", Some this)) in
      Inst.make_subst_with_this this_ty class_type.tc_tparams class_parameters

let constructor env subst = function
  | None -> env, None
  | Some ce ->
    let env, ty = Inst.instantiate subst env ce.ce_type in
    env, Some {ce with ce_type = ty}

(*****************************************************************************)
(* Builds the inherited type when the class lives in Hack *)
(*****************************************************************************)

let inherit_hack_class c env p class_name class_type argl =
  let self = Typing.get_self_from_c env c in
  let subst = make_substitution ~this:(Some self) p class_name class_type argl in
  let instantiate = smap_env (Inst.instantiate_ce subst) in
  let class_type =
    match class_type.tc_kind with
    | Ast.Ctrait ->
        (* Change the private visibility to point to the inheriting class *)
        chown_privates (snd c.c_name) class_type
    | Ast.Cnormal | Ast.Cabstract | Ast.Cinterface ->
        filter_privates class_type
  in
  let env, consts   = instantiate env class_type.tc_consts in
  let env, cvars    = instantiate env class_type.tc_cvars in
  let env, scvars   = instantiate env class_type.tc_scvars in
  let env, methods  = instantiate env class_type.tc_methods in
  let env, smethods = instantiate env class_type.tc_smethods in
  let env, cstr     = Env.get_construct env class_type in
  let env, cstr     = constructor env subst cstr in
  let result = {
    ih_cstr     = cstr;
    ih_consts   = consts;
    ih_cvars    = cvars;
    ih_scvars   = scvars;
    ih_methods  = methods;
    ih_smethods = smethods;
  } in
  env, result

(* mostly copy paste of inherit_hack_class *)
let inherit_hack_class_constants_only env p class_name class_type argl =
  let subst = make_substitution p class_name class_type argl in
  let env, consts =
    smap_env (Inst.instantiate_ce subst) env class_type.tc_consts in
  let result = { empty with
    ih_consts   = consts;
  } in
  env, result

(*****************************************************************************)

let from_class c env hint =
  let pos, class_name, class_params = desugar_class_hint hint in
  let env, class_params = lfold Typing_hint.hint env class_params in
  let env, class_type = Env.get_class_dep env class_name in
  match class_type with
  | None ->
      (* The class lives in PHP, we don't know anything about it *)
      env, empty
  | Some class_ ->
      (* The class lives in Hack *)
      inherit_hack_class c env pos class_name class_ class_params

(* mostly copy paste of from_class *)
let from_class_constants_only env hint =
  let pos, class_name, class_params = desugar_class_hint hint in
  let env, class_params = lfold Typing_hint.hint env class_params in
  let env, class_type = Env.get_class_dep env class_name in
  match class_type with
  | None ->
      (* The class lives in PHP, we don't know anything about it *)
      env, empty
  | Some class_ ->
      (* The class lives in Hack *)
    inherit_hack_class_constants_only env pos class_name class_ class_params

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
  let env, inherited_l = lfold (from_class c) env extends in
  env, List.fold_right add_inherited inherited_l empty

let from_requirements c (env, acc) reqs =
  let env, inherited = from_class c env reqs in
  env, add_inherited inherited acc

let from_trait c (env, acc) uses =
  let env, inherited = from_class c env uses in
  env, add_inherited inherited acc

let from_interface_constants (env, acc) impls =
  let env, inherited = from_class_constants_only env impls in
  env, add_inherited inherited acc

(*****************************************************************************)
(* The API to the outside *)
(*****************************************************************************)

let make env c =
  (* members inherited from parent class ... *)
  let acc = from_parent env c in
  let acc = List.fold_left (from_requirements c) acc c.c_req_extends in
  (* ... are overridden with those inherited from used traits *)
  let acc = List.fold_left (from_trait c) acc c.c_uses in
  (* todo: what about the same constant defined in different interfaces
   * we implement? We should forbid and say "constant already defined".
   * to julien: where is the logic that check for duplicated things?
   * todo: improve constant handling, see task #2487051
   *)
  let acc = List.fold_left from_interface_constants acc c.c_req_implements in
  List.fold_left from_interface_constants acc c.c_implements
