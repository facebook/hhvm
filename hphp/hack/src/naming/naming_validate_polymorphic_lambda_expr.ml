(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let get_in_poly_lambda
    Naming_phase_env.
      {
        validate_polymorphic_lambda =
          Validate_polymorphic_lambda.{ in_poly_lambda };
        _;
      } =
  in_poly_lambda

let set_in_poly_lambda env in_poly_lambda =
  let open Naming_phase_env in
  let validate_polymorphic_lambda =
    Validate_polymorphic_lambda.{ in_poly_lambda }
  in
  { env with validate_polymorphic_lambda }

let on_fun_ on_error fun_ ~ctx =
  let Aast_defs.{ f_tparams; f_params; f_ret; f_span; _ } = fun_ in
  let in_poly_lambda = not @@ List.is_empty f_tparams in
  let () =
    if in_poly_lambda then
      let () =
        match f_ret with
        | (_, None) ->
          on_error
            (Naming_phase_error.naming
            @@ Naming_error.Polymorphic_lambda_missing_return_hint f_span)
        | _ -> ()
      in
      List.iter f_params ~f:(function
          | Aast_defs.{ param_pos; param_name; param_type_hint = (_, None); _ }
            ->
            on_error
              (Naming_phase_error.naming
              @@ Naming_error.Polymorphic_lambda_missing_param_hint
                   { param_pos; param_name })
          | _ -> ())
  in
  let ctx = set_in_poly_lambda ctx in_poly_lambda in
  (ctx, Ok fun_)

let on_func_body func_body ~ctx =
  (* We only want to disallow wildcards in hints appearing the signature, not
     the body *)
  let ctx = set_in_poly_lambda ctx false in
  (ctx, Ok func_body)

let on_hint on_error hint ~ctx =
  let () =
    if get_in_poly_lambda ctx then
      match hint with
      | (pos, Aast_defs.Hwildcard) ->
        on_error
          (Naming_phase_error.naming
          @@ Naming_error.Wildcard_hint_disallowed pos)
      | _ -> ()
    else
      ()
  in
  (ctx, Ok hint)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.
      {
        id with
        on_ty_fun_ = Some (fun elem ~ctx -> on_fun_ on_error elem ~ctx);
        on_ty_hint = Some (fun elem ~ctx -> on_hint on_error elem ~ctx);
        on_ty_func_body = Some (fun elem ~ctx -> on_func_body elem ~ctx);
      }
