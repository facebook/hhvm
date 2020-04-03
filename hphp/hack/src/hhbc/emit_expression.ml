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
open Instruction_sequence
open Ast_class_expr
open Emit_pos
module H = Hhbc_ast
module TC = Hhas_type_constraint
module SN = Naming_special_names
module SU = Hhbc_string_utils
module ULS = Unique_list_string
module Opts = Hhbc_options

let can_inline_gen_functions () =
  not (Opts.jit_enable_rename_function !Opts.compiler_options)

let max_array_elem_on_stack () =
  Hhbc_options.max_array_elem_size_on_the_stack !Hhbc_options.compiler_options

type emit_jmp_result = {
  (* generated instruction sequence *)
  instrs: Instruction_sequence.t;
  (* does instruction sequence fall through *)
  is_fallthrough: bool;
  (* was label associated with emit operation used *)
  is_label_used: bool;
}

(* Locals, array elements, and properties all support the same range of l-value
 * operations. *)
module LValOp = struct
  type t =
    | Set
    | SetOp of eq_op
    | IncDec of incdec_op
    | Unset
end

let jit_enable_rename_function () =
  Hhbc_options.jit_enable_rename_function !Hhbc_options.compiler_options

let is_local_this env (lid : Aast.local_id) : bool =
  let id = Local_id.get_name lid in
  let scope = Emit_env.get_scope env in
  id = SN.SpecialIdents.this
  && Ast_scope.Scope.has_this scope
  && not (Ast_scope.Scope.is_toplevel scope)

module InoutLocals = struct
  (* for every local that appear as a part of inout argument and also mutated inside
     argument list this record stores:
     - position of the first argument when local appears as inout
     - position of the last argument where local is mutated.
     Within the this range at every usage of the local must be captured to make sure
     that later when inout arguments will be written back the same value of the
     local will be used *)
  type alias_info = {
    first_inout: int;
    last_write: int;
    num_uses: int;
  }

  let not_aliased =
    { first_inout = Int.max_value; last_write = Int.min_value; num_uses = 0 }

  let add_inout i r =
    if i < r.first_inout then
      { r with first_inout = i }
    else
      r

  let add_write i r =
    if i > r.last_write then
      { r with last_write = i }
    else
      r

  let add_use _i r = { r with num_uses = r.num_uses + 1 }

  let in_range i r = i > r.first_inout || i <= r.last_write

  let has_single_ref r = r.num_uses < 2

  let update name i f m =
    let r = SMap.find_opt name m |> Option.value ~default:not_aliased |> f i in
    SMap.add name r m

  let add_write name i m = update (Local_id.get_name name) i add_write m

  let add_inout name i m = update (Local_id.get_name name) i add_inout m

  let add_use name i m = update (Local_id.get_name name) i add_use m

  let collect_written_variables env (args : Tast.expr list) : alias_info SMap.t
      =
    (* check value of the argument *)
    let rec handle_arg ~is_top i acc (arg : Tast.expr) =
      match snd arg with
      (* inout $v *)
      | A.Callconv (Ast_defs.Pinout, (_, A.Lvar (_, id)))
        when not (is_local_this env id) ->
        let acc = add_use id i acc in
        if is_top then
          add_inout id i acc
        else
          add_write id i acc
      (* $v *)
      | A.Lvar (_, id) ->
        let acc = add_use id i acc in
        acc
      | _ ->
        (* dive into argument value *)
        dive i acc arg
    (* collect lvars on the left hand side of '=' operator *)
    and collect_lvars_lhs i acc (e : Tast.expr) =
      match snd e with
      | A.Lvar (_, id) when not (is_local_this env id) ->
        let acc = add_use id i acc in
        add_write id i acc
      | A.List exprs -> List.fold_left exprs ~f:(collect_lvars_lhs i) ~init:acc
      | _ -> acc
    (* descend into expression *)
    and dive i (acc : alias_info SMap.t) expr : alias_info SMap.t =
      let state = ref acc in
      let visitor =
        object
          inherit [_] A.iter as super

          (* lhs op= _ *)
          method! on_Binop _ bop l r =
            let _ =
              match bop with
              | Ast_defs.Eq _ -> state := collect_lvars_lhs i !state l
              | _ -> ()
            in
            super#on_Binop () bop l r

          (* $i++ or $i-- *)
          method! on_Unop _ op e =
            let _ =
              match op with
              | Ast_defs.Uincr
              | Ast_defs.Udecr ->
                state := collect_lvars_lhs i !state e
              | _ -> ()
            in
            super#on_Unop () op e

          (* $v *)
          method! on_Lvar _ (_p, id) =
            let _ = state := add_use id 0 !state in
            super#on_Lvar () (_p, id)

          (* f(inout $v) or f(&$v) *)
          method! on_Call _ _ _ _ args uarg =
            let f = handle_arg ~is_top:false i in
            List.iter args ~f:(fun arg -> state := f !state arg);
            Option.iter uarg ~f:(fun arg -> state := f !state arg)
        end
      in
      visitor#on_expr () expr;
      !state
    in
    List.foldi args ~f:(handle_arg ~is_top:true) ~init:SMap.empty

  (* determines if value of a local 'name' that appear in parameter 'i'
     should be saved to local because it might be overwritten later *)
  let should_save_local_value name i aliases =
    Option.value_map ~default:false ~f:(in_range i) (SMap.find_opt name aliases)

  let should_move_local_value local aliases =
    match local with
    | Local.Named name ->
      Option.value_map
        ~default:true
        ~f:has_single_ref
        (SMap.find_opt name aliases)
    | Local.Unnamed _ -> false
end

(* Describes what kind of value is intended to be stored in local *)
type stored_value_kind =
  | Value_kind_local
  | Value_kind_expression

(* represents sequence of instructions interleaved with temp locals.
   <i, None :: rest> - is emitted i :: <rest> (commonly used for final instructions in sequence)
   <i, Some (l, local_kind) :: rest> is emitted as

   i
   .try {
     setl/popl l; depending on local_kind
     <rest>
   } .catch {
     unset l
     throw
   }
   unsetl l
   *)
type instruction_sequence_with_locals =
  (Instruction_sequence.t * (Local.t * stored_value_kind) option) list

(* converts instruction_sequence_with_locals of loads into instruction_sequence.t *)
let rebuild_load_store load store =
  let rec aux = function
    | [] -> ([], [])
    | (i, None) :: xs ->
      let (ld, st) = aux xs in
      (i :: ld, st)
    | (i, Some (l, kind)) :: xs ->
      let (ld, st) = aux xs in
      let set =
        if kind = Value_kind_expression then
          instr_setl l
        else
          instr_popl l
      in
      let unset = instr_unsetl l in
      (i :: set :: ld, unset :: st)
  in
  let (ld, st) = aux load in
  (gather ld, gather (store :: st))

(* result of emit_array_get *)
type array_get_instr =
  (* normal $a[..] that does not need to spill anything*)
  | Array_get_regular of Instruction_sequence.t
  (* subscript expression used as inout argument that need to spill intermediate
     values:
     load - instruction_sequence_with_locals to load value
     store - instruction to set value back (can use locals defined in load part)
  *)
  | Array_get_inout of {
      load: instruction_sequence_with_locals;
      store: Instruction_sequence.t;
    }

type 'a array_get_base_data = {
  base_instrs: 'a;
  cls_instrs: Instruction_sequence.t;
  setup_instrs: Instruction_sequence.t;
  base_stack_size: int;
  cls_stack_size: int;
}

(* result of emit_base *)
type array_get_base =
  (* normal <base> part in <base>[..] that does not need to spill anything *)
  | Array_get_base_regular of Instruction_sequence.t array_get_base_data
  (* base of subscript expression used as inout argument that need to spill
    intermediate values *)
  | Array_get_base_inout of {
      (* instructions to load base part *)
      load: instruction_sequence_with_locals array_get_base_data;
      (* instruction to load base part for setting inout argument back *)
      store: Instruction_sequence.t;
    }

let is_incdec op =
  match op with
  | LValOp.IncDec _ -> true
  | _ -> false

let is_global_namespace env =
  Namespace_env.is_global_namespace (Emit_env.get_namespace env)

let enable_intrinsics_extension () =
  Hhbc_options.enable_intrinsics_extension !Hhbc_options.compiler_options

let optimize_null_checks () =
  Hhbc_options.optimize_null_checks !Hhbc_options.compiler_options

let hack_arr_compat_notices () =
  Hhbc_options.hack_arr_compat_notices !Hhbc_options.compiler_options

let hack_arr_dv_arrs () =
  Hhbc_options.hack_arr_dv_arrs !Hhbc_options.compiler_options

let php7_ltr_assign () =
  Hhbc_options.php7_ltr_assign !Hhbc_options.compiler_options

let widen_is_array () =
  Hhbc_options.widen_is_array !Hhbc_options.compiler_options

(* Strict binary operations; assumes that operands are already on stack *)
let from_binop op =
  let check_int_overflow =
    Hhbc_options.check_int_overflow !Hhbc_options.compiler_options
  in
  match op with
  | Ast_defs.Plus ->
    instr
      (IOp
         ( if check_int_overflow then
           AddO
         else
           Add ))
  | Ast_defs.Minus ->
    instr
      (IOp
         ( if check_int_overflow then
           SubO
         else
           Sub ))
  | Ast_defs.Star ->
    instr
      (IOp
         ( if check_int_overflow then
           MulO
         else
           Mul ))
  | Ast_defs.Slash -> instr (IOp Div)
  | Ast_defs.Eqeq -> instr (IOp Eq)
  | Ast_defs.Eqeqeq -> instr (IOp Same)
  | Ast_defs.Starstar -> instr (IOp Pow)
  | Ast_defs.Diff -> instr (IOp Neq)
  | Ast_defs.Diff2 -> instr (IOp NSame)
  | Ast_defs.Lt -> instr (IOp Lt)
  | Ast_defs.Lte -> instr (IOp Lte)
  | Ast_defs.Gt -> instr (IOp Gt)
  | Ast_defs.Gte -> instr (IOp Gte)
  | Ast_defs.Dot -> instr (IOp Concat)
  | Ast_defs.Amp -> instr (IOp BitAnd)
  | Ast_defs.Bar -> instr (IOp BitOr)
  | Ast_defs.Ltlt -> instr (IOp Shl)
  | Ast_defs.Gtgt -> instr (IOp Shr)
  | Ast_defs.Cmp -> instr (IOp Cmp)
  | Ast_defs.Percent -> instr (IOp Mod)
  | Ast_defs.Xor -> instr (IOp BitXor)
  | Ast_defs.LogXor -> instr (IOp Xor)
  | Ast_defs.Eq _ -> failwith "assignment is emitted differently"
  | Ast_defs.QuestionQuestion ->
    failwith "null coalescence is emitted differently"
  | Ast_defs.Ampamp
  | Ast_defs.Barbar ->
    failwith "short-circuiting operator cannot be generated as a simple binop"

let binop_to_eqop op =
  let check_int_overflow =
    Hhbc_options.check_int_overflow !Hhbc_options.compiler_options
  in
  match op with
  | Ast_defs.Plus ->
    Some
      ( if check_int_overflow then
        PlusEqualO
      else
        PlusEqual )
  | Ast_defs.Minus ->
    Some
      ( if check_int_overflow then
        MinusEqualO
      else
        MinusEqual )
  | Ast_defs.Star ->
    Some
      ( if check_int_overflow then
        MulEqualO
      else
        MulEqual )
  | Ast_defs.Slash -> Some DivEqual
  | Ast_defs.Starstar -> Some PowEqual
  | Ast_defs.Amp -> Some AndEqual
  | Ast_defs.Bar -> Some OrEqual
  | Ast_defs.Xor -> Some XorEqual
  | Ast_defs.Ltlt -> Some SlEqual
  | Ast_defs.Gtgt -> Some SrEqual
  | Ast_defs.Percent -> Some ModEqual
  | Ast_defs.Dot -> Some ConcatEqual
  | _ -> None

let unop_to_incdec_op op =
  let check_int_overflow =
    Hhbc_options.check_int_overflow !Hhbc_options.compiler_options
  in
  match op with
  | Ast_defs.Uincr ->
    if check_int_overflow then
      PreIncO
    else
      PreInc
  | Ast_defs.Udecr ->
    if check_int_overflow then
      PreDecO
    else
      PreDec
  | Ast_defs.Upincr ->
    if check_int_overflow then
      PostIncO
    else
      PostInc
  | Ast_defs.Updecr ->
    if check_int_overflow then
      PostDecO
    else
      PostDec
  | _ -> failwith "invalid incdec op"

let istype_op id =
  match id with
  | "is_int"
  | "is_integer"
  | "is_long" ->
    Some OpInt
  | "is_bool" -> Some OpBool
  | "is_float"
  | "is_real"
  | "is_double" ->
    Some OpDbl
  | "is_string" -> Some OpStr
  | "is_array" ->
    Some
      begin
        if widen_is_array () then
          OpArrLike
        else
          OpArr
      end
  | "is_object" -> Some OpObj
  | "is_null" -> Some OpNull
  (* We don't use IsType with the resource type because `is_resource()` does
     validation in addition to a simple type check. We will use it for
     is-expressions because they only do type checks.
  | "is_resource" -> Some OpRes *)
  | "is_scalar" -> Some OpScalar
  | "HH\\is_keyset" -> Some OpKeyset
  | "HH\\is_dict" -> Some OpDict
  | "HH\\is_vec" -> Some OpVec
  | "HH\\is_varray" ->
    Some
      ( if hack_arr_dv_arrs () then
        OpVec
      else
        OpVArray )
  | "HH\\is_darray" ->
    Some
      ( if hack_arr_dv_arrs () then
        OpDict
      else
        OpDArray )
  | "HH\\is_any_array" -> Some OpArrLike
  | "HH\\is_class_meth" -> Some OpClsMeth
  | "HH\\is_fun" -> Some OpFunc
  | "HH\\is_php_array" -> Some OpPHPArr
  | _ -> None

let is_isexp_op lower_fq_id : Aast.hint option =
  let h n = (Pos.none, Aast.Happly ((Pos.none, n), [])) in
  match lower_fq_id with
  | "is_int"
  | "is_integer"
  | "is_long" ->
    Some (h "\\HH\\int")
  | "is_bool" -> Some (h "\\HH\\bool")
  | "is_float"
  | "is_real"
  | "is_double" ->
    Some (h "\\HH\\float")
  | "is_string" -> Some (h "\\HH\\string")
  | "is_null" -> Some (h "\\HH\\void")
  | "HH\\is_keyset" -> Some (h "\\HH\\keyset")
  | "HH\\is_dict" -> Some (h "\\HH\\dict")
  | "HH\\is_vec" -> Some (h "\\HH\\vec")
  | _ -> None

let get_queryMOpMode op =
  match op with
  | QueryOp.InOut -> MemberOpMode.InOut
  | QueryOp.CGet -> MemberOpMode.Warn
  | _ -> MemberOpMode.ModeNone

(* Returns either Some (index, is_soft) or None *)
let is_reified_tparam ~(is_fun : bool) (env : Emit_env.t) (name : string) =
  let scope = Emit_env.get_scope env in
  let tparams =
    if is_fun then
      Ast_scope.Scope.get_fun_tparams scope
    else
      (Ast_scope.Scope.get_class_tparams scope).A.c_tparam_list
  in
  let is_soft =
    List.exists ~f:(function { A.ua_name = n; _ } ->
        snd n = SN.UserAttributes.uaSoft)
  in
  List.find_mapi
    tparams
    ~f:(fun i
            {
              A.tp_name = (_, id);
              A.tp_reified = reified;
              A.tp_user_attributes = ual;
              _;
            }
            ->
      if (reified = A.Reified || reified = A.SoftReified) && id = name then
        Some (i, is_soft ual)
      else
        None)

let extract_shape_field_name_pstring env annot = function
  | Ast_defs.SFlit_int s -> A.Int (snd s)
  | Ast_defs.SFlit_str s -> A.String (snd s)
  | Ast_defs.SFclass_const (((pn, name) as id), p) ->
    if
      Option.is_some (is_reified_tparam ~is_fun:true env name)
      || Option.is_some (is_reified_tparam ~is_fun:false env name)
    then
      Emit_fatal.raise_fatal_parse
        pn
        "Reified generics cannot be used in shape keys";
    A.Class_const ((annot, A.CI id), p)

let rec text_of_expr (e : Tast.expr) =
  match e with
  (* Note we force string literals to become single-quoted, regardless of
     whether they were single- or double-quoted in the source. Gross. *)
  | (_, A.String s) -> "'" ^ s ^ "'"
  | (_, A.Id (_, id)) -> id
  | (_, A.Lvar (_, id)) -> Local_id.get_name id
  | (_, A.Array_get ((_, A.Lvar (_, id)), Some e)) ->
    Local_id.get_name id ^ "[" ^ text_of_expr e ^ "]"
  | _ ->
    (* TODO: get text of expression *)
    "unknown"

let text_of_class_id (cid : Tast.class_id) =
  match snd cid with
  | A.CIparent -> "parent"
  | A.CIself -> "self"
  | A.CIstatic -> "static"
  | A.CIexpr e -> text_of_expr e
  | A.CI (_, id) -> id

let text_of_prop (prop : Tast.class_get_expr) =
  match prop with
  | A.CGstring (_, s) -> s
  | A.CGexpr e -> text_of_expr e

let from_ast_null_flavor = function
  | A.OG_nullsafe -> Hhbc_ast.Obj_null_safe
  | A.OG_nullthrows -> Hhbc_ast.Obj_null_throws

let parse_include (e : Tast.expr) =
  let strip_backslash p =
    let len = String.length p in
    if len > 0 && p.[0] = '/' then
      String.sub p 1 (len - 1)
    else
      p
  in
  let rec split_var_lit = function
    | (_, A.Binop (Ast_defs.Dot, e1, e2)) ->
      let (v, l) = split_var_lit e2 in
      if v = "" then
        let (var, lit) = split_var_lit e1 in
        (var, lit ^ l)
      else
        (v, "")
    | (_, A.String lit) -> ("", lit)
    | e -> (text_of_expr e, "")
  in
  let (var, lit) = split_var_lit e in
  let (var, lit) =
    if var = SN.PseudoConsts.g__DIR__ then
      ("", strip_backslash lit)
    else
      (var, lit)
  in
  if var = "" then
    if Filename.is_relative lit then
      Hhas_symbol_refs.SearchPathRelative lit
    else
      Hhas_symbol_refs.Absolute lit
  else
    Hhas_symbol_refs.IncludeRootRelative (var, strip_backslash lit)

let rec expr_and_new env pos instr_to_add_new instr_to_add = function
  | A.AFvalue e -> gather [emit_expr env e; emit_pos pos; instr_to_add_new]
  | A.AFkvalue (k, v) ->
    gather [emit_two_exprs env (fst @@ fst k) k v; instr_to_add]

and get_local env (pos, (str : string)) : Hhbc_ast.local_id =
  if str = SN.SpecialIdents.dollardollar then
    match Emit_env.get_pipe_var env with
    | None ->
      Emit_fatal.raise_fatal_runtime
        pos
        "Pipe variables must occur only in the RHS of pipe expressions"
    | Some v -> v
  else if SN.SpecialIdents.is_tmp_var str then
    Local.get_unnamed_local_for_tempname str
  else
    Local.Named str

