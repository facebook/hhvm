(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Hhbc_ast

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
  match
    List.filter x (function
        | Instr_empty -> false
        | _ -> true)
  with
  | [] -> Instr_empty
  | [x] -> x
  | x -> Instr_concat x

let empty = Instr_empty

let optional b instrs =
  if b then
    gather instrs
  else
    empty

let optionally f v =
  match v with
  | None -> empty
  | Some v -> f v

let of_pair (i1, i2) = gather [i1; i2]

let default_fcall_flags =
  {
    has_unpack = false;
    has_generics = false;
    supports_async_eager_return = false;
    lock_while_unwinding = false;
  }

let make_fcall_args
    ?(flags = default_fcall_flags)
    ?(num_rets = 1)
    ?(inouts = [])
    ?async_eager_label
    ?context
    num_args =
  if inouts <> [] && List.length inouts <> num_args then
    failwith "length of inouts must be either zero or num_args";
  (flags, num_args, num_rets, inouts, async_eager_label, context)

let instr_lit_const l = instr (ILitConst l)

let instr_iterinit iter_args label =
  instr (IIterator (IterInit (iter_args, label)))

let instr_iternext iter_args label =
  instr (IIterator (IterNext (iter_args, label)))

let instr_iterfree id = instr (IIterator (IterFree id))

let instr_whresult = instr (IAsync WHResult)

let instr_jmp label = instr (IContFlow (Jmp label))

let instr_jmpz label = instr (IContFlow (JmpZ label))

let instr_jmpnz label = instr (IContFlow (JmpNZ label))

let instr_jmpns label = instr (IContFlow (JmpNS label))

let instr_label label = instr (ILabel label)

let instr_continue level = instr (ISpecialFlow (Continue level))

let instr_break level = instr (ISpecialFlow (Break level))

let instr_goto label = instr (ISpecialFlow (Goto label))

let instr_iter_break label iters =
  let frees = List.rev_map ~f:(fun iter -> IIterator (IterFree iter)) iters in
  instrs (frees @ [IContFlow (Jmp label)])

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

let instr_retc_suspended = instr (IContFlow RetCSuspended)

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

let instr_throwastypestructexception = instr (IOp ThrowAsTypeStructException)

let instr_combine_and_resolve_type_struct i =
  instr (IOp (CombineAndResolveTypeStruct i))

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

let instr_sets = instr (IMutator SetS)

let instr_setl local = instr (IMutator (SetL local))

let instr_unsetl local = instr (IMutator (UnsetL local))

let instr_issetl local = instr (IIsset (IssetL local))

let instr_isunsetl local = instr (IIsset (IsUnsetL local))

let instr_issetg = instr (IIsset IssetG)

let instr_issets = instr (IIsset IssetS)

let instr_cgets = instr (IGet CGetS)

let instr_cgetg = instr (IGet CGetG)

let instr_cgetl local = instr (IGet (CGetL local))

let instr_cugetl local = instr (IGet (CUGetL local))

let instr_cgetl2 local = instr (IGet (CGetL2 local))

let instr_cgetquietl local = instr (IGet (CGetQuietL local))

let instr_classgetc = instr (IGet ClassGetC)

let instr_classgetts = instr (IGet ClassGetTS)

let instr_classname = instr (IMisc ClassName)

let instr_self = instr (IMisc Self)

let instr_lateboundcls = instr (IMisc LateBoundCls)

let instr_parent = instr (IMisc Parent)

let instr_popu = instr (IBasic PopU)

let instr_popc = instr (IBasic PopC)

let instr_popl l = instr (IMutator (PopL l))

let instr_pushl local = instr (IGet (PushL local))

let instr_throw = instr (IContFlow Throw)

let instr_new_vec_array i = instr (ILitConst (NewVecArray i))

let instr_add_elemc = instr (ILitConst AddElemC)

let instr_add_new_elemc = instr (ILitConst AddNewElemC)

let instr_switch labels = instr (IContFlow (Switch (Unbounded, 0, labels)))

let instr_sswitch cases = instr (IContFlow (SSwitch cases))

let instr_newobj = instr (ICall NewObj)

let instr_newobjr = instr (ICall NewObjR)

let instr_newobjd id = instr (ICall (NewObjD id))

let instr_newobjrd id = instr (ICall (NewObjRD id))

let instr_newobjs scref = instr (ICall (NewObjS scref))

let instr_lockobj = instr (IMisc LockObj)

let instr_clone = instr (IOp Clone)

let instr_new_record id keys = instr (ILitConst (NewRecord (id, keys)))

let instr_new_recordarray id keys =
  instr (ILitConst (NewRecordArray (id, keys)))

let instr_newstructarray keys = instr (ILitConst (NewStructArray keys))

let instr_newstructdarray keys = instr (ILitConst (NewStructDArray keys))

let instr_newstructdict keys = instr (ILitConst (NewStructDict keys))

let instr_newcol collection_type = instr (ILitConst (NewCol collection_type))

let instr_colfromarray collection_type =
  instr (ILitConst (ColFromArray collection_type))

let instr_entrynop = instr (IBasic EntryNop)

let instr_typedvalue xs = instr (ILitConst (TypedValue xs))

let instr_basel local mode = instr (IBase (BaseL (local, mode)))

let instr_basec stack_index mode = instr (IBase (BaseC (stack_index, mode)))

let instr_basesc y z mode = instr (IBase (BaseSC (y, z, mode)))

let instr_baseh = instr (IBase BaseH)

let instr_cgetcunop = instr (IMisc CGetCUNop)

let instr_ugetcunop = instr (IMisc UGetCUNop)

let instr_memoget label range = instr (IMisc (MemoGet (label, range)))

let instr_memoget_eager label1 label2 range =
  instr (IMisc (MemoGetEager (label1, label2, range)))

let instr_memoset range = instr (IMisc (MemoSet range))

let instr_memoset_eager range = instr (IMisc (MemoSetEager range))

let instr_getmemokeyl local = instr (IMisc (GetMemoKeyL local))

let instr_checkthis = instr (IMisc CheckThis)

let instr_verifyRetTypeC = instr (IMisc VerifyRetTypeC)

let instr_verifyRetTypeTS = instr (IMisc VerifyRetTypeTS)

let instr_verifyOutType i = instr (IMisc (VerifyOutType i))

let instr_dim op key = instr (IBase (Dim (op, key)))

let instr_dim_warn_pt key = instr_dim MemberOpMode.Warn (MemberKey.PT key)

let instr_dim_define_pt key = instr_dim MemberOpMode.Define (MemberKey.PT key)

let instr_fcallclsmethod ?(is_log_as_dynamic_call = LogAsDynamicCall) fcall_args
    =
  instr (ICall (FCallClsMethod (fcall_args, is_log_as_dynamic_call)))

let instr_fcallclsmethodd fcall_args method_name class_name =
  instr (ICall (FCallClsMethodD (fcall_args, class_name, method_name)))

let instr_fcallclsmethods fcall_args scref =
  instr (ICall (FCallClsMethodS (fcall_args, scref)))

let instr_fcallclsmethodsd fcall_args scref method_name =
  instr (ICall (FCallClsMethodSD (fcall_args, scref, method_name)))

let instr_fcallctor fcall_args = instr (ICall (FCallCtor fcall_args))

let instr_fcallfunc fcall_args = instr (ICall (FCallFunc fcall_args))

let instr_fcallfuncd fcall_args id = instr (ICall (FCallFuncD (fcall_args, id)))

let instr_fcallobjmethod fcall_args flavor =
  instr (ICall (FCallObjMethod (fcall_args, flavor)))

let instr_fcallobjmethodd fcall_args method_ flavor =
  instr (ICall (FCallObjMethodD (fcall_args, flavor, method_)))

let instr_fcallobjmethodd_nullthrows fcall_args method_ =
  instr_fcallobjmethodd fcall_args method_ Obj_null_throws

let instr_querym num_params op key =
  instr (IFinal (QueryM (num_params, op, key)))

let instr_querym_cget_pt num_params key =
  instr_querym num_params QueryOp.CGet (MemberKey.PT key)

let instr_setm num_params key = instr (IFinal (SetM (num_params, key)))

let instr_setm_pt num_params key = instr_setm num_params (MemberKey.PT key)

let instr_resolve_func func_id = instr (IOp (ResolveFunc func_id))

let instr_resolve_meth_caller func_id = instr (IOp (ResolveMethCaller func_id))

let instr_resolve_obj_method = instr (IOp ResolveObjMethod)

let instr_resolveclsmethod method_id = instr (IOp (ResolveClsMethod method_id))

let instr_resolveclsmethodd cls_id method_id =
  instr (IOp (ResolveClsMethodD (cls_id, method_id)))

let instr_resolveclsmethods scref method_id =
  instr (IOp (ResolveClsMethodS (scref, method_id)))

let instr_await = instr (IAsync Await)

let instr_yield = instr (IGenerator Yield)

let instr_yieldk = instr (IGenerator YieldK)

let instr_createcont = instr (IGenerator CreateCont)

let instr_awaitall range = instr (IAsync (AwaitAll range))

let instr_awaitall_list unnamed_locals =
  let (head, tail) =
    match unnamed_locals with
    | head :: tail -> (head, tail)
    | _ -> failwith "Expected at least one await"
  in
  let get_unnamed_offset unnamed_local =
    match unnamed_local with
    | Local.Unnamed i -> i
    | _ -> failwith "Expected unnamed local"
  in
  let _ =
    List.fold_left
      tail
      ~init:(get_unnamed_offset head)
      ~f:(fun acc unnamed_local ->
        let next_offset = get_unnamed_offset unnamed_local in
        assert (acc + 1 = next_offset);
        next_offset)
  in
  instr_awaitall (Some (head, List.length unnamed_locals))

let instr_exit = instr (IOp Hhbc_ast.Exit)

let instr_idx = instr (IMisc Idx)

let instr_array_idx = instr (IMisc ArrayIdx)

let instr_fcallbuiltin n un io s = instr (ICall (FCallBuiltin (n, un, io, s)))

let instr_defcls n = instr (IIncludeEvalDefine (DefCls n))

let instr_defclsnop n = instr (IIncludeEvalDefine (DefClsNop n))

let instr_defrecord n = instr (IIncludeEvalDefine (DefRecord n))

let instr_deftypealias n = instr (IIncludeEvalDefine (DefTypeAlias n))

let instr_defcns n = instr (IIncludeEvalDefine (DefCns n))

let instr_eval = instr (IIncludeEvalDefine Eval)

let instr_silence_start local = instr (IMisc (Silence (local, Start)))

let instr_silence_end local = instr (IMisc (Silence (local, End)))

let instr_contAssignDelegate iter =
  instr (IGenDelegation (ContAssignDelegate iter))

let instr_contEnterDelegate = instr (IGenDelegation ContEnterDelegate)

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
  instr_fcallbuiltin 3 3 0 "trigger_sampled_error"

let instr_nativeimpl = instr (IMisc NativeImpl)

let instr_throw_non_exhaustive_switch = instr (IMisc ThrowNonExhaustiveSwitch)

let create_try_catch
    ?opt_done_label ?(skip_throw = false) try_instrs catch_instrs =
  let done_label =
    match opt_done_label with
    | None -> Label.next_regular ()
    | Some l -> l
  in
  gather
    [
      instr (ITry TryCatchBegin);
      try_instrs;
      instr_jmp done_label;
      instr (ITry TryCatchMiddle);
      catch_instrs;
      ( if skip_throw then
        empty
      else
        instr_throw );
      instr (ITry TryCatchEnd);
      instr_label done_label;
    ]

(* Functions on instr_seq that correspond to existing List functions *)
module InstrSeq = struct
  (* f takes an instruction and produces an instruction list to replace it. *)
  let rec flat_map instrseq ~f =
    let flat_map_list items ~f = List.bind items f in
    match instrseq with
    | Instr_empty -> Instr_empty
    | Instr_one x ->
      begin
        match f x with
        | [] -> Instr_empty
        | [x] -> Instr_one x
        | x -> Instr_list x
      end
    | Instr_list instrl -> Instr_list (flat_map_list instrl ~f)
    | Instr_concat instrseql -> Instr_concat (List.map instrseql (flat_map ~f))

  (* f takes an instruction and produces an instrseq to replace it. *)
  let rec flat_map_seq instrseq ~f =
    match instrseq with
    | Instr_empty -> Instr_empty
    | Instr_one x -> f x
    | Instr_list instrl -> Instr_concat (List.map instrl ~f)
    | Instr_concat instrseql ->
      Instr_concat (List.map instrseql (flat_map_seq ~f))

  let rec fold_left instrseq ~f ~init =
    let fold_instrseq init instrseq = fold_left instrseq ~f ~init in
    match instrseq with
    | Instr_empty -> init
    | Instr_one x -> f init x
    | Instr_list instrl -> List.fold_left instrl ~f ~init
    | Instr_concat instrseql -> List.fold_left instrseql ~f:fold_instrseq ~init

  let rec filter_map instrseq ~f =
    match instrseq with
    | Instr_empty -> Instr_empty
    | Instr_one x ->
      begin
        match f x with
        | Some x -> Instr_one x
        | None -> Instr_empty
      end
    | Instr_list instrl -> Instr_list (List.filter_map instrl ~f)
    | Instr_concat instrseql ->
      Instr_concat (List.map instrseql (filter_map ~f))

  let map instrseq ~f = filter_map instrseq ~f:(fun x -> Some (f x))
end

let instr_seq_to_list t =
  (* TODO: Can we eliminate this helper altogether? *)
  (* Could write this in terms of InstrSeq.fold_left *)
  let rec go acc = function
    | [] -> acc
    | s :: sl ->
      (match s with
      | Instr_empty -> go acc sl
      | Instr_one x -> go (x :: acc) sl
      | Instr_list instrl -> go (List.rev_append instrl acc) sl
      | Instr_concat sl' -> go acc (sl' @ sl))
  in
  let rec compact_src_locs acc = function
    | [] -> acc
    | (ISrcLoc _ as i) :: is ->
      begin
        match acc with
        | ISrcLoc _ :: _ -> compact_src_locs acc is
        | _ -> compact_src_locs (i :: acc) is
      end
    | i :: is -> compact_src_locs (i :: acc) is
  in
  go [] [t] |> compact_src_locs []

