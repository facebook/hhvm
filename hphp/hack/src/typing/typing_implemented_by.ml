(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast_defs
module SN = Naming_special_names

(**
 * Validation for the __ImplementedBy attribute.
 * - the referenced function exists
 * - The method and function signatures are compatible
 *)
let check_method env cls_id m =
  match
    List.find
      ~f:(fun { Aast.ua_name = (_, name); _ } ->
        String.equal name SN.UserAttributes.uaImplementedBy)
      m.m_user_attributes
  with
  | Some { ua_params = [(_, p, Aast.String impl_name)]; _ } ->
    let (env, _, fun_type) =
      Typing.Function_pointer.synth_top_level p (p, impl_name) [] Code env
    in
    let (env, _, meth_type) =
      Typing.Method_caller.synth_function_type m.m_span (cls_id, m.m_name) env
    in
    let (env, ty_err_opt) =
      Typing_subtype.sub_type
        env
        fun_type
        meth_type
        (Some
           Typing_error.(
             Reasons_callback.of_primary_error
             @@ Typing_error.Primary.Unify_error
                  {
                    pos = p;
                    msg_opt =
                      Some
                        "The method must be implemented by a function with a compatible type.";
                    reasons_opt = None;
                  }))
    in
    Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
    env
  | _ -> env
