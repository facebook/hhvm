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
let num_args_of (flags, num_args, _, _, _) =
  if flags.has_unpack then num_args + 1 else num_args

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
let instr_reified_name name = instr (IMisc (ReifiedName name))
let instr_record_reified_generic = instr (IMisc RecordReifiedGeneric)
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
let instr_add_new_elemc = instr (ILitConst (AddNewElemC))
let instr_switch labels = instr (IContFlow (Switch (Unbounded, 0, labels)))
let instr_newobj id op = instr (ICall (NewObj (id, op)))
let instr_newobjd id = instr (ICall (NewObjD id))
let instr_newobjs scref = instr (ICall (NewObjS scref))
let instr_clone = instr (IOp Clone)
let instr_new_record id keys = instr (ILitConst (NewRecord (id, keys)))
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
let instr_fcallfunc fcall_args param_locs = gather [
  instr (ICall (FPushFunc ((num_args_of fcall_args), param_locs)));
  instr (ICall (FCall (fcall_args)))
]
let instr_fcallfuncd fcall_args id = gather [
  instr (ICall (FPushFuncD ((num_args_of fcall_args), id)));
  instr (ICall (FCall (fcall_args)))
]
let instr_fcallctor fcall_args = gather [
  instr (ICall (FPushCtor (num_args_of fcall_args)));
  instr (ICall (FCall (fcall_args)))
]
let instr_fcallobjmethod fcall_args flavor pl = gather [
  instr (ICall (FPushObjMethod ((num_args_of fcall_args), flavor, pl)));
  instr (ICall (FCall (fcall_args)))
]
let instr_fcallobjmethodd fcall_args method_ flavor = gather [
  instr (ICall (FPushObjMethodD ((num_args_of fcall_args), method_, flavor)));
  instr (ICall (FCall (fcall_args)))
]
let instr_fcallobjmethodrd fcall_args method_ flavor = gather [
  instr (ICall (FPushObjMethodRD ((num_args_of fcall_args), method_, flavor)));
  instr (ICall (FCall (fcall_args)))
]
let instr_fcallclsmethodd fcall_args method_name class_name = gather [
  instr (ICall (
    FPushClsMethodD ((num_args_of fcall_args), method_name, class_name)));
  instr (ICall (FCall (fcall_args)))
]
let instr_fcallclsmethod fcall_args pl = gather [
  instr (ICall (
    FPushClsMethod ((num_args_of fcall_args), class_ref_rewrite_sentinel, pl)));
  instr (ICall (FCall (fcall_args)))
]
let instr_fcallclsmethods fcall_args scref = gather [
  instr (ICall (FPushClsMethodS ((num_args_of fcall_args), scref)));
  instr (ICall (FCall (fcall_args)))
]
let instr_fcallclsmethodsd fcall_args scref method_name = gather [
  instr (ICall (
    FPushClsMethodSD ((num_args_of fcall_args), scref, method_name)));
  instr (ICall (FCall (fcall_args)))
]
let instr_fcallobjmethodd_nullthrows fcall_args method_ =
  instr_fcallobjmethodd fcall_args method_ Ast.OG_nullthrows
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

let instr_awaitall_list unnamed_locals =
  let head, tail = match unnamed_locals with
  | head :: tail -> head, tail
  | _ -> failwith "Expected at least one await" in

  let get_unnamed_offset unnamed_local = match unnamed_local with
  | Local.Unnamed i -> i
  | _ -> failwith "Expected unnamed local" in

  let _ = List.fold_left tail
    ~init:(get_unnamed_offset head)
    ~f:(fun acc unnamed_local ->
      let next_offset = get_unnamed_offset unnamed_local in
      assert (acc + 1 = next_offset);
      next_offset
    ) in
  instr_awaitall (Some (head, List.length unnamed_locals))

let instr_exit = instr (IOp Hhbc_ast.Exit)
let instr_idx = instr (IMisc Idx)
let instr_array_idx = instr (IMisc ArrayIdx)

let instr_fcallbuiltin n un s = instr (ICall (FCallBuiltin (n, un, s)))

let instr_defcls n =
  instr (IIncludeEvalDefine (DefCls n))
let instr_defclsnop n =
  instr (IIncludeEvalDefine (DefClsNop n))
let instr_defrecord n =
  instr (IIncludeEvalDefine (DefRecord n))
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
    | Instr_list instrl ->
      Instr_list (flat_map_list instrl ~f)
    | Instr_concat instrseql ->
      Instr_concat (List.map instrseql (flat_map ~f))

  (* f takes an instruction and produces an instrseq to replace it. *)
  let rec flat_map_seq instrseq ~f =
    match instrseq with
    | Instr_empty -> Instr_empty
    | Instr_one x -> f x
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