and emit_local ~notice env (lid : Aast.lid) =
  let (pos, id) = lid in
  let str = Local_id.get_name id in
  if SN.Superglobals.globals = str then
    Emit_fatal.raise_fatal_parse pos "Access $GLOBALS via wrappers"
  else if SN.Superglobals.is_superglobal str then
    gather
      [
        instr_string (SU.Locals.strip_dollar str);
        emit_pos pos;
        instr (IGet CGetG);
      ]
  else
    let local = get_local env (pos, str) in
    if is_local_this env id && not (Emit_env.get_needs_local_this env) then
      emit_pos_then pos @@ instr (IMisc (BareThis notice))
    else
      instr_cgetl local

(* Emit CGetL2 for local variables, and return true to indicate that
 * the result will be just below the top of the stack *)
and emit_first_expr env (expr : Tast.expr) =
  match snd expr with
  | A.Lvar (pos, id)
    when not
           ( (is_local_this env id && not (Emit_env.get_needs_local_this env))
           || Local_id.get_name id = SN.Superglobals.globals
           || SN.Superglobals.is_superglobal (Local_id.get_name id) ) ->
    (instr_cgetl2 (get_local env (pos, Local_id.get_name id)), true)
  | _ -> (emit_expr env expr, false)

(* Special case for binary operations to make use of CGetL2 *)
and emit_two_exprs env (outer_pos : Pos.t) (e1 : Tast.expr) (e2 : Tast.expr) =
  let (instrs1, is_under_top) = emit_first_expr env e1 in
  let instrs2 = emit_expr env e2 in
  let instrs2_is_var =
    match e2 with
    | (_, A.Lvar _) -> true
    | _ -> false
  in
  gather
  @@
  if is_under_top then
    if instrs2_is_var then
      [emit_pos outer_pos; instrs2; instrs1]
    else
      [instrs2; emit_pos outer_pos; instrs1]
  else if instrs2_is_var then
    [instrs1; emit_pos outer_pos; instrs2]
  else
    [instrs1; instrs2; emit_pos outer_pos]

and emit_is_null env (e : Tast.expr) =
  match e with
  | (_, A.Lvar (pos, id)) when not (is_local_this env id) ->
    instr_istypel (get_local env (pos, Local_id.get_name id)) OpNull
  | _ -> gather [emit_expr env e; instr_istypec OpNull]

and emit_binop env annot op (e1 : Tast.expr) (e2 : Tast.expr) =
  let (pos, _) = annot in
  let default () = gather [emit_two_exprs env pos e1 e2; from_binop op] in
  match op with
  | Ast_defs.Ampamp
  | Ast_defs.Barbar ->
    emit_short_circuit_op env annot (A.Binop (op, e1, e2))
  | Ast_defs.Eq None -> emit_lval_op env pos LValOp.Set e1 (Some e2)
  | Ast_defs.Eq (Some Ast_defs.QuestionQuestion) ->
    emit_null_coalesce_assignment env pos e1 e2
  | Ast_defs.Eq (Some obop) ->
    begin
      match binop_to_eqop obop with
      | None -> failwith "illegal eq op"
      | Some op -> emit_lval_op env pos (LValOp.SetOp op) e1 (Some e2)
    end
  | Ast_defs.QuestionQuestion ->
    let end_label = Label.next_regular () in
    gather
      [
        fst (emit_quiet_expr env pos e1);
        instr_dup;
        instr_istypec OpNull;
        instr_not;
        instr_jmpnz end_label;
        instr_popc;
        emit_expr env e2;
        instr_label end_label;
      ]
  | _ ->
    if not (optimize_null_checks ()) then
      default ()
    else (
      match op with
      | Ast_defs.Eqeqeq when snd e2 = A.Null -> emit_is_null env e1
      | Ast_defs.Eqeqeq when snd e1 = A.Null -> emit_is_null env e2
      | Ast_defs.Diff2 when snd e2 = A.Null ->
        gather [emit_is_null env e1; instr_not]
      | Ast_defs.Diff2 when snd e1 = A.Null ->
        gather [emit_is_null env e2; instr_not]
      | _ -> default ()
    )

and get_type_structure_for_hint ~targ_map ~tparams (h : Aast.hint) =
  let tv = Emit_type_constant.hint_to_type_constant ~tparams ~targ_map h in
  let i = Emit_adata.get_array_identifier tv in
  if hack_arr_dv_arrs () then
    instr (ILitConst (Dict i))
  else
    instr (ILitConst (Array i))

(* NOTE: Make sure the type structure retrieval code is synced with emit_is. *)
and emit_as env pos e h is_nullable =
  Local.scope @@ fun () ->
  let arg_local = Local.get_unnamed_local () in
  let type_struct_local = Local.get_unnamed_local () in
  let (ts_instrs, is_static) = emit_reified_arg env ~isas:true pos h in
  let then_label = Label.next_regular () in
  let done_label = Label.next_regular () in
  let main_block ts_instrs resolve =
    gather
      [
        ts_instrs;
        instr_setl type_struct_local;
        instr_istypestructc resolve;
        instr_jmpnz then_label;
        ( if is_nullable then
          gather [instr_null; instr_jmp done_label]
        else
          gather
            [
              instr_pushl arg_local;
              instr_pushl type_struct_local;
              instr_throwastypestructexception;
            ] );
      ]
  in
  (* Set aside the argument value. *)
  gather
    [
      emit_expr env e;
      instr_setl arg_local;
      (* Store type struct in a variable and reuse it. *)
      ( if is_static then
        main_block
          (get_type_structure_for_hint ~targ_map:SMap.empty ~tparams:[] h)
          Resolve
      else
        main_block ts_instrs DontResolve );
      instr_label then_label;
      instr_pushl arg_local;
      instr_unsetl type_struct_local;
      instr_label done_label;
    ]

and emit_is env pos (h : Aast.hint) =
  let (ts_instrs, is_static) = emit_reified_arg env ~isas:true pos h in
  if is_static then
    match snd h with
    | Aast.Happly ((_, id), []) when SU.strip_hh_ns id = SN.Typehints.this ->
      instr_islateboundcls
    | _ ->
      gather
        [
          get_type_structure_for_hint ~targ_map:SMap.empty ~tparams:[] h;
          instr_istypestructc Resolve;
        ]
  else
    gather [ts_instrs; instr_istypestructc DontResolve]

and emit_cast env pos hint expr =
  let op =
    match hint with
    | Aast.Happly ((_, id), []) ->
      let id = SU.strip_ns id in
      let id = SU.strip_hh_ns id in
      begin
        match id with
        | _ when id = SN.Typehints.int -> instr (IOp CastInt)
        | _ when id = SN.Typehints.bool -> instr (IOp CastBool)
        | _ when id = SN.Typehints.string -> instr (IOp CastString)
        | _ when id = SN.Typehints.array -> instr (IOp CastArray)
        | _ when id = SN.Typehints.float -> instr (IOp CastDouble)
        | _ ->
          Emit_fatal.raise_fatal_parse
            pos
            ("Invalid cast type: " ^ SU.strip_global_ns id)
      end
    | _ -> Emit_fatal.raise_fatal_parse pos "Invalid cast type"
  in
  gather [emit_expr env expr; emit_pos pos; op]

and emit_conditional_expression
    env pos (etest : Tast.expr) (etrue : Tast.expr option) (efalse : Tast.expr)
    =
  match etrue with
  | Some etrue ->
    let false_label = Label.next_regular () in
    let end_label = Label.next_regular () in
    let r = emit_jmpz env etest false_label in
    gather
      [
        r.instrs;
        (* only emit true branch if there is fallthrough from condition *)
        begin
          if r.is_fallthrough then
            gather [emit_expr env etrue; emit_pos pos; instr_jmp end_label]
          else
            empty
        end;
        (* only emit false branch if false_label is used *)
        begin
          if r.is_label_used then
            gather [instr_label false_label; emit_expr env efalse]
          else
            empty
        end;
        (* end_label is used to jump out of true branch so they should be emitted
         together *)
        begin
          if r.is_fallthrough then
            instr_label end_label
          else
            empty
        end;
      ]
  | None ->
    let end_label = Label.next_regular () in
    gather
      [
        emit_expr env etest;
        instr_dup;
        instr_jmpnz end_label;
        instr_popc;
        emit_expr env efalse;
        instr_label end_label;
      ]

and get_erased_tparams env =
  Ast_scope.Scope.get_tparams (Emit_env.get_scope env)
  |> List.filter_map ~f:(function
         | { A.tp_name = (_, name); A.tp_reified; _ } ->
         Option.some_if (tp_reified = A.Erased) name)

and has_non_tparam_generics env (targs : Aast.hint list) =
  let erased_tparams = get_erased_tparams env in
  List.exists targs ~f:(function
      | (_, Aast.Happly ((_, id), _))
        when List.mem ~equal:String.equal erased_tparams id ->
        false
      | _ -> true)

and has_non_tparam_generics_targs env (targs : Tast.targ list) =
  let erased_tparams = get_erased_tparams env in
  List.exists targs ~f:(function
      | (_, (_, Aast.Happly ((_, id), _)))
        when List.mem ~equal:String.equal erased_tparams id ->
        false
      | _ -> true)

and emit_reified_targs env pos targs =
  let len = List.length targs in
  let scope = Emit_env.get_scope env in
  let current_fun_tparams = Ast_scope.Scope.get_fun_tparams scope in
  let current_cls_tparam = Ast_scope.Scope.get_class_tparams scope in
  let is_in_lambda = Ast_scope.Scope.is_in_lambda (Emit_env.get_scope env) in
  let is_soft { A.tp_user_attributes = ua; _ } =
    List.exists ua ~f:(function { A.ua_name = n; _ } ->
        snd n = SN.UserAttributes.uaSoft)
  in
  let is_same tparam =
    List.length tparam = len
    && List.for_all2_exn tparam targs ~f:(fun tp ta ->
           match (tp, ta) with
           | ({ A.tp_name = (_, name1); _ }, (_, A.Happly ((_, name2), []))) ->
             name1 = name2 && not (is_soft tp)
           | (_, _) -> false)
  in
  if (not is_in_lambda) && is_same current_fun_tparams then
    instr_cgetl (Local.Named SU.Reified.reified_generics_local_name)
  else if (not is_in_lambda) && is_same current_cls_tparam.A.c_tparam_list then
    gather
      [
        instr_checkthis;
        instr_baseh;
        instr_querym
          0
          QueryOp.CGet
          (MemberKey.PT
             (Hhbc_id.Prop.from_raw_string SU.Reified.reified_prop_name));
      ]
  (* TODO(T31677864): If the full generic array is static and does not require
   * resolution, emit it as static array *)
  else
    gather
      [
        gather
        @@ List.map targs ~f:(fun h ->
               fst @@ emit_reified_arg env ~isas:false pos h);
        instr_lit_const
          ( if hack_arr_dv_arrs () then
            NewVecArray len
          else
            NewVArray len );
      ]

and emit_new
    env
    pos
    (cid : Tast.class_id)
    (targs : Tast.targ list)
    (args : Tast.expr list)
    (uarg : Tast.expr option) =
  if has_inout_args args then
    Emit_fatal.raise_fatal_parse pos "Unexpected inout arg in new expr";
  let scope = Emit_env.get_scope env in
  (* If `new self` or `new parent `when self or parent respectively has
   * reified generics, do not resolve *)
  let resolve_self =
    match cid with
    | (_, A.CIexpr (_, A.Id (_, n))) when SU.is_self n ->
      (Ast_scope.Scope.get_class_tparams scope).A.c_tparam_list
      |> List.for_all ~f:(fun t -> t.A.tp_reified = A.Erased)
    | (_, A.CIexpr (_, A.Id (_, n))) when SU.is_parent n ->
      let cls = Ast_scope.Scope.get_class scope in
      Option.value_map cls ~default:true ~f:(fun cls ->
          match cls.A.c_extends with
          | (_, Aast.Happly (_, l)) :: _ -> not @@ has_non_tparam_generics env l
          | _ -> true)
    | _ -> true
  in
  let cexpr = class_id_to_class_expr ~resolve_self scope cid in
  let (cexpr, has_generics) =
    match cexpr with
    | Class_id (_, name) ->
      begin
        match emit_reified_type_opt env pos name with
        | Some instrs ->
          if not @@ List.is_empty targs then
            Emit_fatal.raise_fatal_parse
              pos
              "Cannot have higher kinded reified generics";
          (Class_reified instrs, H.MaybeGenerics)
        | None when not (has_non_tparam_generics_targs env targs) ->
          (cexpr, H.NoGenerics)
        | None -> (cexpr, H.HasGenerics)
      end
    | _ -> (cexpr, H.NoGenerics)
  in
  let newobj_instrs =
    match cexpr with
    (* Special case for statically-known class *)
    | Class_id (_, cname) ->
      let id = Hhbc_id.Class.from_ast_name cname in
      Emit_symbol_refs.add_class id;
      begin
        match has_generics with
        | H.NoGenerics -> gather [emit_pos pos; instr_newobjd id]
        | H.HasGenerics ->
          gather
            [
              emit_pos pos;
              emit_reified_targs env pos (List.map ~f:snd targs);
              instr_newobjrd id;
            ]
        | H.MaybeGenerics ->
          failwith "Internal error: This case should have been transformed"
      end
    | Class_special cls_ref -> gather [emit_pos pos; instr_newobjs cls_ref]
    | Class_reified instrs when has_generics = H.MaybeGenerics ->
      gather [instrs; instr_classgetts; instr_newobjr]
    | _ -> gather [emit_load_class_ref env pos cexpr; instr_newobj]
  in
  Scope.with_unnamed_locals @@ fun () ->
  let (instr_args, _) = emit_args_and_inout_setters env args in
  let instr_uargs =
    match uarg with
    | None -> empty
    | Some uarg -> emit_expr env uarg
  in
  ( empty,
    gather
      [
        newobj_instrs;
        instr_dup;
        instr_nulluninit;
        instr_nulluninit;
        instr_args;
        instr_uargs;
        emit_pos pos;
        instr_fcallctor
          (get_fcall_args
             ~lock_while_unwinding:true
             ?context:(Emit_env.get_call_context env)
             args
             uarg
             None);
        instr_popc;
        instr_lockobj;
      ],
    empty )

(* TODO(T36697624) more efficient bytecode for static records *)
and emit_record env pos (_, rname) is_array es =
  let id = Hhbc_id.Class.from_ast_name rname in
  let instr =
    if is_array then
      instr_new_recordarray
    else
      instr_new_record
  in
  Emit_symbol_refs.add_class id;
  emit_struct_array env pos es (instr id)

and emit_clone env expr = gather [emit_expr env expr; instr_clone]

and emit_shape
    env (expr : Tast.expr) (fl : (Ast_defs.shape_field_name * Tast.expr) list) =
  let p = fst expr in
  let fl =
    List.map fl ~f:(fun (fn, e) ->
        ((p, extract_shape_field_name_pstring env p fn), e))
  in
  emit_expr env (p, A.Darray (None, fl))

and emit_call_expr env pos e targs args uarg async_eager_label =
  match (snd e, targs, args, uarg) with
  | (A.Id (_, id), _, [(_, A.String data)], None)
    when id = SN.SpecialFunctions.hhas_adata ->
    let v = Typed_value.HhasAdata data in
    emit_pos_then pos @@ instr (ILitConst (TypedValue v))
  | (A.Id (_, id), _, _, None) when id = SN.PseudoFunctions.isset ->
    emit_call_isset_exprs env pos args
  | (A.Id (_, id), _, ([_; _] | [_; _; _]), None)
    when id = SN.FB.idx && not (jit_enable_rename_function ()) ->
    emit_idx env pos args
  | (A.Id (_, id), _, [arg1], None) when id = SN.EmitterSpecialFunctions.eval ->
    emit_eval env pos arg1
  | (A.Id (_, id), _, [arg1], None)
    when id = SN.EmitterSpecialFunctions.set_frame_metadata ->
    gather
      [
        emit_expr env arg1;
        emit_pos pos;
        instr_popl (Local.Named "$86metadata");
        instr_null;
      ]
  | (A.Id (_, s), _, [], None)
    when s = SN.PseudoFunctions.exit || s = SN.PseudoFunctions.die ->
    emit_pos_then pos @@ emit_exit env None
  | (A.Id (_, s), _, [arg1], None)
    when s = SN.PseudoFunctions.exit || s = SN.PseudoFunctions.die ->
    emit_pos_then pos @@ emit_exit env (Some arg1)
  | (_, _, _, _) ->
    let instrs = emit_call env pos e targs args uarg async_eager_label in
    emit_pos_then pos instrs

and emit_known_class_id (_, cname) =
  let cid = Hhbc_id.Class.from_ast_name cname in
  Emit_symbol_refs.add_class cid;
  gather [instr_string (Hhbc_id.Class.to_raw_string cid); instr_classgetc]

and emit_load_class_ref env pos cexpr =
  emit_pos_then pos
  @@
  match cexpr with
  | Class_special SpecialClsRef.Self -> instr_self
  | Class_special SpecialClsRef.Static -> instr_lateboundcls
  | Class_special SpecialClsRef.Parent -> instr_parent
  | Class_id id -> emit_known_class_id id
  | Class_expr expr -> gather [emit_expr env expr; instr_classgetc]
  | Class_reified instrs -> gather [instrs; instr_classgetc]

and emit_load_class_const env pos (cexpr : Ast_class_expr.class_expr) id =
  let load_const =
    if SU.is_class id then
      instr (IMisc ClassName)
    else
      instr (ILitConst (ClsCns (Hhbc_id.Const.from_ast_name id)))
  in
  gather [emit_load_class_ref env pos cexpr; load_const]

and emit_class_expr
    env (cexpr : Ast_class_expr.class_expr) (prop : Tast.class_get_expr) =
  let load_prop () =
    match prop with
    | A.CGstring (pos, id) ->
      emit_pos_then pos @@ instr_string (SU.Locals.strip_dollar id)
    | A.CGexpr e -> emit_expr env e
  in
  let aux e =
    let cexpr_local = emit_expr env e in
    ( empty,
      gather
        [
          cexpr_local;
          Scope.stash_top_in_unnamed_local load_prop;
          instr_classgetc;
        ] )
  in
  match cexpr with
  | Class_expr
      ((_, (A.BracedExpr _ | A.Call _ | A.Binop _ | A.Class_get _)) as e) ->
    aux e
  | Class_expr ((_, A.Lvar (_, id)) as e) when Local_id.get_name id = "$this" ->
    aux e
  | _ ->
    let pos =
      match prop with
      | A.CGstring (pos, _) -> pos
      | A.CGexpr ((pos, _), _) -> pos
    in
    (load_prop (), emit_load_class_ref env pos cexpr)

and emit_class_get env qop (cid : Tast.class_id) (prop : Tast.class_get_expr) =
  let cexpr =
    class_id_to_class_expr ~resolve_self:false (Emit_env.get_scope env) cid
  in
  gather
    [
      of_pair @@ emit_class_expr env cexpr prop;
      (match qop with
      | QueryOp.CGet -> instr_cgets
      | QueryOp.CGetQuiet -> failwith "emit_class_get: CGetQuiet"
      | QueryOp.Isset -> instr_issets
      | QueryOp.InOut -> failwith "emit_class_get: InOut");
    ]

(* Class constant <cid>::<id>.
 * We follow the logic for the Construct::KindOfClassConstantExpression
 * case in emitter.cpp
 *)
and emit_class_const env pos (cid : Tast.class_id) (_, id) :
    Instruction_sequence.t =
  let cexpr =
    class_id_to_class_expr ~resolve_self:true (Emit_env.get_scope env) cid
  in
  let cexpr =
    match cexpr with
    | Class_id (_, name) ->
      Option.value ~default:cexpr (get_reified_var_cexpr env pos name)
    | _ -> cexpr
  in
  match cexpr with
  | Class_id cid -> emit_class_const_impl cid id
  | _ -> emit_load_class_const env pos cexpr id

