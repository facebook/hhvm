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

let two_spaces = "  "
let four_spaces = "    "

(* Naming convention for functions below:
 *   string_of_X converts an X to a string
 *   add_X takes a buffer and an X, and appends to the buffer
 *)
let string_of_basic instruction =
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

let string_of_lit_const instruction =
  match instruction with
    | Null        -> "Null"
    | Int i       -> "Int " ^ Int64.to_string i
    | String str  -> "String \"" ^ str ^ "\""
    | True        -> "True"
    | False       -> "False"
    | _ -> failwith "Not Implemented"

let string_of_operator instruction =
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

let string_of_local_id x =
  match x with
  | Local_unnamed i -> string_of_int i
  | Local_named s -> s

let string_of_get x =
  match x with
  | CGetL id -> "CGetL " ^ string_of_local_id id
  | CGetQuietL id -> "CGetQuietL " ^ string_of_local_id id
  | CGetL2 id -> "CGetL2 " ^ string_of_local_id id
  | CGetL3 id -> "CGetL3 " ^ string_of_local_id id
  | CUGetL id -> "CUGetL " ^ string_of_local_id id
  | PushL id -> "PushL " ^ string_of_local_id id
  | CGetN -> "CGetN"
  | CGetQuietN -> "CGetQuietN"
  | CGetG -> "CGetG"
  | CGetQuietG -> "CGetQuietG"
  | CGetS -> "CGetS"
  | VGetN -> "VGetN"
  | VGetG -> "VGetG"
  | VGetS -> "VGetS"
  | AGetC -> "AGetC"
  | AGetL id -> "AGetL " ^ string_of_local_id id

let string_of_eq_op op =
  match op with
  | PlusEqual -> "PlusEqual"
  | MinusEqual -> "MinusEqual"
  | MulEqual -> "MulEqual"
  | ConcatEqual -> "ConcatEqual"
  | DivEqual -> "DivEqual"
  | PowEqual -> "PowEqual"
  | ModEqual -> "ModEqual"
  | AndEqual -> "AndEqual"
  | OrEqual -> "OrEqual"
  | XorEqual -> "XorEqual"
  | SlEqual -> "SlEqual"
  | SrEqual -> "SrEqual"
  | PlusEqualO -> "PlusEqualO"
  | MinusEqualO -> "MinusEqualO"
  | MulEqualO -> "MulEqualO"

let string_of_incdec_op op =
  match op with
  | PreInc -> "PreInc"
  | PostInc -> "PostInc"
  | PreDec -> "PreDec"
  | PostDec -> "PostDec"
  | PreIncO -> "PreIncO"
  | PostIncO -> "PostIncO"
  | PreDecO -> "PreDecO"
  | PostDecO -> "PostDecO"

let string_of_mutator x =
  match x with
  | SetL id -> "SetL " ^ string_of_local_id id
  | SetN -> "SetN"
  | SetG -> "SetG"
  | SetS -> "SetS"
  | SetOpL (id, op) ->
    "SetOpL " ^ string_of_local_id id ^ " " ^ string_of_eq_op op
  | SetOpN op -> "SetOpN " ^ string_of_eq_op op
  | SetOpG op -> "SetOpG " ^ string_of_eq_op op
  | SetOpS op -> "SetOpS " ^ string_of_eq_op op
  | IncDecL (id, op) ->
    "IncDecL " ^ string_of_local_id id ^ " " ^ string_of_incdec_op op
  | IncDecN op -> "IncDecN " ^ string_of_incdec_op op
  | IncDecG op -> "IncDecG " ^ string_of_incdec_op op
  | IncDecS op -> "IncDecS " ^ string_of_incdec_op op
  | BindL id -> "BindL " ^ string_of_local_id id
  | BindN -> "BindN"
  | BindG -> "BindG"
  | BindS -> "BindS"
  | UnsetN -> "UnsetN"
  | UnsetG -> "UnsetG"
  | CheckProp _ -> failwith "NYI"
  | InitProp _ -> failwith "NYI"

let string_of_control_flow instruction =
  match instruction with
    | RetC -> "RetC"
    | RetV -> "RetV"
    | _ -> failwith "Not Implemented"

let string_of_call instruction =
  match instruction with
    | FPushFuncD (n_params, litstr) ->
      "FPushFuncD "
      ^ string_of_int n_params
      ^ " \"" ^ litstr ^ "\""
  | FCall param_id -> "FCall " ^ param_id
  | _ -> failwith "instruct_call Not Implemented"

let string_of_misc instruction =
  match instruction with
    | VerifyParamType id -> "VerifyParamType " ^ id
    | VerifyRetTypeC -> "VerifyRetTypeC"
    | _ -> failwith "instruct_misc Not Implemented"

let add_instruction_list buffer prefix instructions =
  let process_instr instr =
    B.add_string buffer prefix;
    B.add_string buffer (
      match instr with
      | IBasic    i -> string_of_basic i
      | ILitConst i -> string_of_lit_const i
      | IOp       i -> string_of_operator i
      | IContFlow i -> string_of_control_flow i
      | ICall     i -> string_of_call i
      | IMisc     i -> string_of_misc i
      | IGet      i -> string_of_get i
      | IMutator  i -> string_of_mutator i
    );
    B.add_string buffer "\n" in
  List.iter process_instr instructions

