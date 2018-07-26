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
module SN = Naming_special_names

let shapes_key_exists env shape field_name =
  let _, shape = Tast_env.expand_type env shape in
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
  match shapes_key_exists env shape (SFlit_str field_name) with
  | `DoesExist pos2 ->
    Errors.shapes_key_exists_always_true pos1 (snd field_name) pos2
  | `DoesNotExist (pos2, reason) ->
    Errors.shapes_key_exists_always_false pos1 (snd field_name) pos2 reason
  | `Unknown -> ()

let shapes_idx_invalid_key_check pos1 env ((_, shape), _) field_name =
  match shapes_key_exists env shape (SFlit_str field_name) with
  | `DoesExist _ ->
    Lint.shape_idx_access_required_field pos1 (snd field_name)
  | `DoesNotExist (pos2, reason) ->
    Errors.shapes_idx_with_non_existent_field pos1 (snd field_name) pos2 reason
  | `Unknown -> ()

let handler = object
  inherit Tast_visitor.handler_base

  method! minimum_forward_compat_level = 2018_05_31

  method! at_expr env =
    function
    | (p, _), Call (Aast.Cnormal, (_, Class_const ((_, CI ((_, class_name), _)), (_, method_name))), _, [shape; (pos, _), String field_name], [])
      when
        class_name = SN.Shapes.cShapes &&
        method_name = SN.Shapes.keyExists ->
      trivial_shapes_key_exists_check p env shape (pos, field_name)
    | (p, _), Call (Aast.Cnormal, (_, Class_const ((_, CI ((_, class_name), _)), (_, method_name))), _, [shape; (pos, _), String field_name], [])
      when
        class_name = SN.Shapes.cShapes &&
        method_name = SN.Shapes.idx ->
      shapes_idx_invalid_key_check p env shape (pos, field_name)
    | _ -> ()
end
