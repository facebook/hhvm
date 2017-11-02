(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core

(*****************************************************************************)
(* Table containing all the Abstract Syntax Trees (cf ast.ml) for each file.*)
(*****************************************************************************)

(* We store only the names and declarations in the ParserHeap.
   The full flag in each function runs a full parsing with method bodies. *)

type parse_type = Decl | Full

module ParserHeap = SharedMem.WithCache (Relative_path.S) (struct
    type t = Ast.program * parse_type
    let prefix = Prefix.make()
    let description = "Parser"
  end)

module LocalParserCache = SharedMem.LocalCache (Relative_path.S) (struct
    type t = Ast.program
    let prefix = Prefix.make()
    let description = "ParserLocal"
  end)

let get_from_local_cache ~full popt file_name =
  match LocalParserCache.get file_name with
  | Some ast -> ast
  | None ->
        let contents =
        match File_heap.get_contents file_name with
        | Some contents -> contents
        | None -> "" in
        let contents = let fn = (Relative_path.to_absolute file_name) in
          if (FindUtils.is_php fn
          && not (FilesToIgnore.should_ignore fn))
          && Parser_hack.get_file_mode popt file_name contents <> None then
          contents else "" in
        let { Parser_hack.ast;
          _ } = Parser_hack.program ~quick:(not full) popt file_name contents in
        let ast = Ast_utils.deregister_ignored_attributes ast in
        if full then LocalParserCache.add file_name ast;
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
let get_from_parser_heap ?(full = false) popt file_name =
  match ParserHeap.get file_name with
    | None ->
      let ast = get_from_local_cache ~full popt file_name in
      (* Only store decl asts *)
      if not full then
      ParserHeap.add file_name (ast, Decl);
      ast
    | Some (_, Decl) when full ->
      let ast = get_from_local_cache ~full popt file_name in
      ast
    | Some (defs, _) -> defs

let find_class_in_file ?(full = false) popt file_name class_name =
  get_class (get_from_parser_heap ~full popt file_name) class_name

let find_fun_in_file ?(full = false) popt file_name fun_name =
  get_fun (get_from_parser_heap ~full popt file_name) fun_name

let find_typedef_in_file ?(full = false) popt file_name name =
  get_typedef (get_from_parser_heap ~full popt file_name) name

let find_const_in_file ?(full = false) popt file_name name =
  get_const (get_from_parser_heap ~full popt file_name) name

let find_statements_in_file ?(full = false) popt file_name =
  get_statements (get_from_parser_heap ~full popt file_name)
