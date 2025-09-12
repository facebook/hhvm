(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type env = {
  in_final_class: bool;
  class_name: string option;
  parent_name: string option;
  is_trait: bool;
}

let handler custom_err_config =
  object
    inherit [env] Stateful_aast_visitor.default_nast_visitor_with_state

    method initial_state =
      {
        in_final_class = false;
        class_name = None;
        parent_name = None;
        is_trait = false;
      }

    method! at_class_ _ c =
      let parent_name =
        match c.Aast.c_extends with
        | (_, Aast.Happly ((_, parent_name), _)) :: _ -> Some parent_name
        | _ -> None
      in
      let is_trait = Ast_defs.is_c_trait c.Aast.c_kind in
      {
        in_final_class = c.Aast.c_final;
        class_name = Some (snd c.Aast.c_name);
        parent_name;
        is_trait;
      }

    method! at_expr env (_, _, e) =
      let () =
        match e with
        | Aast.FunctionPointer (Aast.FP_id (pos, name), _) ->
          if Naming_special_names.SpecialFunctions.is_special_function name then
            Errors.add_error
              (Naming_error_utils.to_user_error
                 (Naming_error.Invalid_fun_pointer { pos; name })
                 custom_err_config)
        | Aast.FunctionPointer
            (Aast.FP_class_const ((_, p, Aast.CIself), (_, meth_name)), _) ->
          if not env.in_final_class then
            if env.is_trait then
              Errors.add_error
                (Naming_error_utils.to_user_error
                   (Naming_error.Self_in_non_final_function_pointer
                      { pos = p; class_name = None; meth_name })
                   custom_err_config)
            else
              Errors.add_error
                (Naming_error_utils.to_user_error
                   (Naming_error.Self_in_non_final_function_pointer
                      { pos = p; class_name = env.class_name; meth_name })
                   custom_err_config)
        | Aast.FunctionPointer
            (Aast.FP_class_const ((_, p, Aast.CIparent), (_, meth_name)), _) ->
          Errors.add_error
            (Naming_error_utils.to_user_error
               (Naming_error.Parent_in_function_pointer
                  { pos = p; parent_name = env.parent_name; meth_name })
               custom_err_config)
        | _ -> ()
      in
      env
  end
