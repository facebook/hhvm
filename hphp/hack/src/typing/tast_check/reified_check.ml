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

let match_reified i (tparam, targ) =
  let (_, tparam_id, _, tparam_reified) = tparam in
  let ((targ_pos, _), targ_reified) = targ in
  if (tparam_reified <> targ_reified) then
    Errors.mismatched_reify tparam_id targ_pos targ_reified i

let handler = object
  inherit Tast_visitor.handler_base


  method! at_expr _ x =
    (* only considering functions where one or more params are reified *)
    match x with
    | (pos, _), Call (_, ((_, (_, Tfun { ft_pos; ft_tparams; _ })), _), tal, _, _) ->
      let tparams = fst ft_tparams in
      if List.exists ~f:(fun (_, _, _, r) -> r) tparams && List.is_empty tal then
        Errors.require_args_reify ft_pos pos;
      (* Unequal_lengths case handled during Typing_phase.localize_ft *)
      ignore Option.(
        List.zip tparams tal >>|
        List.iteri ~f:match_reified
      )
    | _ -> ()
end
