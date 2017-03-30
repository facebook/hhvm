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
module SS = String_sequence
open H

(* Generic helpers *)
let sep pieces = String.concat " " pieces

let quote_str s = "\"" ^ Php_escaping.escape s ^ "\""
let quote_str_with_escape s = "\\\"" ^ Php_escaping.escape s ^ "\\\""

let string_of_class_id id = quote_str (Utils.strip_ns id)
let string_of_function_id id = quote_str (Utils.strip_ns id)

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

let string_of_classref id = string_of_int id

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

let string_of_lit_const instruction =
  match instruction with
    | Null        -> "Null"
    | Int i       -> sep ["Int"; Int64.to_string i]
    | String str  -> sep ["String"; quote_str str]
    | True        -> "True"
    | False       -> "False"
    | Double d    -> sep ["Double"; d]
    | AddElemC          -> "AddElemC"
    | AddNewElemC       -> "AddNewElemC"
    | Array (i, _)      -> sep ["Array"; "@A_" ^ string_of_int i]
    | ColAddNewElemC    -> "ColAddNewElemC"
    | ColFromArray i    -> sep ["ColFromArray"; string_of_int i]
    | Dict (i, _)       -> sep ["Dict"; "@A_" ^ string_of_int i]
    | Keyset (i, _)     -> sep ["Keyset"; "@A_" ^ string_of_int i]
    | NewCol i          -> sep ["NewCol"; string_of_int i]
    | NewDictArray i    -> sep ["NewDictArray"; string_of_int i]
    | NewKeysetArray i  -> sep ["NewKeysetArray"; string_of_int i]
    | NewVecArray i     -> sep ["NewVecArray"; string_of_int i]
    | NewMixedArray i   -> sep ["NewMixedArray"; string_of_int i]
    | NewPackedArray i  -> sep ["NewPackedArray"; string_of_int i]
    | NewStructArray l  ->
      sep ["NewStructArray"; "<" ^ string_of_list_of_shape_fields l ^ ">"]
    | Vec (i, _)        -> sep ["Vec"; "@A_" ^ string_of_int i]
    | ClsCns (name, id) ->
      sep ["ClsCns"; quote_str name; string_of_classref id]
    | ClsCnsD (name, class_name) ->
      sep ["ClsCnsD"; quote_str name; string_of_class_id class_name]
    | File -> "File"
    | Dir -> "Dir"
    | NYI text -> "NYI: " ^ text
    | NullUninit -> "NullUninit"
    | AddElemV -> "AddElemV"
    | AddNewElemV -> "AddNewElemV"
    | MapAddElemC -> "MapAddElemC"
    | Method -> "Method"
    | NameA -> "NameA"
    | NewArray n -> sep ["NewArray"; string_of_int n]
    | NewMIArray n -> sep ["NewMIArray"; string_of_int n]
    | NewMSArray n -> sep ["NewMSArray"; string_of_int n]
    | NewLikeArrayL (id, n) ->
      sep ["NewLikeArrayL"; string_of_local_id id; string_of_int n]
    | Cns s -> sep ["Cns"; s]
    | CnsE s -> sep ["CnsE"; s]
    | CnsU (s1, s2) -> sep ["CnsU"; s1; s2]

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
    | InstanceOfD id -> sep ["InstanceOfD"; quote_str id]
    | Print -> "Print"
    | Clone -> "Clone"
    | H.Exit -> "Exit"
    | Fatal op -> sep ["Fatal"; FatalOp.to_string op]

