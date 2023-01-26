(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
module SN = Naming_special_names

let on_expr_ :
      'a 'b.
      _ * ('a, 'b) Aast.expr_ * _ ->
      (_ * ('a, 'b) Aast.expr_ * _, _ * ('a, 'b) Aast.expr_ * _) result =
 fun (env, expr_, err) ->
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
    | Aast.Pipe ((pos, _), e1, e2) ->
      let lid = (pos, Local_id.make_unscoped SN.SpecialIdents.dollardollar) in
      Aast.Pipe (lid, e1, e2)
    | _ -> expr_
  in
  Ok (env, expr_, err)

let pass =
  Naming_phase_pass.(
    top_down Ast_transform.{ identity with on_expr_ = Some on_expr_ })