and emit_class_const_impl (p, cname) const_id =
  let cid = Hhbc_id.Class.from_ast_name cname in
  let cname = Hhbc_id.Class.to_raw_string cid in
  emit_pos_then p
  @@
  if SU.is_class const_id then
    instr_string cname
  else (
    Emit_symbol_refs.add_class cid;
    instr (ILitConst (ClsCnsD (Hhbc_id.Const.from_ast_name const_id, cid)))
  )

and emit_yield env pos = function
  | A.AFvalue e -> gather [emit_expr env e; emit_pos pos; instr_yield]
  | A.AFkvalue (e1, e2) ->
    gather [emit_expr env e1; emit_expr env e2; emit_pos pos; instr_yieldk]

and emit_string2 env pos (exprs : Tast.expr list) =
  match exprs with
  | [e] -> gather [emit_expr env e; emit_pos pos; instr (IOp CastString)]
  | e1 :: e2 :: es ->
    gather
    @@ [
         emit_two_exprs env (fst (fst e1)) e1 e2;
         emit_pos pos;
         instr (IOp Concat);
         gather
           (List.map es (fun e ->
                gather [emit_expr env e; emit_pos pos; instr (IOp Concat)]));
       ]
  | [] -> failwith "String2 with zero arguments is impossible"

and emit_lambda (env : Emit_env.t) (fundef : Tast.fun_) (ids : Aast.lid list) =
  (* Closure conversion puts the class number used for CreateCl in the "name"
   * of the function definition *)
  let fundef_name = snd fundef.A.f_name in
  let class_num = int_of_string fundef_name in
  let explicit_use = SSet.mem fundef_name (Emit_env.get_explicit_use_set ()) in
  let is_in_lambda = Ast_scope.Scope.is_in_lambda (Emit_env.get_scope env) in
  gather
    [
      gather
      @@ List.map ids (fun (pos, id) ->
             match SU.Reified.is_captured_generic @@ Local_id.get_name id with
             | Some (is_fun, i) ->
               if is_in_lambda then
                 instr_cgetl
                   (Local.Named
                      (SU.Reified.reified_generic_captured_name is_fun i))
               else
                 emit_reified_generic_instrs Pos.none ~is_fun i
             | None ->
               let lid = get_local env (pos, Local_id.get_name id) in
               if explicit_use then
                 instr_cgetl lid
               else
                 instr_cugetl lid);
      instr (IMisc (CreateCl (List.length ids, class_num)));
    ]

and emit_id (env : Emit_env.t) ((p, s) : Aast.sid) =
  match s with
  | _ when s = SN.PseudoConsts.g__FILE__ -> instr (ILitConst File)
  | _ when s = SN.PseudoConsts.g__DIR__ -> instr (ILitConst Dir)
  | _ when s = SN.PseudoConsts.g__CLASS__ ->
    gather [instr_self; instr_classname]
  | _ when s = SN.PseudoConsts.g__METHOD__ -> instr (ILitConst Method)
  | _ when s = SN.PseudoConsts.g__FUNCTION_CREDENTIAL__ ->
    instr (ILitConst FuncCred)
  | _ when s = SN.PseudoConsts.g__LINE__ ->
    (* If the expression goes on multi lines, we return the last line *)
    let (_, line, _, _) = Pos.info_pos_extended p in
    instr_int line
  | _ when s = SN.PseudoConsts.g__NAMESPACE__ ->
    let ns = Emit_env.get_namespace env in
    instr_string (Option.value ~default:"" ns.Namespace_env.ns_name)
  | _ when s = SN.PseudoConsts.g__COMPILER_FRONTEND__ -> instr_string "hackc"
  | _ when s = SN.PseudoConsts.exit || s = SN.PseudoConsts.die ->
    emit_exit env None
  | _ ->
    let cid = Hhbc_id.Const.from_ast_name s in
    Emit_symbol_refs.add_constant cid;
    emit_pos_then p @@ instr (ILitConst (CnsE cid))

and emit_xhp
    (env : Emit_env.t) annot (p, name) attributes (children : Tast.expr list) =
  (* Translate into a constructor call. The arguments are:
   *  1) struct-like array of attributes
   *  2) vec-like array of children
   *  3) filename, for debugging
   *  4) line number, for debugging
   *
   *  Spread operators are injected into the attributes array with placeholder
   *  keys that the runtime will interpret as a spread. These keys are not
   *  parseable as user-specified attributes, so they will never collide.
   *)
  let (pos, _) = annot in
  let create_spread p id = (p, "...$" ^ string_of_int id) in
  let convert_attr (spread_id, attrs) = function
    | A.Xhp_simple (name, v) ->
      let attr = (Ast_defs.SFlit_str name, v) in
      (spread_id, attr :: attrs)
    | A.Xhp_spread e ->
      let ((p, _), _) = e in
      let attr = (Ast_defs.SFlit_str (create_spread p spread_id), e) in
      (spread_id + 1, attr :: attrs)
  in
  let (_, attributes) =
    List.fold_left ~f:convert_attr ~init:(0, []) attributes
  in
  let attribute_map = (annot, A.Shape (List.rev attributes)) in
  let children_vec = (annot, A.Varray (None, children)) in
  let filename = (annot, A.Id (pos, SN.PseudoConsts.g__FILE__)) in
  let line = (annot, A.Id (pos, SN.PseudoConsts.g__LINE__)) in
  let renamed_id = Hhbc_id.Class.from_ast_name name in
  Emit_symbol_refs.add_class renamed_id;
  emit_expr env
  @@ ( annot,
       A.New
         ( (annot, A.CI (p, Hhbc_id.Class.to_raw_string renamed_id)),
           [],
           [attribute_map; children_vec; filename; line],
           None,
           annot ) )

and emit_import env annot (flavor : Aast.import_flavor) (e : Tast.expr) =
  let (pos, _) = annot in
  let inc = parse_include e in
  Emit_symbol_refs.add_include inc;
  let (e, import_op) =
    match flavor with
    | Aast.Include -> (e, IIncludeEvalDefine Incl)
    | Aast.Require -> (e, IIncludeEvalDefine Req)
    | Aast.IncludeOnce -> (e, IIncludeEvalDefine InclOnce)
    | Aast.RequireOnce ->
      let include_roots =
        Hhbc_options.include_roots !Hhbc_options.compiler_options
      in
      (match
         Hhas_symbol_refs.resolve_to_doc_root_relative inc ~include_roots
       with
      | Hhas_symbol_refs.DocRootRelative path ->
        ((annot, A.String path), IIncludeEvalDefine ReqDoc)
      | _ -> (e, IIncludeEvalDefine ReqOnce))
  in
  gather [emit_expr env e; emit_pos pos; instr import_op]

and emit_call_isset_expr env outer_pos (expr : Tast.expr) =
  let ((pos, _), expr_) = expr in
  match expr_ with
  | A.Array_get ((_, A.Lvar (_, x)), Some e)
    when Local_id.get_name x = SN.Superglobals.globals ->
    gather [emit_expr env e; emit_pos outer_pos; instr_issetg]
  | A.Array_get (base_expr, opt_elem_expr) ->
    fst (emit_array_get env pos QueryOp.Isset base_expr opt_elem_expr)
  | A.Class_get (cid, id) -> emit_class_get env QueryOp.Isset cid id
  | A.Obj_get (expr, prop, nullflavor) ->
    fst (emit_obj_get env pos QueryOp.Isset expr prop nullflavor)
  | A.Lvar (_, n)
    when SN.Superglobals.is_superglobal (Local_id.get_name n)
         || Local_id.get_name n = SN.Superglobals.globals ->
    gather
      [
        emit_pos outer_pos;
        instr_string @@ SU.Locals.strip_dollar (Local_id.get_name n);
        emit_pos outer_pos;
        instr_issetg;
      ]
  | A.Lvar ((_, name) as id)
    when is_local_this env name && not (Emit_env.get_needs_local_this env) ->
    gather
      [
        emit_pos outer_pos;
        emit_local ~notice:NoNotice env id;
        emit_pos outer_pos;
        instr_istypec OpNull;
        instr_not;
      ]
  | A.Lvar (pos, id) ->
    emit_pos_then outer_pos
    @@ instr (IIsset (IssetL (get_local env (pos, Local_id.get_name id))))
  | _ -> gather [emit_expr env expr; instr_istypec OpNull; instr_not]

and emit_unset_expr env expr =
  emit_lval_op_nonlist env (fst (fst expr)) LValOp.Unset expr empty 0

and emit_set_range_expr env pos name kind args =
  let raise_fatal msg =
    Emit_fatal.raise_fatal_parse pos (Printf.sprintf "%s %s" name msg)
  in
  let (range_op, size, allow_count) = kind in
  let (base, offset, src, args) =
    match args with
    | b :: o :: s :: rest -> (b, o, s, rest)
    | _ -> raise_fatal "expects at least 3 arguments"
  in
  let count_instrs =
    match (args, allow_count) with
    | ([c], true) -> emit_expr env c
    | ([], _) -> instr_int (-1)
    | (_, false) -> raise_fatal "expects no more than 3 arguments"
    | (_, true) -> raise_fatal "expects no more than 4 arguments"
  in
  let (base_expr, cls_expr, base_setup, base_stack, cls_stack) =
    emit_base ~notice:Notice ~is_object:false env MemberOpMode.Define 3 3 base
  in
  gather
    [
      base_expr;
      cls_expr;
      emit_expr env offset;
      emit_expr env src;
      count_instrs;
      base_setup;
      instr (IFinal (SetRangeM (base_stack + cls_stack, range_op, size)));
    ]

and emit_call_isset_exprs env pos (exprs : Tast.expr list) =
  match exprs with
  | [] ->
    Emit_fatal.raise_fatal_parse pos "Cannot use isset() without any arguments"
  | [expr] -> emit_call_isset_expr env pos expr
  | _ ->
    let n = List.length exprs in
    let its_done = Label.next_regular () in
    gather
      [
        gather
        @@ List.mapi exprs (fun i expr ->
               gather
                 [
                   emit_call_isset_expr env pos expr;
                   ( if i < n - 1 then
                     gather [instr_dup; instr_jmpz its_done; instr_popc]
                   else
                     empty );
                 ]);
        instr_label its_done;
      ]

and emit_exit env (expr_opt : Tast.expr option) =
  gather
    [
      (match expr_opt with
      | None -> instr_int 0
      | Some e -> emit_expr env e);
      instr_exit;
    ]

and emit_idx env pos (es : Tast.expr list) =
  let default =
    if List.length es = 2 then
      instr_null
    else
      empty
  in
  gather [emit_exprs env es; emit_pos pos; default; instr_idx]

and emit_eval env pos e = gather [emit_expr env e; emit_pos pos; instr_eval]

and emit_xhp_obj_get env pos (_, ty) (e : Tast.expr) s nullflavor =
  let annot = (pos, ty) in
  let fn_name =
    (annot, A.Obj_get (e, (annot, A.Id (pos, "getAttribute")), nullflavor))
  in
  let args = [(annot, A.String (SU.Xhp.clean s))] in
  emit_call env pos fn_name [] args None None

and try_inline_gen_call env (e : Tast.expr) =
  if not (can_inline_gen_functions ()) then
    None
  else
    match snd e with
    | A.Call (_, (_, A.Id (_, s)), _, [arg], None)
      when SU.strip_global_ns s = "gena" ->
      Some (inline_gena_call env arg)
    | _ -> None

(* emits iteration over the ~collection where loop body is
   produced by ~f *)
and emit_iter ~collection f =
  Scope.with_unnamed_locals_and_iterators @@ fun () ->
  let iter_id = Iterator.get_iterator () in
  let val_id = Local.get_unnamed_local () in
  let key_id = Local.get_unnamed_local () in
  let loop_end = Label.next_regular () in
  let loop_next = Label.next_regular () in
  let iter_args = { iter_id; key_id = Some key_id; val_id } in
  let iter_init = gather [collection; instr_iterinit iter_args loop_end] in
  let iterate =
    gather
      [
        instr_label loop_next;
        f val_id key_id;
        instr_iternext iter_args loop_next;
      ]
  in
  let iter_done =
    gather [instr_unsetl val_id; instr_unsetl key_id; instr_label loop_end]
  in
  (iter_init, iterate, iter_done)

and inline_gena_call env (arg : Tast.expr) =
  Local.scope @@ fun () ->
  (* convert input to array *)
  let load_array = emit_expr env arg in
  Scope.with_unnamed_local @@ fun arr_local ->
  let async_eager_label = Label.next_regular () in
  (* before *)
  ( gather
      [
        load_array;
        ( if hack_arr_dv_arrs () then
          instr_cast_dict
        else
          instr_cast_darray );
        instr_popl arr_local;
      ],
    (* inner *)
    gather
      [
        instr_nulluninit;
        instr_nulluninit;
        instr_nulluninit;
        instr_cgetl arr_local;
        instr_fcallclsmethodd
          (make_fcall_args ~async_eager_label 1)
          (Hhbc_id.Method.from_raw_string
             ( if hack_arr_dv_arrs () then
               "fromDict"
             else
               "fromDArray" ))
          (Hhbc_id.Class.from_raw_string "HH\\AwaitAllWaitHandle");
        instr_await;
        instr_label async_eager_label;
        instr_popc;
        ( emit_iter ~collection:(instr_cgetl arr_local)
        @@ fun value_local key_local ->
          gather
            [
              (* generate code for
           arr_local[key_local] = WHResult (value_local) *)
              instr_cgetl value_local;
              instr_whresult;
              instr_basel arr_local MemberOpMode.Define;
              instr_setm 0 (MemberKey.EL key_local);
              instr_popc;
            ] );
      ],
    (* after *)
    instr_pushl arr_local )

and emit_await env pos (expr : Tast.expr) =
  match try_inline_gen_call env expr with
  | Some r -> r
  | None ->
    let after_await = Label.next_regular () in
    let instrs =
      match snd expr with
      | A.Call (_, e, targs, args, uarg) ->
        emit_call_expr env pos e targs args uarg (Some after_await)
      | _ -> emit_expr env expr
    in
    gather
      [
        instrs;
        emit_pos pos;
        instr_dup;
        instr_istypec OpNull;
        instr_jmpnz after_await;
        instr_await;
        instr_label after_await;
      ]

and emit_callconv _env kind _e =
  match kind with
  | Ast_defs.Pinout ->
    failwith "emit_callconv: This should have been caught at emit_arg"

and get_reified_var_cexpr env pos name : Ast_class_expr.class_expr option =
  match emit_reified_type_opt env pos name with
  | None -> None
  | Some instrs ->
    Some
      (Class_reified
         (gather
            [
              instrs;
              instr_basec 0 MemberOpMode.Warn;
              instr_querym 1 QueryOp.CGet (MemberKey.ET "classname");
            ]))

and emit_reified_generic_instrs pos ~is_fun index =
  let base =
    if is_fun then
      instr_basel
        (Local.Named SU.Reified.reified_generics_local_name)
        MemberOpMode.Warn
    else
      gather
        [
          instr_checkthis;
          instr_baseh;
          instr_dim_warn_pt
          @@ Hhbc_id.Prop.from_raw_string SU.Reified.reified_prop_name;
        ]
  in
  emit_pos_then pos
  @@ gather
       [base; instr_querym 0 QueryOp.CGet (MemberKey.EI (Int64.of_int index))]

and emit_reified_type_opt (env : Emit_env.t) pos name =
  let is_in_lambda = Ast_scope.Scope.is_in_lambda (Emit_env.get_scope env) in
  let cget_instr is_fun i =
    instr_cgetl
      (Local.Named (SU.Reified.reified_generic_captured_name is_fun i))
  in
  let check is_soft =
    if not is_soft then
      ()
    else
      Emit_fatal.raise_fatal_parse
        pos
        ( name
        ^ " is annotated to be a soft reified generic,"
        ^ " it cannot be used until the __Soft annotation is removed" )
  in
  let rec aux ~is_fun =
    match is_reified_tparam ~is_fun env name with
    | Some (i, is_soft) ->
      check is_soft;
      Some
        ( if is_in_lambda then
          cget_instr is_fun i
        else
          emit_reified_generic_instrs pos ~is_fun i )
    | None ->
      if is_fun then
        aux ~is_fun:false
      else
        None
  in
  aux ~is_fun:true

and emit_reified_type env pos name =
  match emit_reified_type_opt env pos name with
  | Some instrs -> instrs
  | None -> Emit_fatal.raise_fatal_runtime Pos.none "Invalid reified param"

and emit_expr (env : Emit_env.t) (expr : Tast.expr) =
  let (((pos, _) as annot), expr_) = expr in
  match expr_ with
  | A.Float _
  | A.String _
  | A.Int _
  | A.Null
  | A.False
  | A.True ->
    let v =
      Ast_constant_folder.expr_to_typed_value (Emit_env.get_namespace env) expr
    in
    emit_pos_then pos @@ instr (ILitConst (TypedValue v))
  | A.PrefixedString (_, e)
  | A.ParenthesizedExpr e ->
    emit_expr env e
  | A.Lvar ((pos, _) as lid) ->
    gather [emit_pos pos; emit_local ~notice:Notice env lid]
  | A.Class_const (cid, id) -> emit_class_const env pos cid id
  | A.Unop (op, e) -> emit_unop env pos op e
  | A.Binop (op, e1, e2) -> emit_binop env annot op e1 e2
  | A.Pipe (_, e1, e2) -> emit_pipe env e1 e2
  | A.Is (e, h) -> gather [emit_expr env e; emit_is env pos h]
  | A.As (e, h, is_nullable) -> emit_as env pos e h is_nullable
  | A.Cast ((_, hint), e) -> emit_cast env pos hint e
  | A.Eif (etest, etrue, efalse) ->
    emit_conditional_expression env pos etest etrue efalse
  | A.Expr_list es -> gather @@ List.map es ~f:(emit_expr env)
  | A.Array_get ((_, A.Lvar (_, x)), Some e)
    when Local_id.get_name x = SN.Superglobals.globals ->
    gather [emit_expr env e; emit_pos pos; instr (IGet CGetG)]
  | A.Array_get (base_expr, opt_elem_expr) ->
    fst (emit_array_get env pos QueryOp.CGet base_expr opt_elem_expr)
  | A.Obj_get (expr, prop, nullflavor) ->
    fst (emit_obj_get env pos QueryOp.CGet expr prop nullflavor)
  | A.Call (_, e, targs, args, uarg) ->
    emit_call_expr env pos e targs args uarg None
  | A.FunctionPointer (e, targs) -> emit_function_pointer env e targs
  | A.New (cid, targs, args, uarg, _constructor_annot) ->
    emit_new env pos cid targs args uarg
  | A.Record (cid, is_array, es) ->
    let es2 = List.map ~f:(fun (e1, e2) -> A.AFkvalue (e1, e2)) es in
    emit_record env pos cid is_array es2
  | A.Array es -> emit_pos_then pos @@ emit_collection env expr es
  | A.Darray (ta, es) ->
    emit_pos_then pos
    @@
    let es2 = List.map ~f:(fun (e1, e2) -> A.AFkvalue (e1, e2)) es in
    let darray_e = (fst expr, A.Darray (ta, es)) in
    emit_collection env darray_e es2
  | A.Varray (ta, es) ->
    emit_pos_then pos
    @@
    let es2 = List.map ~f:(fun e -> A.AFvalue e) es in
    let varray_e = (fst expr, A.Varray (ta, es)) in
    emit_collection env varray_e es2
  | A.Collection ((pos, name), _, fields) ->
    emit_named_collection_str env expr pos name fields
  | A.ValCollection (name, _, el) ->
    let fields = List.map el ~f:(fun e -> A.AFvalue e) in
    let emit n = emit_named_collection env expr pos n fields in
    (match name with
    | A.Vector -> emit CollectionType.Vector
    | A.ImmVector -> emit CollectionType.ImmVector
    | A.Set -> emit CollectionType.Set
    | A.ImmSet -> emit CollectionType.ImmSet
    | _ -> emit_collection env expr fields)
  | A.Pair (e1, e2) ->
    let fields = [A.AFvalue e1; A.AFvalue e2] in
    emit_named_collection env expr pos CollectionType.Pair fields
  | A.KeyValCollection (name, _, fields) ->
    let fields = List.map fields ~f:(fun (e1, e2) -> A.AFkvalue (e1, e2)) in
    let emit n = emit_named_collection env expr pos n fields in
    (match name with
    | A.Map -> emit CollectionType.Map
    | A.ImmMap -> emit CollectionType.ImmMap
    | _ -> emit_collection env expr fields)
  | A.Clone e -> emit_pos_then pos @@ emit_clone env e
  | A.Shape fl -> emit_pos_then pos @@ emit_shape env expr fl
  | A.Await e -> emit_await env pos e
  | A.Yield e -> emit_yield env pos e
  | A.Yield_break -> failwith "yield break should be in statement position"
  | A.Yield_from _ -> failwith "complex yield_from expression"
  | A.Lfun _ ->
    failwith
      "expected Lfun to be converted to Efun during closure conversion emit_expr"
  | A.Efun (fundef, ids) -> emit_pos_then pos @@ emit_lambda env fundef ids
  | A.Class_get (cid, id) -> emit_class_get env QueryOp.CGet cid id
  | A.String2 es -> emit_string2 env pos es
  | A.BracedExpr e -> emit_expr env e
  | A.Id id -> emit_pos_then pos @@ emit_id env id
  | A.Xml (id, attributes, children) ->
    emit_xhp env (fst expr) id attributes children
  | A.Callconv (_, _) ->
    failwith "emit_expr: This should have been caught at emit_arg"
  | A.Import (flavor, e) -> emit_import env annot flavor e
  | A.Omitted -> empty
  | A.Suspend _ -> failwith "Codegen for 'suspend' operator is not supported"
  | A.List _ ->
    Emit_fatal.raise_fatal_parse
      pos
      "list() can only be used as an lvar. Did you mean to use tuple()?"
  | A.Any -> failwith "Cannot codegen from an Any node"
  | A.This
  | A.Lplaceholder _
  | A.Dollardollar _ ->
    failwith "TODO Codegen after naming pass on AAST"
  | A.Typename _ -> failwith "Typename should not occur in expressions"
  | A.PU_atom _
  | A.PU_identifier _ ->
    failwith "TODO(T35357243): Pocket Universes syntax must be erased by now"
  | A.Fun_id _ -> failwith "TODO Unimplemented expression Fun"
  | A.Method_id (_, _) -> failwith "TODO Unimplemented expression Method"
  | A.Method_caller (_, _) -> failwith "TODO Unimplemented expression Method"
  | A.Smethod_id (_, _) -> failwith "TODO Unimplemented expression Smethod"
  | A.Assert _ -> failwith "TODO Unimplemented expression Assert"

