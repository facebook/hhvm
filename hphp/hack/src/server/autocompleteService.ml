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

let auto_complete_id x =
  if Autocomplete.is_auto_complete (snd x)
  then begin
    Autocomplete.argument_global_type := Some Autocomplete.Acid;
    Autocomplete.auto_complete_for_global := snd x
  end

let auto_complete_method is_static class_ id env cid =
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

let auto_complete_smethod = auto_complete_method true

let auto_complete_cmethod = auto_complete_method false

let attach_hooks () =
  Typing_hooks.attach_id_hook auto_complete_id;
  Typing_hooks.attach_smethod_hook auto_complete_smethod;
  Typing_hooks.attach_cmethod_hook auto_complete_cmethod

let detach_hooks () =
  Typing_hooks.remove_all_hooks()
