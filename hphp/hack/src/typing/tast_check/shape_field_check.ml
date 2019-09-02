(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Ast_defs
open Aast
open Typing_defs
module Aast = Aast_defs
module SN = Naming_special_names

let shapes_key_exists env shape field_name =
  let (_, shape) = Tast_env.expand_type env shape in
  match shape with
  | (r, Tshape (shape_kind, fields)) ->
    begin
      match ShapeMap.get field_name fields with
      | None ->
        begin
          match shape_kind with
          | Closed_shape -> `DoesNotExist (Reason.to_pos r, `Undefined)
          | Open_shape -> `Unknown
        end
      | Some { sft_optional; sft_ty } ->
        if not sft_optional then
          `DoesExist (Reason.to_pos (fst sft_ty))
        else
          let nothing = Typing_make_type.nothing Reason.Rnone in
          if Tast_env.is_sub_type env sft_ty nothing then
            `DoesNotExist
              ( Reason.to_pos r,
                `Nothing (Reason.to_string "It is nothing" (fst sft_ty)) )
          else
            `Unknown
    end
  | _ -> `Unknown

let trivial_shapes_key_exists_check pos1 env ((_, shape), _) field_name =
  match shapes_key_exists env shape (SFlit_str field_name) with
  | `DoesExist pos2 ->
    Errors.shapes_key_exists_always_true pos1 (snd field_name) pos2
  | `DoesNotExist (pos2, reason) ->
    Errors.shapes_key_exists_always_false pos1 (snd field_name) pos2 reason
  | `Unknown -> ()

let shapes_method_access_with_non_existent_field
    pos1 env method_name ((_, shape), _) field_name =
  match shapes_key_exists env shape (SFlit_str field_name) with
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
  match shapes_key_exists env shape (SFlit_str field_name) with
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
            ( Aast.Cnormal,
              (_, Class_const ((_, CI (_, class_name)), (_, method_name))),
              _,
              [shape; ((pos, _), String field_name)],
              [] ) )
        when class_name = SN.Shapes.cShapes
             && method_name = SN.Shapes.keyExists ->
        trivial_shapes_key_exists_check p env shape (pos, field_name)
      | ( (p, _),
          Call
            ( Aast.Cnormal,
              (_, Class_const ((_, CI (_, class_name)), (_, method_name))),
              _,
              [shape; ((pos, _), String field_name); _],
              [] ) )
      | ( (p, _),
          Call
            ( Aast.Cnormal,
              (_, Class_const ((_, CI (_, class_name)), (_, method_name))),
              _,
              [shape; ((pos, _), String field_name)],
              [] ) )
        when class_name = SN.Shapes.cShapes
             && (method_name = SN.Shapes.idx || method_name = SN.Shapes.at) ->
        shapes_method_access_with_non_existent_field
          p
          env
          method_name
          shape
          (pos, field_name)
      | ( (p, _),
          Binop
            ( Ast_defs.QuestionQuestion,
              (_, Array_get (shape, Some ((pos, _), String field_name))),
              _ ) ) ->
        shape_access_with_non_existent_field p env shape (pos, field_name)
      | _ -> ()
  end
