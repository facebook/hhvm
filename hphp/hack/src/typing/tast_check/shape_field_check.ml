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
      if is_nothing shape_kind then
        `DoesNotExist (pos, `Undefined)
      else
        `Unknown
    | Some { sft_optional; sft_ty } ->
      if not sft_optional then
        `DoesExist (get_pos sft_ty)
      else
        let nothing = Typing_make_type.nothing Reason.Rnone in
        if Tast_env.is_sub_type env sft_ty nothing then
          `DoesNotExist
            ( get_pos shape,
              `Nothing
                (lazy (Reason.to_string "It is nothing" (get_reason sft_ty))) )
        else
          `Unknown
  in
  let tenv = Tast_env.tast_env_as_typing_env env in
  let shape = Typing_utils.strip_dynamic tenv shape in
  let (_, _, shape) = Typing_utils.strip_supportdyn tenv shape in
  let (_, shape) = Tast_env.expand_type env shape in
  match get_node shape with
  | Tshape { s_origin = _; s_unknown_value = shape_kind; s_fields = fields } ->
    (check (get_pos shape) shape_kind fields, false)
  | Toption maybe_shape ->
    let (_, shape) = Tast_env.expand_type env maybe_shape in
    (match get_node shape with
    | Tshape { s_origin = _; s_unknown_value = shape_kind; s_fields = fields }
      ->
      (check (get_pos shape) shape_kind fields, true)
    | _ -> (`Unknown, true))
  | _ -> (`Unknown, false)

let trivial_shapes_key_exists_check pos1 env (shape, _, _) field_name =
  let (status, optional) =
    shapes_key_exists env shape (TSFlit_str field_name)
  in
  (* Shapes::keyExists only supports non optional shapes, so an error
   * would already have been raised.
   *)
  if not optional then
    match status with
    | `DoesExist decl_pos ->
      Typing_error_utils.add_typing_error
        ~env:(Tast_env.tast_env_as_typing_env env)
        Typing_error.(
          shape
          @@ Primary.Shape.Shapes_key_exists_always_true
               { pos = pos1; field_name = snd field_name; decl_pos })
    | `DoesNotExist (decl_pos, reason) ->
      Typing_error_utils.add_typing_error
        ~env:(Tast_env.tast_env_as_typing_env env)
        Typing_error.(
          shape
          @@ Primary.Shape.Shapes_key_exists_always_false
               { pos = pos1; field_name = snd field_name; decl_pos; reason })
    | `Unknown -> ()

let shapes_method_access_with_non_existent_field
    pos1 env method_name (shape, _, _) field_name =
  let (status, optional_shape) =
    shapes_key_exists env shape (TSFlit_str field_name)
  in
  match (status, optional_shape) with
  | (`DoesExist _, false) ->
    Lint.shape_idx_access_required_field pos1 (snd field_name)
  | (`DoesNotExist (decl_pos, reason), _) ->
    Typing_error_utils.add_typing_error
      ~env:(Tast_env.tast_env_as_typing_env env)
      Typing_error.(
        shape
        @@ Primary.Shape.Shapes_method_access_with_non_existent_field
             {
               pos = pos1;
               field_name = snd field_name;
               decl_pos;
               method_name;
               reason;
             })
  | (`DoesExist _, true)
  | (`Unknown, _) ->
    ()

let shape_access_with_non_existent_field pos1 env (shape, _, _) field_name =
  let (status, optional) =
    shapes_key_exists env shape (TSFlit_str field_name)
  in
  match (status, optional) with
  | (`DoesNotExist (decl_pos, reason), _) ->
    Typing_error_utils.add_typing_error
      ~env:(Tast_env.tast_env_as_typing_env env)
      Typing_error.(
        shape
        @@ Primary.Shape.Shapes_access_with_non_existent_field
             { pos = pos1; field_name = snd field_name; decl_pos; reason })
  | (`DoesExist _, _)
  | (`Unknown, _) ->
    ()

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      function
      | ( _,
          p,
          Call
            {
              func =
                ( _,
                  _,
                  Class_const ((_, _, CI (_, class_name)), (_, method_name)) );
              args = [(_, shape); (_, (_, pos, String field_name))];
              unpacked_arg = None;
              _;
            } )
        when String.equal class_name SN.Shapes.cShapes
             && String.equal method_name SN.Shapes.keyExists ->
        trivial_shapes_key_exists_check
          p
          env
          shape
          (Pos_or_decl.of_raw_pos pos, field_name)
      | ( _,
          _,
          Call
            {
              func =
                ( _,
                  _,
                  Class_const ((_, _, CI (_, class_name)), (_, method_name)) );
              args = [(_, shape); (_, (_, pos, String field_name)); _];
              unpacked_arg = None;
              _;
            } )
      | ( _,
          _,
          Call
            {
              func =
                ( _,
                  _,
                  Class_const ((_, _, CI (_, class_name)), (_, method_name)) );
              args = [(_, shape); (_, (_, pos, String field_name))];
              unpacked_arg = None;
              _;
            } )
        when String.equal class_name SN.Shapes.cShapes
             && (String.equal method_name SN.Shapes.idx
                || String.equal method_name SN.Shapes.at) ->
        shapes_method_access_with_non_existent_field
          pos
          env
          method_name
          shape
          (Pos_or_decl.of_raw_pos pos, field_name)
      | (_, p, Binop { bop = Ast_defs.QuestionQuestion; lhs = exp; _ }) ->
        let rec check_nested_accesses = function
          | (_, _, Array_get (exp, Some (_, pos, String field_name))) ->
            shape_access_with_non_existent_field
              p
              env
              exp
              (Pos_or_decl.of_raw_pos pos, field_name);
            check_nested_accesses exp
          | _ -> ()
        in
        check_nested_accesses exp
      | _ -> ()
  end
