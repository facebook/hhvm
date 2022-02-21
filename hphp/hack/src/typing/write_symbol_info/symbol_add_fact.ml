(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
open Hh_prelude
open Hh_json
module Util = Symbol_json_util
module Build_json = Symbol_build_json
module Predicate = Symbol_predicate
module Fact_id = Symbol_fact_id

type glean_json = {
  classConstDeclaration: Hh_json.json list;
  classConstDefinition: Hh_json.json list;
  classDeclaration: Hh_json.json list;
  classDefinition: Hh_json.json list;
  declarationComment: Hh_json.json list;
  declarationLocation: Hh_json.json list;
  declarationSpan: Hh_json.json list;
  enumDeclaration: Hh_json.json list;
  enumDefinition: Hh_json.json list;
  enumerator: Hh_json.json list;
  fileDeclarations: Hh_json.json list;
  fileLines: Hh_json.json list;
  fileXRefs: Hh_json.json list;
  functionDeclaration: Hh_json.json list;
  functionDefinition: Hh_json.json list;
  globalConstDeclaration: Hh_json.json list;
  globalConstDefinition: Hh_json.json list;
  interfaceDeclaration: Hh_json.json list;
  interfaceDefinition: Hh_json.json list;
  methodDeclaration: Hh_json.json list;
  methodDefinition: Hh_json.json list;
  methodOccurrence: Hh_json.json list;
  methodOverrides: Hh_json.json list;
  namespaceDeclaration: Hh_json.json list;
  propertyDeclaration: Hh_json.json list;
  propertyDefinition: Hh_json.json list;
  traitDeclaration: Hh_json.json list;
  traitDefinition: Hh_json.json list;
  typeConstDeclaration: Hh_json.json list;
  typeConstDefinition: Hh_json.json list;
  typedefDeclaration: Hh_json.json list;
  typedefDefinition: Hh_json.json list;
}

module JsonPredicateMap = WrappedMap.Make (struct
  let compare_json = Hh_json.JsonKey.compare

  type t = json * Predicate.t [@@deriving ord]
end)

type t = {
  resultJson: glean_json;
  factIds: Fact_id.t JsonPredicateMap.t;
}

let update_glean_json predicate json factkey_opt progress =
  let open Symbol_predicate in
  let resultJson =
    match predicate with
    | Hack ClassConstDeclaration ->
      {
        progress.resultJson with
        classConstDeclaration =
          json :: progress.resultJson.classConstDeclaration;
      }
    | Hack ClassConstDefinition ->
      {
        progress.resultJson with
        classConstDefinition = json :: progress.resultJson.classConstDefinition;
      }
    | Hack ClassDeclaration ->
      {
        progress.resultJson with
        classDeclaration = json :: progress.resultJson.classDeclaration;
      }
    | Hack ClassDefinition ->
      {
        progress.resultJson with
        classDefinition = json :: progress.resultJson.classDefinition;
      }
    | Hack DeclarationComment ->
      {
        progress.resultJson with
        declarationComment = json :: progress.resultJson.declarationComment;
      }
    | Hack DeclarationLocation ->
      {
        progress.resultJson with
        declarationLocation = json :: progress.resultJson.declarationLocation;
      }
    | Hack DeclarationSpan ->
      {
        progress.resultJson with
        declarationSpan = json :: progress.resultJson.declarationSpan;
      }
    | Hack EnumDeclaration ->
      {
        progress.resultJson with
        enumDeclaration = json :: progress.resultJson.enumDeclaration;
      }
    | Hack EnumDefinition ->
      {
        progress.resultJson with
        enumDefinition = json :: progress.resultJson.enumDefinition;
      }
    | Hack Enumerator ->
      {
        progress.resultJson with
        enumerator = json :: progress.resultJson.enumerator;
      }
    | Hack FileDeclarations ->
      {
        progress.resultJson with
        fileDeclarations = json :: progress.resultJson.fileDeclarations;
      }
    | Src FileLines ->
      {
        progress.resultJson with
        fileLines = json :: progress.resultJson.fileLines;
      }
    | Hack FileXRefs ->
      {
        progress.resultJson with
        fileXRefs = json :: progress.resultJson.fileXRefs;
      }
    | Hack FunctionDeclaration ->
      {
        progress.resultJson with
        functionDeclaration = json :: progress.resultJson.functionDeclaration;
      }
    | Hack FunctionDefinition ->
      {
        progress.resultJson with
        functionDefinition = json :: progress.resultJson.functionDefinition;
      }
    | Hack GlobalConstDeclaration ->
      {
        progress.resultJson with
        globalConstDeclaration =
          json :: progress.resultJson.globalConstDeclaration;
      }
    | Hack GlobalConstDefinition ->
      {
        progress.resultJson with
        globalConstDefinition =
          json :: progress.resultJson.globalConstDefinition;
      }
    | Hack InterfaceDeclaration ->
      {
        progress.resultJson with
        interfaceDeclaration = json :: progress.resultJson.interfaceDeclaration;
      }
    | Hack InterfaceDefinition ->
      {
        progress.resultJson with
        interfaceDefinition = json :: progress.resultJson.interfaceDefinition;
      }
    | Hack MethodDeclaration ->
      {
        progress.resultJson with
        methodDeclaration = json :: progress.resultJson.methodDeclaration;
      }
    | Hack MethodDefinition ->
      {
        progress.resultJson with
        methodDefinition = json :: progress.resultJson.methodDefinition;
      }
    | Hack MethodOccurrence ->
      {
        progress.resultJson with
        methodOccurrence = json :: progress.resultJson.methodOccurrence;
      }
    | Hack MethodOverrides ->
      {
        progress.resultJson with
        methodOverrides = json :: progress.resultJson.methodOverrides;
      }
    | Hack NamespaceDeclaration ->
      {
        progress.resultJson with
        namespaceDeclaration = json :: progress.resultJson.namespaceDeclaration;
      }
    | Hack PropertyDeclaration ->
      {
        progress.resultJson with
        propertyDeclaration = json :: progress.resultJson.propertyDeclaration;
      }
    | Hack PropertyDefinition ->
      {
        progress.resultJson with
        propertyDefinition = json :: progress.resultJson.propertyDefinition;
      }
    | Hack TraitDeclaration ->
      {
        progress.resultJson with
        traitDeclaration = json :: progress.resultJson.traitDeclaration;
      }
    | Hack TraitDefinition ->
      {
        progress.resultJson with
        traitDefinition = json :: progress.resultJson.traitDefinition;
      }
    | Hack TypeConstDeclaration ->
      {
        progress.resultJson with
        typeConstDeclaration = json :: progress.resultJson.typeConstDeclaration;
      }
    | Hack TypeConstDefinition ->
      {
        progress.resultJson with
        typeConstDefinition = json :: progress.resultJson.typeConstDefinition;
      }
    | Hack TypedefDeclaration ->
      {
        progress.resultJson with
        typedefDeclaration = json :: progress.resultJson.typedefDeclaration;
      }
    | Hack TypedefDefinition ->
      {
        progress.resultJson with
        typedefDefinition = json :: progress.resultJson.typedefDefinition;
      }
  in
  let factIds =
    match factkey_opt with
    | None -> progress.factIds
    | Some (fact_id, json_key) ->
      JsonPredicateMap.add (json_key, predicate) fact_id progress.factIds
  in
  { resultJson; factIds }

(* Add a fact of the given predicate type to the running result, if an identical
 fact has not yet been added. Return the fact's id (which can be referenced in
 other facts), and the updated result. *)