let string_of_get x =
  match x with
  | CGetL id -> sep ["CGetL"; string_of_local_id id]
  | CGetQuietL id -> sep ["CGetQuietL"; string_of_local_id id]
  | CGetL2 id -> sep ["CGetL2"; string_of_local_id id]
  | CGetL3 id -> sep ["CGetL3"; string_of_local_id id]
  | CUGetL id -> sep ["CUGetL"; string_of_local_id id]
  | PushL id -> sep ["PushL"; string_of_local_id id]
  | CGetN -> "CGetN"
  | CGetQuietN -> "CGetQuietN"
  | CGetG -> "CGetG"
  | CGetQuietG -> "CGetQuietG"
  | CGetS id -> sep ["CGetS"; string_of_classref id]
  | VGetN -> "VGetN"
  | VGetG -> "VGetG"
  | VGetS id -> sep ["VGetS"; string_of_classref id]
  | AGetC -> "AGetC"
  | AGetL id -> sep ["AGetL"; string_of_local_id id]
  | ClsRefGetL (id, cr) ->
    sep ["ClsRefGetL"; string_of_local_id id; string_of_int cr]
  | ClsRefGetC cr ->
    sep ["ClsRefGetC"; string_of_int cr]

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
  | SetL id -> sep ["SetL"; string_of_local_id id]
  | SetN -> "SetN"
  | SetG -> "SetG"
  | SetS id -> sep ["SetS"; string_of_classref id]
  | SetOpL (id, op) ->
    sep ["SetOpL"; string_of_local_id id; string_of_eq_op op]
  | SetOpN op -> sep ["SetOpN"; string_of_eq_op op]
  | SetOpG op -> sep ["SetOpG"; string_of_eq_op op]
  | SetOpS (op, id) -> sep ["SetOpS"; string_of_eq_op op; string_of_classref id]
  | IncDecL (id, op) ->
    sep ["IncDecL"; string_of_local_id id; string_of_incdec_op op]
  | IncDecN op -> sep ["IncDecN"; string_of_incdec_op op]
  | IncDecG op -> sep ["IncDecG"; string_of_incdec_op op]
  | IncDecS (op, id) ->
    sep ["IncDecS"; string_of_incdec_op op; string_of_classref id]
  | BindL id -> sep ["BindL"; string_of_local_id id]
  | BindN -> "BindN"
  | BindG -> "BindG"
  | BindS id -> sep ["BindS"; string_of_classref id]
  | UnsetL id -> sep ["UnsetL"; string_of_local_id id]
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
  | FPushFunc n ->
    sep ["FPushFunc"; string_of_int n]
  | FPushFuncD (n, id) ->
    sep ["FPushFuncD"; string_of_int n; quote_str id]
  | FPushFuncU (n, id1, id2) ->
    sep ["FPushFuncU"; string_of_int n; quote_str id1; quote_str id2]
  | FPushObjMethod n ->
    sep ["FPushObjMethod"; string_of_int n]
  | FPushObjMethodD (n, id, nf) ->
    sep ["FPushObjMethodD";
      string_of_int n; quote_str id; string_of_null_flavor nf]
  | FPushClsMethod (n, id) ->
    sep ["FPushClsMethod"; string_of_int n; string_of_classref id]
  | FPushClsMethodF (n, id) ->
    sep ["FPushClsMethodF"; string_of_int n; string_of_classref id]
  | FPushClsMethodD (n, id1, id2) ->
    sep ["FPushClsMethodD";
      string_of_int n; string_of_class_id id1; string_of_function_id id2]
  | FPushCtor (n, id) ->
    sep ["FPushCtor"; string_of_int n; string_of_int id]
  | FPushCtorD (n, id) ->
    sep ["FPushCtorD"; string_of_int n; quote_str id]
  | FPushCtorI (n, id) ->
    sep ["FPushCtorI"; string_of_int n; quote_str id]
  | DecodeCufIter (n, l) ->
    sep ["DecodeCufIter"; string_of_int n; string_of_label l]
  | FPushCufIter (n, id) ->
    sep ["FPushCufIter"; string_of_int n; string_of_iterator_id id]
  | FPushCuf n ->
    sep ["FPushCuf"; string_of_int n]
  | FPushCufF n ->
    sep ["FPushCufF"; string_of_int n]
  | FPushCufSafe n ->
    sep ["FPushCufSafe"; string_of_int n]
  | CufSafeArray -> "CufSafeArray"
  | CufSafeReturn -> "CufSafeReturn"
  | FPassC i ->
    sep ["FPassC"; string_of_param_num i]
  | FPassCW i ->
    sep ["FPassCW"; string_of_param_num i]
  | FPassCE i ->
    sep ["FPassCE"; string_of_param_num i]
  | FPassV i ->
    sep ["FPassV"; string_of_param_num i]
  | FPassVNop i ->
    sep ["FPassVNop"; string_of_param_num i]
  | FPassR i ->
    sep ["FPassR"; string_of_param_num i]
  | FPassL (i, lid) ->
    sep ["FPassL"; string_of_param_num i; string_of_local_id lid]
  | FPassN i ->
    sep ["FPassN"; string_of_param_num i]
  | FPassG i ->
    sep ["FPassG"; string_of_param_num i]
  | FPassS (i, id) ->
    sep ["FPassS"; string_of_param_num i; string_of_classref id]
  | FCall n ->
    sep ["FCall"; string_of_int n]
  | FCallD (n, c, f) ->
    sep ["FCallD";
      string_of_int n; string_of_class_id c; string_of_function_id f]
  | FCallArray -> "FCallArray"
  | FCallAwait (n, c, f) ->
    sep ["FCallAwait";
      string_of_int n; string_of_class_id c; string_of_function_id f]
  | FCallUnpack n ->
    sep ["FCallUnpack"; string_of_int n]
  | FCallBuiltin (n1, n2, id) ->
    sep ["FCallBuiltin"; string_of_int n1; string_of_int n2; quote_str id]

