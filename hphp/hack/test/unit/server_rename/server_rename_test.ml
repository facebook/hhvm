(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let generate source =
  let open Full_fidelity_positioned_syntax in
  let module Syntax_tree =
    Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax) in
  let path = Relative_path.create Relative_path.Dummy "test.php" in
  let source_text = Full_fidelity_source_text.make path source in
  let tree = Syntax_tree.make source_text in
  let rec find_declaration ~class_name node =
    match syntax node with
    | ClassishDeclaration { classish_name; classish_keyword; _ } ->
      if String.equal (text classish_keyword) "interface" then
        failwith "interface eligibility requires a populated naming table";
      List.find_map
        (children node)
        ~f:(find_declaration ~class_name:(Some (text classish_name)))
    | FunctionDeclaration { function_declaration_header = header; _ } ->
      Some (node, header, Symbol_definition.Function)
    | MethodishDeclaration { methodish_function_decl_header = header; _ } ->
      Some
        ( node,
          header,
          Symbol_definition.Member
            {
              member_kind = Symbol_definition.Method;
              class_name = Option.value_exn class_name;
            } )
    | _ -> List.find_map (children node) ~f:(find_declaration ~class_name)
  in
  let (declaration, header, kind) =
    find_declaration ~class_name:None (Syntax_tree.root tree)
    |> Option.value_exn
  in
  let name =
    match syntax header with
    | FunctionDeclarationHeader { function_name; _ } -> function_name
    | _ -> failwith "expected function header"
  in
  let position node =
    Full_fidelity_source_text.relative_pos
      path
      source_text
      (start_offset node)
      (start_offset node + width node)
  in
  let definition =
    Symbol_definition.
      {
        kind;
        name = text name;
        pos = position name;
        span = position declaration;
        modifiers = [];
        params = None;
        docblock = None;
        detail = None;
      }
  in
  Provider_backend.set_local_memory_backend_with_defaults_for_test ();
  List.iter
    [Relative_path.Root; Relative_path.Hhi; Relative_path.Tmp]
    ~f:(fun prefix ->
      Relative_path.set_path_prefix prefix (Path.make Sys_utils.temp_dir_name));
  let ctx =
    Provider_context.empty_for_tool
      ~popt:Parser_options.default
      ~tcopt:Global_options.default
      ~backend:(Provider_backend.get ())
      ~deps_mode:(Typing_deps_mode.InMemoryMode None)
  in
  let (ctx, entry) =
    Provider_context.add_or_overwrite_entry_contents ~ctx ~path ~contents:source
  in
  (* Lookup failures must not masquerade as intentional wrapper suppression. *)
  let found =
    Server_symbol_definition.get_definition_cst_node_ctx
      ~ctx
      ~entry
      ~kind
      ~pos:definition.Symbol_definition.pos
    |> Option.value_exn ~message:"CST definition lookup failed"
  in
  if not (Int.equal (start_offset found) (start_offset declaration)) then
    failwith "CST lookup selected the wrong declaration";
  let insertion_offset = leading_start_offset declaration in
  let wrapper =
    match
      Server_rename.For_test.deprecated_wrapper_patch
        ~definition:(Some definition)
        ~ctx
        "renamed"
    with
    | None -> ""
    | Some (Server_rename_types.Insert { pos; text }) ->
      let (start, _) = Pos.info_raw pos in
      if not (Int.equal start insertion_offset) then
        failwith "wrapper insertion does not precede the declaration";
      (* verify.py ignores leading whitespace, so check indentation here. *)
      let offset = start_offset declaration in
      let line_start =
        String.rindex (String.prefix source offset) '\n'
        |> Option.value ~default:(-1)
        |> ( + ) 1
      in
      let column = offset - line_start in
      let lines =
        String.split_lines text
        |> List.filter ~f:(fun line ->
               not (String.is_empty (String.strip line)))
      in
      List.iteri lines ~f:(fun index line ->
          let indentation =
            String.length line - String.length (String.lstrip line)
          in
          let expected =
            column
            +
            if Int.equal index 2 then
              2
            else
              0
          in
          if not (Int.equal indentation expected) then
            failwith "wrapper indentation differs from the declaration");
      text
    | Some _ -> failwith "expected a wrapper insertion"
  in
  let name_offset = start_offset name in
  String.prefix source insertion_offset
  ^ wrapper
  ^ String.sub source ~pos:insertion_offset ~len:(name_offset - insertion_offset)
  ^ "renamed"
  ^ String.drop_prefix source (name_offset + width name)

let () =
  let filename = Sys.argv.(Array.length Sys.argv - 1) in
  In_channel.read_all filename |> generate |> print_string