let add_fact predicate json_key progress =
  let fact_id = Fact_id.next () in
  let json_fact =
    JSON_Object [("id", Fact_id.to_json_number fact_id); ("key", json_key)]
  in
  match
    ( Predicate.should_cache predicate,
      JsonPredicateMap.find_opt (json_key, predicate) progress.factIds )
  with
  | (false, _) -> (fact_id, update_glean_json predicate json_fact None progress)
  | (true, None) ->
    ( fact_id,
      update_glean_json predicate json_fact (Some (fact_id, json_key)) progress
    )
  | (true, Some fid) -> (fid, progress)

(* Add a namespace fact if the nsenv is non-empty; otherwise,
return progress unchanged *)
let namespace_decl nsenv progress =
  match nsenv.Namespace_env.ns_name with
  | None -> progress (* Global namespace *)
  | Some "" -> progress
  | Some ns ->
    let json_fields =
      [("name", Build_json.build_namespaceqname_json_nested ns)]
    in
    let (_fid, prog) =
      add_fact
        Predicate.(Hack NamespaceDeclaration)
        (JSON_Object json_fields)
        progress
    in
    prog

let container_decl decl_pred name progress =
  let json = JSON_Object [("name", Build_json.build_qname_json_nested name)] in
  add_fact decl_pred json progress