let string_of_misc instruction =
  match instruction with
    | This -> "This"
    | Self -> "Self"
    | Parent id -> sep ["Parent"; string_of_classref id]
    | LateBoundCls id -> sep ["LateBoundCls"; string_of_classref id]
    | VerifyParamType id -> sep ["VerifyParamType"; string_of_param_id id]
    | VerifyRetTypeC -> "VerifyRetTypeC"
    | Catch -> "Catch"
    | CheckThis -> "CheckThis"
    | IsUninit -> "IsUninit"
    | CGetCUNop -> "CGetCUNop"
    | UGetCUNop -> "UGetCUNop"
    | StaticLoc (local, text) ->
      sep ["StaticLoc"; string_of_local_id local; quote_str text]
    | StaticLocInit (local, text) ->
      sep ["StaticLocInit"; string_of_local_id local; quote_str text]
    | MemoGet (count, Local.Unnamed first, local_count) ->
      Printf.sprintf "MemoGet %s L:%d+%d"
        (string_of_int count) first (local_count - 1)
    | MemoGet _ -> failwith "MemoGet needs an unnamed local"
    | MemoSet (count, Local.Unnamed first, local_count) ->
      Printf.sprintf "MemoSet %s L:%d+%d"
        (string_of_int count) first (local_count - 1)
    | MemoSet _ -> failwith "MemoSet needs an unnamed local"
    | GetMemoKeyL local ->
      sep ["GetMemoKeyL"; string_of_local_id local]
    | IsMemoType -> "IsMemoType"
    | MaybeMemoType -> "MaybeMemoType"
    | CreateCl (n, cid) ->
      sep ["CreateCl"; string_of_int n; string_of_int cid]
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
  | IterBreak (label, iterlist) ->
      "IterBreak " ^
      (string_of_label label) ^
      "<" ^
      (let list_item = (fun id -> "(Iter) " ^ (string_of_iterator_id id)) in
      let mapped_list = List.map list_item iterlist in
        String.concat ", " mapped_list) ^
      ">"
  | _ -> "### string_of_iterator instruction not implemented"

