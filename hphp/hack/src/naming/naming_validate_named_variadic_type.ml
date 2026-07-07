(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

module Env = struct
  let named_variadic_type Naming_phase_env.{ named_variadic_type; _ } =
    named_variadic_type
end

let on_hint on_error hint ~ctx =
  let err_opt =
    if Env.named_variadic_type ctx then
      None
    else
      match hint with
      | (_, Aast.(Hfun { hf_named_variadic_ty = Some (pos, _); _ })) ->
        Some
          (Naming_phase_error.naming
          @@ Naming_error.Named_variadic_type_disallowed pos)
      | _ -> None
  in
  Option.iter ~f:on_error err_opt;
  (ctx, Ok hint)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.{ id with on_ty_hint = Some (on_hint on_error) }