(* Helper function for adding facts for container parents, given
a context, a list of declarations, a predicate type, and progress state *)
let parent_decls ctx decls pred prog =
  List.fold decls ~init:([], prog) ~f:(fun (decl_refs, prog) decl ->
      let name = Util.strip_tparams (Util.get_type_from_hint ctx decl) in
      let (decl_id, prog) = container_decl pred name prog in
      let ref = Build_json.build_id_json decl_id in
      (ref :: decl_refs, prog))

let container_defn ctx source_map clss decl_id member_decls prog =
  let prog = namespace_decl clss.c_namespace prog in
  let tparams =
    List.map clss.c_tparams ~f:(Build_json.build_type_param_json ctx source_map)
  in
  let common_fields =
    [
      ("declaration", Build_json.build_id_json decl_id);
      ("members", JSON_Array member_decls);
      ( "attributes",
        Build_json.build_attributes_json_nested
          source_map
          clss.c_user_attributes );
      ("typeParams", JSON_Array tparams);
    ]
  in
  let (req_extends_hints, req_implements_hints) =
    Aast.partition_map_require_kind ~f:(fun x -> x) clss.c_reqs
  in
  let (req_extends, prog) =
    parent_decls
      ctx
      (List.map req_extends_hints ~f:fst)
      Predicate.(Hack ClassDeclaration)
      prog
  in
  let (req_implements, prog) =
    parent_decls
      ctx
      (List.map req_implements_hints ~f:fst)
      Predicate.(Hack InterfaceDeclaration)
      prog
  in
  let (defn_pred, json_fields, prog) =
    match Predicate.get_parent_kind clss with
    | Predicate.InterfaceContainer ->
      let (extends, prog) =
        parent_decls
          ctx
          clss.c_extends
          Predicate.(Hack InterfaceDeclaration)
          prog
      in
      let req_fields =
        common_fields
        @ [
            ("extends_", JSON_Array extends);
            ("requireExtends", JSON_Array req_extends);
          ]
      in
      (Predicate.(Hack InterfaceDefinition), req_fields, prog)
    | Predicate.TraitContainer ->
      let (impls, prog) =
        parent_decls
          ctx
          clss.c_implements
          Predicate.(Hack InterfaceDeclaration)
          prog
      in
      let (uses, prog) =
        parent_decls ctx clss.c_uses Predicate.(Hack TraitDeclaration) prog
      in
      let req_fields =
        common_fields
        @ [
            ("implements_", JSON_Array impls);
            ("uses", JSON_Array uses);
            ("requireExtends", JSON_Array req_extends);
            ("requireImplements", JSON_Array req_implements);
          ]
      in
      (Predicate.(Hack TraitDefinition), req_fields, prog)
    | Predicate.ClassContainer ->
      let is_abstract = Ast_defs.is_c_abstract clss.c_kind in
      let (class_fields, prog) =
        let (impls, prog) =
          parent_decls
            ctx
            clss.c_implements
            Predicate.(Hack InterfaceDeclaration)
            prog
        in
        let (uses, prog) =
          parent_decls ctx clss.c_uses Predicate.(Hack TraitDeclaration) prog
        in
        let req_class_fields =
          common_fields
          @ Hh_json.
              [
                ("isAbstract", JSON_Bool is_abstract);
                ("isFinal", JSON_Bool clss.c_final);
                ("implements_", JSON_Array impls);
                ("uses", JSON_Array uses);
              ]
        in
        match clss.c_extends with
        | [] -> (req_class_fields, prog)
        | [parent] ->
          let (decl_id, prog) =
            let parent_clss =
              Util.strip_tparams (Util.get_type_from_hint ctx parent)
            in
            container_decl Predicate.(Hack ClassDeclaration) parent_clss prog
          in
          ( ("extends_", Build_json.build_id_json decl_id) :: req_class_fields,
            prog )
        | _ ->
          Hh_logger.log
            "WARNING: skipping extends field for class with multiple parents %s"
            (snd clss.c_name);
          (req_class_fields, prog)
      in
      (Predicate.(Hack ClassDefinition), class_fields, prog)
  in
  add_fact defn_pred (JSON_Object json_fields) prog