let string_of_try instruction =
  match instruction with
  | TryFaultBegin label ->
    ".try_fault " ^ (string_of_label label) ^ " {"
  | TryCatchBegin label ->
    ".try_catch " ^ (string_of_label label) ^ " {"
  | TryFaultEnd
  | TryCatchEnd -> "}"

let string_of_async = function
  | Await -> "Await"
  | WHResult -> "# string of WHResult - NYI"

let string_of_generator = function
  | CreateCont -> "CreateCont"
  | Yield -> "Yield"
  | YieldK -> "YieldK"
  | _ -> "### string_of_generator - NYI"

let string_of_include_eval_define = function
  | Incl -> "Incl"
  | InclOnce -> "InclOnce"
  | Req -> "Req"
  | ReqOnce -> "ReqOnce"
  | _ -> "### string_of_include_eval_define - NYI"

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
  | IAsync               i -> string_of_async i
  | IGenerator           i -> string_of_generator i
  | IIncludeEvalDefine   i -> string_of_include_eval_define i
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

let string_of_type_info ?(is_enum = false) ti =
  let user_type = Hhas_type_info.user_type ti in
  let type_constraint = Hhas_type_info.type_constraint ti in
  let flags = Hhas_type_constraint.flags type_constraint in
  let flag_strs = List.map Hhas_type_constraint.string_of_flag flags in
  let name = Hhas_type_constraint.name type_constraint in
  let flags_text = String.concat " " flag_strs in
    "<" ^ quote_str_option user_type ^ " "
        ^ (if not is_enum then quote_str_option name ^ " " else "")
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

let rec string_of_afield = function
  | A.AFvalue e -> string_of_param_default_value e
  | A.AFkvalue (k, v) ->
    string_of_param_default_value k ^ " => " ^ string_of_param_default_value v

and string_of_afield_list afl =
  if List.length afl = 0
  then "\\n"
  else String.concat ", " @@ List.map string_of_afield afl

and shape_field_name_to_expr = function
  | A.SFlit (pos, s)
  | A.SFclass_const (_, (pos, s)) -> (pos, A.String (pos, s))

and string_of_param_default_value expr =
  match snd expr with
  | A.Lvar (_, litstr)
  | A.Float (_, litstr)
  | A.Int (_, litstr) -> litstr
  | A.String (_, litstr) -> "\\\"" ^ litstr ^ "\\\""
  | A.Null -> "NULL"
  | A.True -> "true"
  | A.False -> "false"
  (* For empty array there is a space between array and left paren? a bug ? *)
  | A.Array afl -> "array(" ^ string_of_afield_list afl ^ ")"
  | A.Collection ((_, name), afl) when
    name = "vec" || name = "dict" || name = "keyset" ->
    name ^ "[" ^ string_of_afield_list afl ^ "]"
  | A.Collection ((_, name), afl) when
    name = "Set" || name = "Pair" ->
    "HH\\\\" ^ name ^ "{" ^ string_of_afield_list afl ^ "}"
  | A.Shape fl ->
    let fl =
      List.map
        (fun (f_name, e) ->
          A.AFkvalue (shape_field_name_to_expr f_name, e))
        fl
    in
    string_of_param_default_value (fst expr, A.Array fl)
  (* TODO: printing for other expressions *)
  | _ -> "string_of_param_default_value - NYI"

let string_of_param_default_value_option = function
  | None -> ""
  | Some (label, expr) ->
    " = "
    ^ (string_of_label label)
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

let add_decl_vars buf indent decl_vars = if decl_vars = [] then () else begin
  B.add_string buf (String.make indent ' ');
  B.add_string buf ".declvars ";
  B.add_string buf @@ String.concat " " decl_vars;
  B.add_string buf ";\n"
  end

let add_num_iters buf indent num_iters = if num_iters = 0 then () else begin
  B.add_string buf (String.make indent ' ');
  B.add_string buf ".numiters ";
  B.add_string buf (Printf.sprintf "%d" num_iters);
  B.add_string buf ";\n"
  end

