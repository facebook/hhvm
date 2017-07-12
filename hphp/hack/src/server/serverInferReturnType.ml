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
open Result

type result = (string, string) Result.t

let add_ns name =
  if String_utils.string_starts_with name "\\" then name else "\\" ^ name

type t =
| Function of string
| Method of string * string

let match_hint (_, hint) =
  match hint with
  | Ast.Happly((_, id) , _) -> Ok id
  | _ -> Error "Unable to print type"

let get_fun_return_ty popt fun_name =
  let fun_name = add_ns fun_name in
  let pos =
    of_option (Naming_heap.FunPosHeap.get fun_name) "Could not find function"
  in
  let file = map pos FileInfo.get_pos_filename in
  let funopt =
    map file (fun file -> Parser_heap.find_fun_in_file popt file fun_name)
  in
  let f = join @@ map funopt (of_option ~error:"Could not find function") in
  let hint =
    join @@ map f (fun f -> of_option f.Ast.f_ret "Unknown return type")
  in
  bind hint match_hint

let get_meth_return_ty popt class_name meth_name =
  let class_name = add_ns class_name in
  let pos =
    match Naming_heap.TypeIdHeap.get class_name with
    | Some (pos, `Class) -> Ok pos
    | _ -> Error "Could not find class"
  in
  let file = map pos FileInfo.get_pos_filename in
  let classopt =
    map file (fun file -> Parser_heap.find_class_in_file popt file class_name)
  in
  let c = join @@ map classopt (of_option ~error:"Could not find class") in
  let mopt =
    map c
      (fun c -> List.find c.Ast.c_body
        begin function
          | Ast.Method m -> (snd m.Ast.m_name) = meth_name
          | _ -> false
        end)
  in
  let m =
    bind mopt
      (function
        | Some (Ast.Method m) -> Ok m
        | _ -> Error "Could not find method")
  in
  let hint =
    join @@ map m (fun m -> of_option m.Ast.m_ret "Unknown return type")
  in
  bind hint match_hint
