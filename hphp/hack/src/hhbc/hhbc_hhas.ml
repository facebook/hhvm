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
module A = Ast
open H

(* Generic helpers *)
let sep pieces = String.concat " " pieces

let quote_str s = "\"" ^ Php_escaping.escape s ^ "\""

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
    | PopU        -> "PopU"
    | Dup         -> "Dup"
    | Box         -> "Box"
    | Unbox       -> "Unbox"
    | BoxR        -> "BoxR"
    | UnboxR      -> "UnboxR"
    | UnboxRNop   -> "UnboxRNop"
    | RGetCNop    -> "RGetCNop"

let string_of_list_of_shape_fields sl =
  String.concat " " @@ List.map quote_str sl

let string_of_stack_index si = string_of_int si

let string_of_lit_const instruction =
  match instruction with
    | Null        -> "Null"
    | Int i       -> "Int " ^ Int64.to_string i
    | String str  -> "String \"" ^ str ^ "\""
    | True        -> "True"
    | False       -> "False"
    | Double d    -> "Double " ^ d
    | AddElemC          -> "AddElemC"
    | AddNewElemC       -> "AddNewElemC"
    | Array (i, _)      -> "Array @A_" ^ string_of_int i
    | ColAddNewElemC    -> "ColAddNewElemC"
    | ColFromArray i    -> "ColFromArray " ^ string_of_int i
    | Dict (i, _)       -> "Dict @A_" ^ string_of_int i
    | Keyset (i, _)     -> "Keyset @A_" ^ string_of_int i
    | NewCol i          -> "NewCol " ^ string_of_int i
    | NewDictArray i    -> "NewDictArray " ^ string_of_int i
    | NewKeysetArray i  -> "NewKeysetArray " ^ string_of_int i
    | NewVecArray i     -> "NewVecArray " ^ string_of_int i
    | NewMixedArray i   -> "NewMixedArray " ^ string_of_int i
    | NewPackedArray i  -> "NewPackedArray " ^ string_of_int i
    | NewStructArray l  ->
      "NewStructArray <" ^ string_of_list_of_shape_fields l ^ ">"
    | Vec (i, _)        -> "Vec @A_" ^ string_of_int i

    (* TODO *)
    | _ -> "\r# NYI: unexpected literal kind in string_of_lit_const"

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
    | CastArray -> "CastArray"
    | CastObject -> "CastObject"
    | CastVec -> "CastVec"
    | CastDict -> "CastDict"
    | CastKeyset -> "CastKeyset"
    | InstanceOf -> "InstanceOf"
    | InstanceOfD id -> "InstanceOfD " ^ quote_str id
    | Print -> "Print"
    | Clone -> "Clone"
    | H.Exit -> "Exit"
    | Fatal -> "Fatal Runtime"

let string_of_param_id x =
  match x with
  | Param_unnamed i -> string_of_int i
  | Param_named s -> s

let string_of_param_num i = string_of_int i

let string_of_local_id x =
  match x with
  | Local.Unnamed i -> "_" ^ (string_of_int i)
  | Local.Named s -> s
  | Local.Pipe -> failwith "$$ should not have survived to codegen"

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