let property_decl con_type decl_id name progress =
  let json =
    JSON_Object
      [
        ("name", Build_json.build_name_json_nested name);
        ("container", Build_json.build_container_json_ref con_type decl_id);
      ]
  in
  add_fact Predicate.(Hack PropertyDeclaration) json progress

let class_const_decl con_type decl_id name progress =
  let json =
    JSON_Object
      [
        ("name", Build_json.build_name_json_nested name);
        ("container", Build_json.build_container_json_ref con_type decl_id);
      ]
  in
  add_fact Predicate.(Hack ClassConstDeclaration) json progress

let type_const_decl con_type decl_id name progress =
  let json =
    JSON_Object
      [
        ("name", Build_json.build_name_json_nested name);
        ("container", Build_json.build_container_json_ref con_type decl_id);
      ]
  in
  add_fact Predicate.(Hack TypeConstDeclaration) json progress

let method_decl con_type decl_id name progress =
  let json =
    JSON_Object
      [
        ("name", Build_json.build_name_json_nested name);
        ("container", Build_json.build_container_json_ref con_type decl_id);
      ]
  in
  add_fact Predicate.(Hack MethodDeclaration) json progress

let method_defn ctx source_map meth decl_id progress =
  let tparams =
    List.map meth.m_tparams ~f:(Build_json.build_type_param_json ctx source_map)
  in
  let json =
    JSON_Object
      [
        ("declaration", Build_json.build_id_json decl_id);
        ( "signature",
          Build_json.build_signature_json
            ctx
            source_map
            meth.m_params
            meth.m_ret );
        ("visibility", Build_json.build_visibility_json meth.m_visibility);
        ("isAbstract", JSON_Bool meth.m_abstract);
        ("isAsync", Build_json.build_is_async_json meth.m_fun_kind);
        ("isFinal", JSON_Bool meth.m_final);
        ("isStatic", JSON_Bool meth.m_static);
        ( "attributes",
          Build_json.build_attributes_json_nested
            source_map
            meth.m_user_attributes );
        ("typeParams", JSON_Array tparams);
      ]
  in
  add_fact Predicate.(Hack MethodDefinition) json progress

let method_overrides
    meth_name base_cont_name base_cont_type der_cont_name der_cont_type prog =
  let json =
    JSON_Object
      [
        ( "derived",
          Build_json.build_method_decl_nested
            meth_name
            der_cont_name
            der_cont_type );
        ( "base",
          Build_json.build_method_decl_nested
            meth_name
            base_cont_name
            base_cont_type );
      ]
  in
  add_fact Predicate.(Hack MethodOverrides) json prog

let property_defn ctx source_map prop decl_id progress =
  let base_fields =
    [
      ("declaration", Build_json.build_id_json decl_id);
      ("visibility", Build_json.build_visibility_json prop.cv_visibility);
      ("isFinal", JSON_Bool prop.cv_final);
      ("isAbstract", JSON_Bool prop.cv_abstract);
      ("isStatic", JSON_Bool prop.cv_is_static);
      ( "attributes",
        Build_json.build_attributes_json_nested
          source_map
          prop.cv_user_attributes );
    ]
  in
  let json_fields =
    match hint_of_type_hint prop.cv_type with
    | None -> base_fields
    | Some h ->
      let ty = Util.get_type_from_hint ctx h in
      ("type", Build_json.build_type_json_nested ty) :: base_fields
  in
  add_fact
    Predicate.(Hack PropertyDefinition)
    (JSON_Object json_fields)
    progress

