(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Hhbc_ast

let buffer_of_instruct_basic prefix instruction =
  let result = Buffer.create 0 in
  Buffer.add_string result (
    prefix ^
    match instruction with
    | Nop         -> "Nop"
    | EntryNop    -> "EntryNop"
    | PopA        -> "PopA"
    | PopC        -> "PopC"
    | PopV        -> "PopV"
    | PopR        -> "PopR"
    | Dup         -> "Dup"
    | Box         -> "Box"
    | Unbox       -> "Unbox"
    | BoxR        -> "BoxR"
    | UnboxR      -> "UnboxR"
    | UnboxRNop   -> "UnboxRNop"
    | RGetCNop    -> "RGetCNop"
  ); result

let buffer_of_instruct_lit_const prefix litstr instruction =
  let result = Buffer.create 0 in
  Buffer.add_string result (
    prefix ^
    match instruction with
    | Null        -> "Null"
    | Int i       -> "Int " ^ Int64.to_string i
    (**
     * TODO (hgo): build that map from id to strings
     *)
    | String id    -> "String \"" ^ Hashtbl.find litstr id ^ "\""
    | _ -> failwith "Not Implemented"
  ); result

let buffer_of_instruct_operator prefix instruction =
  let result = Buffer.create 0 in
  Buffer.add_string result (
    prefix ^
    match instruction with
    | Print -> "Print"
    | _ -> failwith "Not Implemented"
  ); result

let buffer_of_instruct_control_flow prefix instruction =
  let result = Buffer.create 0 in
  Buffer.add_string result (
    prefix ^
    match instruction with
    | RetC -> "RetC"
    | _ -> failwith "Not Implemented"
  ); result

let buffer_of_instruct_call prefix litstr instruction =
  let result = Buffer.create 0 in
  Buffer.add_string result (
    prefix ^
    match instruction with
    | FPushFuncD (n_params, litstr_id) ->
      "FPushFuncD "
      ^ string_of_int n_params
      ^ " " ^ Hashtbl.find litstr litstr_id
    | FCall param_id -> "FCall " ^ string_of_int param_id
    | _ -> failwith "instruct_call Not Implemented"
  ); result

let buffer_of_instruction_list litstr instructs =
  let lpad = Buffer.create 2 in
  let prefix = "  " in
  let f_fold acc inst =
    Buffer.add_buffer acc (
      match inst with
      | IBasic basic -> buffer_of_instruct_basic prefix basic
      | ILitConst lit_const ->
        buffer_of_instruct_lit_const prefix litstr lit_const
      | IOp op -> buffer_of_instruct_operator prefix op
      | IContFlow cont_flow -> buffer_of_instruct_control_flow prefix cont_flow
      | ICall f_call -> buffer_of_instruct_call prefix litstr f_call
    );
    Buffer.add_string acc "\n";
    acc in
  List.fold_left f_fold lpad instructs

let buffer_of_fun_def litstr fun_def =
  let fun_name  =
    ".function " ^ Hashtbl.find litstr fun_def.fun_name in
  let buf = Buffer.create 0 in
  Buffer.add_string buf fun_name;
  Buffer.add_string buf " {\n";
  Buffer.add_buffer buf
    (buffer_of_instruction_list fun_def.fun_litstr fun_def.fun_body);
  Buffer.add_string buf "}\n";
  buf

let buffer_of_hhas_prog prog =
  let rec aux acc funs =
    match funs with
    | [] -> acc
    | f::fs -> begin
      Buffer.add_buffer acc (buffer_of_fun_def prog.hhas_litstr f);
      aux acc fs
    end in
  let buf = Buffer.create 0 in
  aux buf prog.hhas_fun
