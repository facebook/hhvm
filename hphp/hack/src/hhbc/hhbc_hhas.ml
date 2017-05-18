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
module SU = Hhbc_string_utils
module TV = Typed_value
module TVMap = Typed_value.TVMap
open H

(* State associated with an entire file *)
let class_counter = ref 0
let next_class_counter () =
  let c = !class_counter in
  class_counter := c + 1;
  c

let typedef_counter = ref 0
let next_typedef_counter () =
  let c = !typedef_counter in
  typedef_counter := c + 1;
  c

(* State associated with a single body of code *)
let total_named_locals = ref 0

(* Array identifier map *)
let array_identifier_counter = ref 0
let next_array_identifier () =
  let c = !array_identifier_counter in
  array_identifier_counter := c + 1;
  c

let array_identifier_map = ref TVMap.empty
let should_create_adata_entry tval =
  if TVMap.mem tval !array_identifier_map
  then (false, -1)
  else begin
    let i = next_array_identifier () in
    array_identifier_map := TVMap.add tval i !array_identifier_map;
    (true, i)
    end
let get_array_identifier tv =
  match TVMap.get tv !array_identifier_map with
  | None -> failwith "Such array does not exist in adata?"
  | Some i -> string_of_int i

(* Generic helpers *)
let sep pieces = String.concat " " pieces

let string_of_class_id id =
  SU.quote_string (Hhbc_id.Class.to_raw_string id)
let string_of_function_id id =
  SU.quote_string (Hhbc_id.Function.to_raw_string id)
let string_of_method_id id =
  SU.quote_string (Hhbc_id.Method.to_raw_string id)
let string_of_const_id id =
  SU.quote_string (Hhbc_id.Const.to_raw_string id)
let string_of_prop_id id =
  SU.quote_string (Hhbc_id.Prop.to_raw_string id)

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
  String.concat " " @@ List.map SU.quote_string sl

let string_of_stack_index si = string_of_int si

let string_of_classref id = string_of_int id

let string_of_param_id x =
  match x with
  | Param_unnamed i -> string_of_int i
  | Param_named s -> s

let string_of_param_num i = string_of_int i

let string_of_local_id x =
  match x with
  | Local.Unnamed i -> "_" ^ (string_of_int (!total_named_locals + i))
  | Local.Named s -> s
  | Local.Pipe -> failwith "$$ should not have survived to codegen"

let string_of_lit_const instruction =
  match instruction with
    | Null        -> "Null"
    | Int i       -> sep ["Int"; Int64.to_string i]
    | String str  -> sep ["String"; SU.quote_string str]
    | True        -> "True"
    | False       -> "False"
    | Double d    -> sep ["Double"; d]
    | AddElemC          -> "AddElemC"
    | AddNewElemC       -> "AddNewElemC"
    | Array tv          -> sep ["Array"; "@A_" ^ get_array_identifier tv]
    | Dict tv           -> sep ["Dict"; "@A_" ^ get_array_identifier tv]
    | Keyset tv         -> sep ["Keyset"; "@A_" ^ get_array_identifier tv]
    | Vec tv            -> sep ["Vec"; "@A_" ^ get_array_identifier tv]
    | ColFromArray i    -> sep ["ColFromArray"; string_of_int i]
    | NewCol i          -> sep ["NewCol"; string_of_int i]
    | NewDictArray i    -> sep ["NewDictArray"; string_of_int i]
    | NewKeysetArray i  -> sep ["NewKeysetArray"; string_of_int i]
    | NewVecArray i     -> sep ["NewVecArray"; string_of_int i]
    | NewMixedArray i   -> sep ["NewMixedArray"; string_of_int i]
    | NewPackedArray i  -> sep ["NewPackedArray"; string_of_int i]
    | NewStructArray l  ->
      sep ["NewStructArray"; "<" ^ string_of_list_of_shape_fields l ^ ">"]
    | NewPair -> "NewPair"
    | ClsCns (cnsid, cr) ->
      sep ["ClsCns"; string_of_const_id cnsid; string_of_classref cr]
    | ClsCnsD (cnsid, cid) ->
      sep ["ClsCnsD"; string_of_const_id cnsid; string_of_class_id cid]
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
    | Cns cnsid -> sep ["Cns"; string_of_const_id cnsid]
    | CnsE cnsid -> sep ["CnsE"; string_of_const_id cnsid]
    | CnsU (id1, id2) ->
      sep ["CnsU"; string_of_const_id id1; SU.quote_string id2]

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
    | InstanceOfD id -> sep ["InstanceOfD"; string_of_class_id id]
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
  | VGetL id -> sep ["VGetL"; string_of_local_id id]
  | ClsRefGetL (id, cr) ->
    sep ["ClsRefGetL"; string_of_local_id id; string_of_int cr]
  | ClsRefGetC cr ->
    sep ["ClsRefGetC"; string_of_int cr]

