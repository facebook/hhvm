(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 * JSON builder functions. These all return JSON objects, which
 * may be used to build up larger objects. The functions with suffix
 * _nested include the key field because they are used for writing
 * nested facts.
 *)

open Aast
open Ast_defs
open Hh_json
open Hh_prelude
open Namespace_env
open Symbol_json_util

let build_id_json fact_id =
  JSON_Object [("id", JSON_Number (string_of_int fact_id))]

let build_comment_json_nested comment =
  JSON_Object [("key", JSON_String comment)]

let build_file_json_nested filepath =
  JSON_Object [("key", JSON_String filepath)]

let build_name_json_nested name =
  (* Remove leading slash, if present, so names such as
  Exception and \Exception are captured by the same fact *)
  let basename = Utils.strip_ns name in
  JSON_Object [("key", JSON_String basename)]

let rec build_namespaceqname_json_nested ns =
  let fields =
    match split_name ns with
    | None -> [("name", build_name_json_nested ns)]
    | Some (parent_ns, namespace) ->
      [
        ("name", build_name_json_nested namespace);
        ("parent", build_namespaceqname_json_nested parent_ns);
      ]
  in
  JSON_Object [("key", JSON_Object fields)]

let build_qname_json_nested qname =
  let fields =
    match split_name qname with
    (* Global namespace *)
    | None -> [("name", build_name_json_nested qname)]
    | Some (ns, name) ->
      [
        ("name", build_name_json_nested name);
        ("namespace_", build_namespaceqname_json_nested ns);
      ]
  in
  JSON_Object [("key", JSON_Object fields)]

(* Returns a singleton list containing the JSON field if there
is a non-empty namespace in the nsenv, or else an empty list *)
let build_namespace_decl_json_nested nsenv =
  match nsenv.ns_name with
  | None -> [] (* Global namespace *)
  | Some "" -> []
  | Some ns ->
    [
      ( "namespace_",
        JSON_Object
          [("key", JSON_Object [("name", build_namespaceqname_json_nested ns)])]
      );
    ]

let build_type_json_nested type_name =
  (* Remove namespace slash from type, if present *)
  let ty = Utils.strip_ns type_name in
  JSON_Object [("key", JSON_String ty)]

let build_signature_json_nested parameters return_type_name =
  let fields =
    let params = [("parameters", JSON_Array parameters)] in
    match return_type_name with
    | None -> params
    | Some ty -> ("returns", build_type_json_nested ty) :: params
  in
  JSON_Object [("key", JSON_Object fields)]

let build_attributes_json_nested source_map attrs =
  let attributes =
    List.map attrs ~f:(fun attr ->
        let (_, name) = attr.ua_name in
        let params =
          List.fold_right attr.ua_params ~init:[] ~f:(fun ((pos, _), _) acc ->
              let fp = Relative_path.to_absolute (Pos.filename pos) in
              match SMap.find_opt fp source_map with
              | Some st -> JSON_String (source_at_span st pos) :: acc
              | None -> acc)
        in
        let fields =
          [
            ("name", build_name_json_nested name);
            ("parameters", JSON_Array params);
          ]
        in
        JSON_Object [("key", JSON_Object fields)])
  in
  JSON_Array attributes

let build_bytespan_json pos =
  let start = fst (Pos.info_raw pos) in
  let length = Pos.length pos in
  JSON_Object
    [
      ("start", JSON_Number (string_of_int start));
      ("length", JSON_Number (string_of_int length));
    ]

let build_rel_bytespan_json offset len =
  JSON_Object
    [
      ("offset", JSON_Number (string_of_int offset));
      ("length", JSON_Number (string_of_int len));
    ]

let build_constraint_kind_json kind =
  let num =
    match kind with
    | Constraint_as -> 0
    | Constraint_eq -> 1
    | Constraint_super -> 2
  in
  JSON_Number (string_of_int num)

let build_constraint_json ctx (kind, hint) =
  let type_string = get_type_from_hint ctx hint in
  JSON_Object
    [
      ("constraintKind", build_constraint_kind_json kind);
      ("type", build_type_json_nested type_string);
    ]

let build_decl_target_json json = JSON_Object [("declaration", json)]

let build_file_lines_json filepath lineLengths endsInNewline hasUnicodeOrTabs =
  let lengths =
    List.map lineLengths (fun len -> JSON_Number (string_of_int len))
  in
  JSON_Object
    [
      ("file", build_file_json_nested filepath);
      ("lengths", JSON_Array lengths);
      ("endsInNewline", JSON_Bool endsInNewline);
      ("hasUnicodeOrTabs", JSON_Bool hasUnicodeOrTabs);
    ]

