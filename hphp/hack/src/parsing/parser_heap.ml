(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core

(*****************************************************************************)
(* Table containing all the Abstract Syntax Trees (cf ast.ml) for each file.*)
(*****************************************************************************)

module ParserHeap = SharedMem.WithCache (Relative_path.S) (struct
    type t = Ast.program
    let prefix = Prefix.make()
    let description = "Parser"
  end)

let find_class_in_file file_name class_name =
  match ParserHeap.get file_name with
  | None -> None
  | Some defs ->
    List.fold_left defs ~init:None ~f:begin fun acc def ->
      match def with
      | Ast.Class c when snd c.Ast.c_name = class_name -> Some c
      | _ -> acc
    end

let find_fun_in_file file_name fun_name =
  match ParserHeap.get file_name with
  | None -> None
  | Some defs ->
    List.fold_left defs ~init:None ~f:begin fun acc def ->
      match def with
      | Ast.Fun f when snd f.Ast.f_name = fun_name -> Some f
      | _ -> acc
    end

let find_typedef_in_file file_name name =
  match ParserHeap.get file_name with
  | None -> None
  | Some defs ->
    List.fold_left defs ~init:None ~f:begin fun acc def ->
      match def with
      | Ast.Typedef typedef when snd typedef.Ast.t_id = name -> Some typedef
      | _ -> acc
    end

let find_const_in_file file_name name =
  match ParserHeap.get file_name with
  | None -> None
  | Some defs ->
    List.fold_left defs ~init:None ~f:begin fun acc def ->
      match def with
      | Ast.Constant cst when snd cst.Ast.cst_name = name -> Some cst
      | _ -> acc
    end
