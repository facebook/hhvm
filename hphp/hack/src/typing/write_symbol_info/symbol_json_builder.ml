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
open Symbol_add_fact
open Symbol_build_json
open Symbol_builder_types
open Symbol_json_util
open SymbolDefinition
open SymbolOccurrence

(* These functions define the process to go through when
encountering symbols of a given type. *)

let process_doc_comment comment decl_ref_json prog =
  match comment with
  | None -> prog
  | Some (pos, _doc) ->
    let (_, prog) = add_decl_comment_fact pos decl_ref_json prog in
    prog

let process_decl_loc
    decl_fun defn_fun decl_ref_fun pos span id elem doc progress =
  let (decl_id, prog) = decl_fun id progress in
  let (_, prog) = defn_fun elem decl_id prog in
  let ref_json = decl_ref_fun decl_id in
  let (_, prog) = add_decl_loc_fact pos ref_json prog in
  let prog = process_doc_comment doc ref_json prog in
  let prog =
    match span with
    | None -> prog
    | Some span_pos -> snd (add_decl_span_fact span_pos ref_json prog)
  in
  (decl_id, prog)

let process_container_decl ctx source_map con (all_decls, progress) =
  let (con_pos, con_name) = con.c_name in
  let (con_type, decl_pred) = parent_decl_predicate (get_parent_kind con) in
  let (con_decl_id, prog) =
    add_container_decl_fact decl_pred con_name progress
  in
  let (prop_decls, prog) =
    List.fold_right con.c_vars ~init:([], prog) ~f:(fun prop (decls, prog) ->
        let (pos, id) = prop.cv_id in
        let (decl_id, prog) =
          process_decl_loc
            (add_property_decl_fact con_type con_decl_id)
            (add_property_defn_fact ctx source_map)
            build_property_decl_json_ref
            pos
            (Some prop.cv_span)
            id
            prop
            prop.cv_doc_comment
            prog
        in
        (build_property_decl_json_ref decl_id :: decls, prog))
  in
  let (class_const_decls, prog) =
    List.fold_right con.c_consts ~init:([], prog) ~f:(fun const (decls, prog) ->
        let (pos, id) = const.cc_id in
        let (decl_id, prog) =
          process_decl_loc
            (add_class_const_decl_fact con_type con_decl_id)
            (add_class_const_defn_fact ctx source_map)
            build_class_const_decl_json_ref
            pos
            None
            id
            const
            const.cc_doc_comment
            prog
        in
        (build_class_const_decl_json_ref decl_id :: decls, prog))
  in
  let (type_const_decls, prog) =
    List.fold_right
      con.c_typeconsts
      ~init:([], prog)
      ~f:(fun tc (decls, prog) ->
        let (pos, id) = tc.c_tconst_name in
        let (decl_id, prog) =
          process_decl_loc
            (add_type_const_decl_fact con_type con_decl_id)
            (add_type_const_defn_fact ctx source_map)
            build_type_const_decl_json_ref
            pos
            (Some tc.c_tconst_span)
            id
            tc
            tc.c_tconst_doc_comment
            prog
        in
        (build_type_const_decl_json_ref decl_id :: decls, prog))
  in
  let (method_decls, prog) =
    List.fold_right con.c_methods ~init:([], prog) ~f:(fun meth (decls, prog) ->
        let (pos, id) = meth.m_name in
        let (decl_id, prog) =
          process_decl_loc
            (add_method_decl_fact con_type con_decl_id)
            (add_method_defn_fact ctx source_map)
            build_method_decl_json_ref
            pos
            (Some meth.m_span)
            id
            meth
            meth.m_doc_comment
            prog
        in
        (build_method_decl_json_ref decl_id :: decls, prog))
  in
  let members =
    type_const_decls @ class_const_decls @ prop_decls @ method_decls
  in
  let (_, prog) =
    add_container_defn_fact ctx source_map con con_decl_id members prog
  in
  let ref_json = build_container_decl_json_ref con_type con_decl_id in
  let (_, prog) = add_decl_loc_fact con_pos ref_json prog in
  let (_, prog) = add_decl_span_fact con.c_span ref_json prog in
  let all_decls = all_decls @ [ref_json] @ members in
  let prog = process_doc_comment con.c_doc_comment ref_json prog in
  (all_decls, prog)

