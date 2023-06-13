(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
open Ast_defs
open Hh_json
open Hh_prelude
module Util = Symbol_json_util
module Fact_id = Symbol_fact_id
module XRefs = Symbol_xrefs

let build_id_json fact_id = JSON_Object [("id", Fact_id.to_json_number fact_id)]

let build_argument_lit_json lit =
  Hh_json.JSON_Object [("lit", Hh_json.JSON_Object [("key", JSON_String lit)])]

let build_argument_xref_json xref = Hh_json.JSON_Object [("xref", xref)]

let build_file_json_nested filepath =
  JSON_Object [("key", JSON_String filepath)]

let build_name_json_nested name =
  (* Remove leading slash, if present, so names such as
     Exception and \Exception are captured by the same fact *)
  let basename = Utils.strip_ns name in
  JSON_Object [("key", JSON_String basename)]

let rec build_namespaceqname_json_nested ns =
  let fields =
    match Util.split_name ns with
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
    match Util.split_name qname with
    (* Global namespace *)
    | None -> [("name", build_name_json_nested qname)]
    | Some (ns, name) ->
      [
        ("name", build_name_json_nested name);
        ("namespace_", build_namespaceqname_json_nested ns);
      ]
  in
  JSON_Object [("key", JSON_Object fields)]

let build_method_decl_nested meth_name con_name con_type =
  let cont_decl = JSON_Object [("name", build_qname_json_nested con_name)] in
  let nested_cont_decl =
    JSON_Object [(con_type, JSON_Object [("key", cont_decl)])]
  in
  let meth_decl =
    JSON_Object
      [
        ("name", build_name_json_nested meth_name);
        ("container", nested_cont_decl);
      ]
  in
  JSON_Object [("key", meth_decl)]

let build_type_json_nested type_name =
  (* Remove namespace slash from type, if present *)
  let ty = Utils.strip_ns type_name in
  JSON_Object [("key", JSON_String ty)]

let build_signature_json_nested parameters ctxs return_type_name return_info =
  let fields =
    let params = [("parameters", JSON_Array parameters)] in
    let params_return_type =
      match return_type_name with
      | None -> params
      | Some ty -> ("returns", build_type_json_nested ty) :: params
    in
    let params_return_info =
      match return_info with
      | None -> params_return_type
      | Some fact_id ->
        ("returnsTypeInfo", build_id_json fact_id) :: params_return_type
    in
    match ctxs with
    | None -> params_return_info
    | Some ctxs_name -> ("contexts", JSON_Array ctxs_name) :: params_return_info
  in
  JSON_Object [("key", JSON_Object fields)]

let build_module_membership_nested decl_id ~internal =
  let fields =
    [("declaration", build_id_json decl_id); ("internal", JSON_Bool internal)]
  in
  JSON_Object fields

let build_attributes_json_nested source_text attrs =
  let attributes =
    List.map attrs ~f:(fun attr ->
        let (_, name) = attr.ua_name in
        let params =
          List.fold_right attr.ua_params ~init:[] ~f:(fun expr acc ->
              Util.ast_expr_to_json source_text expr :: acc)
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

let build_call_arguments_json arguments =
  let argument_json span arg_opt =
    JSON_Object
      (("span", span)
      ::
      (match arg_opt with
      | Some json_obj -> [("argument", json_obj)]
      | None -> []))
  in
  let f (json_fields, last_start) (arg_opt, pos) =
    let (start, _) = Pos.info_raw pos in
    let length = Pos.length pos in
    let rel_span = build_rel_bytespan_json (start - last_start) length in
    (argument_json rel_span arg_opt :: json_fields, start)
  in
  List.fold arguments ~init:([], 0) ~f |> fst |> List.rev

let build_constraint_kind_json kind =
  let num =
    match kind with
    | Constraint_as -> 0
    | Constraint_eq -> 1
    | Constraint_super -> 2
  in
  JSON_Number (string_of_int num)

let build_constraint_json ctx (kind, hint) =
  let type_string = Util.get_type_from_hint ctx hint in
  JSON_Object
    [
      ("constraintKind", build_constraint_kind_json kind);
      ("type", build_type_json_nested type_string);
    ]

let build_decl_target_json json = JSON_Object [("declaration", json)]

let build_occ_target_json json = JSON_Object [("occurrence", json)]

let build_file_lines_json filepath lineLengths endsInNewline hasUnicodeOrTabs =
  let lengths =
    List.map lineLengths ~f:(fun len -> JSON_Number (string_of_int len))
  in
  JSON_Object
    [
      ("file", build_file_json_nested filepath);
      ("lengths", JSON_Array lengths);
      ("endsInNewline", JSON_Bool endsInNewline);
      ("hasUnicodeOrTabs", JSON_Bool hasUnicodeOrTabs);
    ]

let build_string_json_nested str = JSON_Object [("key", JSON_String str)]

let build_gen_code_json
    ~path ~fully_generated ~signature ~source ~command ~class_ =
  let fields =
    [
      ("file", build_file_json_nested path);
      ( "variant",
        JSON_Number
          (if fully_generated then
            "0"
          else
            "1") );
    ]
  in
  let l =
    [
      ("signature", signature);
      ("source", source);
      ("command", command);
      ("class_", class_);
    ]
  in
  let f (key, value_opt) =
    match value_opt with
    | None -> None
    | Some value -> Some (key, build_string_json_nested value)
  in
  JSON_Object (fields @ List.filter_map ~f l)

let build_is_async_json fun_kind =
  let is_async =
    match fun_kind with
    | FAsync -> true
    | FAsyncGenerator -> true
    | _ -> false
  in
  JSON_Bool is_async

let build_readonly_kind_json _ = JSON_Number "0"

let build_parameter_json
    source_text
    param_name
    param_type_name
    def_val
    is_inout
    is_variadic
    readonly_kind_opt
    attrs
    type_info =
  let fields =
    [
      ("name", build_name_json_nested param_name);
      ("isInout", JSON_Bool is_inout);
      ("isVariadic", JSON_Bool is_variadic);
      ("attributes", build_attributes_json_nested source_text attrs);
    ]
  in
  let fields =
    match param_type_name with
    | None -> fields
    | Some ty -> ("type", build_type_json_nested ty) :: fields
  in
  let fields =
    match def_val with
    | None -> fields
    | Some expr ->
      ("defaultValue", JSON_String (Util.strip_nested_quotes expr)) :: fields
  in
  let fields =
    match type_info with
    | None -> fields
    | Some fact_id -> ("typeInfo", build_id_json fact_id) :: fields
  in
  let fields =
    match readonly_kind_opt with
    | None -> fields
    | Some readonly_kind ->
      ("readonly", build_readonly_kind_json readonly_kind) :: fields
  in
  JSON_Object fields

let build_signature_json
    ctx
    source_text
    params
    (ctxs_hints : Aast.contexts option)
    ~ret_ty
    ~return_info =
  let ctx_hint_to_json ctx_hint =
    JSON_Object [("key", JSON_String (Util.get_context_from_hint ctx ctx_hint))]
  in
  let f (_pos, ctx_hint) = List.map ~f:ctx_hint_to_json ctx_hint in
  let ctxs_hints = Option.map ctxs_hints ~f in
  let build_param (p, type_xref, ty) =
    let is_inout =
      match p.param_callconv with
      | Pinout _ -> true
      | Pnormal -> false
    in
    let def_value =
      Option.map p.param_expr ~f:(fun expr ->
          Util.ast_expr_to_string source_text expr)
    in
    build_parameter_json
      source_text
      p.param_name
      ty
      def_value
      is_inout
      p.param_is_variadic
      p.param_readonly
      p.param_user_attributes
      type_xref
  in
  let parameters = List.map params ~f:build_param in
  build_signature_json_nested parameters ctxs_hints ret_ty return_info

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
    | TCConcrete _ -> 1
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

let build_type_param_json ctx source_text tp =
  let (_, name) = tp.tp_name in
  let constraints = List.map tp.tp_constraints ~f:(build_constraint_json ctx) in
  JSON_Object
    [
      ("name", build_name_json_nested name);
      ("variance", build_variance_json tp.tp_variance);
      ("reifyKind", build_reify_kind_json tp.tp_reified);
      ("constraints", JSON_Array constraints);
      ( "attributes",
        build_attributes_json_nested source_text tp.tp_user_attributes );
    ]

let build_visibility_json (visibility : Aast.visibility) =
  let num =
    match visibility with
    | Private -> 0
    | Protected -> 1
    | Public -> 2
    | Internal -> 3
  in
  JSON_Number (string_of_int num)

let build_generic_xrefs_json (sym_pos : (Hh_json.json * Util.pos list) Seq.t) =
  let xrefs =
    Caml.Seq.fold_left
      (fun acc (target_json, pos_list) ->
        let sorted_pos = Caml.List.sort_uniq Util.compare_pos pos_list in
        let (rev_byte_spans, _) =
          List.fold
            sorted_pos
            ~init:([], 0)
            ~f:(fun (spans, last_start) Util.{ start; length } ->
              let span = build_rel_bytespan_json (start - last_start) length in
              (span :: spans, start))
        in
        let byte_spans = List.rev rev_byte_spans in
        let xref =
          JSON_Object
            [("ranges", JSON_Array byte_spans); ("target", target_json)]
        in
        xref :: acc)
      []
      sym_pos
  in
  (* there's no specified order for xref arrays, but it helps to avoid non-determinism
     when diffing dbs *)
  let sorted_xrefs = List.sort ~compare:Hh_json.JsonKey.compare xrefs in
  JSON_Array sorted_xrefs

let build_xrefs_json (fact_map : XRefs.fact_map) =
  let f (_fact_id, (json, pos_list)) =
    let util_pos_list =
      List.map pos_list ~f:(fun pos ->
          let start = fst (Pos.info_raw pos) in
          let length = Pos.length pos in
          Util.{ start; length })
    in
    (json, util_pos_list)
  in
  let sym_pos = Fact_id.Map.to_seq fact_map |> Caml.Seq.map f in
  build_generic_xrefs_json sym_pos

let build_hint_xrefs_json sym_pos =
  Caml.List.to_seq sym_pos |> build_generic_xrefs_json

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
  build_container_decl_json_ref "enum_" fact_id

let build_namespace_decl_json_ref fact_id =
  JSON_Object [("namespace_", build_id_json fact_id)]

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

let build_module_decl_json_ref fact_id =
  JSON_Object [("module", build_id_json fact_id)]

let build_method_occ_json_ref fact_id =
  JSON_Object [("method", build_id_json fact_id)]

let build_class_decl_json_nested receiver =
  JSON_Object
    [
      ( "container",
        JSON_Object
          [
            ( "class_",
              JSON_Object
                [
                  ( "key",
                    JSON_Object [("name", build_qname_json_nested receiver)] );
                ] );
          ] );
    ]
