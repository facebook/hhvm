(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
module SN = Naming_special_names

module Env : sig
  type t

  val empty : t
end = struct
  type t = unit

  let empty = ()
end

let on_expr_ (env, expr_, err) =
  let expr_ =
    match expr_ with
    | Aast.Lvar (pos, local_id) ->
      let lid_str = Local_id.to_string local_id in
      if String.equal lid_str SN.SpecialIdents.this then
        Aast.This
      else if String.equal lid_str SN.SpecialIdents.dollardollar then
        Aast.Dollardollar
          (pos, Local_id.make_unscoped SN.SpecialIdents.dollardollar)
      else if String.equal lid_str SN.SpecialIdents.placeholder then
        Aast.Lplaceholder pos
      else
        expr_
    | _ -> expr_
  in
  Naming_phase_pass.Cont.next (env, expr_, err)

let pass =
  Naming_phase_pass.(top_down { identity with on_expr_ = Some on_expr_ })

let visitor = Naming_phase_pass.mk_visitor [pass]

let elab f ?(env = Env.empty) elem = fst @@ f env elem

let elab_fun_def ?env elem = elab visitor#on_fun_def ?env elem

let elab_typedef ?env elem = elab visitor#on_typedef ?env elem

let elab_module_def ?env elem = elab visitor#on_module_def ?env elem

let elab_gconst ?env elem = elab visitor#on_gconst ?env elem

let elab_class ?env elem = elab visitor#on_class_ ?env elem

let elab_program ?env elem = elab visitor#on_program ?env elem