let build_is_async_json fun_kind =
  let is_async =
    match fun_kind with
    | FAsync -> true
    | FAsyncGenerator -> true
    | _ -> false
  in
  JSON_Bool is_async

let build_parameter_json param_name param_type_name =
  let fields =
    let name_field = [("name", build_name_json_nested param_name)] in
    match param_type_name with
    | None -> name_field
    | Some ty -> ("type", build_type_json_nested ty) :: name_field
  in
  JSON_Object fields

let build_signature_json ctx params ret_ty =
  let parameters =
    List.map params (fun param ->
        let ty =
          match hint_of_type_hint param.param_type_hint with
          | None -> None
          | Some h -> Some (get_type_from_hint ctx h)
        in
        build_parameter_json param.param_name ty)
  in
  let return_type_name =
    match hint_of_type_hint ret_ty with
    | None -> None
    | Some h -> Some (get_type_from_hint ctx h)
  in
  build_signature_json_nested parameters return_type_name

let build_reify_kind_json kind =
  let num =
    match kind with
    | Erased -> 0
    | Reified -> 1
    | SoftReified -> 2
  in
  JSON_Number (string_of_int num)

let build_type_const_kind_json kind =
  let num =
    match kind with
    | TCAbstract _ -> 0
    | TCConcrete -> 1
    | TCPartiallyAbstract -> 2
  in
  JSON_Number (string_of_int num)

let build_variance_json variance =
  let num =
    match variance with
    | Contravariant -> 0
    | Covariant -> 1
    | Invariant -> 2
  in
  JSON_Number (string_of_int num)

let build_type_param_json ctx source_map tp =
  let (_, name) = tp.tp_name in
  let constraints = List.map tp.tp_constraints (build_constraint_json ctx) in
  JSON_Object
    [
      ("name", build_name_json_nested name);
      ("variance", build_variance_json tp.tp_variance);
      ("reifyKind", build_reify_kind_json tp.tp_reified);
      ("constraints", JSON_Array constraints);
      ( "attributes",
        build_attributes_json_nested source_map tp.tp_user_attributes );
    ]

let build_visibility_json (visibility : Aast.visibility) =
  let num =
    match visibility with
    | Private -> 0
    | Protected -> 1
    | Public -> 2
  in
  JSON_Number (string_of_int num)

let build_xrefs_json xref_map =
  let xrefs =
    IMap.fold
      (fun _id (target_json, pos_list) acc ->
        let sorted_pos = List.sort Pos.compare pos_list in
        let (byte_spans, _) =
          List.fold sorted_pos ~init:([], 0) ~f:(fun (spans, last_start) pos ->
              let start = fst (Pos.info_raw pos) in
              let length = Pos.length pos in
              let span = build_rel_bytespan_json (start - last_start) length in
              (span :: spans, start))
        in
        let xref =
          JSON_Object
            [("target", target_json); ("ranges", JSON_Array byte_spans)]
        in
        xref :: acc)
      xref_map
      []
  in
  JSON_Array xrefs

(* These are functions for building JSON to reference some
existing fact. *)

let build_class_const_decl_json_ref fact_id =
  JSON_Object [("classConst", build_id_json fact_id)]

let build_container_json_ref container_type fact_id =
  JSON_Object [(container_type, build_id_json fact_id)]

let build_container_decl_json_ref container_type fact_id =
  let container_json = build_container_json_ref container_type fact_id in
  JSON_Object [("container", container_json)]

let build_enum_decl_json_ref fact_id =
  JSON_Object [("enum_", build_id_json fact_id)]

let build_enumerator_decl_json_ref fact_id =
  JSON_Object [("enumerator", build_id_json fact_id)]

let build_func_decl_json_ref fact_id =
  JSON_Object [("function_", build_id_json fact_id)]

let build_gconst_decl_json_ref fact_id =
  JSON_Object [("globalConst", build_id_json fact_id)]

let build_method_decl_json_ref fact_id =
  JSON_Object [("method", build_id_json fact_id)]

let build_property_decl_json_ref fact_id =
  JSON_Object [("property_", build_id_json fact_id)]

let build_type_const_decl_json_ref fact_id =
  JSON_Object [("typeConst", build_id_json fact_id)]

let build_typedef_decl_json_ref fact_id =
  JSON_Object [("typedef_", build_id_json fact_id)]
