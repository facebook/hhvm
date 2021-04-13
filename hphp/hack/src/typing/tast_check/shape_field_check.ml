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
module SN = Naming_special_names

let shapes_key_exists env shape field_name =
  let (_, shape) = Tast_env.expand_type env shape in
  match get_node shape with
  | Tshape (shape_kind, fields) ->
    begin
      match TShapeMap.find_opt field_name fields with
      | None ->
        begin
          match shape_kind with
          | Closed_shape -> `DoesNotExist (get_pos shape, `Undefined)
          | Open_shape -> `Unknown
        end
      | Some { sft_optional; sft_ty } ->
        if not sft_optional then
          `DoesExist (get_pos sft_ty)
        else
          let nothing = Typing_make_type.nothing Reason.Rnone in
          if Tast_env.is_sub_type env sft_ty nothing then
            `DoesNotExist
              ( get_pos shape,
                `Nothing (Reason.to_string "It is nothing" (get_reason sft_ty))
              )
          else
            `Unknown
    end
  | _ -> `Unknown

let trivial_shapes_key_exists_check pos1 env ((_, shape), _) field_name =
  match shapes_key_exists env shape (TSFlit_str field_name) with
  | `DoesExist pos2 ->
    Errors.shapes_key_exists_always_true pos1 (snd field_name) pos2
  | `DoesNotExist (pos2, reason) ->
    Errors.shapes_key_exists_always_false pos1 (snd field_name) pos2 reason
  | `Unknown -> ()

let shapes_method_access_with_non_existent_field
    pos1 env method_name ((_, shape), _) field_name =
  match shapes_key_exists env shape (TSFlit_str field_name) with
  | `DoesExist _ -> Lint.shape_idx_access_required_field pos1 (snd field_name)
  | `DoesNotExist (pos2, reason) ->
    Errors.shapes_method_access_with_non_existent_field
      pos1
      (snd field_name)
      pos2
      method_name
      reason
  | `Unknown -> ()

let shape_access_with_non_existent_field pos1 env ((_, shape), _) field_name =
  match shapes_key_exists env shape (TSFlit_str field_name) with
  | `DoesNotExist (pos2, reason) ->
    Errors.shape_access_with_non_existent_field
      pos1
      (snd field_name)
      pos2
      reason
  | `DoesExist _
  | `Unknown ->
    ()

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      function
      | ( (p, _),
          Call
            ( (_, Class_const ((_, CI (_, class_name)), (_, method_name))),
              _,
              [shape; ((pos, _), String field_name)],
              None ) )
        when String.equal class_name SN.Shapes.cShapes
             && String.equal method_name SN.Shapes.keyExists ->
        trivial_shapes_key_exists_check
          p
          env
          shape
          (Pos_or_decl.of_raw_pos pos, field_name)
      | ( (p, _),
          Call
            ( (_, Class_const ((_, CI (_, class_name)), (_, method_name))),
              _,
              [shape; ((pos, _), String field_name); _],
              None ) )
      | ( (p, _),
          Call
            ( (_, Class_const ((_, CI (_, class_name)), (_, method_name))),
              _,
              [shape; ((pos, _), String field_name)],
              None ) )
        when String.equal class_name SN.Shapes.cShapes
             && ( String.equal method_name SN.Shapes.idx
                || String.equal method_name SN.Shapes.at ) ->
        shapes_method_access_with_non_existent_field
          p
          env
          method_name
          shape
          (Pos_or_decl.of_raw_pos pos, field_name)
      | ( (p, _),
          Binop
            ( Ast_defs.QuestionQuestion,
              (_, Array_get (shape, Some ((pos, _), String field_name))),
              _ ) ) ->
        shape_access_with_non_existent_field
          p
          env
          shape
          (Pos_or_decl.of_raw_pos pos, field_name)
      | _ -> ()
  end
