(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Core
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
| Instr_list of instruct list
| Instr_concat of t list
| Instr_try_fault of t * t

(* Some helper constructors *)
let instr x = Instr_list [x]
let instrs x = Instr_list x
let gather x = Instr_concat x
let empty = Instr_list []
let optional b instrs = if b then gather instrs else empty

let class_ref_rewrite_sentinel = -100

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
let instr_miterinit iter_id label value =
  instr (IIterator (MIterInit (iter_id, label, value)))
let instr_miterinitk id label key value =
  instr (IIterator (MIterInitK (id, label, key, value)))
let instr_miternext id label value =
  instr (IIterator (MIterNext (id, label, value)))
let instr_miternextk id label key value =
  instr (IIterator (MIterNextK (id, label, key, value)))
let instr_miterfree id =
  instr (IIterator (MIterFree id))

let instr_jmp label = instr (IContFlow (Jmp label))
let instr_jmpz label = instr (IContFlow (JmpZ label))
let instr_jmpnz label = instr (IContFlow (JmpNZ label))
let instr_jmpns label = instr (IContFlow (JmpNS label))
let instr_label label = instr (ILabel label)
let instr_continue level = instr (ISpecialFlow (Continue level))
let instr_break level = instr (ISpecialFlow (Break level))
let instr_iter_break label itrs =
  instr (IIterator (IterBreak (label, itrs)))
let instr_unwind = instr (IContFlow Unwind)
let instr_false = instr (ILitConst False)
let instr_true = instr (ILitConst True)
let instr_eq = instr (IOp Eq)
let instr_gt = instr (IOp Gt)
let instr_concat = instr (IOp Concat)
let instr_print = instr (IOp Print)
let instr_retc = instr (IContFlow RetC)
let instr_retv = instr (IContFlow RetV)
let instr_null = instr (ILitConst Null)
let instr_catch = instr (IMisc Catch)
let instr_dup = instr (IBasic Dup)
let instr_instanceofd s = instr (IOp (InstanceOfD s))
let instr_instanceof = instr (IOp InstanceOf)
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
let instr_setn = instr (IMutator SetN)
let instr_unsetl local = instr (IMutator (UnsetL local))
let instr_issetl local = instr (IIsset (IssetL local))
let instr_issetn = instr (IIsset IssetN)
let instr_issets = instr (IIsset (IssetS class_ref_rewrite_sentinel))
let instr_emptys = instr (IIsset (EmptyS class_ref_rewrite_sentinel))
let instr_emptyn = instr (IIsset EmptyN)
let instr_emptyg = instr (IIsset EmptyG)
let instr_emptyl local = instr (IIsset (EmptyL local))
let instr_cgets =
  instr (IGet (CGetS class_ref_rewrite_sentinel))
let instr_vgets =
  instr (IGet (VGetS class_ref_rewrite_sentinel))
let instr_vgetg = instr (IGet VGetG)
let instr_cgetg = instr (IGet CGetG)
let instr_cgetn = instr (IGet CGetN)
let instr_cgetl local = instr (IGet (CGetL local))
let instr_vgetl local = instr (IGet (VGetL local))
let instr_vgetn = instr (IGet VGetN)
let instr_cgetl2 local = instr (IGet (CGetL2 local))
let instr_cgetquietl local = instr (IGet (CGetQuietL local))
let instr_cgetquietn = instr (IGet CGetQuietN)
let instr_cgetn_seq n = gather @@ List.replicate ~num:n instr_cgetn
let instr_bindn = instr (IMutator BindN)
let instr_bindl local = instr (IMutator (BindL local))
let instr_clsrefgetc =
  instr (IGet (ClsRefGetC class_ref_rewrite_sentinel))
let instr_clsrefname =
  instr (IMisc (ClsRefName class_ref_rewrite_sentinel))
let instr_self =
  instr (IMisc (Self class_ref_rewrite_sentinel))
let instr_parent =
  instr (IMisc (Parent class_ref_rewrite_sentinel))
let instr_fpassl param local = instr (ICall (FPassL (param, local)))
let instr_fpassr i = instr (ICall (FPassR i))
let instr_fpassv i = instr (ICall (FPassV i))
let instr_fpassn i = instr (ICall (FPassN i))
let instr_fpassg i = instr (ICall (FPassG i))
and instr_fpass kind i =
  match kind with
  | PassByRefKind.AllowCell -> instr (ICall (FPassC i))
  | PassByRefKind.WarnOnCell -> instr (ICall (FPassCW i))
  | PassByRefKind.ErrorOnCell -> instr (ICall (FPassCE i))

let instr_popu = instr (IBasic PopU)
let instr_popr = instr (IBasic PopR)
let instr_popc = instr (IBasic PopC)
let instr_popv = instr (IBasic PopV)
let instr_pop flavor =
  match flavor with
  | Flavor.Ref -> instr_popv
  | Flavor.Cell -> instr_popc
  | Flavor.ReturnVal -> instr_popr

let instr_pushl local = instr (IGet (PushL local))
let instr_throw = instr (IContFlow Throw)

let instr_add_elemc = instr (ILitConst (AddElemC))
let instr_add_elemv = instr (ILitConst (AddElemV))
let instr_add_new_elemc = instr (ILitConst (AddNewElemC))
let instr_add_new_elemv = instr (ILitConst (AddNewElemV))
let instr_switch labels = instr (IContFlow (Switch (Unbounded, 0, labels)))
let instr_fpushctord nargs id = instr (ICall (FPushCtorD (nargs, id)))
let instr_fpushctor nargs id = instr (ICall (FPushCtor (nargs, id)))
let instr_fpushcuf nargs = instr (ICall (FPushCuf nargs))
let instr_fpushcuf_safe nargs = instr (ICall (FPushCufSafe nargs))
let instr_fpushcuff nargs = instr (ICall (FPushCufF nargs))
let instr_cuf_safe_array = instr (ICall CufSafeArray)
let instr_cuf_safe_return = instr (ICall CufSafeReturn)
let instr_clone = instr (IOp Clone)
let instr_newstructarray keys = instr (ILitConst (NewStructArray keys))
let instr_newcol collection_type = instr (ILitConst (NewCol collection_type))
let instr_colfromarray collection_type =
  instr (ILitConst (ColFromArray collection_type))
let instr_unboxr = instr (IBasic UnboxR)
let instr_unbox = instr (IBasic Unbox)
let instr_box = instr (IBasic Box)
let instr_boxr = instr (IBasic BoxR)
let instr_unboxr_nop = instr (IBasic UnboxRNop)
let instr_entrynop = instr (IBasic EntryNop)
let instr_typedvalue xs = instr (ILitConst (TypedValue xs))
let instr_staticlocinit local text = instr (IMisc (StaticLocInit(local, text)))
let instr_basel local mode = instr (IBase(BaseL(local, mode)))
let instr_basenl local mode = instr (IBase(BaseNL(local, mode)))
let instr_basenc idx mode = instr (IBase(BaseNC(idx, mode)))
let instr_basesc y =
  instr (IBase(BaseSC(y, class_ref_rewrite_sentinel)))
let instr_basesl local =
  instr (IBase(BaseSL(local, class_ref_rewrite_sentinel)))
let instr_baseh = instr (IBase BaseH)
let instr_baser i = instr (IBase (BaseR i))
let instr_fpushfuncd count text = instr (ICall(FPushFuncD(count, text)))
let instr_fcall count = instr (ICall(FCall count))
let instr_isuninit = instr (IMisc IsUninit)
let instr_cgetcunop = instr (IMisc CGetCUNop)
let instr_ugetcunop = instr (IMisc UGetCUNop)
let instr_memoget count local local_count =
  instr (IMisc (MemoGet(count, local, local_count)))
let instr_memoset count local local_count =
  instr (IMisc (MemoSet(count, local, local_count)))
let instr_getmemokeyl local = instr (IMisc (GetMemoKeyL local))
let instr_ismemotype = instr (IMisc IsMemoType)
let instr_maybememotype = instr (IMisc MaybeMemoType)
let instr_checkthis = instr (IMisc CheckThis)
let instr_verifyRetTypeC = instr (IMisc VerifyRetTypeC)
let instr_verifyRetTypeV = instr (IMisc VerifyRetTypeV)
let instr_dim op key = instr (IBase (Dim (op, key)))
let instr_dim_warn_pt key = instr_dim MemberOpMode.Warn (MemberKey.PT key)
let instr_dim_define_pt key = instr_dim MemberOpMode.Define (MemberKey.PT key)
let instr_fpushobjmethodd num_params method_ flavor =
  instr (ICall (FPushObjMethodD (num_params, method_, flavor)))
let instr_fpushclsmethodd num_params class_name method_name =
  instr (ICall (FPushClsMethodD (num_params, class_name, method_name)))
let instr_fpushclsmethod ~forward num_params =
  instr (ICall (
    if forward
    then FPushClsMethodF (num_params, class_ref_rewrite_sentinel)
    else FPushClsMethod (num_params, class_ref_rewrite_sentinel)))
let instr_fpushobjmethodd_nullthrows num_params method_ =
  instr_fpushobjmethodd num_params method_ Ast.OG_nullthrows
let instr_querym num_params op key =
  instr (IFinal (QueryM (num_params, op, key)))
let instr_querym_cget_pt num_params key =
  instr_querym num_params QueryOp.CGet (MemberKey.PT key)
let instr_setm num_params key = instr (IFinal (SetM (num_params, key)))
let instr_setm_pt num_params key = instr_setm num_params (MemberKey.PT key)

let instr_await = instr (IAsync Await)
let instr_yield = instr (IGenerator Yield)
let instr_yieldk = instr (IGenerator YieldK)
let instr_createcont = instr (IGenerator CreateCont)

let instr_static_loc_check name =
  instr (IMisc (StaticLocCheck (Local.Named name,
    Hhbc_string_utils.Locals.strip_dollar name)))

let instr_static_loc_def name =
  instr (IMisc (StaticLocDef (Local.Named name,
    Hhbc_string_utils.Locals.strip_dollar name)))

let instr_static_loc_init name =
  instr (IMisc (StaticLocInit (Local.Named name,
    Hhbc_string_utils.Locals.strip_dollar name)))

let instr_exit = instr (IOp Hhbc_ast.Exit)
let instr_idx = instr (IMisc Idx)
let instr_array_idx = instr (IMisc ArrayIdx)

let instr_fcallbuiltin n un s = instr (ICall (FCallBuiltin (n, un, s)))

let instr_defcls n =
  instr (IIncludeEvalDefine (DefCls n))
let instr_deftypealias n =
  instr (IIncludeEvalDefine (DefTypeAlias n))
let instr_deffunc n =
  instr (IIncludeEvalDefine (DefFunc n))
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

let instr_trigger_sampled_error =
  instr_fcallbuiltin 3 3 "trigger_sampled_error"

(* Functions on instr_seq that correspond to existing Core.List functions *)
module InstrSeq = struct

  (* f takes an instruction and produces an instruction list to replace it. *)
  let rec flat_map instrseq ~f =
    let flat_map_list items ~f = Core.List.bind items f in
    match instrseq with
    | Instr_try_fault (try_body, fault_body) ->
      Instr_try_fault ((flat_map try_body ~f), (flat_map fault_body ~f))
    | Instr_list instrl ->
      Instr_list (flat_map_list instrl ~f)
    | Instr_concat instrseql ->
      Instr_concat (List.map instrseql (flat_map ~f))

  (* f takes an instruction and produces an instrseq to replace it. *)
  let rec flat_map_seq instrseq ~f =
    match instrseq with
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
    | Instr_try_fault (try_body, fault_body) ->
      fold_left fault_body ~init:(fold_left try_body ~f ~init) ~f
    | Instr_list instrl ->
      List.fold_left instrl ~f ~init
    | Instr_concat instrseql ->
      List.fold_left instrseql ~f:fold_instrseq ~init

  let rec filter_map instrseq ~f =
    match instrseq with
    | Instr_try_fault (try_body, fault_body) ->
      Instr_try_fault ((filter_map try_body ~f), (filter_map fault_body ~f))
    | Instr_list instrl ->
      Instr_list (List.filter_map instrl ~f)
    | Instr_concat instrseql ->
      Instr_concat (List.map instrseql (filter_map ~f))

  let map instrseq ~f = filter_map instrseq ~f:(fun x -> Some (f x))

end

(* TODO: Can we eliminate this helper altogether? *)
(* Could write this in terms of InstrSeq.fold_left *)
let rec instr_seq_to_list_aux sl result =
  match sl with
  | [] -> List.rev result
  | s::sl ->
    match s with
    (* NOTE we discard fault blocks when linearizing an instruction sequence *)
    | Instr_try_fault (try_body, _fault_body) ->
      instr_seq_to_list_aux (try_body :: sl) result
    | Instr_list instrl ->
      instr_seq_to_list_aux sl (List.rev_append instrl result)
    | Instr_concat sl' -> instr_seq_to_list_aux (sl' @ sl) result

