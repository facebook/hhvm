(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
module SN = Naming_special_names

let disallow_isset_inout_args_check env p = function
  | Call { func = (_, _, Id (_, pseudo_func)); args; _ }
    when String.equal pseudo_func SN.PseudoFunctions.isset
         && List.exists
              (function
                | Aast_defs.Ainout _ -> true
                | Aast_defs.Anormal _ -> false
                | Aast_defs.Anamed _ -> false)
              args ->
    Typing_error_utils.add_typing_error
      ~env:(Tast_env.tast_env_as_typing_env env)
      Typing_error.(primary @@ Primary.Isset_inout_arg p)
  | _ -> ()

let well_formed_isset_argument_check env p = function
  | Call { func = (_, _, Id (_, pseudo_func)); args = [arg]; _ } -> begin
    match Aast_utils.arg_to_expr arg with
    | (_, _, Lvar _)
    (* isset($var->thing) but not isset($foo->$bar) *)
    | (_, _, Obj_get (_, (_, _, Id _), _, Is_prop))
    (* isset($var::thing) but not isset($foo::$bar) *)
    | (_, _, Class_get (_, CGexpr (_, _, Id _), _))
      when String.equal pseudo_func SN.PseudoFunctions.isset ->
      Typing_error_utils.add_typing_error
        ~env:(Tast_env.tast_env_as_typing_env env)
        Typing_error.(primary @@ Primary.Isset_in_strict p)
    | _ -> ()
  end
  | _ -> ()

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env (_, p, x) =
      disallow_isset_inout_args_check env p x;
      well_formed_isset_argument_check env p x
  end
