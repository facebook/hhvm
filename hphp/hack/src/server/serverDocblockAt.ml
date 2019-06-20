(**
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
      begin match Decl_provider.get_class class_name with
      | None ->
        helper classes cinfos seen_classes
      | Some class_info ->
        let ancestors =
          Cls.all_ancestor_names class_info
          |> Sequence.fold ~init:classes ~f:(fun acc cid -> cid :: acc) in
        helper
          ancestors
          (class_info :: cinfos)
          (SSet.add class_name seen_classes)
      end
  in
  helper [class_name] [] SSet.empty

let get_docblock_for_member class_info member_name =
  let open Option.Monad_infix in
  Cls.get_method class_info member_name
  >>= fun member ->
  match Lazy.force member.Typing_defs.ce_type with
  | _, Typing_defs.Tfun ft ->
    let pos = ft.Typing_defs.ft_pos in
    let filename = Pos.filename pos in
    File_provider.get_contents filename
    >>= begin fun contents ->
      ServerSymbolDefinition.get_definition_cst_node_from_pos
        SymbolDefinition.Method
        (Full_fidelity_source_text.make filename contents)
        pos
      >>= Docblock_finder.get_docblock
    end
  | _ -> None

let render_ancestor_docblocks docblocks =
  let docblocks_to_ancestor =
    docblocks
    |> List.fold ~init:SMap.empty ~f:begin fun acc (class_name, docblock) ->
      let existing_ancestors = match SMap.find_opt docblock acc with
        | None -> []
        | Some lst -> lst
      in
      SMap.add docblock (class_name :: existing_ancestors) acc
    end
  in
  match SMap.elements docblocks_to_ancestor with
  | [] -> None
  | [docblock, _] -> Some docblock
  | docblock_ancestors_pairs ->
    docblock_ancestors_pairs
    |> List.map ~f:begin fun (docblock, ancestors) ->
      let ancestors_str =
        String.concat ~sep:", " (List.map ~f:Utils.strip_ns ancestors)
      in
      Printf.sprintf "%s\n(from %s)" docblock ancestors_str
    end
    |> String.concat ~sep:"\n\n---\n\n"
    |> fun results -> Some results

let fallback class_name member_name =
  let rec all_interfaces_or_first_class_docblock seen_interfaces ancestors_to_check =
    match ancestors_to_check with
    | [] -> seen_interfaces
    | ancestor :: ancestors ->
      begin match get_docblock_for_member ancestor member_name with
      | None ->
        all_interfaces_or_first_class_docblock seen_interfaces ancestors
      | Some docblock ->
        match Cls.kind ancestor with
        | Ast.Cabstract
        | Ast.Cnormal ->
          [(Cls.name ancestor, docblock)]
        | _ ->
          all_interfaces_or_first_class_docblock
            ((Cls.name ancestor, docblock) :: seen_interfaces)
            ancestors
      end
  in
  get_all_ancestors class_name
  |> all_interfaces_or_first_class_docblock []
  |> render_ancestor_docblocks

let go_def
    ~(def: Relative_path.t SymbolDefinition.t)
    ~(base_class_name: string option)
    ~(file: ServerCommandTypes.file_input)
    ~(basic_only: bool) =
  let open Option.Monad_infix in
  (** Read as "or-else." *)
  let (>>~) opt f = if Option.is_some opt then opt else f () in
  def.SymbolDefinition.docblock
  >>~ fun () ->
  ServerSymbolDefinition.get_definition_cst_node file def
  >>= Docblock_finder.get_docblock
  >>~ fun () ->
  match def.SymbolDefinition.kind, base_class_name with
  | SymbolDefinition.Method, Some base_class_name when not basic_only ->
    fallback base_class_name def.SymbolDefinition.name
  | _ -> None

(* Locate a symbol and return file, line, column, and base_class *)
let go_locate_symbol
    ~(env: ServerEnv.env)
    ~(symbol: string)
    ~(kind: SearchUtils.si_kind): DocblockService.dbs_symbol_location_result =

  (* We may need env in the future, so avoid the warning *)
  let _ = env in

  (* Look up this class name *)
  match SymbolIndex.get_position_for_symbol symbol kind with
  | None -> None
  | Some (relpath, line, column) ->
    let filename = Relative_path.to_absolute relpath in

    (* Determine base class properly *)
    let base_class_name = if kind = SearchUtils.SI_Class then begin
      let ns_name = Utils.add_ns symbol in
      match Decl_provider.get_class ns_name with
      | None -> None
      | Some class_info -> Some (Cls.name class_info)

    (* Anything other than a class doesn't have an ancestor *)
    end else None
    in

    (* Here are the results *)
    Some { DocblockService.
      dbs_filename = filename;
      dbs_line = line;
      dbs_column = column;
      dbs_base_class = base_class_name;
    }

(* Given a location, find best doc block *)
let go_docblock_at
  ~(env: ServerEnv.env)
  ~(filename: string)
  ~(line: int)
  ~(column: int)
  ~(base_class_name: string option)
  ~(basic_only: bool): DocblockService.result =
  let relpath = Relative_path.create_detect_prefix filename in

  (* Okay, now that we know its position, let's gather a docblock *)
  let ServerEnv.{ tcopt; _ } = env in
  let contents_opt = File_provider.get_contents relpath in
  match contents_opt with
  | None -> None
  | Some contents ->
    let definitions =
      ServerIdentifyFunction.go contents line column tcopt
      |> List.filter_map ~f:(fun (_, def) -> def)
    in
    match List.hd definitions with
    | None -> None
    | Some def ->
      let result = go_def
        ~base_class_name
        ~file:(ServerCommandTypes.FileName filename)
        ~basic_only
        ~def in
      result
