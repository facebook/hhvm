(*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*)

open Hhbc_ast
open Hhbc_id
open Instr_utils
module HTC = Hhas_type_constraint
type lazy_instruct = unit -> instruct

let _ = Random.self_init ()

let rand_elt lst =
  if List.length lst < 0 then failwith "Cannot get rand elt of zero length lst";
  let i = Random.int (List.length lst) in
  List.nth lst i

(* Generate random parameters to instructions. In general these are separate
   from their corresponding mutation functions in fuzzer.ml, because those
   are intended to generate values within a certain "distance" of their input,
   while these generate entirely random values *)

let random_mode () : MemberOpMode.t =
  [MemberOpMode.ModeNone; MemberOpMode.Define; MemberOpMode.Warn;
   MemberOpMode.Unset] |> rand_elt

let random_local () : local_id = Local.Unnamed (Random.int 10)
let random_param_id () : param_id = Param_unnamed (Random.int 10)

let random_op_type () : istype_op =
  [OpNull; OpBool; OpInt; OpDbl; OpStr; OpArr; OpObj; OpScalar; OpKeyset;
   OpDict; OpVec] |> rand_elt

let random_eq_op () : eq_op =
  [PlusEqual; MinusEqual; MulEqual; ConcatEqual; DivEqual; PowEqual; ModEqual;
   AndEqual; OrEqual; XorEqual; SlEqual; SrEqual; PlusEqualO; MinusEqualO;
   MulEqualO] |> rand_elt

let random_incdec_op () : incdec_op =
  [PreInc; PostInc; PreDec; PostDec; PreIncO; PostIncO; PreDecO; PostDecO] |>
   rand_elt

let random_flag () : HTC.type_constraint_flag =
  [HTC.Nullable; HTC.HHType; HTC.ExtendedHint; HTC.TypeVar; HTC.Soft;
   HTC.TypeConstant] |> rand_elt

let random_collection_type () : CollectionType.t =
  let open CollectionType in
  [Vector; Map; Set; Pair; ImmVector; ImmMap; ImmSet] |> rand_elt

let random_p_op () : initprop_op = [Static; NonStatic] |> rand_elt

let random_query_op () : QueryOp.t =
  [QueryOp.CGet; QueryOp.CGetQuiet; QueryOp.Isset; QueryOp.Empty] |> rand_elt

let random_bare_op () : bare_this_op = [Notice; NoNotice] |> rand_elt

let random_class_kind () : class_kind = [KClass; KInterface; KTrait] |> rand_elt

let random_silence () : op_silence = [Start; End] |> rand_elt

let random_check () : check_started = [IgnoreStarted; CheckStarted] |> rand_elt

let rec random_typed_value () : Typed_value.t =
 ([(fun () -> Typed_value.Uninit);
   (fun () -> Typed_value.Int (Random.bits () |> Int64.of_int));
   (fun () -> Typed_value.Bool (Random.bool ()));
   (fun () -> Typed_value.Float (Random.float 100.0));
   (fun () -> Typed_value.String "");
   (fun () -> Typed_value.Null);
   (fun () -> Typed_value.Array [random_typed_value (), random_typed_value ()]);
   (fun () -> Typed_value.Vec [random_typed_value ()]);
   (fun () -> Typed_value.Keyset [random_typed_value ()]);
   (fun () -> Typed_value.Dict [random_typed_value (), random_typed_value ()])]
   |> rand_elt) ()

let random_key () : MemberKey.t =
  let open MemberKey in
   ([(fun () -> EC (Random.int 10));
     (fun () -> EL (random_local ())); (fun () -> ET "");
     (fun () -> EI (Random.int64 (Int64.of_int 100)));
     (fun () -> PC (Random.int 10));
     (fun () -> PL (random_local ()));
     (fun () -> PT (Prop.from_raw_string ""));
     (fun () -> QT (Prop.from_raw_string ""));
      fun () -> W]
     |> rand_elt) ()