let process_xref decl_fun decl_ref_fun symbol_name symbol_pos (xrefs, prog) =
  let (target_id, prog) = decl_fun symbol_name prog in
  let xref_json = decl_ref_fun target_id in
  let target_json = build_decl_target_json xref_json in
  let xrefs = add_xref target_json target_id symbol_pos xrefs in
  (xrefs, prog)

let process_container_xref
    (con_type, decl_pred) symbol_name symbol_pos (xrefs, prog) =
  process_xref
    (add_container_decl_fact decl_pred)
    (build_container_decl_json_ref con_type)
    symbol_name
    symbol_pos
    (xrefs, prog)

let process_enum_decl ctx source_map enm (all_decls, progress) =
  let (pos, id) = enm.c_name in
  match enm.c_enum with
  | None ->
    Hh_logger.log "WARNING: skipping enum with missing data - %s" id;
    (all_decls, progress)
  | Some enum_data ->
    let (enum_id, prog) = add_enum_decl_fact id progress in
    let enum_decl_ref = build_enum_decl_json_ref enum_id in
    let (_, prog) = add_decl_loc_fact pos enum_decl_ref prog in
    let (_, prog) = add_decl_span_fact enm.c_span enum_decl_ref prog in
    let (enumerators, decl_refs, prog) =
      List.fold_right
        enm.c_consts
        ~init:([], [], prog)
        ~f:(fun enumerator (decls, refs, prog) ->
          let (pos, id) = enumerator.cc_id in
          let (decl_id, prog) = add_enumerator_fact enum_id id prog in
          let ref_json = build_enumerator_decl_json_ref decl_id in
          let (_, prog) = add_decl_loc_fact pos ref_json prog in
          let prog =
            process_doc_comment enumerator.cc_doc_comment ref_json prog
          in
          (build_id_json decl_id :: decls, ref_json :: refs, prog))
    in
    let (_, prog) =
      add_enum_defn_fact ctx source_map enm enum_id enum_data enumerators prog
    in
    let prog = process_doc_comment enm.c_doc_comment enum_decl_ref prog in
    (all_decls @ (enum_decl_ref :: decl_refs), prog)

let process_enum_xref symbol_name pos (xrefs, prog) =
  process_xref
    add_enum_decl_fact
    build_enum_decl_json_ref
    symbol_name
    pos
    (xrefs, prog)

let process_func_decl ctx source_map fd (all_decls, progress) =
  let elem = fd.fd_fun in
  let (pos, id) = elem.f_name in
  let (decl_id, prog) =
    process_decl_loc
      add_func_decl_fact
      (add_func_defn_fact ctx source_map)
      build_func_decl_json_ref
      pos
      (Some elem.f_span)
      id
      fd
      elem.f_doc_comment
      progress
  in
  (all_decls @ [build_func_decl_json_ref decl_id], prog)

let process_function_xref symbol_name pos (xrefs, prog) =
  process_xref
    add_func_decl_fact
    build_func_decl_json_ref
    symbol_name
    pos
    (xrefs, prog)

let process_gconst_decl ctx source_map elem (all_decls, progress) =
  let (pos, id) = elem.cst_name in
  let (decl_id, prog) =
    process_decl_loc
      add_gconst_decl_fact
      (add_gconst_defn_fact ctx source_map)
      build_gconst_decl_json_ref
      pos
      (Some elem.cst_span)
      id
      elem
      None
      progress
  in
  (all_decls @ [build_gconst_decl_json_ref decl_id], prog)

let process_gconst_xref symbol_def pos (xrefs, prog) =
  process_xref
    add_gconst_decl_fact
    build_gconst_decl_json_ref
    symbol_def
    pos
    (xrefs, prog)

