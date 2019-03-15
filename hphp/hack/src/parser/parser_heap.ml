(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module Lowerer = Full_fidelity_ast

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

let parse_failure_scuba_table = Scuba.Table.of_name "hh_parse_failure"

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
        match Full_fidelity_parser.parse_mode source with
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
    let ast = Option.value_map ~default:[] ~f (File_heap.get_contents file_name) in
    let ast =
      if (Relative_path.prefix file_name = Relative_path.Hhi)
      && ParserOptions.deregister_php_stdlib popt
      then Ast_utils.deregister_ignored_attributes ast
      else ast
    in
    let () = if full then LocalParserCache.add file_name ast in
    ast

let get_class defs class_name =
  let rec get acc defs =
  List.fold_left defs ~init:acc ~f:begin fun acc def ->
    match def with
    | Ast.Class c when snd c.Ast.c_name = class_name -> Some c
    | Ast.Namespace(_, defs) -> get acc defs
    | _ -> acc
  end in
  get None defs

let get_fun defs fun_name =
  let rec get acc defs =
  List.fold_left defs ~init:acc ~f:begin fun acc def ->
    match def with
    | Ast.Fun f when snd f.Ast.f_name = fun_name -> Some f
    | Ast.Namespace(_, defs) -> get acc defs
    | _ -> acc
  end in
  get None defs

let get_typedef defs name =
  let rec get acc defs =
  List.fold_left defs ~init:acc ~f:begin fun acc def ->
    match def with
    | Ast.Typedef typedef when snd typedef.Ast.t_id = name -> Some typedef
    | Ast.Namespace(_, defs) -> get acc defs
    | _ -> acc
  end in
  get None defs

let get_const defs name =
  let rec get acc defs =
  List.fold_left defs ~init:acc ~f:begin fun acc def ->
    match def with
    | Ast.Constant cst when snd cst.Ast.cst_name = name -> Some cst
    | Ast.Namespace(_, defs) -> get acc defs
    | _ -> acc
  end in
  get None defs

(* Get top-level statements from definitions *)
let rec get_statements defs =
  List.concat @@ List.map defs begin fun def ->
    match def with
    | Ast.Stmt st -> [st]
    | Ast.Namespace(_, defs) -> get_statements defs
    | _ -> []
  end

(* Get an AST directly from the parser heap. Will return empty AProgram
   if the file does not exist
*)
let get_from_parser_heap ?(full = false) file_name =
  match ParserHeap.get file_name with
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

let find_class_in_file ?(full = false) file_name class_name =
  get_class (get_from_parser_heap ~full file_name) class_name

let find_fun_in_file ?(full = false) file_name fun_name =
  get_fun (get_from_parser_heap ~full file_name) fun_name

let find_typedef_in_file ?(full = false) file_name name =
  get_typedef (get_from_parser_heap ~full file_name) name

let find_const_in_file ?(full = false) file_name name =
  get_const (get_from_parser_heap ~full file_name) name
