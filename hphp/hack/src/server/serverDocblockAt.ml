(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module Cls = Decl_provider.Class

let get_all_ancestors class_name =
  let rec helper classes_to_check cinfos seen_classes =
    match classes_to_check with
    | [] -> cinfos
    | class_name :: classes when SSet.mem class_name seen_classes ->
      helper classes cinfos seen_classes
    | class_name :: classes ->
      begin
        match Decl_provider.get_class class_name with
        | None -> helper classes cinfos seen_classes
        | Some class_info ->
          let ancestors =
            Cls.all_ancestor_names class_info
            |> Sequence.fold ~init:classes ~f:(fun acc cid -> cid :: acc)
          in
          helper
            ancestors
            (class_info :: cinfos)
            (SSet.add class_name seen_classes)
      end
  in
  helper [class_name] [] SSet.empty

let get_docblock_for_member class_info member_name =
  Option.Monad_infix.(
    Cls.get_method class_info member_name
    >>= fun member ->
    match Lazy.force member.Typing_defs.ce_type with
    | (_, Typing_defs.Tfun ft) ->
      let pos = ft.Typing_defs.ft_pos in
      let filename = Pos.filename pos in
      File_provider.get_contents filename
      >>= fun contents ->
      ServerSymbolDefinition.get_definition_cst_node_from_pos
        SymbolDefinition.Method
        (Full_fidelity_source_text.make filename contents)
        pos
      >>= Docblock_finder.get_docblock
    | _ -> None)

let render_ancestor_docblocks docblocks =
  let docblocks_to_ancestor =
    docblocks
    |> List.fold ~init:SMap.empty ~f:(fun acc (class_name, docblock) ->
           let existing_ancestors =
             match SMap.find_opt docblock acc with
             | None -> []
             | Some lst -> lst
           in
           SMap.add docblock (class_name :: existing_ancestors) acc)
  in
  match SMap.elements docblocks_to_ancestor with
  | [] -> None
  | [(docblock, _)] -> Some docblock
  | docblock_ancestors_pairs ->
    docblock_ancestors_pairs
    |> List.map ~f:(fun (docblock, ancestors) ->
           let ancestors_str =
             String.concat ~sep:", " (List.map ~f:Utils.strip_ns ancestors)
           in
           Printf.sprintf "%s\n(from %s)" docblock ancestors_str)
    |> String.concat ~sep:"\n\n---\n\n"
    |> (fun results -> Some results)

let fallback class_name member_name =
  let rec all_interfaces_or_first_class_docblock
      seen_interfaces ancestors_to_check =
    match ancestors_to_check with
    | [] -> seen_interfaces
    | ancestor :: ancestors ->
      begin
        match get_docblock_for_member ancestor member_name with
        | None ->
          all_interfaces_or_first_class_docblock seen_interfaces ancestors
        | Some docblock ->
          (match Cls.kind ancestor with
          | Ast_defs.Cabstract
          | Ast_defs.Cnormal ->
            [(Cls.name ancestor, docblock)]
          | _ ->
            all_interfaces_or_first_class_docblock
              ((Cls.name ancestor, docblock) :: seen_interfaces)
              ancestors)
      end
  in
  get_all_ancestors class_name
  |> all_interfaces_or_first_class_docblock []
  |> render_ancestor_docblocks

(* Attempt to clean obvious cruft from a docblock *)
let clean_comments (raw_comments : string) : string = String.strip raw_comments

(* This is a faster version of "go_comments_for_symbol" because it already knows
   the position *)
let go_comments_for_position
    ~(line : int)
    ~(column : int)
    ~(path : Relative_path.t)
    ~(filename : string)
    ~(contents : string)
    ~(kind : SymbolDefinition.kind) : string option =
  let source_text = Full_fidelity_source_text.make path contents in
  let lp =
    {
      Lexing.pos_fname = filename;
      pos_lnum = line;
      pos_cnum = column;
      pos_bol = 0;
    }
  in
  let pos = Pos.make_from_lexing_pos filename lp lp in
  let ffps_opt =
    ServerSymbolDefinition.get_definition_cst_node_from_pos
      kind
      source_text
      pos
  in
  match ffps_opt with
  | None -> None
  | Some ffps ->
    (match Docblock_finder.get_docblock ffps with
    | Some db -> Some (clean_comments db)
    | None -> None)

(* Fetch a definition *)
let go_comments_for_symbol
    ~(def : 'a SymbolDefinition.t)
    ~(base_class_name : string option)
    ~(file : ServerCommandTypes.file_input) : string option =
  match def.SymbolDefinition.docblock with
  | Some db -> Some (clean_comments db)
  | None ->
    let ffps_opt =
      ServerSymbolDefinition.get_definition_cst_node_from_file_input file def
    in
    (match ffps_opt with
    | None -> None
    | Some ffps ->
      (match Docblock_finder.get_docblock ffps with
      | Some db -> Some (clean_comments db)
      | None ->
        (match (def.SymbolDefinition.kind, base_class_name) with
        | (SymbolDefinition.Method, Some base_class_name) ->
          fallback base_class_name def.SymbolDefinition.name
        | _ -> None)))

(* Locate a symbol and return file, line, column, and base_class *)
let go_locate_symbol
    ~(env : ServerEnv.env) ~(symbol : string) ~(kind : SearchUtils.si_kind) :
    DocblockService.dbs_symbol_location_result =
  (* We may need env in the future, so avoid the warning *)
  let _ = env in
  (* Look up this class name *)
  match SymbolIndex.get_position_for_symbol symbol kind with
  | None -> None
  | Some (path, line, column) ->
    let filename = Relative_path.to_absolute path in
    (* Determine base class properly *)
    let base_class_name =
      match kind with
      | SearchUtils.SI_Class
      | SearchUtils.SI_Enum
      | SearchUtils.SI_Function
      | SearchUtils.SI_GlobalConstant
      | SearchUtils.SI_Interface
      | SearchUtils.SI_Trait
      | SearchUtils.SI_Typedef ->
        Some (Utils.add_ns symbol)
      | _ -> None
    in
    (* Here are the results *)
    Some
      {
        DocblockService.dbs_filename = filename;
        dbs_line = line;
        dbs_column = column;
        dbs_base_class = base_class_name;
      }

let symboldefinition_kind_from_si_kind (kind : SearchUtils.si_kind) :
    SymbolDefinition.kind =
  match kind with
  | SearchUtils.SI_Class -> SymbolDefinition.Class
  | SearchUtils.SI_Interface -> SymbolDefinition.Interface
  | SearchUtils.SI_Enum -> SymbolDefinition.Enum
  | SearchUtils.SI_Trait -> SymbolDefinition.Trait
  | SearchUtils.SI_Unknown -> SymbolDefinition.Class
  | SearchUtils.SI_Mixed -> SymbolDefinition.LocalVar
  | SearchUtils.SI_Function -> SymbolDefinition.Function
  | SearchUtils.SI_Typedef -> SymbolDefinition.Typedef
  | SearchUtils.SI_RecordDef -> SymbolDefinition.RecordDef
  | SearchUtils.SI_GlobalConstant -> SymbolDefinition.Const
  | SearchUtils.SI_XHP -> SymbolDefinition.Class
  | SearchUtils.SI_ClassMethod -> SymbolDefinition.Method
  | SearchUtils.SI_Literal -> SymbolDefinition.LocalVar
  | SearchUtils.SI_ClassConstant -> SymbolDefinition.Const
  | SearchUtils.SI_Property -> SymbolDefinition.Property
  | SearchUtils.SI_LocalVariable -> SymbolDefinition.LocalVar
  | SearchUtils.SI_Constructor -> SymbolDefinition.Method
  | SearchUtils.SI_Keyword -> failwith "Cannot look up a keyword"
  | SearchUtils.SI_Namespace -> failwith "Cannot look up a namespace"

(* Given a location and file contents, find best doc block *)
let go_docblock_at_contents
    ~(filename : string)
    ~(contents : string)
    ~(line : int)
    ~(column : int)
    ~(kind : SearchUtils.si_kind) : DocblockService.result =
  let def_kind = symboldefinition_kind_from_si_kind kind in
  let path = Relative_path.create_detect_prefix filename in
  match
    go_comments_for_position
      ~line
      ~column
      ~path
      ~filename
      ~contents
      ~kind:def_kind
  with
  | None -> []
  | Some "" -> []
  | Some comments -> [DocblockService.Markdown comments]

(* Given a location and filename, find best doc block *)
let go_docblock_at
    ~(filename : string)
    ~(line : int)
    ~(column : int)
    ~(kind : SearchUtils.si_kind) : DocblockService.result =
  (* Convert relative path and kind *)
  let path = Relative_path.create_detect_prefix filename in
  (* Okay, now that we know its position, let's gather a docblock *)
  let contents_opt = File_provider.get_contents path in
  match contents_opt with
  | None -> []
  | Some contents ->
    go_docblock_at_contents ~filename ~contents ~line ~column ~kind

(* Locate a symbol and return its docblock, no extra steps *)
let go_docblock_for_symbol
    ~(env : ServerEnv.env) ~(symbol : string) ~(kind : SearchUtils.si_kind) :
    DocblockService.result =
  (* Shortcut for namespaces, since they don't have locations *)
  if kind = SearchUtils.SI_Namespace then
    let namespace_declaration = Printf.sprintf "namespace %s;" symbol in
    [DocblockService.HackSnippet namespace_declaration]
  else if kind = SearchUtils.SI_Keyword then
    let txt = Printf.sprintf "Hack language keyword: %s;" symbol in
    [DocblockService.HackSnippet txt]
  else
    match go_locate_symbol ~env ~symbol ~kind with
    | None ->
      let msg =
        Printf.sprintf
          "The symbol %s (%s) has been added recently. To use this symbol, please rebase."
          symbol
          (SearchUtils.show_si_kind kind)
      in
      [DocblockService.Markdown msg]
    | Some location ->
      DocblockService.(
        go_docblock_at
          ~filename:location.dbs_filename
          ~line:location.dbs_line
          ~column:location.dbs_column
          ~kind)
