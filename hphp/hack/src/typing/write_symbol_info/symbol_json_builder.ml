(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
open Hh_prelude
module Add_fact = Symbol_add_fact
module Fact_acc = Symbol_predicate.Fact_acc
module Fact_id = Symbol_fact_id
module Util = Symbol_json_util
module Build = Symbol_build_json
module Predicate = Symbol_predicate
module File_info = Symbol_file_info

let is_enum_or_enum_class = function
  | Ast_defs.Cenum
  | Ast_defs.Cenum_class _ ->
    true
  | Ast_defs.(Cinterface | Cclass _ | Ctrait) -> false

(* These functions define the process to go through when
encountering symbols of a given type. *)

let process_doc_comment
    (comment : Aast.doc_comment option)
    (decl_ref_json : Hh_json.json)
    (prog : Fact_acc.t) : Fact_acc.t =
  match comment with
  | None -> prog
  | Some (pos, _doc) -> snd (Add_fact.decl_comment pos decl_ref_json prog)

let process_span
    (span : Relative_path.t Pos.pos option)
    (ref_json : Hh_json.json)
    (prog : Fact_acc.t) : Fact_acc.t =
  match span with
  | None -> prog
  | Some span_pos -> snd (Add_fact.decl_span span_pos ref_json prog)

let process_decl_loc
    (decl_fun : string -> Fact_acc.t -> Fact_id.t * Fact_acc.t)
    (defn_fun : 'elem -> Fact_id.t -> Fact_acc.t -> Fact_id.t * Fact_acc.t)
    (decl_ref_fun : Fact_id.t -> Hh_json.json)
    (pos : Relative_path.t Pos.pos)
    (span : Relative_path.t Pos.pos option)
    (id : Ast_defs.id_)
    (elem : 'elem)
    (doc : Aast.doc_comment option)
    (progress : Fact_acc.t) : Fact_id.t * Fact_acc.t =
  let (decl_id, prog) = decl_fun id progress in
  let (_, prog) = defn_fun elem decl_id prog in
  let ref_json = decl_ref_fun decl_id in
  let (_, prog) = Add_fact.decl_loc pos ref_json prog in
  let prog = process_doc_comment doc ref_json prog in
  let prog = process_span span ref_json prog in
  (decl_id, prog)

let process_container_decl ctx source_text con (all_decls, progress) =
  let (con_pos, con_name) = con.c_name in
  let (con_type, decl_pred) =
    Predicate.parent_decl_predicate (Predicate.get_parent_kind con)
  in
  let (con_decl_id, prog) =
    Add_fact.container_decl decl_pred con_name progress
  in
  let (prop_decls, prog) =
    List.fold_right con.c_vars ~init:([], prog) ~f:(fun prop (decls, prog) ->
        let (pos, id) = prop.cv_id in
        let (decl_id, prog) =
          process_decl_loc
            (Add_fact.property_decl con_type con_decl_id)
            (Add_fact.property_defn ctx source_text)
            Build.build_property_decl_json_ref
            pos
            (Some prop.cv_span)
            id
            prop
            prop.cv_doc_comment
            prog
        in
        (Build.build_property_decl_json_ref decl_id :: decls, prog))
  in
  let (class_const_decls, prog) =
    List.fold_right con.c_consts ~init:([], prog) ~f:(fun const (decls, prog) ->
        let (pos, id) = const.cc_id in
        let (decl_id, prog) =
          process_decl_loc
            (Add_fact.class_const_decl con_type con_decl_id)
            (Add_fact.class_const_defn ctx source_text)
            Build.build_class_const_decl_json_ref
            pos
            None
            id
            const
            const.cc_doc_comment
            prog
        in
        (Build.build_class_const_decl_json_ref decl_id :: decls, prog))
  in
  let (type_const_decls, prog) =
    List.fold_right
      con.c_typeconsts
      ~init:([], prog)
      ~f:(fun tc (decls, prog) ->
        let (pos, id) = tc.c_tconst_name in
        let (decl_id, prog) =
          process_decl_loc
            (Add_fact.type_const_decl con_type con_decl_id)
            (Add_fact.type_const_defn ctx source_text)
            Build.build_type_const_decl_json_ref
            pos
            (Some tc.c_tconst_span)
            id
            tc
            tc.c_tconst_doc_comment
            prog
        in
        (Build.build_type_const_decl_json_ref decl_id :: decls, prog))
  in
  let (method_decls, prog) =
    List.fold_right con.c_methods ~init:([], prog) ~f:(fun meth (decls, prog) ->
        let (pos, id) = meth.m_name in
        let (decl_id, prog) =
          process_decl_loc
            (Add_fact.method_decl con_type con_decl_id)
            (Add_fact.method_defn ctx source_text)
            Build.build_method_decl_json_ref
            pos
            (Some meth.m_span)
            id
            meth
            meth.m_doc_comment
            prog
        in
        (Build.build_method_decl_json_ref decl_id :: decls, prog))
  in
  let members =
    type_const_decls @ class_const_decls @ prop_decls @ method_decls
  in
  let (_, prog) =
    Add_fact.container_defn ctx source_text con con_decl_id members prog
  in
  let ref_json = Build.build_container_decl_json_ref con_type con_decl_id in
  let (_, prog) = Add_fact.decl_loc con_pos ref_json prog in
  let (_, prog) = Add_fact.decl_span con.c_span ref_json prog in
  let all_decls = all_decls @ [ref_json] @ members in
  let prog = process_doc_comment con.c_doc_comment ref_json prog in
  (all_decls, prog)

let process_xref decl_fun decl_ref_fun symbol_name symbol_pos (xrefs, prog) =
  let (target_id, prog) = decl_fun symbol_name prog in
  let xref_json = decl_ref_fun target_id in
  let target_json = Build.build_decl_target_json xref_json in
  let xrefs = Util.add_xref target_json target_id symbol_pos xrefs in
  (xrefs, prog)

let process_container_xref
    (con_type, decl_pred) symbol_name symbol_pos (xrefs, prog) =
  process_xref
    (Add_fact.container_decl decl_pred)
    (Build.build_container_decl_json_ref con_type)
    symbol_name
    symbol_pos
    (xrefs, prog)

let process_enum_decl ctx source_text enm (all_decls, progress) =
  let (pos, id) = enm.c_name in
  match enm.c_enum with
  | None ->
    Hh_logger.log "WARNING: skipping enum with missing data - %s" id;
    (all_decls, progress)
  | Some enum_data ->
    let (enum_id, prog) = Add_fact.enum_decl id progress in
    let enum_decl_ref = Build.build_enum_decl_json_ref enum_id in
    let (_, prog) = Add_fact.decl_loc pos enum_decl_ref prog in
    let (_, prog) = Add_fact.decl_span enm.c_span enum_decl_ref prog in
    let (enumerators, decl_refs, prog) =
      List.fold_right
        enm.c_consts
        ~init:([], [], prog)
        ~f:(fun enumerator (decls, refs, prog) ->
          let (pos, id) = enumerator.cc_id in
          let (decl_id, prog) = Add_fact.enumerator enum_id id prog in
          let ref_json = Build.build_enumerator_decl_json_ref decl_id in
          let (_, prog) = Add_fact.decl_loc pos ref_json prog in
          let prog =
            process_doc_comment enumerator.cc_doc_comment ref_json prog
          in
          (Build.build_id_json decl_id :: decls, ref_json :: refs, prog))
    in
    let (_, prog) =
      Add_fact.enum_defn ctx source_text enm enum_id enum_data enumerators prog
    in
    let prog = process_doc_comment enm.c_doc_comment enum_decl_ref prog in
    (all_decls @ enum_decl_ref :: decl_refs, prog)

let process_enum_xref symbol_name pos (xrefs, prog) =
  process_xref
    Add_fact.enum_decl
    Build.build_enum_decl_json_ref
    symbol_name
    pos
    (xrefs, prog)

let process_func_decl ctx source_text fd (all_decls, progress) =
  let elem = fd.fd_fun in
  let (pos, id) = elem.f_name in
  let (decl_id, prog) =
    process_decl_loc
      Add_fact.func_decl
      (Add_fact.func_defn ctx source_text)
      Build.build_func_decl_json_ref
      pos
      (Some elem.f_span)
      id
      fd
      elem.f_doc_comment
      progress
  in
  (all_decls @ [Build.build_func_decl_json_ref decl_id], prog)

let process_function_xref symbol_name pos (xrefs, prog) =
  process_xref
    Add_fact.func_decl
    Build.build_func_decl_json_ref
    symbol_name
    pos
    (xrefs, prog)

let process_gconst_decl ctx source_text elem (all_decls, progress) =
  let (pos, id) = elem.cst_name in
  let (decl_id, prog) =
    process_decl_loc
      Add_fact.gconst_decl
      (Add_fact.gconst_defn ctx source_text)
      Build.build_gconst_decl_json_ref
      pos
      (Some elem.cst_span)
      id
      elem
      None
      progress
  in
  (all_decls @ [Build.build_gconst_decl_json_ref decl_id], prog)

let process_gconst_xref symbol_def pos (xrefs, prog) =
  process_xref
    Add_fact.gconst_decl
    Build.build_gconst_decl_json_ref
    symbol_def
    pos
    (xrefs, prog)

let process_member_xref ctx member pos mem_decl_fun ref_fun (xrefs, prog) =
  let SymbolDefinition.{ name; full_name; kind; _ } = member in
  match Str.split (Str.regexp "::") full_name with
  | [] -> (xrefs, prog)
  | con_name :: _mem_name ->
    let con_name_with_ns = Utils.add_ns con_name in
    (match ServerSymbolDefinition.get_class_by_name ctx con_name_with_ns with
    | None ->
      Hh_logger.log
        "WARNING: could not find parent container %s processing reference to %s"
        con_name_with_ns
        full_name;
      (xrefs, prog)
    | Some cls ->
      if is_enum_or_enum_class cls.c_kind then
        match kind with
        | SymbolDefinition.Const ->
          let (enum_id, prog) = Add_fact.enum_decl con_name prog in
          process_xref
            (Add_fact.enumerator enum_id)
            Build.build_enumerator_decl_json_ref
            name
            pos
            (xrefs, prog)
        (* This includes references to built-in enum methods *)
        | _ -> (xrefs, prog)
      else
        let con_kind = Predicate.get_parent_kind cls in
        let (con_type, decl_pred) = Predicate.parent_decl_predicate con_kind in
        let (con_decl_id, prog) =
          Add_fact.container_decl decl_pred con_name prog
        in
        process_xref
          (mem_decl_fun con_type con_decl_id)
          ref_fun
          name
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
      if is_enum_or_enum_class cls.c_kind then (
        Hh_logger.log
          "WARNING: unexpected enum %s processing attribute reference %s"
          con_name_with_ns
          con_name;
        None
      ) else
        Some Predicate.(parent_decl_predicate (get_parent_kind cls))
  in
  (* Process <<__Override>>, for which we write a MethodOverrides fact
     instead of a cross-reference *)
  let SymbolOccurrence.{ name; pos; _ } = attr in
  if String.equal name "__Override" then
    match opt_info with
    | None ->
      Hh_logger.log "WARNING: no override info for <<__Override>> instance";
      (xrefs, prog)
    | Some SymbolOccurrence.{ class_name; method_name; _ } ->
      (match get_con_preds_from_name class_name with
      | None -> (xrefs, prog)
      | Some override_con_pred_types ->
        (match ServerSymbolDefinition.go ctx None attr with
        | None ->
          Hh_logger.log
            "WARNING: could not find source method for <<__Override>> %s::%s"
            class_name
            method_name;
          (xrefs, prog)
        | Some sym_def ->
          (match
             Str.split (Str.regexp "::") sym_def.SymbolDefinition.full_name
           with
          | [] -> (xrefs, prog)
          | base_con_name :: _mem_name ->
            (match get_con_preds_from_name base_con_name with
            | None ->
              Hh_logger.log
                "WARNING: could not compute parent container type for override %s::%s"
                class_name
                method_name;
              (xrefs, prog)
            | Some base_con_pred_types ->
              let (_fid, prog) =
                Add_fact.method_overrides
                  method_name
                  base_con_name
                  (fst base_con_pred_types)
                  class_name
                  (fst override_con_pred_types)
                  prog
              in
              (* Cross-references for overrides could be added to FileXRefs by calling
                 'process_member_xref' here with 'sym_def' and 'attr.pos' *)
              (xrefs, prog)))))
  (* Ignore other built-in attributes *)
  else if String.is_prefix name ~prefix:"__" then
    (xrefs, prog)
  (* Process user-defined attributes *)
  else
    try
      (* Look for a container declaration with the same name as the attribute,
         which will be where it is defined *)
      match get_con_preds_from_name name with
      | None -> (xrefs, prog)
      | Some con_pred_types ->
        process_container_xref con_pred_types name pos (xrefs, prog)
    with
    | e ->
      Hh_logger.log
        "WARNING: error processing reference to attribute %s\n: %s\n"
        name
        (Exn.to_string e);
      (xrefs, prog)

let process_typedef_decl ctx source_text elem (all_decls, progress) =
  let (pos, id) = elem.t_name in
  let (decl_id, prog) =
    process_decl_loc
      Add_fact.typedef_decl
      (Add_fact.typedef_defn ctx source_text)
      Build.build_typedef_decl_json_ref
      pos
      (Some elem.t_span)
      id
      elem
      None
      progress
  in
  (all_decls @ [Build.build_typedef_decl_json_ref decl_id], prog)

let process_typedef_xref symbol_name pos (xrefs, prog) =
  process_xref
    Add_fact.typedef_decl
    Build.build_typedef_decl_json_ref
    symbol_name
    pos
    (xrefs, prog)

let process_decls ctx prog File_info.{ path; tast; source_text } =
  let filepath = Relative_path.to_absolute path in
  Fact_acc.set_ownership_unit prog (Some filepath);
  let (file_decls, prog) =
    let (_, prog) = Add_fact.file_lines filepath source_text prog in
    List.fold tast ~init:([], prog) ~f:(fun acc def ->
        match def with
        | Class en when is_enum_or_enum_class en.c_kind ->
          process_enum_decl ctx source_text en acc
        | Class cd -> process_container_decl ctx source_text cd acc
        | Constant gd -> process_gconst_decl ctx source_text gd acc
        | Fun fd -> process_func_decl ctx source_text fd acc
        | Typedef td -> process_typedef_decl ctx source_text td acc
        | _ -> acc)
  in
  Add_fact.file_decls filepath file_decls prog |> snd

let process_xrefs ctx prog File_info.{ path; tast; _ } =
  let open SymbolDefinition in
  let open SymbolOccurrence in
  let symbols = IdentifySymbolService.all_symbols ctx tast in
  let filepath = Relative_path.to_absolute path in
  Fact_acc.set_ownership_unit prog (Some filepath);
  (* file_xrefs : (Hh_json.json * Relative_path.t Pos.pos list) Fact_id.Map.t SMap.t *)
  let (file_xrefs, prog) =
    List.fold symbols ~init:(SMap.empty, prog) ~f:(fun (xrefs, prog) occ ->
        if occ.is_declaration then
          (xrefs, prog)
        else
          let pos = occ.pos in
          match occ.type_ with
          | Attribute info -> process_attribute_xref ctx occ info (xrefs, prog)
          | _ ->
            let symbol_def_res = ServerSymbolDefinition.go ctx None occ in
            (match symbol_def_res with
            | None ->
              (* no symbol info - likely dynamic *)
              (match occ.type_ with
              | Method (receiver_class, name) ->
                let (target_id, prog) =
                  Add_fact.method_occ receiver_class name prog
                in
                let xref_json = Build.build_method_occ_json_ref target_id in
                let _target_json = Build.build_occ_target_json xref_json in
                (xrefs, prog)
              | _ -> (xrefs, prog))
            | Some sym_def ->
              let proc_mem = process_member_xref ctx sym_def occ.pos in
              let name = sym_def.name in
              (match sym_def.kind with
              | Class ->
                let con_kind =
                  Predicate.parent_decl_predicate Predicate.ClassContainer
                in
                process_container_xref con_kind name pos (xrefs, prog)
              | Const ->
                (match occ.type_ with
                | ClassConst _ ->
                  let ref_fun = Build.build_class_const_decl_json_ref in
                  proc_mem Add_fact.class_const_decl ref_fun (xrefs, prog)
                | GConst -> process_gconst_xref name pos (xrefs, prog)
                | _ -> (xrefs, prog))
              | Enum -> process_enum_xref name pos (xrefs, prog)
              | Function -> process_function_xref name pos (xrefs, prog)
              | Interface ->
                let con_kind =
                  Predicate.parent_decl_predicate Predicate.InterfaceContainer
                in
                process_container_xref con_kind name pos (xrefs, prog)
              | Method ->
                let ref_fun = Build.build_method_decl_json_ref in
                proc_mem Add_fact.method_decl ref_fun (xrefs, prog)
              | Property ->
                let ref_fun = Build.build_property_decl_json_ref in
                proc_mem Add_fact.property_decl ref_fun (xrefs, prog)
              | Typeconst ->
                let ref_fun = Build.build_type_const_decl_json_ref in
                proc_mem Add_fact.type_const_decl ref_fun (xrefs, prog)
              | Typedef -> process_typedef_xref name pos (xrefs, prog)
              | Trait ->
                let con_kind =
                  Predicate.parent_decl_predicate Predicate.TraitContainer
                in
                process_container_xref con_kind name pos (xrefs, prog)
              | _ -> (xrefs, prog))))
  in
  SMap.fold
    (fun path target_map acc -> Add_fact.file_xrefs path target_map acc |> snd)
    file_xrefs
    prog

let process_xrefs_all_files ctx files_info progress =
  List.fold files_info ~init:progress ~f:(process_xrefs ctx)

let process_decls_all_files ctx files_info progress =
  List.fold files_info ~init:progress ~f:(process_decls ctx)

(* This function processes declarations, starting with an
empty fact cache. *)
let build_decls_json ctx files_info ~ownership =
  Fact_acc.init ~ownership
  |> process_decls_all_files ctx files_info
  |> Fact_acc.to_json

(* This function processes cross-references, starting with an
empty fact cache. *)
let build_xrefs_json ctx files_info ~ownership =
  Fact_acc.init ~ownership
  |> process_xrefs_all_files ctx files_info
  |> Fact_acc.to_json

(* This function processes both declarations and cross-references,
sharing the declaration fact cache between them. *)
let build_json ctx files_info ~ownership =
  Fact_acc.init ~ownership
  |> process_decls_all_files ctx files_info
  |> process_xrefs_all_files ctx files_info
  |> Fact_acc.to_json