let process_member_xref ctx member pos mem_decl_fun ref_fun (xrefs, prog) =
  match Str.split (Str.regexp "::") member.full_name with
  | [] -> (xrefs, prog)
  | con_name :: _mem_name ->
    let con_name_with_ns = Utils.add_ns con_name in
    (match ServerSymbolDefinition.get_class_by_name ctx con_name_with_ns with
    | None ->
      Hh_logger.log
        "WARNING: could not find parent container %s processing reference to %s"
        con_name_with_ns
        member.full_name;
      (xrefs, prog)
    | Some cls ->
      if phys_equal cls.c_kind Cenum then
        match member.kind with
        | Const ->
          let (enum_id, prog) = add_enum_decl_fact con_name prog in
          process_xref
            (add_enumerator_fact enum_id)
            build_enumerator_decl_json_ref
            member.name
            pos
            (xrefs, prog)
        (* This includes references to built-in enum methods *)
        | _ -> (xrefs, prog)
      else
        let con_kind = get_parent_kind cls in
        let (con_type, decl_pred) = parent_decl_predicate con_kind in
        let (con_decl_id, prog) =
          add_container_decl_fact decl_pred con_name prog
        in
        process_xref
          (mem_decl_fun con_type con_decl_id)
          ref_fun
          member.name
          pos
          (xrefs, prog))

let process_attribute_xref ctx attr opt_info (xrefs, prog) =
  let get_con_preds_from_name con_name =
    let con_name_with_ns = Utils.add_ns con_name in
    match ServerSymbolDefinition.get_class_by_name ctx con_name_with_ns with
    | None ->
      Hh_logger.log
        "WARNING: could not find declaration container %s for attribute reference to %s"
        con_name_with_ns
        con_name;
      None
    | Some cls ->
      if phys_equal cls.c_kind Cenum then (
        Hh_logger.log
          "WARNING: unexpected enum %s processing attribute reference %s"
          con_name_with_ns
          con_name;
        None
      ) else
        Some (parent_decl_predicate (get_parent_kind cls))
  in
  (* Process <<__Override>>, for which we write a MethodOverrides fact
  instead of a cross-reference *)
  if String.equal attr.name "__Override" then
    match opt_info with
    | None ->
      Hh_logger.log "WARNING: no override info for <<__Override>> instance";
      (xrefs, prog)
    | Some info ->
      (match get_con_preds_from_name info.class_name with
      | None -> (xrefs, prog)
      | Some override_con_pred_types ->
        (match ServerSymbolDefinition.go ctx None attr with
        | None ->
          Hh_logger.log
            "WARNING: could not find source method for <<__Override>> %s::%s"
            info.class_name
            info.method_name;
          (xrefs, prog)
        | Some sym_def ->
          (match Str.split (Str.regexp "::") sym_def.full_name with
          | [] -> (xrefs, prog)
          | base_con_name :: _mem_name ->
            (match get_con_preds_from_name base_con_name with
            | None ->
              Hh_logger.log
                "WARNING: could not compute parent container type for override %s::%s"
                info.class_name
                info.method_name;
              (xrefs, prog)
            | Some base_con_pred_types ->
              let (_fid, prog) =
                add_method_overrides_fact
                  info.method_name
                  base_con_name
                  (fst base_con_pred_types)
                  info.class_name
                  (fst override_con_pred_types)
                  prog
              in
              (* Cross-references for overrides could be added to FileXRefs by calling
                'process_member_xref' here with 'sym_def' and 'attr.pos' *)
              (xrefs, prog)))))
  (* Ignore other built-in attributes *)
  else if String.is_prefix attr.name ~prefix:"__" then
    (xrefs, prog)
  (* Process user-defined attributes *)
  else
    try
      (* Look for a container declaration with the same name as the attribute,
      which will be where it is defined *)
      match get_con_preds_from_name attr.name with
      | None -> (xrefs, prog)
      | Some con_pred_types ->
        process_container_xref con_pred_types attr.name attr.pos (xrefs, prog)
    with e ->
      Hh_logger.log
        "WARNING: error processing reference to attribute %s\n: %s\n"
        attr.name
        (Exn.to_string e);
      (xrefs, prog)

