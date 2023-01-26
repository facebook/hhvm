(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module SN = Naming_special_names

module Env = struct
  let in_class t Aast.{ c_name; c_kind; c_final; _ } =
    Naming_phase_env.
      {
        t with
        elab_shape_field_name =
          Elab_shape_field_name.
            { current_class = Some (c_name, c_kind, c_final) };
      }

  let current_class
      Naming_phase_env.
        { elab_shape_field_name = Elab_shape_field_name.{ current_class }; _ } =
    current_class
end

let on_class_ (env, c) = Ok (Env.in_class env c, c)

(* We permit class constants to be used as shape field names. Here we replace
    uses of `self` with the class to which they refer or `unknown` if the shape
   is not defined within the context of a class *)
let canonical_shape_name current_class sfld =
  match sfld with
  | Ast_defs.SFclass_const ((class_pos, class_name), cst)
    when String.equal class_name SN.Classes.cSelf ->
    (match current_class with
    | Some ((_, class_name), _, _) ->
      Ok (Ast_defs.SFclass_const ((class_pos, class_name), cst))
    | None ->
      let err =
        Naming_phase_error.typing
        @@ Typing_error.Primary.Self_outside_class class_pos
      in
      Error (Ast_defs.SFclass_const ((class_pos, SN.Classes.cUnknown), cst), err))
  | _ -> Ok sfld

let on_expr_ on_error =
  let handler
        : 'a 'b.
          Naming_phase_env.t * ('a, 'b) Aast_defs.expr_ ->
          (Naming_phase_env.t * ('a, 'b) Aast_defs.expr_, _) result =
   fun (env, expr_) ->
    let (expr_, errs) =
      match expr_ with
      | Aast.Shape fdl ->
        let (fdl, err_opts) =
          List.unzip
          @@ List.map fdl ~f:(fun (nm, v) ->
                 let (nm, err_opt) =
                   match canonical_shape_name (Env.current_class env) nm with
                   | Ok nm -> (nm, None)
                   | Error (nm, err) -> (nm, Some err)
                 in
                 ((nm, v), err_opt))
        in
        let err =
          List.fold_right err_opts ~init:[] ~f:(fun err_opt acc ->
              Option.value_map err_opt ~default:acc ~f:(fun err -> err :: acc))
        in
        (Aast.Shape fdl, err)
      | _ -> (expr_, [])
    in
    List.iter ~f:on_error errs;
    Ok (env, expr_)
  in
  handler

let on_shape_field_info on_error (env, (Aast.{ sfi_name; _ } as sfi)) =
  match canonical_shape_name (Env.current_class env) sfi_name with
  | Ok sfi_name -> Ok (env, Aast.{ sfi with sfi_name })
  | Error (sfi_name, err) ->
    on_error err;
    Error (env, Aast.{ sfi with sfi_name })

let top_down_pass =
  Naming_phase_pass.(
    top_down Ast_transform.{ identity with on_class_ = Some on_class_ })

let bottom_up_pass on_error =
  Naming_phase_pass.(
    bottom_up
      Ast_transform.
        {
          identity with
          on_expr_ = Some (on_expr_ on_error);
          on_shape_field_info = Some (on_shape_field_info on_error);
        })
