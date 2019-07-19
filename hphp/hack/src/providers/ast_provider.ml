(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

(*****************************************************************************)
(* Table containing all the Abstract Syntax Trees (cf ast.ml) for each file.*)
(*****************************************************************************)

(* We store only the names and declarations in the ParserHeap.
   The full flag in each function runs a full parsing with method bodies. *)

type parse_type = Decl | Full

module ParserHeap = SharedMem.WithCache (SharedMem.ProfiledImmediate) (Relative_path.S) (struct
    type t = Ast.program * parse_type
    let prefix = Prefix.make()
    let description = "Parser"
  end)

module LocalParserCache = SharedMem.LocalCache (Relative_path.S) (struct
    type t = Ast.program
    let prefix = Prefix.make()
    let description = "ParserLocal"
  end)

let parse_file_input
    ?(full = false)
    (file_name: Relative_path.t)
    (file_input: ServerCommandTypes.file_input)
    : Full_fidelity_ast.result =
  let popt = GlobalParserOptions.get () in
  let parser_env = Full_fidelity_ast.make_env
    ~quick_mode:(not full)
    ~keep_errors:false
    ~parser_options:popt
    file_name
  in
  let result =
    let source = ServerCommandTypesUtils.source_tree_of_file_input file_input in
    Full_fidelity_ast.from_text
      parser_env
      source
  in
  let ast =
    if (Relative_path.prefix file_name = Relative_path.Hhi)
    && ParserOptions.deregister_php_stdlib popt
    then Ast_utils.deregister_ignored_attributes result.Full_fidelity_ast.ast
    else result.Full_fidelity_ast.ast
  in
  { result with Full_fidelity_ast.ast }


let get_from_local_cache ~full file_name =
  let fn = Relative_path.to_absolute file_name in
  match LocalParserCache.get file_name with
  | Some ast -> ast
  | None ->
    let popt = GlobalParserOptions.get () in
    let f contents =
      let contents = if (FindUtils.file_filter fn) then contents else "" in
      match Ide_parser_cache.get_ast_if_active popt file_name contents with
      | Some ast -> ast.Parser_return.ast
      | None ->
        let source = Full_fidelity_source_text.make file_name contents in
        match Full_fidelity_parser.parse_mode ~rust:(ParserOptions.rust popt) source with
        | None
        | Some FileInfo.Mphp -> []
        | Some _ ->
          (Full_fidelity_ast.defensive_program
            ~quick:(not full)
            popt
            file_name
            contents
          ).Parser_return.ast
    in
    let ast = Option.value_map
      ~default:[]
      ~f
      (File_provider.get_contents file_name) in
    let ast =
      if (Relative_path.prefix file_name = Relative_path.Hhi)
      && ParserOptions.deregister_php_stdlib popt
      then Ast_utils.deregister_ignored_attributes ast
      else ast
    in
    let () = if full then LocalParserCache.add file_name ast in
    ast

let get_class ?(case_insensitive = false) defs class_name =
  let class_name =
    if case_insensitive then Caml.String.lowercase_ascii class_name else class_name
  in
  let rec get acc defs =
  List.fold_left defs ~init:acc ~f:begin fun acc def ->
    match def with
    | Ast.Class c ->
      let def_name = snd c.Ast.c_name in
      let def_name = if case_insensitive then Caml.String.lowercase_ascii def_name else def_name in
      if def_name = class_name then Some c else acc
    | Ast.Namespace(_, defs) -> get acc defs
    | _ -> acc
  end in
  get None defs

let get_fun ?(case_insensitive = false) defs fun_name =
  let fun_name = if case_insensitive then Caml.String.lowercase_ascii fun_name else fun_name in
  let rec get acc defs =
  List.fold_left defs ~init:acc ~f:begin fun acc def ->
    match def with
    | Ast.Fun f ->
      let def_name = snd f.Ast.f_name in
      let def_name = if case_insensitive then Caml.String.lowercase_ascii def_name else def_name in
      if def_name = fun_name then Some f else acc
    | Ast.Namespace(_, defs) -> get acc defs
    | _ -> acc
  end in
  get None defs

let get_typedef ?(case_insensitive = false) defs name =
  let name = if case_insensitive then Caml.String.lowercase_ascii name else name in
  let rec get acc defs =
  List.fold_left defs ~init:acc ~f:begin fun acc def ->
    match def with
    | Ast.Typedef typedef ->
      let def_name = snd typedef.Ast.t_id in
      let def_name = if case_insensitive then Caml.String.lowercase_ascii def_name else def_name in
      if def_name = name then Some typedef else acc
    | Ast.Namespace(_, defs) -> get acc defs
    | _ -> acc
  end in
  get None defs

let get_gconst defs name =
  let rec get acc defs =
  List.fold_left defs ~init:acc ~f:begin fun acc def ->
    match def with
    | Ast.Constant cst when snd cst.Ast.cst_name = name -> Some cst
    | Ast.Namespace(_, defs) -> get acc defs
    | _ -> acc
  end in
  get None defs

(* Get an AST directly from the parser heap. Will return empty AProgram
   if the file does not exist
*)
let get_ast ?(full = false) file_name =
  match Provider_config.get_backend () with
  | Provider_config.Lru_shared_memory
  | Provider_config.Shared_memory ->
    begin match ParserHeap.get file_name with
      | None ->
        let ast = get_from_local_cache ~full file_name in
        (* Only store decl asts *)
        if not full then
        ParserHeap.add file_name (ast, Decl);
        ast
      | Some (_, Decl) when full ->
        let ast = get_from_local_cache ~full file_name in
        ast
      | Some (defs, _) -> defs
    end
  | Provider_config.Local_memory _ ->
    let result =
      parse_file_input ~full file_name
        (ServerCommandTypes.FileName (Relative_path.to_absolute file_name))
    in
    result.Full_fidelity_ast.ast

let get_nast ?(full = false) file_name =
  let ast = get_ast ~full file_name in
  Ast_to_nast.convert ast

let find_class_in_file ?(full = false) ?(case_insensitive=false) file_name class_name =
  get_class (get_ast ~full file_name) ~case_insensitive class_name

let find_fun_in_file ?(full = false) ?(case_insensitive=false) file_name fun_name =
  get_fun (get_ast ~full file_name) ~case_insensitive fun_name

let find_typedef_in_file ?(full = false) ?(case_insensitive=false) file_name name =
  get_typedef (get_ast ~full file_name) ~case_insensitive name

let find_gconst_in_file ?(full = false) file_name name =
  get_gconst (get_ast ~full file_name) name

let find_class_in_file_nast ?(full = false) ?(case_insensitive=false) file_name class_name =
  let cls = find_class_in_file ~full ~case_insensitive file_name class_name in
  Option.map cls Ast_to_nast.on_class

let find_fun_in_file_nast ?(full = false) ?(case_insensitive=false) file_name fun_name =
  let f = find_fun_in_file ~full ~case_insensitive file_name fun_name in
  Option.map f Ast_to_nast.on_fun

let find_typedef_in_file_nast ?(full = false) ?(case_insensitive=false) file_name name =
  let t = find_typedef_in_file ~full ~case_insensitive file_name name in
  Option.map t Ast_to_nast.on_typedef

let find_gconst_in_file_nast ?(full = false) file_name name =
  let cst = find_gconst_in_file ~full file_name name in
  Option.map cst Ast_to_nast.on_constant

let local_changes_push_stack () =
  ParserHeap.LocalChanges.push_stack ()

let local_changes_pop_stack () =
  ParserHeap.LocalChanges.pop_stack ()

let local_changes_commit_batch paths =
  ParserHeap.LocalChanges.commit_batch paths

let local_changes_revert_batch paths =
  ParserHeap.LocalChanges.revert_batch paths

let provide_ast_hint
    (path: Relative_path.t)
    (program: Ast.program)
    (parse_type: parse_type)
    : unit =
  ParserHeap.write_around path (program, parse_type)

let remove_batch paths =
  ParserHeap.remove_batch paths

let has_for_test (path: Relative_path.t) : bool =
  ParserHeap.mem path
