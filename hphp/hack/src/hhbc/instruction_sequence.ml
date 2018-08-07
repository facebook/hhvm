(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Hh_core
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
let instr_print = instr (IOp Print)
let instr_cast_darray = instr (IOp CastDArray)
let instr_cast_dict = instr (IOp CastDict)
let instr_retc = instr (IContFlow RetC)
let instr_retm p = instr (IContFlow (RetM p))
let instr_retv = instr (IContFlow RetV)
let instr_null = instr (ILitConst Null)
let instr_nulluninit = instr (ILitConst NullUninit)
let instr_catch = instr (IMisc Catch)
let instr_chain_faults = instr (IMisc ChainFaults)
let instr_dup = instr (IBasic Dup)
let instr_nop = instr (IBasic Nop)
let instr_instanceofd s = instr (IOp (InstanceOfD s))
let instr_instanceof = instr (IOp InstanceOf)
let instr_istypestruct s = instr (IOp (IsTypeStruct s))
let instr_astypestruct s = instr (IOp (AsTypeStruct s))
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
let instr_issetg = instr (IIsset IssetG)
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
let instr_fis_param_by_ref i hint =
  instr (ICall (FIsParamByRef (i, hint)))
let instr_fthrow_on_ref_mismatch by_refs =
  instr (ICall (FThrowOnRefMismatch by_refs))
let instr_fhandle_ref_mismatch i hint name =
  instr (ICall (FHandleRefMismatch (i, hint, name)))

let instr_popu = instr (IBasic PopU)
let instr_popr = instr (IBasic PopR)
let instr_popc = instr (IBasic PopC)
let instr_popv = instr (IBasic PopV)
let instr_popl l = instr (IMutator (PopL l))
let instr_pop flavor =
  match flavor with
  | Flavor.Ref -> instr_popv
  | Flavor.Cell -> instr_popc
  | Flavor.ReturnVal -> instr_popr

let instr_pushl local = instr (IGet (PushL local))
let instr_throw = instr (IContFlow Throw)

let instr_new_vec_array i = instr (ILitConst (NewVecArray i))
let instr_add_elemc = instr (ILitConst (AddElemC))
let instr_add_elemv = instr (ILitConst (AddElemV))
let instr_add_new_elemc = instr (ILitConst (AddNewElemC))
let instr_add_new_elemv = instr (ILitConst (AddNewElemV))
let instr_switch labels = instr (IContFlow (Switch (Unbounded, 0, labels)))
let instr_fpushctord nargs id = instr (ICall (FPushCtorD (nargs, id)))
let instr_fpushctor nargs id = instr (ICall (FPushCtor (nargs, id)))
let instr_fpushctors nargs scref = instr (ICall (FPushCtorS (nargs, scref)))
let instr_fpushctori nargs clsnum = instr (ICall (FPushCtorI (nargs, clsnum)))
let instr_clone = instr (IOp Clone)
let instr_newstructarray keys = instr (ILitConst (NewStructArray keys))
let instr_newstructdarray keys = instr (ILitConst (NewStructDArray keys))
let instr_newstructdict keys = instr (ILitConst (NewStructDict keys))
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
let instr_basec stack_index mode = instr (IBase (BaseC(stack_index, mode)))
let instr_basenl local mode = instr (IBase(BaseNL(local, mode)))
let instr_basenc idx mode = instr (IBase(BaseNC(idx, mode)))
let instr_basesc y mode =
  instr (IBase(BaseSC(y, class_ref_rewrite_sentinel, mode)))
let instr_basesl local mode =
  instr (IBase(BaseSL(local, class_ref_rewrite_sentinel, mode)))
let instr_baseh = instr (IBase BaseH)
let instr_baser i mode = instr (IBase (BaseR(i, mode)))
let instr_fpushfunc n param_locs = instr (ICall(FPushFunc(n, param_locs)))
let instr_fpushfuncd count text = instr (ICall(FPushFuncD(count, text)))
let instr_fcall count has_unpack nrets =
  let no_class = Hhbc_id.Class.from_raw_string "" in
  let no_func = Hhbc_id.Function.from_raw_string "" in
  instr (ICall(FCall(count, has_unpack, nrets, no_class, no_func)))
let instr_cgetcunop = instr (IMisc CGetCUNop)
let instr_ugetcunop = instr (IMisc UGetCUNop)
let instr_memoget label range =
  instr (IMisc (MemoGet(label, range)))
let instr_memoset range =
  instr (IMisc (MemoSet range))
let instr_getmemokeyl local = instr (IMisc (GetMemoKeyL local))
let instr_checkthis = instr (IMisc CheckThis)
let instr_verifyRetTypeC = instr (IMisc VerifyRetTypeC)
let instr_verifyRetTypeV = instr (IMisc VerifyRetTypeV)
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
let instr_defclsnop n =
  instr (IIncludeEvalDefine (DefClsNop n))
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

let instr_contcheck_check = instr (IGenerator (ContCheck CheckStarted))
let instr_contcheck_ignore = instr (IGenerator (ContCheck IgnoreStarted))
let instr_contenter = instr (IGenerator ContEnter)
let instr_contraise = instr (IGenerator ContRaise)
let instr_contvalid = instr (IGenerator ContValid)
let instr_contstarted = instr (IGenerator ContStarted)
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
| IBase (BaseSC (si, _, m)) -> (num - 1, IBase (BaseSC (si, num, m)))
| IBase (BaseSL (l, _, m)) -> (num - 1, IBase (BaseSL (l, num, m)))
| ICall (FPushCtor (np, _)) -> (num - 1, ICall (FPushCtor (np, num)))
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
  | A.Darray es ->
    List.for_all es ~f:(fun (k, v) ->
      can_initialize_static_var k
      && can_initialize_static_var v)
  | A.Varray es ->
    List.for_all es ~f:can_initialize_static_var
  | A.Class_const(_, (_, name)) ->
    String.lowercase_ascii name = Naming_special_names.Members.mClass
  | A.Collection ((_, name), fields) ->
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

(* returns a pair:
   1. number of elements instruction pops from the stack
   2. number of elements instruction will push to the stack  *)
let get_input_output_count i =
  match i with
  | IBasic i ->
    begin match i with
    | Nop | EntryNop -> (0, 0)
    | PopC | PopV | PopR | PopU -> (1, 0)
    | Dup -> (1, 2)
    | Box | Unbox | BoxR | BoxRNop | UnboxR | UnboxRNop | RGetCNop -> (1, 1)
    end
  | ILitConst i ->
    begin match i with
    | Null | True | False | NullUninit | Int _ | Double _ | String _
    | Array _ | Vec _ | Dict _ | Keyset _ | NewArray _ | NewMixedArray _
    | NewDictArray _ | NewDArray _ | NewLikeArrayL _ | Cns _ | CnsE _
    | CnsU _ | ClsCns _ | ClsCnsD _ | File | Dir | Method | NewCol _ -> (0, 1)
    | NewVArray c | NewVecArray c | NewKeysetArray c
    | NewPackedArray c -> (c, 1)
    | NewStructArray l | NewStructDArray l | NewStructDict l -> (List.length l, 1)
    | AddElemC | AddElemV -> (3, 1)
    | NewPair | AddNewElemC | AddNewElemV -> (2, 1)
    | ColFromArray _ -> (1, 1)
    | TypedValue _ -> failwith "this pseudo-instruction is internal to HackC"
    end
  | IIncludeEvalDefine i ->
    begin match i with
    | Incl | InclOnce | Req | ReqOnce | ReqDoc | AliasCls _ | DefCns _
    | Eval -> (1, 1)
    | DefFunc _ | DefCls _ | DefClsNop _ | DefTypeAlias _ -> (0, 0)
    end
  | IGenDelegation i ->
    begin match i with
    | ContAssignDelegate _
    | ContEnterDelegate -> (1, 0)
    | YieldFromDelegate _ -> (0, 1)
    | ContUnsetDelegate _ -> (0, 0)
    end
  | IGenerator i ->
    begin match i with
    | CreateCont | ContValid | ContStarted | ContKey | ContCurrent
    | ContGetReturn  -> (0, 1)
    | ContEnter | ContRaise | Yield -> (1, 1)
    | YieldK -> (2, 1)
    | ContCheck _ -> (0, 0)
    end
  | IAsync i ->
    begin match i with
    | WHResult | Await -> (1, 1)
    | AwaitAll _ -> (0, 1)
    end
  | ISrcLoc _ | IComment _ -> (0, 0)
  | IOp i ->
    begin match i with
    | Concat | Add | Sub | Mul | AddO | SubO | MulO | Div | Mod | Xor | Same
    | NSame | Eq | Neq | Lt | Lte | Gt | Gte | Cmp | BitAnd | BitOr | BitXor
    | Shl | Shr | InstanceOf | Pow
    | ResolveObjMethod | ResolveClsMethod -> (2, 1)
    | Sqrt | Not | BitNot | Floor | Ceil | CastBool | CastInt | CastDouble
    | CastString | CastArray | CastObject | CastVec | CastDict | CastKeyset
    | CastVArray | CastDArray | InstanceOfD _ | IsTypeStruct _ | AsTypeStruct _
    | Print | Clone | Hhbc_ast.Exit | Abs -> (1, 1)
    | Fatal _ -> (1, 0)
    | ConcatN n -> (n, 1)
    | ResolveFunc _ -> (0, 1)
    end
  | ICall i ->
    begin match i with
    | FPushObjMethodD _ | FPushClsMethod _ | FPushClsMethodS _ | DecodeCufIter _
    | FPushFunc _ -> (1, 0)
    | FPushFuncU _ | FPushClsMethodD _ | FPushClsMethodSD _ | FPushCufIter _
    | FPushFuncD _ | FThrowOnRefMismatch _ | FHandleRefMismatch _ -> (0, 0)
    | FPushObjMethod _ -> (2, 0)
    | FPushCtor _ | FPushCtorD _ | FPushCtorI _ | FPushCtorS _
    | FIsParamByRef _ | FIsParamByRefCufIter _ -> (0, 1)
    | FCall (n1, u, n2, _, _) -> (n1 + (if u then 1 else 0), n2)
    | FCallAwait (n, _, _) | FCallBuiltin (n, _, _) -> (n, 1)
    end
  | IMisc i ->
    begin match i with
    | This | BareThis _ | StaticLocCheck _ | Catch | ChainFaults | ClsRefName _
    | GetMemoKeyL _ -> (0, 1)
    | CheckThis | InitThisLoc _ | VerifyParamType _ | Self _ | Parent _
    | LateBoundCls _ | NativeImpl | AssertRATL _ | AssertRATStk _
    | BreakTraceHint| Silence _ -> (0, 0)
    | StaticLocDef _ | StaticLocInit _ -> (1, 0)
    | OODeclExists _ | AKExists -> (2, 1)
    | VerifyOutType _ | VerifyRetTypeC | VerifyRetTypeV | CGetCUNop
    | UGetCUNop -> (1, 1)
    | CreateCl (n, _) -> (n, 1)
    | MemoGet _ -> (0, 1) | MemoSet _ -> (0, 0)
    | Idx | ArrayIdx -> (3, 1)
    end
  | IGet i ->
    begin match i with
    | CGetL _ | CGetQuietL _ | CUGetL _ | PushL _ | VGetL _ -> (0, 1)
    | CGetL2 _ -> (1, 2)
    | CGetN | CGetQuietN | CGetG | CGetQuietG | CGetS _ | VGetN
    | VGetG | VGetS _ -> (1, 1)
    | ClsRefGetL _ -> (0, 0)
    | ClsRefGetC _ -> (1, 0)
    end
  | IMutator i ->
    begin match i with
    | SetL _ | PopL _ | SetOpL _ | IncDecN _ | IncDecG _ | IncDecS _
    | BindL _ -> (1, 1)
    | SetN | SetG | SetS _ | SetOpN _ | SetOpG _ | SetOpS _ | BindN
    | BindG | BindS _ -> (2, 1)
    | IncDecL _ | CheckProp _ -> (0, 1)
    | UnsetL _ -> (0, 0)
    | UnsetN | UnsetG | InitProp _ -> (1, 0)
    end
  | IIsset i ->
    begin match i with
    | IssetC | IssetN | IssetG | IssetS _ | EmptyN | EmptyG | EmptyS _
    | IsTypeC _ -> (1, 1)
    | IssetL _ | EmptyL _ | IsTypeL _ -> (0, 1)
    end
  | IBase i ->
    begin match i with
    | BaseNC _ | BaseSC _ | BaseSL _ -> (1, 1)
    | BaseNL _ -> (0, 1)
    | BaseGC _ | BaseGL _ | BaseL _ | BaseC _ | BaseR _ | BaseH
    | Dim _ -> (0, 0)
    end
  | IFinal i ->
    begin match i with
    | QueryM (n, _, _) | VGetM (n, _) | IncDecM (n, _, _)
    | UnsetM (n, _) -> (n, 1)
    | SetM (n, _) | SetOpM (n, _, _) | BindM (n, _) -> (n + 1, 1)
    | SetWithRefLML _ -> (0, 0)
    | SetWithRefRML _ -> (1, 0)
    end
  | ISpecialFlow _ ->
    failwith "this pseudo-instruction is internal to HackC"
  | IIterator i ->
    begin match i with
    | IterInit _ | IterInitK _ | WIterInit _ | WIterInitK _
    | MIterInit _ | MIterInitK _ -> (1, 0)
    | LIterInit _ | LIterInitK _
    | IterNext _ | IterNextK _ | LIterNext _ | LIterNextK _
    | WIterNext _ | WIterNextK _ | MIterNext _ | MIterNextK _
    | IterFree _ | MIterFree _ | CIterFree _ | LIterFree _
    | IterBreak _ -> (0, 0)
    end
  | IContFlow _
  | ILabel _
  | ITry _ ->
    failwith "should be handled by the get_estimated_stack_depth"

module LabelMap: MyMap.S with type key = Label.t = MyMap.Make(struct
  type t = Label.t
  let compare = Pervasives.compare
end)

type stack_state = {
  depth: int;
  labels: int LabelMap.t;
  catch_block_nesting: int;
}

let initial_state = {
  depth = 0;
  labels = LabelMap.empty;
  catch_block_nesting = 0
}

(* records assumed depth of evaluation stack at given label
   if it does not yet exist. If depth value is not provided it is taken from
   the state. *)
let set_depth_at_label ?depth label state =
  match LabelMap.get label state.labels with
  | Some _ -> state
  | None ->
    let depth = Option.value ~default:state.depth depth in
    { state with labels = LabelMap.add label depth state.labels }

let get_depth_at_label label state =
  LabelMap.get label state.labels

(* This functions returns an estimated depth of evaluation stack
   after executing given instructions.
   NOTE: in order to simplify the implementation we use the fact that depth
   of the stack for any point must be the same for every control flow path
   that reaches the point - for cases like try/catch blocks we follow only one
   path and ignore another assuming that if we go there stack depth should be
   the same *)
let get_estimated_stack_depth instrs =
  let rec instr_seq_aux init instrs  =
    match instrs with
    | Instr_empty -> init
    | Instr_one i -> instr_aux init i
    | Instr_try_fault (t, _f) -> instr_seq_aux init t
    | Instr_list l -> List.fold_left l ~init ~f:instr_aux
    | Instr_concat is -> List.fold_left is ~init ~f:instr_seq_aux
  and instr_aux acc i  =
    (* if we are in catch block of top level try-catch we instructions
       until we see matching top level TryCatchEnd. This might be a bit tricky
       since if catch block contains nested try-catches we need to ignore their
       TryCatchEnd instructions - use counter to detect this situations *)
    if acc.catch_block_nesting > 0
    then begin match i with
    | ITry TryCatchBegin ->
      { acc with catch_block_nesting = acc.catch_block_nesting + 1 }
    | ITry TryCatchEnd ->
      { acc with catch_block_nesting = acc.catch_block_nesting - 1 }
    | _ -> acc
    end
    else begin match i with
    | IOp (Fatal _) ->
      (* reset stack after fatal *)
      { acc with depth = 0 }

    | ILabel l ->
      (* if we already have some state for label - pick it, otherwise record
         current stack state for label *)
      begin match get_depth_at_label l acc with
      | Some depth -> { acc with depth }
      | None -> set_depth_at_label l acc
      end
    | IContFlow i ->
      begin match i with
      | Jmp l | JmpNS l ->
        (* for unconditional jumps
         - record assumed stack depth at label
         - set stack depth to 0, if subsequent instruction is target of some
          forward jump - it will have previosly recorded stack depth *)
        { (set_depth_at_label l acc) with depth = 0 }
      | JmpZ l | JmpNZ l ->
        (* JmpZ/JmpNZ consumes one value from the stack *)
        (* for conditional jump just record assumed stack depth at label *)
        let acc = { acc with depth = acc.depth - 1 } in
        set_depth_at_label l acc
      (* bounded, base, offset vector *)
      | Switch (_, _, ls) ->
        (* switch consumes one value from the stack *)
        let acc = { acc with depth = acc.depth - 1 } in
        let aux s l = set_depth_at_label l s in
        let acc = List.fold_left ls ~init:acc ~f:aux in
        (* depth after switch is 0 *)
        { acc with depth = 0 }
      | SSwitch ls ->
        (* sswitch consumes one value from the stack *)
        let acc = { acc with depth = acc.depth - 1 } in
        let aux s (_, l) = set_depth_at_label l s in
        let acc = List.fold_left ls ~init:acc ~f:aux in
        (* depth after switch is 0 *)
        { acc with depth = 0 }
      | RetM _
      | RetC
      | RetV
      | Unwind
      | Throw ->
        (* assume stack depth = 0 after Ret*/Throw/Unwind *)
        { acc with depth = 0 }
      end
    | ITry i ->
      begin match i with
      | TryFaultBegin l | TryCatchLegacyBegin l ->
        (* assume stack depth = 0 at the beginning of fault part/catch block *)
        set_depth_at_label ~depth:0 l acc
      | TryCatchBegin | TryFaultEnd | TryCatchLegacyEnd ->
        acc
      | TryCatchMiddle ->
        { acc with catch_block_nesting = acc.catch_block_nesting + 1 }
      | TryCatchEnd ->
        failwith "TryCatchEnd, should be handled in instr_aux, when catch_block_nesting > 1"
      end
    | _ ->
      let (i, o) = get_input_output_count i in
      { acc with depth = acc.depth - i + o }
    end in

  (instr_seq_aux initial_state instrs).depth

let collect_locals f instrs =
  let add acc l =
    match l with
    | Local.Named s when f s -> Unique_list_string.add acc s
    | _ -> acc in
  let add_member_key acc mk =
    match mk with
    | MemberKey.EL l | MemberKey.PL l -> add acc l
    | _ -> acc in
  let aux acc i =
    match i with
    | ILitConst (NewLikeArrayL (l, _))
    | IGet (
      CGetL l | CGetQuietL l |CGetL2 l | CUGetL l | PushL l |
      VGetL l | ClsRefGetL (l, _))
    | IIsset (IssetL l | EmptyL l | IsTypeL (l, _))
    | IMutator (SetL l | PopL l | SetOpL (l, _) | IncDecL (l, _) | BindL l |
                UnsetL l)
    | IBase (BaseNL (l, _) | BaseGL (l, _))
    | IFinal (SetWithRefRML l)
    | IIterator (
      IterInit (_, _, l) | WIterInit (_, _, l) | MIterInit (_, _, l) |
      IterNext (_, _, l) | WIterNext (_, _, l) | MIterNext (_, _, l) |
      LIterFree (_, l)
      )
    | IMisc (InitThisLoc l | StaticLocCheck (l, _) | StaticLocDef (l, _) |
             StaticLocInit (l, _) | AssertRATL (l, _) | Silence (l, _) |
             GetMemoKeyL l |
             MemoGet (_, Some (l, _)) | MemoSet (Some (l, _)))
    | IAsync (AwaitAll (Some (l, _)))
      -> add acc l
    | IFinal (SetWithRefLML (l1, l2))
    | IIterator (
      IterInitK (_, _, l1, l2) | WIterInitK (_, _, l1, l2) | MIterInitK (_, _, l1, l2) |
      IterNextK (_, _, l1, l2) | WIterNextK (_, _, l1, l2) | MIterNextK (_, _, l1, l2) |
      LIterInit (_, l1, _, l2) | LIterNext (_, l1, _, l2)
      )
      -> add (add acc l1) l2
    | IIterator (
      LIterInitK (_, l1, _, l2, l3) | LIterNextK (_, l1, _, l2, l3)
      )
      -> add (add (add acc l1) l2) l3
    | IBase (Dim (_, mk))
    | IFinal (QueryM (_, _, mk) | VGetM (_, mk) | SetM (_, mk) |
              IncDecM (_, _, mk) | BindM (_, mk) | UnsetM (_, mk))
      -> add_member_key acc mk
    | _ -> acc
  in
  InstrSeq.fold_left instrs ~f:aux ~init:Unique_list_string.empty