let rec attribute_argument_to_string argument =
  match argument with
  | Null -> SS.str "N;"
  | Double f -> SS.str @@ Printf.sprintf "d:%s;" f
  | String s -> SS.str @@
    Printf.sprintf "s:%d:%s;" (String.length s) (quote_str_with_escape s)
  (* TODO: The False case seems to sometimes be b:0 and sometimes i:0.  Why? *)
  | False -> SS.str "i:0;"
  | True -> SS.str "i:1;"
  | Int i -> SS.str @@ "i:" ^ (Int64.to_string i) ^ ";"
  | Array (num, fields) ->
    attribute_collection_argument_to_string "a" num fields
  | Vec (num, fields) -> attribute_collection_argument_to_string "v" num fields
  | Dict (num, fields) -> attribute_collection_argument_to_string "D" num fields
  | Keyset (num, fields) ->
    attribute_collection_argument_to_string "k" num fields
  | NYI text -> SS.str @@ "NYI: " ^ text
  | NullUninit | AddElemC | AddElemV | AddNewElemC | AddNewElemV
  | MapAddElemC | ColAddNewElemC | File | Dir | Method | NameA
  | NewArray _ | NewMixedArray _ | NewDictArray _
  | NewMIArray _ | NewMSArray _ | NewLikeArrayL (_, _) | NewPackedArray _
  | NewStructArray _ | NewVecArray _ | NewKeysetArray _ | NewCol _
  | ColFromArray _ | Cns _ | CnsE _ | CnsU (_, _) | ClsCns (_, _)
  | ClsCnsD (_, _) -> SS.str
    "\r# NYI: unexpected literal kind in attribute_argument_to_string"

and attribute_collection_argument_to_string col_type num fields =
  let fields = attribute_arguments_to_string fields in
  SS.gather [
    SS.str @@ Printf.sprintf "%s:%d:{" col_type num;
    fields;
    SS.str "}"
  ]

and attribute_arguments_to_string arguments =
  arguments
    |> Core.List.map ~f:attribute_argument_to_string
    |> SS.gather

let attribute_to_string_helper ~has_keys ~if_class_attribute name args =
  let count = List.length args in
  let count =
    if not has_keys then count
    else
      (if count mod 2 = 0 then count / 2
      else failwith
        "attribute string with keys should have even amount of arguments")
  in
  let arguments = attribute_arguments_to_string args in
  let attribute_str = format_of_string @@
    if if_class_attribute
    then "\"%s\"(\"\"\"a:%n:{"
    else "\"\"\"%s:%n:{"
  in
  let attribute_begin = Printf.sprintf attribute_str name count in
  let attribute_end =
    if if_class_attribute
    then "}\"\"\")"
    else "}\"\"\""
  in
  SS.gather [
    SS.str attribute_begin;
    arguments;
    SS.str attribute_end;
  ]

let attribute_to_string a =
  let name = Hhas_attribute.name a in
  let args = Hhas_attribute.arguments a in
  SS.seq_to_string @@
  attribute_to_string_helper ~has_keys:true ~if_class_attribute:true name args

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
  let function_num_iters = Hhas_function.num_iters fun_def in
  let function_is_async = Hhas_function.is_async fun_def in
  let function_is_generator = Hhas_function.is_generator fun_def in
  let function_is_pair_generator = Hhas_function.is_pair_generator fun_def in
  B.add_string buf "\n.function ";
  B.add_string buf (function_attributes fun_def);
  B.add_string buf (string_of_type_info_option function_return_type);
  B.add_string buf function_name;
  B.add_string buf (string_of_params function_params);
  if function_is_generator then B.add_string buf " isGenerator";
  if function_is_async then B.add_string buf " isAsync";
  if function_is_pair_generator then B.add_string buf " isPairGenerator";
  B.add_string buf " {\n";
  add_decl_vars buf 2 function_decl_vars;
  add_num_iters buf 2 function_num_iters;
  add_instruction_list buf 2 function_body;
  B.add_string buf "}\n"