let random_fault_label () : Label.t = Label.Fault (Random.int 10)
let random_catch_label () : Label.t = Label.Catch (Random.int 10)
let random_adata_id () : adata_id =  "A_" ^ (Random.int 10 |> string_of_int)

(* A list of generators for instructions. Doesn't include all instructions;
 not all can be meaningfully generated in a random fashion.
 TODO: autogenerate this somehow; this doesn't scale well for adding
 instructions at all. Perhaps the project for generating code based on the
 bytecode spec could handle this *)
let all_instrs (fn : IS.t) : lazy_instruct list =
   let cls_ref_slts = IS.get_num_cls_ref_slots fn in
   [(fun () -> IBasic Nop);
    (fun () -> IBasic EntryNop);
    (fun () -> IGet (VGetL (random_local ())));
    (fun () -> ILitConst Null);
    (fun () -> ILitConst True);
    (fun () -> ILitConst (Int (Random.int64 Int64.max_int)));
    (fun () -> ILitConst (Double (Random.float 100.0 |> string_of_float)));
    (fun () -> ILitConst (String ""));
    (fun () -> ILitConst (Array (random_adata_id ())));
    (fun () -> ILitConst (Vec (random_adata_id ())));
    (fun () -> ILitConst (Dict (random_adata_id ())));
    (fun () -> ILitConst (Keyset (random_adata_id ())));
    (fun () -> ILitConst (NewArray (Random.int 1000)));
    (fun () -> ILitConst (NewMixedArray (Random.int 1000)));
    (fun () -> ILitConst (NewDictArray (Random.int 1000)));
    (*(fun () -> ILitConst (NewMIArray (Random.int 1000)));*)
    (*(fun () -> ILitConst (NewMSArray (Random.int 1000)));*)
    (fun () -> ILitConst (NewLikeArrayL (random_local (), Random.int 1000)));
    (fun () -> ILitConst (NewCol (random_collection_type ())));
    (fun () -> ILitConst (Cns (Const.from_raw_string "")));
    (fun () -> ILitConst (CnsE (Const.from_raw_string "")));
    (fun () -> ILitConst (CnsU (Const.from_raw_string "", "")));
    (fun () -> ILitConst (ClsCnsD (Const.from_raw_string "",
                                   Class.from_raw_string "")));
    (fun () -> ILitConst File);
    (fun () -> ILitConst Dir);
    (fun () -> ILitConst Method);
    (fun () -> IGet (CGetL (random_local ())));
    (fun () -> IGet (CGetQuietL (random_local ())));
    (fun () -> IGet (CUGetL (random_local ())));
    (fun () -> IGet (PushL (random_local ())));
    (fun () -> IIsset (IssetL (random_local ())));
    (fun () -> IIsset (EmptyL (random_local ())));
    (fun () -> IIsset (IsTypeL (random_local (),random_op_type ())));
    (fun () -> ILitConst NullUninit);
    (fun () -> IBasic PopC);
    (fun () -> IBasic PopR);
    (fun () -> IBasic PopU);
    (fun () -> IBasic PopV);
    (fun () -> ILitConst (ColFromArray (random_collection_type ())));
    (*(fun () -> IOp Abs);*)
    (*(fun () -> IOp Sqrt);*)
    (fun () -> IOp Not);
    (*(fun () -> IOp Floor);
    (fun () -> IOp Ceil);*)
    (fun () -> IOp CastBool);
    (fun () -> IOp CastInt);
    (fun () -> IOp CastDouble);
    (fun () -> IOp CastString);
    (fun () -> IOp CastArray);
    (fun () -> IOp CastObject);
    (fun () -> IOp CastVec);
    (fun () -> IOp CastDict);
    (fun () -> IOp CastKeyset);
    (fun () -> IOp (InstanceOfD (Class.from_raw_string "")));
    (fun () -> IOp Print);
    (fun () -> IOp Clone);
    (fun () -> IOp Hhbc_ast.Exit);
    (fun () -> IGet (CGetL2 (random_local ())));
    (fun () -> IGet CGetN);
    (fun () -> IGet CGetQuietN);
    (fun () -> IGet CGetG);
    (fun () -> IGet CGetQuietG);
    (*(fun () -> IIsset IssetC);*)
    (fun () -> IIsset IssetN);
    (fun () -> IIsset IssetG);
    (fun () -> IIsset EmptyN);
    (fun () -> IIsset EmptyG);
    (fun () -> IIsset (IsTypeC (random_op_type ())));
    (fun () -> IMisc MaybeMemoType);
    (fun () -> IMisc IsMemoType);
    (fun () -> IMutator (SetL (random_local ())));
    (fun () -> IMutator (SetOpL (random_local (), random_eq_op ())));
    (fun () -> IMutator (SetOpG (random_eq_op ())));
    (fun () -> IMutator (SetOpN (random_eq_op ())));
    (fun () -> IBasic Box);
    (fun () -> IGet VGetN);
    (fun () -> IGet VGetG);
    (fun () -> IBasic Unbox);
    (*(fun () -> IBasic BoxRNop);*)
    (fun () -> IBasic BoxR);
    (fun () -> IBasic UnboxR);
    (fun () -> IBasic UnboxRNop);
    (fun () -> IBasic RGetCNop);
    (fun () -> IMisc UGetCUNop);
    (fun () -> IMisc CGetCUNop);
    (fun () -> IBasic Dup);
    (fun () -> IMisc IsUninit);
    (fun () -> ILitConst AddNewElemC);
    (fun () -> ILitConst NewPair);
    (fun () -> IOp Concat);
    (fun () -> IOp Add);
    (fun () -> IOp Mul);
    (fun () -> IOp Sub);
    (fun () -> IOp AddO);
    (fun () -> IOp SubO);
    (fun () -> IOp MulO);
    (fun () -> IOp Div);
    (fun () -> IOp Mul);
    (fun () -> IOp Pow);
    (fun () -> IOp Xor);
    (fun () -> IOp Same);
    (fun () -> IOp NSame);
    (fun () -> IOp Eq);
    (fun () -> IOp Neq);
    (fun () -> IOp Lt);
    (fun () -> IOp Lte);
    (fun () -> IOp Gt);
    (fun () -> IOp Gte);
    (fun () -> IOp Cmp);
    (fun () -> IOp BitAnd);
    (fun () -> IOp BitOr);
    (fun () -> IOp BitXor);
    (fun () -> IOp BitNot);
    (fun () -> IOp Shl);
    (fun () -> IOp Shr);
    (fun () -> IOp InstanceOf);
    (fun () -> IMutator SetN);
    (fun () -> IMutator SetG);
    (fun () -> ILitConst AddNewElemV);
    (fun () -> ILitConst AddElemC);
    (fun () -> ILitConst AddElemV);
    (fun () -> IMutator (IncDecL (random_local (), random_incdec_op ())));
    (fun () -> IMutator (IncDecG (random_incdec_op ())));
    (fun () -> IMutator (IncDecN (random_incdec_op ())));
    (fun () -> IMutator (BindL (random_local ())));
    (fun () -> IMutator BindN);
    (fun () -> IMutator BindG);
    (fun () -> IMutator (UnsetL (random_local ())));
    (fun () -> IMutator UnsetN);
    (fun () -> IMutator UnsetG);
    (fun () -> IMutator (CheckProp (Prop.from_raw_string "")));
    (fun () -> IMutator (InitProp (Prop.from_raw_string "", random_p_op ())));
    (fun () -> ICall CufSafeArray);
    (fun () -> ICall CufSafeReturn);
    (fun () -> IIncludeEvalDefine Incl);
    (fun () -> IIncludeEvalDefine InclOnce);
    (fun () -> IIncludeEvalDefine Req);
    (fun () -> IIncludeEvalDefine ReqOnce);
    (fun () -> IIncludeEvalDefine ReqDoc);
    (fun () -> IIncludeEvalDefine Eval);
    (fun () -> IIncludeEvalDefine (AliasCls ("", "")));
    (fun () -> IIncludeEvalDefine (DefFunc (Random.int 10)));
    (fun () -> IIncludeEvalDefine (DefCls (Random.int 10)));
    (fun () -> IIncludeEvalDefine (DefClsNop (Random.int 10)));
    (fun () -> IIncludeEvalDefine (DefCns (Const.from_raw_string "")));
    (fun () -> IIncludeEvalDefine (DefTypeAlias (Random.int 10)));
    (fun () -> IMisc This);
    (fun () -> IMisc (BareThis (random_bare_op())));
    (fun () -> IMisc CheckThis);
    (fun () -> IMisc (InitThisLoc (random_local ())));
    (fun () -> IMisc (StaticLocCheck (random_local (), "")));
    (fun () -> IMisc (StaticLocDef (random_local (), "")));
    (fun () -> IMisc (StaticLocInit (random_local (), "")));
    (fun () -> IMisc Catch);
    (fun () -> IMisc (OODeclExists (random_class_kind ())));
    (fun () -> IMisc (VerifyParamType (random_param_id ())));
    (fun () -> IMisc VerifyRetTypeC);
    (fun () -> IMisc VerifyRetTypeV);
    (*(fun () -> IMisc NativeImpl);*)
    (*(fun () -> IMisc (IncStat (Random.int 100, Random.int 100)));*)
    (fun () -> IMisc AKExists);
    (fun () -> IMisc (CreateCl (Random.int 10, Random.int 10)));
    (fun () -> IMisc Idx);
    (fun () -> IMisc ArrayIdx);
    (*(fun () -> IMisc BreakTraceHint);*)
    (fun () -> IMisc (Silence (random_local (), random_silence ())));
    (fun () -> IMisc (GetMemoKeyL (random_local ())));
    (*(fun () -> IMisc VarEnvDynCall);*)
    (fun () -> IMisc (MemoSet (Random.int 10, random_local (), Random.int 10)));
    (fun () -> IMisc (MemoGet (Random.int 10, random_local (), Random.int 10)));
    (fun () -> IAsync WHResult);
    (fun () -> IAsync Await)] @
    begin
      if cls_ref_slts <= 0 then [] else
      [(fun () -> IMisc (Self (Random.int cls_ref_slts)));
       (fun () -> IIsset (IssetS (Random.int cls_ref_slts)));
       (fun () -> IIsset (EmptyS (Random.int cls_ref_slts)));
       (fun () -> IMisc (Parent (Random.int cls_ref_slts)));
       (fun () -> IMisc (LateBoundCls (Random.int cls_ref_slts)));
       (fun () -> IMutator (SetOpS (random_eq_op (), Random.int cls_ref_slts)));
       (fun () -> IGet (CGetS (Random.int cls_ref_slts)));
       (fun () -> IGet (VGetS (Random.int cls_ref_slts)));
       (fun () -> ILitConst (ClsCns ((Const.from_raw_string ""),
                                      Random.int cls_ref_slts)));
       (fun () -> IMutator (SetS (Random.int cls_ref_slts)));
       (fun () -> IMisc (ClsRefName (Random.int cls_ref_slts)));
       (fun () -> IMutator (IncDecS (random_incdec_op (),
                                     Random.int cls_ref_slts)));
       (fun () -> IGet (ClsRefGetC (Random.int cls_ref_slts)));
       (fun () -> IMutator (BindS (Random.int cls_ref_slts)));
       (fun () -> IGet (ClsRefGetL (random_local (), Random.int cls_ref_slts)))]
    end