let instr_seq_to_list t = instr_seq_to_list_aux [t] []

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

(*  Note that at this time we do NOT want to recurse on the instruction
    sequence in the fault block. Why not?  Consider:
    try { x } finally { try { y } finally { z } }
    We make a copy of the code generated for "try { y } finally { z }" in
    both the "finally" code which follows try-fault F1 { x }, and in
    the fault block for the outer try. Which means that now there are two
    places in the code where there is a TryFaultBegin instruction for the
    *inner*  try. We don't want to detect it twice and generate fault blocks
    twice.

    This means that if we ever synthesize a fault-only try-fault, without
    a finally block copying its contents, and that fault block itself
    contains a try-fault or try-finally, then the fault block of the inner
    try-fault will never be detected here. Right now we never do that; we
    only generate synthetic try-faults for simple cleanup operations. If we
    ever do generate nested try-faults then we'll need a more sophisticated
    algorithm here to ensure that each fault block is emitted once.
 *)
let extract_fault_instructions instrseq =
  let rec aux instrseq acc =
    match instrseq with
    | Instr_try_fault (try_body, fault_body) ->
    (* collect fault handlers in try body, then prepend
       the outer fault handler *)
      fault_body :: aux try_body acc
    | Instr_list _ -> acc
    | Instr_concat ([]) -> acc
    | Instr_concat (h :: t) -> aux (Instr_concat t) (aux h acc) in
  (* fault handlers are accumulated in reverse so result list needs to
     be reversed to get the correct order *)
  gather (List.rev @@ aux instrseq [])

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
        | IGet (ClsRefGetL (_, id))
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
| IGet (ClsRefGetL (lid, _)) -> (num + 1, IGet (ClsRefGetL (lid, num + 1)))
| IGet (ClsRefGetC _) -> (num + 1, IGet (ClsRefGetC (num + 1)))
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
| IMutator (BindS _) -> (num - 1, IMutator (BindS num))
| IBase (BaseSC (si, _)) -> (num - 1, IBase (BaseSC (si, num)))
| IBase (BaseSL (l, _)) -> (num - 1, IBase (BaseSL (l, num)))
| ICall (FPassS (np, _)) -> (num - 1, ICall (FPassS (np, num)))
| ICall (FPushCtor (np, _)) -> (num - 1, ICall (FPushCtor (np, num)))
| ICall (FPushClsMethod (np, _)) -> (num - 1, ICall (FPushClsMethod (np, num)))
| ICall (FPushClsMethodF (np, _)) ->
  (num - 1, ICall (FPushClsMethodF (np, num)))