let method_attributes m =
  let user_attrs = Hhas_method.attributes m in
  let attrs = List.map attribute_to_string user_attrs in
  let attrs = if Hhas_method.is_abstract m then "abstract" :: attrs else attrs in
  let attrs = if Hhas_method.is_static m then "static" :: attrs else attrs in
  let attrs = if Hhas_method.is_final m then "final" :: attrs else attrs in
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
  let method_decl_vars = Hhas_method.decl_vars method_def in
  let method_num_iters = Hhas_method.num_iters method_def in
  let method_is_async = Hhas_method.is_async method_def in
  let method_is_generator = Hhas_method.is_generator method_def in
  let method_is_pair_generator = Hhas_method.is_pair_generator method_def in
  let method_is_closure_body = Hhas_method.is_closure_body method_def in
  B.add_string buf "\n  .method ";
  B.add_string buf (method_attributes method_def);
  B.add_string buf (string_of_type_info_option method_return_type);
  B.add_string buf method_name;
  B.add_string buf (string_of_params method_params);
  if method_is_generator then B.add_string buf " isGenerator";
  if method_is_async then B.add_string buf " isAsync";
  if method_is_pair_generator then B.add_string buf " isPairGenerator";
  if method_is_closure_body then B.add_string buf " isClosureBody";
  B.add_string buf " {\n";
  add_decl_vars buf 4 method_decl_vars;
  add_num_iters buf 4 method_num_iters;
  add_instruction_list buf 4 method_body;
  B.add_string buf "  }"

let class_special_attributes c =
  let user_attrs = Hhas_class.attributes c in
  let attrs = List.map attribute_to_string user_attrs in
  let attrs = if Hhas_class.is_closure_class c
              then "no_override" :: "unique" :: attrs
              else attrs in
  let attrs = if Hhas_class.is_trait c then "trait" :: attrs else attrs in
  let attrs =
    if Hhas_class.is_interface c then "interface" :: attrs else attrs
  in
  let attrs = if Hhas_class.is_final c then "final" :: attrs else attrs in
  let attrs =
    if Hhas_class.enum_type c <> None then "enum" :: attrs else attrs
  in
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
    B.add_string buf ")";
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

let add_property class_def buf property =
  B.add_string buf "\n  .property ";
  B.add_string buf (property_attributes property);
  B.add_string buf (Hhas_property.name property);
  B.add_string buf " =\n    ";
  if Hhas_class.is_closure_class class_def
  then B.add_string buf "uninit;"
  else begin
    B.add_string buf "\"\"\"";
    let init = match Hhas_property.initial_value property with
      | None -> SS.str "N;"
      | Some value -> attribute_argument_to_string value
    in
    SS.add_string_from_seq buf init;
    B.add_string buf "\"\"\";"
  end

let add_constant buf c =
  let name = Hhas_constant.name c in
  let value = Hhas_constant.value c in
  B.add_string buf "\n  .const ";
  B.add_string buf name;
  B.add_string buf " = \"\"\"";
  (* TODO: attribute_argument_to_string could stand to be renamed. *)
  SS.add_string_from_seq buf @@ attribute_argument_to_string value;
  B.add_string buf "\"\"\";"

let add_type_constant buf c =
  B.add_string buf "\n  .const ";
  B.add_string buf (Hhas_type_constant.name c);
  (* TODO: Get the actual initializer when we can codegen it. *)
  B.add_string buf " isType = \"\"\"N;\"\"\";"

let add_enum_ty buf c =
  match Hhas_class.enum_type c with
  | Some et ->
    B.add_string buf "\n  .enum_ty ";
    B.add_string buf @@ string_of_type_info ~is_enum:true et;
    B.add_string buf ";"
  | _ -> ()

