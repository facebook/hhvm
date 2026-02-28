(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs
module Env = Tast_env

let check_xhp_children env pos ty =
  let (is_xhp_child, subty_err_opt) = Env.is_xhp_child env pos ty in
  let Equal = Tast_env.eq_typing_env in
  Option.iter subty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
  if not is_xhp_child then
    let ty_str = lazy (Env.print_error_ty ~ignore_dynamic:true env ty) in
    let ty_reason_msg =
      Lazy.map ty_str ~f:(fun ty_str ->
          Reason.to_string ("This is " ^ ty_str) (get_reason ty))
    in
    let Equal = Tast_env.eq_typing_env in
    Typing_error_utils.add_typing_error
      ~env
      Typing_error.(xhp @@ Primary.Xhp.Illegal_xhp_child { pos; ty_reason_msg })

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      function
      | (_, _, Xml (_, _, tel)) ->
        List.iter tel ~f:(fun (ty, pos, _) -> check_xhp_children env pos ty)
      | _ -> ()

    method! at_xhp_child env child =
      match child with
      | ChildName (p, name)
        when (not @@ Naming_special_names.XHP.is_reserved name)
             && (not @@ Naming_special_names.XHP.is_xhp_category name) -> begin
        match Env.get_class env name with
        | Decl_entry.Found _
        | Decl_entry.NotYetAvailable ->
          ()
        | Decl_entry.DoesNotExist ->
          let custom_err_config =
            let tcopt = Tast_env.get_tcopt env in
            TypecheckerOptions.custom_error_config tcopt
          in
          Diagnostics.add_diagnostic
            (Naming_error_utils.to_user_diagnostic
               (Naming_error.Unbound_name
                  { pos = p; name; kind = Name_context.ClassContext })
               custom_err_config)
      end
      | _ -> ()
  end