let process_typedef_decl ctx source_map elem (all_decls, progress) =
  let (pos, id) = elem.t_name in
  let (decl_id, prog) =
    process_decl_loc
      add_typedef_decl_fact
      (add_typedef_defn_fact ctx source_map)
      build_typedef_decl_json_ref
      pos
      (Some elem.t_span)
      id
      elem
      None
      progress
  in
  (all_decls @ [build_typedef_decl_json_ref decl_id], prog)

let process_typedef_xref symbol_name pos (xrefs, prog) =
  process_xref
    add_typedef_decl_fact
    build_typedef_decl_json_ref
    symbol_name
    pos
    (xrefs, prog)

let process_decls ctx (files_info : file_info list) =
  let (source_map, progress) =
    List.fold
      files_info
      ~init:(SMap.empty, init_progress)
      ~f:(fun (fm, prog) (fp, _, source_text) ->
        let filepath = Relative_path.to_absolute fp in
        match source_text with
        | None ->
          Hh_logger.log "WARNING: no source text for %s" filepath;
          (fm, prog)
        | Some st ->
          let fm = SMap.add filepath st fm in
          let (_, prog) = add_file_lines_fact filepath st prog in
          (fm, prog))
  in
  List.fold files_info ~init:progress ~f:(fun prog (fp, tast, _) ->
      let (file_decls, prog) =
        List.fold tast ~init:([], prog) ~f:(fun acc def ->
            match def with
            | Class en when phys_equal en.c_kind Cenum ->
              process_enum_decl ctx source_map en acc
            | Class cd -> process_container_decl ctx source_map cd acc
            | Constant gd -> process_gconst_decl ctx source_map gd acc
            | Fun fd -> process_func_decl ctx source_map fd acc
            | Typedef td -> process_typedef_decl ctx source_map td acc
            | _ -> acc)
      in
      let filepath = Relative_path.to_absolute fp in
      let (_, prog) = add_file_decls_fact filepath file_decls prog in
      prog)

let process_xrefs ctx (tasts : Tast.program list) progress =
  List.fold tasts ~init:progress ~f:(fun prog tast ->
      let symbols = IdentifySymbolService.all_symbols ctx tast in
      (* file_xrefs : (Hh_json.json * Relative_path.t Pos.pos list) IMap.t SMap.t *)
      let (file_xrefs, prog) =
        List.fold symbols ~init:(SMap.empty, prog) ~f:(fun (xrefs, prog) occ ->
            if occ.is_declaration then
              (xrefs, prog)
            else
              match occ.type_ with
              | Attribute info ->
                process_attribute_xref ctx occ info (xrefs, prog)
              | _ ->
                let symbol_def_res = ServerSymbolDefinition.go ctx None occ in
                (match symbol_def_res with
                | None -> (xrefs, prog)
                | Some sym_def ->
                  let proc_mem = process_member_xref ctx sym_def occ.pos in
                  let name = sym_def.name in
                  let pos = occ.pos in
                  (match sym_def.kind with
                  | Class ->
                    let con_kind = parent_decl_predicate ClassContainer in
                    process_container_xref con_kind name pos (xrefs, prog)
                  | Const ->
                    (match occ.type_ with
                    | ClassConst _ ->
                      let ref_fun = build_class_const_decl_json_ref in
                      proc_mem add_class_const_decl_fact ref_fun (xrefs, prog)
                    | GConst -> process_gconst_xref name pos (xrefs, prog)
                    | _ -> (xrefs, prog))
                  | Enum -> process_enum_xref name pos (xrefs, prog)
                  | Function -> process_function_xref name pos (xrefs, prog)
                  | Interface ->
                    let con_kind = parent_decl_predicate InterfaceContainer in
                    process_container_xref con_kind name pos (xrefs, prog)
                  | Method ->
                    let ref_fun = build_method_decl_json_ref in
                    proc_mem add_method_decl_fact ref_fun (xrefs, prog)
                  | Property ->
                    let ref_fun = build_property_decl_json_ref in
                    proc_mem add_property_decl_fact ref_fun (xrefs, prog)
                  | Typeconst ->
                    let ref_fun = build_type_const_decl_json_ref in
                    proc_mem add_type_const_decl_fact ref_fun (xrefs, prog)
                  | Typedef -> process_typedef_xref name pos (xrefs, prog)
                  | Trait ->
                    let con_kind = parent_decl_predicate TraitContainer in
                    process_container_xref con_kind name pos (xrefs, prog)
                  | _ -> (xrefs, prog))))
      in
      SMap.fold
        (fun fp target_map acc ->
          let (_, res) = add_file_xrefs_fact fp target_map acc in
          res)
        file_xrefs
        prog)