let add_uses buf c =
  let use_l = Hhas_class.class_uses c in
  match use_l with
  | [] -> ()
  | _  ->
    B.add_string buf @@ Printf.sprintf "\n  .use %s;"
      @@ String.concat " " @@ List.map Utils.strip_ns use_l

let add_class_def buf class_def =
  let class_name = fmt_name (Hhas_class.name class_def) in
  (* TODO: user attributes *)
  B.add_string buf "\n.class ";
  B.add_string buf (class_special_attributes class_def);
  B.add_string buf class_name;
  add_extends buf (Hhas_class.base class_def);
  add_implements buf (Hhas_class.implements class_def);
  B.add_string buf " {";
  add_uses buf class_def;
  add_enum_ty buf class_def;
  List.iter (add_constant buf) (Hhas_class.constants class_def);
  List.iter (add_type_constant buf) (Hhas_class.type_constants class_def);
  List.iter (add_property class_def buf) (Hhas_class.properties class_def);
  List.iter (add_method_def buf) (Hhas_class.methods class_def);
  (* TODO: other members *)
  B.add_string buf "\n}\n"

let add_defcls buf classes =
  List.iteri
    (fun count _ -> B.add_string buf (Printf.sprintf "  DefCls %n\n" count))
    classes

let add_data_region_element ~has_keys buf name num arguments =
  B.add_string buf ".adata A_";
  B.add_string buf @@ string_of_int num;
  B.add_string buf " = ";
  SS.add_string_from_seq buf
    @@ attribute_to_string_helper
      ~if_class_attribute:false
      ~has_keys
      name
      arguments;
  B.add_string buf ";\n"

let add_data_region buf top_level_body functions =
  let rec add_data_region_list buf instr =
    List.iter (add_data_region_aux buf) instr
  and add_data_region_aux buf = function
    | ILitConst (Array (num, arguments)) ->
      add_data_region_element ~has_keys:true buf "a" num arguments
    | ILitConst (Dict (num, arguments)) ->
      add_data_region_element ~has_keys:true buf "D" num arguments
    | ILitConst (Vec (num, arguments)) ->
      add_data_region_element ~has_keys:false buf "v" num arguments
    | ILitConst (Keyset (num, arguments)) ->
      add_data_region_element ~has_keys:false buf "k" num arguments
    | _ -> ()
  and iter_aux buf fun_def =
    let function_body = Hhas_function.body fun_def in
    add_data_region_list buf function_body
  in
  add_data_region_list buf top_level_body;
  List.iter (iter_aux buf) functions;
  B.add_string buf "\n"

let add_top_level buf hhas_prog =
  let non_closure_classes =
    List.filter (fun c -> not (Hhas_class.is_closure_class c))
    (Hhas_program.classes hhas_prog) in
  let main = Hhas_program.main hhas_prog in
  let main_stmts = Hhas_main.body main in
  let main_decl_vars = Hhas_main.decl_vars main in
  let main_num_iters = Hhas_main.num_iters main in
  let fun_name = ".main {\n" in
  B.add_string buf fun_name;
  add_decl_vars buf 2 main_decl_vars;
  add_num_iters buf 2 main_num_iters;
  add_defcls buf non_closure_classes;
  add_instruction_list buf 2 main_stmts;
  B.add_string buf "}\n"

let add_program buf hhas_prog =
  B.add_string buf "#starts here\n";
  let functions = Hhas_program.functions hhas_prog in
  let top_level_body = Hhas_main.body @@ Hhas_program.main hhas_prog in
  add_data_region buf top_level_body functions;
  add_top_level buf hhas_prog;
  List.iter (add_fun_def buf) functions;
  List.iter (add_class_def buf) (Hhas_program.classes hhas_prog);
  B.add_string buf "\n#ends here\n"

let to_string hhas_prog =
  let buf = Buffer.create 1024 in
  add_program buf hhas_prog;
  B.contents buf
