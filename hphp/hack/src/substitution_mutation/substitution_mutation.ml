(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module A = Aast

let tparam_id tparam = snd tparam.A.tp_name

let mutate_type_hint tparam_mapping (hint : 'a A.type_hint) =
  (* TODO: The implementation is shallow. Use a visitor to deeply replace type
     hints. *)
  match hint with
  | (ann, Some (_, A.Happly ((pos, id), [])))
  | (ann, Some (pos, A.Habstr (id, _))) -> begin
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
  let default_hint = A.(Hprim Tint) in
  let eliminate_tparam (tparams, mapping) tparam =
    (* TODO: Respect the bounds on the type parameter *)
    let mapping = SMap.add (tparam_id tparam) default_hint mapping in
    (tparams, mapping)
  in
  List.fold ~init:([], SMap.empty) ~f:eliminate_tparam tparams

let mutate_fun_def fd =
  let fun_ = fd.A.fd_fun in
  let (fd_tparams, tparam_mapping) = mutate_tparams fd.A.fd_tparams in
  let f_params = List.map ~f:(mutate_param tparam_mapping) fun_.A.f_params in
  let f_ret = mutate_type_hint tparam_mapping fun_.A.f_ret in
  let fd_fun = A.{ fun_ with f_params; f_ret } in
  A.{ fd with fd_fun; fd_tparams }