let string_of_flag f =
  match f with
  | Nullable -> "nullable"
  | HHType -> "hh_type"
  | ExtendedHint -> "extended_hint"
  | TypeVar -> "type_var"
  | Soft -> "soft"
  | TypeConstant -> "type_constant"

let quote_str s = "\"" ^ Php_escaping.escape s ^ "\""

(* HHVM uses `N` to denote absence of type information. Otherwise the type
 * is a quoted string *)
let quote_str_option s =
  match s with
  | None -> "N"
  | Some s -> quote_str s

let string_of_type_info ti =
    "<" ^ quote_str_option ti.ti_user_type ^ " "
        ^ quote_str_option ti.ti_type_constraint.tc_name ^ " "
        ^ String.concat " "
            (List.map string_of_flag ti.ti_type_constraint.tc_flags)
    ^ " >"

let add_type_info buf ti =
  B.add_string buf (string_of_type_info ti)

let string_of_type_info_option tio =
  match tio with
  | None -> ""
  | Some ti -> string_of_type_info ti ^ " "

let string_of_param p =
  string_of_type_info_option p.param_type_info ^ p.param_name

let string_of_params ps =
  "(" ^ String.concat ", " (List.map string_of_param ps) ^ ")"

let add_fun_def buf fun_def =
  B.add_string buf "\n.function ";
  B.add_string buf (string_of_type_info_option fun_def.f_return_type);
  B.add_string buf fun_def.f_name;
  B.add_string buf (string_of_params fun_def.f_params);
  B.add_string buf " {\n";
  add_instruction_list buf two_spaces fun_def.f_body;
  B.add_string buf "}\n"

let method_special_attributes m =
  let attrs = [] in
  let attrs = if m.method_is_static then "static" :: attrs else attrs in
  let attrs = if m.method_is_final then "final" :: attrs else attrs in
  let attrs = if m.method_is_abstract then "abstract" :: attrs else attrs in
  let attrs = if m.method_is_public then "public" :: attrs else attrs in
  let attrs = if m.method_is_protected then "protected" :: attrs else attrs in
  let attrs = if m.method_is_private then "private" :: attrs else attrs in
  let text = String.concat " " attrs in
  let text = if text = "" then "" else "[" ^ text ^ "] " in
  text

let add_method_def buf method_def =
  (* TODO: attributes *)
  B.add_string buf "\n  .method ";
  B.add_string buf (method_special_attributes method_def);
  B.add_string buf method_def.method_name;
  (* TODO: generic type parameters *)
  (* TODO: parameters *)
  (* TODO: where clause *)
  B.add_string buf "()";
  (* TODO: return type *)
  B.add_string buf " {\n";
  add_instruction_list buf four_spaces method_def.method_body;
  B.add_string buf "  }\n"

let class_special_attributes c =
  let attrs = [] in
  let attrs = if c.class_is_trait then "trait" :: attrs else attrs in
  let attrs = if c.class_is_interface then "interface" :: attrs else attrs in
  let attrs = if c.class_is_final then "final" :: attrs else attrs in
  let attrs = if c.class_is_enum then "enum" :: attrs else attrs in
  let attrs = if c.class_is_abstract then "abstract" :: attrs else attrs in
  let text = String.concat " " attrs in
  let text = if text = "" then "" else "[" ^ text ^ "] " in
  text

let add_extends buf class_base =
  match class_base with
  | None -> ()
  | Some type_info ->
    begin
      B.add_string buf " extends ";
      add_type_info buf type_info;
    end

let add_class_def buf class_def =
  (* TODO: user attributes *)
  (* TODO: attributes *)
  B.add_string buf "\n.class ";
  B.add_string buf (class_special_attributes class_def);
  B.add_string buf class_def.class_name;
  add_extends buf class_def.class_base;
  (* TODO: implements *)
  B.add_string buf " {\n";
  List.iter (add_method_def buf) class_def.class_methods;
  (* TODO: other members *)
  (* TODO: If there is no ctor, generate one *)
  B.add_string buf "}\n"

let add_prog buf prog =
  List.iter (add_fun_def buf) prog.hhas_fun;
  List.iter (add_class_def buf) prog.hhas_classes

let add_defcls buf classes =
  List.iteri
    (fun count _ -> B.add_string buf (Printf.sprintf "  DefCls %n\n" count))
    classes

let add_top_level buf hhas_prog =
  let main_stmts =
    [ ILitConst (Int Int64.one)
    ; IContFlow RetC
    ] in
  let fun_name = ".main {\n" in
  B.add_string buf fun_name;
  add_defcls buf hhas_prog.hhas_classes;
  add_instruction_list buf two_spaces main_stmts;
  B.add_string buf "}\n"

let to_string hhas_prog =
  let buf = Buffer.create 1024 in
  B.add_string buf "#starts here\n";
  add_top_level buf hhas_prog;
  add_prog buf hhas_prog;
  B.add_string buf "\n#ends here\n";
  B.contents buf
