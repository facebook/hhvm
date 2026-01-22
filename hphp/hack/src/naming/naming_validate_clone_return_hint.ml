(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Aast

let on_method
    _on_error
    ({ m_name = (_, name); m_ret; _ } as m : ('a, 'b) Aast.method_)
    ~ctx =
  if String.equal name Naming_special_names.Members.__clone then
    match m_ret with
    | (_, Some (_, Hprim Tvoid)) -> ()
    | (_, Some (p, _)) ->
      Diagnostics.add_diagnostic
        Nast_check_error.(to_user_diagnostic @@ Clone_return_type p)
    | _ -> ()
  else
    ();
  (ctx, Ok m)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.bottom_up
    Aast.Pass.
      { id with on_ty_method_ = Some (fun m ~ctx -> on_method on_error m ~ctx) }
