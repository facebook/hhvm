(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module SN = Naming_special_names

let vec_classes =
  SSet.of_list
    [
      SN.Collections.cVec;
      SN.Collections.cVector;
      SN.Collections.cImmVector;
      SN.Collections.cConstVector;
    ]

let classify_type env ty =
  let ty = Tast_env.strip_dynamic env ty in
  let (_, ty) = Tast_env.strip_supportdyn env ty in
  match Typing_defs.get_node ty with
  | Typing_defs.Ttuple _ -> (Analyze_list_logger_types.Tuple, ty)
  | Typing_defs.Tclass ((_, name), _, _) ->
    if String.equal name SN.Collections.cPair then
      (Analyze_list_logger_types.Pair, ty)
    else if SSet.mem name vec_classes then
      (Analyze_list_logger_types.Vec, ty)
    else
      (Analyze_list_logger_types.Misc, ty)
  | Typing_defs.Tunion _ -> (Analyze_list_logger_types.Union, ty)
  | Typing_defs.Tintersection _ -> (Analyze_list_logger_types.Intersection, ty)
  | Typing_defs.Tdynamic -> (Analyze_list_logger_types.Dynamic, ty)
  | Typing_defs.Tgeneric _ -> (Analyze_list_logger_types.Generic, ty)
  | _ -> (Analyze_list_logger_types.Misc, ty)

let ty_node_kind ty =
  match Typing_defs.get_node ty with
  | Typing_defs.Tany _ -> "Tany"
  | Typing_defs.Tnonnull -> "Tnonnull"
  | Typing_defs.Tdynamic -> "Tdynamic"
  | Typing_defs.Toption _ -> "Toption"
  | Typing_defs.Tprim _ -> "Tprim"
  | Typing_defs.Tfun _ -> "Tfun"
  | Typing_defs.Ttuple _ -> "Ttuple"
  | Typing_defs.Tshape _ -> "Tshape"
  | Typing_defs.Tgeneric _ -> "Tgeneric"
  | Typing_defs.Tunion _ -> "Tunion"
  | Typing_defs.Tintersection _ -> "Tintersection"
  | Typing_defs.Tvec_or_dict _ -> "Tvec_or_dict"
  | Typing_defs.Taccess _ -> "Taccess"
  | Typing_defs.Tclass_ptr _ -> "Tclass_ptr"
  | Typing_defs.Tvar _ -> "Tvar"
  | Typing_defs.Tnewtype _ -> "Tnewtype"
  | Typing_defs.Tdependent _ -> "Tdependent"
  | Typing_defs.Tclass _ -> "Tclass"
  | Typing_defs.Tneg _ -> "Tneg"
  | Typing_defs.Tlabel _ -> "Tlabel"

let rec has_complex_lvalue el =
  List.exists el ~f:(fun (_, _, e) ->
      match e with
      | Aast.Lvar _
      | Aast.Omitted
      | Aast.Lplaceholder _ ->
        false
      | Aast.List sub_el -> has_complex_lvalue sub_el
      | Aast.ReadonlyExpr (_, _, inner_e) ->
        (match inner_e with
        | Aast.Lvar _ -> false
        | _ -> true)
      | _ -> true)

(** Extract original source text from a position *)
let source_text_of_pos (file : Relative_path.t) (pos : Pos.t) : string =
  match File_provider.get_contents file with
  | Some content ->
    (try Pos.get_text_from_pos ~content pos with
    | _ -> "<source unavailable>")
  | None -> "<source unavailable>"

(** Classify the kind of a non-trivial lvalue expression *)
let lvalue_kind (_, _, e) =
  match e with
  | Aast.Array_get (_, None) -> Analyze_list_logger_types.Array_append
  | Aast.Array_get _ -> Analyze_list_logger_types.Array_get
  | Aast.Obj_get _ -> Analyze_list_logger_types.Obj_get
  | Aast.Class_get _ -> Analyze_list_logger_types.Class_get
  | Aast.Call _ -> Analyze_list_logger_types.Call
  | Aast.ReadonlyExpr _ -> Analyze_list_logger_types.ReadonlyExpr
  | _ -> Analyze_list_logger_types.Other

(** Convert a class_id_ to receiver_kind *)
let receiver_kind_of_class_id_ = function
  | Aast.CIself -> Analyze_list_logger_types.Self
  | Aast.CIstatic -> Analyze_list_logger_types.Static
  | Aast.CIparent -> Analyze_list_logger_types.Parent
  | Aast.CI _ -> Analyze_list_logger_types.Explicit
  | Aast.CIexpr _ -> Analyze_list_logger_types.Expr

(** Check whether an lvalue expression involves a function call anywhere
    in its sub-expression chain (e.g. foo()[3] has Array_get at the top
    but involves a call as the base expression) *)
let rec involves_call (_, _, e) =
  match e with
  | Aast.Call _ -> true
  | Aast.Array_get (base, _) -> involves_call base
  | Aast.Obj_get (base, _, _, _) -> involves_call base
  | Aast.ReadonlyExpr inner -> involves_call inner
  | _ -> false

let pos_info_of_pos p =
  let p = Pos.to_relative_string p in
  let (line, char_start, char_end) = Pos.info_pos p in
  {
    Analyze_list_logger_types.filename = Pos.filename p;
    line;
    char_start;
    char_end;
  }

(** Count total leaf lvalue positions in a list() expression.
    Nested List nodes are recursed into (not counted themselves).
    Everything else counts as one position. *)
let rec count_lvalue_positions el =
  List.fold el ~init:0 ~f:(fun count (_, _, e) ->
      match e with
      | Aast.List sub_el -> count + count_lvalue_positions sub_el
      | _ -> count + 1)

(** Extract the receiver_kind for a Class_get expression *)
let class_get_receiver_kind (_, _, ci) = Some (receiver_kind_of_class_id_ ci)

(** Collect info about non-trivial lvalue elements in a list() expression.
    Skips Lvar, Omitted, and recursively descends into nested List.
    Uses original source text from [file] for lvalue_code. *)
let rec collect_complex_lvalues file acc el =
  List.fold el ~init:acc ~f:(fun acc ((_, p, e) as expr) ->
      match e with
      | Aast.Lvar _
      | Aast.Omitted
      | Aast.Lplaceholder _ ->
        acc
      | Aast.List sub_el -> collect_complex_lvalues file acc sub_el
      | Aast.ReadonlyExpr ((_, _, inner_e) as inner) ->
        (match inner_e with
        | Aast.Lvar _ -> acc
        | _ ->
          let lvalue_receiver_kind =
            match inner_e with
            | Aast.Class_get (class_id, _, _) ->
              class_get_receiver_kind class_id
            | _ -> None
          in
          let elem =
            {
              Analyze_list_logger_types.lvalue_kind = lvalue_kind inner;
              lvalue_pos = pos_info_of_pos p;
              lvalue_code = source_text_of_pos file p;
              lvalue_involves_call = involves_call inner;
              lvalue_receiver_kind;
            }
          in
          elem :: acc)
      | _ ->
        let lvalue_receiver_kind =
          match e with
          | Aast.Class_get (class_id, _, _) -> class_get_receiver_kind class_id
          | _ -> None
        in
        let elem =
          {
            Analyze_list_logger_types.lvalue_kind = lvalue_kind expr;
            lvalue_pos = pos_info_of_pos p;
            lvalue_code = source_text_of_pos file p;
            lvalue_involves_call = involves_call expr;
            lvalue_receiver_kind;
          }
        in
        elem :: acc)

(* Level 1: write to out_channel for test capture and structured pipeline consumption.
   Level > 1: write to Hh_logger for server logs (used in WWW runs). *)
let log_output level output =
  if level > 1 then
    Hh_logger.log "%s" output
  else begin
    Printf.sprintf "%s\n" output
    |> Out_channel.output_string !Typing_log.out_channel;
    Out_channel.flush !Typing_log.out_channel
  end

let create_handler ctx =
  let level =
    Provider_context.get_tcopt ctx
    |> TypecheckerOptions.log_levels
    |> SMap.find_opt "list_logger"
    |> Option.value ~default:1
  in
  object
    inherit Tast_visitor.handler_base

    method! at_expr env (_, _, e) =
      match e with
      | Aast.Assign (((_, lhs_pos, Aast.List el) as _lhs), None, rhs_expr) ->
        let rhs_ty = Tast.get_type rhs_expr in
        let (bucket, stripped_ty) = classify_type env rhs_ty in
        let type_string = Tast_env.print_ty env rhs_ty in
        let complex = has_complex_lvalue el in
        let file = Tast_env.get_file env in
        let complex_lvalue_elements =
          if complex then
            List.rev (collect_complex_lvalues file [] el)
          else
            []
        in
        let (_, rhs_pos, _) = rhs_expr in
        let lhs_code = source_text_of_pos file lhs_pos in
        let total_lvalue_count = count_lvalue_positions el in
        let is_generated = Tast_logger_util.is_generated lhs_pos in
        let entry : Analyze_list_logger_types.t =
          {
            bucket;
            type_string;
            has_complex_lvalue = complex;
            pos = pos_info_of_pos rhs_pos;
            lhs_code;
            total_lvalue_count;
            is_generated;
            ty_node_kind =
              (match bucket with
              | Misc -> Some (ty_node_kind stripped_ty)
              | _ -> None);
            complex_lvalue_elements;
          }
        in
        let output =
          Printf.sprintf
            "@list_logger:%s"
            (Analyze_list_logger_types.to_json_string entry)
        in
        log_output level output
      | _ -> ()
  end