let string_of_member_key mk =
  let open MemberKey in
  match mk with
  | EC i -> "EC:" ^ string_of_stack_index i
  | EL id -> "EL:" ^ string_of_local_id id
  | ET str -> "ET:" ^ SU.quote_string str
  | EI i -> "EI:" ^ Int64.to_string i
  | PC i -> "PC:" ^ string_of_stack_index i
  | PL id -> "PL:" ^ string_of_local_id id
  | PT id -> "PT:" ^ string_of_prop_id id
  | QT id -> "QT:" ^ string_of_prop_id id
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
  | OpVec -> "Vec"
  | OpDict -> "Dict"
  | OpKeyset -> "Keyset"

let string_of_initprop_op op =
  match op with
  | NonStatic -> "NonStatic"
  | Static -> "Static"

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
  | CheckProp id -> sep ["CheckProp"; string_of_prop_id id]
  | InitProp (id, op) -> sep ["InitProp"; string_of_prop_id id;
      string_of_initprop_op op]

let string_of_label label =
  match label with
    | Label.Regular id -> "L" ^ (string_of_int id)
    | Label.Catch id -> "C" ^ (string_of_int id)
    | Label.Fault id -> "F" ^ (string_of_int id)
    | Label.DefaultArg id -> "DV" ^ (string_of_int id)
    | Label.Named _ -> failwith "Label should be rewritten before this point"

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
  | IssetS cls -> "IssetS " ^ string_of_int cls
  | EmptyL id -> "EmptyL " ^ string_of_local_id id
  | EmptyN -> "EmptyN"
  | EmptyG -> "EmptyG"
  | EmptyS cls -> "EmptyS " ^ string_of_int cls
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
  | BaseSC (si, id) ->
    sep ["BaseSC"; string_of_stack_index si; string_of_classref id]
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
    sep ["UnsetM";
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
    sep ["FPushFuncD"; string_of_int n; string_of_function_id id]
  | FPushFuncU (n, id1, id2) ->
    sep ["FPushFuncU"; string_of_int n; string_of_function_id id1; SU.quote_string id2]
  | FPushObjMethod (n, nf) ->
    sep ["FPushObjMethod"; string_of_int n; string_of_null_flavor nf]
  | FPushObjMethodD (n, id, nf) ->
    sep ["FPushObjMethodD";
      string_of_int n; string_of_method_id id; string_of_null_flavor nf]
  | FPushClsMethod (n, id) ->
    sep ["FPushClsMethod"; string_of_int n; string_of_classref id]
  | FPushClsMethodF (n, id) ->
    sep ["FPushClsMethodF"; string_of_int n; string_of_classref id]
  | FPushClsMethodD (n, id, cid) ->
    sep ["FPushClsMethodD";
      string_of_int n;
      string_of_method_id id; string_of_class_id cid]
  | FPushCtor (n, id) ->
    sep ["FPushCtor"; string_of_int n; string_of_int id]
  | FPushCtorD (n, cid) ->
    sep ["FPushCtorD"; string_of_int n; string_of_class_id cid]
  | FPushCtorI (n, id) ->
    sep ["FPushCtorI"; string_of_int n; string_of_classref id]
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
    sep ["FCallBuiltin"; string_of_int n1; string_of_int n2; SU.quote_string id]

let string_of_barethis_op i =
  match i with
  | Notice -> "Notice"
  | NoNotice -> "NoNotice"

let string_of_misc instruction =
  match instruction with
    | This -> "This"
    | BareThis op -> sep ["BareThis"; string_of_barethis_op op]
    | Self id -> sep ["Self"; string_of_classref id]
    | Parent id -> sep ["Parent"; string_of_classref id]
    | LateBoundCls id -> sep ["LateBoundCls"; string_of_classref id]
    | ClsRefName id -> sep ["ClsRefName"; string_of_classref id]
    | VerifyParamType id -> sep ["VerifyParamType"; string_of_param_id id]
    | VerifyRetTypeC -> "VerifyRetTypeC"
    | Catch -> "Catch"
    | CheckThis -> "CheckThis"
    | IsUninit -> "IsUninit"
    | CGetCUNop -> "CGetCUNop"
    | UGetCUNop -> "UGetCUNop"
    | StaticLoc (local, text) ->
      sep ["StaticLoc"; string_of_local_id local; "\"" ^ text ^ "\""]
    | StaticLocInit (local, text) ->
      sep ["StaticLocInit"; string_of_local_id local; "\"" ^ text ^ "\""]
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
    | Idx -> "Idx"
    | ArrayIdx -> "ArrayIdx"
    | InitThisLoc id -> sep ["InitThisLoc"; string_of_local_id id]
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
  | TryCatchLegacyBegin label ->
    ".try_catch " ^ (string_of_label label) ^ " {"
  | TryFaultEnd
  | TryCatchLegacyEnd -> "}"
  | TryCatchBegin -> ".try {"
  | TryCatchMiddle -> "} .catch {"
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
  | ReqDoc -> "ReqDoc"
  | Eval -> "Eval"
  | DefFunc id -> sep ["DefFunc"; string_of_function_id id]
  | DefCls _id -> sep ["DefCls"; string_of_int (next_class_counter ())]
  | DefClsNop id -> sep ["DefClsNop"; string_of_class_id id]
  | DefCns id -> sep ["DefCns"; string_of_const_id id]
  | DefTypeAlias _id ->
    sep ["DefTypeAlias"; string_of_int (next_typedef_counter ())]


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
  | ITry TryCatchLegacyEnd
  | ITry TryCatchMiddle
  | ITry TryCatchEnd -> indent - 2
  | _ -> indent

let new_indent instruction indent =
  match instruction with
  | ITry (TryFaultBegin _)
  | ITry (TryCatchLegacyBegin _)
  | ITry TryCatchBegin -> indent + 2
  | ITry TryFaultEnd
  | ITry TryCatchLegacyEnd
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
  | Some s -> SU.quote_string s

let string_of_type_flags flags =
  let flag_strs = List.map Hhas_type_constraint.string_of_flag flags in
  let flags_text = String.concat " " flag_strs in
  flags_text

let string_of_type_info ?(is_enum = false) ti =
  let user_type = Hhas_type_info.user_type ti in
  let type_constraint = Hhas_type_info.type_constraint ti in
  let flags = Hhas_type_constraint.flags type_constraint in
  let flags_text = string_of_type_flags flags in
  let name = Hhas_type_constraint.name type_constraint in
    "<" ^ quote_str_option user_type ^ " "
        ^ (if not is_enum then quote_str_option name ^ " " else "")
        ^ flags_text
    ^ " >"

let string_of_typedef_info ti =
  let type_constraint = Hhas_type_info.type_constraint ti in
  let name = Hhas_type_constraint.name type_constraint in
  let flags = Hhas_type_constraint.flags type_constraint in
  (* TODO: check if other flags are emitted for type aliases *)
  let flags = List.filter (fun f -> f = Hhas_type_constraint.Nullable) flags in
  let flags_text = string_of_type_flags flags in
    "<" ^ SU.quote_string (Option.value ~default:"" name)
    ^ " " ^ flags_text ^ " >"

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
  then ""
  else String.concat ", " @@ List.map string_of_afield afl

and shape_field_name_to_expr = function
  | A.SFlit (pos, s)
  | A.SFclass_const (_, (pos, s)) -> (pos, A.String (pos, s))

and string_of_bop = function
  | A.Plus -> "+"
  | A.Minus -> "-"
  | A.Star -> "*"
  | A.Slash -> "/"
  | A.Eqeq -> "=="
  | A.EQeqeq -> "==="
  | A.Starstar -> "**"
  | A.Eq None -> "="
  | A.Eq (Some bop) -> "=" ^ string_of_bop bop
  | A.AMpamp -> "&&"
  | A.BArbar -> "||"
  | A.Lt -> "<"
  | A.Lte -> "<="
  | A.Cmp -> "<=>"
  | A.Gt -> ">"
  | A.Gte -> ">="
  | A.Dot -> "."
  | A.Amp -> "&"
  | A.Bar -> "|"
  | A.Ltlt -> "<<"
  | A.Gtgt -> ">>"
  | A.Percent -> "%"
  | A.Xor -> "^"
  | A.Diff -> "!="
  | A.Diff2 -> "!=="

and string_of_uop = function
  | A.Utild -> "~"
  | A.Unot -> "!"
  | A.Uplus -> "+"
  | A.Uminus -> "-"
  | A.Uincr -> "++"
  | A.Udecr -> "--"
  | A.Uref -> "&"
  | A.Upincr
  | A.Updecr
  | A.Usplat -> failwith "string_of_uop - should have been captures earlier"

and string_of_hint ~ns h =
  let h =
    Emit_type_hint.fmt_hint
      ~tparams:[]
      ~namespace:Namespace_env.empty_with_default_popt
      h
  in
  if ns then h else SU.strip_ns h

and string_of_import_flavor = function
  | A.Include -> "include"
  | A.Require -> "require"
  | A.IncludeOnce -> "include_once"
  | A.RequireOnce -> "require_once"

and string_of_fun f use_list =
  let string_of_args p =
    let hint =
      Option.value_map p.A.param_hint ~default:"" ~f:(string_of_hint ~ns:true)
    in
    let is_ref = if p.A.param_is_reference then "&" else "" in
    let name = snd @@ p.A.param_id in
    let default_val =
      Option.value_map
        p.A.param_expr
        ~default:""
        ~f:(fun e -> " = " ^ (string_of_param_default_value e))
    in
    hint ^ " " ^ is_ref ^ name ^ default_val
  in
  let args = String.concat ", " @@ List.map string_of_args f.A.f_params in
  let use_list_helper ((_, id), b) = (if b then "&" else "") ^ id in
  let use_statement = match use_list with
    | [] -> ""
    | _ ->
      "use ("
      ^ (String.concat ", " @@ List.map use_list_helper use_list)
      ^ ") "
  in
  (* TODO: Pretty print body for closure as a default value *)
  let body = "NYI: default value closure body" in
  "function ("
  ^ args
  ^ ") "
  ^ use_statement
  ^ "{"
  ^ body
  ^ "}\\n"

and string_of_xml (_, id) attributes children =
  let p = Pos.none in
  let name = SU.Xhp.mangle id in
  let attributes = string_of_param_default_value @@
   (p, A.Array (
    List.map (fun (id, e) -> A.AFkvalue ((p, A.Id id), e)) attributes))
  in
  let children = string_of_param_default_value @@
   (p, A.Array (List.map (fun e -> A.AFvalue e) children))
  in
  "new "
  ^ name
  ^ "("
  ^ attributes
  ^ ", "
  ^ children
  ^ ", __FILE__, __LINE__)"

and string_of_param_default_value expr =
  let middle_aux e1 s e2 =
    let e1 = string_of_param_default_value e1 in
    let e2 = string_of_param_default_value e2 in
    e1 ^ s ^ e2
  in
  let expr = Ast_constant_folder.fold_expr
    Namespace_env.empty_with_default_popt expr in
  match snd expr with
  | A.Id (_, litstr)
  | A.Id_type_arguments ((_, litstr), _)
  | A.Lvar (_, litstr)
  | A.Float (_, litstr)
  | A.Int (_, litstr) -> litstr
  | A.String (_, litstr) -> "\\\"" ^ litstr ^ "\\\""
  | A.Null -> "NULL"
  | A.True -> "true"
  | A.False -> "false"
  (* For arrays and collections, we are making a concious decision to not
   * match HHMV has HHVM's emitter has inconsistencies in the pretty printer
   * https://fburl.com/tzom2qoe *)
  | A.Array afl ->
    "array(" ^ string_of_afield_list afl ^ ")"
  | A.Collection ((_, name), afl) when
    name = "vec" || name = "dict" || name = "keyset" ->
    name ^ "[" ^ string_of_afield_list afl ^ "]"
  | A.Collection ((_, name), afl) when
    name = "Set" || name = "Pair" || name = "Vector" || name = "Map" ||
    name = "ImmSet" || name = "ImmVector" || name = "ImmMap" ->
    "HH\\\\" ^ name ^ "{" ^ string_of_afield_list afl ^ "}"
  | A.Collection ((_, name), _) ->
    "NYI - Default value for an unknown collection - " ^ name
  | A.Shape fl ->
    let fl =
      List.map
        (fun (f_name, e) ->
          A.AFkvalue (shape_field_name_to_expr f_name, e))
        fl
    in
    string_of_param_default_value (fst expr, A.Array fl)
  | A.Binop (bop, e1, e2) ->
    let bop = string_of_bop bop in
    let e1 = string_of_param_default_value e1 in
    let e2 = string_of_param_default_value e2 in
    e1 ^ " " ^ bop ^ " " ^ e2
  | A.New (e, es, ues)
  | A.Call (e, es, ues) ->
    let e = String_utils.lstrip (string_of_param_default_value e) "\\\\" in
    let es = List.map string_of_param_default_value (es @ ues) in
    let prefix = match snd expr with A.New (_, _, _) -> "new " | _ -> "" in
    prefix
    ^ e
    ^ "("
    ^ String.concat ", " es
    ^ ")"
  | A.Class_get ((_, s1), (_, s2))
  | A.Class_const ((_, s1), (_, s2)) -> "\\\\" ^ s1 ^ "::" ^ s2
  | A.Unop (uop, e) -> begin
    let e = string_of_param_default_value e in
    match uop with
    | A.Upincr -> e ^ "++"
    | A.Updecr -> e ^ "--"
    | A.Usplat -> e
    | _ -> string_of_uop uop ^ e
  end
  | A.Obj_get (e1, e2, f) ->
    let e1 = string_of_param_default_value e1 in
    let e2 = string_of_param_default_value e2 in
    let f = match f with A.OG_nullthrows -> "->" | A.OG_nullsafe -> "?->" in
    e1 ^ f ^ e2
  | A.Clone e -> "clone " ^ string_of_param_default_value e
  | A.Array_get (e, eo) ->
    let e = string_of_param_default_value e in
    let eo = Option.value_map eo ~default:"" ~f:string_of_param_default_value in
    e ^ "[" ^ eo ^ "]"
  | A.String2 es ->
    String.concat " . " @@ List.map string_of_param_default_value es
  | A.Eif (cond, etrue, efalse) ->
    let cond = string_of_param_default_value cond in
    let etrue =
      Option.value_map etrue ~default:"" ~f:string_of_param_default_value
    in
    let efalse = string_of_param_default_value efalse in
    cond ^ " \\? " ^ etrue ^ " : " ^ efalse
  | A.Lvarvar (n, (_, s)) ->
    let prefix =
      String.init (2 * n) (fun x -> if x mod 2 = 0 then '$' else '{')
    in
    let suffix = String.make n '}' in
    prefix ^ s ^ suffix
  | A.Unsafeexpr e -> string_of_param_default_value e
  | A.Cast (h, e) ->
    let h = string_of_hint ~ns: false h in
    let e = string_of_param_default_value e in
    "(" ^ h ^ ")" ^ e
  | A.Pipe (e1, e2) -> middle_aux e1 " |> " e2
  | A.NullCoalesce (e1, e2) -> middle_aux e1 " \\?\\? " e2
  | A.InstanceOf (e1, e2) -> middle_aux e1 " instanceof " e2
  | A.Varray es ->
    string_of_param_default_value @@
     (Pos.none, A.Array (List.map (fun e -> A.AFvalue e) es))
  | A.Darray es ->
    string_of_param_default_value @@
     (Pos.none, A.Array (List.map (fun (e1, e2) -> A.AFkvalue (e1, e2)) es))
  | A.Import (fl, e) ->
    let fl = string_of_import_flavor fl in
    let e = string_of_param_default_value e in
    fl ^ " " ^ e
  | A.Xml (id, attributes, children) ->
    string_of_xml id attributes children
  | A.Efun (f, use_list) -> string_of_fun f use_list
  | A.Lfun _ ->
    failwith "expected Lfun to be converted to Efun during closure conversion"
  | A.Dollardollar
  | A.Yield _
  | A.Yield_break
  | A.Await _
  | A.List _
  | A.Expr_list _ -> failwith "illegal default value"

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
  ^ (if Hhas_param.is_reference p then "&" else "")
  ^ param_name
  ^ string_of_param_default_value_option param_default_value

let string_of_params ps =
  "(" ^ String.concat ", " (List.map string_of_param ps) ^ ")"

let add_indent buf indent = B.add_string buf (String.make indent ' ')
let add_indented_line buf indent str =
  add_indent buf indent;
  B.add_string buf str;
  B.add_string buf "\n"

let add_num_cls_ref_slots buf indent num_cls_ref_slots =
  if num_cls_ref_slots <> 0
  then add_indented_line buf indent
    (Printf.sprintf ".numclsrefslots %d;" num_cls_ref_slots)

let add_decl_vars buf indent decl_vars =
  if decl_vars <> []
  then add_indented_line buf indent
    (".declvars " ^ String.concat " " decl_vars ^ ";")

let add_num_iters buf indent num_iters =
  if num_iters <> 0
  then add_indented_line buf indent
    (Printf.sprintf ".numiters %d;" num_iters)

let add_body buf indent body =
  add_num_iters buf indent (Hhas_body.num_iters body);
  add_num_cls_ref_slots buf indent (Hhas_body.num_cls_ref_slots body);
  add_decl_vars buf indent (Hhas_body.decl_vars body);
  total_named_locals := List.length (Hhas_body.decl_vars body) +
    List.length (Hhas_body.params body);
  add_instruction_list buf indent
    (Instruction_sequence.instr_seq_to_list (Hhas_body.instrs body))

let function_attributes f =
  let user_attrs = Hhas_function.attributes f in
  let attrs = Emit_adata.attributes_to_strings user_attrs in
  let text = String.concat " " attrs in
  if text = "" then "" else "[" ^ text ^ "] "

let add_fun_def buf fun_def =
  let function_name = Hhas_function.name fun_def in
  let function_body = Hhas_function.body fun_def in
  let function_return_type = Hhas_body.return_type function_body in
  let function_params = Hhas_body.params function_body in
  let function_is_async = Hhas_function.is_async fun_def in
  let function_is_generator = Hhas_function.is_generator fun_def in
  let function_is_pair_generator = Hhas_function.is_pair_generator fun_def in
  B.add_string buf "\n.function ";
  B.add_string buf (function_attributes fun_def);
  B.add_string buf (string_of_type_info_option function_return_type);
  B.add_string buf (Hhbc_id.Function.to_raw_string function_name);
  B.add_string buf (string_of_params function_params);
  if function_is_generator then B.add_string buf " isGenerator";
  if function_is_async then B.add_string buf " isAsync";
  if function_is_pair_generator then B.add_string buf " isPairGenerator";
  B.add_string buf " {\n";
  add_body buf 2 function_body;
  B.add_string buf "}\n"

let method_attributes m =
  let user_attrs = Hhas_method.attributes m in
  let attrs = Emit_adata.attributes_to_strings user_attrs in
  let attrs = if Hhas_method.is_abstract m then "abstract" :: attrs else attrs in
  let attrs = if Hhas_method.is_final m then "final" :: attrs else attrs in
  let attrs = if Hhas_method.is_static m then "static" :: attrs else attrs in
  let attrs = if Hhas_method.is_public m then "public" :: attrs else attrs in
  let attrs = if Hhas_method.is_protected m then "protected" :: attrs else attrs in
  let attrs = if Hhas_method.is_private m then "private" :: attrs else attrs in
  let text = String.concat " " attrs in
  let text = if text = "" then "" else "[" ^ text ^ "] " in
  text

let add_method_def buf method_def =
  let method_name = Hhas_method.name method_def in
  let method_body = Hhas_method.body method_def in
  let method_return_type = Hhas_body.return_type method_body in
  let method_params = Hhas_body.params method_body in
  let method_is_async = Hhas_method.is_async method_def in
  let method_is_generator = Hhas_method.is_generator method_def in
  let method_is_pair_generator = Hhas_method.is_pair_generator method_def in
  let method_is_closure_body = Hhas_method.is_closure_body method_def in
  B.add_string buf "\n  .method ";
  B.add_string buf (method_attributes method_def);
  B.add_string buf (string_of_type_info_option method_return_type);
  B.add_string buf (Hhbc_id.Method.to_raw_string method_name);
  B.add_string buf (string_of_params method_params);
  if method_is_generator then B.add_string buf " isGenerator";
  if method_is_async then B.add_string buf " isAsync";
  if method_is_pair_generator then B.add_string buf " isPairGenerator";
  if method_is_closure_body then B.add_string buf " isClosureBody";
  B.add_string buf " {\n";
  add_body buf 4 method_body;
  B.add_string buf "  }"

let class_special_attributes c =
  let user_attrs = Hhas_class.attributes c in
  let attrs = Emit_adata.attributes_to_strings user_attrs in
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
      B.add_string buf (Hhbc_id.Class.to_raw_string name);
    end

let add_implements buf class_implements =
  match class_implements with
  | [] -> ()
  | _ ->
  begin
    B.add_string buf " implements (";
    B.add_string buf (String.concat " "
      (List.map Hhbc_id.Class.to_raw_string class_implements));
    B.add_string buf ")";
  end

let property_attributes p =
  let module P = Hhas_property in
  let attrs = [] in
  let attrs = if P.is_deep_init p then "deep_init" :: attrs else attrs in
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
  B.add_string buf (Hhbc_id.Prop.to_raw_string (Hhas_property.name property));
  B.add_string buf " =\n    ";
  let initial_value = Hhas_property.initial_value property in
  if Hhas_class.is_closure_class class_def
  || initial_value = Some Typed_value.Uninit
  then B.add_string buf "uninit;"
  else begin
    B.add_string buf "\"\"\"";
    let init = match initial_value with
      | None -> SS.str "N;"
      | Some value -> Emit_adata.adata_to_string_seq value
    in
    SS.add_string_from_seq buf init;
    B.add_string buf "\"\"\";"
  end

let add_constant buf c =
  let name = Hhas_constant.name c in
  let value = Hhas_constant.value c in
  B.add_string buf "\n  .const ";
  B.add_string buf name;
  B.add_string buf " = ";
  begin match value with
  | Typed_value.Uninit -> B.add_string buf "uninit"
  | _ -> B.add_string buf "\"\"\"";
  SS.add_string_from_seq buf @@ Emit_adata.adata_to_string_seq value;
  B.add_string buf "\"\"\""
  end;
  B.add_string buf ";"

let add_type_constant buf c =
  B.add_string buf "\n  .const ";
  B.add_string buf (Hhas_type_constant.name c);
  let initializer_t = Hhas_type_constant.initializer_t c in
  B.add_string buf " isType = \"\"\"";
  B.add_string buf @@ SS.seq_to_string @@
    Emit_adata.adata_to_string_seq initializer_t;
  B.add_string buf "\"\"\";"

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
  let class_name = Hhas_class.name class_def in
  (* TODO: user attribuqtes *)
  B.add_string buf "\n.class ";
  B.add_string buf (class_special_attributes class_def);
  B.add_string buf (Hhbc_id.Class.to_raw_string class_name);
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

let add_data_region_element buf argument =
  let b, num = should_create_adata_entry argument in
  if b then begin
    B.add_string buf ".adata A_";
    B.add_string buf @@ string_of_int num;
    B.add_string buf " = \"\"\"";
    SS.add_string_from_seq buf
      @@ Emit_adata.adata_to_string_seq argument;
    B.add_string buf "\"\"\";\n"
  end

let add_data_region buf top_level_body functions classes =
  let rec add_data_region_list buf instrs =
    List.iter (add_data_region_aux buf)
    (Instruction_sequence.instr_seq_to_list instrs)
  and add_data_region_aux buf = function
    | ILitConst (Array argument
                | Dict argument
                | Vec argument
                | Keyset argument
                ) ->
      add_data_region_element buf argument
    | _ -> ()
  and iter_aux_fun buf fun_def =
    let function_body = Hhas_function.body fun_def in
    let instrs = Hhas_body.instrs function_body in
    add_data_region_list buf instrs
  and iter_aux_class buf class_def =
    let methods = Hhas_class.methods class_def in
    List.iter (iter_aux_method buf) methods
  and iter_aux_method buf method_def =
    let method_body = Hhas_method.body method_def in
    let instrs = Hhas_body.instrs method_body in
    add_data_region_list buf instrs
  in
  add_data_region_list buf (Hhas_body.instrs top_level_body);
  List.iter (iter_aux_fun buf) functions;
  List.iter (iter_aux_class buf) classes;
  B.add_string buf "\n"

let add_top_level buf hhas_prog =
  B.add_string buf ".main {\n";
  add_body buf 2 (Hhas_program.main hhas_prog);
  B.add_string buf "}\n"

let add_typedef buf typedef =
  let name = Hhas_typedef.name typedef in
  let type_info = Hhas_typedef.type_info typedef in
  B.add_string buf "\n.alias ";
  B.add_string buf (Hhbc_id.Class.to_raw_string name);
  B.add_string buf (" = " ^ string_of_typedef_info type_info ^ ";")

let add_program buf hhas_prog =
  B.add_string buf "#starts here\n";
  let functions = Hhas_program.functions hhas_prog in
  let top_level_body = Hhas_program.main hhas_prog in
  let classes = Hhas_program.classes hhas_prog in
  add_data_region buf top_level_body functions classes;
  add_top_level buf hhas_prog;
  List.iter (add_fun_def buf) functions;
  List.iter (add_class_def buf) classes;
  List.iter (add_typedef buf) (Hhas_program.typedefs hhas_prog);
  B.add_string buf "\n#ends here\n"

let to_string hhas_prog =
  let buf = Buffer.create 1024 in
  class_counter := 0;
  add_program buf hhas_prog;
  B.contents buf