| IIsset (IssetS _) -> (num - 1, IIsset (IssetS num))
| IIsset (EmptyS _) -> (num - 1, IIsset (EmptyS num))
| ILitConst (TypedValue tv) -> (num, Emit_adata.rewrite_typed_value tv)
| i -> (num, i)

(* Cannot use InstrSeq.fold_left since we want to maintain the exact
 * placement of try blocks *)
let rewrite_class_refs instrseq =
  let rec aux instrseq num =
    match instrseq with
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
    | A.Darray es ->
      List.for_all es ~f:(fun (k, v) ->
        can_initialize_static_var k
        && can_initialize_static_var v)
    | A.Varray es ->
      List.for_all es ~f:can_initialize_static_var
    | _ -> false

  let rewrite_static_instrseq static_var_map emit_expr env instrseq =
    let rewrite_static_instr instruction =
      match instruction with
      | IMisc (StaticLocInit (Local.Named name, _)) ->
        begin match (SMap.get name static_var_map) with
              | None ->
                failwith "rewrite_static_instr: No value in static map!"
              | Some None -> gather [instr_null; instr_static_loc_init name;]
              | Some (Some e) ->
                if can_initialize_static_var e then
                  gather [
                    emit_expr env e;
                    instr_static_loc_init name;
                  ]
                else
                  let l = Label.next_regular () in
                  gather [
                    instr_static_loc_check name;
                    instr_jmpnz l;
                    emit_expr env e;
                    instr_static_loc_def name;
                    instr_label l;
                  ]
        end
      | _ -> instr instruction
    in
    InstrSeq.flat_map_seq instrseq rewrite_static_instr

let first instrs =
  let rec aux instrs =
    match instrs with
    | Instr_list l -> List.hd l
    | Instr_concat l -> List.find_map ~f:aux l
    | Instr_try_fault (t, f) -> match aux t with None -> aux f | v -> v
  in
  aux instrs

let is_empty instrs =
  let rec aux instrs =
    match instrs with
    | Instr_list l -> List.is_empty l
    | Instr_concat l -> List.for_all ~f:aux l
    | Instr_try_fault (t, f) -> aux t && aux f
  in
  aux instrs