and emit_static_collection env ~transform_to_collection pos tv =
  let arrprov_enabled =
    Hhbc_options.array_provenance !Hhbc_options.compiler_options
  in
  let transform_instr =
    match transform_to_collection with
    | Some collection_type -> instr_colfromarray collection_type
    | _ -> empty
  in
  (* This is a nasty hack to make sure that static arrays in a PSF function are tagged
   * using dynamic provenance information *)
  if
    arrprov_enabled
    && Ast_scope.Scope.has_function_attribute
         (Emit_env.get_scope env)
         "__ProvenanceSkipFrame"
  then
    gather
      [
        emit_pos pos;
        instr_nulluninit;
        instr_nulluninit;
        instr_nulluninit;
        instr (ILitConst (TypedValue tv));
        instr_fcallfuncd
          (make_fcall_args 1)
          (Hhbc_id.Function.from_raw_string "HH\\tag_provenance_here");
        transform_instr;
      ]
  else
    gather [emit_pos pos; instr (ILitConst (TypedValue tv)); transform_instr]

and emit_value_only_collection env pos es constructor =
  let limit = max_array_elem_on_stack () in
  let inline exprs =
    gather
      [
        gather
        @@ List.map exprs ~f:(function
               (* Drop the keys *)
               | A.AFkvalue (_, e)
               | A.AFvalue e
               -> emit_expr env e);
        emit_pos pos;
        instr @@ ILitConst (constructor @@ List.length exprs);
      ]
  in
  let outofline exprs =
    gather
    @@ List.map exprs ~f:(function
           (* Drop the keys *)
           | A.AFkvalue (_, e)
           | A.AFvalue e
           -> gather [emit_expr env e; instr_add_new_elemc])
  in
  match List.groupi ~break:(fun i _ _ -> i = limit) es with
  | [] -> empty
  | [x1] -> inline x1
  | x1 :: x2 :: _ -> gather [inline x1; outofline x2]

and emit_keyvalue_collection ctype env pos es constructor =
  let (transform_instr, add_elem_instr) =
    match ctype with
    | CollectionType.Dict
    | CollectionType.Array ->
      (empty, instr_add_new_elemc)
    | _ -> (instr_colfromarray ctype, gather [instr_dup; instr_add_elemc])
  in
  gather
    [
      emit_pos pos;
      instr (ILitConst constructor);
      gather
        (List.map es ~f:(expr_and_new env pos add_elem_instr instr_add_elemc));
      emit_pos pos;
      transform_instr;
    ]

and emit_struct_array env pos es ctor =
  let es =
    List.map es ~f:(function
        | A.AFkvalue (k, v) ->
          let ns = Emit_env.get_namespace env in
          (* TODO: Consider reusing folded keys from is_struct_init *)
          begin
            match snd @@ Ast_constant_folder.fold_expr ns k with
            | A.String s -> (s, emit_expr env v)
            | _ -> failwith "Key must be a string"
          end
        | _ -> failwith "impossible")
  in
  gather
    [gather @@ List.map es ~f:snd; emit_pos pos; ctor @@ List.map es ~f:fst]

(* isPackedInit() returns true if this expression list looks like an
 * array with no keys and no ref values *)
and is_packed_init ?(hack_arr_compat = true) es =
  let is_only_values =
    List.for_all es ~f:(function
        | A.AFkvalue _ -> false
        | _ -> true)
  in
  let keys_are_zero_indexed_properly_formed =
    List.foldi es ~init:true ~f:(fun i b f ->
        b
        &&
        match f with
        | A.AFkvalue ((_, A.Int k), _) -> int_of_string k = i
        (* arrays with int-like string keys are still considered packed
         and should be emitted via NewArray *)
        | A.AFkvalue ((_, A.String k), _) when not hack_arr_compat ->
          (try int_of_string k = i with Failure _ -> false)
        (* True and False are considered 1 and 0, respectively *)
        | A.AFkvalue ((_, A.True), _) -> i = 1
        | A.AFkvalue ((_, A.False), _) -> i = 0
        | A.AFvalue _ -> true
        | _ -> false)
  in
  let has_bool_keys =
    List.exists es ~f:(function
        | A.AFkvalue ((_, (A.True | A.False)), _) -> true
        | _ -> false)
  in
  (is_only_values || keys_are_zero_indexed_properly_formed)
  && (not (has_bool_keys && hack_arr_compat && hack_arr_compat_notices ()))
  && List.length es > 0

and is_struct_init env es allow_numerics =
  let keys = ULS.empty in
  let (are_all_keys_non_numeric_strings, keys) =
    List.fold_right es ~init:(true, keys) ~f:(fun field (b, keys) ->
        match field with
        | A.AFkvalue (key, _) ->
          let ns = Emit_env.get_namespace env in
          begin
            match snd @@ Ast_constant_folder.fold_expr ns key with
            | A.String s ->
              ( b
                && Option.is_none
                   @@ Typed_value.string_to_int_opt ~allow_inf:false s,
                ULS.add keys s )
            | _ -> (false, keys)
          end
        | _ -> (false, keys))
  in
  let num_keys = List.length es in
  let has_duplicate_keys = ULS.cardinal keys <> num_keys in
  let limit = max_array_elem_on_stack () in
  (allow_numerics || are_all_keys_non_numeric_strings)
  && (not has_duplicate_keys)
  && num_keys <= limit
  && num_keys <> 0

(* transform_to_collection argument keeps track of
 * what collection to transform to *)
