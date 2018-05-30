(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Ast_defs
open Tast
open Typing_defs

module Aast = Aast_defs
module Env = Tast_env
module SN = Naming_special_names

let trivial_equality_check p bop env ((_, ty1), _ as te1 : expr) ((_, ty2), _ as te2 : expr) =
  begin match te1, te2 with
  | (_, Null), ((_, ty), _) | ((_, ty), _), (_, Null) ->
    Tast_env.assert_nullable p bop env ty
  | _ -> ()
  end;
  Tast_env.assert_nontrivial p bop env ty1 ty2

let shapes_key_exists env shape field_name =
  let _, shape = Env.expand_type env shape in
  match shape with
  | r, Tshape (fields_known, fields) ->
    begin match fields_known with
    | FieldsFullyKnown ->
      begin match ShapeMap.get field_name fields with
      | None -> `DoesNotExist (Reason.to_pos r, `Undefined)
      | Some {sft_optional; sft_ty} ->
        if sft_optional
        then `Unknown
        else `DoesExist (Reason.to_pos (fst sft_ty))
      end
    | FieldsPartiallyKnown unset_fields ->
      if ShapeMap.mem field_name unset_fields
      then `DoesNotExist (Reason.to_pos r, `Unset)
      else
        begin match ShapeMap.get field_name fields with
        | None -> `Unknown
        | Some {sft_optional; sft_ty} ->
          if sft_optional
          then `Unknown
          else `DoesExist (Reason.to_pos (fst sft_ty))
        end
    end
  | _ -> `Unknown

let trivial_shapes_key_exists_check pos1 env ((_, shape), _) field_name =
  let printable_field_name =
    Typing_utils.get_printable_shape_field_name field_name in
  match shapes_key_exists env shape field_name with
  | `DoesExist pos2 ->
    Errors.shapes_key_exists_always_true pos1 printable_field_name pos2
  | `DoesNotExist (pos2, reason) ->
    Errors.shapes_key_exists_always_false pos1 printable_field_name pos2 reason
  | `Unknown -> ()

let handler = object
  inherit Tast_visitor.handler_base

  method! at_expr env = function
    | (p, _), Binop ((EQeqeq | Diff2) as bop, te1, te2) ->
      trivial_equality_check p bop env te1 te2
    | (p, _), Call (Aast.Cnormal, (_, Class_const ((_, CI ((_, class_name), _)), (_, method_name))), _, [shape; _, String field_name], [])
      when
        class_name = SN.Shapes.cShapes &&
        method_name = SN.Shapes.keyExists &&
        TypecheckerOptions.experimental_feature_enabled
          (Tast_env.get_tcopt env)
          TypecheckerOptions.experimental_shape_field_check ->
      trivial_shapes_key_exists_check p env shape (SFlit field_name)
    | _ -> ()
end