(* Generators for base instructions *)
let base_instrs (fn : IS.t) : lazy_instruct list =
  let cls_ref_slts = IS.get_num_cls_ref_slots fn in
  [(fun () -> IBase (BaseNC (Random.int 10, random_mode ())));
   (fun () -> IBase (BaseNL (random_local (), random_mode ())));
   (fun () -> IBase (FPassBaseNC (Random.int 10, Random.int 10)));
   (fun () -> IBase (FPassBaseNL (Random.int 10, random_local ())));
   (fun () -> IBase (BaseGC (Random.int 10, random_mode ())));
   (fun () -> IBase (BaseGL (random_local (), random_mode ())));
   (fun () -> IBase (FPassBaseGC (Random.int 10, Random.int 10)));
   (fun () -> IBase (FPassBaseGL (Random.int 10, random_local ())));
   (fun () -> IBase (BaseL (random_local (), random_mode ())));
   (fun () -> IBase (FPassBaseL (Random.int 10, random_local ())));
   (fun () -> IBase (BaseC (Random.int 10)));
   (fun () -> IBase (BaseR (Random.int 10)));
   (fun () -> IBase BaseH);
   (fun () -> IBase (Dim (random_mode(), random_key ())));
   (fun () -> IBase (FPassDim (Random.int 10, random_key())))] @
   begin
     if cls_ref_slts <= 0 then [] else
     [(fun () -> IBase (BaseSC (Random.int 10, Random.int 10)));
     (fun () -> IBase (BaseSL (random_local (), Random.int 10)))]
    end