let instr_try_catch_begin = instr (ITry TryCatchBegin)
let instr_try_catch_middle = instr (ITry TryCatchMiddle)
let instr_try_catch_end = instr (ITry TryCatchEnd)

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

(* TODO: This function lacks any awareness of control flow and will produce
 * garbage when control flow is present. However, instead of implementing
 * control flow, we should just kill class ref slots in favor of class pointers.
 *)
let rewrite_class_refs_instr num stack = function
| IGet (ClsRefGetC _) -> (num + 1, stack, IGet (ClsRefGetC (num + 1)))
| IGet (ClsRefGetTS _) -> (num + 1, stack, IGet (ClsRefGetTS (num + 1)))
| IMisc (Parent _) -> (num + 1, stack, IMisc (Parent (num + 1)))
| IMisc (LateBoundCls _) -> (num + 1, stack, IMisc (LateBoundCls (num + 1)))
| IMisc (Self _) -> (num + 1, stack, IMisc (Self (num + 1)))
| IMisc (ClsRefName _) -> (num - 1, stack, IMisc (ClsRefName num))
| ILitConst (ClsCns (id, _)) -> (num - 1, stack, ILitConst (ClsCns (id, num)))
| IGet (CGetS _) -> (num - 1, stack, IGet (CGetS num))
| IGet (VGetS _) -> (num - 1, stack, IGet (VGetS num))
| IMutator (SetS _) -> (num - 1, stack, IMutator (SetS num))
| IMutator (SetOpS (o, _)) -> (num - 1, stack, IMutator (SetOpS (o, num)))
| IMutator (IncDecS (o, _)) -> (num - 1, stack, IMutator (IncDecS (o, num)))
| IBase (BaseSC (si, _, m)) -> (num - 1, stack, IBase (BaseSC (si, num, m)))
| ICall (NewObj (_, op)) -> (num - 1, stack, ICall (NewObj (num, op)))
| ICall (FPushClsMethod (np, _, pl)) ->
  (num - 1, stack, ICall (FPushClsMethod (np, num, pl)))
| IIsset (IssetS _) -> (num - 1, stack, IIsset (IssetS num))
| IIsset (EmptyS _) -> (num - 1, stack, IIsset (EmptyS num))
| ILitConst (TypedValue tv) -> (num, stack, Emit_adata.rewrite_typed_value tv)
(* Limited support for try/catch while class ref slot is active. Propagate the
 * number of active class ref slots from the end of the try block to the code
 * after try/catch.
 *)
| ITry TryCatchMiddle -> (num, num :: stack, ITry TryCatchMiddle)
| ITry TryCatchEnd ->
  begin match stack with
  | [] -> failwith "mismatched TryCatchEnd"
  | top :: stack -> (top, stack, ITry TryCatchEnd)
  end
(* Fatal and Throw enter unwinder, so we don't know anything about the number of
 * active class ref slots after it.
 *)
| (IOp (Fatal _) | IContFlow Throw) as i -> (-1, stack, i)
| i -> (num, stack, i)

(* Cannot use InstrSeq.fold_left since we want to maintain the exact
 * placement of try blocks *)
let rewrite_class_refs instrseq =
  let rec aux instrseq num stack =
    match instrseq with
    | Instr_empty ->
      Instr_empty, num, stack
    | Instr_one i ->
      let n, s, i = rewrite_class_refs_instr num stack i in
      Instr_one i, n, s
    | Instr_concat l ->
      let l, num, stack = List.fold_left l
        ~f:(fun (acc, n, s) is -> let l, n, s = aux is n s in l :: acc, n, s)
        ~init:([], num, stack)
      in
      Instr_concat (List.rev l), num, stack
    | Instr_list l ->
      let l, num, stack = List.fold_left l
        ~f:(fun (acc, n, s) i ->
            let n, s, i = rewrite_class_refs_instr n s i in i :: acc, n, s)
        ~init:([], num, stack)
      in
      Instr_list (List.rev l), num, stack
  in
  let is, _, _ = aux instrseq (-1) [] in
  is

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
  in
  aux instrs

let is_empty instrs =
  let rec aux instrs =
    match instrs with
    | Instr_empty -> true
    | Instr_one i -> is_srcloc i
    | Instr_list l -> List.is_empty l || List.for_all ~f:is_srcloc l
    | Instr_concat l -> List.for_all ~f:aux l
  in
  aux instrs
