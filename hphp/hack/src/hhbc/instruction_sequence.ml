(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Core_kernel
open Common
open Hhbc_ast

module A = Ast

(* The various from_X functions below take some kind of AST (expression,
 * statement, etc.) and produce what is logically a sequence of instructions.
 * This could simply be represented by a list, but then we would need to
 * use an accumulator to avoid the quadratic complexity associated with
 * repeated appending to a list. Instead, we simply build a tree of
 * instructions which can easily be flattened at the end.
 *)
type t =
| Instr_empty
| Instr_one of instruct
| Instr_list of instruct list
| Instr_concat of t list
| Instr_try_fault of t * t

(* Some helper constructors *)
let instr x = Instr_one x
let instrs x = Instr_list x
let gather x =
  match List.filter x (function Instr_empty -> false | _ -> true) with
  | [] -> Instr_empty
  | [x] -> x
  | x -> Instr_concat x
let empty = Instr_empty
let optional b instrs = if b then gather instrs else empty
let optionally f v = match v with None -> empty | Some v -> f v
let of_pair (i1, i2) = gather [i1; i2]

let class_ref_rewrite_sentinel = -100

let default_fcall_flags = {
  has_unpack = false;
  supports_async_eager_return = false;
}
let make_fcall_args ?(flags=default_fcall_flags) ?(num_rets=1)
  ?(by_refs=[]) ?async_eager_label num_args =
  if by_refs <> [] && (List.length by_refs) <> num_args then
    failwith "length of by_refs must be either zero or num_args";
  flags, num_args, num_rets, by_refs, async_eager_label

let instr_lit_const l =
  instr (ILitConst l)

let instr_lit_empty_varray =
  instr_lit_const (TypedValue (Typed_value.VArray []))
let instr_iterinit iter_id label value =
  instr (IIterator (IterInit (iter_id, label, value)))
let instr_iterinitk id label key value =
  instr (IIterator (IterInitK (id, label, key, value)))
let instr_iternext id label value =
  instr (IIterator (IterNext (id, label, value)))
let instr_iternextk id label key value =
  instr (IIterator (IterNextK (id, label, key, value)))
let instr_iterfree id =
  instr (IIterator (IterFree id))
let instr_whresult =
  instr (IAsync WHResult)

let instr_jmp label = instr (IContFlow (Jmp label))
let instr_jmpz label = instr (IContFlow (JmpZ label))
let instr_jmpnz label = instr (IContFlow (JmpNZ label))
let instr_jmpns label = instr (IContFlow (JmpNS label))
let instr_label label = instr (ILabel label)
let instr_continue level = instr (ISpecialFlow (Continue level))
let instr_break level = instr (ISpecialFlow (Break level))
let instr_goto label = instr (ISpecialFlow (Goto label))
let instr_iter_break label itrs =
  instr (IIterator (IterBreak (label, itrs)))
let instr_unwind = instr (IContFlow Unwind)
let instr_false = instr (ILitConst False)
let instr_true = instr (ILitConst True)
let instr_eq = instr (IOp Eq)
let instr_gt = instr (IOp Gt)
let instr_concat = instr (IOp Concat)
let instr_concatn n = instr (IOp (ConcatN n))
let instr_print = instr (IOp Print)
let instr_cast_darray = instr (IOp CastDArray)
let instr_cast_dict = instr (IOp CastDict)
let instr_retc = instr (IContFlow RetC)
let instr_retc_suspended = instr (IContFlow (RetCSuspended))
let instr_retm p = instr (IContFlow (RetM p))
let instr_null = instr (ILitConst Null)
let instr_nulluninit = instr (ILitConst NullUninit)
let instr_catch = instr (IMisc Catch)
let instr_chain_faults = instr (IMisc ChainFaults)
let instr_dup = instr (IBasic Dup)
let instr_nop = instr (IBasic Nop)
let instr_instanceofd s = instr (IOp (InstanceOfD s))
let instr_instanceof = instr (IOp InstanceOf)
let instr_islateboundcls = instr (IOp IsLateBoundCls)
let instr_istypestructc mode = instr (IOp (IsTypeStructC mode))
let instr_astypestructc mode = instr (IOp (AsTypeStructC mode))
let instr_combine_and_resolve_type_struct i =
  instr (IOp (CombineAndResolveTypeStruct i))
let instr_reified_name i name = instr (IMisc (ReifiedName (i, name)))
let instr_record_reified_generic i = instr (IMisc (RecordReifiedGeneric i))
let instr_check_reified_generic_mismatch =
  instr (IMisc CheckReifiedGenericMismatch)
let instr_int i = instr (ILitConst (Int (Int64.of_int i)))
let instr_int64 i = instr (ILitConst (Int i))
let instr_int_of_string litstr =
  instr (ILitConst (Int (Int64.of_string litstr)))
let instr_double litstr = instr (ILitConst (Double litstr))
let instr_string litstr = instr (ILitConst (String litstr))
let instr_this = instr (IMisc This)
let instr_istypec op = instr (IIsset (IsTypeC op))
let instr_istypel id op = instr (IIsset (IsTypeL (id, op)))
let instr_not = instr (IOp Not)
let instr_sets =
  instr (IMutator (SetS class_ref_rewrite_sentinel))
let instr_setl local = instr (IMutator (SetL local))
let instr_unsetl local = instr (IMutator (UnsetL local))
let instr_issetl local = instr (IIsset (IssetL local))
let instr_issetg = instr (IIsset IssetG)
let instr_issets = instr (IIsset (IssetS class_ref_rewrite_sentinel))
let instr_emptys = instr (IIsset (EmptyS class_ref_rewrite_sentinel))
let instr_emptyg = instr (IIsset EmptyG)
let instr_emptyl local = instr (IIsset (EmptyL local))
let instr_cgets =
  instr (IGet (CGetS class_ref_rewrite_sentinel))
let instr_vgets =
  instr (IGet (VGetS class_ref_rewrite_sentinel))
let instr_cgetg = instr (IGet CGetG)
let instr_cgetl local = instr (IGet (CGetL local))
let instr_cugetl local = instr (IGet (CUGetL local))
let instr_vgetl local = instr (IGet (VGetL local))
let instr_cgetl2 local = instr (IGet (CGetL2 local))
let instr_cgetquietl local = instr (IGet (CGetQuietL local))
let instr_clsrefgetc =
  instr (IGet (ClsRefGetC class_ref_rewrite_sentinel))
let instr_clsrefgetts =
  instr (IGet (ClsRefGetTS class_ref_rewrite_sentinel))
let instr_clsrefname =
  instr (IMisc (ClsRefName class_ref_rewrite_sentinel))
let instr_self =
  instr (IMisc (Self class_ref_rewrite_sentinel))
let instr_lateboundcls =
  instr (IMisc (LateBoundCls class_ref_rewrite_sentinel))
let instr_parent =
  instr (IMisc (Parent class_ref_rewrite_sentinel))

let instr_popu = instr (IBasic PopU)
let instr_popc = instr (IBasic PopC)
let instr_popl l = instr (IMutator (PopL l))

let instr_pushl local = instr (IGet (PushL local))
let instr_throw = instr (IContFlow Throw)

let instr_new_vec_array i = instr (ILitConst (NewVecArray i))
let instr_add_elemc = instr (ILitConst (AddElemC))
let instr_add_elemv = instr (ILitConst (AddElemV))
let instr_add_new_elemc = instr (ILitConst (AddNewElemC))
let instr_add_new_elemv = instr (ILitConst (AddNewElemV))
let instr_switch labels = instr (IContFlow (Switch (Unbounded, 0, labels)))
let instr_newobj id op = instr (ICall (NewObj (id, op)))
let instr_newobjd id = instr (ICall (NewObjD id))
let instr_newobjs scref = instr (ICall (NewObjS scref))
let instr_fpushctor nargs = instr (ICall (FPushCtor nargs))
let instr_clone = instr (IOp Clone)
let instr_newstructarray keys = instr (ILitConst (NewStructArray keys))
let instr_newstructdarray keys = instr (ILitConst (NewStructDArray keys))
let instr_newstructdict keys = instr (ILitConst (NewStructDict keys))
let instr_newcol collection_type = instr (ILitConst (NewCol collection_type))
let instr_colfromarray collection_type =
  instr (ILitConst (ColFromArray collection_type))
let instr_box = instr (IBasic Box)
let instr_entrynop = instr (IBasic EntryNop)
let instr_typedvalue xs = instr (ILitConst (TypedValue xs))
let instr_basel local mode = instr (IBase(BaseL(local, mode)))
let instr_basec stack_index mode = instr (IBase (BaseC(stack_index, mode)))
let instr_basesc y mode =
  instr (IBase(BaseSC(y, class_ref_rewrite_sentinel, mode)))
let instr_baseh = instr (IBase BaseH)
let instr_fpushfunc n param_locs = instr (ICall(FPushFunc(n, param_locs)))
let instr_fpushfuncd n id = instr (ICall(FPushFuncD(n, id)))
let instr_fpushfuncu n id fallback = instr (ICall(FPushFuncU(n, id, fallback)))
let instr_fcall fcall_args =
  let no_class = Hhbc_id.Class.from_raw_string "" in
  let no_func = Hhbc_id.Function.from_raw_string "" in
  instr (ICall(FCall(fcall_args, no_class, no_func)))
let instr_cgetcunop = instr (IMisc CGetCUNop)
let instr_ugetcunop = instr (IMisc UGetCUNop)
let instr_memoget label range =
  instr (IMisc (MemoGet(label, range)))
let instr_memoget_eager label1 label2 range =
  instr (IMisc (MemoGetEager(label1, label2, range)))
let instr_memoset range =
  instr (IMisc (MemoSet range))
let instr_memoset_eager range =
  instr (IMisc (MemoSetEager range))
let instr_getmemokeyl local = instr (IMisc (GetMemoKeyL local))
let instr_checkthis = instr (IMisc CheckThis)
let instr_func_num_args = instr (IMisc FuncNumArgs)
let instr_verifyRetTypeC = instr (IMisc VerifyRetTypeC)
let instr_verifyRetTypeTS = instr (IMisc VerifyRetTypeTS)
let instr_verifyOutType i = instr (IMisc (VerifyOutType i))
let instr_dim op key = instr (IBase (Dim (op, key)))
let instr_dim_warn_pt key = instr_dim MemberOpMode.Warn (MemberKey.PT key)
let instr_dim_define_pt key = instr_dim MemberOpMode.Define (MemberKey.PT key)
let instr_fpushobjmethod n flavor pl =
  instr (ICall (FPushObjMethod (n, flavor, pl)))
let instr_fpushobjmethodd num_params method_ flavor =
  instr (ICall (FPushObjMethodD (num_params, method_, flavor)))
let instr_fpushclsmethodd num_params method_name class_name =
  instr (ICall (FPushClsMethodD (num_params, method_name, class_name)))
let instr_fpushclsmethod num_params pl =
  instr (ICall (FPushClsMethod (num_params, class_ref_rewrite_sentinel, pl)))
let instr_fpushclsmethods num_params scref =
  instr (ICall (FPushClsMethodS (num_params, scref)))
let instr_fpushclsmethodsd num_params scref method_name =
  instr (ICall (FPushClsMethodSD (num_params, scref, method_name)))
let instr_fpushobjmethodd_nullthrows num_params method_ =
  instr_fpushobjmethodd num_params method_ Ast.OG_nullthrows
let instr_querym num_params op key =
  instr (IFinal (QueryM (num_params, op, key)))
let instr_querym_cget_pt num_params key =
  instr_querym num_params QueryOp.CGet (MemberKey.PT key)
let instr_setm num_params key = instr (IFinal (SetM (num_params, key)))
let instr_setm_pt num_params key = instr_setm num_params (MemberKey.PT key)
let instr_resolve_func func_id = instr (IOp (ResolveFunc func_id))
let instr_resolve_obj_method = instr (IOp (ResolveObjMethod))
let instr_resolve_cls_method = instr (IOp (ResolveClsMethod))
let instr_await = instr (IAsync Await)
let instr_yield = instr (IGenerator Yield)
let instr_yieldk = instr (IGenerator YieldK)
let instr_createcont = instr (IGenerator CreateCont)
let instr_awaitall range = instr (IAsync (AwaitAll range))

let instr_exit = instr (IOp Hhbc_ast.Exit)
let instr_idx = instr (IMisc Idx)
let instr_array_idx = instr (IMisc ArrayIdx)

let instr_fcallbuiltin n un s = instr (ICall (FCallBuiltin (n, un, s)))

let instr_defcls n =
  instr (IIncludeEvalDefine (DefCls n))
let instr_defclsnop n =
  instr (IIncludeEvalDefine (DefClsNop n))
let instr_deftypealias n =
  instr (IIncludeEvalDefine (DefTypeAlias n))
let instr_defcns s =
  instr (IIncludeEvalDefine (DefCns (Hhbc_id.Const.from_raw_string s)))
let instr_eval = instr (IIncludeEvalDefine Eval)
let instr_alias_cls c1 c2 =
  instr (IIncludeEvalDefine (AliasCls (c1, c2)))

let instr_silence_start local =
  instr (IMisc (Silence (local, Start)))
let instr_silence_end local =
  instr (IMisc (Silence (local, End)))

let instr_contAssignDelegate iter =
  instr (IGenDelegation (ContAssignDelegate iter))
let instr_contEnterDelegate =
  instr (IGenDelegation ContEnterDelegate)
let instr_yieldFromDelegate iter l =
  instr (IGenDelegation (YieldFromDelegate (iter, l)))
let instr_contUnsetDelegate_free iter =
  instr (IGenDelegation (ContUnsetDelegate (FreeIter, iter)))
let instr_contUnsetDelegate_ignore iter =
  instr (IGenDelegation (ContUnsetDelegate (IgnoreIter, iter)))

let instr_contcheck_check = instr (IGenerator (ContCheck CheckStarted))
let instr_contcheck_ignore = instr (IGenerator (ContCheck IgnoreStarted))
let instr_contenter = instr (IGenerator ContEnter)
let instr_contraise = instr (IGenerator ContRaise)
let instr_contvalid = instr (IGenerator ContValid)
let instr_contcurrent = instr (IGenerator ContCurrent)
let instr_contkey = instr (IGenerator ContKey)
let instr_contgetreturn = instr (IGenerator ContGetReturn)

let instr_trigger_sampled_error =
  instr_fcallbuiltin 3 3 "trigger_sampled_error"

let instr_nativeimpl = instr (IMisc NativeImpl)


(* Functions on instr_seq that correspond to existing Hh_core.List functions *)
module InstrSeq = struct

  (* f takes an instruction and produces an instruction list to replace it. *)
  let rec flat_map instrseq ~f =
    let flat_map_list items ~f = Hh_core.List.bind items f in
    match instrseq with
    | Instr_empty -> Instr_empty
    | Instr_one x ->
      begin match f x with
      | [] -> Instr_empty
      | [x] -> Instr_one x
      | x -> Instr_list x
      end
    | Instr_try_fault (try_body, fault_body) ->
      Instr_try_fault ((flat_map try_body ~f), (flat_map fault_body ~f))
    | Instr_list instrl ->
      Instr_list (flat_map_list instrl ~f)
    | Instr_concat instrseql ->
      Instr_concat (List.map instrseql (flat_map ~f))

  (* f takes an instruction and produces an instrseq to replace it. *)
  let rec flat_map_seq instrseq ~f =
    match instrseq with
    | Instr_empty -> Instr_empty
    | Instr_one x -> f x
    | Instr_try_fault (try_body, fault_body) ->
      Instr_try_fault ((flat_map_seq try_body ~f), (flat_map_seq fault_body ~f))
    | Instr_list instrl ->
      Instr_concat (List.map instrl ~f)
    | Instr_concat instrseql ->
      Instr_concat (List.map instrseql (flat_map_seq ~f))

  let rec fold_left instrseq ~f ~init =
    let fold_instrseq init instrseq =
      fold_left instrseq ~f ~init in
    match instrseq with
    | Instr_empty -> init
    | Instr_one x -> f init x
    | Instr_try_fault (try_body, fault_body) ->
      fold_left fault_body ~init:(fold_left try_body ~f ~init) ~f
    | Instr_list instrl ->
      List.fold_left instrl ~f ~init
    | Instr_concat instrseql ->
      List.fold_left instrseql ~f:fold_instrseq ~init

  let rec filter_map instrseq ~f =
    match instrseq with
    | Instr_empty -> Instr_empty
    | Instr_one x ->
      begin match f x with
      | Some x -> Instr_one x
      | None -> Instr_empty
      end
    | Instr_try_fault (try_body, fault_body) ->
      Instr_try_fault ((filter_map try_body ~f), (filter_map fault_body ~f))
    | Instr_list instrl ->
      Instr_list (List.filter_map instrl ~f)
    | Instr_concat instrseql ->
      Instr_concat (List.map instrseql (filter_map ~f))

  let map instrseq ~f = filter_map instrseq ~f:(fun x -> Some (f x))
end

let instr_seq_to_list t =
  (* TODO: Can we eliminate this helper altogether? *)
  (* Could write this in terms of InstrSeq.fold_left *)
  let rec go acc = function
    | [] -> acc
    | s::sl ->
      match s with
      | Instr_empty -> go acc sl
      | Instr_one x -> go (x :: acc) sl
      (* NOTE we discard fault blocks when linearizing an instruction sequence *)
      | Instr_try_fault (try_body, _fault_body) -> go acc (try_body :: sl)
      | Instr_list instrl -> go (List.rev_append instrl acc) sl
      | Instr_concat sl' -> go acc (sl' @ sl) in
  let rec compact_src_locs acc = function
    | [] -> acc
    | ((ISrcLoc _) as i) :: is -> begin
      match acc with
      | (ISrcLoc _) :: _ -> compact_src_locs acc is
      | _ -> compact_src_locs (i :: acc) is
    end
    | i :: is -> compact_src_locs (i :: acc) is in
  go [] [t] |> compact_src_locs []