let get_or_put_label name_label_map name =
  match SMap.find_opt name name_label_map with
  | Some label -> (label, name_label_map)
  | None ->
    let label = Label.next_regular () in
    (label, SMap.add name label name_label_map)

let rewrite_user_labels_instr name_label_map instruction =
  let get_result = get_or_put_label name_label_map in
  match instruction with
  | IContFlow (Jmp (Label.Named name)) ->
    let (label, name_label_map) = get_result name in
    (IContFlow (Jmp label), name_label_map)
  | IContFlow (JmpNS (Label.Named name)) ->
    let (label, name_label_map) = get_result name in
    (IContFlow (JmpNS label), name_label_map)
  | IContFlow (JmpZ (Label.Named name)) ->
    let (label, name_label_map) = get_result name in
    (IContFlow (JmpZ label), name_label_map)
  | IContFlow (JmpNZ (Label.Named name)) ->
    let (label, name_label_map) = get_result name in
    (IContFlow (JmpNZ label), name_label_map)
  | ILabel (Label.Named name) ->
    let (label, name_label_map) = get_result name in
    (ILabel label, name_label_map)
  | i -> (i, name_label_map)

let rewrite_user_labels instrseq =
  let rec aux instrseq name_label_map =
    match instrseq with
    | Instr_empty -> (Instr_empty, name_label_map)
    | Instr_one i ->
      let (i, map) = rewrite_user_labels_instr name_label_map i in
      (Instr_one i, map)
    | Instr_concat l ->
      let (l, name_label_map) =
        List.fold_left
          l
          ~f:(fun (acc, map) s ->
            let (l, map) = aux s map in
            (l :: acc, map))
          ~init:([], name_label_map)
      in
      (Instr_concat (List.rev l), name_label_map)
    | Instr_list l ->
      let (l, name_label_map) =
        List.fold_left
          l
          ~f:(fun (acc, map) i ->
            let (i, map) = rewrite_user_labels_instr map i in
            (i :: acc, map))
          ~init:([], name_label_map)
      in
      (Instr_list (List.rev l), name_label_map)
  in
  fst @@ aux instrseq SMap.empty

let is_srcloc i =
  match i with
  | ISrcLoc _ -> true
  | _ -> false

let first instrs =
  let rec aux instrs =
    match instrs with
    | Instr_empty -> None
    | Instr_one i ->
      if is_srcloc i then
        None
      else
        Some i
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
