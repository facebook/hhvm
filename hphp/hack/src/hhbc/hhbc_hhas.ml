(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

module B = Buffer
module H = Hhbc_ast
open H

let buffer_of_instruct_basic prefix instruction =
  let result = B.create 0 in
  B.add_string result (
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

let buffer_of_instruct_lit_const prefix instruction =
  let result = B.create 0 in
  B.add_string result (
    prefix ^
    match instruction with
    | Null        -> "Null"
    | Int i       -> "Int " ^ Int64.to_string i
    (**
     * TODO (hgo): build that map from id to strings
     *)
    | String str    -> "String \"" ^ str ^ "\""
    | _ -> failwith "Not Implemented"
  ); result

let buffer_of_instruct_operator prefix instruction =
  let result = B.create 0 in
  B.add_string result (
    prefix ^
    match instruction with
    | Concat -> "Concat"
    | Abs -> "Abs"
    | Add -> "Add"
    | Sub -> "Sub"
    | Mul -> "Mul"
    | AddO -> "AddO"
    | SubO -> "SubO"
    | MulO -> "MulO"
    | Div -> "Div"
    | Mod -> "Mod"
    | Pow -> "Pow"
    | Sqrt -> "Sqrt"
    | Xor -> "Xor"
    | Not -> "Not"
    | Same -> "Same"
    | NSame -> "NSame"
    | Eq -> "Eq"
    | Neq -> "Neq"
    | Lt -> "Lt"
    | Lte -> "Lte"
    | Gt -> "Gt"
    | Gte -> "Gte"
    | Cmp -> "Cmp"
    | BitAnd -> "BitAnd"
    | BitOr -> "BitOr"
    | BitXor -> "BitXor"
    | BitNot -> "BitNot"
    | Shl -> "Shl"
    | Shr -> "Shr"
    | Floor -> "Floor"
    | Ceil -> "Ceil"
    | CastBool -> "CastBool"
    | CastInt -> "CastInt"
    | CastDouble -> "CastDouble"
    | CastString -> "CastString"
    | CastArray -> "Cast"
    | CastObject -> "CastObject"
    | CastVec -> "CastVec"
    | CastDict -> "CastDict"
    | CastKeyset -> "CastKeyset"
    | InstanceOf -> "InstanceOf"
    | InstanceOfD -> "InstanceOfD"
    | Print -> "Print"
    | Clone -> "Clone"
    | H.Exit -> "Exit"
    | Fatal -> "Fatal"
  ); result

let buffer_of_instruct_control_flow prefix instruction =
  let result = B.create 0 in
  B.add_string result (
    prefix ^
    match instruction with
    | RetC -> "RetC"
    | RetV -> "RetV"
    | _ -> failwith "Not Implemented"
  ); result

let buffer_of_instruct_call prefix instruction =
  let result = B.create 0 in
  B.add_string result (
    prefix ^
    match instruction with
    | FPushFuncD (n_params, litstr) ->
      "FPushFuncD "
      ^ string_of_int n_params
      ^ " \"" ^ litstr ^ "\""
    | FCall param_id -> "FCall " ^ string_of_int param_id
    | _ -> failwith "instruct_call Not Implemented"
  ); result

let buffer_of_instruction_list instructs =
  let lpad = B.create 2 in
  let prefix = "  " in
  let f_fold acc inst =
    B.add_buffer acc (
      match inst with
      | IBasic basic -> buffer_of_instruct_basic prefix basic
      | ILitConst lit_const ->
        buffer_of_instruct_lit_const prefix lit_const
      | IOp op -> buffer_of_instruct_operator prefix op
      | IContFlow cont_flow -> buffer_of_instruct_control_flow prefix cont_flow
      | ICall f_call -> buffer_of_instruct_call prefix f_call
    );
    B.add_string acc "\n";
    acc in
  List.fold_left f_fold lpad instructs

let buffer_of_return_types return_types =
  let buf = B.create 0 in
  B.add_string buf "<";
  (match return_types with
  | [] -> B.add_string buf "\"HH\\void\" N  "
  | _  -> B.add_string buf "we only support functions returning void for the moment");
  B.add_string buf "> ";
  buf

let buffer_of_fun_def fun_def =
  let buf = B.create 0 in
  B.add_string buf "\n.function ";
  B.add_buffer buf @@ buffer_of_return_types fun_def.f_return_types;
  B.add_string buf fun_def.f_name;
  B.add_string buf "()";
  B.add_string buf " {\n";
  B.add_buffer buf
    (buffer_of_instruction_list fun_def.f_body);
  B.add_string buf "}\n";
  buf

let buffer_of_class_def class_def =
  let buf = B.create 0 in
  (* TODO: final *)
  (* TODO: abstract *)
  (* TODO: attributes *)
  B.add_string buf "\n.class ";
  B.add_string buf class_def.class_name;
  B.add_string buf " {\n";
  (* TODO: members *)
  (* TODO: If there is no ctor, generate one *)
  B.add_string buf "}\n";
  buf

let buffer_of_hhas_prog prog =
  let buf = B.create 0 in
  List.iter
    (fun x -> B.add_buffer buf (buffer_of_fun_def x);) prog.hhas_fun;
  List.iter
    (fun x -> B.add_buffer buf (buffer_of_class_def x);) prog.hhas_classes;
  buf

let buffer_of_defcls classes =
  let buf = B.create 0 in
  let rec aux c count =
    match c with
    | [] -> ()
    | _ :: t ->
      begin
        B.add_string buf (Printf.sprintf "  DefCls %n\n" count);
        aux t (count + 1)
      end in
  aux classes 0;
  buf

let buffer_of_top_level hhas_prog =
  let main_stmts =
    [ ILitConst (Int Int64.one)
    ; IContFlow RetC
    ] in
  let fun_name = ".main {\n" in
  let buf = B.create 0 in
  B.add_string buf fun_name;
  B.add_buffer buf (buffer_of_defcls hhas_prog.hhas_classes);
  B.add_buffer buf (buffer_of_instruction_list main_stmts);
  B.add_string buf "}\n";
  buf

let to_string hhas_prog =
  let final_buf = buffer_of_top_level hhas_prog in
  B.add_buffer final_buf @@ buffer_of_hhas_prog hhas_prog;
  B.add_string final_buf "\n";
  B.contents final_buf