let instr_try_fault fault_label try_body fault_body =
  let instr_try_fault_body = gather [
    instr (ITry (TryFaultBegin (fault_label)));
    try_body;
    instr (ITry TryFaultEnd)
  ] in
  let fault_body = gather [
    instr_label fault_label;
    fault_body
  ] in
  Instr_try_fault (instr_try_fault_body, fault_body)

let instr_try_catch_begin = instr (ITry TryCatchBegin)
let instr_try_catch_middle = instr (ITry TryCatchMiddle)
let instr_try_catch_end = instr (ITry TryCatchEnd)

(* Return a list of all fault funclets in instrseq, to be emitted after the
function's main body. *)
let extract_fault_funclets instrseq =
  let rec aux instrseq acc =
    match instrseq with
    | Instr_empty
    | Instr_one _ -> acc
    | Instr_try_fault (try_body, fault_body) ->
    (* collect fault handlers in try body and fault body, then prepend
       the outer fault handler *)
      fault_body :: aux fault_body (aux try_body acc)
    | Instr_list _ -> acc
    | Instr_concat ([]) -> acc
    | Instr_concat (h :: t) -> aux (Instr_concat t) (aux h acc) in
  (* fault handlers are accumulated in reverse so result list needs to
     be reversed to get the correct order *)
  gather (List.rev @@ aux instrseq [])

(* Return a copy of instrseq with any fault bodies stripped out. This is used
when we copy the body of a finally block, to avoid generating two copies of the
fault body. *)
let rec strip_fault_bodies instrseq =
  match instrseq with
  | Instr_empty
  | Instr_one _
  | Instr_list _ -> instrseq
  | Instr_concat l -> Instr_concat (List.map l strip_fault_bodies)
  | Instr_try_fault (try_body, _) ->
      Instr_try_fault (strip_fault_bodies try_body, Instr_empty)

let get_num_cls_ref_slots instrseq =
  InstrSeq.fold_left
    instrseq
    ~init:0
    ~f:(fun num i ->
        match i with
        | IMisc (Parent id)
        | IMisc (LateBoundCls id)
        | IMisc (Self id)
        | IMisc (ClsRefName id)
        | IGet (ClsRefGetTS id)
        | IGet (ClsRefGetC id) -> if id + 1 > num then id + 1 else num
        | _ -> num)

let get_or_put_label name_label_map name =
  match SMap.get name name_label_map with
  | Some label -> label, name_label_map
  | None ->
      let label = Label.next_regular () in
      label, SMap.add name label name_label_map

let rewrite_user_labels_instr name_label_map instruction =
  let get_result = get_or_put_label name_label_map in
  match instruction with
  | IContFlow (Jmp (Label.Named name)) ->
      let label, name_label_map = get_result name in
      IContFlow (Jmp label), name_label_map
  | IIterator (IterBreak ((Label.Named name), iters)) ->
      let label, name_label_map = get_result name in
      IIterator (IterBreak (label, iters)), name_label_map
  | IContFlow (JmpNS (Label.Named name)) ->
      let label, name_label_map = get_result name in
      IContFlow (JmpNS label), name_label_map
  | IContFlow (JmpZ (Label.Named name)) ->
      let label, name_label_map = get_result name in
      IContFlow (JmpZ label), name_label_map
  | IContFlow (JmpNZ (Label.Named name)) ->
      let label, name_label_map = get_result name in
      IContFlow (JmpNZ label), name_label_map
  | ILabel (Label.Named name) ->
      let label, name_label_map = get_result name in
      ILabel label, name_label_map
  | i -> i, name_label_map

let rewrite_user_labels instrseq =
  let rec aux instrseq name_label_map =
    match instrseq with
    | Instr_empty ->
      Instr_empty, name_label_map
    | Instr_one i ->
      let i, map = rewrite_user_labels_instr name_label_map i in
      (Instr_one i), map
    | Instr_try_fault (try_body, fault_body) ->
      let try_body, name_label_map = aux try_body name_label_map in
      let fault_body, name_label_map = aux fault_body name_label_map in
      Instr_try_fault (try_body, fault_body), name_label_map
    | Instr_concat l ->
      let l, name_label_map = List.fold_left l
        ~f:(fun (acc, map) s -> let l, map = aux s map in l :: acc, map)
        ~init:([], name_label_map)
      in
      Instr_concat (List.rev l), name_label_map
    | Instr_list l ->
      let l, name_label_map = List.fold_left l
        ~f:(fun (acc, map) i ->
            let i, map = rewrite_user_labels_instr map i in i :: acc, map)
        ~init:([], name_label_map)
      in
      Instr_list (List.rev l), name_label_map in
  fst @@ aux instrseq SMap.empty

(* TODO: What other instructions manipulate the class ref stack *)
let rewrite_class_refs_instr num = function
| IGet (ClsRefGetC _) -> (num + 1, IGet (ClsRefGetC (num + 1)))
| IGet (ClsRefGetTS _) -> (num + 1, IGet (ClsRefGetTS (num + 1)))
| IMisc (Parent _) -> (num + 1, IMisc (Parent (num + 1)))
| IMisc (LateBoundCls _) -> (num + 1, IMisc (LateBoundCls (num + 1)))
| IMisc (Self _) -> (num + 1, IMisc (Self (num + 1)))
| IMisc (ClsRefName _) -> (num - 1, IMisc (ClsRefName num))
| ILitConst (ClsCns (id, _)) -> (num - 1, ILitConst (ClsCns (id, num)))
| IGet (CGetS _) -> (num - 1, IGet (CGetS num))
| IGet (VGetS _) -> (num - 1, IGet (VGetS num))
| IMutator (SetS _) -> (num - 1, IMutator (SetS num))
| IMutator (SetOpS (o, _)) -> (num - 1, IMutator (SetOpS (o, num)))
| IMutator (IncDecS (o, _)) -> (num - 1, IMutator (IncDecS (o, num)))
| IBase (BaseSC (si, _, m)) -> (num - 1, IBase (BaseSC (si, num, m)))
| ICall (NewObj (_, op)) -> (num - 1, ICall (NewObj (num, op)))
| ICall (FPushClsMethod (np, _, pl)) ->
  (num - 1, ICall (FPushClsMethod (np, num, pl)))
| IIsset (IssetS _) -> (num - 1, IIsset (IssetS num))
| IIsset (EmptyS _) -> (num - 1, IIsset (EmptyS num))
| ILitConst (TypedValue tv) -> (num, Emit_adata.rewrite_typed_value tv)
(* all class ref slots at every catch clause must be uninitialized
   Technically we can reset the counter at ICatchMiddle however this won't work
   in following case:
   try {

   }
   catch (A $e) { unset(AA::$x); }
   catch (B $e) { unset(AA::$x); }

   During codegen all catch clauses are flattened to something like:
   try {
     ...
   }
   catch ($e) {
     if (!($e instanceof A)) goto L1;
     load_class_ref(AA);
     raise_fatal "cannot unset static property"

     goto AfterCatch:
     if (!($e instanceof B)) throw $e; // (**)
     load_class_ref(AA);
     raise_fatal "cannot unset static property"

     goto AfterCatch
   }
   AfterCatch:

   Resetting counter at the beginning of catch clause will not help
   to reset it at line (**). What we do instead - reset counter
   at instructions that raise errors

   *)
| (IOp (Fatal _) | IContFlow Throw) as i -> (-1, i)
| i -> (num, i)

(* Cannot use InstrSeq.fold_left since we want to maintain the exact
 * placement of try blocks *)
let rewrite_class_refs instrseq =
  let rec aux instrseq num =
    match instrseq with
    | Instr_empty ->
      Instr_empty, num
    | Instr_one i ->
      let n, i = rewrite_class_refs_instr num i in
      Instr_one i, n
    | Instr_try_fault (try_body, fault_body) ->
      let try_body, num = aux try_body num in
      let fault_body, num = aux fault_body num in
      Instr_try_fault (try_body, fault_body), num
    | Instr_concat l ->
      let l, num = List.fold_left l
        ~f:(fun (acc, n) s -> let l, n = aux s n in l :: acc, n)
        ~init:([], num)
      in
      Instr_concat (List.rev l), num
    | Instr_list l ->
      let l, num = List.fold_left l
        ~f:(fun (acc, n) i ->
            let n, i = rewrite_class_refs_instr n i in i :: acc, n)
        ~init:([], num)
      in
      Instr_list (List.rev l), num
  in
  fst @@ aux instrseq (-1)

let rec can_initialize_static_var e =
  match snd e with
  | A.Float _ | A.String _ | A.Int _ | A.Null | A.False | A.True -> true
  | A.Array es ->
    List.for_all es ~f:(function
      | A.AFvalue v ->
        can_initialize_static_var v
      | A.AFkvalue (k, v) ->
        can_initialize_static_var k
        && can_initialize_static_var v)
  | A.Darray (_, es) ->
    List.for_all es ~f:(fun (k, v) ->
      can_initialize_static_var k
      && can_initialize_static_var v)
  | A.Varray (_, es) ->
    List.for_all es ~f:can_initialize_static_var
  | A.Class_const(_, (_, name)) ->
    String.lowercase name = Naming_special_names.Members.mClass
  | A.Collection ((_, name), _, fields) ->
    let name =
      Hhbc_string_utils.Types.fix_casing @@ Hhbc_string_utils.strip_ns name in
    begin match name with
    | "vec" ->
      List.for_all fields ~f:(function
        | A.AFvalue e -> can_initialize_static_var e
        | _ -> false)
    | "keyset" ->
      List.for_all fields ~f:(function
        | A.AFvalue (_, (A.String _ | A.Int _)) -> true
        | _ -> false)
    | "dict" ->
      List.for_all fields ~f:(function
        | A.AFkvalue ((_, (A.String _ | A.Int _)), v) ->
          can_initialize_static_var v
        | _ -> false)
    | _ -> false
    end
  | _ -> false

let is_srcloc i =
  match i with
  | ISrcLoc _ -> true
  | _ -> false

let first instrs =
  let rec aux instrs =
    match instrs with
    | Instr_empty -> None
    | Instr_one i -> if is_srcloc i then None else Some i
    | Instr_list l -> List.find l (fun x -> not (is_srcloc x))
    | Instr_concat l -> List.find_map ~f:aux l
    | Instr_try_fault (t, f) -> match aux t with None -> aux f | v -> v
  in
  aux instrs

let is_empty instrs =
  let rec aux instrs =
    match instrs with
    | Instr_empty -> true
    | Instr_one i -> is_srcloc i
    | Instr_list l -> List.is_empty l || List.for_all ~f:is_srcloc l
    | Instr_concat l -> List.for_all ~f:aux l
    | Instr_try_fault (t, f) -> aux t && aux f
  in
  aux instrs