let class_const_defn ctx source_map const decl_id progress =
  let base_fields = [("declaration", Build_json.build_id_json decl_id)] in
  let json_fields =
    match const.cc_kind with
    | CCAbstract None -> base_fields
    | CCAbstract (Some (_, expr_pos, _))
    | CCConcrete (_, expr_pos, _) ->
      let fp = Relative_path.to_absolute (Pos.filename expr_pos) in
      let value =
        match SMap.find_opt fp source_map with
        | Some st -> Util.source_at_span st expr_pos
        | None -> ""
      in
      ("value", JSON_String (Util.strip_nested_quotes value)) :: base_fields
  in
  let json_fields =
    match const.cc_type with
    | None -> json_fields
    | Some h ->
      let ty = Util.get_type_from_hint ctx h in
      ("type", Build_json.build_type_json_nested ty) :: json_fields
  in
  add_fact
    Predicate.(Hack ClassConstDefinition)
    (JSON_Object json_fields)
    progress

let type_const_defn ctx source_map tc decl_id progress =
  let base_fields =
    [
      ("declaration", Build_json.build_id_json decl_id);
      ("kind", Build_json.build_type_const_kind_json tc.c_tconst_kind);
      ( "attributes",
        Build_json.build_attributes_json_nested
          source_map
          tc.c_tconst_user_attributes );
    ]
  in
  let json_fields =
    (* TODO(T88552052) should the default of an abstract type constant be used
     * as a value here *)
    match tc.c_tconst_kind with
    | TCConcrete { c_tc_type = h }
    | TCAbstract { c_atc_default = Some h; _ } ->
      let ty = Util.get_type_from_hint ctx h in
      ("type", Build_json.build_type_json_nested ty) :: base_fields
    | TCAbstract { c_atc_default = None; _ } -> base_fields
  in
  add_fact
    Predicate.(Hack TypeConstDefinition)
    (JSON_Object json_fields)
    progress

let enum_decl name progress =
  let json = JSON_Object [("name", Build_json.build_qname_json_nested name)] in
  add_fact Predicate.(Hack EnumDeclaration) json progress

let enum_defn ctx source_map enm enum_id enum_data enumerators progress =
  let prog = namespace_decl enm.c_namespace progress in
  let (includes, prog) =
    parent_decls ctx enum_data.e_includes Predicate.(Hack EnumDeclaration) prog
  in
  let is_enum_class = Aast.is_enum_class enm in
  let json_fields =
    [
      ("declaration", Build_json.build_id_json enum_id);
      ( "enumBase",
        Build_json.build_type_json_nested
          (Util.get_type_from_hint ctx enum_data.e_base) );
      ("enumerators", JSON_Array enumerators);
      ( "attributes",
        Build_json.build_attributes_json_nested source_map enm.c_user_attributes
      );
      ("includes", JSON_Array includes);
      ("isEnumClass", JSON_Bool is_enum_class);
    ]
  in
  let json_fields =
    match enum_data.e_constraint with
    | None -> json_fields
    | Some c ->
      ( "enumConstraint",
        Build_json.build_type_json_nested (Util.get_type_from_hint ctx c) )
      :: json_fields
  in
  add_fact Predicate.(Hack EnumDefinition) (JSON_Object json_fields) prog

let enumerator decl_id const_name progress =
  let json =
    JSON_Object
      [
        ("name", Build_json.build_name_json_nested const_name);
        ("enumeration", Build_json.build_id_json decl_id);
      ]
  in
  add_fact Predicate.(Hack Enumerator) json progress

let func_decl name progress =
  let json = JSON_Object [("name", Build_json.build_qname_json_nested name)] in
  add_fact Predicate.(Hack FunctionDeclaration) json progress

let func_defn ctx source_map fd decl_id progress =
  let elem = fd.fd_fun in
  let prog = namespace_decl fd.fd_namespace progress in
  let tparams =
    List.map elem.f_tparams ~f:(Build_json.build_type_param_json ctx source_map)
  in
  let json_fields =
    [
      ("declaration", Build_json.build_id_json decl_id);
      ( "signature",
        Build_json.build_signature_json ctx source_map elem.f_params elem.f_ret
      );
      ("isAsync", Build_json.build_is_async_json elem.f_fun_kind);
      ( "attributes",
        Build_json.build_attributes_json_nested
          source_map
          elem.f_user_attributes );
      ("typeParams", JSON_Array tparams);
    ]
  in
  add_fact Predicate.(Hack FunctionDefinition) (JSON_Object json_fields) prog

let typedef_decl name progress =
  let json = JSON_Object [("name", Build_json.build_qname_json_nested name)] in
  add_fact Predicate.(Hack TypedefDeclaration) json progress

