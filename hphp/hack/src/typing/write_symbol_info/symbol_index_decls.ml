(*
 * Copyright (c) Meta, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
open Hh_prelude
open Symbol_glean_schema.Hack
module Add_fact = Symbol_add_fact
module Fact_acc = Symbol_predicate.Fact_acc
module Fact_id = Symbol_fact_id
module Util = Symbol_json_util
module Predicate = Symbol_predicate
module File_info = Symbol_file_info
module XRefs = Symbol_xrefs

let process_doc_comment
    (comment : Aast.doc_comment option)
    (path : string)
    (decl_ref : Declaration.t)
    (prog : Fact_acc.t) : Fact_acc.t =
  match comment with
  | None -> prog
  | Some (pos, _doc) -> snd (Add_fact.decl_comment ~path pos decl_ref prog)

let process_loc_span
    path (pos : Pos.t) (span : Pos.t) (ref : Declaration.t) (prog : Fact_acc.t)
    : Fact_acc.t =
  let (_, prog) = Add_fact.decl_loc ~path pos ref prog in
  let (_, prog) = Add_fact.decl_span ~path span ref prog in
  prog

let process_decl_loc
    (decl_fun : string -> Fact_acc.t -> Fact_id.t * Fact_acc.t)
    (defn_fun : 'elem -> Fact_id.t -> Fact_acc.t -> Fact_id.t * Fact_acc.t)
    (decl_ref_fun : Fact_id.t -> Declaration.t)
    ~(path : string)
    (pos : Pos.t)
    (span : Pos.t)
    (id : Ast_defs.id_)
    (elem : 'elem)
    (doc : Aast.doc_comment option)
    (progress : Fact_acc.t) : Fact_id.t * Fact_acc.t =
  let (decl_id, prog) = decl_fun id progress in
  let (_, prog) = defn_fun elem decl_id prog in
  let ref = decl_ref_fun decl_id in
  let prog = process_doc_comment doc path ref prog in
  let prog = process_loc_span path pos span ref prog in
  (decl_id, prog)

let process_member
    add_member_decl
    build_decl_ref
    ~container_type
    ~container_id
    (member_ids, prog)
    File_info.{ name } =
  let (member_id, prog) =
    add_member_decl container_type container_id name prog
  in
  (build_decl_ref member_id :: member_ids, prog)

let process_member_cluster mc prog =
  let (container_type, container_id, prog) =
    let parent_kind =
      Predicate.get_parent_kind mc.File_info.container.File_info.kind
    in
    let decl_pred = Predicate.parent_decl_predicate parent_kind in
    let (con_decl_id, prog) =
      Add_fact.container_decl
        decl_pred
        mc.File_info.container.File_info.name
        prog
    in
    (parent_kind, con_decl_id, prog)
  in
  let process add_member_decl build_decl_ref members acc =
    List.fold
      members
      ~init:acc
      ~f:
        (process_member
           add_member_decl
           build_decl_ref
           ~container_type
           ~container_id)
  in
  ([], prog)
  |> process
       Add_fact.method_decl
       (fun x -> Declaration.Method (MethodDeclaration.Id x))
       mc.File_info.methods
  |> process
       Add_fact.property_decl
       (fun x -> Declaration.Property_ (PropertyDeclaration.Id x))
       mc.File_info.properties
  |> process
       Add_fact.type_const_decl
       (fun x -> Declaration.TypeConst (TypeConstDeclaration.Id x))
       mc.File_info.type_constants
  |> process
       Add_fact.class_const_decl
       (fun x -> Declaration.ClassConst (ClassConstDeclaration.Id x))
       mc.File_info.class_constants

let process_container_decl
    ctx path source_text con member_clusters (xrefs, all_decls, progress) =
  let (con_pos, con_name) = con.c_name in
  let parent_kind = Predicate.get_parent_kind con.c_kind in
  let decl_pred = Predicate.parent_decl_predicate parent_kind in
  let (con_decl_id, prog) =
    Add_fact.container_decl decl_pred con_name progress
  in
  let (prop_decls, prog) =
    List.fold_right con.c_vars ~init:([], prog) ~f:(fun prop (decls, prog) ->
        let (pos, id) = prop.cv_id in
        let (decl_id, prog) =
          process_decl_loc
            (Add_fact.property_decl parent_kind con_decl_id)
            (Add_fact.property_defn ctx source_text)
            (fun x -> Declaration.Property_ (PropertyDeclaration.Id x))
            ~path
            pos
            prop.cv_span
            id
            prop
            prop.cv_doc_comment
            prog
        in
        (Declaration.Property_ (PropertyDeclaration.Id decl_id) :: decls, prog))
  in
  let (class_const_decls, prog) =
    List.fold_right con.c_consts ~init:([], prog) ~f:(fun const (decls, prog) ->
        let (pos, id) = const.cc_id in
        let (decl_id, prog) =
          process_decl_loc
            (Add_fact.class_const_decl parent_kind con_decl_id)
            (Add_fact.class_const_defn ctx source_text)
            (fun x -> Declaration.ClassConst (ClassConstDeclaration.Id x))
            ~path
            pos
            const.cc_span
            id
            const
            const.cc_doc_comment
            prog
        in
        ( Declaration.ClassConst (ClassConstDeclaration.Id decl_id) :: decls,
          prog ))
  in
  let (type_const_decls, prog) =
    List.fold_right
      con.c_typeconsts
      ~init:([], prog)
      ~f:(fun tc (decls, prog) ->
        let (pos, id) = tc.c_tconst_name in
        let (decl_id, prog) =
          process_decl_loc
            (Add_fact.type_const_decl parent_kind con_decl_id)
            (Add_fact.type_const_defn ctx source_text)
            (fun x -> Declaration.TypeConst (TypeConstDeclaration.Id x))
            ~path
            pos
            tc.c_tconst_span
            id
            tc
            tc.c_tconst_doc_comment
            prog
        in
        (Declaration.TypeConst (TypeConstDeclaration.Id decl_id) :: decls, prog))
  in
  let (method_decls, prog) =
    List.fold_right con.c_methods ~init:([], prog) ~f:(fun meth (decls, prog) ->
        let (pos, id) = meth.m_name in
        let (decl_id, prog) =
          process_decl_loc
            (Add_fact.method_decl parent_kind con_decl_id)
            (Add_fact.method_defn ctx source_text)
            (fun x -> Declaration.Method (MethodDeclaration.Id x))
            ~path
            pos
            meth.m_span
            id
            meth
            meth.m_doc_comment
            prog
        in
        (Declaration.Method (MethodDeclaration.Id decl_id) :: decls, prog))
  in
  let members =
    type_const_decls @ class_const_decls @ prop_decls @ method_decls
  in
  let (prog, inherited_member_clusters) =
    List.fold_map member_clusters ~init:prog ~f:(fun prog mc ->
        let (members, prog) = process_member_cluster mc prog in
        let (mc, prog) = Add_fact.member_cluster ~members prog in
        (prog, mc))
  in
  let prog =
    if not @@ List.is_empty inherited_member_clusters then
      let (_id, prog) =
        Add_fact.inherited_members
          ~container_type:parent_kind
          ~container_id:con_decl_id
          ~member_clusters:inherited_member_clusters
          prog
      in
      prog
    else
      prog
  in
  let (_, prog) =
    Add_fact.container_defn ctx source_text con con_decl_id members prog
  in
  let ref = Predicate.container_ref parent_kind con_decl_id in
  let prog = process_loc_span path con_pos con.c_span ref prog in
  let all_decls = all_decls @ [ref] @ members in
  let prog = process_doc_comment con.c_doc_comment path ref prog in
  (xrefs, all_decls, prog)

let process_enum_decl ctx path source_text enm (xrefs, all_decls, progress) =
  let (pos, id) = enm.c_name in
  match enm.c_enum with
  | None ->
    Hh_logger.log "WARNING: skipping enum with missing data - %s" id;
    (xrefs, all_decls, progress)
  | Some enum_data ->
    let (enum_id, prog) = Add_fact.enum_decl id progress in
    let enum_decl_ref =
      Declaration.Container
        (ContainerDeclaration.Enum_ (EnumDeclaration.Id enum_id))
    in
    let prog = process_loc_span path pos enm.c_span enum_decl_ref prog in
    let (enumerators, decl_refs, prog) =
      List.fold_right
        enm.c_consts
        ~init:([], [], prog)
        ~f:(fun enumerator (decls, refs, prog) ->
          let (pos, id) = enumerator.cc_id in
          let (decl_id, prog) = Add_fact.enumerator enum_id id prog in
          let _span = enumerator.cc_span in
          let ref = Declaration.Enumerator (Enumerator.Id decl_id) in
          (* TODO we're using pos instead of _span. _span refer to the whole enum container,
             rather than the enumerator *)
          let prog = process_loc_span path pos pos ref prog in
          let prog =
            process_doc_comment enumerator.cc_doc_comment path ref prog
          in
          (decl_id :: decls, ref :: refs, prog))
    in
    let (_, prog) =
      Add_fact.enum_defn ctx source_text enm enum_id enum_data enumerators prog
    in
    let prog = process_doc_comment enm.c_doc_comment path enum_decl_ref prog in
    (xrefs, all_decls @ (enum_decl_ref :: decl_refs), prog)

let process_func_decl ctx path source_text fd (xrefs, all_decls, progress) =
  let elem = fd.fd_fun in
  let (pos, id) = fd.fd_name in
  let (decl_id, prog) =
    process_decl_loc
      Add_fact.func_decl
      (Add_fact.func_defn ctx source_text)
      (fun x -> Declaration.Function_ (FunctionDeclaration.Id x))
      ~path
      pos
      elem.f_span
      id
      fd
      elem.f_doc_comment
      progress
  in
  ( xrefs,
    all_decls @ [Declaration.Function_ (FunctionDeclaration.Id decl_id)],
    prog )

let process_gconst_decl ctx path source_text elem (xrefs, all_decls, progress) =
  let (pos, id) = elem.cst_name in
  let (decl_id, prog) =
    process_decl_loc
      Add_fact.gconst_decl
      (Add_fact.gconst_defn ctx source_text)
      (fun x -> Declaration.GlobalConst (GlobalConstDeclaration.Id x))
      ~path
      pos
      elem.cst_span
      id
      elem
      None
      progress
  in
  ( xrefs,
    all_decls @ [Declaration.GlobalConst (GlobalConstDeclaration.Id decl_id)],
    prog )

let process_typedef_decl ctx path source_text elem (xrefs, all_decls, progress)
    =
  let (pos, id) = elem.t_name in
  let (decl_id, prog) =
    process_decl_loc
      Add_fact.typedef_decl
      (Add_fact.typedef_defn ctx source_text)
      (fun x -> Declaration.Typedef_ (TypedefDeclaration.Id x))
      ~path
      pos
      elem.t_span
      id
      elem
      elem.t_doc_comment
      progress
  in
  ( xrefs,
    all_decls @ [Declaration.Typedef_ (TypedefDeclaration.Id decl_id)],
    prog )

let process_module_decl ctx path source_text elem (xrefs, all_decls, progress) =
  let (pos, id) = elem.md_name in
  let (decl_id, prog) =
    process_decl_loc
      Add_fact.module_decl
      (Add_fact.module_defn ctx source_text)
      (fun x -> Declaration.ModuleDeclaration (ModuleDeclaration.Id x))
      ~path
      pos
      elem.md_span
      id
      elem
      None
      progress
  in
  ( xrefs,
    all_decls @ [Declaration.ModuleDeclaration (ModuleDeclaration.Id decl_id)],
    prog )

let process_namespace_decl ~path (pos, id) (all_decls, progress) =
  let (decl_id, prog) =
    process_decl_loc
      Add_fact.namespace_decl
      (fun () fid acc -> (fid, acc))
      (fun x -> Declaration.Namespace_ (NamespaceDeclaration.Id x))
      ~path
      pos
      pos (* no span for a namespace decl, we re-use the location *)
      id
      ()
      None
      progress
  in
  (all_decls @ [Declaration.Namespace_ (NamespaceDeclaration.Id decl_id)], prog)

let process_cst_decls st path root (decls, prog) =
  let open Full_fidelity_positioned_syntax in
  match root.syntax with
  | Script { script_declarations = { syntax = SyntaxList cst_decls; _ } } ->
    List.fold cst_decls ~init:(decls, prog) ~f:(fun acc cst_decl ->
        match cst_decl.syntax with
        | NamespaceDeclaration
            {
              namespace_header =
                {
                  syntax =
                    NamespaceDeclarationHeader
                      { namespace_name = { syntax = ns_ast; _ }; _ };
                  _;
                };
              _;
            } ->
          (try
             let (pos, id) = Util.namespace_ast_to_pos_id ns_ast st in
             process_namespace_decl ~path (pos, id) acc
           with
          | Util.Empty_namespace -> acc
          | Util.Ast_error ->
            Hh_logger.log "Couldn't extract namespace from declaration";
            acc)
        | _ -> acc)
  | _ ->
    Hh_logger.log "Couldn't extract namespaces declarations";
    (decls, prog)

let process_mod_xref prog xrefs (pos, id) =
  let (target_id, prog) = Add_fact.module_decl id prog in
  let target =
    XRefTarget.Declaration
      (Declaration.ModuleDeclaration (ModuleDeclaration.Id target_id))
  in
  (XRefs.add xrefs target_id pos XRefs.{ target; receiver_type = None }, prog)

let process_tast_decls ctx ~path tast source_text (decls, prog) =
  List.fold tast ~init:(XRefs.empty, decls, prog) ~f:(fun acc (def, im) ->
      match def with
      | Class en when Util.is_enum_or_enum_class en.c_kind ->
        process_enum_decl ctx path source_text en acc
      | Class cd -> process_container_decl ctx path source_text cd im acc
      | Constant gd -> process_gconst_decl ctx path source_text gd acc
      | Fun fd -> process_func_decl ctx path source_text fd acc
      | Typedef td -> process_typedef_decl ctx path source_text td acc
      | Module md -> process_module_decl ctx path source_text md acc
      | SetModule sm ->
        let (xrefs, _, prog) = acc in
        let (xrefs, prog) = process_mod_xref prog xrefs sm in
        (xrefs, decls, prog)
      | _ -> acc)

let process_decls ctx prog File_info.{ path; tast; source_text; cst; _ } =
  Fact_acc.set_ownership_unit prog (Some path);
  let (_, prog) = Add_fact.file_lines ~path source_text prog in
  let (mod_xrefs, decls, prog) =
    process_tast_decls ctx ~path tast source_text ([], prog)
  in
  let (decls, prog) = process_cst_decls source_text path cst (decls, prog) in
  (mod_xrefs, Add_fact.file_decls ~path decls prog |> snd)
