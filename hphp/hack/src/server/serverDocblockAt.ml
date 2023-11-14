(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Cls = Decl_provider.Class

let get_all_ancestors ctx class_name =
  let rec helper classes_to_check cinfos seen_classes =
    match classes_to_check with
    | [] -> cinfos
    | class_name :: classes when SSet.mem class_name seen_classes ->
      helper classes cinfos seen_classes
    | class_name :: classes -> begin
      match Decl_provider.get_class ctx class_name with
      | None -> helper classes cinfos seen_classes
      | Some class_info ->
        let ancestors =
          Cls.all_ancestor_names class_info
          |> List.fold ~init:classes ~f:(fun acc cid -> cid :: acc)
        in
        helper
          ancestors
          (class_info :: cinfos)
          (SSet.add class_name seen_classes)
    end
  in
  helper [class_name] [] SSet.empty

let get_docblock_for_member ctx class_info member_name =
  let open Option.Monad_infix in
  Cls.get_method class_info member_name >>= fun member ->
  match Typing_defs.get_node @@ Lazy.force member.Typing_defs.ce_type with
  | Typing_defs.Tfun _ ->
    let pos =
      Lazy.force member.Typing_defs.ce_pos
      |> Naming_provider.resolve_position ctx
    in
    let path = Pos.filename pos in
    let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
    ServerSymbolDefinition.get_definition_cst_node_ctx
      ~ctx
      ~entry
      ~kind:SymbolDefinition.Method
      ~pos
    >>= Docblock_finder.get_docblock
  | _ -> None

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
    |> fun results -> Some results

let fallback ctx class_name member_name =
  let rec all_interfaces_or_first_class_docblock
      seen_interfaces ancestors_to_check =
    match ancestors_to_check with
    | [] -> seen_interfaces
    | ancestor :: ancestors -> begin
      match get_docblock_for_member ctx ancestor member_name with
      | None -> all_interfaces_or_first_class_docblock seen_interfaces ancestors
      | Some docblock ->
        (match Cls.kind ancestor with
        | Ast_defs.Cclass _ -> [(Cls.name ancestor, docblock)]
        | Ast_defs.(Ctrait | Cinterface | Cenum | Cenum_class _) ->
          all_interfaces_or_first_class_docblock
            ((Cls.name ancestor, docblock) :: seen_interfaces)
            ancestors)
    end
  in
  get_all_ancestors ctx class_name
  |> all_interfaces_or_first_class_docblock []
  |> render_ancestor_docblocks

let re_generated =
  Str.regexp "^This file is generated\\.\\(.*\n\\)*^\u{40}generated.*$"

let re_partially_generated =
  Str.regexp
    "^This file is partially generated\\.\\(.*\n\\)*^\u{40}partially-generated.*$"

let re_copyright1 = Str.regexp "^(c) Facebook.*$"

let re_copyright2 = Str.regexp "^Copyright [0-9][0-9][0-9][0-9].*$"

let re_copyright3 = Str.regexp "^Copyright (c).*$"

let clean_comments (s : string) : string =
  s
  |> Str.global_replace re_generated ""
  |> Str.global_replace re_partially_generated ""
  |> Str.global_replace re_copyright1 ""
  |> Str.global_replace re_copyright2 ""
  |> Str.global_replace re_copyright3 ""
  |> String.strip

let go_comments_from_source_text
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(line : int)
    ~(column : int)
    ~(kind : SymbolDefinition.kind) : string option =
  let _ = ctx in
  let filename = Relative_path.to_absolute entry.Provider_context.path in
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
    ServerSymbolDefinition.get_definition_cst_node_ctx ~ctx ~entry ~kind ~pos
  in
  match ffps_opt with
  | None -> None
  | Some ffps ->
    (match Docblock_finder.get_docblock ffps with
    | Some db -> Some (clean_comments db)
    | None -> None)

(* Fetch a definition *)
let go_comments_for_symbol_ctx
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(def : 'a SymbolDefinition.t)
    ~(base_class_name : string option) : string option =
  match def.SymbolDefinition.docblock with
  | Some db -> Some (clean_comments db)
  | None ->
    (match
       ServerSymbolDefinition.get_definition_cst_node_ctx
         ~ctx
         ~entry
         ~kind:def.SymbolDefinition.kind
         ~pos:def.SymbolDefinition.pos
     with
    | None -> None
    | Some ffps ->
      (match Docblock_finder.get_docblock ffps with
      | Some db -> Some (clean_comments db)
      | None ->
        (match (def.SymbolDefinition.kind, base_class_name) with
        | (SymbolDefinition.Method, Some base_class_name) ->
          fallback ctx base_class_name def.SymbolDefinition.name
        | _ -> None)))

(* Locate a symbol and return file, line, column, and base_class *)
let go_locate_symbol
    ~(ctx : Provider_context.t) ~(symbol : string) ~(kind : FileInfo.si_kind) :
    DocblockService.dbs_symbol_location_result =
  (* Look up this class name *)
  match SymbolIndexCore.get_position_for_symbol ctx symbol kind with
  | None -> None
  | Some (path, line, column) ->
    let filename = Relative_path.to_absolute path in
    (* Determine base class properly *)
    let base_class_name =
      match kind with
      | FileInfo.SI_Class
      | FileInfo.SI_Enum
      | FileInfo.SI_Function
      | FileInfo.SI_GlobalConstant
      | FileInfo.SI_Interface
      | FileInfo.SI_Trait
      | FileInfo.SI_Typedef ->
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

let symboldefinition_kind_from_si_kind (kind : FileInfo.si_kind) :
    SymbolDefinition.kind =
  match kind with
  | FileInfo.SI_Class -> SymbolDefinition.Class
  | FileInfo.SI_Interface -> SymbolDefinition.Interface
  | FileInfo.SI_Enum -> SymbolDefinition.Enum
  | FileInfo.SI_Trait -> SymbolDefinition.Trait
  | FileInfo.SI_Unknown -> SymbolDefinition.Class
  | FileInfo.SI_Mixed -> SymbolDefinition.LocalVar
  | FileInfo.SI_Function -> SymbolDefinition.Function
  | FileInfo.SI_Typedef -> SymbolDefinition.Typedef
  | FileInfo.SI_GlobalConstant -> SymbolDefinition.GlobalConst
  | FileInfo.SI_XHP -> SymbolDefinition.Class
  | FileInfo.SI_ClassMethod -> SymbolDefinition.Method
  | FileInfo.SI_Literal -> SymbolDefinition.LocalVar
  | FileInfo.SI_ClassConstant -> SymbolDefinition.ClassConst
  | FileInfo.SI_Property -> SymbolDefinition.Property
  | FileInfo.SI_LocalVariable -> SymbolDefinition.LocalVar
  | FileInfo.SI_Constructor -> SymbolDefinition.Method
  | FileInfo.SI_Keyword -> failwith "Cannot look up a keyword"
  | FileInfo.SI_Namespace -> failwith "Cannot look up a namespace"

let rec go_docblock_ctx
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(line : int)
    ~(column : int)
    ~(kind : FileInfo.si_kind) : DocblockService.result =
  let def_kind = symboldefinition_kind_from_si_kind kind in
  match
    go_comments_from_source_text ~ctx ~entry ~line ~column ~kind:def_kind
  with
  | None ->
    (* Special case: Classes with an assumed default constructor *)
    if FileInfo.equal_si_kind kind FileInfo.SI_Constructor then
      go_docblock_ctx ~ctx ~entry ~line ~column ~kind:FileInfo.SI_Class
    else
      []
  | Some "" -> []
  | Some comments -> [DocblockService.Markdown comments]

(* Locate a symbol and return its docblock, no extra steps *)
let go_docblock_for_symbol
    ~(ctx : Provider_context.t) ~(symbol : string) ~(kind : FileInfo.si_kind) :
    DocblockService.result =
  (* Shortcut for namespaces, since they don't have locations *)
  if FileInfo.equal_si_kind kind FileInfo.SI_Namespace then
    let namespace_declaration = Printf.sprintf "namespace %s;" symbol in
    [DocblockService.HackSnippet namespace_declaration]
  else if FileInfo.equal_si_kind kind FileInfo.SI_Keyword then
    let txt = Printf.sprintf "Hack language keyword: %s;" symbol in
    [DocblockService.HackSnippet txt]
  else
    match go_locate_symbol ~ctx ~symbol ~kind with
    | None ->
      let msg =
        Printf.sprintf
          "Could not find the symbol '%s' (expected to be a %s). This symbol may need namespace information (e.g. HH\\a\\b\\c) to resolve correctly. You can also consider rebasing."
          symbol
          (FileInfo.show_si_kind kind)
      in
      [DocblockService.Markdown msg]
    | Some location ->
      DocblockService.(
        let (ctx, entry) =
          Provider_context.add_entry_if_missing
            ~ctx
            ~path:(Relative_path.create_detect_prefix location.dbs_filename)
        in
        go_docblock_ctx
          ~ctx
          ~entry
          ~line:location.dbs_line
          ~column:location.dbs_column
          ~kind)
