(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Core_kernel
module Acc = Mutable_accumulator
module H = Hhbc_ast
module A = Ast
module SU = Hhbc_string_utils
module SN = Naming_special_names
module ULS = Unique_list_string
open H

(* Generic helpers *)
let sep pieces = String.concat ~sep:" " pieces

let indent_text = "  "

let string_of_optional_value f v =
  Option.value_map v
    ~default:""
    ~f:(fun s -> " " ^ (f s))

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
let string_of_class_num id =
  string_of_int id
let string_of_typedef_num id =
  string_of_int id
let string_of_pos pos =
  let {line_begin; line_end; col_begin; col_end} = pos in
  Printf.sprintf "%d:%d,%d:%d" line_begin col_begin line_end col_end
let string_of_span (line_begin, line_end) =
  Printf.sprintf "(%d,%d)" line_begin line_end

(* Naming convention for functions below:
 *   string_of_X converts an X to a string
 *   add_X takes a buffer and an X, and appends to the buffer
 *)
let string_of_basic instruction =
  match instruction with
    | Nop         -> "Nop"
    | EntryNop    -> "EntryNop"
    | PopC        -> "PopC"
    | PopV        -> "PopV"
    | PopU        -> "PopU"
    | Dup         -> "Dup"
    | Box         -> "Box"

let string_of_list_of_shape_fields sl =
  String.concat ~sep:" " @@ List.map ~f:SU.quote_string sl

let string_of_stack_index si = string_of_int si

let string_of_classref id = string_of_int id

let string_of_adata_id id = "@" ^ id

let string_of_param_id x =
  match x with
  | Param_unnamed i -> string_of_int i
  | Param_named s -> s

let string_of_param_num i = string_of_int i

let string_of_local_id x =
  match x with
  | Local.Unnamed i -> "_" ^ (string_of_int i)
  | Local.Named s -> s

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
    | Array id          -> sep ["Array"; string_of_adata_id id]
    | Dict id           -> sep ["Dict"; string_of_adata_id id]
    | Keyset id         -> sep ["Keyset"; string_of_adata_id id]
    | Vec id            -> sep ["Vec"; string_of_adata_id id]
    | TypedValue _tv    -> failwith "string_of_lit_const: TypedValue"
    | ColFromArray t    -> sep ["ColFromArray"; CollectionType.to_string t]
    | NewCol t          -> sep ["NewCol"; CollectionType.to_string t]
    | NewDictArray i    -> sep ["NewDictArray"; string_of_int i]
    | NewKeysetArray i  -> sep ["NewKeysetArray"; string_of_int i]
    | NewVecArray i     -> sep ["NewVecArray"; string_of_int i]
    | NewVArray i       -> sep ["NewVArray"; string_of_int i]
    | NewDArray i       -> sep ["NewDArray"; string_of_int i]
    | NewMixedArray i   -> sep ["NewMixedArray"; string_of_int i]
    | NewPackedArray i  -> sep ["NewPackedArray"; string_of_int i]
    | NewStructArray l  ->
      sep ["NewStructArray"; "<" ^ string_of_list_of_shape_fields l ^ ">"]
    | NewStructDArray l  ->
      sep ["NewStructDArray"; "<" ^ string_of_list_of_shape_fields l ^ ">"]
    | NewStructDict l  ->
      sep ["NewStructDict"; "<" ^ string_of_list_of_shape_fields l ^ ">"]
    | NewPair -> "NewPair"
    | NewRecord (cid, l) ->
      sep ["NewRecord"; string_of_class_id cid; "<" ^ string_of_list_of_shape_fields l ^ ">"]
    | ClsCns (cnsid, cr) ->
      sep ["ClsCns"; string_of_const_id cnsid; string_of_classref cr]
    | ClsCnsD (cnsid, cid) ->
      sep ["ClsCnsD"; string_of_const_id cnsid; string_of_class_id cid]
    | File -> "File"
    | Dir -> "Dir"
    | NullUninit -> "NullUninit"
    | Method -> "Method"
    | NewArray n -> sep ["NewArray"; string_of_int n]
    | NewLikeArrayL (id, n) ->
      sep ["NewLikeArrayL"; string_of_local_id id; string_of_int n]
    | CnsE cnsid -> sep ["CnsE"; string_of_const_id cnsid]


let string_of_typestruct_resolve_op = function
  | Resolve -> "Resolve"
  | DontResolve -> "DontResolve"

let string_of_has_generics_op = function
  | NoGenerics -> "NoGenerics"
  | HasGenerics -> "HasGenerics"
  | MaybeGenerics -> "MaybeGenerics"

let string_of_operator instruction =
  match instruction with
    | Concat -> "Concat"
    | ConcatN n -> sep ["ConcatN"; string_of_int n]
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
    | CastVArray -> "CastVArray"
    | CastDArray -> "CastDArray"
    | InstanceOf -> "InstanceOf"
    | InstanceOfD id -> sep ["InstanceOfD"; string_of_class_id id]
    | IsLateBoundCls -> "IsLateBoundCls"
    | IsTypeStructC op ->
      sep ["IsTypeStructC"; string_of_typestruct_resolve_op op]
    | AsTypeStructC op ->
      sep ["AsTypeStructC"; string_of_typestruct_resolve_op op]
    | CombineAndResolveTypeStruct n ->
      sep ["CombineAndResolveTypeStruct"; string_of_int n]
    | Print -> "Print"
    | Clone -> "Clone"
    | H.Exit -> "Exit"
    | ResolveFunc id -> sep ["ResolveFunc"; string_of_function_id id]
    | ResolveObjMethod -> sep ["ResolveObjMethod"]
    | ResolveClsMethod -> sep ["ResolveClsMethod"]
    | Fatal op -> sep ["Fatal"; FatalOp.to_string op]

let string_of_get x =
  match x with
  | CGetL id -> sep ["CGetL"; string_of_local_id id]
  | CGetQuietL id -> sep ["CGetQuietL"; string_of_local_id id]
  | CGetL2 id -> sep ["CGetL2"; string_of_local_id id]
  | CUGetL id -> sep ["CUGetL"; string_of_local_id id]
  | PushL id -> sep ["PushL"; string_of_local_id id]
  | CGetG -> "CGetG"
  | CGetQuietG -> "CGetQuietG"
  | CGetS id -> sep ["CGetS"; string_of_classref id]
  | VGetS id -> sep ["VGetS"; string_of_classref id]
  | VGetL id -> sep ["VGetL"; string_of_local_id id]
  | ClsRefGetC cr -> sep ["ClsRefGetC"; string_of_int cr]
  | ClsRefGetTS cr -> sep ["ClsRefGetTS"; string_of_int cr]

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

let string_of_setrange_op = function
  | Forward -> "Forward"
  | Reverse -> "Reverse"

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
  | OpRes -> "Res"
  | OpScalar -> "Scalar"
  | OpVec -> "Vec"
  | OpDict -> "Dict"
  | OpKeyset -> "Keyset"
  | OpArrLike -> "ArrLike"
  | OpVArray -> "VArray"
  | OpDArray -> "DArray"
  | OpClsMeth -> "ClsMeth"

let string_of_initprop_op op =
  match op with
  | NonStatic -> "NonStatic"
  | Static -> "Static"

let string_of_mutator x =
  match x with
  | SetL id -> sep ["SetL"; string_of_local_id id]
  | PopL id -> sep ["PopL"; string_of_local_id id]
  | SetG -> "SetG"
  | SetS id -> sep ["SetS"; string_of_classref id]
  | SetOpL (id, op) ->
    sep ["SetOpL"; string_of_local_id id; string_of_eq_op op]
  | SetOpG op -> sep ["SetOpG"; string_of_eq_op op]
  | SetOpS (op, id) -> sep ["SetOpS"; string_of_eq_op op; string_of_classref id]
  | IncDecL (id, op) ->
    sep ["IncDecL"; string_of_local_id id; string_of_incdec_op op]
  | IncDecG op -> sep ["IncDecG"; string_of_incdec_op op]
  | IncDecS (op, id) ->
    sep ["IncDecS"; string_of_incdec_op op; string_of_classref id]
  | UnsetL id -> sep ["UnsetL"; string_of_local_id id]
  | UnsetG -> "UnsetG"
  | CheckProp id -> sep ["CheckProp"; string_of_prop_id id]
  | InitProp (id, op) -> sep ["InitProp"; string_of_prop_id id;
      string_of_initprop_op op]

let string_of_label = function
  | Label.Regular id -> "L" ^ (string_of_int id)
  | Label.DefaultArg id -> "DV" ^ (string_of_int id)
  | Label.Named id -> id

let string_of_optional_label opt_label =
  match opt_label with
  | None -> "-"
  | Some label -> string_of_label label

let string_of_fcall_flags fl =
  let fl = [
    if fl.has_unpack then "Unpack" else "";
    if fl.supports_async_eager_return then "SupportsAER" else "";
  ] in
  "<" ^ (String.concat ~sep:" " @@ List.filter ~f:(fun f -> f <> "") fl) ^ ">"

let string_of_list_of_bools l =
  let bool_to_str b = if b then "1" else "0" in
  "\"" ^ (String.concat ~sep:"" (List.map ~f:bool_to_str l)) ^ "\""

let string_of_fcall_args fcall_args =
  let flags, num_args, num_rets, by_refs, async_eager_label = fcall_args in
  sep [
    string_of_fcall_flags flags;
    string_of_int num_args;
    string_of_int num_rets;
    string_of_list_of_bools by_refs;
    string_of_optional_label async_eager_label
  ]

let string_of_switch_kind = function
  | H.Unbounded -> "Unbounded"
  | H.Bounded -> "Bounded"

let string_of_switch kind base labels =
  let kind = string_of_switch_kind kind in
  let labels = String.concat ~sep:" " @@ List.map ~f:string_of_label labels in
  Printf.sprintf "Switch %s %d <%s>" kind base labels

let string_of_sswitch cases =
  let revcases = List.rev cases in
  match revcases with
   | [] -> failwith "sswitch should have at least one case"
   | (_dummystring, lastlabel) :: revrest ->
       let reststring =
         String.concat  ~sep:" " @@
         List.rev_map ~f:(function (s,l) -> SU.quote_string s ^ ":" ^ string_of_label l) revrest in
       let laststring = "-:" ^ string_of_label lastlabel in
         Printf.sprintf "SSwitch <%s %s>" reststring laststring

let string_of_control_flow instruction =
  match instruction with
  | Jmp l -> "Jmp " ^ string_of_label l
  | JmpNS l -> "JmpNS " ^ string_of_label l
  | JmpZ l -> "JmpZ " ^ string_of_label l
  | JmpNZ l -> "JmpNZ " ^ string_of_label l
  | RetC -> "RetC"
  | RetCSuspended -> "RetCSuspended"
  | RetM p -> "RetM " ^ string_of_int p
  | Throw -> "Throw"
  | Switch (kind, base, labels) -> string_of_switch kind base labels
  | SSwitch cases -> string_of_sswitch cases

let string_of_iterator_id i = Iterator.to_string i
let string_of_null_flavor nf =
  match nf with
  | Ast.OG_nullthrows -> "NullThrows"
  | Ast.OG_nullsafe -> "NullSafe"

let string_of_class_kind ck =
  match ck with
  | KClass -> "Class"
  | KInterface -> "Interface"
  | KTrait -> "Trait"

let string_of_isset instruction =
  match instruction with
  | IssetC -> "IssetC"
  | IssetL id -> "IssetL " ^ string_of_local_id id
  | IssetG -> "IssetG"
  | IssetS cls -> "IssetS " ^ string_of_int cls
  | EmptyL id -> "EmptyL " ^ string_of_local_id id
  | EmptyG -> "EmptyG"
  | EmptyS cls -> "EmptyS " ^ string_of_int cls
  | IsTypeC op -> "IsTypeC " ^ string_of_istype_op op
  | IsTypeL (id, op) ->
    "IsTypeL " ^ string_of_local_id id ^ " " ^ string_of_istype_op op

let string_of_base x =
  match x with
  | BaseGC (si, m) ->
    sep ["BaseGC"; string_of_stack_index si; MemberOpMode.to_string m]
  | BaseGL (id, m) ->
    sep ["BaseGL"; string_of_local_id id; MemberOpMode.to_string m]
  | BaseSC (si, id, m) ->
    sep ["BaseSC";
         string_of_stack_index si; string_of_classref id; MemberOpMode.to_string m]
  | BaseL (lid, m) ->
    sep ["BaseL"; string_of_local_id lid; MemberOpMode.to_string m]
  | BaseC (si, m) ->
    sep ["BaseC"; string_of_stack_index si; MemberOpMode.to_string m]
  | BaseH ->
    "BaseH"
  | Dim (m, mk) ->
    sep ["Dim"; MemberOpMode.to_string m; string_of_member_key mk]

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
  | SetM (i, mk) ->
    sep ["SetM";
      string_of_param_num i; string_of_member_key mk]
  | SetOpM (i, op, mk) ->
    sep ["SetOpM";
      string_of_param_num i; string_of_eq_op op; string_of_member_key mk]
  | IncDecM (i, op, mk) ->
    sep ["IncDecM";
      string_of_param_num i; string_of_incdec_op op; string_of_member_key mk]
  | SetRangeM (i, op, s) ->
    sep ["SetRangeM";
      string_of_int i; string_of_setrange_op op; string_of_int s]

(*
| IncDecM of num_params * incdec_op * MemberKey.t
| SetOpM of num_params  * eq_op * MemberKey.t
*)

let string_of_param_locations pl =
  if List.length pl = 0 then "" else
  "<" ^ (String.concat ~sep:", " (List.map ~f:string_of_int pl)) ^ ">"

let string_of_call instruction =
  match instruction with
  | FPushFunc (n, pl) ->
    sep ["FPushFunc"; string_of_int n; string_of_param_locations pl]
  | FPushFuncD (n, id) ->
    sep ["FPushFuncD"; string_of_int n; string_of_function_id id]
  | FPushObjMethod (n, nf, pl) ->
    sep ["FPushObjMethod"; string_of_int n; string_of_null_flavor nf; string_of_param_locations pl]
  | FPushObjMethodD (n, id, nf) ->
    sep ["FPushObjMethodD";
      string_of_int n; string_of_method_id id; string_of_null_flavor nf]
  | FPushObjMethodRD (n, id, nf) ->
    sep ["FPushObjMethodRD";
      string_of_int n; string_of_method_id id; string_of_null_flavor nf]
  | FPushClsMethod (n, id, pl) ->
    sep ["FPushClsMethod"; string_of_int n; string_of_classref id; string_of_param_locations pl]
  | FPushClsMethodD (n, id, cid) ->
    sep ["FPushClsMethodD";
      string_of_int n;
      string_of_method_id id; string_of_class_id cid]
  | FPushClsMethodS (n, r) ->
     sep ["FPushClsMethodS";
          string_of_int n;
          SpecialClsRef.to_string r]
  | FPushClsMethodSD (n, r, id) ->
     sep ["FPushClsMethodSD";
          string_of_int n;
          SpecialClsRef.to_string r;
          string_of_method_id id]
  | NewObj (id, op) ->
    sep ["NewObj"; string_of_int id; string_of_has_generics_op op]
  | NewObjD cid ->
    sep ["NewObjD"; string_of_class_id cid]
  | NewObjS r ->
    sep ["NewObjS"; SpecialClsRef.to_string r]
  | FPushCtor n ->
    sep ["FPushCtor"; string_of_int n]
  | FCall fcall_args ->
    sep ["FCall"; string_of_fcall_args fcall_args; "\"\""; "\"\"" ]
  | FCallBuiltin (n1, n2, id) ->
    sep ["FCallBuiltin"; string_of_int n1; string_of_int n2; SU.quote_string id]

let string_of_barethis_op i =
  match i with
  | Notice -> "Notice"
  | NoNotice -> "NoNotice"
  | NeverNull -> "NeverNull"

let string_of_op_silence op =
  match op with
  | Start -> "Start"
  | End -> "End"

let string_of_misc instruction =
  match instruction with
    | This -> "This"
    | BareThis op -> sep ["BareThis"; string_of_barethis_op op]
    | Self id -> sep ["Self"; string_of_classref id]
    | Parent id -> sep ["Parent"; string_of_classref id]
    | LateBoundCls id -> sep ["LateBoundCls"; string_of_classref id]
    | ClsRefName id -> sep ["ClsRefName"; string_of_classref id]
    | ReifiedName name -> sep ["ReifiedName"; SU.quote_string name]
    | RecordReifiedGeneric-> "RecordReifiedGeneric"
    | CheckReifiedGenericMismatch -> "CheckReifiedGenericMismatch"
    | VerifyParamType id -> sep ["VerifyParamType"; string_of_param_id id]
    | VerifyParamTypeTS id -> sep ["VerifyParamTypeTS"; string_of_param_id id]
    | VerifyOutType id -> sep ["VerifyOutType"; string_of_param_id id]
    | VerifyRetTypeC -> "VerifyRetTypeC"
    | VerifyRetTypeTS -> "VerifyRetTypeTS"
    | ChainFaults -> "ChainFaults"
    | CheckThis -> "CheckThis"
    | CGetCUNop -> "CGetCUNop"
    | UGetCUNop -> "UGetCUNop"
    | MemoGet (label, Some (Local.Unnamed first, local_count)) ->
      Printf.sprintf "MemoGet %s L:%d+%d"
                     (string_of_label label) first local_count
    | MemoGet (label, None) ->
      Printf.sprintf "MemoGet %s L:0+0" (string_of_label label)
    | MemoGet _ -> failwith "MemoGet needs an unnamed local"
    | MemoGetEager (label1, label2, Some (Local.Unnamed first, local_count)) ->
      Printf.sprintf "MemoGetEager %s %s L:%d+%d"
                     (string_of_label label1) (string_of_label label2) first local_count
    | MemoGetEager (label1, label2, None) ->
      Printf.sprintf "MemoGetEager %s %s L:0+0" (string_of_label label1) (string_of_label label2)
    | MemoGetEager _ -> failwith "MemoGetEager needs an unnamed local"
    | MemoSet (Some (Local.Unnamed first, local_count)) ->
       Printf.sprintf "MemoSet L:%d+%d" first local_count
    | MemoSet None ->
       Printf.sprintf "MemoSet L:0+0"
    | MemoSet _ -> failwith "MemoSet needs an unnamed local"
    | MemoSetEager (Some (Local.Unnamed first, local_count)) ->
       Printf.sprintf "MemoSetEager L:%d+%d" first local_count
    | MemoSetEager None ->
       Printf.sprintf "MemoSetEager L:0+0"
    | MemoSetEager _ -> failwith "MemoSetEager needs an unnamed local"
    | GetMemoKeyL local ->
      sep ["GetMemoKeyL"; string_of_local_id local]
    | CreateCl (n, cid) ->
      sep ["CreateCl"; string_of_int n; string_of_int cid]
    | Idx -> "Idx"
    | ArrayIdx -> "ArrayIdx"
    | InitThisLoc id -> sep ["InitThisLoc"; string_of_local_id id]
    | FuncNumArgs -> "FuncNumArgs"
    | AKExists -> "AKExists"
    | OODeclExists ck -> sep ["OODeclExists"; string_of_class_kind ck]
    | Silence (local, op) ->
      sep ["Silence"; string_of_local_id local; string_of_op_silence op]
    | AssertRATL (local, s) ->
      sep ["AssertRATL"; string_of_local_id local; s]
    | AssertRATStk (n, s) ->
      sep ["AssertRATStk"; string_of_int n; s]
    | NativeImpl -> "NativeImpl"
    | BreakTraceHint -> "BreakTraceHint"

let iterator_instruction_name_prefix instruction =
  let iterator_instruction_name =
    match instruction with
    | IterInit _ -> "IterInit"
    | LIterInit _ -> "LIterInit"
    | IterInitK _ -> "IterInitK"
    | LIterInitK _ -> "LIterInitK"
    | IterNext _ -> "IterNext"
    | LIterNext _ -> "LIterNext"
    | IterNextK _ -> "IterNextK"
    | LIterNextK _ -> "LIterNextK"
    | IterFree _ -> "IterFree"
    | LIterFree _ -> "LIterFree"
    | _ -> failwith "invalid iterator instruction"
  in
  iterator_instruction_name ^ " "

let string_of_iterator instruction =
  match instruction with
  | IterInit (id, label, value) ->
    (iterator_instruction_name_prefix instruction) ^
    (string_of_iterator_id id) ^ " " ^
    (string_of_label label) ^ " " ^
      (string_of_local_id value)
  | LIterInit (id, base, label, value) ->
     (iterator_instruction_name_prefix instruction) ^
     (string_of_iterator_id id) ^ " " ^
     (string_of_local_id base) ^ " " ^
     (string_of_label label) ^ " " ^
     (string_of_local_id value)
  | IterInitK (id, label, key, value) ->
    (iterator_instruction_name_prefix instruction) ^
    (string_of_iterator_id id) ^ " " ^
    (string_of_label label) ^ " " ^
    (string_of_local_id key) ^ " " ^
    (string_of_local_id value)
  | LIterInitK (id, base, label, key, value) ->
     (iterator_instruction_name_prefix instruction) ^
     (string_of_iterator_id id) ^ " " ^
     (string_of_local_id base) ^ " " ^
     (string_of_label label) ^ " " ^
     (string_of_local_id key) ^ " " ^
     (string_of_local_id value)
  | IterNext (id, label, value) ->
    (iterator_instruction_name_prefix instruction) ^
    (string_of_iterator_id id) ^ " " ^
    (string_of_label label) ^ " " ^
    (string_of_local_id value)
  | LIterNext (id, base, label, value) ->
     (iterator_instruction_name_prefix instruction) ^
     (string_of_iterator_id id) ^ " " ^
     (string_of_local_id base) ^ " " ^
     (string_of_label label) ^ " " ^
     (string_of_local_id value)
  | IterNextK (id, label, key, value) ->
    (iterator_instruction_name_prefix instruction) ^
    (string_of_iterator_id id) ^ " " ^
    (string_of_label label) ^ " " ^
    (string_of_local_id key) ^ " " ^
    (string_of_local_id value)
  | LIterNextK (id, base, label, key, value) ->
     (iterator_instruction_name_prefix instruction) ^
     (string_of_iterator_id id) ^ " " ^
     (string_of_local_id base) ^ " " ^
     (string_of_label label) ^ " " ^
     (string_of_local_id key) ^ " " ^
     (string_of_local_id value)
  | IterFree id ->
    (iterator_instruction_name_prefix instruction) ^
      (string_of_iterator_id id)
  | LIterFree (id, base) ->
     (iterator_instruction_name_prefix instruction) ^
     (string_of_iterator_id id) ^ " " ^
     (string_of_local_id base)
  | IterBreak (label, iterlist) ->
      let map_item (kind, id) =
        let id = string_of_iterator_id id in
        match kind with
        | Iter -> "(Iter) " ^ id
        | LIter -> "(LIter) " ^ id
      in
      let values =
        String.concat ~sep:", " (List.rev_map ~f:map_item iterlist) in
      "IterBreak " ^ (string_of_label label) ^ " <" ^ values ^ ">"

let string_of_try instruction =
  match instruction with
  | TryCatchBegin -> ".try {"
  | TryCatchMiddle -> "} .catch {"
  | TryCatchEnd -> "}"

let string_of_async = function
  | Await -> "Await"
  | WHResult -> "WHResult"
  | AwaitAll (Some (Local.Unnamed local, count)) ->
    Printf.sprintf "AwaitAll L:%d+%d" local count
  | AwaitAll None ->
    Printf.sprintf "AwaitAll L:0+0"
  | AwaitAll _ -> failwith "AwaitAll needs an unnamed local"

let string_of_generator = function
  | CreateCont -> "CreateCont"
  | ContEnter -> "ContEnter"
  | ContRaise -> "ContRaise"
  | Yield -> "Yield"
  | YieldK -> "YieldK"
  | ContCheck IgnoreStarted -> "ContCheck IgnoreStarted"
  | ContCheck CheckStarted -> "ContCheck CheckStarted"
  | ContValid -> "ContValid"
  | ContKey -> "ContKey"
  | ContGetReturn -> "ContGetReturn"
  | ContCurrent -> "ContCurrent"

let string_of_include_eval_define = function
  | H.Incl -> "Incl"
  | InclOnce -> "InclOnce"
  | Req -> "Req"
  | ReqOnce -> "ReqOnce"
  | ReqDoc -> "ReqDoc"
  | Eval -> "Eval"
  | AliasCls (c1, c2) ->
    sep ["AliasCls"; SU.quote_string c1; SU.quote_string c2]
  | DefCls id -> sep ["DefCls"; string_of_class_num id]
  | DefClsNop id -> sep ["DefClsNop"; string_of_class_num id]
  | DefRecord id -> sep ["DefRecord"; string_of_class_num id]
  | DefCns id -> sep ["DefCns"; string_of_const_id id]
  | DefTypeAlias id -> sep ["DefTypeAlias"; string_of_typedef_num id]

let string_of_free_iterator = function
  | IgnoreIter -> "IgnoreIter"
  | FreeIter -> "FreeIter"

let string_of_gen_delegation = function
  | ContAssignDelegate i -> sep ["ContAssignDelegate"; string_of_iterator_id i]
  | ContEnterDelegate -> "ContEnterDelegate"
  | YieldFromDelegate (i, l) ->
    sep ["YieldFromDelegate"; string_of_iterator_id i; string_of_label l]
  | ContUnsetDelegate (free, i) ->
    sep ["ContUnsetDelegate";
         string_of_free_iterator free;
         string_of_iterator_id i]

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
  | ISrcLoc              p -> ".srcloc " ^ string_of_pos p ^ ";"
  | IAsync               i -> string_of_async i
  | IGenerator           i -> string_of_generator i
  | IIncludeEvalDefine   i -> string_of_include_eval_define i
  | IGenDelegation       i -> string_of_gen_delegation i
  | _ -> failwith "invalid instruction" in
  s ^ "\n"

let adjusted_indent instruction indent =
  match instruction with
  | IComment _ -> 0
  | ILabel _
  | ITry TryCatchMiddle
  | ITry TryCatchEnd -> indent - 2
  | _ -> indent

let new_indent instruction indent =
  match instruction with
  | ITry TryCatchBegin -> indent + 2
  | ITry TryCatchEnd -> indent - 2
  | _ -> indent

let add_instruction_list buffer indent instructions =
  let rec aux instructions indent =
    match instructions with
    | [] -> ()
    | ISpecialFlow _ :: t ->
      let fatal =
        Emit_fatal.emit_fatal_runtime Pos.none "Cannot break/continue 1 level"
      in
      let fatal = Instruction_sequence.instr_seq_to_list fatal in
      aux fatal indent;
      aux t indent
    | instruction :: t ->
      begin
      let actual_indent = adjusted_indent instruction indent in
      Acc.add buffer (String.make actual_indent ' ');
      Acc.add buffer (string_of_instruction instruction);
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
  let flag_strs = List.map ~f:Hhas_type_constraint.string_of_flag flags in
  let flags_text = String.concat ~sep:" " flag_strs in
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
  let flags =
    List.filter ~f:(fun f -> f = Hhas_type_constraint.Nullable) flags in
  let flags_text = string_of_type_flags flags in
    "<" ^ SU.quote_string (Option.value ~default:"" name)
    ^ " " ^ flags_text ^ " >"

let string_of_type_info_option tio =
  match tio with
  | None -> ""
  | Some ti -> string_of_type_info ti ^ " "

type default_value_printing_env = {
  codegen_env      : Emit_env.t option;
  in_xhp: bool;
}

let rec string_of_afield ~env = function
  | A.AFvalue e ->
    string_of_param_default_value ~env e
  | A.AFkvalue (k, v) ->
    string_of_param_default_value ~env k ^
    " => " ^ string_of_param_default_value ~env v

and string_of_afield_list ~env afl =
  if List.length afl = 0
  then ""
  else String.concat ~sep:", " @@ List.map ~f:(string_of_afield ~env) afl

and shape_field_name_to_expr = function
  | A.SFlit_int (pos, s) -> (pos, A.Int s)
  | A.SFlit_str (pos, s)
  | A.SFclass_const (_, (pos, s)) -> (pos, A.String s)

and string_of_bop = function
  | A.Plus -> "+"
  | A.Minus -> "-"
  | A.Star -> "*"
  | A.Slash -> "/"
  | A.Eqeq -> "=="
  | A.Eqeqeq -> "==="
  | A.Starstar -> "**"
  | A.Eq None -> "="
  | A.Eq (Some bop) -> "=" ^ string_of_bop bop
  | A.Ampamp -> "&&"
  | A.Barbar -> "||"
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
  | A.LogXor -> "xor"
  | A.Diff -> "!="
  | A.Diff2 -> "!=="
  | A.QuestionQuestion -> "\\?\\?"

and string_of_uop = function
  | A.Utild -> "~"
  | A.Unot -> "!"
  | A.Uplus -> "+"
  | A.Uminus -> "-"
  | A.Uincr -> "++"
  | A.Udecr -> "--"
  | A.Uref -> "&"
  | A.Usilence -> "@"
  | A.Upincr
  | A.Updecr
    -> failwith "string_of_uop - should have been captures earlier"

and string_of_hint ~ns h =
  let h =
    Emit_type_hint.fmt_hint
      ~tparams:[]
      ~namespace:Namespace_env.empty_with_default_popt
      h
  in
  let h = if ns then h else SU.strip_ns h in
  Php_escaping.escape h

and string_of_import_flavor = function
  | A.Include -> "include"
  | A.Require -> "require"
  | A.IncludeOnce -> "include_once"
  | A.RequireOnce -> "require_once"

and string_of_is_variadic b =
  if b then "..." else ""
and string_of_is_reference b =
  if b then "&" else ""

and string_of_fun ~env f use_list =
  let string_of_args p =
    match snd @@ p.A.param_id with
    | "" | "..." -> None
    | name ->
    let inout = if p.A.param_callconv = Some A.Pinout then "inout " else "" in
    let hint =
      Option.value_map p.A.param_hint ~default:"" ~f:(string_of_hint ~ns:true)
    in
    let default_val =
      Option.value_map
        p.A.param_expr
        ~default:""
        ~f:(fun e -> " = " ^ (string_of_param_default_value ~env e)) in
    let param_text =
      inout ^ string_of_is_variadic p.A.param_is_variadic ^ hint in
    let param_text =
      if String.length param_text = 0
      then param_text
      else param_text ^ " " in
    let param_text =
      param_text ^ string_of_is_reference p.A.param_is_reference ^ name in
    Some (param_text ^ default_val)
  in
  let args = String.concat ~sep:", " @@ List.filter_map ~f:string_of_args f.A.f_params in
  let use_statement = match use_list with
    | [] -> ""
    | _ ->
      "use ("
      ^ (String.concat ~sep:", " @@ List.map ~f:snd use_list)
      ^ ") "
  in
  (if f.A.f_static then "static " else "")
  ^ (if f.A.f_fun_kind = A.FAsync || f.A.f_fun_kind = A.FAsyncGenerator then "async " else "")
  ^ "function ("
  ^ args
  ^ ") "
  ^ use_statement
  ^ (string_of_statement ~env ~indent:"" (Pos.none, A.Block f.A.f_body))

and string_of_optional_expr ~env e =
  string_of_optional_value (string_of_expression ~env) e

and string_of_block_ ~env ~start_indent ~block_indent ~end_indent block =
  let lines =
    (String.concat ~sep:"" @@
      List.map ~f:(string_of_statement ~env ~indent:block_indent) block) in
  start_indent ^ "{\\n" ^ lines ^ end_indent ^ "}\\n"

and string_of_block ~env ~indent (block:A.stmt list) =
  match block with
  | [] | [_, A.Noop] -> ""
  | [(_, A.Block ([_] as block))]
  | (_::_::_ as block)->
    string_of_block_
      ~env
      ~start_indent:""
      ~block_indent:(indent ^ indent_text)
      ~end_indent:indent
      block
  | [stmt] ->
    string_of_statement ~env ~indent:"" stmt

and string_of_statement ~env ~indent ((_, stmt_) : A.stmt) =
  let text, is_single_line =
    match stmt_ with
    | A.Return e ->
      "return" ^ (string_of_optional_expr ~env e), true
    | A.Expr e ->
      string_of_expression ~env e, true
    | A.Break level_opt ->
      "break" ^ (string_of_optional_expr ~env level_opt), true
    | A.Continue level_opt ->
      "continue" ^ (string_of_optional_expr ~env level_opt), true
    | A.Throw e ->
      "throw " ^ (string_of_expression ~env e), true
    | A.Block block ->
      string_of_block_
        ~env
        ~start_indent:indent
        ~block_indent:(indent ^ indent_text)
        ~end_indent:indent
        block,
      false
    | A.While (cond, body) ->
      let header_text =
        indent ^ "while (" ^ (string_of_expression ~env cond) ^ ") " in
      let body_text =
        string_of_block
        ~env
        ~indent
        body in
      header_text ^ body_text, false

    | A.If (cond, then_block, else_block) ->
      let header_text =
        indent ^ "if (" ^ (string_of_expression ~env cond) ^ ") " in
      let then_text = string_of_block
        ~env
        ~indent
        then_block in
      let else_text = string_of_block
        ~env
        ~indent
        else_block in
      header_text ^ then_text ^
      (if String.length else_text <> 0 then " else " ^ else_text else ""),
      false
    | A.Noop -> "", false
    | _ -> (* TODO(T29869930) *) "NYI: Default value printing", false in
  let text =
    if is_single_line then indent ^ text ^ ";\\n"
    else text in
  text

and string_of_expression ~env e =
  string_of_param_default_value ~env e

and string_of_xml ~env (_, id) attributes children =
  let env = { env with in_xhp = true } in
  let p = Pos.none in
  let name = SU.Xhp.mangle id in
  let _, attributes =
    List.fold_right ~f:(string_of_xhp_attr p) attributes ~init:(0, [])
  in
  let attributes = string_of_param_default_value ~env (p, A.Darray (None, attributes)) in
  let children = string_of_param_default_value ~env
   (p, A.Varray (None, children))
  in
  "new "
  ^ name
  ^ "("
  ^ attributes
  ^ ", "
  ^ children
  ^ ", __FILE__, __LINE__)"

and string_of_xhp_attr p attr (spread_id, attrs) = match attr with
  | A.Xhp_simple ((_, s), e) -> (spread_id, ((p, A.String s), e)::attrs)
  | A.Xhp_spread e ->
    let s = "...$" ^ (string_of_int spread_id) in
    (spread_id + 1, ((p, A.String s), e)::attrs)

and string_of_param_default_value ~env expr =
  let p = Pos.none in
  let middle_aux e1 s e2 =
    let e1 = string_of_param_default_value ~env e1 in
    let e2 = string_of_param_default_value ~env e2 in
    e1 ^ s ^ e2
  in
  let fmt_class_name ~is_class_constant cn =
    let cn = if SU.Xhp.is_xhp (Utils.strip_ns cn)
    then SU.Xhp.mangle cn else cn in
    let cn = (Php_escaping.escape (SU.strip_global_ns cn)) in
    if is_class_constant then "\\\\" ^ cn else cn in
  let get_special_class_name ~env ~is_class_constant id =
    let scope = match env with
      | None -> Ast_scope.Scope.toplevel
      | Some env -> Emit_env.get_scope env in
    let module ACE = Ast_class_expr in
    let e =
      let p0 = Pos.none in
      (p0, (A.Id (p0, id))) in
    fmt_class_name ~is_class_constant @@
      match ACE.expr_to_class_expr ~check_traits:true ~resolve_self:true scope e with
      | ACE.Class_id (_, name) -> name
      | _ -> id
  in
  let get_class_name_from_id ~env ~should_format ~is_class_constant id =
    if id = SN.Classes.cSelf || id = SN.Classes.cParent || id = SN.Classes.cStatic
    then get_special_class_name ~env ~is_class_constant id
    else
    let id =
      match env with
      | Some env ->
        Hhbc_id.Class.to_raw_string @@
          Hhbc_id.Class.elaborate_id
            (Emit_env.get_namespace env) (p, id)
      | _ -> id
    in
    if should_format then fmt_class_name ~is_class_constant id else id
  in
  let handle_possible_colon_colon_class_expr ~env ~is_array_get = function
    | _, A.Class_const ((_, A.Id (p, s1)), (_, s2))
      when SU.is_class s2 && not
        (SU.is_self s1 || SU.is_parent s1 || SU.is_static s1) ->

      let s1 = get_class_name_from_id
        ~env:env.codegen_env ~should_format:false ~is_class_constant:false s1 in
      let e =
        (fst expr, if is_array_get then A.Id (p, s1) else A.String s1) in
      Some (string_of_param_default_value ~env e)
    | _ -> None
  in
  let escape_char_for_printing = function
    | '\\' | '$' | '"' -> "\\\\"
    | '\n' | '\r' | '\t' -> "\\"
    | c when not (Php_escaping.is_lit_printable c) -> "\\"
    | _ -> ""
  in
  let escape_fn c = escape_char_for_printing c ^ Php_escaping.escape_char c in
  match snd expr with
  | A.Id (p, id) ->
    let id = match env.codegen_env with
      | Some env when SU.has_ns id ->
        let id =
          Hhbc_id.Const.elaborate_id
            (Emit_env.get_namespace env) (p, id)
        in
        "\\" ^ Hhbc_id.Const.to_raw_string id
      | _ -> id
    in
    Php_escaping.escape id
  | A.Lvar (_, litstr) -> Php_escaping.escape litstr
  | A.Float litstr -> SU.Float.with_scientific_notation litstr
  | A.Int litstr -> SU.Integer.to_decimal litstr
  | A.String litstr ->
    SU.quote_string_with_escape ~f:escape_fn litstr
  | A.Null -> "NULL"
  | A.True -> "true"
  | A.False -> "false"
  (* For arrays and collections, we are making a conscious decision to not
   * match HHMV has HHVM's emitter has inconsistencies in the pretty printer
   * https://fburl.com/tzom2qoe *)
  | A.Array afl ->
    "array(" ^ string_of_afield_list ~env afl ^ ")"
  | A.Collection ((_, name), _, afl) when
    name = "vec" || name = "dict" || name = "keyset" ->
    name ^ "[" ^ string_of_afield_list ~env afl ^ "]"
  | A.Collection ((_, name), _, afl) ->
    let name = SU.Types.fix_casing @@ SU.strip_ns name in
    begin match name with
    | "Set" | "Pair" | "Vector" | "Map"
    | "ImmSet" | "ImmVector" | "ImmMap" ->
      let elems = string_of_afield_list ~env afl in
      let elems =
        if String.length elems <> 0 then " " ^ elems ^ " " else elems in
      "HH\\\\" ^ name ^ " {" ^ elems ^ "}"
    | _ ->
      failwith ("Default value for an unknown collection - " ^ name)
    end
  | A.Shape fl ->
    let fl =
      List.map
        ~f:(fun (f_name, e) ->
          (shape_field_name_to_expr f_name, e))
        fl
    in
    string_of_param_default_value ~env (fst expr, A.Darray (None, fl))
  | A.Binop (bop, e1, e2) ->
    let bop = string_of_bop bop in
    let e1 = string_of_param_default_value ~env e1 in
    let e2 = string_of_param_default_value ~env e2 in
    e1 ^ " " ^ bop ^ " " ^ e2
  | A.New (e, _, es, ues)
  | A.Call (e, _, es, ues) ->
    let e = String_utils.lstrip (string_of_param_default_value ~env e) "\\\\" in
    let es = List.map ~f:(string_of_param_default_value ~env) (es @ ues) in
    let prefix = match snd expr with A.New (_, _, _, _) -> "new " | _ -> "" in
    prefix
    ^ e
    ^ "("
    ^ String.concat ~sep:", " es
    ^ ")"
  | A.Record (e, es) ->
    let es = List.map ~f:(fun (e1, e2) -> A.AFkvalue (e1, e2)) es in
    let e = String_utils.lstrip (string_of_param_default_value ~env e) "\\\\" in
    e ^ (string_of_afield_list ~env es)
  | A.Class_get (e1, e2) ->
    let s1 = match snd e1 with
      | A.Id (_, s1) ->
        get_class_name_from_id
          ~env:env.codegen_env ~should_format:true ~is_class_constant:false s1
      | _ -> string_of_param_default_value ~env e1 in
    let s2 = string_of_param_default_value ~env e2 in
    s1 ^ "::" ^ s2
  | A.Class_const (e1, (_, s2)) ->
    let cexpr_o =
      handle_possible_colon_colon_class_expr ~env ~is_array_get:false expr in
    begin match snd e1, cexpr_o with
    | _, Some cexpr_o -> cexpr_o
    | A.Id (_, s1), _ ->
      let s1 = get_class_name_from_id
        ~env:env.codegen_env ~should_format:true ~is_class_constant:true s1 in
      s1 ^ "::" ^ s2
    | _ ->
      let s1 = string_of_param_default_value ~env e1 in
      s1 ^ "::" ^ s2
    end
  | A.Unop (uop, e) -> begin
    let e = string_of_param_default_value ~env e in
    match uop with
    | A.Upincr -> e ^ "++"
    | A.Updecr -> e ^ "--"
    | _ -> string_of_uop uop ^ e
    end
  | A.Obj_get (e1, e2, f) ->
    let e1 = string_of_param_default_value ~env e1 in
    let e2 = string_of_param_default_value ~env e2 in
    let f = match f with A.OG_nullthrows -> "->" | A.OG_nullsafe -> "\\?->" in
    e1 ^ f ^ e2
  | A.Clone e -> "clone " ^ string_of_param_default_value ~env e
  | A.Array_get (e, eo) ->
    let e = string_of_param_default_value ~env e in
    let eo =
      Option.value_map eo ~default:""
        ~f:(fun e ->
              let cexpr_o =
                handle_possible_colon_colon_class_expr
                  ~env ~is_array_get:true e in
              match cexpr_o with
              | Some s -> s
              | None -> string_of_param_default_value ~env e)
    in
    e ^ "[" ^ eo ^ "]"
  | A.String2 es ->
    String.concat ~sep:" . " @@ List.map ~f:(string_of_param_default_value ~env) es
  | A.PrefixedString (name, e) ->
    String.concat ~sep:" . " @@ [name; string_of_param_default_value ~env e]
  | A.Eif (cond, etrue, efalse) ->
    let cond = string_of_param_default_value ~env cond in
    let etrue =
      Option.value_map etrue ~default:"" ~f:(string_of_param_default_value ~env)
    in
    let efalse = string_of_param_default_value ~env efalse in
    cond ^ " \\? " ^ etrue ^ " : " ^ efalse
  | A.Unsafeexpr e -> string_of_param_default_value ~env  e
  | A.BracedExpr e -> "{" ^ string_of_param_default_value ~env e ^ "}"
  | A.ParenthesizedExpr e -> "(" ^ string_of_param_default_value ~env e ^ ")"
  | A.Cast (h, e) ->
    let h = string_of_hint ~ns: false h in
    let e = string_of_param_default_value ~env e in
    "(" ^ h ^ ")" ^ e
  | A.Pipe (e1, e2) -> middle_aux e1 " |> " e2
  | A.InstanceOf (e1, e2) -> middle_aux e1 " instanceof " e2
  | A.Is (e, h) ->
    let e = string_of_param_default_value ~env e in
    let h = string_of_hint ~ns:true h in
    e ^ " is " ^ h
  | A.As (e, h, b) ->
    let e = string_of_param_default_value ~env e in
    let o = if b then " ?as " else " as " in
    let h = string_of_hint ~ns:true h in
    e ^ o ^ h
  | A.Varray (_, es) ->
    let es = List.map ~f:(string_of_param_default_value ~env) es in
    "varray[" ^ (String.concat ~sep:", " es) ^ "]"
  | A.Darray (_, es) ->
    let es = List.map ~f:(fun (e1, e2) -> A.AFkvalue (e1, e2)) es in
    "darray[" ^ (string_of_afield_list ~env es) ^ "]"
  | A.List l ->
    let l = List.map ~f:(string_of_param_default_value ~env) l in
    "list(" ^ (String.concat ~sep:", " l) ^ ")"
  | A.Yield y ->
    "yield " ^ (string_of_afield ~env y)
  | A.Await a ->
    "await " ^ (string_of_param_default_value ~env a)
  | A.Yield_break ->
    "return"
  | A.Yield_from e ->
    "yield from " ^ (string_of_param_default_value ~env e)
  | A.Import (fl, e) ->
    let fl = string_of_import_flavor fl in
    let e = string_of_param_default_value ~env e in
    fl ^ " " ^ e
  | A.Xml (id, attributes, children) ->
    string_of_xml ~env id attributes children
  | A.Efun (f, use_list) -> string_of_fun ~env f use_list
  | A.Omitted -> ""
  | A.Lfun _ ->
    failwith "expected Lfun to be converted to Efun during closure conversion"
  | A.Suspend _
  | A.Callconv _
  | A.Expr_list _ -> failwith "illegal default value"
  | A.PU_atom _
  | A.PU_identifier _ ->
    failwith "TODO(T35357243): Pocket Universes syntax must be erased by now"

let string_of_param_default_value_option env = function
  | None -> ""
  | Some (label, expr) ->
    let env = { codegen_env = env; in_xhp = false } in
    " = "
    ^ (string_of_label label)
    ^ "(\"\"\""
    ^ (string_of_param_default_value ~env expr)
    ^ "\"\"\")"

let string_of_param_user_attributes p =
  match Hhas_param.user_attributes p with
  | [] -> ""
  | user_attrs ->
    let attrs = Emit_adata.attributes_to_strings user_attrs in
    "[" ^ (String.concat ~sep:" " attrs) ^ "]"

let string_of_is_inout b = if b then "inout " else ""

let string_of_param env p =
  let param_type_info = Hhas_param.type_info p in
  let param_name = Hhas_param.name p in
  let param_default_value = Hhas_param.default_value p in
    string_of_param_user_attributes p
  ^ string_of_is_inout (Hhas_param.is_inout p)
  ^ string_of_is_variadic (Hhas_param.is_variadic p)
  ^ string_of_type_info_option param_type_info
  ^ string_of_is_reference (Hhas_param.is_reference p)
  ^ param_name
  ^ string_of_param_default_value_option env param_default_value

let string_of_params env ps =
  "(" ^ String.concat ~sep:", " (List.map ~f:(string_of_param env) ps) ^ ")"

let add_indent buf indent = Acc.add buf (String.make indent ' ')
let add_indented_line buf indent str =
  Acc.add buf "\n";
  add_indent buf indent;
  Acc.add buf str

let add_num_cls_ref_slots buf indent num_cls_ref_slots =
  if num_cls_ref_slots <> 0
  then add_indented_line buf indent
    (Printf.sprintf ".numclsrefslots %d;" num_cls_ref_slots)

let is_bareword_char c =
  match Char.lowercase c with
  | '_' | '.' | '$' | '\\' -> true
  | c -> (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z')

let is_bareword_string s =
  let rec aux i =
    i >= String.length s || (is_bareword_char (String.get s i) && aux (i + 1)) in
  aux 0

let add_decl_vars buf indent decl_vars =
  let decl_vars = List.map ~f:(fun s ->
    if is_bareword_string s
    then s
    else "\"" ^ (Php_escaping.escape s) ^ "\""
  ) decl_vars in
  if decl_vars <> []
  then add_indented_line buf indent
    (".declvars " ^ String.concat ~sep:" " decl_vars ^ ";")

let add_num_iters buf indent num_iters =
  if num_iters <> 0
  then add_indented_line buf indent
    (Printf.sprintf ".numiters %d;" num_iters)

let add_doc buf indent doc_comment =
  match doc_comment with
  | Some cmt ->
    add_indented_line buf indent @@
      Printf.sprintf ".doc %s;" (SU.triple_quote_string cmt)
  | None -> ()

let add_body buf indent body =
  add_doc buf indent (Hhas_body.doc_comment body);
  if Hhas_body.is_memoize_wrapper body
  then add_indented_line buf indent ".ismemoizewrapper;";
  if Hhas_body.is_memoize_wrapper_lsb body
  then add_indented_line buf indent ".ismemoizewrapperlsb;";
  add_num_iters buf indent (Hhas_body.num_iters body);
  add_num_cls_ref_slots buf indent (Hhas_body.num_cls_ref_slots body);
  add_decl_vars buf indent (Hhas_body.decl_vars body);
  Acc.add buf "\n";
  add_instruction_list buf indent
    (Instruction_sequence.instr_seq_to_list (Hhas_body.instrs body))

let function_attributes f =
  let user_attrs = Hhas_function.attributes f in
  let attrs = Emit_adata.attributes_to_strings user_attrs in
  let attrs = if Emit_env.is_systemlib ()
    then "unique" :: "builtin" :: "persistent" :: attrs else attrs in
  let attrs =
    if Emit_env.is_systemlib () ||
         ((Hhas_attribute.has_dynamically_callable user_attrs) &&
            not (Hhas_function.is_memoize_impl f))
    then "dyn_callable" :: attrs else attrs
  in
  let attrs =
    if not (Hhas_function.is_top f) then "nontop" :: attrs else attrs in
  let attrs =
    if Hhas_function.inout_wrapper f then "inout_wrapper" :: attrs else attrs in
  let attrs =
    if Hhas_function.no_injection f then "no_injection" :: attrs else attrs in
  let attrs =
    if Hhas_attribute.has_native user_attrs then "skip_frame" :: attrs else attrs in
  let attrs =
    if Hhas_attribute.has_foldable user_attrs then "foldable" :: attrs else attrs in
  let attrs =
    if Hhas_function.is_interceptable f then "interceptable" :: attrs else attrs in
  let attrs = match Rx.rx_level_to_attr_string (Hhas_function.rx_level f) with
    | Some s -> s :: attrs
    | None -> attrs
  in
  let text = String.concat ~sep:" " attrs in
  if text = "" then "" else "[" ^ text ^ "] "

let add_fun_def buf fun_def =
  let function_name = Hhas_function.name fun_def in
  let function_body = Hhas_function.body fun_def in
  let function_span = Hhas_function.span fun_def in
  let function_return_type = Hhas_body.return_type function_body in
  let env = Hhas_body.env function_body in
  let function_params = Hhas_body.params function_body in
  let function_is_async = Hhas_function.is_async fun_def in
  let function_is_generator = Hhas_function.is_generator fun_def in
  let function_is_pair_generator = Hhas_function.is_pair_generator fun_def in
  let function_rx_disabled = Hhas_function.rx_disabled fun_def in
  Acc.add buf "\n.function ";
  Acc.add buf (function_attributes fun_def);
  if Hhbc_options.source_mapping !Hhbc_options.compiler_options
  then Acc.add buf (string_of_span function_span ^ " ");
  Acc.add buf (string_of_type_info_option function_return_type);
  Acc.add buf (Hhbc_id.Function.to_raw_string function_name);
  Acc.add buf (string_of_params env function_params);
  if function_is_generator then Acc.add buf " isGenerator";
  if function_is_async then Acc.add buf " isAsync";
  if function_is_pair_generator then Acc.add buf " isPairGenerator";
  if function_rx_disabled then Acc.add buf " isRxDisabled";
  Acc.add buf " {";
  add_body buf 2 function_body;
  Acc.add buf "}\n"

let attributes_to_string attrs =
  let text = String.concat ~sep:" " attrs in
  let text = if text = "" then "" else "[" ^ text ^ "] " in
  text

let method_attributes m =
  let user_attrs = Hhas_method.attributes m in
  let attrs = Emit_adata.attributes_to_strings user_attrs in
  let is_native_opcode_impl = Hhas_attribute.is_native_opcode_impl user_attrs in
  let is_native = not is_native_opcode_impl && Hhas_attribute.has_native user_attrs in
  let is_systemlib = Emit_env.is_systemlib () in
  let attrs =
    if Emit_env.is_systemlib () ||
         ((Hhas_attribute.has_dynamically_callable user_attrs) &&
            not (Hhas_method.is_memoize_impl m))
    then "dyn_callable" :: attrs else attrs
  in
  let attrs = if is_systemlib && is_native then "persistent" :: attrs else attrs in
  let attrs = if is_systemlib then "builtin" :: attrs else attrs in
  let attrs = if is_systemlib && is_native then "unique" :: attrs else attrs in
  let attrs = if Hhas_method.inout_wrapper m then "inout_wrapper" :: attrs else attrs in
  let attrs = if Hhas_method.no_injection m then "no_injection" :: attrs else attrs in
  let attrs = if is_systemlib && is_native then "skip_frame" :: attrs else attrs in
  let attrs =
    if Hhas_attribute.has_foldable user_attrs then "foldable" :: attrs else attrs in
  let attrs = if Hhas_method.is_abstract m then "abstract" :: attrs else attrs in
  let attrs = if Hhas_method.is_final m then "final" :: attrs else attrs in
  let attrs = if Hhas_method.is_static m then "static" :: attrs else attrs in
  let attrs = if Hhas_method.is_public m then "public" :: attrs else attrs in
  let attrs = if Hhas_method.is_protected m then "protected" :: attrs else attrs in
  let attrs = if Hhas_method.is_private m then "private" :: attrs else attrs in
  let attrs = if Hhas_method.is_interceptable m then "interceptable" :: attrs else attrs in
  let attrs = match Rx.rx_level_to_attr_string (Hhas_method.rx_level m) with
    | Some s -> s :: attrs
    | None -> attrs
  in
  attributes_to_string attrs

let typedef_attributes t =
  let user_attrs = Hhas_typedef.attributes t in
  let attrs = Emit_adata.attributes_to_strings user_attrs in
  let attrs =
    if Emit_env.is_systemlib () then "persistent" :: attrs else attrs in
  attributes_to_string attrs

let add_method_def buf method_def =
  let method_name = Hhas_method.name method_def in
  let method_body = Hhas_method.body method_def in
  let method_return_type = Hhas_body.return_type method_body in
  let method_params = Hhas_body.params method_body in
  let env = Hhas_body.env method_body in
  let method_span = Hhas_method.span method_def in
  let method_is_async = Hhas_method.is_async method_def in
  let method_is_generator = Hhas_method.is_generator method_def in
  let method_is_pair_generator = Hhas_method.is_pair_generator method_def in
  let method_is_closure_body = Hhas_method.is_closure_body method_def in
  let method_rx_disabled = Hhas_method.rx_disabled method_def in
  Acc.add buf "\n  .method ";
  Acc.add buf (method_attributes method_def);
  if Hhbc_options.source_mapping !Hhbc_options.compiler_options
  then Acc.add buf (string_of_span method_span ^ " ");
  Acc.add buf (string_of_type_info_option method_return_type);
  Acc.add buf (Hhbc_id.Method.to_raw_string method_name);
  Acc.add buf (string_of_params env method_params);
  if method_is_generator then Acc.add buf " isGenerator";
  if method_is_async then Acc.add buf " isAsync";
  if method_is_pair_generator then Acc.add buf " isPairGenerator";
  if method_is_closure_body then Acc.add buf " isClosureBody";
  if method_rx_disabled then Acc.add buf " isRxDisabled";
  Acc.add buf " {";
  add_body buf 4 method_body;
  Acc.add buf "  }"

let class_special_attributes c =
  let user_attrs = Hhas_class.attributes c in
  let attrs = Emit_adata.attributes_to_strings user_attrs in
  let attrs = if Hhas_class.needs_no_reifiedinit c
    then "noreifiedinit" :: attrs else attrs in
  let attrs = if Hhas_class.no_dynamic_props c
    then "no_dynamic_props" :: attrs else attrs in
  let attrs =
    if Hhas_class.has_immutable c then "has_immutable" :: attrs else attrs in
  let attrs =
    if Hhas_class.is_immutable c then "is_immutable" :: attrs else attrs in
  let attrs =
    if Hhas_attribute.has_foldable user_attrs then "foldable" :: attrs else attrs in
  let attrs = if Emit_env.is_systemlib ()
    then "unique" :: "builtin" :: "persistent" :: attrs else attrs in
  let attrs =
    if Hhas_attribute.has_dynamically_constructible user_attrs
    then "dyn_constructible" :: attrs else attrs
  in
  let attrs = if not (Hhas_class.is_top c) then "nontop" :: attrs else attrs in
  let attrs =
    if Hhas_class.is_closure_class c && not @@ Emit_env.is_systemlib ()
    then "unique" :: attrs else attrs in
  let attrs =
    if Hhas_class.is_closure_class c then "no_override" :: attrs else attrs in
  let attrs = if Hhas_class.is_trait c then "trait" :: attrs else attrs in
  let attrs =
    if Hhas_class.is_interface c then "interface" :: attrs else attrs
  in
  let attrs = if Hhas_class.is_final c then "final" :: attrs else attrs in
  let attrs = if Hhas_class.is_sealed c then "sealed" :: attrs else attrs in
  let attrs =
    if Hhas_class.enum_type c <> None then "enum" :: attrs else attrs
  in
  let attrs = if Hhas_class.is_abstract c then "abstract" :: attrs else attrs in
  let text = String.concat ~sep:" " attrs in
  let text = if text = "" then "" else "[" ^ text ^ "] " in
  text

let add_extends buf class_base =
  match class_base with
  | None -> ()
  | Some name ->
    begin
      Acc.add buf " extends ";
      Acc.add buf (Hhbc_id.Class.to_raw_string name);
    end

let add_implements buf class_implements =
  match class_implements with
  | [] -> ()
  | _ ->
  begin
    Acc.add buf " implements (";
    Acc.add buf (String.concat ~sep:" "
      (List.map ~f:Hhbc_id.Class.to_raw_string class_implements));
    Acc.add buf ")";
  end

let property_attributes p =
  let module P = Hhas_property in
  let user_attrs = P.attributes p in
  let attrs = Emit_adata.attributes_to_strings user_attrs in
  let attrs = if P.is_late_init p then "late_init" :: attrs else attrs in
  let attrs = if P.is_soft_late_init p then "late_init_soft" :: attrs else attrs in
  let attrs = if P.is_no_bad_redeclare p then "no_bad_redeclare" :: attrs else attrs in
  let attrs = if P.initial_satisfies_tc p then "initial_satisfies_tc" :: attrs else attrs in
  let attrs = if P.no_implicit_null p then "no_implicit_null" :: attrs else attrs in
  let attrs = if P.has_system_initial p then "sys_initial_val" :: attrs else attrs in
  let attrs = if P.is_immutable p then "is_immutable" :: attrs else attrs in
  let attrs = if P.is_deep_init p then "deep_init" :: attrs else attrs in
  let attrs = if P.is_lsb p then "lsb" :: attrs else attrs in
  let attrs = if P.is_static p then "static" :: attrs else attrs in
  let attrs = if P.is_public p then "public" :: attrs else attrs in
  let attrs = if P.is_protected p then "protected" :: attrs else attrs in
  let attrs = if P.is_private p then "private" :: attrs else attrs in
  let text = String.concat ~sep:" " attrs in
  let text = if text = "" then "" else "[" ^ text ^ "] " in
  text

let property_type_info p =
  let tinfo = Hhas_property.type_info p in
  (string_of_type_info ~is_enum:false tinfo) ^ " "

let property_doc_comment p =
  match Hhas_property.doc_comment p with
  | None -> ""
  | Some s -> Printf.sprintf "%s " (SU.triple_quote_string s)

let add_property class_def buf property =
  Acc.add buf "\n  .property ";
  Acc.add buf (property_attributes property);
  Acc.add buf (property_doc_comment property);
  Acc.add buf (property_type_info property);
  Acc.add buf (Hhbc_id.Prop.to_raw_string (Hhas_property.name property));
  Acc.add buf " =\n    ";
  let initial_value = Hhas_property.initial_value property in
  if Hhas_class.is_closure_class class_def
  || initial_value = Some Typed_value.Uninit
  then Acc.add buf "uninit;"
  else begin
    Acc.add buf "\"\"\"";
    begin match initial_value with
      | None -> Acc.add buf "N;"
      | Some value -> Emit_adata.adata_to_buffer buf value
    end;
    Acc.add buf "\"\"\";"
  end

let add_constant buf c =
  let name = Hhas_constant.name c in
  let value = Hhas_constant.value c in
  Acc.add buf "\n  .const ";
  Acc.add buf name;
  begin match value with
  | Some Typed_value.Uninit ->
    Acc.add buf " = uninit"
  | Some value ->
    Acc.add buf " = \"\"\"";
    Emit_adata.adata_to_buffer buf value;
    Acc.add buf "\"\"\""
  | None -> ()
    end;
  Acc.add buf ";"

let add_type_constant buf c =
  Acc.add buf "\n  .const ";
  Acc.add buf (Hhas_type_constant.name c);
  let initializer_t = Hhas_type_constant.initializer_t c in
  Acc.add buf " isType";
  match initializer_t with
  | Some init ->
    Acc.add buf " = \"\"\"";
    Emit_adata.adata_to_buffer buf init;
    Acc.add buf "\"\"\";"
  | None -> Acc.add buf ";"

let add_requirement buf r =
  Acc.add buf "\n  .require ";
  match r with
  | (Ast.MustExtend, name) ->
      Acc.add buf ("extends <" ^ name ^ ">;")
  | (Ast.MustImplement, name) ->
      Acc.add buf ("implements <" ^ name ^ ">;")

let add_enum_ty buf c =
  match Hhas_class.enum_type c with
  | Some et ->
    Acc.add buf "\n  .enum_ty ";
    Acc.add buf @@ string_of_type_info ~is_enum:true et;
    Acc.add buf ";"
  | _ -> ()

let add_use_precedence buf (id1, id2, ids) =
  let name = id1 ^ "::" ^ id2 in
  let unique_ids = List.fold_left ~f:ULS.add ~init:ULS.empty ids in
  let ids = String.concat ~sep:" " @@ ULS.items unique_ids in
  Acc.add buf @@ Printf.sprintf "\n    %s insteadof %s;" name ids

let add_use_alias buf (ido1, id, ido2, kindl) =
  let aliasing_id =
    Option.value_map ~f:(fun id1 -> id1 ^ "::" ^ id) ~default:id ido1
  in
  let kind =
    match kindl with
    | [] -> None
    | x -> Some ("[" ^ (String.concat ~sep:" " @@ List.map ~f:Ast.string_of_kind x ) ^ "]")
  in
  let rest = Option.merge kind ido2 ~f:(fun x y -> x ^ " " ^ y) in
  let rest = Option.value ~default:"" rest in
  Acc.add buf @@ Printf.sprintf "\n    %s as %s;" aliasing_id rest

let add_replace buf (trait, id, new_id, kindl, fun_kind) =
  Acc.add buf @@ Printf.sprintf
    "\n    %s::%s as strict "
    trait id;
  if fun_kind = Ast_defs.FAsync || fun_kind = Ast_defs.FAsyncGenerator
  then Acc.add buf "async ";
  Acc.add buf "[";
  let rec concat kindl =
    match kindl with
    | [] -> ()
    | kind :: [] ->
      Acc.add buf (Printf.sprintf "%s" (Ast.string_of_kind kind))
    | kind :: kindl ->
      Acc.add buf (Printf.sprintf "%s " (Ast.string_of_kind kind));
      concat kindl in
  concat kindl;
  Acc.add buf "] ";
  Acc.add buf (Printf.sprintf "%s;" new_id)

let add_uses buf c =
  let use_l = Hhas_class.class_uses c in
  let use_alias_list = Hhas_class.class_use_aliases c in
  let use_precedence_list = Hhas_class.class_use_precedences c in
  let class_method_trait_resolutions = Hhas_class.class_method_trait_resolutions c in
  if use_l = [] && class_method_trait_resolutions = [] then () else
    begin
      let unique_ids =
        List.fold_left ~f:(fun l e -> ULS.add l (Utils.strip_ns e)) ~init:ULS.empty use_l
      in
      let use_l = String.concat ~sep:" " @@ ULS.items unique_ids in
      Acc.add buf @@ Printf.sprintf "\n  .use %s" use_l;
      if use_alias_list = [] && use_precedence_list = [] && class_method_trait_resolutions = []
      then Acc.add buf ";" else
      begin
        Acc.add buf " {";
        List.iter ~f:(add_use_precedence buf) use_precedence_list;
        List.iter ~f:(add_use_alias buf) use_alias_list;
        List.iter ~f:(add_replace buf) class_method_trait_resolutions;
        Acc.add buf "\n  }";
      end
    end

let add_class_def buf class_def =
  let class_name = Hhas_class.name class_def in
  (* TODO: user attributes *)
  Acc.add buf
    (if Hhas_class.is_record class_def then "\n.record " else "\n.class ");
  Acc.add buf (class_special_attributes class_def);
  Acc.add buf (Hhbc_id.Class.to_raw_string class_name);
  if Hhbc_options.source_mapping !Hhbc_options.compiler_options
  then Acc.add buf (" " ^ string_of_span (Hhas_class.span class_def));
  add_extends buf (Hhas_class.base class_def);
  add_implements buf (Hhas_class.implements class_def);
  Acc.add buf " {";
  add_doc buf 2 (Hhas_class.doc_comment class_def);
  add_uses buf class_def;
  add_enum_ty buf class_def;
  List.iter ~f:(add_requirement buf) (Hhas_class.requirements class_def);
  List.iter ~f:(add_constant buf) (Hhas_class.constants class_def);
  List.iter ~f:(add_type_constant buf) (Hhas_class.type_constants class_def);
  List.iter ~f:(add_property class_def buf) (Hhas_class.properties class_def);
  List.iter ~f:(add_method_def buf) (Hhas_class.methods class_def);
  (* TODO: other members *)
  Acc.add buf "\n}\n"

let add_data_region_element buf argument =
  Acc.add buf ".adata ";
  Acc.add buf @@ (Hhas_adata.id argument);
  Acc.add buf " = \"\"\"";
  Emit_adata.adata_to_buffer buf (Hhas_adata.value argument);
  Acc.add buf "\"\"\";\n"

let add_data_region buf adata =
  List.iter ~f:(add_data_region_element buf) adata;
  Acc.add buf "\n"

let add_top_level buf body =
  Acc.add buf ".main ";
  if Hhbc_options.source_mapping !Hhbc_options.compiler_options
  then Acc.add buf "(1,1) ";
  Acc.add buf "{";
  add_body buf 2 body;
  Acc.add buf "}\n"

let add_typedef buf typedef =
  let name = Hhas_typedef.name typedef in
  let type_info = Hhas_typedef.type_info typedef in
  let opt_ts = Hhas_typedef.type_structure typedef in
  Acc.add buf "\n.alias ";
  Acc.add buf (typedef_attributes typedef);
  Acc.add buf (Hhbc_id.Class.to_raw_string name);
  Acc.add buf (" = " ^ string_of_typedef_info type_info);
  match opt_ts with
  | Some ts ->
    Acc.add buf " \"\"\"";
    Emit_adata.adata_to_buffer buf ts;
    Acc.add buf "\"\"\";"
  | None ->
    Acc.add buf ";"

let add_file_attributes buf file_attributes =
  match file_attributes with
  | [] -> ()
  | _ ->
    let attrs = Emit_adata.attributes_to_strings file_attributes in
    let attrs = attributes_to_string attrs in
    Acc.add buf "\n.file_attributes ";
    Acc.add buf attrs;
    Acc.add buf ";\n"

let add_include_region
    ?path ?doc_root ?search_paths ?include_roots ?(check_paths_exist=true)
    buf includes =
  let write_if_exists p =
    if not check_paths_exist || Sys.file_exists p
    then (Acc.add buf ("\n  " ^ p); true)
    else false in
  let write_include inc =
    let include_roots = Option.value include_roots ~default:SMap.empty in
    match Hhas_symbol_refs.resolve_to_doc_root_relative inc ~include_roots with
      | Hhas_symbol_refs.Absolute p -> ignore @@ write_if_exists p
      | Hhas_symbol_refs.SearchPathRelative p ->
        if not check_paths_exist
        then Acc.add buf ("\n  " ^ p)
        else
          let rec try_paths = function
            | [] -> ()
            | prefix :: rest ->
              if write_if_exists (Filename.concat prefix p)
              then ()
              else try_paths rest in
          let dirname =
            Option.value_map path ~default:[] ~f:(fun p -> [Filename.dirname p]) in
          try_paths (dirname @ Option.value search_paths ~default:[])
      | Hhas_symbol_refs.IncludeRootRelative (v, p) -> if p <> "" then
          Option.iter (SMap.find_opt v include_roots) (fun ir ->
            let doc_root = Option.value doc_root ~default:"" in
            let resolved = Filename.concat doc_root (Filename.concat ir p) in
            ignore @@ write_if_exists resolved)
      | Hhas_symbol_refs.DocRootRelative p -> ignore @@
        let doc_root = Option.value doc_root ~default:"" in
        let resolved = Filename.concat doc_root p in
        write_if_exists resolved
  in
  if not (Hhas_symbol_refs.IncludePathSet.is_empty includes) then begin
    Acc.add buf "\n.includes {";
    Hhas_symbol_refs.IncludePathSet.iter write_include includes;
    Acc.add buf "\n}\n"
  end

let add_symbol_ref_regions buf symbol_refs =
  let add_region name refs =
    if not (SSet.is_empty refs) then begin
      Acc.add buf ("\n." ^ name);
      Acc.add buf " {";
      SSet.iter (fun s -> Acc.add buf ("\n  " ^ s)) refs;
      Acc.add buf "\n}\n";
    end in
  add_region "constant_refs" symbol_refs.Hhas_symbol_refs.constants;
  add_region "function_refs" symbol_refs.Hhas_symbol_refs.functions;
  add_region "class_refs" symbol_refs.Hhas_symbol_refs.classes

let add_program_content ?path dump_symbol_refs buf hhas_prog =
  let is_hh = if Hhas_program.is_hh hhas_prog then "1" else "0" in
  Acc.add buf @@ "\n.hh_file " ^ is_hh ^ ";\n";
  let functions = Hhas_program.functions hhas_prog in
  let top_level_body = Hhas_program.main hhas_prog in
  let classes = Hhas_program.classes hhas_prog in
  let adata = Hhas_program.adata hhas_prog in
  let symbol_refs = Hhas_program.symbol_refs hhas_prog in
  add_data_region buf adata;
  add_top_level buf top_level_body;
  List.iter ~f:(add_fun_def buf) functions;
  List.iter ~f:(add_class_def buf) classes;
  List.iter ~f:(add_typedef buf) (Hhas_program.typedefs hhas_prog);
  add_file_attributes buf (Hhas_program.file_attributes hhas_prog);
  if dump_symbol_refs then begin
    let opts = !Hhbc_options.compiler_options in
    add_include_region ?path buf symbol_refs.Hhas_symbol_refs.includes
      ~doc_root:(Hhbc_options.doc_root opts)
      ~search_paths:(Hhbc_options.include_search_paths opts)
      ~include_roots:(Hhbc_options.include_roots opts);
    add_symbol_ref_regions buf symbol_refs
  end

let add_program ?path dump_symbol_refs buf hhas_prog =
  let strict_types =
    match Hhas_program.strict_types hhas_prog with
    | Some true -> ".strict 1;\n\n"
    | Some false -> ".strict 0;\n\n"
    | None -> "" in
  match path with
  | Some p ->
    let p = Php_escaping.escape @@ Relative_path.to_absolute p in
    Acc.add buf
      (Printf.sprintf "# %s starts here\n\n%s.filepath \"%s\";\n" p strict_types p);
    add_program_content ~path:p dump_symbol_refs buf hhas_prog;
    Acc.add buf (Printf.sprintf "\n# %s ends here\n" p)
  | None ->
      Acc.add buf "#starts here\n";
      Acc.add buf strict_types;
      add_program_content dump_symbol_refs buf hhas_prog;
      Acc.add buf "\n#ends here\n"

let to_segments ?path ?(dump_symbol_refs=false) hhas_prog =
  let buf = Acc.create () in
  add_program ?path dump_symbol_refs buf hhas_prog;
  Acc.segments buf

let to_string ?path ?dump_symbol_refs =
  Fn.compose (String.concat ~sep:"") (to_segments ?path ?dump_symbol_refs)
