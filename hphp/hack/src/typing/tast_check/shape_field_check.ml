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

(* Returns the status `DoesExists, `DoesNotExist or `Unknown along with
 * a boolean to track if the shape was optional or not.
 *)
let shapes_key_exists env shape field_name =
  let check pos shape_kind fields =
    match TShapeMap.find_opt field_name fields with
    | None ->
      begin
        match shape_kind with
        | Closed_shape -> `DoesNotExist (pos, `Undefined)
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
              `Nothing (Reason.to_string "It is nothing" (get_reason sft_ty)) )
        else
          `Unknown
  in
  let (_, shape) = Tast_env.expand_type env shape in
  match get_node shape with
  | Tshape (shape_kind, fields) ->
    (check (get_pos shape) shape_kind fields, false)
  | Toption maybe_shape ->
    let (_, shape) = Tast_env.expand_type env maybe_shape in
    (match get_node shape with
    | Tshape (shape_kind, fields) ->
      (check (get_pos shape) shape_kind fields, true)
    | _ -> (`Unknown, true))
  | _ -> (`Unknown, false)

let trivial_shapes_key_exists_check pos1 env ((_, shape), _) field_name =
  let (status, optional) =
    shapes_key_exists env shape (TSFlit_str field_name)
  in
  (* Shapes::keyExists only supports non optional shapes, so an error
   * would already have been raised.
   *)
  if not optional then
    match status with
    | `DoesExist pos2 ->
      Errors.shapes_key_exists_always_true pos1 (snd field_name) pos2
    | `DoesNotExist (pos2, reason) ->
      Errors.shapes_key_exists_always_false pos1 (snd field_name) pos2 reason
    | `Unknown -> ()

let shapes_method_access_with_non_existent_field
    pos1 env method_name ((_, shape), _) field_name =
  let (status, optional_shape) =
    shapes_key_exists env shape (TSFlit_str field_name)
  in
  match (status, optional_shape) with
  | (`DoesExist _, false) ->
    Lint.shape_idx_access_required_field pos1 (snd field_name)
  | (`DoesNotExist (pos2, reason), false) ->
    Errors.shapes_method_access_with_non_existent_field
      pos1
      (snd field_name)
      pos2
      method_name
      reason
  | (`DoesNotExist _, true) when String.equal method_name SN.Shapes.idx ->
    (* Shapes::at only supports non optional shapes *)
    Lint.opt_closed_shape_idx_missing_field (Some method_name) pos1
  | (`DoesExist _, true)
  | (`DoesNotExist _, true)
  | (`Unknown, _) ->
    ()

let shape_access_with_non_existent_field pos1 env ((_, shape), _) field_name =
  let (status, optional) =
    shapes_key_exists env shape (TSFlit_str field_name)
  in
  match (status, optional) with
  | (`DoesNotExist (pos2, reason), false) ->
    Errors.shape_access_with_non_existent_field
      pos1
      (snd field_name)
      pos2
      reason
  | (`DoesNotExist _, true) -> Lint.opt_closed_shape_idx_missing_field None pos1
  | (`DoesExist _, _)
  | (`Unknown, _) ->
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