let typedef_defn ctx source_map elem decl_id progress =
  let prog = namespace_decl elem.t_namespace progress in
  let is_transparent =
    match elem.t_vis with
    | Transparent -> true
    | Tinternal -> true
    | Opaque -> false
  in
  let tparams =
    List.map elem.t_tparams ~f:(Build_json.build_type_param_json ctx source_map)
  in
  let json_fields =
    [
      ("declaration", Build_json.build_id_json decl_id);
      ("isTransparent", JSON_Bool is_transparent);
      ( "attributes",
        Build_json.build_attributes_json_nested
          source_map
          elem.t_user_attributes );
      ("typeParams", JSON_Array tparams);
    ]
  in
  add_fact Predicate.(Hack TypedefDefinition) (JSON_Object json_fields) prog

let gconst_decl name progress =
  let json = JSON_Object [("name", Build_json.build_qname_json_nested name)] in
  add_fact Predicate.(Hack GlobalConstDeclaration) json progress

let gconst_defn ctx source_map elem decl_id progress =
  let prog = namespace_decl elem.cst_namespace progress in
  let value =
    let (_, expr_pos, _) = elem.cst_value in
    let fp = Relative_path.to_absolute (Pos.filename expr_pos) in
    match SMap.find_opt fp source_map with
    | Some st -> Util.source_at_span st expr_pos
    | None -> ""
  in
  let req_fields =
    [
      ("declaration", Build_json.build_id_json decl_id);
      ("value", JSON_String (Util.strip_nested_quotes value));
    ]
  in
  let json_fields =
    match elem.cst_type with
    | None -> req_fields
    | Some h ->
      let ty = Util.get_type_from_hint ctx h in
      ("type", Build_json.build_type_json_nested ty) :: req_fields
  in
  let json = JSON_Object json_fields in
  add_fact Predicate.(Hack GlobalConstDefinition) json prog

let decl_loc pos decl_json progress =
  let filepath = Relative_path.to_absolute (Pos.filename pos) in
  let json =
    JSON_Object
      [
        ("declaration", decl_json);
        ("file", Build_json.build_file_json_nested filepath);
        ("span", Build_json.build_bytespan_json pos);
      ]
  in
  add_fact Predicate.(Hack DeclarationLocation) json progress

let decl_comment pos decl_json progress =
  let filepath = Relative_path.to_absolute (Pos.filename pos) in
  let json =
    JSON_Object
      [
        ("declaration", decl_json);
        ("file", Build_json.build_file_json_nested filepath);
        ("span", Build_json.build_bytespan_json pos);
      ]
  in
  add_fact Predicate.(Hack DeclarationComment) json progress

let decl_span pos decl_json progress =
  let filepath = Relative_path.to_absolute (Pos.filename pos) in
  let json =
    JSON_Object
      [
        ("declaration", decl_json);
        ("file", Build_json.build_file_json_nested filepath);
        ("span", Build_json.build_bytespan_json pos);
      ]
  in
  add_fact Predicate.(Hack DeclarationSpan) json progress

let file_lines filepath sourceText progress =
  let lineLengths =
    Line_break_map.offsets_to_line_lengths
      sourceText.Full_fidelity_source_text.offset_map
  in
  let endsInNewline = Util.ends_in_newline sourceText in
  let hasUnicodeOrTabs = Util.has_tabs_or_multibyte_codepoints sourceText in
  let json =
    Build_json.build_file_lines_json
      filepath
      lineLengths
      endsInNewline
      hasUnicodeOrTabs
  in
  add_fact Predicate.(Src FileLines) json progress

let file_xrefs filepath xref_map progress =
  let json =
    JSON_Object
      [
        ("file", Build_json.build_file_json_nested filepath);
        ("xrefs", Build_json.build_xrefs_json xref_map);
      ]
  in
  add_fact Predicate.(Hack FileXRefs) json progress

let file_decls filepath decls progress =
  let json =
    JSON_Object
      [
        ("file", Build_json.build_file_json_nested filepath);
        ("declarations", JSON_Array decls);
      ]
  in
  add_fact Predicate.(Hack FileDeclarations) json progress

