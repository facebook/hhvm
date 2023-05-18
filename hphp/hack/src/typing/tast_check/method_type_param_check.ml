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

let check_tparams env tps =
  let check_tparam tp =
    match tp.tp_variance with
    | Ast_defs.Invariant -> ()
    | _ ->
      Typing_error_utils.add_typing_error
        ~env:(Tast_env.tast_env_as_typing_env env)
        Typing_error.(primary @@ Primary.Method_variance (fst tp.tp_name))
  in
  List.iter tps ~f:check_tparam

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_method_ env m = check_tparams env m.m_tparams

    method! at_fun_def env fd = check_tparams env fd.fd_tparams
  end
