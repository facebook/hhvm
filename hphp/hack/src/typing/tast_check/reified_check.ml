(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Tast
open Typing_defs

let match_reified i (tp, ((targ_pos, _), targ_reified)) =
  if (tp.tp_reified <> targ_reified) then
    Errors.mismatched_reify tp.tp_name targ_pos targ_reified i

let verify_targs expr_pos decl_pos targs tparams =
  if List.exists ~f:(fun t -> t.tp_reified) tparams && List.is_empty targs then
    Errors.require_args_reify decl_pos expr_pos;
  (* Unequal_lengths case handled elsewhere *)
  ignore Option.(
    List.zip tparams targs >>|
    List.iteri ~f:match_reified
  )

let handler = object
  inherit Tast_visitor.handler_base

  method! at_expr env x =
    (* only considering functions where one or more params are reified *)
    match x with
    | (pos, _), Call (_, ((_, (_, Tfun { ft_pos; ft_tparams; _ })), _), tal, _, _) ->
      let tparams = fst ft_tparams in
      verify_targs pos ft_pos tal tparams
    | (pos, _), New ((_, CI ((_, class_id), tal)), _, _, _, _) ->
      begin match Tast_env.get_class env class_id with
      | Some cls ->
        let tparams = Typing_classes_heap.tparams cls in
        let class_pos = Typing_classes_heap.pos cls in
        verify_targs pos class_pos tal tparams
      | None -> () end
    | _ -> ()
end