let method_occ receiver_class name progress =
  let module SO = SymbolOccurrence in
  let json =
    List.concat
    @@ [
         [("name", Build_json.build_name_json_nested name)];
         (match receiver_class with
         | SO.ClassName className ->
           [("className", Build_json.build_name_json_nested className)]
         | SO.UnknownClass -> []);
       ]
  in
  add_fact Predicate.(Hack MethodOccurrence) (JSON_Object json) progress

let init_progress =
  let resultJson =
    {
      classConstDeclaration = [];
      classConstDefinition = [];
      classDeclaration = [];
      classDefinition = [];
      declarationComment = [];
      declarationLocation = [];
      declarationSpan = [];
      enumDeclaration = [];
      enumDefinition = [];
      enumerator = [];
      fileDeclarations = [];
      fileLines = [];
      fileXRefs = [];
      functionDeclaration = [];
      functionDefinition = [];
      globalConstDeclaration = [];
      globalConstDefinition = [];
      interfaceDeclaration = [];
      interfaceDefinition = [];
      methodDeclaration = [];
      methodDefinition = [];
      methodOccurrence = [];
      methodOverrides = [];
      namespaceDeclaration = [];
      propertyDeclaration = [];
      propertyDefinition = [];
      traitDeclaration = [];
      traitDefinition = [];
      typeConstDeclaration = [];
      typeConstDefinition = [];
      typedefDeclaration = [];
      typedefDefinition = [];
    }
  in
  { resultJson; factIds = JsonPredicateMap.empty }

let progress_to_json progress =
  let resultJson = progress.resultJson in
  let preds =
    (* The order is the reverse of how these items appear in the JSON,
       which is significant because later entries can refer to earlier ones
       by id only *)
    List.map
      ~f:(fun (pred, res) -> (Predicate.to_string pred, res))
      Predicate.
        [
          (Src FileLines, resultJson.fileLines);
          (Hack FileDeclarations, resultJson.fileDeclarations);
          (Hack FileXRefs, resultJson.fileXRefs);
          (Hack MethodOverrides, resultJson.methodOverrides);
          (Hack MethodDefinition, resultJson.methodDefinition);
          (Hack FunctionDefinition, resultJson.functionDefinition);
          (Hack EnumDefinition, resultJson.enumDefinition);
          (Hack ClassConstDefinition, resultJson.classConstDefinition);
          (Hack PropertyDefinition, resultJson.propertyDefinition);
          (Hack TypeConstDefinition, resultJson.typeConstDefinition);
          (Hack ClassDefinition, resultJson.classDefinition);
          (Hack TraitDefinition, resultJson.traitDefinition);
          (Hack InterfaceDefinition, resultJson.interfaceDefinition);
          (Hack TypedefDefinition, resultJson.typedefDefinition);
          (Hack GlobalConstDefinition, resultJson.globalConstDefinition);
          (Hack DeclarationComment, resultJson.declarationComment);
          (Hack DeclarationLocation, resultJson.declarationLocation);
          (Hack DeclarationSpan, resultJson.declarationSpan);
          (Hack MethodDeclaration, resultJson.methodDeclaration);
          (Hack ClassConstDeclaration, resultJson.classConstDeclaration);
          (Hack PropertyDeclaration, resultJson.propertyDeclaration);
          (Hack TypeConstDeclaration, resultJson.typeConstDeclaration);
          (Hack FunctionDeclaration, resultJson.functionDeclaration);
          (Hack Enumerator, resultJson.enumerator);
          (Hack EnumDeclaration, resultJson.enumDeclaration);
          (Hack ClassDeclaration, resultJson.classDeclaration);
          (Hack TraitDeclaration, resultJson.traitDeclaration);
          (Hack InterfaceDeclaration, resultJson.interfaceDeclaration);
          (Hack TypedefDeclaration, resultJson.typedefDeclaration);
          (Hack GlobalConstDeclaration, resultJson.globalConstDeclaration);
          (Hack NamespaceDeclaration, resultJson.namespaceDeclaration);
          (Hack MethodOccurrence, resultJson.methodOccurrence);
        ]
  in
  let json_array =
    List.fold preds ~init:[] ~f:(fun acc (pred, json_lst) ->
        JSON_Object
          [("predicate", JSON_String pred); ("facts", JSON_Array json_lst)]
        :: acc)
  in
  json_array
