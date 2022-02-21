(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs

(* meth_caller does not support methods with inout parameters *)
let check_parameters =
  let get_illegal_parameter ftype =
    List.find
      ~f:(fun ft_param ->
        match get_fp_mode ft_param with
        | FPnormal -> false
        | _ -> true)
      ftype.ft_params
  in
  fun pos ft ->
    match get_node ft with
    | Tfun ftype ->
      begin
        match get_illegal_parameter ftype with
        | Some fparam ->
          let convention =
            match get_fp_mode fparam with
            | FPinout -> "`inout`"
            | FPnormal -> "normal"
          in
          Errors.add_typing_error
            Typing_error.(
              primary
              @@ Primary.Invalid_meth_caller_calling_convention
                   { pos; decl_pos = fparam.fp_pos; convention })
        | None -> ()
      end
    | _ -> ()

let check_readonly_return env pos ft =
  match get_node ft with
  | Tfun ftype ->
    if Flags.get_ft_returns_readonly ftype then
      let (_, expanded_ty) = Tast_env.expand_type env ft in
      let r = get_reason expanded_ty in
      let rpos = Typing_reason.to_pos r in
      Errors.add_typing_error
        Typing_error.(
          primary
          @@ Primary.Invalid_meth_caller_readonly_return
               { pos; decl_pos = rpos })
  | _ -> ()

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env e =
      match e with
      | (ft, pos, Method_caller _) ->
        check_parameters pos ft;
        check_readonly_return env pos ft
      | _ -> ()
  end
