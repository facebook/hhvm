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
(* Checks that a class implements an interface *)
(*****************************************************************************)

open Utils
open Typing_defs
open Typing_ops
open Silent

module Env = Typing_env
module TUtils = Typing_utils
module Inst = Typing_instantiate

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let is_private = function
  | { ce_visibility = Vprivate _ } -> true
  | _ -> false

(*****************************************************************************)
(* Errors *)
(*****************************************************************************)

module Error = struct

  (* Incompatible visibilities *)
  let visibility parent_class_elt class_elt =
    let parent_pos = Reason.to_pos (fst parent_class_elt.ce_type) in
    let pos = Reason.to_pos (fst class_elt.ce_type) in
    let parent_vis = TUtils.string_of_visibility parent_class_elt.ce_visibility in
    let vis = TUtils.string_of_visibility class_elt.ce_visibility in
    let msg1 = pos, "This member visibility is: " ^ vis in
    let msg2 = parent_pos, parent_vis ^ " was expected" in
    error_l [msg1; msg2]

  (* Method missing *)
  let member_not_implemented member_name parent_pos pos defn_pos =
    let msg1 = pos, "This object doesn't implement the method "^member_name in
    let msg2 = parent_pos, "Which is required by this interface" in
    let msg3 = defn_pos, "As defined here" in
    error_l [msg1; msg2; msg3]

  (* Incompatible override *)
  let override (parent_pos, parent_name) (pos, name) error_message_l =
    let msg1 = pos, ("This object is of type "^name) in
    let msg2 = parent_pos,
      ("It is incompatible with this object of type "^parent_name^
       "\nbecause some of their methods are incompatible."^
       "\nRead the following to see why:"
      ) in
    (* This is a cascading error message *)
    raise (Error (msg1 :: msg2 :: error_message_l))

  let missing_constructor pos =
    error pos "The constructor is not implemented"

end

(*****************************************************************************)
(* Given a map of members, check that the overriding is correct.
 * Please note that 'members' has a very general meaning here.
 * It can be class variables, methods, static methods etc ... The same logic
 * is applied to verify that the overriding is correct.
 *)
(*****************************************************************************)

let use_parent_for_known = false
let check_partially_known_method_returns = false
let check_partially_known_method_params = false
let check_partially_known_method_visibility = true

(* Rules for visibility *)
let check_visibility parent_class_elt class_elt =
  match parent_class_elt.ce_visibility, class_elt.ce_visibility with
  | Vpublic      , Vpublic
  | Vprivate _   , Vprivate _
  | Vprotected _ , Vprotected _
  | Vprotected _ , Vpublic       -> ()
  | _ -> Error.visibility parent_class_elt class_elt

(* Check that all the required members are implemented *)
let check_members_implemented parent_reason reason parent_members members =
  SMap.iter begin fun member_name class_elt ->
    match class_elt.ce_visibility with
    | Vprivate _ -> ()
    | _ when not (SMap.mem member_name members) ->
        let defn_reason = Reason.to_pos (fst class_elt.ce_type) in
        Error.member_not_implemented member_name parent_reason reason defn_reason
    | _ -> ()
  end parent_members

(* Check that overriding is correct *)
let check_override env parent_class class_ parent_class_elt class_elt =
  let class_known = if use_parent_for_known then parent_class.tc_members_fully_known
    else class_.tc_members_fully_known in
  let check_vis = class_known || check_partially_known_method_visibility in
  if check_vis then check_visibility parent_class_elt class_elt else ();
  let check_params = class_known || check_partially_known_method_params in
  if check_params then
    (* Replace the parent's this type with the child's. This avoids complaining
     * about how this as Base and this as Child are different types *)
    let self = Env.get_self env in
    let this_ty = fst self, Tgeneric ("this", Some self) in
    let env, parent_ce_type =
      Inst.instantiate_this env parent_class_elt.ce_type this_ty in
    match parent_ce_type, class_elt.ce_type with
      | (r1, Tfun ft1), (r2, Tfun ft2) ->
        let subtype_funs =
          if class_known || check_partially_known_method_returns then
            SubType.subtype_funs else SubType.subtype_funs_no_return in
        ignore (subtype_funs env r1 ft1 r2 ft2)
      | fty1, fty2 ->
        let pos = Reason.to_pos (fst fty2) in
        ignore (unify pos Typing_reason.URnone env fty1 fty2)

(* Privates are only visible in the parent, we don't need to check them *)
let filter_privates members =
  SMap.fold begin fun name class_elt acc ->
    if is_private class_elt
    then acc
    else SMap.add name class_elt acc
  end members SMap.empty

let check_members env parent_class class_ parent_members members =
  let parent_members = filter_privates parent_members in
  SMap.iter begin fun member_name parent_class_elt ->
    match SMap.get member_name members with
    | Some class_elt  ->
      check_override env parent_class class_ parent_class_elt class_elt
    | None -> ()
 end parent_members

(*****************************************************************************)
(* Before checking that a class implements an interface, we have to
 * substitute the type parameters with their real type.
 *)
(*****************************************************************************)

(* Instantiation basically applies the substitution *)
let instantiate_members subst env members =
  smap_env (Inst.instantiate_ce subst) env members

(* TODO constant inheritance is broken. We don't inherit constants that
 * come from interfaces. *)
let make_all_members class_ = [
(* class_.tc_consts; *)
  class_.tc_cvars;
  class_.tc_scvars;
  class_.tc_methods;
  class_.tc_smethods;
]

(* When an interface defines a constructor, we check that they are compatible *)
let check_constructors env parent_class class_ =
  if parent_class.tc_kind <> Ast.Cinterface then () else
  match parent_class.tc_construct, class_.tc_construct with
  | None, _ -> ()
  | Some parent_cstr, None ->
      let pos = fst parent_cstr.ce_type in
      Error.missing_constructor (Reason.to_pos pos)
  | Some parent_cstr, Some cstr ->
      check_override env parent_class class_ parent_cstr cstr

let check_class_implements env parent_class class_ =
  let parent_pos, parent_class, parent_tparaml = parent_class in
  let pos, class_, tparaml = class_ in
  let fully_known = class_.tc_members_fully_known in
  let psubst = Inst.make_subst parent_class.tc_tparams parent_tparaml in
  let subst = Inst.make_subst class_.tc_tparams tparaml in
  let pmemberl = make_all_members parent_class in
  let memberl = make_all_members class_ in
  check_constructors env parent_class class_;
  let env, pmemberl = lfold (instantiate_members psubst) env pmemberl in
  let env, memberl = lfold (instantiate_members subst) env memberl in
  if not fully_known then () else
    List.iter2 (check_members_implemented parent_pos pos) pmemberl memberl;
  List.iter2 (check_members env parent_class class_) pmemberl memberl;
  ()

(*****************************************************************************)
(* The externally visible function *)
(*****************************************************************************)

let open_class_hint = function
  | r, Tapply (name, tparaml) -> Reason.to_pos r, name, tparaml
  | _ -> assert false

let check_implements env parent_type type_ =
  if !is_silent_mode then () else
  let parent_pos, parent_name, parent_tparaml = open_class_hint parent_type in
  let pos, name, tparaml = open_class_hint type_ in
  let env, parent_class = Env.get_class env (snd parent_name) in
  let env, class_ = Env.get_class env (snd name) in
  match parent_class, class_ with
  | None, _ | _, None -> ()
  | Some parent_class, Some class_ ->
      let parent_class = parent_pos, parent_class, parent_tparaml in
      let class_ = pos, class_, tparaml in
      try
        check_class_implements env parent_class class_
      with Error errorl ->
        Error.override parent_name name errorl
