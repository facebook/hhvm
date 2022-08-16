(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module A = Aast
module SN = Naming_special_names

let tparam_id tparam = snd tparam.A.tp_name

let mutate_type_hint tparam_mapping (hint : 'a A.type_hint) =
  (* TODO: The implementation is shallow. Use a visitor to deeply replace type
     hints. *)
  match hint with
  | (ann, Some (pos, A.Happly ((_, id), []))) ->
    begin
      match SMap.find_opt id tparam_mapping with
      | Some hint_ -> (ann, Some (pos, hint_))
      | None -> hint
    end
  | hint -> hint

let mutate_param tparam_mapping param =
  let param_type_hint =
    mutate_type_hint tparam_mapping param.A.param_type_hint
  in
  A.{ param with param_type_hint }

let mutate_tparams tparams =
  let default_hint pos = A.(Happly ((pos, SN.Typehints.int), [])) in
  let eliminate_tparam (tparams, mapping) tparam =
    (* TODO: Respect the bounds on the type parameter *)
    let default_hint = default_hint (fst tparam.A.tp_name) in
    let mapping = SMap.add (tparam_id tparam) default_hint mapping in
    (tparams, mapping)
  in
  List.fold ~init:([], SMap.empty) ~f:eliminate_tparam tparams

let mutate_fun fun_ =
  let (f_tparams, tparam_mapping) = mutate_tparams fun_.A.f_tparams in
  let f_params = List.map ~f:(mutate_param tparam_mapping) fun_.A.f_params in
  let f_ret = mutate_type_hint tparam_mapping fun_.A.f_ret in
  A.{ fun_ with f_tparams; f_params; f_ret }

let mutate_fun_def fd =
  let fd_fun = mutate_fun fd.A.fd_fun in
  A.{ fd with fd_fun }
