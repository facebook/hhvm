(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Utils

(*****************************************************************************)
(* Dependencies *)
(*****************************************************************************)

(* Module adding the dependencies related to inheritance.
 * We need a complete accurate graph of dependencies related to inheritance.
 * Because without them, we can't recompute the set of files that must be
 * rechecked when something changes.
 * It is safer for us to add them as soon as possible, that's why we add
 * them just after parsing, because that's as soon as it gets.
 *)
module AddDeps = struct
  module Dep = Typing_deps.Dep
  open Ast

  let rec program defl = List.iter def defl

  and def = function
    | Class c -> class_ c
    | Fun _  | Stmt _  | Typedef _ | Constant _ -> ()
    | Namespace _ | NamespaceUse _ -> assert false

  and class_ c =
    let name = snd c.c_name in
    if SMap.mem "Injectable" c.c_user_attributes ||
       SMap.mem "InjectableSingleton" c.c_user_attributes
    then begin
      Typing_deps.add_idep (Some (Dep.Class name)) Dep.Injectable;
    end;
    List.iter (hint name) c.c_extends;
    List.iter (hint name) c.c_implements;
    List.iter (class_def name) c.c_body

  and class_def root = function
    | ClassUse h -> hint root h
    | ClassTraitRequire (_, h) -> hint root h
    | Attributes _  | Const _ | ClassVars _ | Method _-> ()

  and hint root (_, h) =
    match h with
    | Happly ((_, parent), _) ->
        Typing_deps.add_idep (Some (Dep.Class root)) (Dep.Extends parent)
    | Hoption _ | Hfun _ | Htuple _ | Hshape _ -> ()


end


(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

(* Using a C primitive that systematically reads from mmap makes things
 * much faster in incremental mode. I don't exatly know why ...
 * My guess is that when the filesystem is busy each system call costs
 * us more, mapping everything in a file is only one system call, so
 * that could explain it. But really, I don't know.
 *)
external hh_read_file: string -> string = "hh_read_file"

let neutral = (SMap.empty, [], SSet.empty, SSet.empty)

let empty_file_info : FileInfo.t = {
  FileInfo.funs = [];
  classes = [];
  types = [];
  consts = [];
  comments = [];
  consider_names_just_for_autoload = false;
}

(* Given a Ast.program, give me the list of entities it defines *)
let get_defs ast =
  List.fold_left begin fun (acc1, acc2, acc3, acc4) def ->
    match def with
    | Ast.Fun f -> f.Ast.f_name :: acc1, acc2, acc3, acc4
    | Ast.Class c -> acc1, c.Ast.c_name :: acc2, acc3, acc4
    | Ast.Typedef t -> acc1, acc2, t.Ast.t_id :: acc3, acc4
    | Ast.Constant cst -> acc1, acc2, acc3, cst.Ast.cst_name :: acc4
    | Ast.Namespace _
    | Ast.NamespaceUse _ -> assert false
     (* toplevel statements are ignored *)
    | Ast.Stmt _ -> acc1, acc2, acc3, acc4
  end ([], [], [], []) ast

let parse_file fn =
  let content = hh_read_file fn in
  if String.length content = 0
  then begin
    Parser_heap.ParserHeap.add fn [];
    empty_file_info
  end
  else begin
    Pos.file := fn;
    try
      Ast.mtime := (Unix.stat fn).Unix.st_mtime;
      Ast.mode := Ast.Mstrict;
      Parser_hack.is_hh_file := false;
      let comments, ast = Parser_hack.from_file_with_comments fn in
      let ast = Namespaces.elaborate_defs ast in
      AddDeps.program ast;
      let funs, classes, types, consts = get_defs ast in
      Parser_heap.ParserHeap.add fn ast;
      {FileInfo.funs; classes; types; consts; comments; 
      consider_names_just_for_autoload = false}
    with
    | e ->
        Parser_heap.ParserHeap.add fn [];
        raise e
  end

let legacy_php_file_info = ref (fun fn ->
  empty_file_info
)

(* Parsing a file without failing
 * acc is a file_info
 * errorl is a list of errors
 * error_files is SSet.t of files that we failed to parse
 *)
let parse (acc, errorl, error_files, php_files) fn =
  try
    let defs = parse_file fn in
    let acc = SMap.add fn defs acc in
    acc, errorl, error_files, php_files
  with
  | Utils.Error l ->
      let acc, php_files =
        if !(Parser_hack.is_hh_file)
        then SMap.add fn empty_file_info acc, php_files
        (* we also now keep in the file_info regular php files
         * as we need at least their names in hack build
         *)
        else 
          let info = !legacy_php_file_info fn in
          SMap.add fn info acc, SSet.add fn php_files 
      in
      if !(Parser_hack.is_hh_file)
      then acc, l :: errorl, SSet.add fn error_files, php_files
      else acc, errorl, error_files, php_files
  | exn ->
      if !(Parser_hack.is_hh_file)
      then
        let l = [Pos.make_from_file(), Printexc.to_string exn] in
        acc, l :: errorl, SSet.add fn error_files, php_files
      else acc, errorl, error_files, php_files

(* Merging the results when the operation is done in parallel *)
let merge_parse
    (acc1, status1, files1, pfiles1)
    (acc2, status2, files2, pfiles2) =
  SMap.fold SMap.add acc1 acc2, List.rev_append status1 status2,
  SSet.union files1 files2,
  SSet.union pfiles1 pfiles2

let parse_files acc fnl =
  List.fold_left parse acc fnl

let parse_parallel workers get_next =
  MultiWorker.call
      workers
      ~job:parse_files
      ~neutral:neutral
      ~merge:merge_parse
      ~next:get_next

(*****************************************************************************)
(* Main entry points *)
(*****************************************************************************)

let go workers files ~get_next =
  let fast, errorl, failed_parsing, php_files =
    parse_parallel workers get_next in
  let fast = SSet.fold begin fun fn acc ->
    if SMap.mem fn files
    then SMap.add fn empty_file_info acc
    else acc
  end php_files fast in
  fast, errorl, failed_parsing