let progress_to_json progress =
  let preds =
    (* The order is the reverse of how these items appear in the JSON,
    which is significant because later entries can refer to earlier ones
    by id only *)
    [
      ("src.FileLines.1", progress.resultJson.fileLines);
      ("hack.FileDeclarations.5", progress.resultJson.fileDeclarations);
      ("hack.FileXRefs.5", progress.resultJson.fileXRefs);
      ("hack.MethodOverrides.5", progress.resultJson.methodOverrides);
      ("hack.MethodDefinition.5", progress.resultJson.methodDefinition);
      ("hack.FunctionDefinition.5", progress.resultJson.functionDefinition);
      ("hack.EnumDefinition.5", progress.resultJson.enumDefinition);
      ("hack.ClassConstDefinition.5", progress.resultJson.classConstDefinition);
      ("hack.PropertyDefinition.5", progress.resultJson.propertyDefinition);
      ("hack.TypeConstDefinition.5", progress.resultJson.typeConstDefinition);
      ("hack.ClassDefinition.5", progress.resultJson.classDefinition);
      ("hack.TraitDefinition.5", progress.resultJson.traitDefinition);
      ("hack.InterfaceDefinition.5", progress.resultJson.interfaceDefinition);
      ("hack.TypedefDefinition.5", progress.resultJson.typedefDefinition);
      ("hack.GlobalConstDefinition.5", progress.resultJson.globalConstDefinition);
      ("hack.DeclarationComment.5", progress.resultJson.declarationComment);
      ("hack.DeclarationLocation.5", progress.resultJson.declarationLocation);
      ("hack.DeclarationSpan.5", progress.resultJson.declarationSpan);
      ("hack.MethodDeclaration.5", progress.resultJson.methodDeclaration);
      ("hack.ClassConstDeclaration.5", progress.resultJson.classConstDeclaration);
      ("hack.PropertyDeclaration.5", progress.resultJson.propertyDeclaration);
      ("hack.TypeConstDeclaration.5", progress.resultJson.typeConstDeclaration);
      ("hack.FunctionDeclaration.5", progress.resultJson.functionDeclaration);
      ("hack.Enumerator.5", progress.resultJson.enumerator);
      ("hack.EnumDeclaration.5", progress.resultJson.enumDeclaration);
      ("hack.ClassDeclaration.5", progress.resultJson.classDeclaration);
      ("hack.TraitDeclaration.5", progress.resultJson.traitDeclaration);
      ("hack.InterfaceDeclaration.5", progress.resultJson.interfaceDeclaration);
      ("hack.TypedefDeclaration.5", progress.resultJson.typedefDeclaration);
      ( "hack.GlobalConstDeclaration.5",
        progress.resultJson.globalConstDeclaration );
      ("hack.NamespaceDeclaration.5", progress.resultJson.namespaceDeclaration);
    ]
  in
  let json_array =
    List.fold preds ~init:[] ~f:(fun acc (pred, json_lst) ->
        JSON_Object
          [("predicate", JSON_String pred); ("facts", JSON_Array json_lst)]
        :: acc)
  in
  json_array

(* This function processes declarations, starting with an
empty fact cache. *)
let build_decls_json ctx files_info =
  let progress = process_decls ctx files_info in
  progress_to_json progress

(* This function processes cross-references, starting with an
empty fact cache. *)
let build_xrefs_json ctx tasts =
  let progress = process_xrefs ctx tasts init_progress in
  progress_to_json progress

(* This function processes both declarations and cross-references,
sharing the declaration fact cache between them. *)
let build_json ctx files_info =
  let progress = process_decls ctx files_info in
  let progress =
    process_xrefs
      ctx
      (List.map files_info ~f:(fun (_, tast, _) -> tast))
      progress
  in
  progress_to_json progress
