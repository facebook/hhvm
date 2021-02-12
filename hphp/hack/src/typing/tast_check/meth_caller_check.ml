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
          Errors.invalid_meth_caller_calling_convention
            pos
            fparam.fp_pos
            convention
        | None -> ()
      end
    | _ -> ()

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr _env e =
      match e with
      | ((pos, ft), Method_caller _) -> check_parameters pos ft
      | _ -> ()
  end
