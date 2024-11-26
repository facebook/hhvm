(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Typing_defs
open Aast_defs

let error env level pos cls_name =
  if level > 1 then
    Typing_error_utils.add_typing_error
      ~env
      Typing_error.(
        primary @@ Primary.Class_pointer_to_string { pos; cls_name })
  (* error *)
  else
    Lint.class_pointer_to_string pos cls_name

let string_of_class_id_ = function
  | CIparent -> "parent"
  | CIself -> "self"
  | CIstatic -> "static"
  | CIexpr _ -> "" (* <expr>::class is banned *)
  | CI (_, name) -> name

let check_string_coercion_point env ~flag expr ty =
  if env.Typing_env_types.emit_string_coercion_error then
    let level =
      TypecheckerOptions.class_pointer_level (Typing_env.get_tcopt env) flag
    in
    if level > 0 then
      match expr with
      | (_, pos, Class_const ((_, _, cid_), (_, cls)))
        when cls = Naming_special_names.Members.mClass -> begin
        match cid_ with
        | CI (_, name) when Typing_env.get_reified env name = Reified ->
          () (* nameof T is illegal (T187575261) *)
        | _ ->
          let check ty =
            match get_node ty with
            | Tprim Tstring -> error env level pos (string_of_class_id_ cid_)
            | _ -> ()
          in
          let (_env, ty) = Typing_dynamic_utils.strip_dynamic env ty in
          begin
            match get_node ty with
            | Toption ty -> check ty
            | _ -> check ty
          end
      end
      | _ -> ()