and emit_dynamic_collection env (expr : Tast.expr) es =
  let ((pos, _), expr_) = expr in
  let count = List.length es in
  let emit_collection_helper ctype =
    if is_struct_init env es true then
      gather
        [
          emit_struct_array env pos es instr_newstructdict;
          emit_pos pos;
          instr_colfromarray ctype;
        ]
    else
      emit_keyvalue_collection ctype env pos es (NewDictArray count)
  in
  match expr_ with
  | A.ValCollection (A.Vec, _, _)
  | A.Collection ((_, "vec"), _, _) ->
    emit_value_only_collection env pos es (fun n -> NewVecArray n)
  | A.ValCollection (A.Keyset, _, _)
  | A.Collection ((_, "keyset"), _, _) ->
    emit_value_only_collection env pos es (fun n -> NewKeysetArray n)
  | A.KeyValCollection (A.Dict, _, _)
  | A.Collection ((_, "dict"), _, _) ->
    if is_struct_init env es true then
      emit_struct_array env pos es instr_newstructdict
    else
      emit_keyvalue_collection
        CollectionType.Dict
        env
        pos
        es
        (NewDictArray count)
  | A.Collection ((_, name), _, _) when SU.strip_ns name = "Set" ->
    emit_collection_helper CollectionType.Set
  | A.ValCollection (A.Set, _, _) -> emit_collection_helper CollectionType.Set
  | A.Collection ((_, name), _, _) when SU.strip_ns name = "ImmSet" ->
    emit_collection_helper CollectionType.ImmSet
  | A.ValCollection (A.ImmSet, _, _) ->
    emit_collection_helper CollectionType.ImmSet
  | A.Collection ((_, name), _, _) when SU.strip_ns name = "Map" ->
    emit_collection_helper CollectionType.Map
  | A.KeyValCollection (A.Map, _, _) ->
    emit_collection_helper CollectionType.Map
  | A.Collection ((_, name), _, _) when SU.strip_ns name = "ImmMap" ->
    emit_collection_helper CollectionType.ImmMap
  | A.KeyValCollection (A.ImmMap, _, _) ->
    emit_collection_helper CollectionType.ImmMap
  | A.Varray _ ->
    emit_value_only_collection env pos es (fun n ->
        if hack_arr_dv_arrs () then
          NewVecArray n
        else
          NewVArray n)
  | A.Darray _ ->
    if is_struct_init env es false then
      emit_struct_array env pos es (fun arg ->
          emit_pos_then pos
          @@
          if hack_arr_dv_arrs () then
            instr_newstructdict arg
          else
            instr_newstructdarray arg)
    else
      emit_keyvalue_collection
        CollectionType.Array
        env
        pos
        es
        ( if hack_arr_dv_arrs () then
          NewDictArray count
        else
          NewDArray count )
  | _ ->
    (* From here on, we're only dealing with PHP arrays *)
    if is_packed_init es then
      emit_value_only_collection env pos es (fun n -> NewPackedArray n)
    else if is_struct_init env es false then
      emit_struct_array env pos es instr_newstructarray
    else if is_packed_init ~hack_arr_compat:false es then
      emit_keyvalue_collection CollectionType.Array env pos es (NewArray count)
    else
      emit_keyvalue_collection
        CollectionType.Array
        env
        pos
        es
        (NewMixedArray count)

and emit_named_collection_str env (expr : Tast.expr) pos name fields =
  let name = SU.Types.fix_casing @@ SU.strip_ns name in
  let ctype =
    match name with
    | "dict" -> CollectionType.Dict
    | "vec" -> CollectionType.Vec
    | "keyset" -> CollectionType.Keyset
    | "Vector" -> CollectionType.Vector
    | "ImmVector" -> CollectionType.ImmVector
    | "Map" -> CollectionType.Map
    | "ImmMap" -> CollectionType.ImmMap
    | "Set" -> CollectionType.Set
    | "ImmSet" -> CollectionType.ImmSet
    | "Pair" -> CollectionType.Pair
    | _ -> failwith @@ "collection: " ^ name ^ " does not exist"
  in
  emit_named_collection env expr pos ctype fields

and emit_named_collection env (expr : Tast.expr) pos ctype fields =
  let emit_vector_like collection_type =
    if fields = [] then
      emit_pos_then pos @@ instr_newcol collection_type
    else
      gather
        [emit_vec_collection env pos fields; instr_colfromarray collection_type]
  in
  let emit_map_or_set collection_type =
    if fields = [] then
      emit_pos_then pos @@ instr_newcol collection_type
    else
      emit_collection ~transform_to_collection:collection_type env expr fields
  in
  match ctype with
  | CollectionType.Dict
  | CollectionType.Vec
  | CollectionType.Keyset ->
    emit_pos_then pos @@ emit_collection env expr fields
  | CollectionType.Vector
  | CollectionType.ImmVector ->
    emit_vector_like ctype
  | CollectionType.Map
  | CollectionType.ImmMap
  | CollectionType.Set
  | CollectionType.ImmSet ->
    emit_map_or_set ctype
  | CollectionType.Pair ->
    gather
      [
        gather
          (List.map fields (function
              | A.AFvalue e -> emit_expr env e
              | _ -> failwith "impossible Pair argument"));
        instr (ILitConst NewPair);
      ]
  | _ -> failwith "Unexpected named collection type"

and is_php_array = function
  | (_, A.Array _) -> true
  | (_, A.Varray _) -> not (hack_arr_dv_arrs ())
  | (_, A.Darray _) -> not (hack_arr_dv_arrs ())
  | _ -> false

and emit_collection ?transform_to_collection env (expr : Tast.expr) es =
  let pos = Tast_annotate.get_pos expr in
  match
    Ast_constant_folder.expr_to_opt_typed_value
      ~allow_maps:true
      (Emit_env.get_namespace env)
      expr
  with
  | Some tv -> emit_static_collection env ~transform_to_collection pos tv
  | None -> emit_dynamic_collection env expr es

and emit_vec_collection env pos es =
  match
    Ast_constant_folder.vec_to_typed_value (Emit_env.get_namespace env) pos es
  with
  | tv -> emit_static_collection env pos tv ~transform_to_collection:None
  | exception Ast_constant_folder.(NotLiteral | UserDefinedConstant) ->
    emit_value_only_collection env pos es (fun t -> NewVecArray t)

and emit_pipe env (e1 : Tast.expr) (e2 : Tast.expr) =
  let lhs_instrs = emit_expr env e1 in
  Scope.with_unnamed_local @@ fun temp ->
  let env = Emit_env.with_pipe_var temp env in
  let rhs_instrs = emit_expr env e2 in
  (gather [lhs_instrs; instr_popl temp], rhs_instrs, instr_unsetl temp)

(* Emit code that is equivalent to
 *   <code for expr>
 *   JmpZ label
 * Generate specialized code in case expr is statically known, and for
 * !, && and || expressions
 *)
and emit_jmpz env (expr : Tast.expr) label : emit_jmp_result =
  let ((pos, _), expr_) = expr in
  let with_pos i = emit_pos_then pos i in
  let opt = optimize_null_checks () in
  let ns = Emit_env.get_namespace env in
  match Ast_constant_folder.expr_to_opt_typed_value ns expr with
  | Some v ->
    let b = Typed_value.to_bool v in
    if b then
      { instrs = with_pos empty; is_fallthrough = true; is_label_used = false }
    else
      {
        instrs = with_pos @@ instr_jmp label;
        is_fallthrough = false;
        is_label_used = true;
      }
  | None ->
    begin
      match expr_ with
      | A.Unop (Ast_defs.Unot, e) ->
        let (annot, expr_) = e in
        emit_jmpnz env annot expr_ label
      | A.Binop (Ast_defs.Barbar, e1, e2) ->
        let skip_label = Label.next_regular () in
        let (e1_annot, e1_expr_) = e1 in
        let r1 = emit_jmpnz env e1_annot e1_expr_ skip_label in
        if not r1.is_fallthrough then
          let instrs =
            if r1.is_label_used then
              gather [r1.instrs; instr_label skip_label]
            else
              r1.instrs
          in
          {
            instrs = with_pos instrs;
            is_fallthrough = r1.is_label_used;
            is_label_used = false;
          }
        else
          let r2 = emit_jmpz env e2 label in
          let instrs =
            gather
              [
                r1.instrs;
                r2.instrs;
                optional r1.is_label_used [instr_label skip_label];
              ]
          in
          {
            instrs = with_pos instrs;
            is_fallthrough = r2.is_fallthrough || r1.is_label_used;
            is_label_used = r2.is_label_used;
          }
      | A.Binop (Ast_defs.Ampamp, e1, e2) ->
        let r1 = emit_jmpz env e1 label in
        if not r1.is_fallthrough then
          {
            instrs = with_pos r1.instrs;
            is_fallthrough = false;
            is_label_used = r1.is_label_used;
          }
        else
          let r2 = emit_jmpz env e2 label in
          {
            instrs = with_pos @@ gather [r1.instrs; r2.instrs];
            is_fallthrough = r2.is_fallthrough;
            is_label_used = r1.is_label_used || r2.is_label_used;
          }
      | A.Binop (Ast_defs.Eqeqeq, e, (_, A.Null))
      | A.Binop (Ast_defs.Eqeqeq, (_, A.Null), e)
        when opt ->
        {
          instrs = with_pos @@ gather [emit_is_null env e; instr_jmpz label];
          is_fallthrough = true;
          is_label_used = true;
        }
      | A.Binop (Ast_defs.Diff2, e, (_, A.Null))
      | A.Binop (Ast_defs.Diff2, (_, A.Null), e)
        when opt ->
        {
          instrs = with_pos @@ gather [emit_is_null env e; instr_jmpnz label];
          is_fallthrough = true;
          is_label_used = true;
        }
      | _ ->
        {
          instrs = with_pos @@ gather [emit_expr env expr; instr_jmpz label];
          is_fallthrough = true;
          is_label_used = true;
        }
    end

(* Emit code that is equivalent to
 *   <code for expr>
 *   JmpNZ label
 * Generate specialized code in case expr is statically known, and for
 * !, && and || expressions
 *)
and emit_jmpnz env annot (expr_ : Tast.expr_) label : emit_jmp_result =
  let (pos, _) = annot in
  let with_pos i = emit_pos_then pos i in
  let opt = optimize_null_checks () in
  match
    Ast_constant_folder.expr_to_opt_typed_value
      (Emit_env.get_namespace env)
      (annot, expr_)
  with
  | Some v ->
    if Typed_value.to_bool v then
      {
        instrs = with_pos @@ instr_jmp label;
        is_fallthrough = false;
        is_label_used = true;
      }
    else
      { instrs = with_pos empty; is_fallthrough = true; is_label_used = false }
  | None ->
    begin
      match expr_ with
      | A.Unop (Ast_defs.Unot, e) -> emit_jmpz env e label
      | A.Binop (Ast_defs.Barbar, (annot1, e1), (annot2, e2)) ->
        let r1 = emit_jmpnz env annot1 e1 label in
        if not r1.is_fallthrough then
          r1
        else
          let r2 = emit_jmpnz env annot2 e2 label in
          {
            instrs = with_pos @@ gather [r1.instrs; r2.instrs];
            is_fallthrough = r2.is_fallthrough;
            is_label_used = r1.is_label_used || r2.is_label_used;
          }
      | A.Binop (Ast_defs.Ampamp, e1, (annot2, e2)) ->
        let skip_label = Label.next_regular () in
        let r1 = emit_jmpz env e1 skip_label in
        if not r1.is_fallthrough then
          {
            instrs =
              with_pos
              @@ gather
                   [
                     r1.instrs;
                     optional r1.is_label_used [instr_label skip_label];
                   ];
            is_fallthrough = r1.is_label_used;
            is_label_used = false;
          }
        else
          let r2 = emit_jmpnz env annot2 e2 label in
          {
            instrs =
              with_pos
              @@ gather
                   [
                     r1.instrs;
                     r2.instrs;
                     optional r1.is_label_used [instr_label skip_label];
                   ];
            is_fallthrough = r2.is_fallthrough || r1.is_label_used;
            is_label_used = r2.is_label_used;
          }
      | A.Binop (Ast_defs.Eqeqeq, e, (_, A.Null))
      | A.Binop (Ast_defs.Eqeqeq, (_, A.Null), e)
        when opt ->
        {
          instrs = with_pos @@ gather [emit_is_null env e; instr_jmpnz label];
          is_fallthrough = true;
          is_label_used = true;
        }
      | A.Binop (Ast_defs.Diff2, e, (_, A.Null))
      | A.Binop (Ast_defs.Diff2, (_, A.Null), e)
        when opt ->
        {
          instrs = with_pos @@ gather [emit_is_null env e; instr_jmpz label];
          is_fallthrough = true;
          is_label_used = true;
        }
      | _ ->
        {
          instrs =
            with_pos @@ gather [emit_expr env (annot, expr_); instr_jmpnz label];
          is_fallthrough = true;
          is_label_used = true;
        }
    end

and emit_short_circuit_op env annot (expr : Tast.expr_) =
  let (pos, _) = annot in
  let its_true = Label.next_regular () in
  let its_done = Label.next_regular () in
  let r1 = emit_jmpnz env annot expr its_true in
  let if_true =
    if r1.is_label_used then
      gather [instr_label its_true; emit_pos pos; instr_true]
    else
      empty
  in
  if r1.is_fallthrough then
    gather
      [
        r1.instrs;
        emit_pos pos;
        instr_false;
        instr_jmp its_done;
        if_true;
        instr_label its_done;
      ]
  else
    gather [r1.instrs; if_true]

and emit_null_coalesce_assignment env pos (e1 : Tast.expr) (e2 : Tast.expr) =
  let end_label = Label.next_regular () in
  let do_set_label = Label.next_regular () in
  let l_nonnull = Local.get_unnamed_local () in
  let (quiet_instr, querym_n_unpopped) =
    emit_quiet_expr ~null_coalesce_assignment:true env pos e1
  in
  let emit_popc_n n_unpopped =
    match n_unpopped with
    | Some n -> gather (List.init n (fun _ -> instr_popc))
    | None -> empty
  in
  gather
    [
      quiet_instr;
      instr_dup;
      instr_istypec OpNull;
      instr_jmpnz do_set_label;
      instr_popl l_nonnull;
      emit_popc_n querym_n_unpopped;
      instr_pushl l_nonnull;
      instr_jmp end_label;
      instr_label do_set_label;
      instr_popc;
      emit_lval_op
        ~null_coalesce_assignment:true
        env
        pos
        LValOp.Set
        e1
        (Some e2);
      instr_label end_label;
    ]

and emit_quiet_expr
    ?(null_coalesce_assignment = false)
    (env : Emit_env.t)
    pos
    (expr : Tast.expr) : Instruction_sequence.t * Hhbc_ast.num_params option =
  let (_, expr_) = expr in
  match expr_ with
  | A.Lvar (name_pos, name)
    when Local_id.get_name name = SN.Superglobals.globals ->
    ( gather
        [
          emit_pos name_pos;
          instr_string (SU.Locals.strip_dollar (Local_id.get_name name));
          emit_pos pos;
          instr (IGet CGetG);
        ],
      None )
  | A.Lvar (_, name) when not (is_local_this env name) ->
    (instr_cgetquietl (get_local env (pos, Local_id.get_name name)), None)
  | A.Array_get ((_, A.Lvar (_, x)), Some e)
    when Local_id.get_name x = SN.Superglobals.globals ->
    (gather [emit_expr env e; emit_pos pos; instr (IGet CGetG)], None)
  | A.Array_get (base_expr, opt_elem_expr) ->
    emit_array_get
      ~null_coalesce_assignment
      env
      pos
      QueryOp.CGetQuiet
      base_expr
      opt_elem_expr
  | A.Obj_get (expr, prop, nullflavor) ->
    emit_obj_get
      ~null_coalesce_assignment
      env
      pos
      QueryOp.CGetQuiet
      expr
      prop
      nullflavor
  | _ -> (emit_expr env expr, None)

(* returns instruction that will represent setter for $base[local] where
   is_base is true when result cell is base for another subscript operator and
   false when it is final left hand side of the assignment *)
and emit_store_for_simple_base
    ~is_base env pos elem_stack_size (base_expr : Tast.expr) local =
  let (base_expr_instrs_begin, base_expr_instrs_end, base_setup_instrs, _, _) =
    emit_base
      ~is_object:false
      ~notice:Notice
      env
      MemberOpMode.Define
      elem_stack_size
      0
      base_expr
  in
  let expr =
    let mk = MemberKey.EL local in
    if is_base then
      instr_dim MemberOpMode.Define mk
    else
      instr_setm 0 mk
  in
  gather
    [
      base_expr_instrs_begin;
      base_expr_instrs_end;
      emit_pos pos;
      base_setup_instrs;
      expr;
    ]

(* get LocalTempKind option for a given expression
   - None - expression can be emitted as is
   - Some Value_kind_local - expression represents local that will be
     overwritten later
   - Some Value_kind_expression - spilled non-trivial expression *)
and get_local_temp_kind ~is_base inout_param_info env (e_opt : Tast.expr option)
    =
  match (e_opt, inout_param_info) with
  (* not inout case - no need to save *)
  | (_, None) -> None
  (* local that will later be overwritten *)
  | (Some (_, A.Lvar (_, id)), Some (i, aliases))
    when InoutLocals.should_save_local_value (Local_id.get_name id) i aliases ->
    Some Value_kind_local
  (* non-trivial expression *)
  | (Some e, _) ->
    if is_trivial ~is_base env e then
      None
    else
      Some Value_kind_expression
  | (None, _) -> None

and is_trivial ~is_base env (_, e) =
  match e with
  | A.Int _
  | A.String _ ->
    true
  | A.Lvar (_, s) ->
    (not (is_local_this env s)) || Emit_env.get_needs_local_this env
  | A.Array_get _ when not is_base -> false
  | A.Array_get (b, None) -> is_trivial ~is_base env b
  | A.Array_get (b, Some e) ->
    is_trivial ~is_base env b && is_trivial ~is_base env e
  | _ -> false

(* Emit code for e1[e2] or isset(e1[e2]).
 *)
and emit_array_get
    ?(null_coalesce_assignment = false)
    ?(no_final = false)
    ?mode
    env
    outer_pos
    qop
    (base_expr : Tast.expr)
    (opt_elem_expr : Tast.expr option) =
  let result =
    emit_array_get_worker
      ~null_coalesce_assignment
      ~no_final
      ?mode
      ~inout_param_info:None
      env
      outer_pos
      qop
      base_expr
      opt_elem_expr
  in
  match result with
  | (Array_get_regular i, querym_n_unpopped) -> (i, querym_n_unpopped)
  | (Array_get_inout _, _) -> failwith "unexpected inout"

and emit_array_get_worker
    ?(null_coalesce_assignment = false)
    ?(no_final = false)
    ?mode
    ~inout_param_info
    env
    outer_pos
    qop
    (base_expr : Tast.expr)
    (opt_elem_expr : Tast.expr option) =
  (* Disallow use of array(..)[] *)
  match (base_expr, opt_elem_expr) with
  | (((pos, _), A.Array _), None) ->
    Emit_fatal.raise_fatal_parse
      pos
      "Can't use array() as base in write context"
  | (((pos, _), _), None) when not (Emit_env.does_env_allow_array_append env) ->
    Emit_fatal.raise_fatal_runtime pos "Can't use [] for reading"
  | _ ->
    let local_temp_kind =
      get_local_temp_kind ~is_base:false inout_param_info env opt_elem_expr
    in
    let mode =
      if null_coalesce_assignment then
        MemberOpMode.Warn
      else
        Option.value mode ~default:(get_queryMOpMode qop)
    in
    let querym_n_unpopped = ref None in
    let (elem_expr_instrs, elem_stack_size) =
      emit_elem_instrs
        ~local_temp_kind
        ~null_coalesce_assignment
        env
        opt_elem_expr
    in
    let base_result =
      emit_base_worker
        ~is_object:false
        ~inout_param_info
        ~notice:
          (match qop with
          | QueryOp.Isset -> NoNotice
          | _ -> Notice)
        ~null_coalesce_assignment
        env
        mode
        elem_stack_size
        0
        base_expr
    in
    let cls_stack_size =
      match base_result with
      | Array_get_base_regular base -> base.cls_stack_size
      | Array_get_base_inout base -> base.load.cls_stack_size
    in
    let mk =
      get_elem_member_key
        ~null_coalesce_assignment
        env
        cls_stack_size
        opt_elem_expr
    in
    let make_final total_stack_size =
      if no_final then
        empty
      else
        instr
          (IFinal
             ( if null_coalesce_assignment then (
               querym_n_unpopped := Some total_stack_size;
               QueryM (0, qop, mk)
             ) else
               QueryM (total_stack_size, qop, mk) ))
    in
    let instr =
      match (base_result, local_temp_kind) with
      | (Array_get_base_regular base, None) ->
        (* both base and expression don't need to store anything *)
        Array_get_regular
          (gather
             [
               base.base_instrs;
               elem_expr_instrs;
               base.cls_instrs;
               emit_pos outer_pos;
               base.setup_instrs;
               make_final
                 (base.base_stack_size + base.cls_stack_size + elem_stack_size);
             ])
      | (Array_get_base_regular base, Some local_kind) ->
        (* base does not need temp locals but index expression does *)
        let local = Local.get_unnamed_local () in
        let load =
          [
            (* load base and indexer, value of indexer will be saved in local *)
            ( gather [base.base_instrs; elem_expr_instrs],
              Some (local, local_kind) );
            (* finish loading the value *)
            ( gather
                [
                  base.base_instrs;
                  emit_pos outer_pos;
                  base.setup_instrs;
                  make_final
                    ( base.base_stack_size
                    + base.cls_stack_size
                    + elem_stack_size );
                ],
              None );
          ]
        in
        let store =
          gather
            [
              emit_store_for_simple_base
                ~is_base:false
                env
                outer_pos
                elem_stack_size
                base_expr
                local;
              instr_popc;
            ]
        in
        Array_get_inout { load; store }
      | (Array_get_base_inout base, None) ->
        (* base needs temp locals, indexer - does not,
       simply concat two instruction sequences *)
        let load =
          base.load.base_instrs
          @ [
              ( gather
                  [
                    elem_expr_instrs;
                    base.load.cls_instrs;
                    emit_pos outer_pos;
                    base.load.setup_instrs;
                    make_final
                      ( base.load.base_stack_size
                      + base.load.cls_stack_size
                      + elem_stack_size );
                  ],
                None );
            ]
        in
        let store = gather [base.store; instr_setm 0 mk; instr_popc] in
        Array_get_inout { load; store }
      | (Array_get_base_inout base, Some local_kind) ->
        (* both base and index need temp locals,
       create local for index value *)
        let local = Local.get_unnamed_local () in
        let load =
          (* load base *)
          base.load.base_instrs
          @ [
              (* load index, value will be saved in local *)
              (elem_expr_instrs, Some (local, local_kind));
              ( gather
                  [
                    base.load.cls_instrs;
                    emit_pos outer_pos;
                    base.load.setup_instrs;
                    make_final
                      ( base.load.base_stack_size
                      + base.load.cls_stack_size
                      + elem_stack_size );
                  ],
                None );
            ]
        in
        let store =
          gather [base.store; instr_setm 0 (MemberKey.EL local); instr_popc]
        in
        Array_get_inout { load; store }
    in
    (instr, !querym_n_unpopped)

(* Emit code for e1->e2 or e1?->e2 or isset(e1->e2).
 *)
and emit_obj_get
    ?(null_coalesce_assignment = false)
    env
    pos
    qop
    (expr : Tast.expr)
    (prop : Tast.expr)
    null_flavor =
  let (annot, expr_) = expr in
  match expr_ with
  | A.Lvar (pos, id)
    when Local_id.get_name id = SN.SpecialIdents.this
         && null_flavor = Ast_defs.OG_nullsafe ->
    Emit_fatal.raise_fatal_parse pos "?-> is not allowed with $this"
  | _ ->
    begin
      match snd prop with
      | A.Id (_, s) when SU.Xhp.is_xhp s ->
        (emit_xhp_obj_get env pos annot expr s null_flavor, None)
      | _ ->
        let mode =
          if null_coalesce_assignment then
            MemberOpMode.Warn
          else
            get_queryMOpMode qop
        in
        let (_, _, prop_stack_size) =
          emit_prop_expr ~null_coalesce_assignment env null_flavor 0 prop
        in
        let ( base_expr_instrs_begin,
              base_expr_instrs_end,
              base_setup_instrs,
              base_stack_size,
              cls_stack_size ) =
          emit_base
            ~is_object:true
            ~notice:Notice
            ~null_coalesce_assignment
            env
            mode
            prop_stack_size
            0
            expr
        in
        let (mk, prop_expr_instrs, _) =
          emit_prop_expr
            ~null_coalesce_assignment
            env
            null_flavor
            cls_stack_size
            prop
        in
        let total_stack_size =
          prop_stack_size + base_stack_size + cls_stack_size
        in
        let num_params =
          if null_coalesce_assignment then
            0
          else
            total_stack_size
        in
        let final_instr = instr (IFinal (QueryM (num_params, qop, mk))) in
        let querym_n_unpopped =
          if null_coalesce_assignment then
            Some total_stack_size
          else
            None
        in
        let instr =
          gather
            [
              base_expr_instrs_begin;
              prop_expr_instrs;
              base_expr_instrs_end;
              emit_pos pos;
              base_setup_instrs;
              final_instr;
            ]
        in
        (instr, querym_n_unpopped)
    end

and is_special_class_constant_accessed_with_class_id cName (id : string) =
  let is_self_parent_or_static =
    match cName with
    | A.CIexpr (_, A.Id (_, cName))
      when SU.is_self cName || SU.is_parent cName || SU.is_static cName ->
      true
    | _ -> false
  in
  SU.is_class id && not is_self_parent_or_static

and emit_elem_instrs
    env
    ~local_temp_kind
    ?(null_coalesce_assignment = false)
    (opt_elem_expr : Tast.expr option) =
  match opt_elem_expr with
  (* These all have special inline versions of member keys *)
  | Some (_, (A.Int _ | A.String _)) -> (empty, 0)
  | Some (_, A.Lvar (pos, id)) when not (is_local_this env id) ->
    if Option.is_some local_temp_kind then
      (instr_cgetquietl (get_local env (pos, Local_id.get_name id)), 0)
    else if null_coalesce_assignment then
      (instr_cgetl (get_local env (pos, Local_id.get_name id)), 1)
    else
      (empty, 0)
  (* Handle Foo::class but not self::class. *)
  | Some (_, A.Class_const ((_, cid), (_, id)))
    when is_special_class_constant_accessed_with_class_id cid id ->
    (empty, 0)
  | Some expr -> (emit_expr env expr, 1)
  | None -> (empty, 0)

(* Get the member key for an array element expression: the `elem` in
 * expressions of the form `base[elem]`.
 * If the array element is missing, use the special key `W`.
 *)
and get_elem_member_key
    ?(null_coalesce_assignment = false)
    env
    stack_index
    (opt_expr : Tast.expr option) =
  match opt_expr with
  (* Special case for local *)
  | Some (_, A.Lvar (p, id)) when not (is_local_this env id) ->
    if null_coalesce_assignment then
      MemberKey.EC stack_index
    else
      MemberKey.EL (get_local env (p, Local_id.get_name id))
  (* Special case for literal integer *)
  | Some ((_, A.Int str) as int_expr) ->
    Ast_constant_folder.(
      let namespace = Emit_env.get_namespace env in
      begin
        match expr_to_typed_value namespace int_expr with
        | TV.Int i -> MemberKey.EI i
        | _ -> failwith (str ^ " is not a valid integer index")
      end)
  (* Special case for literal string *)
  | Some (_, A.String str) -> MemberKey.ET str
  (* Special case for class name *)
  | Some (_, A.Class_const ((_, cid), (_, id)))
    when is_special_class_constant_accessed_with_class_id cid id ->
    let cName =
      match (cid, Ast_scope.Scope.get_class (Emit_env.get_scope env)) with
      | (A.CIself, Some cd) -> SU.strip_global_ns @@ snd cd.A.c_name
      | (A.CIexpr (_, A.Id (_, id)), _)
      | (A.CI (_, id), _) ->
        SU.strip_global_ns id
      | _ ->
        failwith
          "Unreachable due to is_special_class_constant_accessed_with_class_id"
    in
    let fq_id = Hhbc_id.Class.from_ast_name cName in
    MemberKey.ET (Hhbc_id.Class.to_raw_string fq_id)
  (* General case *)
  | Some _ -> MemberKey.EC stack_index
  (* ELement missing (so it's array append) *)
  | None -> MemberKey.W

(* Get the member key for a property, and return any instructions and
 * the size of the stack in the case that the property cannot be
 * placed inline in the instruction. *)
and emit_prop_expr
    ?(null_coalesce_assignment = false)
    env
    null_flavor
    stack_index
    (prop_expr : Tast.expr) =
  let mk =
    match snd prop_expr with
    | A.Id (pos, name) when String_utils.string_starts_with name "$" ->
      MemberKey.PL (get_local env (pos, name))
    (* Special case for known property name *)
    | A.Id (_, id)
    | A.String id ->
      let pid = Hhbc_id.Prop.from_ast_name id in
      begin
        match null_flavor with
        | Ast_defs.OG_nullthrows -> MemberKey.PT pid
        | Ast_defs.OG_nullsafe -> MemberKey.QT pid
      end
    | A.Lvar (pos, name) when not (is_local_this env name) ->
      MemberKey.PL (get_local env (pos, Local_id.get_name name))
    (* General case *)
    | _ -> MemberKey.PC stack_index
  in
  (* For nullsafe access, insist that property is known *)
  begin
    match mk with
    | MemberKey.PL _
    | MemberKey.PC _ ->
      let ((pos, _), _) = prop_expr in
      if null_flavor = Ast_defs.OG_nullsafe then
        Emit_fatal.raise_fatal_parse
          pos
          "?-> can only be used with scalar property names"
    | _ -> ()
  end;
  match mk with
  | MemberKey.PC _ -> (mk, emit_expr env prop_expr, 1)
  | MemberKey.PL local ->
    if null_coalesce_assignment then
      (MemberKey.PC stack_index, instr_cgetl local, 1)
    else
      (mk, empty, 0)
  | _ -> (mk, empty, 0)

(* Emit code for a base expression `expr` that forms part of
 * an element access `expr[elem]` or field access `expr->fld`.
 * The instructions are divided into three sections:
 *   1. base and element/property expression instructions:
 *      push non-trivial base and key values on the stack
 *   2. base selector instructions: a sequence of Base/Dim instructions that
 *      actually constructs the base address from "member keys" that are inlined
 *      in the instructions, or pulled from the key values that
 *      were pushed on the stack in section 1.
 *   3. (constructed by the caller) a final accessor e.g. QueryM or setter
 *      e.g. SetOpM instruction that has the final key inlined in the
 *      instruction, or pulled from the key values that were pushed on the
 *      stack in section 1.
 * The function returns a triple (base_instrs, base_setup_instrs, stack_size)
 * where base_instrs is section 1 above, base_setup_instrs is section 2, and
 * stack_size is the number of values pushed onto the stack by section 1.
 *
 * For example, the r-value expression $arr[3][$ix+2]
 * will compile to
 *   # Section 1, pushing the value of $ix+2 on the stack
 *   Int 2
 *   CGetL2 $ix
 *   AddO
 *   # Section 2, constructing the base address of $arr[3]
 *   BaseL $arr Warn
 *   Dim Warn EI:3
 *   # Section 3, indexing the array using the value at stack position 0 (EC:0)
 *   QueryM 1 CGet EC:0
 *)
and emit_base
    ~is_object
    ~notice
    ?(null_coalesce_assignment = false)
    env
    mode
    base_offset
    rhs_stack_size
    (e : Tast.expr) =
  let result =
    emit_base_worker
      ~is_object
      ~notice
      ~inout_param_info:None
      ~null_coalesce_assignment
      env
      mode
      base_offset
      rhs_stack_size
      e
  in
  match result with
  | Array_get_base_regular i ->
    ( i.base_instrs,
      i.cls_instrs,
      i.setup_instrs,
      i.base_stack_size,
      i.cls_stack_size )
  | Array_get_base_inout _ -> failwith "unexpected inout"

and emit_base_worker
    ~is_object
    ~notice
    ~inout_param_info
    ?(null_coalesce_assignment = false)
    env
    mode
    base_offset
    rhs_stack_size
    (expr : Tast.expr) =
  let (((pos, _) as annot), expr_) = expr in
  let base_mode =
    if mode = MemberOpMode.InOut then
      MemberOpMode.Warn
    else
      mode
  in
  let local_temp_kind =
    get_local_temp_kind ~is_base:true inout_param_info env (Some expr)
  in
  (* generic handler that will try to save local into temp if this is necessary *)
  let emit_default
      base_instrs cls_instrs setup_instrs base_stack_size cls_stack_size =
    match local_temp_kind with
    | Some local_temp ->
      let local = Local.get_unnamed_local () in
      Array_get_base_inout
        {
          load =
            {
              (* run begin part, result will be stored into temp  *)
              base_instrs = [(base_instrs, Some (local, local_temp))];
              cls_instrs;
              setup_instrs;
              base_stack_size;
              cls_stack_size;
            };
          store = instr_basel local MemberOpMode.Define;
        }
    | _ ->
      Array_get_base_regular
        {
          base_instrs;
          cls_instrs;
          setup_instrs;
          base_stack_size;
          cls_stack_size;
        }
  in
  match expr_ with
  | A.Lvar (_, x) when Local_id.get_name x = SN.Superglobals.globals ->
    Emit_fatal.raise_fatal_runtime pos "Cannot use [] with $GLOBALS"
  | A.Lvar (name_pos, x)
    when SN.Superglobals.is_superglobal (Local_id.get_name x) ->
    emit_default
      ( emit_pos_then name_pos
      @@ instr_string (SU.Locals.strip_dollar (Local_id.get_name x)) )
      empty
      (instr (IBase (BaseGC (base_offset, base_mode))))
      1
      0
  | A.Lvar (thispos, x)
    when is_object && Local_id.get_name x = SN.SpecialIdents.this ->
    emit_default
      (emit_pos_then thispos @@ instr (IMisc CheckThis))
      empty
      (instr (IBase BaseH))
      0
      0
  | A.Lvar (pos, str)
    when (not (is_local_this env str)) || Emit_env.get_needs_local_this env ->
    let v = get_local env (pos, Local_id.get_name str) in
    if Option.is_some local_temp_kind then
      emit_default (instr_cgetquietl v) empty (instr_basel v base_mode) 0 0
    else
      emit_default empty empty (instr (IBase (BaseL (v, base_mode)))) 0 0
  | A.Lvar lid ->
    emit_default
      (emit_local ~notice env lid)
      empty
      (instr (IBase (BaseC (base_offset, base_mode))))
      1
      0
  | A.Array_get ((_, A.Lvar (_, x)), Some (_, A.Lvar (y_pos, y_id)))
    when Local_id.get_name x = SN.Superglobals.globals ->
    let v = get_local env (y_pos, Local_id.get_name y_id) in
    emit_default empty empty (instr (IBase (BaseGL (v, base_mode)))) 0 0
  | A.Array_get ((_, A.Lvar (_, x)), Some e)
    when Local_id.get_name x = SN.Superglobals.globals ->
    let elem_expr_instrs = emit_expr env e in
    emit_default
      elem_expr_instrs
      empty
      (instr (IBase (BaseGC (base_offset, base_mode))))
      1
      0
  (* $a[] can not be used as the base of an array get unless as an lval *)
  | A.Array_get (_, None) when not (Emit_env.does_env_allow_array_append env) ->
    Emit_fatal.raise_fatal_runtime pos "Can't use [] for reading"
  (* base is in turn array_get - do a specific handling for inout params
      if necessary *)
  | A.Array_get (base_expr, opt_elem_expr) ->
    let local_temp_kind =
      get_local_temp_kind ~is_base:false inout_param_info env opt_elem_expr
    in
    let (elem_expr_instrs, elem_stack_size) =
      emit_elem_instrs
        ~local_temp_kind
        ~null_coalesce_assignment
        env
        opt_elem_expr
    in
    let base_result =
      emit_base_worker
        ~notice
        ~is_object:false
        ~inout_param_info
        ~null_coalesce_assignment
        env
        mode
        (base_offset + elem_stack_size)
        rhs_stack_size
        base_expr
    in
    let cls_stack_size =
      match base_result with
      | Array_get_base_regular base -> base.cls_stack_size
      | Array_get_base_inout base -> base.load.cls_stack_size
    in
    let mk =
      get_elem_member_key
        ~null_coalesce_assignment
        env
        (base_offset + cls_stack_size)
        opt_elem_expr
    in
    let make_setup_instrs base_setup_instrs =
      gather [base_setup_instrs; instr (IBase (Dim (mode, mk)))]
    in
    begin
      match (base_result, local_temp_kind) with
      (* both base and index don't use temps - fallback to default handler  *)
      | (Array_get_base_regular base, None) ->
        emit_default
          (gather [base.base_instrs; elem_expr_instrs])
          base.cls_instrs
          (make_setup_instrs base.setup_instrs)
          (base.base_stack_size + elem_stack_size)
          base.cls_stack_size
      | (Array_get_base_regular base, Some local_temp) ->
        (* base does not need temps but index does *)
        let local = Local.get_unnamed_local () in
        let base_instrs = gather [base.base_instrs; elem_expr_instrs] in
        Array_get_base_inout
          {
            load =
              {
                (* store result of instr_begin to temp *)
                base_instrs = [(base_instrs, Some (local, local_temp))];
                cls_instrs = base.cls_instrs;
                setup_instrs = make_setup_instrs base.setup_instrs;
                base_stack_size = base.base_stack_size + elem_stack_size;
                cls_stack_size = base.cls_stack_size;
              };
            store =
              emit_store_for_simple_base
                ~is_base:true
                env
                pos
                elem_stack_size
                base_expr
                local;
          }
      | (Array_get_base_inout base, None) ->
        (* base needs temps, index - does not *)
        Array_get_base_inout
          {
            load =
              {
                (* concat index evaluation to base *)
                base_instrs = base.load.base_instrs @ [(elem_expr_instrs, None)];
                cls_instrs = base.load.cls_instrs;
                setup_instrs = make_setup_instrs base.load.setup_instrs;
                base_stack_size = base.load.base_stack_size + elem_stack_size;
                cls_stack_size = base.load.cls_stack_size;
              };
            store = gather [base.store; instr_dim MemberOpMode.Define mk];
          }
      | (Array_get_base_inout base, Some local_kind) ->
        (* both base and index needs locals *)
        let local = Local.get_unnamed_local () in
        Array_get_base_inout
          {
            load =
              {
                base_instrs =
                  base.load.base_instrs
                  @ [
                      (* evaluate index, result will be stored in local *)
                      (elem_expr_instrs, Some (local, local_kind));
                    ];
                cls_instrs = base.load.cls_instrs;
                setup_instrs = make_setup_instrs base.load.setup_instrs;
                base_stack_size = base.load.base_stack_size + elem_stack_size;
                cls_stack_size = base.load.cls_stack_size;
              };
            store =
              gather
                [base.store; instr_dim MemberOpMode.Define (MemberKey.EL local)];
          }
    end
  | A.Obj_get (base_expr, prop_expr, null_flavor) ->
    begin
      match snd prop_expr with
      | A.Id (_, s) when SU.Xhp.is_xhp s ->
        emit_default
          (emit_xhp_obj_get env pos annot base_expr s null_flavor)
          empty
          (gather [instr_basec base_offset base_mode])
          1
          0
      | _ ->
        let (_, _, prop_stack_size) =
          emit_prop_expr ~null_coalesce_assignment env null_flavor 0 prop_expr
        in
        let ( base_expr_instrs_begin,
              base_expr_instrs_end,
              base_setup_instrs,
              base_stack_size,
              cls_stack_size ) =
          emit_base
            ~notice:Notice
            ~is_object:true
            ~null_coalesce_assignment
            env
            mode
            (base_offset + prop_stack_size)
            rhs_stack_size
            base_expr
        in
        let (mk, prop_expr_instrs, _) =
          emit_prop_expr
            ~null_coalesce_assignment
            env
            null_flavor
            (base_offset + cls_stack_size)
            prop_expr
        in
        let total_stack_size = prop_stack_size + base_stack_size in
        let final_instr = instr (IBase (Dim (mode, mk))) in
        emit_default
          (gather [base_expr_instrs_begin; prop_expr_instrs])
          base_expr_instrs_end
          (gather [base_setup_instrs; final_instr])
          total_stack_size
          cls_stack_size
    end
  | A.Class_get (cid, prop) ->
    let cexpr =
      class_id_to_class_expr ~resolve_self:false (Emit_env.get_scope env) cid
    in
    let (cexpr_begin, cexpr_end) = emit_class_expr env cexpr prop in
    emit_default
      cexpr_begin
      cexpr_end
      (instr_basesc (base_offset + 1) rhs_stack_size base_mode)
      1
      1
  | _ ->
    let base_expr_instrs = emit_expr env expr in
    emit_default
      base_expr_instrs
      empty
      (emit_pos_then pos @@ instr (IBase (BaseC (base_offset, base_mode))))
      1
      0

and emit_ignored_expr env ?(pop_pos : Pos.t = Pos.none) (e : Tast.expr) =
  match snd e with
  | A.Expr_list es -> gather @@ List.map ~f:(emit_ignored_expr env ~pop_pos) es
  | _ -> gather [emit_expr env e; emit_pos_then pop_pos instr_popc]

(*
 * Replaces erased generics with underscores or
 * raises a parse error if used with is/as expressions
 *)
and fixup_type_arg (env : Emit_env.t) ~isas (hint : Aast.hint) =
  let erased_tparams = get_erased_tparams env in
  let rec aux ((p, hint) as _x : Aast.hint) =
    match hint with
    | Aast.Hoption h -> (p, Aast.Hoption (aux h))
    | Aast.Hlike h -> (p, Aast.Hlike (aux h))
    | Aast.Hfun
        Aast.
          {
            hf_reactive_kind;
            hf_is_coroutine;
            hf_param_tys;
            hf_param_kinds;
            hf_param_mutability;
            hf_variadic_ty;
            hf_return_ty;
            hf_is_mutable_return;
          } ->
      ( p,
        Aast.Hfun
          Aast.
            {
              hf_reactive_kind;
              hf_is_coroutine;
              hf_param_tys = List.map ~f:aux hf_param_tys;
              hf_param_kinds;
              hf_param_mutability;
              (* TODO: shouldn't we also replace the hint in here? *)
              hf_variadic_ty;
              hf_return_ty = aux hf_return_ty;
              hf_is_mutable_return;
            } )
    | Aast.Htuple hl -> (p, Aast.Htuple (List.map ~f:aux hl))
    | Aast.Happly ((_, id), _)
      when List.mem ~equal:String.equal erased_tparams id ->
      if isas then
        Emit_fatal.raise_fatal_parse
          p
          "Erased generics are not allowed in is/as expressions"
      else
        (p, Aast.Happly ((p, "_"), []))
    | Aast.Happly (id, hl) -> (p, Aast.Happly (id, List.map ~f:aux hl))
    | Aast.Hshape
        { Aast.nsi_allows_unknown_fields = uf; Aast.nsi_field_map = fm } ->
      ( p,
        Aast.Hshape
          {
            Aast.nsi_allows_unknown_fields = uf;
            Aast.nsi_field_map = List.map ~f:aux_sf fm;
          } )
    | Aast.Haccess _ -> (p, hint)
    | Aast.Hsoft h -> (p, Aast.Hsoft (aux h))
    | Aast.Hpu_access (h, name, pu_loc) ->
      let h = aux h in
      (p, Aast.Hpu_access (h, name, pu_loc))
    | _ -> failwith "todo"
  and aux_sf sfi = { sfi with Aast.sfi_hint = aux sfi.Aast.sfi_hint } in
  aux hint

and emit_reified_arg env ~isas pos (hint : Aast.hint) =
  let hint = fixup_type_arg env ~isas hint in
  let scope = Emit_env.get_scope env in
  let f tparam acc =
    match tparam with
    | { A.tp_name = (_, id); A.tp_reified; _ } when not (tp_reified = A.Erased)
      ->
      SSet.add id acc
    | _ -> acc
  in
  let current_targs =
    List.fold_right (Ast_scope.Scope.get_fun_tparams scope) ~init:SSet.empty ~f
  in
  let current_targs =
    List.fold_right
      (Ast_scope.Scope.get_class_tparams scope).A.c_tparam_list
      ~init:current_targs
      ~f
  in
  let acc = ref (0, SMap.empty) in
  let visitor =
    object
      inherit [_] A.iter as super

      method! on_hint_ _ h =
        let add_name name =
          let (i, map) = !acc in
          if SSet.mem name current_targs && not (SMap.mem name map) then
            acc := (i + 1, SMap.add name i map)
        in
        match h with
        | A.Haccess (_, sids) -> List.iter sids (fun sid -> add_name (snd sid))
        | A.Happly ((_, name), h) ->
          add_name name;
          let _ = List.map ~f:(super#on_hint ()) h in
          ()
        | A.Habstr name -> add_name name
        | _ ->
          ();
          super#on_hint_ () h
    end
  in
  visitor#on_hint () hint;
  let (count, targ_map) = !acc in
  match snd hint with
  | Aast.Happly ((_, name), []) when SSet.mem name current_targs ->
    (emit_reified_type env pos name, false)
  | _ ->
    let ts = get_type_structure_for_hint ~targ_map ~tparams:[] hint in
    let ts_list =
      if count = 0 then
        ts
      else
        (* Sort map from key 0 to count and convert each identified into cgetl *)
        let values =
          SMap.bindings targ_map
          |> List.sort ~compare:(fun (_, x) (_, y) -> Int.compare x y)
          |> List.map ~f:(fun (v, _) -> emit_reified_type env pos v)
        in
        gather [gather values; ts]
    in
    ( gather [ts_list; instr_combine_and_resolve_type_struct (count + 1)],
      count = 0 )

(* Emit arguments of a function call and inout setter for inout args *)
and emit_args_and_inout_setters env (args : Tast.expr list) =
  let aliases =
    if has_inout_args args then
      InoutLocals.collect_written_variables env args
    else
      SMap.empty
  in
  let emit_arg_and_inout_setter i (arg : Tast.expr) =
    match snd arg with
    (* inout $var *)
    | A.Callconv (Ast_defs.Pinout, (_, A.Lvar (pos, id))) ->
      let local = get_local env (pos, Local_id.get_name id) in
      let not_in_try = not (Emit_env.is_in_try env) in
      let move_instrs =
        if not_in_try && InoutLocals.should_move_local_value local aliases then
          gather [instr_null; instr_popl local]
        else
          empty
      in
      (gather [instr_cgetl local; move_instrs], instr_popl local)
    (* inout $arr[...][...] *)
    | A.Callconv
        (Ast_defs.Pinout, ((pos, _), A.Array_get (base_expr, opt_elem_expr))) ->
      let array_get_result =
        fst
          (emit_array_get_worker
             ~inout_param_info:(Some (i, aliases))
             env
             pos
             QueryOp.InOut
             base_expr
             opt_elem_expr)
      in
      begin
        match array_get_result with
        | Array_get_regular instrs ->
          let setter_base =
            fst
              (emit_array_get
                 ~no_final:true
                 ~mode:MemberOpMode.Define
                 env
                 pos
                 QueryOp.InOut
                 base_expr
                 opt_elem_expr)
          in
          let setter =
            gather
              [
                setter_base;
                instr_setm 0 (get_elem_member_key env 0 opt_elem_expr);
                instr_popc;
              ]
          in
          (instrs, setter)
        | Array_get_inout { load; store } -> rebuild_load_store load store
      end
    (* unsupported inout *)
    | A.Callconv (Ast_defs.Pinout, _) ->
      failwith "emit_arg_and_inout_setter: Unexpected inout expression type"
    (* regular argument *)
    | _ -> (emit_expr env arg, empty)
  in
  let rec aux i (args : Tast.expr list) =
    match args with
    | [] -> (empty, empty)
    | arg :: rem_args ->
      let (this_arg, this_setter) = emit_arg_and_inout_setter i arg in
      let (rem_args, rem_setters) = aux (i + 1) rem_args in
      (gather [this_arg; rem_args], gather [this_setter; rem_setters])
  in
  let (instr_args, instr_setters) = aux 0 args in
  if has_inout_args args then
    let retval = Local.get_unnamed_local () in
    (instr_args, gather [instr_popl retval; instr_setters; instr_pushl retval])
  else
    (instr_args, empty)

(* Create fcall_args for a given call *)
and get_fcall_args
    ?(lock_while_unwinding = false) ?context args uarg async_eager_label =
  let num_args = List.length args in
  let num_rets =
    List.fold_left args ~init:1 ~f:(fun acc arg ->
        if is_inout_arg arg then
          acc + 1
        else
          acc)
  in
  let flags =
    {
      default_fcall_flags with
      has_unpack = Option.is_some uarg;
      lock_while_unwinding;
    }
  in
  let inouts = List.map args is_inout_arg in
  make_fcall_args ~flags ~num_rets ~inouts ?async_eager_label ?context num_args

(* Expression that appears in an object context, such as expr->meth(...) *)
and emit_object_expr env (expr : Tast.expr) =
  let (_, expr_) = expr in
  match expr_ with
  | A.Lvar (_, x) when is_local_this env x -> instr_this
  | _ -> emit_expr env expr

and is_inout_arg = function
  | (_, A.Callconv (Ast_defs.Pinout, _)) -> true
  | _ -> false

and has_inout_args es = List.exists es ~f:is_inout_arg

and emit_call_lhs_and_fcall
    env (expr : Tast.expr) fcall_args (targs : Tast.targ list) =
  let ((pos, _), expr_) = expr in
  let does_not_have_non_tparam_generics =
    not (has_non_tparam_generics_targs env targs)
  in
  let emit_generics (flags, a, b, c, d, e) =
    if does_not_have_non_tparam_generics then
      (empty, (flags, a, b, c, d, e))
    else
      ( emit_reified_targs env pos (List.map ~f:snd targs),
        ({ flags with has_generics = true }, a, b, c, d, e) )
  in
  match expr_ with
  | A.Obj_get (obj, (_, A.String id), null_flavor)
  | A.Obj_get (obj, (_, A.Id (_, id)), null_flavor) ->
    let name = Hhbc_id.Method.from_ast_name id in
    let obj = emit_object_expr env obj in
    let (generics, fcall_args) = emit_generics fcall_args in
    let null_flavor = from_ast_null_flavor null_flavor in
    ( gather [obj; instr_nulluninit; instr_nulluninit],
      gather [generics; instr_fcallobjmethodd fcall_args name null_flavor] )
  | A.Obj_get (obj, method_expr, null_flavor) ->
    let obj = emit_object_expr env obj in
    let tmp = Local.get_unnamed_local () in
    let null_flavor = from_ast_null_flavor null_flavor in
    ( gather
        [
          obj;
          instr_nulluninit;
          instr_nulluninit;
          emit_expr env method_expr;
          instr_popl tmp;
        ],
      gather [instr_pushl tmp; instr_fcallobjmethod fcall_args null_flavor] )
  | A.Class_const (cid, (_, id)) ->
    let cexpr =
      class_id_to_class_expr ~resolve_self:false (Emit_env.get_scope env) cid
    in
    let method_id = Hhbc_id.Method.from_ast_name id in
    let method_id_string = Hhbc_id.Method.to_raw_string method_id in
    let cexpr =
      match cexpr with
      | Class_id (_, name) ->
        Option.value ~default:cexpr (get_reified_var_cexpr env pos name)
      | _ -> cexpr
    in
    begin
      match cexpr with
      (* Statically known *)
      | Class_id (_, cname) ->
        let cid = Hhbc_id.Class.from_ast_name cname in
        Emit_symbol_refs.add_class cid;
        let (generics, fcall_args) = emit_generics fcall_args in
        ( gather [instr_nulluninit; instr_nulluninit; instr_nulluninit],
          gather [generics; instr_fcallclsmethodd fcall_args method_id cid] )
      | Class_special clsref ->
        let (generics, fcall_args) = emit_generics fcall_args in
        ( gather [instr_nulluninit; instr_nulluninit; instr_nulluninit],
          gather [generics; instr_fcallclsmethodsd fcall_args clsref method_id]
        )
      | Class_expr expr ->
        let (generics, fcall_args) = emit_generics fcall_args in
        let emit_fcall instr_meth =
          gather
            [
              generics;
              instr_meth;
              emit_expr env expr;
              instr_classgetc;
              instr_fcallclsmethod
                ~is_log_as_dynamic_call:DontLogAsDynamicCall
                fcall_args;
            ]
        in
        ( gather [instr_nulluninit; instr_nulluninit; instr_nulluninit],
          emit_fcall (instr_string method_id_string) )
      | Class_reified instrs ->
        (* TODO(T31677864): Implement reification here *)
        let tmp = Local.get_unnamed_local () in
        ( gather
            [
              instr_nulluninit;
              instr_nulluninit;
              instr_nulluninit;
              instrs;
              instr_popl tmp;
            ],
          gather
            [
              instr_string method_id_string;
              instr_pushl tmp;
              instr_classgetc;
              instr_fcallclsmethod fcall_args;
            ] )
    end
  | A.Class_get (cid, e) ->
    let cexpr =
      class_id_to_class_expr ~resolve_self:false (Emit_env.get_scope env) cid
    in
    let emit_meth_name () =
      match e with
      | A.CGstring (pos, id) ->
        emit_pos_then pos @@ instr_cgetl (Local.Named id)
      | A.CGexpr e -> emit_expr env e
    in
    let cexpr =
      match cexpr with
      | Class_id (_, name) ->
        Option.value ~default:cexpr (get_reified_var_cexpr env pos name)
      | _ -> cexpr
    in
    begin
      match cexpr with
      | Class_id cid ->
        let tmp = Local.get_unnamed_local () in
        ( gather
            [
              instr_nulluninit;
              instr_nulluninit;
              instr_nulluninit;
              emit_meth_name ();
              instr_popl tmp;
            ],
          gather
            [
              instr_pushl tmp;
              emit_known_class_id cid;
              instr_fcallclsmethod fcall_args;
            ] )
      | Class_special clsref ->
        let tmp = Local.get_unnamed_local () in
        ( gather
            [
              instr_nulluninit;
              instr_nulluninit;
              instr_nulluninit;
              emit_meth_name ();
              instr_popl tmp;
            ],
          gather [instr_pushl tmp; instr_fcallclsmethods fcall_args clsref] )
      | Class_expr expr ->
        let cls = Local.get_unnamed_local () in
        let meth = Local.get_unnamed_local () in
        ( gather
            [
              instr_nulluninit;
              instr_nulluninit;
              instr_nulluninit;
              emit_expr env expr;
              instr_popl cls;
              emit_meth_name ();
              instr_popl meth;
            ],
          gather
            [
              instr_pushl meth;
              instr_pushl cls;
              instr_classgetc;
              instr_fcallclsmethod fcall_args;
            ] )
      | Class_reified instrs ->
        let cls = Local.get_unnamed_local () in
        let meth = Local.get_unnamed_local () in
        ( gather
            [
              instr_nulluninit;
              instr_nulluninit;
              instr_nulluninit;
              instrs;
              instr_popl cls;
              emit_meth_name ();
              instr_popl meth;
            ],
          gather
            [
              instr_pushl meth;
              instr_pushl cls;
              instr_classgetc;
              instr_fcallclsmethod fcall_args;
            ] )
    end
  | A.Id (_, s) ->
    let (flags, num_args, _, _, _, _) = fcall_args in
    let fq_id =
      match SU.strip_global_ns s with
      | "min" when num_args = 2 && not flags.has_unpack ->
        Hhbc_id.Function.from_raw_string "__SystemLib\\min2"
      | "max" when num_args = 2 && not flags.has_unpack ->
        Hhbc_id.Function.from_raw_string "__SystemLib\\max2"
      | _ -> Hhbc_id.Function.from_ast_name s
    in
    let (generics, fcall_args) = emit_generics fcall_args in
    ( gather [instr_nulluninit; instr_nulluninit; instr_nulluninit],
      gather [generics; instr_fcallfuncd fcall_args fq_id] )
  | A.String s ->
    let fq_id = Hhbc_id.Function.from_raw_string s in
    let (generics, fcall_args) = emit_generics fcall_args in
    ( gather [instr_nulluninit; instr_nulluninit; instr_nulluninit],
      gather [generics; instr_fcallfuncd fcall_args fq_id] )
  | _ ->
    let tmp = Local.get_unnamed_local () in
    ( gather
        [
          instr_nulluninit;
          instr_nulluninit;
          instr_nulluninit;
          emit_expr env expr;
          instr_popl tmp;
        ],
      gather [instr_pushl tmp; instr_fcallfunc fcall_args] )

and get_call_builtin_func_info id =
  match id with
  | "array_key_exists" -> Some (2, IMisc AKExists)
  | "hphp_array_idx" -> Some (3, IMisc ArrayIdx)
  | "intval" -> Some (1, IOp CastInt)
  | "boolval" -> Some (1, IOp CastBool)
  | "strval" -> Some (1, IOp CastString)
  | "floatval"
  | "doubleval" ->
    Some (1, IOp CastDouble)
  | "HH\\vec" -> Some (1, IOp CastVec)
  | "HH\\keyset" -> Some (1, IOp CastKeyset)
  | "HH\\dict" -> Some (1, IOp CastDict)
  | "HH\\varray" ->
    Some
      ( 1,
        IOp
          ( if hack_arr_dv_arrs () then
            CastVec
          else
            CastVArray ) )
  | "HH\\darray" ->
    Some
      ( 1,
        IOp
          ( if hack_arr_dv_arrs () then
            CastDict
          else
            CastDArray ) )
  | "HH\\global_get" -> Some (1, IGet CGetG)
  | "HH\\global_isset" -> Some (1, IIsset IssetG)
  | _ -> None

(* TODO: work out what HHVM does special here *)
and emit_name_string env e = emit_expr env e

and emit_special_function
    env pos annot id (args : Tast.expr list) (uarg : Tast.expr option) default =
  let nargs =
    List.length args
    +
    if Option.is_some uarg then
      1
    else
      0
  in
  let lower_fq_name =
    Hhbc_id.Function.from_ast_name id |> Hhbc_id.Function.to_raw_string
  in
  (* Make sure that we do not treat a special function that is aliased as not
   * aliased *)
  match (lower_fq_name, args) with
  | (id, _) when id = SN.SpecialFunctions.echo ->
    let instrs =
      gather
      @@ List.mapi args (fun i arg ->
             gather
               [
                 emit_expr env arg;
                 emit_pos pos;
                 instr (IOp Print);
                 ( if i = nargs - 1 then
                   empty
                 else
                   instr_popc );
               ])
    in
    Some instrs
  | ("HH\\invariant", e :: fmt :: rest) ->
    let l = Label.next_regular () in
    let annot = (pos, snd annot) in
    (* TODO: Can we capitalize hh to HH? *)
    let expr_id = (annot, A.Id (pos, "\\hh\\invariant_violation")) in
    Some
      (gather
         [
           (* Could use emit_jmpnz for better code *)
           emit_expr env e;
           instr_jmpnz l;
           emit_ignored_expr
             env
             (annot, A.Call (Aast.Cnormal, expr_id, [], fmt :: rest, uarg));
           Emit_fatal.emit_fatal_runtime pos "invariant_violation";
           instr_label l;
           instr_null;
         ])
  | ("assert", _) ->
    let l0 = Label.next_regular () in
    let l1 = Label.next_regular () in
    Some
      (gather
         [
           instr_string "zend.assertions";
           instr_fcallbuiltin 1 1 0 "ini_get";
           instr_int 0;
           instr_gt;
           instr_jmpz l0;
           default ();
           instr_jmp l1;
           instr_label l0;
           instr_true;
           instr_label l1;
         ])
  | ("HH\\sequence", []) -> Some instr_null
  | ("HH\\sequence", args) ->
    Some
      (gather (List.intersperse (List.map args ~f:(emit_expr env)) instr_popc))
  | ((("class_exists" | "interface_exists" | "trait_exists") as id), arg1 :: _)
    when nargs = 1 || nargs = 2 ->
    let class_kind =
      match id with
      | "class_exists" -> KClass
      | "interface_exists" -> KInterface
      | "trait_exists" -> KTrait
      | _ -> failwith "class_kind"
    in
    Some
      (gather
         [
           emit_name_string env arg1;
           instr (IOp CastString);
           ( if nargs = 1 then
             instr_true
           else
             gather [emit_expr env (List.nth_exn args 1); instr (IOp CastBool)]
           );
           instr (IMisc (OODeclExists class_kind));
         ])
  | (("exit" | "die"), _) when nargs = 0 || nargs = 1 ->
    Some (emit_exit env (List.hd args))
  | ("HH\\fun", _) ->
    if nargs <> 1 then
      Emit_fatal.raise_fatal_runtime
        pos
        ("fun() expects exactly 1 parameter, " ^ string_of_int nargs ^ " given")
    else (
      match args with
      | [(_, A.String func_name)] -> Some (emit_hh_fun func_name)
      | _ ->
        Emit_fatal.raise_fatal_runtime pos "Constant string expected in fun()"
    )
  | ("__systemlib\\meth_caller", _) ->
    (* used by meth_caller() to directly emit func ptr *)
    if nargs <> 1 then
      Emit_fatal.raise_fatal_runtime
        pos
        ("fun() expects exactly 1 parameter, " ^ string_of_int nargs ^ " given")
    else (
      match args with
      | [(_, A.String func_name)] ->
        let func_name = SU.strip_global_ns func_name in
        Some
          ( instr_resolve_meth_caller
          @@ Hhbc_id.Function.from_raw_string func_name )
      | _ ->
        Emit_fatal.raise_fatal_runtime pos "Constant string expected in fun()"
    )
  | ("__systemlib\\__debugger_is_uninit", _) ->
    if nargs <> 1 then
      Emit_fatal.raise_fatal_runtime
        pos
        ( "__debugger_is_uninit() expects exactly 1 parameter, "
        ^ string_of_int nargs
        ^ " given" )
    else (
      match args with
      | [(_, A.Lvar (_, id))] ->
        Some (instr_isunsetl (get_local env (pos, Local_id.get_name id)))
      | _ ->
        Emit_fatal.raise_fatal_runtime
          pos
          "Local variable expected in __debugger_is_uninit()"
    )
  | ("HH\\inst_meth", _) ->
    begin
      match args with
      | [obj_expr; method_name] ->
        Some (emit_inst_meth env obj_expr method_name)
      | _ ->
        Emit_fatal.raise_fatal_runtime
          pos
          ( "inst_meth() expects exactly 2 parameters, "
          ^ string_of_int nargs
          ^ " given" )
    end
  | ("HH\\class_meth", cls :: meth :: _) when nargs = 2 ->
    begin
      match (cls, meth) with
      | ((_, A.Class_const (_, (_, id))), (_, A.String _)) when SU.is_class id
        ->
        Some (emit_class_meth env cls meth)
      | ((_, A.Id (_, s)), (_, A.String _)) when s = SN.PseudoConsts.g__CLASS__
        ->
        Some (emit_class_meth env cls meth)
      | ((_, A.String _), (_, A.String _)) ->
        Some (emit_class_meth env cls meth)
      | _ ->
        Emit_fatal.raise_fatal_runtime
          pos
          ( "class_meth() expects a literal class name or ::class constant, "
          ^ "followed by a constant string that refers to a static method "
          ^ "on that class" )
    end
  | ("HH\\class_meth", _) ->
    Emit_fatal.raise_fatal_runtime
      pos
      ( "class_meth() expects exactly 2 parameters, "
      ^ string_of_int nargs
      ^ " given" )
  | ("HH\\global_set", _) ->
    begin
      match args with
      | [gkey; gvalue] ->
        Some
          (gather
             [
               emit_expr env gkey;
               emit_expr env gvalue;
               emit_pos pos;
               instr (IMutator SetG);
               instr_popc;
               instr_null;
             ])
      | _ ->
        Emit_fatal.raise_fatal_runtime
          pos
          ( "global_set() expects exactly 2 parameters, "
          ^ string_of_int nargs
          ^ " given" )
    end
  | ("HH\\global_unset", _) ->
    begin
      match args with
      | [gkey] ->
        Some
          (gather
             [
               emit_expr env gkey;
               emit_pos pos;
               instr (IMutator UnsetG);
               instr_null;
             ])
      | _ ->
        Emit_fatal.raise_fatal_runtime
          pos
          ( "global_unset() expects exactly 1 parameter, "
          ^ string_of_int nargs
          ^ " given" )
    end
  | ("__hhvm_internal_whresult", [(_, A.Lvar (_, param))])
    when Emit_env.is_systemlib () ->
    Some
      (gather
         [instr_cgetl (Local.Named (Local_id.get_name param)); instr_whresult])
  | ("__hhvm_internal_newlikearrayl", [(_, A.Lvar (_, param)); (_, A.Int n)])
    when Emit_env.is_systemlib () ->
    Some
      (instr
         (ILitConst
            (NewLikeArrayL
               (Local.Named (Local_id.get_name param), int_of_string n))))
  | _ ->
    begin
      match (args, istype_op lower_fq_name, is_isexp_op lower_fq_name) with
      | ([arg_expr], _, Some h) ->
        Some (gather [emit_expr env arg_expr; emit_is env pos h])
      | ([(_, A.Lvar ((_, arg_str) as arg_id))], Some i, _)
        when SN.Superglobals.is_superglobal (Local_id.get_name arg_str)
             || Local_id.get_name arg_str = SN.Superglobals.globals ->
        Some
          (gather
             [
               emit_local ~notice:NoNotice env arg_id;
               emit_pos pos;
               instr (IIsset (IsTypeC i));
             ])
      | ([(_, A.Lvar (arg_pos, arg_str))], Some i, _)
        when not (is_local_this env arg_str) ->
        Some
          (instr
             (IIsset
                (IsTypeL (get_local env (arg_pos, Local_id.get_name arg_str), i))))
      | ([arg_expr], Some i, _) ->
        Some
          (gather
             [emit_expr env arg_expr; emit_pos pos; instr (IIsset (IsTypeC i))])
      | _ ->
        begin
          match get_call_builtin_func_info lower_fq_name with
          | Some (nargs, i) when nargs = List.length args ->
            Some (gather [emit_exprs env args; emit_pos pos; instr i])
          | _ -> None
        end
    end

and emit_hh_fun func_name =
  let func_name = SU.strip_global_ns func_name in
  if Hhbc_options.emit_func_pointers !Hhbc_options.compiler_options then
    instr_resolve_func @@ Hhbc_id.Function.from_raw_string func_name
  else
    instr_string func_name

and emit_class_meth env cls meth =
  if Hhbc_options.emit_cls_meth_pointers !Hhbc_options.compiler_options then
    let method_id =
      match meth with
      | (_, A.String method_name) -> Hhbc_id.Method.from_raw_string method_name
      | _ -> failwith "emit_class_meth: unhandled method"
    in
    match cls with
    | ((pos, _), A.Class_const (cid, (_, id))) when SU.is_class id ->
      emit_class_meth_native env pos cid method_id
    | (_, A.Id (_, s)) when s = SN.PseudoConsts.g__CLASS__ ->
      instr_resolveclsmethods SpecialClsRef.Self method_id
    | (_, A.String class_name) ->
      instr_resolveclsmethodd
        (Hhbc_id.Class.from_raw_string @@ SU.strip_global_ns class_name)
        method_id
    | _ -> failwith "emit_class_meth: unhandled class"
  else
    gather
      [
        emit_expr env cls;
        emit_expr env meth;
        instr
          (ILitConst
             ( if hack_arr_dv_arrs () then
               NewVecArray 2
             else
               NewVArray 2 ));
      ]

and emit_class_meth_native env pos cid method_id =
  let cexpr =
    class_id_to_class_expr ~resolve_self:true (Emit_env.get_scope env) cid
  in
  let cexpr =
    match cexpr with
    | Class_id (_, name) ->
      Option.value ~default:cexpr (get_reified_var_cexpr env pos name)
    | _ -> cexpr
  in
  match cexpr with
  | Class_id (_, cname) ->
    instr_resolveclsmethodd (Hhbc_id.Class.from_ast_name cname) method_id
  | Class_special clsref -> instr_resolveclsmethods clsref method_id
  | Class_expr _ ->
    failwith "emit_class_meth_native: Class_expr should be impossible"
  | Class_reified instrs ->
    gather [instrs; instr_classgetc; instr_resolveclsmethod method_id]

and emit_inst_meth env obj_expr method_name =
  gather
    [
      emit_expr env obj_expr;
      emit_expr env method_name;
      ( if Hhbc_options.emit_inst_meth_pointers !Hhbc_options.compiler_options
      then
        instr_resolve_obj_method
      else
        instr
          (ILitConst
             ( if hack_arr_dv_arrs () then
               NewVecArray 2
             else
               NewVArray 2 )) );
    ]

and emit_call
    env
    pos
    expr
    (targs : Tast.targ list)
    (args : Tast.expr list)
    (uarg : Tast.expr option)
    async_eager_label =
  let (annot, expr_) = expr in
  (match expr_ with
  | A.Id (_, s) ->
    let fid = Hhbc_id.Function.from_ast_name s in
    Emit_symbol_refs.add_function fid
  | _ -> ());
  let fcall_args =
    get_fcall_args
      ?context:(Emit_env.get_call_context env)
      args
      uarg
      async_eager_label
  in
  let (_flags, _args, num_ret, _inouts, _eager, _ctx) = fcall_args in
  let num_uninit = num_ret - 1 in
  let default () =
    Scope.with_unnamed_locals @@ fun () ->
    let (instr_lhs, instr_fcall) =
      emit_call_lhs_and_fcall env expr fcall_args targs
    in
    let (instr_args, instr_inout_setters) =
      emit_args_and_inout_setters env args
    in
    let instr_uargs =
      match uarg with
      | None -> empty
      | Some uarg -> emit_expr env uarg
    in
    ( empty,
      gather
        [
          gather @@ List.init num_uninit ~f:(fun _ -> instr_nulluninit);
          instr_lhs;
          instr_args;
          instr_uargs;
          emit_pos pos;
          instr_fcall;
          instr_inout_setters;
        ],
      empty )
  in
  match expr_ with
  | A.Id (_, id) ->
    let special_fn_opt =
      emit_special_function env pos annot id args uarg default
    in
    begin
      match special_fn_opt with
      | Some instrs -> instrs
      | None -> default ()
    end
  | _ -> default ()

(* How do we make these work with reified generics? *)
and emit_function_pointer env (annot, e) _targs =
  match e with
  (* This is a function name. Equivalent to HH\fun('str') *)
  | A.Id (_, str) -> emit_hh_fun str
  (* class_meth *)
  | A.Class_const (cid, (_, method_name)) ->
    let method_id = Hhbc_id.Method.from_ast_name method_name in
    emit_class_meth_native env (fst annot) cid method_id
  (* inst_meth *)
  | A.Obj_get
      (obj_expr, (((pos, _) as annot), A.Id (_, method_name)), null_flavor) ->
    if null_flavor = A.OG_nullsafe then
      let end_label = Label.next_regular () in
      gather
        [
          fst (emit_quiet_expr env pos obj_expr);
          instr_dup;
          instr_istypec OpNull;
          instr_jmpnz end_label;
          emit_expr env (annot, A.String method_name);
          ( if
            Hhbc_options.emit_inst_meth_pointers !Hhbc_options.compiler_options
          then
            instr_resolve_obj_method
          else
            instr
              (ILitConst
                 ( if hack_arr_dv_arrs () then
                   NewVecArray 2
                 else
                   NewVArray 2 )) );
          instr_label end_label;
        ]
    else
      let substitute_method_name = (annot, A.String method_name) in
      emit_inst_meth env obj_expr substitute_method_name
  | _ -> failwith "What else could go here?"

and emit_final_member_op stack_index op mk =
  match op with
  | LValOp.Set -> instr (IFinal (SetM (stack_index, mk)))
  | LValOp.SetOp op -> instr (IFinal (SetOpM (stack_index, op, mk)))
  | LValOp.IncDec op -> instr (IFinal (IncDecM (stack_index, op, mk)))
  | LValOp.Unset -> instr (IFinal (UnsetM (stack_index, mk)))

and emit_final_local_op pos op lid =
  emit_pos_then pos
  @@
  match op with
  | LValOp.Set -> instr (IMutator (SetL lid))
  | LValOp.SetOp op -> instr (IMutator (SetOpL (lid, op)))
  | LValOp.IncDec op -> instr (IMutator (IncDecL (lid, op)))
  | LValOp.Unset -> instr (IMutator (UnsetL lid))

and emit_final_global_op pos op =
  match op with
  | LValOp.Set -> emit_pos_then pos @@ instr (IMutator SetG)
  | LValOp.SetOp op -> instr (IMutator (SetOpG op))
  | LValOp.IncDec op -> instr (IMutator (IncDecG op))
  | LValOp.Unset -> emit_pos_then pos @@ instr (IMutator UnsetG)

and emit_final_static_op cid (prop : Tast.class_get_expr) op =
  match op with
  | LValOp.Set -> instr (IMutator SetS)
  | LValOp.SetOp op -> instr (IMutator (SetOpS op))
  | LValOp.IncDec op -> instr (IMutator (IncDecS op))
  | LValOp.Unset ->
    let pos =
      match prop with
      | A.CGexpr ((pos, _), _) -> pos
      | A.CGstring (pos, _) -> pos
    in
    let cid = text_of_class_id cid in
    let id = text_of_prop prop in
    Emit_fatal.emit_fatal_runtime
      pos
      ("Attempt to unset static property " ^ SU.strip_ns cid ^ "::" ^ id)

(* Given a local $local and a list of integer array indices i_1, ..., i_n,
 * generate code to extract the value of $local[i_n]...[i_1]:
 *   BaseL $local Warn
 *   Dim Warn EI:i_n ...
 *   Dim Warn EI:i_2
 *   QueryM 0 CGet EI:i_1
 *)
and emit_array_get_fixed last_usage local indices =
  let (base, stack_count) =
    if last_usage then
      (gather [instr_pushl local; instr_basec 0 MemberOpMode.Warn], 1)
    else
      (instr_basel local MemberOpMode.Warn, 0)
  in
  let indices =
    gather
    @@ List.rev_mapi indices (fun i ix ->
           let mk = MemberKey.EI (Int64.of_int ix) in
           if i = 0 then
             instr (IFinal (QueryM (stack_count, QueryOp.CGet, mk)))
           else
             instr (IBase (Dim (MemberOpMode.Warn, mk))))
  in
  gather [base; indices]

and can_use_as_rhs_in_list_assignment (expr : Tast.expr_) =
  Aast.(
    match expr with
    | Call (_, (_, Id (_, s)), _, _, _) when s = SN.SpecialFunctions.echo ->
      false
    | Lvar _
    | Array_get _
    | Obj_get _
    | Class_get _
    | PU_atom _
    | Call _
    | FunctionPointer _
    | New _
    | Record _
    | Expr_list _
    | Yield _
    | Cast _
    | Eif _
    | Array _
    | Varray _
    | Darray _
    | Collection _
    | Clone _
    | Unop _
    | As _
    | Await _ ->
      true
    | Pipe (_, _, (_, r))
    | Binop (Ast_defs.Eq None, (_, List _), (_, r)) ->
      can_use_as_rhs_in_list_assignment r
    | Binop (Ast_defs.Plus, _, _)
    | Binop (Ast_defs.QuestionQuestion, _, _)
    | Binop (Ast_defs.Eq _, _, _)
    | Class_const _ ->
      true
    (* Everything below is false *)
    | This
    | Any
    | ValCollection _
    | KeyValCollection _
    | Dollardollar _
    | Lplaceholder _
    | Fun_id _
    | Method_id (_, _)
    | Method_caller (_, _)
    | Smethod_id (_, _)
    | Pair (_, _)
    | Assert _
    | Typename _
    | Binop _
    | Shape _
    | Null
    | True
    | False
    | Omitted
    | Id _
    | Int _
    | Float _
    | String _
    | String2 _
    | PrefixedString _
    | Yield_break
    | Yield_from _
    | Suspend _
    | Is _
    | BracedExpr _
    | ParenthesizedExpr _
    | Efun _
    | Lfun _
    | Xml _
    | Import _
    | Callconv _
    | List _ ->
      false
    | PU_identifier _ ->
      failwith "TODO(T35357243): Pocket Universes syntax must be erased by now")

(* Generate code for each lvalue assignment in a list destructuring expression.
 * Lvalues are assigned right-to-left, regardless of the nesting structure. So
 *     list($a, list($b, $c)) = $d
 * and list(list($a, $b), $c) = $d
 * will both assign to $c, $b and $a in that order.
 * Returns a pair of instructions:
 * 1. initialization part of the left hand side
 * 2. assignment
 * this is necessary to handle cases like:
 * list($a[$f()]) = b();
 * here f() should be invoked before b()
 *)
and emit_lval_op_list
    ?(last_usage = false)
    (env : Emit_env.t)
    (outer_pos : Pos.t)
    local
    (indices : int list)
    (expr : Tast.expr) =
  let is_ltr = php7_ltr_assign () in
  match snd expr with
  | A.List exprs ->
    let last_non_omitted =
      (* last usage of the local will happen when processing last non-omitted
         element in the list - find it *)
      if last_usage then
        if is_ltr then
          exprs
          |> List.foldi ~init:None ~f:(fun i acc (_, v) ->
                 if v = A.Omitted then
                   acc
                 else
                   Some i)
        (* in right-to-left case result list will be reversed
           so we need to find first non-omitted expression *)
        else
          exprs
          |> List.findi ~f:(fun _ (_, v) -> v <> A.Omitted)
          |> Option.map ~f:fst
      else
        None
    in
    let (lhs_instrs, set_instrs) =
      List.mapi exprs (fun i expr ->
          emit_lval_op_list
            ~last_usage:(Some i = last_non_omitted)
            env
            outer_pos
            local
            (i :: indices)
            expr)
      |> List.unzip
    in
    ( gather lhs_instrs,
      gather
        ( if not is_ltr then
          List.rev set_instrs
        else
          set_instrs ) )
  | A.Omitted -> (empty, empty)
  | _ ->
    (* Generate code to access the element from the array *)
    let access_instrs =
      match (local, indices) with
      | (Some local, _ :: _) -> emit_array_get_fixed last_usage local indices
      | (Some local, []) ->
        if last_usage then
          instr_pushl local
        else
          instr_cgetl local
      | (None, _) -> instr_null
    in
    (* Generate code to assign to the lvalue *)
    (* Return pair: side effects to initialize lhs + assignment *)
    let (lhs_instrs, rhs_instrs, set_op) =
      emit_lval_op_nonlist_steps env outer_pos LValOp.Set expr access_instrs 1
    in
    let lhs =
      if is_ltr then
        empty
      else
        lhs_instrs
    in
    let rest =
      gather
        [
          ( if is_ltr then
            lhs_instrs
          else
            empty );
          rhs_instrs;
          set_op;
          instr_popc;
        ]
    in
    (lhs, rest)

(* Emit code for an l-value operation *)
and emit_lval_op
    ?(null_coalesce_assignment = false)
    (env : Emit_env.t)
    pos
    op
    (expr1 : Tast.expr)
    opt_expr2 =
  match (op, expr1, opt_expr2) with
  (* Special case for list destructuring, only on assignment *)
  | (LValOp.Set, (_, A.List l), Some expr2) ->
    let instr_rhs = emit_expr env expr2 in
    let has_elements =
      List.exists l ~f:(function
          | (_, A.Omitted) -> false
          | _ -> true)
    in
    if not has_elements then
      instr_rhs
    else
      Scope.with_unnamed_local @@ fun local ->
      let loc =
        if can_use_as_rhs_in_list_assignment (snd expr2) then
          Some local
        else
          None
      in
      let (instr_lhs, instr_assign) = emit_lval_op_list env pos loc [] expr1 in
      (* before *)
      ( gather [instr_lhs; instr_rhs; instr_popl local],
        (* innner *)
        instr_assign,
        (* after *)
        instr_pushl local )
  | _ ->
    Local.scope @@ fun () ->
    let (rhs_instrs, rhs_stack_size) =
      match opt_expr2 with
      | None -> (empty, 0)
      | Some (_, A.Yield af) ->
        let temp = Local.get_unnamed_local () in
        ( gather
            [
              emit_yield env pos af;
              instr_setl temp;
              instr_popc;
              instr_pushl temp;
            ],
          1 )
      | Some e -> (emit_expr env e, 1)
    in
    emit_lval_op_nonlist
      ~null_coalesce_assignment
      env
      pos
      op
      expr1
      rhs_instrs
      rhs_stack_size

and emit_lval_op_nonlist
    ?(null_coalesce_assignment = false) env pos op e rhs_instrs rhs_stack_size =
  let (lhs, rhs, setop) =
    emit_lval_op_nonlist_steps
      ~null_coalesce_assignment
      env
      pos
      op
      e
      rhs_instrs
      rhs_stack_size
  in
  gather [lhs; rhs; setop]

and emit_lval_op_nonlist_steps
    ?(null_coalesce_assignment = false)
    (env : Emit_env.t)
    outer_pos
    op
    (expr : Tast.expr)
    rhs_instrs
    rhs_stack_size =
  let ((pos, _), expr_) = expr in
  let env =
    match op with
    (* Unbelieveably, $test[] += 5; is legal in PHP, but $test[] = $test[] + 5 is not *)
    | LValOp.Set
    | LValOp.SetOp _
    | LValOp.IncDec _ ->
      { env with Emit_env.env_allows_array_append = true }
    | _ -> env
  in
  match expr_ with
  | A.Lvar (name_pos, id)
    when SN.Superglobals.is_superglobal (Local_id.get_name id)
         || Local_id.get_name id = SN.Superglobals.globals ->
    ( emit_pos_then name_pos
      @@ instr_string
      @@ SU.Locals.strip_dollar (Local_id.get_name id),
      rhs_instrs,
      emit_final_global_op outer_pos op )
  | A.Lvar ((_, str) as id) when is_local_this env str && is_incdec op ->
    (emit_local ~notice:Notice env id, rhs_instrs, empty)
  | A.Lvar (pos, name) when (not (is_local_this env name)) || op = LValOp.Unset
    ->
    ( empty,
      rhs_instrs,
      emit_final_local_op
        outer_pos
        op
        (get_local env (pos, Local_id.get_name name)) )
  | A.Array_get ((_, A.Lvar (_, x)), Some e)
    when Local_id.get_name x = SN.Superglobals.globals ->
    let final_global_op_instrs = emit_final_global_op pos op in
    if rhs_stack_size = 0 then
      (emit_expr env e, empty, final_global_op_instrs)
    else
      let (index_instrs, under_top) = emit_first_expr env e in
      if under_top then
        (empty, gather [rhs_instrs; index_instrs], final_global_op_instrs)
      else
        (index_instrs, rhs_instrs, final_global_op_instrs)
  | A.Array_get (_, None) when not (Emit_env.does_env_allow_array_append env) ->
    Emit_fatal.raise_fatal_runtime pos "Can't use [] for reading"
  | A.Array_get (base_expr, opt_elem_expr) ->
    let mode =
      match op with
      | LValOp.Unset -> MemberOpMode.Unset
      | _ -> MemberOpMode.Define
    in
    let (elem_expr_instrs, elem_stack_size) =
      emit_elem_instrs
        ~local_temp_kind:None
        ~null_coalesce_assignment
        env
        opt_elem_expr
    in
    let elem_expr_instrs =
      if null_coalesce_assignment then
        empty
      else
        elem_expr_instrs
    in
    let base_offset = elem_stack_size + rhs_stack_size in
    let ( base_expr_instrs_begin,
          base_expr_instrs_end,
          base_setup_instrs,
          base_stack_size,
          cls_stack_size ) =
      emit_base
        ~is_object:false
        ~notice:Notice
        ~null_coalesce_assignment
        env
        mode
        base_offset
        rhs_stack_size
        base_expr
    in
    let mk =
      get_elem_member_key
        ~null_coalesce_assignment
        env
        (rhs_stack_size + cls_stack_size)
        opt_elem_expr
    in
    let total_stack_size = elem_stack_size + base_stack_size + cls_stack_size in
    let final_instr =
      emit_pos_then pos @@ emit_final_member_op total_stack_size op mk
    in
    ( gather
        [
          ( if null_coalesce_assignment then
            empty
          else
            base_expr_instrs_begin );
          elem_expr_instrs;
          ( if null_coalesce_assignment then
            empty
          else
            base_expr_instrs_end );
        ],
      rhs_instrs,
      gather [emit_pos pos; base_setup_instrs; final_instr] )
  | A.Obj_get (e1, e2, null_flavor) ->
    if null_flavor = Ast_defs.OG_nullsafe then
      Emit_fatal.raise_fatal_parse pos "?-> is not allowed in write context";
    let mode =
      match op with
      | LValOp.Unset -> MemberOpMode.Unset
      | _ -> MemberOpMode.Define
    in
    let (_, _, prop_stack_size) =
      emit_prop_expr ~null_coalesce_assignment env null_flavor 0 e2
    in
    let base_offset = prop_stack_size + rhs_stack_size in
    let ( base_expr_instrs_begin,
          base_expr_instrs_end,
          base_setup_instrs,
          base_stack_size,
          cls_stack_size ) =
      emit_base
        ~notice:Notice
        ~is_object:true
        ~null_coalesce_assignment
        env
        mode
        base_offset
        rhs_stack_size
        e1
    in
    let (mk, prop_expr_instrs, _) =
      emit_prop_expr
        ~null_coalesce_assignment
        env
        null_flavor
        (rhs_stack_size + cls_stack_size)
        e2
    in
    let prop_expr_instrs =
      if null_coalesce_assignment then
        empty
      else
        prop_expr_instrs
    in
    let total_stack_size = prop_stack_size + base_stack_size + cls_stack_size in
    let final_instr =
      emit_pos_then pos @@ emit_final_member_op total_stack_size op mk
    in
    ( gather
        [
          ( if null_coalesce_assignment then
            empty
          else
            base_expr_instrs_begin );
          prop_expr_instrs;
          ( if null_coalesce_assignment then
            empty
          else
            base_expr_instrs_end );
        ],
      rhs_instrs,
      gather [base_setup_instrs; final_instr] )
  | A.Class_get (cid, prop) ->
    let cexpr =
      class_id_to_class_expr ~resolve_self:false (Emit_env.get_scope env) cid
    in
    let final_instr = emit_pos_then pos @@ emit_final_static_op cid prop op in
    (of_pair @@ emit_class_expr env cexpr prop, rhs_instrs, final_instr)
  | A.Unop (uop, e) ->
    ( empty,
      rhs_instrs,
      gather
        [emit_lval_op_nonlist env pos op e empty rhs_stack_size; from_unop uop]
    )
  | _ ->
    Emit_fatal.raise_fatal_parse pos "Can't use return value in write context"

and from_unop op =
  let check_int_overflow =
    Hhbc_options.check_int_overflow !Hhbc_options.compiler_options
  in
  match op with
  | Ast_defs.Utild -> instr (IOp BitNot)
  | Ast_defs.Unot -> instr (IOp Not)
  | Ast_defs.Uplus ->
    instr
      (IOp
         ( if check_int_overflow then
           AddO
         else
           Add ))
  | Ast_defs.Uminus ->
    instr
      (IOp
         ( if check_int_overflow then
           SubO
         else
           Sub ))
  | Ast_defs.Uincr
  | Ast_defs.Udecr
  | Ast_defs.Upincr
  | Ast_defs.Updecr
  | Ast_defs.Usilence ->
    failwith "this unary operation cannot be translated"

and emit_unop env pos op e =
  match op with
  | Ast_defs.Utild
  | Ast_defs.Unot ->
    gather [emit_expr env e; emit_pos_then pos @@ from_unop op]
  | Ast_defs.Uplus
  | Ast_defs.Uminus ->
    gather
      [
        emit_pos pos;
        instr (ILitConst (Int Int64.zero));
        emit_expr env e;
        emit_pos_then pos @@ from_unop op;
      ]
  | Ast_defs.Uincr
  | Ast_defs.Udecr
  | Ast_defs.Upincr
  | Ast_defs.Updecr ->
    emit_lval_op env pos (LValOp.IncDec (unop_to_incdec_op op)) e None
  | Ast_defs.Usilence ->
    Local.scope @@ fun () ->
    let temp_local = Local.get_unnamed_local () in
    gather
      [
        emit_pos pos;
        instr_silence_start temp_local;
        create_try_catch
          (emit_expr env e)
          (gather [emit_pos pos; instr_silence_end temp_local]);
        emit_pos pos;
        instr_silence_end temp_local;
      ]

and emit_exprs env (exprs : Tast.expr list) =
  match exprs with
  | [] -> empty
  | expr :: exprs ->
    gather (emit_expr env expr :: List.map exprs (emit_expr env))