let string_of_member_key mk =
  let open MemberKey in
  match mk with
  | EC i -> "EC:" ^ string_of_stack_index i
  (* hhas doesn't yet support this syntax *)
  | EL id -> "EL:" ^ string_of_local_id id
  | ET str -> "ET:" ^ quote_str str
  | EI i -> "EI:" ^ Int64.to_string i
  | PC i -> "PC:" ^ string_of_stack_index i
  (* hhas doesn't yet support this syntax *)
  | PL id -> "PL:" ^ string_of_local_id id
  | PT str -> "PT:" ^ quote_str str
  | QT str -> "QT:" ^ quote_str str
  | W -> "W"

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

let string_of_istype_op op =
  match op with
  | OpNull -> "Null"
  | OpBool -> "Bool"
  | OpInt -> "Int"
  | OpDbl -> "Dbl"
  | OpStr -> "Str"
  | OpArr -> "Arr"
  | OpObj -> "Obj"
  | OpScalar -> "Scalar"

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
  | UnsetL id -> "UnsetL " ^ string_of_local_id id
  | UnsetN -> "UnsetN"
  | UnsetG -> "UnsetG"
  | CheckProp _ -> failwith "NYI"
  | InitProp _ -> failwith "NYI"

let string_of_label label =
  match label with
    | Label.Regular id -> "L" ^ (string_of_int id)
    | Label.Catch id -> "C" ^ (string_of_int id)
    | Label.Fault id -> "F" ^ (string_of_int id)
    | Label.DefaultArg id -> "DV" ^ (string_of_int id)

let string_of_switch_kind kind =
  match kind with
  | Unbounded -> "Unbounded"
  | Bounded -> "Bounded"

let string_of_switch kind base labels =
  let kind = string_of_switch_kind kind in
  let labels = String.concat " " @@ List.map string_of_label labels in
  Printf.sprintf "Switch %s %d <%s>" kind base labels

let string_of_control_flow instruction =
  match instruction with
  | Jmp l -> "Jmp " ^ string_of_label l
  | JmpNS l -> "JmpNS " ^ string_of_label l
  | JmpZ l -> "JmpZ " ^ string_of_label l
  | JmpNZ l -> "JmpNZ " ^ string_of_label l
  | RetC -> "RetC"
  | RetV -> "RetV"
  | Throw -> "Throw"
  | Unwind -> "Unwind"
  | Switch (kind, base, labels) -> string_of_switch kind base labels
  | _ -> failwith "instruction_control_flow Not Implemented"

let string_of_iterator_id i = Iterator.to_string i
let string_of_class_id id = quote_str id
let string_of_function_id id = quote_str id
let string_of_null_flavor nf =
  match nf with
  | Ast.OG_nullthrows -> "NullThrows"
  | Ast.OG_nullsafe -> "NullSafe"

let string_of_isset instruction =
  match instruction with
  | IssetC -> "IssetC"
  | IssetL id -> "IssetL " ^ string_of_local_id id
  | IssetN -> "IssetN"
  | IssetG -> "IssetG"
  | IssetS -> "IssetS"
  | EmptyL id -> "EmptyL " ^ string_of_local_id id
  | EmptyN -> "EmptyN"
  | EmptyG -> "EmptyG"
  | EmptyS -> "EmptyS"
  | IsTypeC op -> "IsTypeC " ^ string_of_istype_op op
  | IsTypeL (id, op) ->
    "IsTypeL " ^ string_of_local_id id ^ " " ^ string_of_istype_op op

let string_of_base x =
  match x with
  | BaseNC (si, m) ->
    sep ["BaseNC"; string_of_stack_index si; MemberOpMode.to_string m]
  | BaseNL (id, m) ->
    sep ["BaseNL"; string_of_local_id id; MemberOpMode.to_string m]
  | FPassBaseNC (i, si) ->
    sep ["FBaseBaseNC"; string_of_param_num i; string_of_stack_index si]
  | FPassBaseNL (i, lid) ->
    sep ["FPassBaseNL"; string_of_param_num i; string_of_local_id lid]
  | BaseGC (si, m) ->
    sep ["BaseGC"; string_of_stack_index si; MemberOpMode.to_string m]
  | BaseGL (id, m) ->
    sep ["BaseGL"; string_of_local_id id; MemberOpMode.to_string m]
  | FPassBaseGC (i, si) ->
    sep ["FPassBaseGC"; string_of_param_num i; string_of_stack_index si]
  | FPassBaseGL (i, lid) ->
    sep ["FPassBaseGL"; string_of_param_num i; string_of_local_id lid]
  | BaseSC (si1, si2) ->
    sep ["BaseSC"; string_of_stack_index si1; string_of_stack_index si2]
  | BaseSL (lid, si) ->
    sep ["BaseSL"; string_of_local_id lid; string_of_stack_index si]
  | BaseL (lid, m) ->
    sep ["BaseL"; string_of_local_id lid; MemberOpMode.to_string m]
  | FPassBaseL (i, lid) ->
    sep ["FPassBaseL"; string_of_param_num i; string_of_local_id lid]
  | BaseC si ->
    sep ["BaseC"; string_of_stack_index si]
  | BaseR si ->
    sep ["BaseR"; string_of_stack_index si]
  | BaseH ->
    "BaseH"
  | Dim (m, mk) ->
    sep ["Dim"; MemberOpMode.to_string m; string_of_member_key mk]
  | FPassDim (i, mk) ->
    sep ["FPassDim"; string_of_param_num i; string_of_member_key mk]

let string_of_final instruction =
  match instruction with
  | QueryM (n, op, mk) ->
    sep ["QueryM";
      string_of_int n; QueryOp.to_string op; string_of_member_key mk]
  | VGetM (n, mk) ->
    sep ["VGetM";
      string_of_int n; string_of_member_key mk]
  | UnsetM (n, mk) ->
    sep ["SetM";
      string_of_int n; string_of_member_key mk]
  | BindM (n, mk) ->
    sep ["BindM";
      string_of_int n; string_of_member_key mk]
  | FPassM (i, n, mk) ->
    sep ["FPassM";
      string_of_param_num i; string_of_int n; string_of_member_key mk]
  | SetM (i, mk) ->
    sep ["SetM";
      string_of_param_num i; string_of_member_key mk]
  | SetOpM (i, op, mk) ->
    sep ["SetOpM";
      string_of_param_num i; string_of_eq_op op; string_of_member_key mk]
  | IncDecM (i, op, mk) ->
    sep ["IncDecM";
      string_of_param_num i; string_of_incdec_op op; string_of_member_key mk]
  | _ ->
    "# string_of_final NYI"
(*
| IncDecM of num_params * incdec_op * MemberKey.t
| SetOpM of num_params  * eq_op * MemberKey.t
| SetWithRefLML of local_id * local_id
| SetWithRefRML of local_id
*)

let string_of_call instruction =
  match instruction with
  | FPushFunc n -> "FPushFunc " ^ string_of_int n
  | FPushFuncD (n, id) -> "FPushFuncD " ^ string_of_int n ^ " " ^ quote_str id
  | FPushFuncU (n, id1, id2) ->
    "FPushFuncU " ^ string_of_int n ^ " " ^ quote_str id1 ^ " " ^ quote_str id2
  | FPushObjMethod n -> "FPushObjMethod " ^ string_of_int n
  | FPushObjMethodD (n, id, nf) ->
    "FPushObjMethodD " ^ string_of_int n ^ " " ^ quote_str id
    ^ " " ^ string_of_null_flavor nf
  | FPushClsMethod n -> "FPushClsMethod " ^ string_of_int n
  | FPushClsMethodF n -> "FPushClsMethodF " ^ string_of_int n
  | FPushClsMethodD (n, id1, id2) -> "FPushClsMethodD " ^ string_of_int n
    ^ " " ^ string_of_class_id id1 ^ " " ^  string_of_function_id id2
  | FPushCtor n -> "FPushCtor " ^ string_of_int n
  | FPushCtorD (n, id) -> "FPushCtorD " ^ string_of_int n ^ " " ^ quote_str id
  | FPushCtorI (n, id) -> "FPushCtorI " ^ string_of_int n ^ " " ^ quote_str id
  | DecodeCufIter (n, l) ->
    "DecodeCufIter " ^ string_of_int n ^ " " ^ string_of_label l
  | FPushCufIter (n, id) ->
    "FPushCufIter " ^ string_of_int n ^ " " ^ string_of_iterator_id id
  | FPushCuf n -> "FPushCuf " ^ string_of_int n
  | FPushCufF n -> "FPushCufF " ^  string_of_int n
  | FPushCufSafe n -> "FPushCufSafe " ^ string_of_int n
  | CufSafeArray -> "CufSafeArray"
  | CufSafeReturn -> "CufSafeReturn"
  | FPassC i -> "FPassC " ^ string_of_param_num i
  | FPassCW i -> "FPassCW " ^ string_of_param_num i
  | FPassCE i -> "FPassCE " ^ string_of_param_num i
  | FPassV i -> "FPassV " ^ string_of_param_num i
  | FPassVNop i -> "FPassVNop " ^ string_of_param_num i
  | FPassR i -> "FPassR " ^ string_of_param_num i
  | FPassL (i, lid) ->
    "FPassL " ^ string_of_param_num i ^ " " ^ string_of_local_id lid
  | FPassN i -> "FPassN " ^ string_of_param_num i
  | FPassG i -> "FPassG " ^ string_of_param_num i
  | FPassS i -> "FPassS " ^ string_of_param_num i
  | FCall n -> "FCall " ^ string_of_int n
  | FCallD (n, c, f) ->
    "FCallD " ^ string_of_int n ^ " " ^
    string_of_class_id c ^ " " ^ string_of_function_id f
  | FCallArray -> "FCallArray"
  | FCallAwait (n, c, f) ->
    "FCallAwait " ^ string_of_int n ^ " " ^
    string_of_class_id c ^ " " ^ string_of_function_id f
  | FCallUnpack n -> "FCallUnpack " ^ string_of_int n
  | FCallBuiltin (n1, n2, id) ->
    "FCallBuiltin " ^ string_of_int n1 ^ " " ^ string_of_int n2 ^ " " ^
    quote_str id

let string_of_misc instruction =
  match instruction with
    | This -> "This"
    | Self -> "Self"
    | Parent -> "Parent"
    | LateBoundCls -> "LateBoundCls"
    | VerifyParamType id -> "VerifyParamType " ^ string_of_param_id id
    | VerifyRetTypeC -> "VerifyRetTypeC"
    | Catch -> "Catch"
    | CheckThis -> "CheckThis"
    | IsUninit -> "IsUninit"
    | CGetCUNop -> "CGetCUNop"
    | UGetCUNop -> "UGetCUNop"
    | StaticLoc (local, text) ->
      "StaticLoc " ^ (string_of_local_id local) ^ " " ^ (quote_str text)
    | StaticLocInit (local, text) -> (* TODO: The $ is unnecessarily escaped. *)
      "StaticLocInit " ^ (string_of_local_id local) ^ " " ^ (quote_str text)
    | MemoGet (count, Local.Unnamed first, local_count) ->
      Printf.sprintf "MemoGet %s L:%d+%d"
        (string_of_int count) first (local_count - 1)
    | MemoGet _ -> failwith "MemoGet needs an unnamed local"
    | MemoSet (count, Local.Unnamed first, local_count) ->
      Printf.sprintf "MemoSet %s L:%d+%d"
        (string_of_int count) first (local_count - 1)
    | MemoSet _ -> failwith "MemoSet needs an unnamed local"
    | GetMemoKeyL local -> "GetMemoKeyL " ^ (string_of_local_id local)
    | IsMemoType -> "IsMemoType"
    | MaybeMemoType -> "MaybeMemoType"
    | _ -> failwith "instruct_misc Not Implemented"

let string_of_iterator instruction =
  match instruction with
  | IterInit (id, label, value) ->
    "IterInit " ^
    (string_of_iterator_id id) ^ " " ^
    (string_of_label label) ^ " " ^
    (string_of_local_id value)
  | IterInitK (id, label, key, value) ->
    "IterInitK " ^
    (string_of_iterator_id id) ^ " " ^
    (string_of_label label) ^ " " ^
    (string_of_local_id key) ^ " " ^
    (string_of_local_id value)
  | IterNext (id, label, value) ->
    "IterNext " ^
    (string_of_iterator_id id) ^ " " ^
    (string_of_label label) ^ " " ^
    (string_of_local_id value)
  | IterNextK (id, label, key, value) ->
    "IterNextK " ^
    (string_of_iterator_id id) ^ " " ^
    (string_of_label label) ^ " " ^
    (string_of_local_id key) ^ " " ^
    (string_of_local_id value)
  | IterFree id ->
    "IterFree " ^ (string_of_iterator_id id)
  | _ -> "### string_of_iterator instruction not implemented"

let string_of_try instruction =
  match instruction with
  | TryFaultBegin label ->
    ".try_fault " ^ (string_of_label label) ^ " {"
  | TryCatchBegin label ->
    ".try_catch " ^ (string_of_label label) ^ " {"
  | TryFaultEnd
  | TryCatchEnd -> "}"

let string_of_instruction instruction =
  let s = match instruction with
  | IIterator            i -> string_of_iterator i
  | IBasic               i -> string_of_basic i
  | ILitConst            i -> string_of_lit_const i
  | IOp                  i -> string_of_operator i
  | IContFlow            i -> string_of_control_flow i
  | ICall                i -> string_of_call i
  | IMisc                i -> string_of_misc i
  | IGet                 i -> string_of_get i
  | IMutator             i -> string_of_mutator i
  | ILabel               l -> string_of_label l ^ ":"
  | IIsset               i -> string_of_isset i
  | IBase                i -> string_of_base i
  | IFinal               i -> string_of_final i
  | ITry                 i -> string_of_try i
  | IComment             s -> "# " ^ s
  | _ -> failwith "invalid instruction" in
  s ^ "\n"

let adjusted_indent instruction indent =
  match instruction with
  | IComment _ -> 0
  | ILabel _
  | ITry TryFaultEnd
  | ITry TryCatchEnd -> indent - 2
  | _ -> indent

let new_indent instruction indent =
  match instruction with
  | ITry (TryFaultBegin _)
  | ITry (TryCatchBegin _) -> indent + 2
  | ITry TryFaultEnd
  | ITry TryCatchEnd -> indent - 2
  | _ -> indent

let add_instruction_list buffer indent instructions =
  let rec aux instructions indent =
    match instructions with
    | [] -> ()
    | instruction :: t ->
      begin
      let actual_indent = adjusted_indent instruction indent in
      B.add_string buffer (String.make actual_indent ' ');
      B.add_string buffer (string_of_instruction instruction);
      aux t (new_indent instruction indent)
      end in
  aux instructions indent

(* HHVM uses `N` to denote absence of type information. Otherwise the type
 * is a quoted string *)
let quote_str_option s =
  match s with
  | None -> "N"
  | Some s -> quote_str s

let string_of_type_info ti =
  let user_type = Hhas_type_info.user_type ti in
  let type_constraint = Hhas_type_info.type_constraint ti in
  let flags = Hhas_type_constraint.flags type_constraint in
  let flag_strs = List.map Hhas_type_constraint.string_of_flag flags in
  let name = Hhas_type_constraint.name type_constraint in
  let flags_text = String.concat " " flag_strs in
    "<" ^ quote_str_option user_type ^ " "
        ^ quote_str_option name ^ " "
        ^ flags_text
    ^ " >"

let string_of_type_infos type_infos =
  let strs = List.map string_of_type_info type_infos in
  String.concat " " strs

let add_type_info buf ti =
  B.add_string buf (string_of_type_info ti)

let add_type_infos buf type_infos =
  B.add_string buf (string_of_type_infos type_infos)

let string_of_type_info_option tio =
  match tio with
  | None -> ""
  | Some ti -> string_of_type_info ti ^ " "

let string_of_param_default_value expr =
  match snd expr with
  | A.Float (_, litstr)
  | A.Int (_, litstr) -> litstr
  | A.String (_, litstr) -> "\\\"" ^ litstr ^ "\\\""
  | A.Null -> "NULL"
  | A.True -> "true"
  | A.False -> "false"
  (* TODO: printing for other expressions e.g. arrays, vecs, shapes.. *)
  | _ -> "string_of_param_default_value - NYI"

let string_of_param_default_value_option = function
  | None -> ""
  | Some (label, expr) ->
    " = DV"
    ^ (string_of_int (Label.id label))
    ^ "(\"\"\""
    ^ (string_of_param_default_value expr)
    ^ "\"\"\")"

let string_of_param p =
  let param_type_info = Hhas_param.type_info p in
  let param_name = Hhas_param.name p in
  let param_default_value = Hhas_param.default_value p in
  string_of_type_info_option param_type_info
  ^ param_name
  ^ string_of_param_default_value_option param_default_value

let string_of_params ps =
  "(" ^ String.concat ", " (List.map string_of_param ps) ^ ")"

(* Taken from emitter/emitter_core.ml *)
let fix_xhp_name s =
    if String.length s = 0 || s.[0] <> ':' then s else
      "xhp_" ^
        String_utils.lstrip s ":" |>
        Str.global_replace (Str.regexp ":") "__" |>
        Str.global_replace (Str.regexp "-") "_"

let fmt_name s = fix_xhp_name (Utils.strip_ns s)

let add_decl_vars buf decl_vars = if decl_vars = [] then () else begin
  B.add_string buf "  .declvars ";
  B.add_string buf @@ String.concat " " decl_vars;
  B.add_string buf ";\n"
  end

let attribute_argument_to_string argument =
  let value = match argument with
  | Null -> "N"
  | Double f -> Printf.sprintf "d:%s" f
  | String s ->
    Printf.sprintf "s:%d:%s" (String.length s) ("\\" ^ quote_str s ^ "\\")
  | False -> "i:0"
  | True -> "i:1"
  | Int i -> "i:" ^ (Int64.to_string i)
  | _ -> failwith "unexpected value in attribute_argument_to_string" in
  Printf.sprintf "%s;" value

let attribute_arguments_to_string arguments =
  let rec aux arguments acc =
    match arguments with
    | h :: t -> aux t (acc ^ attribute_argument_to_string h)
    | _ -> acc in
  aux arguments ""

let attribute_to_string_helper ~if_class_attribute name args =
  let count = List.length args in
  let count =
    if count mod 2 = 0 then count / 2
    else failwith "attribute string should have even amount of arguments"
  in
  let arguments = attribute_arguments_to_string args in
  let attribute_str = format_of_string @@
    if if_class_attribute
    then "\"%s\"(\"\"\"a:%n:{%s}\"\"\")"
    else "\"\"\"%s:%n:{%s}\"\"\""
  in
  Printf.sprintf attribute_str name count arguments

let attribute_to_string a =
  let name = Hhas_attribute.name a in
  let args = Hhas_attribute.arguments a in
  attribute_to_string_helper ~if_class_attribute:true name args

let function_attributes f =
  let user_attrs = Hhas_function.attributes f in
  let attrs = List.map attribute_to_string user_attrs in
  let text = String.concat " " attrs in
  if text = "" then "" else "[" ^ text ^ "] "

let add_fun_def buf fun_def =
  let function_name = fmt_name (Hhas_function.name fun_def) in
  let function_return_type = Hhas_function.return_type fun_def in
  let function_params = Hhas_function.params fun_def in
  let function_body = Hhas_function.body fun_def in
  let function_decl_vars = Hhas_function.decl_vars fun_def in
  B.add_string buf "\n.function ";
  B.add_string buf (function_attributes fun_def);
  B.add_string buf (string_of_type_info_option function_return_type);
  B.add_string buf function_name;
  B.add_string buf (string_of_params function_params);
  B.add_string buf " {\n";
  add_decl_vars buf function_decl_vars;
  add_instruction_list buf 2 function_body;
  B.add_string buf "}\n"

let method_attributes m =
  let user_attrs = Hhas_method.attributes m in
  let attrs = List.map attribute_to_string user_attrs in
  let attrs = if Hhas_method.is_static m then "static" :: attrs else attrs in
  let attrs = if Hhas_method.is_final m then "final" :: attrs else attrs in
  let attrs = if Hhas_method.is_abstract m then "abstract" :: attrs else attrs in
  let attrs = if Hhas_method.is_public m then "public" :: attrs else attrs in
  let attrs = if Hhas_method.is_protected m then "protected" :: attrs else attrs in
  let attrs = if Hhas_method.is_private m then "private" :: attrs else attrs in
  let text = String.concat " " attrs in
  let text = if text = "" then "" else "[" ^ text ^ "] " in
  text

let add_method_def buf method_def =
  (* TODO: In the original codegen sometimes a missing return type is not in
  the text at all and sometimes it is <"" N  > -- which should we generate,
  and when? *)
  let method_name = fmt_name (Hhas_method.name method_def) in
  let method_return_type = Hhas_method.return_type method_def in
  let method_params = Hhas_method.params method_def in
  let method_body = Hhas_method.body method_def in
  B.add_string buf "\n  .method ";
  B.add_string buf (method_attributes method_def);
  B.add_string buf (string_of_type_info_option method_return_type);
  B.add_string buf method_name;
  B.add_string buf (string_of_params method_params);
  B.add_string buf " {\n";
  add_instruction_list buf 4 method_body;
  B.add_string buf "  }"

let class_special_attributes c =
  let user_attrs = Hhas_class.attributes c in
  let attrs = List.map attribute_to_string user_attrs in
  let attrs = if Hhas_class.is_trait c then "trait" :: attrs else attrs in
  let attrs = if Hhas_class.is_interface c then "interface" :: attrs else attrs in
  let attrs = if Hhas_class.is_final c then "final" :: attrs else attrs in
  let attrs = if Hhas_class.is_enum c then "enum" :: attrs else attrs in
  let attrs = if Hhas_class.is_abstract c then "abstract" :: attrs else attrs in
  let text = String.concat " " attrs in
  let text = if text = "" then "" else "[" ^ text ^ "] " in
  text

let add_extends buf class_base =
  match class_base with
  | None -> ()
  | Some name ->
    begin
      B.add_string buf " extends ";
      B.add_string buf (fmt_name name);
    end

let add_implements buf class_implements =
  match class_implements with
  | [] -> ()
  | _ ->
  begin
    B.add_string buf " implements (";
    B.add_string buf (String.concat " " (List.map fmt_name class_implements));
    B.add_string buf ") ";
  end

let property_attributes p =
  let module P = Hhas_property in
  let attrs = [] in
  let attrs = if P.is_static p then "static" :: attrs else attrs in
  let attrs = if P.is_public p then "public" :: attrs else attrs in
  let attrs = if P.is_protected p then "protected" :: attrs else attrs in
  let attrs = if P.is_private p then "private" :: attrs else attrs in
  let text = String.concat " " attrs in
  let text = if text = "" then "" else "[" ^ text ^ "] " in
  text

let add_property buf property =
  B.add_string buf "\n  .property ";
  B.add_string buf (property_attributes property);
  B.add_string buf (Hhas_property.name property);
  (* TODO: Get the actual initializer when we can codegen it. Properties
  that lack an initializer get a null. *)
  B.add_string buf " =\n    \"\"\"N;\"\"\";"

let add_constant buf c =
  B.add_string buf "\n  .const ";
  B.add_string buf (Hhas_constant.name c);
  (* TODO: Get the actual initializer when we can codegen it. *)
  B.add_string buf " = \"\"\"N;\"\"\";"

let add_type_constant buf c =
  B.add_string buf "\n  .const ";
  B.add_string buf (Hhas_type_constant.name c);
  (* TODO: Get the actual initializer when we can codegen it. *)
  B.add_string buf " isType = \"\"\"N;\"\"\";"

let add_class_def buf class_def =
  let class_name = fmt_name (Hhas_class.name class_def) in
  (* TODO: user attributes *)
  B.add_string buf "\n.class ";
  B.add_string buf (class_special_attributes class_def);
  B.add_string buf class_name;
  add_extends buf (Hhas_class.base class_def);
  add_implements buf (Hhas_class.implements class_def);
  B.add_string buf " {";
  List.iter (add_constant buf) (Hhas_class.constants class_def);
  List.iter (add_type_constant buf) (Hhas_class.type_constants class_def);
  List.iter (add_property buf) (Hhas_class.properties class_def);
  List.iter (add_method_def buf) (Hhas_class.methods class_def);
  (* TODO: other members *)
  B.add_string buf "\n}\n"

let add_defcls buf classes =
  List.iteri
    (fun count _ -> B.add_string buf (Printf.sprintf "  DefCls %n\n" count))
    classes

let add_data_region_element buf name num arguments =
  B.add_string buf ".adata A_";
  B.add_string buf @@ string_of_int num;
  B.add_string buf " = ";
  B.add_string buf
    @@ attribute_to_string_helper ~if_class_attribute:false name arguments;
  B.add_string buf ";\n"

let add_data_region buf functions =
  let rec add_data_region_list buf instr =
    List.iter (add_data_region_aux buf) instr
  and add_data_region_aux buf = function
    | ILitConst (Array (num, arguments)) ->
      add_data_region_element buf "a" num arguments
    | ILitConst (Dict (num, arguments)) ->
      add_data_region_element buf "D" num arguments
    | ILitConst (Vec (num, arguments)) ->
      add_data_region_element buf "v" num arguments
    | ILitConst (Keyset (num, arguments)) ->
      add_data_region_element buf "k" num arguments
    | _ -> ()
  and iter_aux buf fun_def =
    let function_body = Hhas_function.body fun_def in
    add_data_region_list buf function_body
  in
  List.iter (iter_aux buf) functions;
  B.add_string buf "\n"

let add_top_level buf hhas_prog =
  let main_stmts =
    [ ILitConst (Int Int64.one)
    ; IContFlow RetC
    ] in
  let fun_name = ".main {\n" in
  B.add_string buf fun_name;
  add_defcls buf (Hhas_program.classes hhas_prog);
  add_instruction_list buf 2 main_stmts;
  B.add_string buf "}\n"

let add_program buf hhas_prog =
  B.add_string buf "#starts here\n";
  let functions = Hhas_program.functions hhas_prog in
  add_data_region buf functions;
  add_top_level buf hhas_prog;
  List.iter (add_fun_def buf) functions;
  List.iter (add_class_def buf) (Hhas_program.classes hhas_prog);
  B.add_string buf "\n#ends here\n"

let to_string hhas_prog =
  let buf = Buffer.create 1024 in
  add_program buf hhas_prog;
  B.contents buf
