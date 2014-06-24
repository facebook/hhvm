(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Utils
open Typing_defs

let auto_complete_suffix = "AUTO332"
let suffix_len = String.length auto_complete_suffix
let is_auto_complete x =
  String.length x >= suffix_len &&
  let suffix = String.sub x (String.length x - suffix_len) suffix_len in
  suffix = auto_complete_suffix

let make_result name ty env =
  let type_ = Typing_print.full_strip_ns env ty in
  let pos = Reason.to_pos (fst ty) in
  Autocomplete.make_result name pos type_

let autocomplete_token ac_type x =
  if Autocomplete.is_auto_complete (snd x)
  then begin
    Autocomplete.argument_global_type := Some ac_type;
    Autocomplete.auto_complete_for_global := snd x
  end
  
let autocomplete_id = autocomplete_token Autocomplete.Acid

let autocomplete_hint = autocomplete_token Autocomplete.Actype

let autocomplete_new = autocomplete_token Autocomplete.Acnew

let autocomplete_method is_static class_ id env cid =
  if is_auto_complete (snd id)
  then begin
    Autocomplete.argument_global_type := Some Autocomplete.Acclass_get;
    let results =
      if is_static
      then SMap.union class_.tc_smethods
                      (SMap.union class_.tc_consts class_.tc_scvars)
      else SMap.union class_.tc_methods class_.tc_cvars
    in
    let results = SMap.filter begin fun _ x ->
      if class_.tc_members_fully_known then
        match Typing.is_visible env x.ce_visibility cid with
          | None -> true
          | _ -> false
      else true end results
    in
    Autocomplete.auto_complete_result :=
      SMap.fold begin fun x class_elt acc ->
        let ty = class_elt.ce_type in
        let type_ = Typing_print.full_strip_ns env ty in
        let pos = Reason.to_pos (fst ty) in
        let sig_ = x^" "^type_ in
        SMap.add sig_ (Autocomplete.make_result x pos type_) acc
      end results SMap.empty
  end

let autocomplete_smethod = autocomplete_method true

let autocomplete_cmethod = autocomplete_method false

let autocomplete_lvar_naming id locals =
  if is_auto_complete (snd id)
  then begin
    Autocomplete.argument_global_type := Some Autocomplete.Acvar;
    (* Store the position and a map of name to ident so we can add
     * types at this point later *)
    Autocomplete.auto_complete_pos := Some (fst id);
    Autocomplete.auto_complete_vars := SMap.map snd locals
  end

let autocomplete_lvar_typing id env =
  if Some (fst id)= !(Autocomplete.auto_complete_pos)
  then begin
    (* Get the types of all the variables in scope at this point *)
    Autocomplete.auto_complete_result :=
      SMap.mapi begin fun x ident ->
        let _, ty = Typing_env.get_local env ident in
        make_result x ty env
      end !Autocomplete.auto_complete_vars;
    (* Add $this if we're in a instance method *)
    let ty = Typing_env.get_self env in
    if not (Typing_env.is_static env) && (fst ty) <> Reason.Rnone
    then Autocomplete.auto_complete_result :=
           SMap.add "$this"
                     (make_result "$this" ty env)
                     !Autocomplete.auto_complete_result
  end

let attach_hooks () =
  Typing_hooks.attach_id_hook autocomplete_id;
  Typing_hooks.attach_smethod_hook autocomplete_smethod;
  Typing_hooks.attach_cmethod_hook autocomplete_cmethod;
  Naming_hooks.attach_hint_hook autocomplete_hint;
  Naming_hooks.attach_new_id_hook autocomplete_new;
  Naming_hooks.attach_lvar_hook autocomplete_lvar_naming;
  Typing_hooks.attach_lvar_hook autocomplete_lvar_typing

let detach_hooks () =
  Typing_hooks.remove_all_hooks();
  Naming_hooks.remove_all_hooks()
