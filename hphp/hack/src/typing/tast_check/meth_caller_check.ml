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
let check_parameters env =
  let get_illegal_parameter ftype =
    List.find
      ~f:(fun ft_param ->
        match get_fp_mode ft_param with
        | FPnormal -> false
        | _ -> true)
      ftype.ft_params
  in
  let rec check pos ft =
    match get_node ft with
    | Tfun ftype -> begin
      match get_illegal_parameter ftype with
      | Some fparam ->
        let convention =
          match get_fp_mode fparam with
          | FPinout -> "`inout`"
          | FPnormal -> "normal"
        in
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(
            primary
            @@ Primary.Invalid_meth_caller_calling_convention
                 { pos; decl_pos = fparam.fp_pos; convention })
      | None -> ()
    end
    | Tnewtype (name, [ty], _)
      when String.equal Naming_special_names.Classes.cSupportDyn name
           || String.equal Naming_special_names.Classes.cFunctionRef name ->
      check pos ty
    | _ -> ()
  in
  check

let check_readonly_return env pos ft =
  match get_node ft with
  | Tfun ftype ->
    if Flags.get_ft_returns_readonly ftype then
      let (_, expanded_ty) = Tast_env.expand_type env ft in
      let r = get_reason expanded_ty in
      let rpos = Typing_reason.to_pos r in
      Typing_error_utils.add_typing_error
        ~env:(Tast_env.tast_env_as_typing_env env)
        Typing_error.(
          primary
          @@ Primary.Invalid_meth_caller_readonly_return
               { pos; decl_pos = rpos })
  | _ -> ()

let rec strip_supportdyn ty =
  match get_node ty with
  | Tnewtype (name, [ty], _)
    when String.equal name Naming_special_names.Classes.cSupportDyn
         || String.equal name Naming_special_names.Classes.cFunctionRef ->
    strip_supportdyn ty
  | _ -> ty

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env e =
      match e with
      | (ft, pos, Method_caller _) ->
        let ft = strip_supportdyn ft in
        check_parameters (Tast_env.tast_env_as_typing_env env) pos ft;
        check_readonly_return env pos ft
      | _ -> ()
  end