(* Generators for final instructions *)
let final_instrs (_ : IS.t) : lazy_instruct list =
  [(fun () -> IFinal (QueryM (Random.int 10,
    random_query_op (), random_key ())));
   (fun () -> IFinal (VGetM (Random.int 10, random_key ())));
   (fun () -> IFinal (FPassM (Random.int 10, Random.int 10, random_key ())));
   (fun () -> IFinal (SetM (Random.int 10, random_key ())));
   (fun () -> IFinal (IncDecM (Random.int 10,
     random_incdec_op (), random_key ())));
   (fun () -> IFinal (SetOpM (Random.int 10,
     random_eq_op (), random_key ())));
   (fun () -> IFinal (BindM (Random.int 10, random_key ())));
   (fun () -> IFinal (UnsetM (Random.int 10, random_key ())));
   (fun () -> IFinal (SetWithRefLML (random_local (), random_local())));
   (fun () -> IFinal (SetWithRefRML (random_local ())))]

(* Generators for FPass* instructions *)
let fpass_instrs (fn : IS.t) : lazy_instruct list =
  let cls_ref_slts = IS.get_num_cls_ref_slots fn in
  [(fun () -> ICall (FPassCW (Random.int 10)));
   (fun () -> ICall (FPassCE (Random.int 10)));
   (fun () -> ICall (FPassV (Random.int 10)));
   (fun () -> ICall (FPassVNop (Random.int 10)));
   (fun () -> ICall (FPassR (Random.int 10)));
   (fun () -> ICall (FPassL (Random.int 10, random_local ())));
   (fun () -> ICall (FPassN (Random.int 10)));
   (fun () -> ICall (FPassG (Random.int 10)));
   (fun () -> ICall (FPassS (Random.int 10, Random.int 10)))] @
   begin
     if cls_ref_slts <= 0 then [] else
     [(fun () -> ICall (FPassC (Random.int 10)))]
   end

(* An association list of stack signatures to random generators for
    instructions with that stack signature, produced from input list of
    generators *)
let by_signature (gens : lazy_instruct list) :
                                      (stack_sig * lazy_instruct list) list =
  let add acc elem =
    let dat = elem () |> stk_data in
    let old = if List.mem_assoc dat acc then List.assoc dat acc else [] in
    (dat, elem :: old) :: List.remove_assoc dat acc in
  List.fold_left add [] gens

let sig_map_all   fn  = all_instrs   fn |> by_signature
let sig_map_base  fn  = base_instrs  fn |> by_signature
let sig_map_final fn  = final_instrs fn |> by_signature
let sig_map_fpass fn  = fpass_instrs fn |> by_signature
