(**
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
  let opts = !Opts.compiler_options in
  Emit_env.is_hh_syntax_enabled () &&
  (Opts.enable_hiphop_syntax opts) &&
  not (Opts.jit_enable_rename_function opts)

let max_array_elem_on_stack () =
  Hhbc_options.max_array_elem_size_on_the_stack !Hhbc_options.compiler_options

type genva_inline_context =
  | GI_expression
  | GI_ignore_result
  | GI_list_assignment of A.expr list

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

let is_local_this env (lid: Aast.local_id): bool =
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
    { first_inout = Int.max_value; last_write = Int.min_value; num_uses = 0; }

  let add_inout i r =
    if i < r.first_inout then { r with first_inout = i } else r

  let add_write i r =
    if i > r.last_write then { r with last_write = i } else r

  let add_use _i r =
    { r with num_uses = r.num_uses + 1 }

  let in_range i r = i > r.first_inout || i <= r.last_write
  let has_single_ref r = r.num_uses < 2

  let update name i f m =
    let r =
      SMap.get name m
      |> Option.value ~default:not_aliased
      |> f i in
    SMap.add name r m

  let add_write name i m = update (Local_id.get_name name) i add_write m
  let add_inout name i m = update (Local_id.get_name name) i add_inout m
  let add_use name i m = update (Local_id.get_name name) i add_use m

  let collect_written_variables env (args : A.expr list) : alias_info SMap.t =
    (* check value of the argument *)
    let rec handle_arg ~is_top i acc (arg : A.expr) =
      match snd arg with
      (* inout $v *)
      | A.Callconv (Ast.Pinout, (_, A.Lvar (_, id)))
        when not (is_local_this env id) ->
        let acc = add_use id i acc in
        if is_top then add_inout id i acc else add_write id i acc
      (* &$v *)
      | A.Unop (Ast.Uref, (_, A.Lvar (_, id))) ->
        let acc = add_use id i acc in
        add_write id i acc
      (* $v *)
      | A.Lvar (_, id) ->
        let acc = add_use id i acc in
        acc
      | _ ->
      (* dive into argument value *)
        dive i acc arg

    (* collect lvars on the left hand side of '=' operator *)
    and collect_lvars_lhs i acc (e : A.expr) =
      match snd e with
      | A.Lvar (_, id) when not (is_local_this env id) ->
        let acc = add_use id i acc in
        add_write id i acc
      | A.List exprs ->
        List.fold_left exprs ~f:(collect_lvars_lhs i) ~init:acc
      | _ -> acc

    (* descend into expression *)
    and dive i (acc : alias_info SMap.t) expr: alias_info SMap.t =
      let state = ref acc in
      let visitor = object (_)
        inherit [_] A.iter as super

        (* lhs op= _ *)
        method! on_Binop _ bop l r =
          let _ =
            match bop with
            | Ast.Eq _ -> state := collect_lvars_lhs i !state l
            | _ -> () in
          super#on_Binop () bop l r

        (* $i++ or $i-- *)
        method! on_Unop _ op e =
          let _ =
            match op with
            | Ast.Uincr
            | Ast.Udecr -> state := collect_lvars_lhs i !state e
            | _ -> () in
          super#on_Unop () op e

        (* $v *)
        method! on_Lvar _ (_p, id) =
          let _ = state := add_use id 0 !state in
          super#on_Lvar () (_p, id)

        (* f(inout $v) or f(&$v) *)
        method! on_Call _ _ _ _ args uargs =
          let f = handle_arg ~is_top:false i in
          List.iter args ~f:(fun arg -> state := f !state arg);
          List.iter uargs ~f:(fun arg -> state := f !state arg)

      end in
      visitor#on_expr () expr;
      !state
    in
    List.foldi args ~f:(handle_arg ~is_top:true) ~init:SMap.empty

  (* determines if value of a local 'name' that appear in parameter 'i'
     should be saved to local because it might be overwritten later *)
  let should_save_local_value name i aliases =
    Option.value_map ~default:false ~f:(in_range i) (SMap.get name aliases)
  let should_move_local_value local aliases =
    match local with
    | Local.Named name ->
      Option.value_map ~default:true ~f:has_single_ref (SMap.get name aliases)
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
  | [] -> [], []
  | (i, None) :: xs ->
    let ld, st = aux xs in
    i :: ld, st
  | (i, Some (l, kind)) :: xs ->
    let ld, st = aux xs in
    let set =
      if kind = Value_kind_expression then instr_setl l else instr_popl l in
    let unset = instr_unsetl l in
    i :: set :: ld, unset :: st
  in
  let ld, st = aux load in
  gather ld, gather (store :: st)

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
    store: Instruction_sequence.t
  }

type 'a array_get_base_data = {
  instrs_begin: 'a;
  instrs_end: Instruction_sequence.t;
  setup_instrs: Instruction_sequence.t;
  stack_size: int
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
    store: Instruction_sequence.t
  }

let is_incdec op =
  match op with
  | LValOp.IncDec _ -> true
  | _ -> false

let is_global_namespace env =
  Namespace_env.is_global_namespace (Emit_env.get_namespace env)

let enable_intrinsics_extension () =
  Hhbc_options.enable_intrinsics_extension !Hhbc_options.compiler_options

let optimize_null_check () =
  Hhbc_options.optimize_null_check !Hhbc_options.compiler_options

let hack_arr_compat_notices () =
  Hhbc_options.hack_arr_compat_notices !Hhbc_options.compiler_options

let hack_arr_dv_arrs () =
  Hhbc_options.hack_arr_dv_arrs !Hhbc_options.compiler_options

let php7_ltr_assign () =
  Hhbc_options.php7_ltr_assign !Hhbc_options.compiler_options

let reified_generics () =
  Hhbc_options.enable_reified_generics !Hhbc_options.compiler_options ||
  Emit_env.is_systemlib ()

(* Strict binary operations; assumes that operands are already on stack *)
let from_binop op =
  match op with
  | Ast.Plus -> instr (IOp Add)
  | Ast.Minus -> instr (IOp Sub)
  | Ast.Star -> instr (IOp Mul)
  | Ast.Slash -> instr (IOp Div)
  | Ast.Eqeq -> instr (IOp Eq)
  | Ast.Eqeqeq -> instr (IOp Same)
  | Ast.Starstar -> instr (IOp Pow)
  | Ast.Diff -> instr (IOp Neq)
  | Ast.Diff2 -> instr (IOp NSame)
  | Ast.Lt -> instr (IOp Lt)
  | Ast.Lte -> instr (IOp Lte)
  | Ast.Gt -> instr (IOp Gt)
  | Ast.Gte -> instr (IOp Gte)
  | Ast.Dot -> instr (IOp Concat)
  | Ast.Amp -> instr (IOp BitAnd)
  | Ast.Bar -> instr (IOp BitOr)
  | Ast.Ltlt -> instr (IOp Shl)
  | Ast.Gtgt -> instr (IOp Shr)
  | Ast.Cmp -> instr (IOp Cmp)
  | Ast.Percent -> instr (IOp Mod)
  | Ast.Xor -> instr (IOp BitXor)
  | Ast.LogXor -> instr (IOp Xor)
  | Ast.Eq _ -> failwith "assignment is emitted differently"
  | Ast.QuestionQuestion -> failwith "null coalescence is emitted differently"
  | Ast.Ampamp
  | Ast.Barbar ->
    failwith "short-circuiting operator cannot be generated as a simple binop"

let binop_to_eqop op =
  match op with
  | Ast.Plus -> Some PlusEqual
  | Ast.Minus -> Some MinusEqual
  | Ast.Star -> Some MulEqual
  | Ast.Slash -> Some DivEqual
  | Ast.Starstar -> Some PowEqual
  | Ast.Amp -> Some AndEqual
  | Ast.Bar -> Some OrEqual
  | Ast.Xor -> Some XorEqual
  | Ast.Ltlt -> Some SlEqual
  | Ast.Gtgt -> Some SrEqual
  | Ast.Percent -> Some ModEqual
  | Ast.Dot -> Some ConcatEqual
  | _ -> None

let unop_to_incdec_op op =
  match op with
  | Ast.Uincr -> PreInc
  | Ast.Udecr -> PreDec
  | Ast.Upincr -> PostInc
  | Ast.Updecr -> PostDec
  | _ -> failwith "invalid incdec op"

let collection_type = function
  | "Vector"    -> CollectionType.Vector
  | "Map"       -> CollectionType.Map
  | "Set"       -> CollectionType.Set
  | "Pair"      -> CollectionType.Pair
  | "ImmVector" -> CollectionType.ImmVector
  | "ImmMap"    -> CollectionType.ImmMap
  | "ImmSet"    -> CollectionType.ImmSet
  | x -> failwith ("unknown collection type '" ^ x ^ "'")

let istype_op lower_fq_id =
  match lower_fq_id with
  | "is_int" | "is_integer" | "is_long" -> Some OpInt
  | "is_bool" -> Some OpBool
  | "is_float" | "is_real" | "is_double" -> Some OpDbl
  | "is_string" -> Some OpStr
  | "is_array" -> Some OpArr
  | "is_object" -> Some OpObj
  | "is_null" -> Some OpNull
  (* We don't use IsType with the resource type because `is_resource()` does
     validation in addition to a simple type check. We will use it for
     is-expressions because they only do type checks.
  | "is_resource" -> Some OpRes *)
  | "is_scalar" -> Some OpScalar
  | "hh\\is_keyset" -> Some OpKeyset
  | "hh\\is_dict" -> Some OpDict
  | "hh\\is_vec" -> Some OpVec
  | "hh\\is_varray" -> Some (if hack_arr_dv_arrs () then OpVec else OpVArray)
  | "hh\\is_darray" -> Some (if hack_arr_dv_arrs () then OpDict else OpDArray)
  | "hh\\is_any_array" -> Some OpArrLike
  | "hh\\is_class_meth" -> Some OpClsMeth
  | _ -> None

(* T29079834: Using this for the is expressions migration *)
let is_isexp_op lower_fq_id: Aast.hint option =
  if not (Hhbc_options.enable_is_expr_primitive_migration !Hhbc_options.compiler_options)
  then None else
  let h n = Pos.none, Aast.Happly ((Pos.none, n), []) in
  match lower_fq_id with
  | "is_int" | "is_integer" | "is_long" -> Some (h "int")
  | "is_bool" -> Some (h "bool")
  | "is_float" | "is_real" | "is_double" -> Some (h "double")
  | "is_string" -> Some (h "string")
  | "is_null" -> Some (h "void")
  | "hh\\is_keyset" -> Some (h "keyset")
  | "hh\\is_dict" -> Some (h "dict")
  | "hh\\is_vec" -> Some (h "vec")
  | _ -> None

let get_queryMOpMode need_ref op =
  match op with
  | QueryOp.InOut -> MemberOpMode.InOut
  | QueryOp.CGet -> MemberOpMode.Warn
  | QueryOp.Empty when need_ref -> MemberOpMode.Define
  | _ -> MemberOpMode.ModeNone

(* Returns either Some (index, is_soft) or None *)
let is_reified_tparam ~(is_fun : bool) (env : Emit_env.t) (name : string) =
  let scope = Emit_env.get_scope env in
  let tparams =
    if is_fun then Ast_scope.Scope.get_fun_tparams scope
    else (Ast_scope.Scope.get_class_tparams scope).A.c_tparam_list
  in
  let is_soft =
    List.exists ~f:(function { A.ua_name = n; _ } ->
      snd n = SN.UserAttributes.uaSoft) in
  List.find_mapi tparams
    ~f:(fun i { A.tp_name = (_, id)
              ; A.tp_reified = reified
              ; A.tp_user_attributes = ual
              ; _ } ->
      if (reified = A.Reified || reified = A.SoftReified)
          && id = name then Some (i, is_soft ual) else None)

let extract_shape_field_name_pstring env annot = function
  | Ast.SFlit_int s -> A.Int (snd s)
  | Ast.SFlit_str s -> A.String (snd s)
  | Ast.SFclass_const ((pn, name) as id, p) ->
    if Option.is_some (is_reified_tparam ~is_fun:true env name) ||
       Option.is_some (is_reified_tparam ~is_fun:false env name) then
      Emit_fatal.raise_fatal_parse pn
        "Reified generics cannot be used in shape keys";
    A.Class_const ((annot, A.CI id), p)

let rec text_of_expr (e : A.expr) = match e with
  (* Note we force string literals to become single-quoted, regardless of
     whether they were single- or double-quoted in the source. Gross. *)
  | _, A.String s -> "'" ^ s ^ "'"
  | _, A.Id (_, id) -> id
  | _, A.Lvar (_, id) -> Local_id.get_name id
  | _, A.Array_get ((_, A.Lvar (_, id)), Some e) ->
     (Local_id.get_name id) ^ "[" ^ (text_of_expr e) ^ "]"
  | _ -> "unknown" (* TODO: get text of expression *)

let text_of_class_id (cid : A.class_id) =
  match (snd cid) with
  | A.CIparent -> "parent"
  | A.CIself -> "self"
  | A.CIstatic -> "static"
  | A.CIexpr e -> text_of_expr e
  | A.CI (_, id) -> id

let text_of_prop (prop : A.class_get_expr) =
  match prop with
  | A.CGstring (_, s) -> s
  | A.CGexpr e -> text_of_expr e

let parse_include (e : A.expr) =
  let strip_backslash p =
    let len = String.length p in
    if len > 0 && p.[0] = '/' then String.sub p 1 (len-1) else p in
  let rec split_var_lit = function
    | _, A.Binop (Ast.Dot, e1, e2) -> begin
      let v, l = split_var_lit e2 in
      if v = ""
      then let var, lit = split_var_lit e1 in var, lit ^ l
      else v, ""
    end
    | _, A.String lit -> "", lit
    | e -> text_of_expr e, "" in
  let var, lit = split_var_lit e in
  let var, lit =
    if var = "__DIR__" then ("", strip_backslash lit) else (var, lit) in
  if var = ""
  then
    if Filename.is_relative lit
    then Hhas_symbol_refs.SearchPathRelative lit
    else Hhas_symbol_refs.Absolute lit
  else
    Hhas_symbol_refs.IncludeRootRelative (var, strip_backslash lit)

let rec expr_and_new env pos instr_to_add_new instr_to_add = function
  | A.AFvalue e -> gather [emit_expr env e; emit_pos pos; instr_to_add_new]
  | A.AFkvalue (k, v) ->
    gather [
      emit_two_exprs env (fst @@ fst k) k v;
      instr_to_add;
    ]

and get_local env (pos, (str : string)): Hhbc_ast.local_id =
  if str = SN.SpecialIdents.dollardollar
  then
    match Emit_env.get_pipe_var env with
    | None -> Emit_fatal.raise_fatal_runtime pos
      "Pipe variables must occur only in the RHS of pipe expressions"
    | Some v -> v
  else (if SN.SpecialIdents.is_tmp_var str
    then Local.get_unnamed_local_for_tempname str
    else Local.Named str)

and emit_local ~notice ~need_ref env (lid: Aast.lid) =
  let (pos, id) = lid in
  let str = Local_id.get_name id in
  if SN.Superglobals.is_superglobal str then
    if need_ref then
      Emit_fatal.raise_fatal_parse pos
        "Superglobals may not be taken by reference."
    else gather [
      instr_string (SU.Locals.strip_dollar str);
      emit_pos pos;
      instr (IGet CGetG)
    ]
  else
  let local = get_local env (pos, str) in
  if is_local_this env id && not (Emit_env.get_needs_local_this env) then
    if need_ref then
      instr_vgetl local
    else
      emit_pos_then pos @@ instr (IMisc (BareThis notice))
  else if need_ref then
    instr_vgetl local
  else
    instr_cgetl local

(* Emit CGetL2 for local variables, and return true to indicate that
 * the result will be just below the top of the stack *)
and emit_first_expr env (expr : A.expr) =
  match snd expr with
  | A.Lvar (pos, id)
    when not ((is_local_this env id) && not (Emit_env.get_needs_local_this env)
              || SN.Superglobals.is_superglobal (Local_id.get_name id)) ->
     instr_cgetl2 (get_local env (pos, (Local_id.get_name id))), true
  | _ ->
     emit_expr env expr, false

(* Special case for binary operations to make use of CGetL2 *)
and emit_two_exprs env (outer_pos : Pos.t) (e1 : A.expr) (e2 : A.expr) =
  let instrs1, is_under_top = emit_first_expr env e1 in
  let instrs2 = emit_expr env e2 in
  let instrs2_is_var =
    match e2 with
    | _, A.Lvar _ -> true
    | _ -> false in
  gather @@
    if is_under_top
    then
      if instrs2_is_var
      then [emit_pos outer_pos; instrs2; instrs1]
      else [instrs2; emit_pos outer_pos; instrs1]
    else
      if instrs2_is_var
      then [instrs1; emit_pos outer_pos; instrs2]
      else [instrs1; instrs2; emit_pos outer_pos]

and emit_is_null env (e : A.expr) =
  match e with
  | (_, A.Lvar (pos, id)) when not (is_local_this env id) ->
     instr_istypel (get_local env (pos, (Local_id.get_name id))) OpNull
  | _ ->
    gather [
      emit_expr env e;
      instr_istypec OpNull
    ]

and emit_binop env annot op (e1: A.expr) (e2: A.expr) =
  let (pos, _) = annot in
  let default () = gather [
    emit_two_exprs env pos e1 e2;
    from_binop op
  ] in
  match op with
  | Ast.Ampamp | Ast.Barbar ->
    emit_short_circuit_op env annot (A.Binop (op, e1, e2))
  | Ast.Eq None ->
    emit_lval_op env pos LValOp.Set e1 (Some e2)
  | Ast.Eq (Some Ast.QuestionQuestion) ->
    emit_null_coalesce_assignment env pos e1 e2
  | Ast.Eq (Some obop) ->
    begin match binop_to_eqop obop with
    | None -> failwith "illegal eq op"
    | Some op -> emit_lval_op env pos (LValOp.SetOp op) e1 (Some e2)
    end
  | Ast.QuestionQuestion ->
    let end_label = Label.next_regular () in
    gather [
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
    if not (optimize_null_check ())
    then default ()
    else
    match op with
    | Ast.Eqeqeq when snd e2 = A.Null ->
      emit_is_null env e1
    | Ast.Eqeqeq when snd e1 = A.Null ->
      emit_is_null env e2
    | Ast.Diff2 when snd e2 = A.Null ->
      gather [
        emit_is_null env e1;
        instr_not
      ]
    | Ast.Diff2 when snd e1 = A.Null ->
      gather [
        emit_is_null env e2;
        instr_not
      ]
    | _ ->
      default ()

and emit_instanceof (env : Emit_env.t) pos (e: A.expr) (cid: A.class_id) =
  let lhs = emit_expr env e in
  let from_class_ref instrs =
    gather [
        lhs;
        emit_pos pos;
        instrs;
        instr_instanceof;
      ] in
  let scope = Emit_env.get_scope env in
  match class_id_to_class_expr ~resolve_self:true scope cid with
  | Class_special clsref ->
    let instr_clsref =
      match clsref with
      | SpecialClsRef.Self -> instr_self
      | SpecialClsRef.Static -> instr_lateboundcls
      | SpecialClsRef.Parent -> instr_parent in
     from_class_ref @@ gather [ instr_clsref; instr_clsrefname; ]
  | Class_id name ->
      let n =
        Hhbc_id.Class.elaborate_id (Emit_env.get_namespace env) name in
      gather [
        lhs;
        instr_instanceofd n;
        ]
  | Class_expr e2 ->
    gather [
      emit_expr env e;
      emit_expr env e2;
      instr_instanceof; ]
  | Class_reified _ ->
     failwith "cannot get this shape from from Aast.Id"

and get_type_structure_for_hint env ~targ_map (h : Aast.hint) =
  let namespace = Emit_env.get_namespace env in
  let tv = Emit_type_constant.hint_to_type_constant
    ~tparams:[] ~namespace ~targ_map h in
  let i = Emit_adata.get_array_identifier tv in
  if hack_arr_dv_arrs () then
    instr (ILitConst (Dict i)) else instr (ILitConst (Array i))

(* NOTE: Make sure the type structure retrieval code is synced with emit_is. *)
and emit_as env pos e h is_nullable = Local.scope @@ fun () ->
  let arg_local = Local.get_unnamed_local () in
  let type_struct_local = Local.get_unnamed_local () in
  let ts_instrs, is_static = emit_reified_arg env ~isas:true pos h in
  let then_label = Label.next_regular () in
  let done_label = Label.next_regular () in
  let push_typestructc_args_block = gather [
    instr_pushl arg_local;
    instr_pushl type_struct_local ] in
  (* Set aside the argument value. *)
  gather [
    emit_expr env e;
    instr_setl arg_local;
    (* Store type struct in a variable and reuse it. *)
    if is_static then
      gather [
        get_type_structure_for_hint env ~targ_map:SMap.empty h;
        instr_setl type_struct_local;
        instr_istypestructc Resolve;
        instr_jmpnz then_label;
        if is_nullable then instr_null
        else
          gather [push_typestructc_args_block; instr_astypestructc Resolve]
      ]
    else
      gather [
        ts_instrs;
        instr_setl type_struct_local;
        instr_istypestructc DontResolve;
        instr_jmpnz then_label;
        if is_nullable then instr_null
        else
          gather [push_typestructc_args_block; instr_astypestructc DontResolve]
      ];
    instr_jmp done_label;
    instr_label then_label;
    instr_pushl arg_local;
    instr_unsetl type_struct_local;
    instr_label done_label;
  ]

and emit_is env pos (h: Aast.hint) =
  let ts_instrs, is_static = emit_reified_arg env ~isas:true pos h in
  if is_static
  then
    match snd h with
    | Aast.Happly ((_, id), []) when id = SN.Typehints.this ->
      instr_islateboundcls
    | _ ->
      gather [
        get_type_structure_for_hint env ~targ_map:SMap.empty h;
        instr_istypestructc Resolve;
      ]
  else
    gather [
      ts_instrs;
      instr_istypestructc DontResolve;
    ]

and emit_cast env pos hint expr =
  let op =
    begin match hint with
    | Aast.Happly((_, id), []) ->
      let id = String.lowercase id in
      begin match id with
      | _ when id = SN.Typehints.int
            || id = SN.Typehints.integer -> instr (IOp CastInt)
      | _ when id = SN.Typehints.bool
            || id = SN.Typehints.boolean -> instr (IOp CastBool)
      | _ when id = SN.Typehints.string ||
               id = "binary" -> instr (IOp CastString)
      | _ when id = SN.Typehints.object_cast -> instr (IOp CastObject)
      | _ when id = SN.Typehints.array -> instr (IOp CastArray)
      | _ when id = SN.Typehints.double
            || id = SN.Typehints.float -> instr (IOp CastDouble)
      | _ when id = "unset" -> gather [ instr_popc; instr_null ]
      | _ -> Emit_fatal.raise_fatal_parse pos ("Invalid cast type: " ^ id)
      end
    | _ ->
      Emit_fatal.raise_fatal_parse pos "Invalid cast type"
    end in
  gather [
    emit_expr env expr;
    emit_pos pos;
    op;
  ]

and emit_conditional_expression env pos (etest : A.expr) (etrue : A.expr option) (efalse : A.expr) =
  match etrue with
  | Some etrue ->
    let false_label = Label.next_regular () in
    let end_label = Label.next_regular () in
    let r = emit_jmpz env etest false_label in
    gather [
      r.instrs;
      (* only emit true branch if there is fallthrough from condition *)
      begin if r.is_fallthrough
      then gather [
        emit_expr env etrue;
        emit_pos pos;
        instr_jmp end_label
      ]
      else empty
      end;
      (* only emit false branch if false_label is used *)
      begin if r.is_label_used
      then gather [
        instr_label false_label;
        emit_expr env efalse;
      ]
      else empty
      end;
      (* end_label is used to jump out of true branch so they should be emitted
         together *)
      begin if r.is_fallthrough
      then instr_label end_label
      else empty
      end;
    ]
  | None ->
    let end_label = Label.next_regular () in
    gather [
      emit_expr env etest;
      instr_dup;
      instr_jmpnz end_label;
      instr_popc;
      emit_expr env efalse;
      instr_label end_label;
    ]

and get_erased_tparams env =
  Ast_scope.Scope.get_tparams (Emit_env.get_scope env)
  |> List.filter_map ~f:(function { A.tp_name = (_, name); A.tp_reified; _ }
       -> Option.some_if (tp_reified = A.Erased) name)

and has_non_tparam_generics env (targs : Aast.hint list) =
  let erased_tparams = get_erased_tparams env in
  List.exists targs ~f:(function
    | _, Aast.Happly ((_, id), _)
      when List.mem ~equal:String.equal erased_tparams id -> false
    | _ -> true)

and emit_reified_targs env pos targs =
  let len = List.length targs in
  let scope = Emit_env.get_scope env in
  let current_fun_tparams = Ast_scope.Scope.get_fun_tparams scope in
  let current_cls_tparam = Ast_scope.Scope.get_class_tparams scope in
  let is_in_lambda = Ast_scope.Scope.is_in_lambda (Emit_env.get_scope env) in
  let is_soft { A.tp_user_attributes = ua; _} =
    List.exists ua ~f:(function { A.ua_name = n; _ } ->
      snd n = SN.UserAttributes.uaSoft) in
  let is_same tparam =
    List.length tparam = len &&
    List.for_all2_exn tparam targs
      ~f:(fun tp ta -> match tp, ta with
          | { A.tp_name = (_, name1); _}, (_, A.Happly ((_, name2), [])) ->
              name1 = name2 && not (is_soft tp)
          | _, _ -> false)
  in
  if not is_in_lambda && is_same current_fun_tparams then
    instr_cgetl (Local.Named SU.Reified.reified_generics_local_name)
  else if not is_in_lambda && is_same (current_cls_tparam.A.c_tparam_list) then
    gather [
      instr_checkthis;
      instr_baseh;
      instr_querym 0 QueryOp.CGet (MemberKey.PT (
        Hhbc_id.Prop.from_raw_string SU.Reified.reified_prop_name));
    ]
  (* TODO(T31677864): If the full generic array is static and does not require
   * resolution, emit it as static array *)
  else begin
    gather [
      gather @@
        List.map targs
          ~f:(fun h -> fst @@ emit_reified_arg env ~isas:false pos h);
      instr_lit_const
        (if hack_arr_dv_arrs () then NewVecArray len else NewVArray len);
    ]
  end

and emit_new env pos (cid : A.class_id) (targs : Aast.targ list) (args : A.expr list) (uargs : A.expr list) =
  if has_inout_args args then
    Emit_fatal.raise_fatal_parse pos "Unexpected inout arg in new expr";
  let scope = Emit_env.get_scope env in
  (* If `new self` or `new parent `when self or parent respectively has
   * reified generics, do not resolve *)
  let resolve_self = match cid with
    | _, A.CIexpr (_, A.Id (_, n)) when SU.is_self n ->
      (Ast_scope.Scope.get_class_tparams scope).A.c_tparam_list
      |> List.for_all ~f:(fun t -> t.A.tp_reified = A.Erased)
    | _, A.CIexpr (_, A.Id (_, n)) when SU.is_parent n ->
       let cls = Ast_scope.Scope.get_class scope in
      Option.value_map cls ~default:true ~f:(fun cls ->
      match cls.A.c_extends with
      | (_, Aast.Happly (_, l)) :: _ ->
        not @@ has_non_tparam_generics env l
      | _ -> true)
    | _ -> true in
  let cexpr = class_id_to_class_expr ~resolve_self scope cid in
  let cexpr, has_generics = match cexpr with
    | Class_id (_, name) ->
      let cexpr =
        Option.value ~default:cexpr (get_reified_var_cexpr env pos name) in
      begin match emit_reified_type_opt env pos name with
      | Some instrs ->
        if not @@ List.is_empty targs then Emit_fatal.raise_fatal_parse pos
          "Cannot have higher kinded reified generics";
        Class_reified instrs, H.MaybeGenerics
      | None when not (has_non_tparam_generics env targs) ->
        cexpr, H.NoGenerics
      | None ->
        let instrs = match cexpr with
            | Class_id id ->
              let fq_id =
                Hhbc_id.Class.elaborate_id (Emit_env.get_namespace env) id
                |> Hhbc_id.Class.to_raw_string in
              gather [
                emit_reified_targs env pos targs;
                instr_reified_name fq_id;
              ]
            | Class_reified instrs -> instrs
            | _ -> failwith "Internal error: This node can only be id or reified"
        in
        Class_reified instrs, H.HasGenerics
      end
    | _ -> cexpr, H.NoGenerics in
  let newobj_instrs = match cexpr with
    (* Special case for statically-known class *)
  | Class_id id ->
    let fq_id =
      Hhbc_id.Class.elaborate_id (Emit_env.get_namespace env) id in
    Emit_symbol_refs.add_class (Hhbc_id.Class.to_raw_string fq_id);
    gather [ emit_pos pos; instr_newobjd fq_id ]
  | Class_special cls_ref -> gather [ emit_pos pos; instr_newobjs cls_ref ]
  | Class_reified instrs when has_generics = H.MaybeGenerics ->
    gather [ instrs; instr_clsrefgetts; instr_newobj 0 has_generics ]
  | _ ->
    gather [ emit_load_class_ref env pos cexpr; instr_newobj 0 has_generics ]
  in
  Scope.with_unnamed_locals @@ fun () ->
  let instr_args, _ = emit_args_and_inout_setters env args in
  let instr_uargs = match uargs with
    | [] -> empty
    | uargs :: _ -> emit_expr env uargs
  in
  empty,
  gather [
    newobj_instrs;
    instr_dup; instr_nulluninit; instr_nulluninit;
    instr_args;
    instr_uargs;
    emit_pos pos;
    instr_fcallctor (get_fcall_args args uargs None);
    instr_popc
  ],
  empty

(* TODO(T36697624) more efficient bytecode for static records *)
and emit_record env pos cid es =
  let cexpr = class_id_to_class_expr ~resolve_self:false
    (Emit_env.get_scope env) cid
  in
  match cexpr with
  | Class_id id ->
    let fq_id =
      Hhbc_id.Class.elaborate_id (Emit_env.get_namespace env) id in
    Emit_symbol_refs.add_class (Hhbc_id.Class.to_raw_string fq_id);
    emit_struct_array env pos es (instr_new_record fq_id)
  | _ ->
    failwith "No record with specified name found"

and emit_clone env expr =
  gather [
    emit_expr env expr;
    instr_clone;
  ]

and emit_shape env (expr : A.expr) (fl : (Ast.shape_field_name * A.expr) list) =
  let p = fst expr in
  let fl =
    List.map fl
             ~f:(fun (fn, e) ->
                   ((p, extract_shape_field_name_pstring env p fn), e))
  in
  emit_expr env (p, A.Darray (None, fl))

and emit_call_expr env pos e targs args uargs async_eager_label =
  match snd e, targs, args, uargs with
  | A.Id (_, "__hhas_adata"), _, [ (_, A.String data) ], [] ->
    let v = Typed_value.HhasAdata data in
    emit_pos_then pos @@ instr (ILitConst (TypedValue v))
  | A.Id (_, id), _, _, []
    when String.lowercase id = "isset" ->
    emit_call_isset_exprs env pos args
  | A.Id (_, id), _, [arg1], []
    when String.lowercase id = "empty" ->
    emit_call_empty_expr env pos arg1
  | A.Id (_, id), _, ([_; _] | [_; _; _]), []
    when String.lowercase id = "idx" && not (jit_enable_rename_function ()) ->
    emit_idx env pos args

  | A.Id (_, id), _, [arg1], []
    when String.lowercase id = "eval" ->
    emit_eval env pos arg1
  | A.Id (_, "class_alias"), _, [_, A.String c1; _, A.String c2], []
    when is_global_namespace env -> gather [
      emit_pos pos;
      instr_true;
      instr_alias_cls c1 c2
    ]
  | A.Id (_, "class_alias"), _, [_, A.String c1; _, A.String c2; arg3], []
    when is_global_namespace env -> gather [
      emit_expr env arg3;
      emit_pos pos;
      instr_alias_cls c1 c2
    ]
  | A.Id (_, id), _, [arg1], []
    when String.lowercase id = "hh\\set_frame_metadata" ||
         String.lowercase id = "\\hh\\set_frame_metadata" ->
    gather [
      emit_expr env arg1;
      emit_pos pos;
      instr_popl (Local.Named "$86metadata");
      instr_null;
    ]
  | A.Id (_, s), _, [], []
    when (String.lowercase s = "exit" || String.lowercase s = "die") ->
    emit_pos_then pos @@ emit_exit env None
  | A.Id (_, s), _, [arg1], []
    when (String.lowercase s = "exit" || String.lowercase s = "die") ->
    emit_pos_then pos @@ emit_exit env (Some arg1)
  | _, _, _, _ ->
    let instrs = emit_call env pos e targs args uargs async_eager_label in
    emit_pos_then pos instrs

and emit_known_class_id (env : Emit_env.t) id =
  let fq_id = Hhbc_id.Class.elaborate_id (Emit_env.get_namespace env) id in
  Emit_symbol_refs.add_class (Hhbc_id.Class.to_raw_string fq_id);
  gather [
    instr_string (Hhbc_id.Class.to_raw_string fq_id);
    instr_clsrefgetc;
  ]

and emit_load_class_ref env pos cexpr =
  emit_pos_then pos @@
  match cexpr with
  | Class_special SpecialClsRef.Self -> instr_self
  | Class_special SpecialClsRef.Static -> instr_lateboundcls
  | Class_special SpecialClsRef.Parent -> instr_parent
  | Class_id id -> emit_known_class_id env id
  | Class_expr expr ->
    gather [
      emit_pos pos;
      emit_expr env expr;
      instr_clsrefgetc
    ]
  | Class_reified instrs ->
    gather [
      emit_pos pos;
      instrs;
      instr_clsrefgetc
    ]

and emit_load_class_const env pos (cexpr : Ast_class_expr.class_expr) id =
  let load_const =
    if SU.is_class id
    then instr (IMisc (ClsRefName 0))
    else instr (ILitConst (ClsCns (Hhbc_id.Const.from_ast_name id, 0)))
  in
  gather [
    emit_load_class_ref env pos cexpr;
    load_const;
  ]

and emit_class_expr env (cexpr : Ast_class_expr.class_expr) (prop : A.class_get_expr) =
  let load_prop () =
    match prop with
    | A.CGstring (pos, id) ->
      emit_pos_then pos @@
      instr_string (SU.Locals.strip_dollar id)
    | A.CGexpr e ->
      emit_expr env e
  in
  let aux e =
    let cexpr_local = emit_expr env e in
    empty,
    gather [
      cexpr_local;
      Scope.stash_top_in_unnamed_local load_prop;
      instr_clsrefgetc;
    ] in
  match cexpr with
  | Class_expr ((_, (A.BracedExpr _ |
                      A.Call _ |
                      A.Binop _ |
                      A.Class_get _)) as e) -> aux e
  | Class_expr (_, A.Lvar (_, id) as e)
    when (Local_id.get_name id = "$this") -> aux e
  | _ ->
    begin
      let pos =
        match prop with
        | A.CGstring (pos, _) -> pos
        | A.CGexpr ((pos, _), _) -> pos in
      load_prop (), emit_load_class_ref env pos cexpr
    end

and emit_class_get env qop (cid : A.class_id) (prop : A.class_get_expr) =
  let cexpr = class_id_to_class_expr ~resolve_self:false
    (Emit_env.get_scope env) cid
  in
  gather [
    of_pair @@ emit_class_expr env cexpr prop;
    match qop with
    | QueryOp.CGet -> instr_cgets
    | QueryOp.CGetQuiet -> failwith "emit_class_get: CGetQuiet"
    | QueryOp.Isset -> instr_issets
    | QueryOp.Empty -> instr_emptys
    | QueryOp.InOut -> failwith "emit_class_get: InOut"
  ]

(* Class constant <cid>::<id>.
 * We follow the logic for the Construct::KindOfClassConstantExpression
 * case in emitter.cpp
 *)
and emit_class_const env pos (cid: A.class_id) (_, id): Instruction_sequence.t =
  let cexpr = class_id_to_class_expr ~resolve_self:true
    (Emit_env.get_scope env) cid in
  let cexpr = match cexpr with
    | Class_id (_, name) ->
      Option.value ~default:cexpr (get_reified_var_cexpr env pos name)
    | _ -> cexpr in
  match cexpr with
  | Class_id cid ->
    emit_class_const_impl env cid id
  | _ ->
    emit_load_class_const env pos cexpr id

and emit_class_const_impl (env : Emit_env.t) cid id =
  let fq_id =
    Hhbc_id.Class.elaborate_id (Emit_env.get_namespace env) cid in
  let fq_id_str = Hhbc_id.Class.to_raw_string fq_id in
  emit_pos_then (fst cid) @@
  if SU.is_class id
  then instr_string fq_id_str
  else
    begin
      Emit_symbol_refs.add_class fq_id_str;
      instr (ILitConst (ClsCnsD (Hhbc_id.Const.from_ast_name id, fq_id)))
    end

and emit_yield env pos = function
  | A.AFvalue e ->
    gather [
      emit_expr env e;
      emit_pos pos;
      instr_yield;
    ]
  | A.AFkvalue (e1, e2) ->
    gather [
      emit_expr env e1;
      emit_expr env e2;
      emit_pos pos;
      instr_yieldk;
    ]

and emit_string2 env pos (exprs : A.expr list) =
  match exprs with
  | [e] ->
    gather [
      emit_expr env e;
      emit_pos pos;
      instr (IOp CastString)
    ]
  | e1::e2::es ->
    gather @@ [
      emit_two_exprs env (fst (fst e1)) e1 e2;
      emit_pos pos;
      instr (IOp Concat);
      gather (List.map es (fun e ->
        gather [emit_expr env e;
          emit_pos pos; instr (IOp Concat)]))
    ]

  | [] -> failwith "String2 with zero arguments is impossible"

and emit_lambda (env : Emit_env.t) (fundef : A.fun_) (ids : Aast.lid list) =
  (* Closure conversion puts the class number used for CreateCl in the "name"
   * of the function definition *)
  let fundef_name = snd fundef.A.f_name in
  let class_num = int_of_string fundef_name in
  let explicit_use = SSet.mem fundef_name (Emit_env.get_explicit_use_set ()) in
  let is_in_lambda = Ast_scope.Scope.is_in_lambda (Emit_env.get_scope env) in
  gather [
    gather @@ List.map ids
      (fun (pos, id) ->
        match SU.Reified.is_captured_generic @@ (Local_id.get_name id) with
        | Some (is_fun, i) ->
          if is_in_lambda then
            instr_cgetl (Local.Named (
              SU.Reified.reified_generic_captured_name is_fun i))
          else emit_reified_generic_instrs Pos.none ~is_fun i
        | None ->
          let lid = get_local env (pos, (Local_id.get_name id)) in
          if explicit_use then instr_cgetl lid else instr_cugetl lid
      );
    instr (IMisc (CreateCl (List.length ids, class_num)))
  ]

and emit_id (env : Emit_env.t) (id: Aast.sid) =
  let (p, s) = id in
  let s = String.uppercase s in
  match s with
  | "__FILE__" -> instr (ILitConst File)
  | "__DIR__" -> instr (ILitConst Dir)
  | "__CLASS__" -> gather [instr_self; instr_clsrefname]
  | "__METHOD__" -> instr (ILitConst Method)
  | "__LINE__" ->
    (* If the expression goes on multi lines, we return the last line *)
    let _, line, _, _ = Pos.info_pos_extended p in
    instr_int line
  | "__NAMESPACE__" ->
    let ns = Emit_env.get_namespace env in
    instr_string (Option.value ~default:"" ns.Namespace_env.ns_name)
  | "__COMPILER_FRONTEND__" -> instr_string "hackc"
  | ("EXIT" | "DIE") ->
    emit_exit env None
  | _ ->
    let fq_id = Hhbc_id.Const.elaborate_id (Emit_env.get_namespace env) id in
    Emit_symbol_refs.add_constant (snd id);
    emit_pos_then p @@ instr (ILitConst (CnsE fq_id))

and rename_xhp (p, s) = (p, SU.Xhp.mangle s)

and emit_xhp (env : Emit_env.t) (annot : A.expr_annotation) id (attributes : A.xhp_attribute list) (children : A.expr list) =
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
  let create_spread p id = (p, "...$" ^ string_of_int(id)) in
  let convert_attr (spread_id, attrs) = function
    | A.Xhp_simple (name, v) ->
        let attr = (Ast.SFlit_str name, v) in
        (spread_id, attr::attrs)
    | A.Xhp_spread e ->
        let ((p, _), _) = e in
        let attr = (Ast.SFlit_str (create_spread p spread_id), e) in
        (spread_id + 1, attr::attrs) in
  let (_, attributes) = List.fold_left ~f:convert_attr ~init:(0, []) attributes in
  let attribute_map = annot, A.Shape (List.rev attributes) in
  let children_vec = annot, A.Varray (None, children) in
  let filename = annot, A.Id (pos, "__FILE__") in
  let line = annot, A.Id (pos, "__LINE__") in
  let renamed_id = rename_xhp id in
  Emit_symbol_refs.add_class (snd renamed_id);
  emit_expr env @@
    (annot, A.New (
      (annot, A.CI renamed_id),
      [],
      [attribute_map ; children_vec ; filename ; line],
      [],
      annot))

and emit_import env annot (flavor : Aast.import_flavor) (e : A.expr) =
  let (pos, _) = annot in
  let inc = parse_include e in
  Emit_symbol_refs.add_include inc;
  let e, import_op = match flavor with
    | Aast.Include -> e, IIncludeEvalDefine Incl
    | Aast.Require -> e, IIncludeEvalDefine Req
    | Aast.IncludeOnce -> e, IIncludeEvalDefine InclOnce
    | Aast.RequireOnce ->
      let include_roots = Hhbc_options.include_roots !Hhbc_options.compiler_options in
      match Hhas_symbol_refs.resolve_to_doc_root_relative inc ~include_roots with
        | Hhas_symbol_refs.DocRootRelative path ->
          (annot, A.String path), IIncludeEvalDefine ReqDoc
        | _ -> e, IIncludeEvalDefine ReqOnce
  in
  gather [
    emit_expr env e;
    emit_pos pos;
    instr import_op;
  ]

and emit_call_isset_expr env outer_pos (expr : A.expr) =
  let ((pos, _), expr_) = expr in
  match expr_ with
  | A.Array_get ((_, A.Lvar (_, x)), Some e) when (Local_id.get_name x) = SN.Superglobals.globals ->
    gather [
      emit_expr env e;
      emit_pos outer_pos;
      instr_issetg
    ]
  | A.Array_get (base_expr, opt_elem_expr) ->
    fst (emit_array_get env pos QueryOp.Isset base_expr opt_elem_expr)
  | A.Class_get (cid, id)  ->
    emit_class_get env QueryOp.Isset cid id
  | A.Obj_get (expr, prop, nullflavor) ->
    fst (emit_obj_get env pos QueryOp.Isset expr prop nullflavor)
  | A.Lvar (_, n) when SN.Superglobals.is_superglobal (Local_id.get_name n) ->
    gather [
      emit_pos outer_pos;
      instr_string @@ SU.Locals.strip_dollar (Local_id.get_name n);
      emit_pos outer_pos;
      instr_issetg
    ]
  | A.Lvar ((_, name) as id)
    when is_local_this env name && not (Emit_env.get_needs_local_this env) ->
    gather [
      emit_pos outer_pos;
      emit_local ~notice:NoNotice ~need_ref:false env id;
      emit_pos outer_pos;
      instr_istypec OpNull;
      instr_not
    ]
  | A.Lvar (pos, id) ->
    emit_pos_then outer_pos @@
    instr (IIsset (IssetL (get_local env (pos, (Local_id.get_name id)))))
  | _ ->
    gather [
      emit_expr env expr;
      instr_istypec OpNull;
      instr_not
    ]

and emit_call_empty_expr env outer_pos (annot, expr_ as expr) =
  let (pos, _) = annot in
  match expr_ with
  | A.Array_get((_, A.Lvar (_, x)), Some e) when (Local_id.get_name x) = SN.Superglobals.globals ->
    gather [
      emit_expr env e;
      emit_pos outer_pos;
      instr_emptyg
    ]
  | A.Array_get(base_expr, opt_elem_expr) ->
    fst (emit_array_get env pos QueryOp.Empty base_expr opt_elem_expr)
  | A.Class_get (cid, id) ->
    emit_class_get env QueryOp.Empty cid id
  | A.Obj_get (expr, prop, nullflavor) ->
    fst (emit_obj_get env pos QueryOp.Empty expr prop nullflavor)
  | A.Lvar(_, id) when SN.Superglobals.is_superglobal (Local_id.get_name id) ->
    gather [
      instr_string @@ SU.Locals.strip_dollar (Local_id.get_name id);
      emit_pos outer_pos;
      instr_emptyg
    ]
  | A.Lvar (pos, id) ->
     if not (is_local_this env id) ||
      Emit_env.get_needs_local_this env
    then
      emit_pos_then outer_pos @@
      instr_emptyl (get_local env (pos, (Local_id.get_name id)))
    else
      gather [
        emit_pos pos;
        instr (IMisc (BareThis NoNotice));
        emit_pos outer_pos;
        instr_not
      ]
  | _ ->
    gather [
      emit_expr env expr;
      instr_not
    ]

and emit_unset_expr env expr =
  emit_lval_op_nonlist env (fst (fst expr)) LValOp.Unset expr empty 0

and emit_set_range_expr env pos name kind args =
  let raise_fatal msg =
    Emit_fatal.raise_fatal_parse pos (Printf.sprintf "%s %s" name msg) in
  let range_op, size, allow_count = kind in
  let base, offset, src, args =
    match args with
    | b::o::s::rest -> (b, o, s, rest)
    | _ -> raise_fatal "expects at least 3 arguments" in
  let count_instrs =
    match args, allow_count with
    | [c], true -> emit_expr env c
    | [], _ -> instr_int (-1)
    | _, false -> raise_fatal "expects no more than 3 arguments"
    | _, true -> raise_fatal "expects no more than 4 arguments" in
  let base_expr_begin, base_expr_end, base_setup, base_stack =
    emit_base ~notice:Notice ~is_object:false
      env MemberOpMode.Define 3 base in
  gather [
    base_expr_begin;
    base_expr_end;
    emit_expr env offset;
    emit_expr env src;
    count_instrs;
    base_setup;
    instr (IFinal (SetRangeM (base_stack, range_op, size)))
  ]

and emit_call_isset_exprs env pos (exprs : A.expr list) =
  match exprs with
  | [] -> Emit_fatal.raise_fatal_parse
    pos "Cannot use isset() without any arguments"
  | [expr] -> emit_call_isset_expr env pos expr
  | _ ->
    let n = List.length exprs in
    let its_done = Label.next_regular () in
      gather [
        gather @@
        List.mapi exprs
        begin fun i expr ->
          gather [
            emit_call_isset_expr env pos expr;
            if i < n-1 then
            gather [
              instr_dup;
              instr_jmpz its_done;
              instr_popc
            ] else empty
          ]
        end;
        instr_label its_done
      ]

and emit_exit env (expr_opt : A.expr option) =
  gather [
    (match expr_opt with
      | None -> instr_int 0
      | Some e -> emit_expr env e);
    instr_exit;
  ]

and emit_idx env pos (es : A.expr list) =
  let default = if List.length es = 2 then instr_null else empty in
  gather [
    emit_exprs env es;
    emit_pos pos;
    default;
    instr_idx;
  ]

and emit_eval env pos e =
  gather [
    emit_expr env e;
    emit_pos pos;
    instr_eval;
  ]

and emit_xhp_obj_get env pos (_, ty) (e : A.expr) s nullflavor =
  let annot = (pos, ty) in
  let fn_name = annot, A.Obj_get (e, (annot, A.Id (pos, "getAttribute")), nullflavor) in
  let args = [annot, A.String (SU.Xhp.clean s)] in
  emit_call env pos fn_name [] args [] None

and try_inline_gen_call env (e : A.expr) =
  if not (can_inline_gen_functions ()) then None
  else match snd e with
  | A.Call (_, (_, A.Id (_, s)), _, [arg], [])
    when String.lowercase (SU.strip_global_ns s) = "gena"->
    Some (inline_gena_call env arg)
  | _ ->
    try_inline_genva_call env e GI_expression

and try_inline_genva_call env (e : A.expr) (inline_context : genva_inline_context) =
  if not (can_inline_gen_functions ()) then None
  else match e with
  | pos, A.Call (_, (_, A.Id (_, s)), _, args, uargs)
    when String.lowercase (SU.strip_global_ns s) = "genva"->
    try_inline_genva_call_ env pos args uargs inline_context
  | _ -> None

(* emits iteration over the ~collection where loop body is
   produced by ~f *)
and emit_iter ~collection f =
  Scope.with_unnamed_locals_and_iterators @@ fun () ->
  let iter = Iterator.get_iterator () in
  let value_local = Local.get_unnamed_local () in
  let key_local = Local.get_unnamed_local () in
  let loop_end = Label.next_regular () in
  let loop_next = Label.next_regular () in
  let iter_init = gather [
    collection;
    instr_iterinitk iter loop_end value_local key_local
  ] in
  let iterate = gather [
    instr_label loop_next;
    f value_local key_local;
    instr_iternextk iter loop_next value_local key_local
  ] in
  let iter_done = gather [
    instr_unsetl value_local;
    instr_unsetl key_local;
    instr_label loop_end
  ] in
  iter_init, iterate, iter_done

and inline_gena_call env (arg : A.expr) = Local.scope @@ fun () ->
  (* convert input to array *)
  let load_array = emit_expr env arg in
  Scope.with_unnamed_local @@ fun arr_local ->
  let async_eager_label = Label.next_regular () in
  (* before *)
  gather [
    load_array;
    if hack_arr_dv_arrs () then instr_cast_dict else instr_cast_darray;
    instr_popl arr_local
  ],
  (* inner *)
  gather [
    instr_nulluninit; instr_nulluninit; instr_nulluninit;
    instr_cgetl arr_local;
    instr_fcallclsmethodd
      (make_fcall_args ~async_eager_label 1)
      (Hhbc_id.Method.from_raw_string
         (if hack_arr_dv_arrs () then "fromDict" else "fromDArray"))
      (Hhbc_id.Class.from_raw_string "HH\\AwaitAllWaitHandle");
    instr_await;
    instr_label async_eager_label;
    instr_popc;
    emit_iter ~collection:(instr_cgetl arr_local) @@
    begin fun value_local key_local ->
      gather [
        (* generate code for
           arr_local[key_local] = WHResult (value_local) *)
        instr_cgetl value_local;
        instr_whresult;
        instr_basel arr_local MemberOpMode.Define;
        instr_setm 0 (MemberKey.EL key_local);
        instr_popc;
      ]
    end
  ],
  (* after *)
  instr_pushl arr_local

and try_inline_genva_call_ env annot (args : A.expr list) (uargs : A.expr list) (inline_context : genva_inline_context) =
  let (pos, _) = annot in
  let args_count = List.length args in
  let is_valid_list_assignment l =
    List.findi l ~f:(fun i (_, x) -> i >= args_count && x <> A.Omitted)
    |> Option.is_none in
  let emit_list_assignment (lhs : A.expr list) rhs =
    let rec combine (lhs : A.expr list) rhs =
      (* ensure that list of values on left hand side and right hand size
         has the same length *)
      match lhs, rhs with
      | l :: lhs, r :: rhs -> (l, r) :: combine lhs rhs
      (* left hand size is smaller - pad with omitted expression *)
      | [], r :: rhs -> ((Tast_annotate.null_annotation Pos.none, A.Omitted), r) :: combine [] rhs
      | _, [] -> [] in
    let generate values ~is_ltr =
      let rec aux lhs_acc set_acc = function
      | [] -> (if is_ltr then List.rev lhs_acc else lhs_acc), List.rev set_acc
      | ((_, A.Omitted), _) :: tail -> aux lhs_acc set_acc tail
      | (lhs, rhs) :: tail ->
        let lhs_instrs, set_instrs =
          emit_lval_op_list ~last_usage:true env pos (Some rhs) [] lhs in
        aux (lhs_instrs::lhs_acc) (set_instrs::set_acc) tail in
      aux [] [] (if is_ltr then values else List.rev values) in
    let reify = gather @@ List.map rhs ~f:begin fun l ->
      let label_done = Label.next_regular () in
      gather [
        instr_istypel l OpNull;
        instr_jmpnz label_done;
        instr_pushl l;
        instr_whresult;
        instr_popl l;
        instr_label label_done;
      ]
    end in
    let pairs = combine lhs rhs in
    let lhs, set = generate pairs ~is_ltr:(php7_ltr_assign ()) in
    gather [
      reify;
      gather lhs;
      gather set;
      gather @@ List.map pairs
        ~f:(function (_, A.Omitted), l -> instr_unsetl l | _ -> empty);
    ] in
  match inline_context with
  | GI_list_assignment l when not (is_valid_list_assignment l) ->
    None
  | _ when not (List.is_empty uargs) ->
    Emit_fatal.raise_fatal_runtime pos "do not use ...$args with genva()"
  | GI_ignore_result | GI_list_assignment _ when args_count = 0 ->
    Some empty
  | GI_expression when args_count = 0 ->
    Some instr_lit_empty_varray
  | _ when args_count > max_array_elem_on_stack () ->
    None
  | _ ->
  let load_args =
    gather @@ List.map args ~f:begin fun arg ->
      emit_expr env arg
    end in
  let some result = Some result in
  some @@ Scope.with_unnamed_locals @@ fun () ->
  let reserved_locals =
    List.init args_count (fun _ -> Local.get_unnamed_local ()) in
  let reserved_locals_reversed =
    List.rev reserved_locals in
  let init_locals =
    gather @@ List.map reserved_locals_reversed ~f:instr_popl in
  let await_all =
    gather [
      instr_awaitall_list reserved_locals_reversed;
      instr_popc;
    ] in
  let process_results =
    let reify ~pop_result =
      gather @@ List.map reserved_locals ~f:begin fun l ->
        let label_done = Label.next_regular() in
        gather [
          instr_pushl l;
          instr_dup;
          instr_istypec OpNull;
          instr_jmpnz label_done;
          instr_whresult;
          instr_label label_done;
          if pop_result then instr_popc else empty;
        ]
      end in
    match inline_context with
    | GI_ignore_result ->
      reify ~pop_result:true
    | GI_expression ->
      gather [
        reify ~pop_result:false;
        instr_lit_const (if hack_arr_dv_arrs ()
                         then (NewVecArray args_count)
                         else (NewVArray args_count));
      ]
    | GI_list_assignment l ->
      emit_list_assignment l reserved_locals in
  gather [ load_args; init_locals ],     (* before *)
  gather [ await_all; process_results ], (* inner *)
  empty                                  (* after *)

and emit_await env pos (expr : A.expr) =
  begin match try_inline_gen_call env expr with
  | Some r -> r
  | None ->
    let after_await = Label.next_regular () in
    let instrs = match snd expr with
    | A.Call (_, e, targs, args, uargs) ->
      emit_call_expr env pos e targs args uargs (Some after_await)
    | _ ->
      emit_expr env expr
    in gather [
      instrs;
      emit_pos pos;
      instr_dup;
      instr_istypec OpNull;
      instr_jmpnz after_await;
      instr_await;
      instr_label after_await;
    ]
  end

and emit_callconv _env kind _e =
  match kind with
  | Ast.Pinout ->
    failwith "emit_callconv: This should have been caught at emit_arg"

and get_reified_var_cexpr env pos name: Ast_class_expr.class_expr option =
  match emit_reified_type_opt env pos name with
  | None -> None
  | Some instrs -> Some (Class_reified (
    gather [
      instrs;
      instr_basec 0 MemberOpMode.Warn;
      instr_querym 1 QueryOp.CGet (MemberKey.ET "classname");
    ]))

and emit_reified_generic_instrs pos ~is_fun index =
  let base = if is_fun then
      instr_basel
        (Local.Named SU.Reified.reified_generics_local_name)
        MemberOpMode.Warn
    else
      gather [
        instr_checkthis;
        instr_baseh;
        instr_dim_warn_pt @@
          Hhbc_id.Prop.from_raw_string SU.Reified.reified_prop_name;
      ]
  in
  emit_pos_then pos @@ gather [
    base;
    instr_querym 0 QueryOp.CGet (MemberKey.EI (Int64.of_int index))
  ]

and emit_reified_type_opt (env : Emit_env.t) pos name =
  let is_in_lambda = Ast_scope.Scope.is_in_lambda (Emit_env.get_scope env) in
  let cget_instr is_fun i =
    instr_cgetl (Local.Named (SU.Reified.reified_generic_captured_name is_fun i))
  in
  let check is_soft = if not is_soft then () else
    Emit_fatal.raise_fatal_parse pos
      (name
       ^ " is annotated to be a soft reified generic,"
       ^ " it cannot be used until the __Soft annotation is removed")
  in
  let rec aux ~is_fun =
    match is_reified_tparam ~is_fun env name with
    | Some (i, is_soft) ->
      check is_soft;
      Some (if is_in_lambda then cget_instr is_fun i
            else emit_reified_generic_instrs pos ~is_fun i)
    | None -> if is_fun then aux ~is_fun:false else None
  in
  aux ~is_fun:true

and emit_reified_type env pos name =
  match emit_reified_type_opt env pos name with
  | Some instrs -> instrs
  | None -> Emit_fatal.raise_fatal_runtime Pos.none "Invalid reified param"

and emit_expr (env : Emit_env.t) (expr: A.expr) =
  let ((pos, _) as annot, expr_) = expr in
  match expr_ with
  | A.Float _ | A.String _ | A.Int _ | A.Null | A.False | A.True ->
    let v = Ast_constant_folder.expr_to_typed_value (Emit_env.get_namespace env) expr in
    emit_pos_then pos @@
    instr (ILitConst (TypedValue v))
  | A.PrefixedString (_, e)
  | A.ParenthesizedExpr e ->
    emit_expr env e
  | A.Lvar (pos, _ as lid) ->
    gather [
      emit_pos pos;
      emit_local ~notice:Notice ~need_ref:false env lid
    ]
  | A.Class_const (cid, id) ->
    emit_class_const env pos cid id
  | A.Unop (op, e) ->
    emit_unop env pos op e
  | A.Binop (op, e1, e2) ->
    emit_binop env annot op e1 e2
  | A.Pipe (_, e1, e2) ->
    emit_pipe env e1 e2
  | A.InstanceOf (e1, e2) ->
    emit_instanceof env pos e1 e2
  | A.Is (e, h) ->
    gather [
      emit_expr env e;
      emit_is env pos h;
    ]
  | A.As (e, h, is_nullable) ->
    emit_as env pos e h is_nullable
  | A.Cast((_, hint), e) ->
    emit_cast env pos hint e
  | A.Eif (etest, etrue, efalse) ->
    emit_conditional_expression env pos etest etrue efalse
  | A.Expr_list es -> gather @@ List.map es ~f:(emit_expr env)
  | A.Array_get ((_, A.Lvar (_, x)), Some e) when (Local_id.get_name x) = SN.Superglobals.globals ->
    gather [
      emit_expr  env e;
      emit_pos pos;
      instr (IGet CGetG)
    ]
  | A.Array_get (base_expr, opt_elem_expr) ->
    fst (emit_array_get env pos QueryOp.CGet base_expr opt_elem_expr)
  | A.Obj_get (expr, prop, nullflavor) ->
    fst (emit_obj_get env pos QueryOp.CGet expr prop nullflavor)
  | A.Call (_, e, targs, args, uargs) ->
    emit_call_expr env pos e targs args uargs None
  | A.New (cid, targs, args, uargs, _constructor_annot) ->
    emit_new env pos cid targs args uargs
  | A.Record (cid, es) ->
    let es2 = List.map ~f:(fun (e1, e2) -> A.AFkvalue (e1, e2)) es in
    emit_record env pos cid es2
  | A.Array es ->
    emit_pos_then pos @@ emit_collection env expr es
  | A.Darray (ta, es) ->
    emit_pos_then pos @@
    let es2 = List.map ~f:(fun (e1, e2) -> A.AFkvalue (e1, e2)) es in
    let darray_e = fst expr, A.Darray (ta, es) in
    emit_collection env darray_e es2
  | A.Varray (ta, es) ->
    emit_pos_then pos @@
    let es2 = List.map ~f:(fun e -> A.AFvalue e) es in
    let varray_e = fst expr, A.Varray (ta, es) in
    emit_collection env varray_e es2
  | A.Collection ((pos, name), _, fields) ->
    emit_named_collection env expr pos name fields
  | A.ValCollection (`Vector as name, _, el)
  | A.ValCollection (`ImmVector as name, _, el)
  | A.ValCollection (`Set as name, _, el)
  | A.ValCollection (`ImmSet as name, _, el) ->
    let name = match name with
      | `Vector -> "Vector"
      | `ImmVector -> "ImmVector"
      | `Set -> "Set"
      | `ImmSet -> "ImmSet" in
    let fields = List.map el ~f:(fun e -> A.AFvalue e) in
    emit_named_collection env expr pos name fields
  | A.ValCollection (_, _, el) ->
    let fields = List.map el ~f:(fun e -> A.AFvalue e) in
    emit_collection env expr fields
  | A.Pair (e1, e2) ->
    let fields = [A.AFvalue e1; A.AFvalue e2] in
    emit_named_collection env expr pos "Pair" fields
  | A.KeyValCollection (`Map as name, _, fields)
  | A.KeyValCollection (`ImmMap as name, _, fields) ->
    let fields = List.map fields ~f:(fun (e1, e2) -> A.AFkvalue (e1, e2)) in
    let name = match name with
      | `Map -> "Map"
      | `ImmMap -> "ImmMap" in
    emit_named_collection env expr pos name fields
  | A.KeyValCollection (_, _, fields) ->
    let fields = List.map fields ~f:(fun (e1, e2) -> A.AFkvalue (e1, e2)) in
    emit_collection env expr fields
  | A.Clone e ->
    emit_pos_then pos @@ emit_clone env e
  | A.Shape fl ->
    emit_pos_then pos @@ emit_shape env expr fl
  | A.Await e -> emit_await env pos e
  | A.Yield e -> emit_yield env pos e
  | A.Yield_break ->
    failwith "yield break should be in statement position"
  | A.Yield_from _ -> failwith "complex yield_from expression"
  | A.Lfun _ ->
    failwith "expected Lfun to be converted to Efun during closure conversion emit_expr"
  | A.Efun (fundef, ids) ->
    emit_pos_then pos @@ emit_lambda env fundef ids
  | A.Class_get (cid, id)  ->
    emit_class_get env QueryOp.CGet cid id
  | A.String2 es ->
    emit_string2 env pos es
  | A.BracedExpr e -> emit_expr env e
  | A.Id id ->
    emit_pos_then pos @@ emit_id env id
  | A.Xml (id, attributes, children) ->
    emit_xhp env (fst expr) id attributes children
  | A.Callconv (kind, e) ->
    emit_callconv env kind e
  | A.Import (flavor, e) ->
    emit_import env annot flavor e
  | A.Omitted -> empty
  | A.Unsafe_expr _ ->
    failwith "Unsafe expression should be removed during closure conversion"
  | A.Suspend _ ->
    failwith "Codegen for 'suspend' operator is not supported"
  | A.List _ ->
    Emit_fatal.raise_fatal_parse pos
      "list() can only be used as an lvar. Did you mean to use tuple()?"
  | A.Any ->
     failwith "Cannot codegen from an Any node"
  | A.This
  | A.Lplaceholder _
  | A.Dollardollar _ ->
     failwith "TODO Codegen after naming pass on AAST"
  | A.Typename _ -> failwith "Typename should not occur in expressions"
  | A.ImmutableVar _ -> failwith "Codegen for 'let' is not supported"
  | A.PU_atom _
  | A.PU_identifier _ ->
     failwith "TODO(T35357243): Pocket Universes syntax must be erased by now"
  | A.Fun_id _ -> failwith "TODO Unimplemented expression Fun"
  | A.Method_id (_, _) -> failwith "TODO Unimplemented expression Method"
  | A.Method_caller (_, _) -> failwith "TODO Unimplemented expression Method"
  | A.Smethod_id (_, _) -> failwith "TODO Unimplemented expression Smethod"
  | A.Special_func _ -> failwith "TODO Unimplemented expression Special"
  | A.Assert _ -> failwith "TODO Unimplemented expression Assert"

and emit_static_collection ~transform_to_collection pos tv =
  let transform_instr =
    match transform_to_collection with
    | Some collection_type ->
      instr_colfromarray collection_type
    | _ -> empty
  in
  gather [
    emit_pos pos;
    instr (ILitConst (TypedValue tv));
    transform_instr;
  ]

and emit_value_only_collection env pos (es : A.afield list) constructor =
  let limit = max_array_elem_on_stack () in
  let inline (exprs : A.afield list) =
    gather
    [gather @@ List.map exprs
      ~f:(function
        (* Drop the keys *)
        | A.AFkvalue (_, e)
        | A.AFvalue e -> emit_expr env e);
      emit_pos pos;
      instr @@ ILitConst (constructor @@ List.length exprs)]
  in
  let outofline (exprs : A.afield list) =
    gather @@
    List.map exprs
      ~f:(function
        (* Drop the keys *)
        | A.AFkvalue (_, e)
        | A.AFvalue e -> gather [emit_expr env e; instr_add_new_elemc])
  in
  match (List.groupi ~break:(fun i _ _ -> i = limit) es) with
    | [] -> empty
    | x1 :: [] -> inline x1
    | x1 :: x2 :: _ -> gather [inline x1; outofline x2]

and emit_keyvalue_collection name env pos (es : A.afield list) constructor =
  let name = SU.strip_ns name in
  let transform_instr =
    if name = "dict" || name = "array" then empty else
      let collection_type = collection_type name in
      instr_colfromarray collection_type
  in
  let add_elem_instr =
    if name = "array" then instr_add_new_elemc
    else gather [instr_dup; instr_add_elemc]
  in
  gather [
    emit_pos pos;
    instr (ILitConst constructor);
    gather (List.map es ~f:(expr_and_new env pos add_elem_instr instr_add_elemc));
    emit_pos pos;
    transform_instr;
  ]

and emit_struct_array env pos (es : A.afield list) ctor =
  let es =
    List.map es
      ~f:(function A.AFkvalue (k, v) ->
            let ns = Emit_env.get_namespace env in
            (* TODO: Consider reusing folded keys from is_struct_init *)
            begin match snd @@ Ast_constant_folder.fold_expr ns k with
            | A.String s -> s, emit_expr env v
            | _ -> failwith "Key must be a string"
            end
          | _ -> failwith "impossible")
  in
  gather [
    gather @@ List.map es ~f:snd;
    emit_pos pos;
    ctor @@ List.map es ~f:fst;
  ]

(* isPackedInit() returns true if this expression list looks like an
 * array with no keys and no ref values *)
and is_packed_init ?(hack_arr_compat=true) (es : A.afield list) =
  let is_only_values =
    List.for_all es ~f:(function A.AFkvalue _ -> false | _ -> true)
  in
  let keys_are_zero_indexed_properly_formed =
    List.foldi es ~init:true ~f:(fun i b f -> b && match f with
      | A.AFkvalue ((_, A.Int k), _) ->
        int_of_string k = i
      (* arrays with int-like string keys are still considered packed
         and should be emitted via NewArray *)
      | A.AFkvalue ((_, A.String k), _) when not hack_arr_compat ->
        (try int_of_string k = i with Failure _ -> false)
      (* True and False are considered 1 and 0, respectively *)
      | A.AFkvalue ((_, A.True), _) ->
        i = 1
      | A.AFkvalue ((_, A.False), _) ->
        i = 0
      | A.AFvalue _ ->
        true
      | _ -> false)
  in
  let has_references =
    (* Reference can only exist as a value *)
    List.exists es
      ~f:(function A.AFkvalue (_, e)
                 | A.AFvalue e -> expr_starts_with_ref e)
  in
  let has_bool_keys =
    List.exists es
      ~f:(function A.AFkvalue ((_, (A.True | A.False)), _) -> true | _ -> false)
  in
  (is_only_values || keys_are_zero_indexed_properly_formed)
  && not (has_bool_keys && (hack_arr_compat && hack_arr_compat_notices()))
  && not has_references
  && (List.length es) > 0

and is_struct_init env (es : A.afield list) allow_numerics =
  let has_references =
    (* Reference can only exist as a value *)
    List.exists es
      ~f:(function A.AFkvalue (_, e)
                 | A.AFvalue e -> expr_starts_with_ref e)
  in
  let keys = ULS.empty in
  let are_all_keys_non_numeric_strings, keys =
    List.fold_right es ~init:(true, keys) ~f:(fun field (b, keys) ->
      match field with
      | A.AFkvalue (key, _) ->
        let ns = Emit_env.get_namespace env in
        begin match snd @@ Ast_constant_folder.fold_expr ns key with
        | A.String s ->
          b && (Option.is_none
            @@ Typed_value.string_to_int_opt
                ~allow_following:false ~allow_inf:false s),
          ULS.add keys s
        | _ -> false, keys
        end
      | _ -> false, keys)
  in
  let num_keys = List.length es in
  let has_duplicate_keys =
    ULS.cardinal keys <> num_keys
  in
  let limit = max_array_elem_on_stack () in
  (allow_numerics || are_all_keys_non_numeric_strings)
  && not has_duplicate_keys
  && not has_references
  && num_keys <= limit
  && num_keys <> 0

(* transform_to_collection argument keeps track of
 * what collection to transform to *)
and emit_dynamic_collection env (expr : A.expr) (es : A.afield list) =
  let ((pos, _), expr_) = expr in
  let count = List.length es in
  match expr_ with
  | A.ValCollection (`Vec, _, _)
  | A.Collection ((_, "vec"), _, _) ->
    emit_value_only_collection env pos es (fun n -> NewVecArray n)
  | A.ValCollection (`Keyset, _, _)
  | A.Collection ((_, "keyset"), _, _) ->
    emit_value_only_collection env pos es (fun n -> NewKeysetArray n)
  | A.KeyValCollection (`Dict, _, _)
  | A.Collection ((_, "dict"), _, _) ->
     if is_struct_init env es true then
       emit_struct_array env pos es instr_newstructdict
     else
       emit_keyvalue_collection "dict" env pos es (NewDictArray count)
  | A.ValCollection (`Set, _, _) ->
     if is_struct_init env es true then
       gather [
           emit_struct_array env pos es instr_newstructdict;
           emit_pos pos;
           instr_colfromarray (collection_type "Set");
         ]
     else
       emit_keyvalue_collection "Set" env pos es (NewDictArray count)
  | A.ValCollection (`ImmSet, _, _) ->
     if is_struct_init env es true then
       gather [
           emit_struct_array env pos es instr_newstructdict;
           emit_pos pos;
           instr_colfromarray (collection_type "ImmSet");
         ]
     else
       emit_keyvalue_collection "ImmSet" env pos es (NewDictArray count)
  | A.KeyValCollection (`Map, _, _) ->
     if is_struct_init env es true then
       gather [
           emit_struct_array env pos es instr_newstructdict;
           emit_pos pos;
           instr_colfromarray (collection_type "Map");
         ]
     else
       emit_keyvalue_collection "Map" env pos es (NewDictArray count)
  | A.KeyValCollection (`ImmMap, _, _) ->
     if is_struct_init env es true then
       gather [
           emit_struct_array env pos es instr_newstructdict;
           emit_pos pos;
           instr_colfromarray (collection_type "ImmMap");
         ]
     else
       emit_keyvalue_collection "ImmMap" env pos es (NewDictArray count)
  | A.Collection ((_, name), _, _)
     when SU.strip_ns name = "Set"
      || SU.strip_ns name = "ImmSet"
      || SU.strip_ns name = "Map"
      || SU.strip_ns name = "ImmMap" ->
     if is_struct_init env es true then
       gather [
           emit_struct_array env pos es instr_newstructdict;
           emit_pos pos;
           instr_colfromarray (collection_type (SU.strip_ns name));
         ]
     else
       emit_keyvalue_collection name env pos es (NewDictArray count)

  | A.Varray _ ->
     emit_value_only_collection env pos es
       (fun n -> if hack_arr_dv_arrs () then (NewVecArray n) else (NewVArray n))
  | A.Darray _ ->
     if is_struct_init env es false then
       emit_struct_array env pos es
         (fun arg -> emit_pos_then pos @@
           if hack_arr_dv_arrs () then instr_newstructdict arg else instr_newstructdarray arg)
     else
       emit_keyvalue_collection "array" env pos es
         (if hack_arr_dv_arrs () then (NewDictArray count) else (NewDArray count))
  | _ ->
  (* From here on, we're only dealing with PHP arrays *)
  if is_packed_init es then
    emit_value_only_collection env pos es (fun n -> NewPackedArray n)
  else if is_struct_init env es false then
    emit_struct_array env pos es instr_newstructarray
  else if is_packed_init ~hack_arr_compat:false es then
    emit_keyvalue_collection "array" env pos es (NewArray count)
  else
    emit_keyvalue_collection "array" env pos es (NewMixedArray count)

and emit_named_collection env (expr : A.expr) pos name (fields : A.afield list) =
  let name = SU.Types.fix_casing @@ SU.strip_ns name in
  match name with
  | "dict" | "vec" | "keyset" ->
    emit_pos_then pos @@
    emit_collection env expr fields
  | "Vector" | "ImmVector" ->
    let collection_type = collection_type name in
    if fields = []
    then
      emit_pos_then pos @@
      instr_newcol collection_type
    else
    let ((_, ty), _) = expr in
    let annot = (pos, ty) in
    gather [
      emit_collection env (annot, A.Collection ((pos, "vec"), None, fields)) fields;
      instr_colfromarray collection_type;
    ]
  | "Map" | "ImmMap" | "Set" | "ImmSet" ->
    let collection_type = collection_type name in
    if fields = []
    then
      emit_pos_then pos @@
      instr_newcol collection_type
    else
      emit_collection
        ~transform_to_collection:collection_type
        env
        expr
        fields
  | "Pair" ->
    gather [
      gather (List.map fields (function
        | A.AFvalue e -> emit_expr env e
        | _ -> failwith "impossible Pair argument"));
      instr (ILitConst NewPair);
    ]
  | _ -> failwith @@ "collection: " ^ name ^ " does not exist"

and is_php_array = function
 | _, A.Array _ -> true
 | _, A.Varray _ -> not (hack_arr_dv_arrs ())
 | _, A.Darray _ -> not (hack_arr_dv_arrs ())
 | _ -> false

and emit_collection ?(transform_to_collection) env (expr : A.expr) (es : A.afield list) =
  let pos = Tast_annotate.get_pos expr in
  match Ast_constant_folder.expr_to_opt_typed_value
          ~allow_maps:true
          ~restrict_keys:(not @@ is_php_array expr)
          (Emit_env.get_namespace env)
          expr
  with
  | Some tv ->
    emit_static_collection ~transform_to_collection pos tv
  | None ->
    emit_dynamic_collection env expr es

and emit_pipe env (e1: A.expr) (e2: A.expr) =
  let lhs_instrs = emit_expr env e1 in
  Scope.with_unnamed_local @@ fun temp ->
  let env = Emit_env.with_pipe_var temp env in
  let rhs_instrs = emit_expr env e2 in
  gather [ lhs_instrs; instr_popl temp ], rhs_instrs, instr_unsetl temp

(* Emit code that is equivalent to
 *   <code for expr>
 *   JmpZ label
 * Generate specialized code in case expr is statically known, and for
 * !, && and || expressions
 *)
and emit_jmpz env (expr : A.expr) label: emit_jmp_result =
  let ((pos, _), expr_) = expr in
  let with_pos i = emit_pos_then pos i in
  let opt = optimize_null_check () in
  let ns = Emit_env.get_namespace env in
  match Ast_constant_folder.expr_to_opt_typed_value ns expr with
  | Some v ->
    let b = Typed_value.to_bool v in
    if b then
      { instrs = with_pos empty;
        is_fallthrough = true;
        is_label_used = false; }
    else
      { instrs = with_pos @@ instr_jmp label;
        is_fallthrough = false;
        is_label_used = true; }
  | None ->
    begin match expr_ with
    | A.Unop(Ast.Unot, e) ->
      let (annot, expr_) = e in
      emit_jmpnz env annot expr_ label
    | A.Binop(Ast.Barbar, e1, e2) ->
      let skip_label = Label.next_regular () in
      let (e1_annot, e1_expr_) = e1 in
      let r1 = emit_jmpnz env e1_annot e1_expr_ skip_label in
      if not r1.is_fallthrough
      then
        let instrs =
          if r1.is_label_used then gather [ r1.instrs; instr_label skip_label; ]
          else r1.instrs in
        { instrs = with_pos instrs;
          is_fallthrough = r1.is_label_used;
          is_label_used = false }
      else
        let r2 = emit_jmpz env e2 label in
        let instrs = gather [
          r1.instrs;
          r2.instrs;
          optional r1.is_label_used [instr_label skip_label];
        ] in
        { instrs = with_pos instrs;
          is_fallthrough = r2.is_fallthrough || r1.is_label_used;
          is_label_used = r2.is_label_used }
    | A.Binop(Ast.Ampamp, e1, e2) ->
      let r1 = emit_jmpz env e1 label in
      if not r1.is_fallthrough
      then
        { instrs = with_pos r1.instrs;
          is_fallthrough = false;
          is_label_used = r1.is_label_used }
      else
        let r2 = emit_jmpz env e2 label in
        { instrs = with_pos @@ gather [ r1.instrs; r2.instrs; ];
          is_fallthrough = r2.is_fallthrough;
           is_label_used = r1.is_label_used || r2.is_label_used }
    | A.Binop(Ast.Eqeqeq, e, (_, A.Null))
    | A.Binop(Ast.Eqeqeq, (_, A.Null), e) when opt ->
      { instrs = with_pos @@ gather [
          emit_is_null env e;
          instr_jmpz label;
        ];
        is_fallthrough = true;
        is_label_used = true; }
    | A.Binop(Ast.Diff2, e, (_, A.Null))
    | A.Binop(Ast.Diff2, (_, A.Null), e) when opt ->
      { instrs = with_pos @@ gather [
          emit_is_null env e;
          instr_jmpnz label;
        ];
        is_fallthrough = true;
        is_label_used = true; }
    | _ ->
      { instrs = with_pos @@ gather [
          emit_expr env expr;
          instr_jmpz label;
        ];
        is_fallthrough = true;
        is_label_used = true; }
    end

(* Emit code that is equivalent to
 *   <code for expr>
 *   JmpNZ label
 * Generate specialized code in case expr is statically known, and for
 * !, && and || expressions
 *)
and emit_jmpnz env annot (expr_ : A.expr_) label: emit_jmp_result =
  let (pos, _) = annot in
  let with_pos i = emit_pos_then pos i in
  let opt = optimize_null_check () in
  match Ast_constant_folder.expr_to_opt_typed_value (Emit_env.get_namespace env) (annot, expr_) with
  | Some v ->
    if Typed_value.to_bool v
    then
      { instrs = with_pos @@ instr_jmp label;
        is_fallthrough = false;
        is_label_used = true }
    else
      { instrs = with_pos empty;
        is_fallthrough = true;
        is_label_used = false }
  | None ->
    begin match expr_ with
    | A.Unop(Ast.Unot, e) ->
      emit_jmpz env e label
    | A.Binop(Ast.Barbar, (annot1, e1), (annot2, e2)) ->
       let r1 = emit_jmpnz env annot1 e1 label in
      if not r1.is_fallthrough then r1
      else
        let r2 = emit_jmpnz env annot2 e2 label in
        { instrs = with_pos @@ gather [ r1.instrs; r2.instrs ];
          is_fallthrough = r2.is_fallthrough;
          is_label_used = r1.is_label_used || r2.is_label_used }
    | A.Binop(Ast.Ampamp, e1, (annot2, e2)) ->
      let skip_label = Label.next_regular () in
      let r1 = emit_jmpz env e1 skip_label in
      if not r1.is_fallthrough
      then
        { instrs = with_pos @@ gather [
            r1.instrs;
            optional r1.is_label_used [instr_label skip_label]
          ];
          is_fallthrough = r1.is_label_used;
          is_label_used = false }
      else begin
          let r2 = emit_jmpnz env annot2 e2 label in
        { instrs = with_pos @@ gather [
            r1.instrs;
            r2.instrs;
            optional r1.is_label_used [instr_label skip_label]
          ];
          is_fallthrough = r2.is_fallthrough || r1.is_label_used;
          is_label_used = r2.is_label_used }
      end
    | A.Binop(Ast.Eqeqeq, e, (_, A.Null))
    | A.Binop(Ast.Eqeqeq, (_, A.Null), e) when opt ->
      { instrs = with_pos @@ gather [
          emit_is_null env e;
          instr_jmpnz label;
        ];
        is_fallthrough = true;
        is_label_used = true; }
    | A.Binop(Ast.Diff2, e, (_, A.Null))
    | A.Binop(Ast.Diff2, (_, A.Null), e) when opt ->
      { instrs = with_pos @@ gather [
          emit_is_null env e;
          instr_jmpz label;
        ];
        is_fallthrough = true;
        is_label_used = true; }
    | _ ->
      { instrs = with_pos @@ gather [
          emit_expr env (annot, expr_);
          instr_jmpnz label;
        ];
        is_fallthrough = true;
        is_label_used = true; }
    end

and emit_short_circuit_op env annot (expr : A.expr_) =
  let (pos, _) = annot in
  let its_true = Label.next_regular () in
  let its_done = Label.next_regular () in
  let r1 = emit_jmpnz env annot expr its_true in
  let if_true =
    if r1.is_label_used then gather [
      instr_label its_true;
      emit_pos pos;
      instr_true;
    ]
    else empty in
  if r1.is_fallthrough
  then gather [
    r1.instrs;
    emit_pos pos;
    instr_false;
    instr_jmp its_done;
    if_true;
    instr_label its_done ]
  else gather [
    r1.instrs;
    if_true; ]

and emit_null_coalesce_assignment env pos (e1 : A.expr) (e2 : A.expr) =
  let end_label = Label.next_regular () in
  let do_set_label = Label.next_regular () in
  let l_nonnull = Local.get_unnamed_local () in
  let quiet_instr, querym_n_unpopped =
    emit_quiet_expr ~null_coalesce_assignment:true env pos e1 in
  let emit_popc_n n_unpopped =
    match n_unpopped with
    | Some n -> gather (List.init n (fun _ -> instr_popc))
    | None -> empty
  in
  gather [
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
    emit_lval_op ~null_coalesce_assignment:true env pos LValOp.Set e1 (Some e2);
    instr_label end_label;
  ]

and emit_quiet_expr ?(null_coalesce_assignment=false) (env : Emit_env.t) pos (expr : A.expr): Instruction_sequence.t * Hhbc_ast.num_params option =
  let (_, expr_) = expr in
  match expr_ with
  | A.Lvar (name_pos, name) when (Local_id.get_name name) = SN.Superglobals.globals ->
    gather [
      emit_pos name_pos;
      instr_string (SU.Locals.strip_dollar (Local_id.get_name name));
      emit_pos pos;
      instr (IGet CGetQuietG)
    ], None
  | A.Lvar ((_, name)) when not (is_local_this env name) ->
    instr_cgetquietl (get_local env (pos, (Local_id.get_name name))), None
  | A.Array_get((_, A.Lvar (_, x)), Some e) when (Local_id.get_name x) = SN.Superglobals.globals ->
    gather [
      emit_expr env e;
      emit_pos pos;
      instr (IGet CGetQuietG)
    ], None
  | A.Array_get(base_expr, opt_elem_expr) ->
    emit_array_get ~null_coalesce_assignment
      env pos QueryOp.CGetQuiet base_expr opt_elem_expr
  | A.Obj_get (expr, prop, nullflavor) ->
    emit_obj_get ~null_coalesce_assignment
      env pos QueryOp.CGetQuiet expr prop nullflavor
  | _ ->
    emit_expr env expr, None

(* returns instruction that will represent setter for $base[local] where
   is_base is true when result cell is base for another subscript operator and
   false when it is final left hand side of the assignment *)
and emit_store_for_simple_base ~is_base env pos elem_stack_size (base_expr : A.expr) local =
  let base_expr_instrs_begin,
      base_expr_instrs_end,
      base_setup_instrs,
      _ =
    emit_base ~is_object:false ~notice:Notice env MemberOpMode.Define
      elem_stack_size base_expr in
  let expr =
    let mk = MemberKey.EL local in
    if is_base then instr_dim MemberOpMode.Define mk else instr_setm 0 mk in
  gather [
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
and get_local_temp_kind ~is_base inout_param_info env (e_opt : A.expr option) =
  match e_opt, inout_param_info with
  (* not inout case - no need to save *)
  | _, None -> None
  (* local that will later be overwritten *)
  | Some (_, A.Lvar (_, id)), Some (i, aliases)
    when InoutLocals.should_save_local_value (Local_id.get_name id) i aliases -> Some Value_kind_local
  (* non-trivial expression *)
  | Some e, _ ->
    if is_trivial ~is_base env e then None else Some Value_kind_expression
  | None, _ -> None

and is_trivial ~is_base env (_, e) =
  match e with
  | A.Int _ | A.String _ ->
    true
  | A.Lvar (_, s) ->
    not (is_local_this env s) || Emit_env.get_needs_local_this env
  | A.Array_get _ when not is_base -> false
  | A.Array_get (b, None) -> is_trivial ~is_base env b
  | A.Array_get (b, Some e) ->
    is_trivial ~is_base env b && is_trivial ~is_base env e
  | _ ->
    false

(* Emit code for e1[e2] or isset(e1[e2]).
 *)
and emit_array_get ?(null_coalesce_assignment=false) ?(no_final=false) ?mode
 env outer_pos qop (base_expr : A.expr) (opt_elem_expr : A.expr option) =
 let result =
   emit_array_get_worker ~null_coalesce_assignment ~no_final ?mode ~inout_param_info:None
   env outer_pos qop base_expr opt_elem_expr in
 match result with
 | Array_get_regular i, querym_n_unpopped -> i, querym_n_unpopped
 | Array_get_inout _, _ -> failwith "unexpected inout"

and emit_array_get_worker ?(null_coalesce_assignment=false) ?(no_final=false) ?mode ~inout_param_info
  env outer_pos qop (base_expr : A.expr) (opt_elem_expr : A.expr option) =
  (* Disallow use of array(..)[] *)
  match base_expr, opt_elem_expr with
  | ((pos, _), A.Array _), None ->
    Emit_fatal.raise_fatal_parse pos "Can't use array() as base in write context"
  | ((pos, _), _), None when not (Emit_env.does_env_allow_array_append env)->
    Emit_fatal.raise_fatal_runtime pos "Can't use [] for reading"
  | _ ->
  let local_temp_kind =
    get_local_temp_kind ~is_base:false inout_param_info env opt_elem_expr in
  let mode =
    if null_coalesce_assignment then MemberOpMode.Warn
    else Option.value mode ~default:(get_queryMOpMode false qop) in
  let querym_n_unpopped = ref None in
  let elem_expr_instrs, elem_stack_size =
    emit_elem_instrs ~local_temp_kind ~null_coalesce_assignment env opt_elem_expr in
  let mk = get_elem_member_key ~null_coalesce_assignment env 0 opt_elem_expr in
  let base_result =
    emit_base_worker ~is_object:false ~inout_param_info
      ~notice:(match qop with QueryOp.Isset -> NoNotice | _ -> Notice)
      ~null_coalesce_assignment
      env mode elem_stack_size base_expr in
  let make_final total_stack_size =
    if no_final then empty else
    instr (IFinal (
      if null_coalesce_assignment then begin
        querym_n_unpopped := Some total_stack_size;
        QueryM (0, qop, mk)
      end else
        QueryM (total_stack_size, qop, mk)
    )) in
  let instr = match base_result, local_temp_kind with
  | Array_get_base_regular base, None ->
    (* both base and expression don't need to store anything *)
    Array_get_regular (gather [
      base.instrs_begin;
      elem_expr_instrs;
      base.instrs_end;
      emit_pos outer_pos;
      base.setup_instrs;
      make_final (base.stack_size + elem_stack_size);
    ])
  | Array_get_base_regular base, Some local_kind ->
    (* base does not need temp locals but index expression does *)
    let local = Local.get_unnamed_local () in
    let load =
      [
        (* load base and indexer, value of indexer will be saved in local *)
        gather [
          base.instrs_begin;
          elem_expr_instrs
        ], Some (local, local_kind);
        (* finish loading the value *)
        gather [
          base.instrs_end;
          emit_pos outer_pos;
          base.setup_instrs;
          make_final (base.stack_size + elem_stack_size);
        ], None
      ] in
    let store =
      gather [
        emit_store_for_simple_base ~is_base:false env outer_pos elem_stack_size
          base_expr local;
        instr_popc;
      ] in
    Array_get_inout { load; store }

  | Array_get_base_inout base, None ->
    (* base needs temp locals, indexer - does not,
       simply concat two instruction sequences *)
    let load = base.load.instrs_begin @ [
      gather [
        elem_expr_instrs;
        base.load.instrs_end;
        emit_pos outer_pos;
        base.load.setup_instrs;
        make_final (base.load.stack_size + elem_stack_size);
      ], None
    ] in
    let store = gather [
      base.store;
      instr_setm 0 mk;
      instr_popc;
    ] in
    Array_get_inout { load; store }

  | Array_get_base_inout base, Some local_kind ->
    (* both base and index need temp locals,
       create local for index value *)
    let local = Local.get_unnamed_local () in
    let load =
      (* load base *)
      base.load.instrs_begin @ [
      (* load index, value will be saved in local *)
      elem_expr_instrs, Some (local, local_kind);
      gather [
        base.load.instrs_end;
        emit_pos outer_pos;
        base.load.setup_instrs;
        make_final (base.load.stack_size + elem_stack_size);
      ], None
    ] in
    let store = gather [
      base.store;
      instr_setm 0 (MemberKey.EL local);
      instr_popc;
    ] in
    Array_get_inout { load; store }
  in
  instr, !querym_n_unpopped

(* Emit code for e1->e2 or e1?->e2 or isset(e1->e2).
 *)
and emit_obj_get ?(null_coalesce_assignment=false) ?(arg_by_ref=false)
  env pos qop (expr : A.expr) (prop : A.expr) null_flavor =
  let (annot, expr_) = expr in
  match expr_ with
  | A.Lvar (pos, id)
    when (Local_id.get_name id) = SN.SpecialIdents.this && null_flavor = Ast.OG_nullsafe ->
    Emit_fatal.raise_fatal_parse
      pos "?-> is not allowed with $this"
  | _ ->
    begin match snd prop with
    | A.Id (_, s) when SU.Xhp.is_xhp s ->
      emit_xhp_obj_get env pos annot expr s null_flavor, None
    | _ ->
      let mode =
        if null_coalesce_assignment then MemberOpMode.Warn
        else get_queryMOpMode arg_by_ref qop in
      let mk, prop_expr_instrs, prop_stack_size =
        emit_prop_expr ~null_coalesce_assignment env null_flavor 0 prop in
      let base_expr_instrs_begin,
          base_expr_instrs_end,
          base_setup_instrs,
          base_stack_size =
        emit_base
          ~is_object:true ~notice:Notice ~null_coalesce_assignment
          env mode prop_stack_size expr
      in
      let total_stack_size = prop_stack_size + base_stack_size in
      let final_instr =
        instr (IFinal (
          if arg_by_ref then
            VGetM (total_stack_size, mk)
          else if null_coalesce_assignment then
            QueryM (0, qop, mk)
          else
            QueryM (total_stack_size, qop, mk)
        )) in
      let querym_n_unpopped =
        if null_coalesce_assignment then Some total_stack_size else None in
      let instr =
        gather [
          base_expr_instrs_begin;
          prop_expr_instrs;
          base_expr_instrs_end;
          emit_pos pos;
          base_setup_instrs;
          final_instr
        ] in
      instr, querym_n_unpopped
    end

and is_special_class_constant_accessed_with_class_id (cName : A.class_id_) (id : string) =
  let is_self_parent_or_static =
    match cName with
    | A.CIexpr (_, A.Id (_, cName))
      when SU.is_self cName || SU.is_parent cName || SU.is_static cName -> true
    | _ -> false in
  SU.is_class id && (not is_self_parent_or_static)

and emit_elem_instrs env ~local_temp_kind ?(null_coalesce_assignment=false) (opt_elem_expr : A.expr option) =
  match opt_elem_expr with
  (* These all have special inline versions of member keys *)
  | Some (_, (A.Int _ | A.String _)) -> empty, 0
  | Some (_, (A.Lvar (pos, id))) when not (is_local_this env id) ->
    if Option.is_some local_temp_kind
    then instr_cgetquietl (get_local env (pos, (Local_id.get_name id))), 0
    else if null_coalesce_assignment then instr_cgetl (get_local env (pos, (Local_id.get_name id))), 1
    else empty, 0
  (* Handle Foo::class but not self::class. *)
  | Some (_, (A.Class_const ((_, cid), (_, id))))
    when is_special_class_constant_accessed_with_class_id cid id -> empty, 0
  | Some expr -> emit_expr env expr, 1
  | None -> empty, 0

(* Get the member key for an array element expression: the `elem` in
 * expressions of the form `base[elem]`.
 * If the array element is missing, use the special key `W`.
 *)
and get_elem_member_key ?(null_coalesce_assignment=false) env stack_index (opt_expr : A.expr option) =
  match opt_expr with
  (* Special case for local *)
  | Some (_, A.Lvar (p, id)) when not (is_local_this env id) ->
    if null_coalesce_assignment then MemberKey.EC stack_index
    else MemberKey.EL (get_local env (p, (Local_id.get_name id)))
  (* Special case for literal integer *)
  | Some (_, A.Int str as int_expr)->
    let open Ast_constant_folder in
    let namespace = Emit_env.get_namespace env in
    begin match expr_to_typed_value namespace int_expr with
    | TV.Int i -> MemberKey.EI i
    | _ -> failwith (str ^ " is not a valid integer index")
    end
  (* Special case for literal string *)
  | Some (_, A.String str) -> MemberKey.ET str
  (* Special case for class name *)
  | Some (_, (A.Class_const ((_, cid), (p, id))))
    when is_special_class_constant_accessed_with_class_id cid id ->
    let cName =
      match cid,
            Ast_scope.Scope.get_class (Emit_env.get_scope env)
      with
      | A.CIself, Some cd -> SU.strip_global_ns @@ snd cd.A.c_name
      | A.CIexpr (_, A.Id (_, id)), _
      | A.CI (_, id), _ -> SU.strip_global_ns id
      | _ -> failwith "Unreachable due to is_special_class_constant_accessed_with_class_id"
    in
    let fq_id =
      Hhbc_id.Class.elaborate_id (Emit_env.get_namespace env) (p, cName) in
    MemberKey.ET (Hhbc_id.Class.to_raw_string fq_id)
  (* General case *)
  | Some _ -> MemberKey.EC stack_index
  (* ELement missing (so it's array append) *)
  | None -> MemberKey.W

(* Get the member key for a property, and return any instructions and
 * the size of the stack in the case that the property cannot be
 * placed inline in the instruction. *)
and emit_prop_expr ?(null_coalesce_assignment=false) env null_flavor
  stack_index (prop_expr : A.expr) =
  let mk =
    match snd prop_expr with
    | A.Id (pos, name) when String_utils.string_starts_with name "$" ->
       MemberKey.PL (get_local env (pos, name))
    (* Special case for known property name *)
    | A.Id (_, id)
    | A.String id ->
      let pid = Hhbc_id.Prop.from_ast_name id in
      begin match null_flavor with
      | Ast.OG_nullthrows -> MemberKey.PT pid
      | Ast.OG_nullsafe -> MemberKey.QT pid
      end
    | A.Lvar (pos, name) when not (is_local_this env name) ->
      MemberKey.PL (get_local env (pos, (Local_id.get_name name)))
    (* General case *)
    | _ ->
      MemberKey.PC stack_index
  in
  (* For nullsafe access, insist that property is known *)
  begin match mk with
  | MemberKey.PL _ | MemberKey.PC _ ->
    let ((pos, _), _) = prop_expr in
    if null_flavor = Ast.OG_nullsafe then
      Emit_fatal.raise_fatal_parse pos
        "?-> can only be used with scalar property names"
  | _ -> ()
  end;
  match mk with
  | MemberKey.PC _ ->
    mk, emit_expr env prop_expr, 1
  | MemberKey.PL local ->
    if null_coalesce_assignment
    then MemberKey.PC stack_index, instr_cgetl local, 1
    else mk, empty, 0
  | _ ->
    mk, empty, 0

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

and emit_base ~is_object ~notice ?(null_coalesce_assignment=false) env mode base_offset (e : A.expr) =
  let result = emit_base_worker ~is_object ~notice ~inout_param_info:None ~null_coalesce_assignment
    env mode base_offset e in
  match result with
  | Array_get_base_regular i ->
    i.instrs_begin,
    i.instrs_end,
    i.setup_instrs,
    i.stack_size
  | Array_get_base_inout _ -> failwith "unexpected inout"

and emit_base_worker ~is_object ~notice ~inout_param_info ?(null_coalesce_assignment=false)
  env mode base_offset
  (expr : A.expr) =
  let ((pos, _) as annot, expr_) = expr in
  let base_mode =
    if mode = MemberOpMode.InOut
    then
      MemberOpMode.Warn
    else mode in
    let local_temp_kind =
      get_local_temp_kind ~is_base:true inout_param_info env (Some expr) in
    (* generic handler that will try to save local into temp if this is necessary *)
    let emit_default instrs_begin instrs_end setup_instrs stack_size =
      match local_temp_kind with
      | Some local_temp ->
        let local = Local.get_unnamed_local () in
        Array_get_base_inout {
          load = {
            (* run begin part, result will be stored into temp  *)
            instrs_begin = [instrs_begin, Some (local, local_temp)];
            instrs_end;
            setup_instrs;
            stack_size };
          store =  instr_basel local MemberOpMode.Define
        }
      | _ ->
        Array_get_base_regular {
          instrs_begin; instrs_end; setup_instrs; stack_size }
   in
   match expr_ with
   | A.Lvar (name_pos, x) when SN.Superglobals.is_superglobal (Local_id.get_name x) ->
     emit_default
       (emit_pos_then name_pos @@ instr_string (SU.Locals.strip_dollar (Local_id.get_name x)))
       empty
       (instr (IBase (BaseGC (base_offset, base_mode))))
       1

   | A.Lvar (thispos, x) when is_object && (Local_id.get_name x) = SN.SpecialIdents.this ->
     emit_default
       (emit_pos_then thispos @@ instr (IMisc CheckThis))
       empty
       (instr (IBase BaseH))
       0

   | A.Lvar (pos, str)
     when not (is_local_this env str) || Emit_env.get_needs_local_this env ->
     let v = get_local env (pos, (Local_id.get_name str)) in
     if Option.is_some local_temp_kind
     then begin
       emit_default
         (instr_cgetquietl v)
         empty
         (instr_basel v base_mode)
         0
     end
     else begin
       emit_default
         empty
         empty
         (instr (IBase (BaseL (v, base_mode))))
         0
     end

   | A.Lvar lid ->
     emit_default
       (emit_local ~notice ~need_ref:false env lid)
       empty
       (instr (IBase (BaseC (base_offset, base_mode))))
       1

   | A.Array_get ((_, A.Lvar (_, x)), Some (_, A.Lvar (y_pos, y_id)))
     when (Local_id.get_name x) = SN.Superglobals.globals ->
     let v = get_local env (y_pos, (Local_id.get_name y_id)) in
     emit_default
       empty
       empty
       (instr (IBase (BaseGL (v, base_mode))))
       0

   | A.Array_get ((_, A.Lvar (_, x)), Some e) when (Local_id.get_name x) = SN.Superglobals.globals ->
     let elem_expr_instrs = emit_expr env e in
     emit_default
       elem_expr_instrs
       empty
       (instr (IBase (BaseGC (base_offset, base_mode))))
       1
   (* $a[] can not be used as the base of an array get unless as an lval *)
   | A.Array_get (_, None)
      when not (Emit_env.does_env_allow_array_append env) ->
      Emit_fatal.raise_fatal_runtime pos "Can't use [] for reading"
   (* base is in turn array_get - do a specific handling for inout params
      if necessary *)
   | A.Array_get(base_expr, opt_elem_expr) ->

     let local_temp_kind =
       get_local_temp_kind ~is_base:false inout_param_info env opt_elem_expr in
     let elem_expr_instrs, elem_stack_size =
       emit_elem_instrs ~local_temp_kind ~null_coalesce_assignment env opt_elem_expr in
     let base_result =
       emit_base_worker
         ~notice ~is_object:false ~inout_param_info ~null_coalesce_assignment
         env mode (base_offset + elem_stack_size) base_expr
     in
     let mk = get_elem_member_key ~null_coalesce_assignment env base_offset opt_elem_expr in
     let make_setup_instrs base_setup_instrs =
       gather [
         base_setup_instrs;
         instr (IBase (Dim (mode, mk)))
       ] in
     begin match base_result, local_temp_kind with
     (* both base and index don't use temps - fallback to default handler  *)
     | Array_get_base_regular base, None ->
       emit_default
         (gather [
           base.instrs_begin;
           elem_expr_instrs;
         ])
         base.instrs_end
         (make_setup_instrs base.setup_instrs)
         (base.stack_size + elem_stack_size)
     | Array_get_base_regular base, Some local_temp ->
       (* base does not need temps but index does *)
       let local = Local.get_unnamed_local () in
       let instrs_begin = gather [
         base.instrs_begin;
         elem_expr_instrs;
       ] in
       Array_get_base_inout {
         load = {
           (* store result of instr_begin to temp *)
           instrs_begin = [instrs_begin, Some (local, local_temp)];
           instrs_end = base.instrs_end;
           setup_instrs = make_setup_instrs base.setup_instrs;
           stack_size = base.stack_size + elem_stack_size };
         store = emit_store_for_simple_base ~is_base:true env pos elem_stack_size
                 base_expr local
       }
     | Array_get_base_inout base, None ->
       (* base needs temps, index - does not *)
       Array_get_base_inout {
         load = {
           (* concat index evaluation to base *)
           instrs_begin = base.load.instrs_begin @ [elem_expr_instrs, None];
           instrs_end = base.load.instrs_end;
           setup_instrs = make_setup_instrs base.load.setup_instrs;
           stack_size = base.load.stack_size + elem_stack_size };
         store = gather [
          base.store;
          instr_dim MemberOpMode.Define mk;
         ]
       }
      | Array_get_base_inout base, Some local_kind ->
        (* both base and index needs locals *)
        let local = Local.get_unnamed_local () in
        Array_get_base_inout {
          load = {
            instrs_begin =
              base.load.instrs_begin @ [
                (* evaluate index, result will be stored in local *)
                elem_expr_instrs, Some (local, local_kind)
              ];
            instrs_end = base.load.instrs_end;
            setup_instrs = make_setup_instrs base.load.setup_instrs;
            stack_size = base.load.stack_size + elem_stack_size };
          store = gather [
            base.store;
            instr_dim MemberOpMode.Define (MemberKey.EL local);
          ]
        }
     end

   | A.Obj_get(base_expr, prop_expr, null_flavor) ->
     begin match snd prop_expr with
     | A.Id (_, s) when SU.Xhp.is_xhp s ->
       emit_default
         (emit_xhp_obj_get env pos annot base_expr s null_flavor)
         empty
         (gather [ instr_basec base_offset base_mode ])
         1
     | _ ->
       let mk, prop_expr_instrs, prop_stack_size =
         emit_prop_expr ~null_coalesce_assignment env null_flavor base_offset prop_expr in
       let base_expr_instrs_begin,
           base_expr_instrs_end,
           base_setup_instrs,
           base_stack_size =
         emit_base ~notice:Notice ~is_object:true ~null_coalesce_assignment
           env mode (base_offset + prop_stack_size) base_expr
       in
       let total_stack_size = prop_stack_size + base_stack_size in
       let final_instr = instr (IBase (Dim (mode, mk))) in
       emit_default
         (gather [
           base_expr_instrs_begin;
           prop_expr_instrs;
         ])
         base_expr_instrs_end
         (gather [
           base_setup_instrs;
           final_instr
         ])
         total_stack_size
     end

   | A.Class_get(cid, prop) ->
      let cexpr = class_id_to_class_expr ~resolve_self:false
       (Emit_env.get_scope env) cid in
     let cexpr_begin, cexpr_end = emit_class_expr env cexpr prop in
     emit_default
       cexpr_begin
       cexpr_end
       (instr_basesc base_offset base_mode)
       1
   | _ ->
     let base_expr_instrs = emit_expr env expr in
     emit_default
       base_expr_instrs
       empty
       (emit_pos_then pos @@
       instr (IBase (BaseC (base_offset, base_mode))))
       1

and emit_ignored_expr env ?(pop_pos : Pos.t = Pos.none) (e : A.expr) =
 match snd e with
 | A.Expr_list es -> gather @@ List.map ~f:(emit_ignored_expr env ~pop_pos) es
 | _ ->
   gather [
     emit_expr env e;
     emit_pos_then pop_pos @@ instr_popc
   ]

(*
 * Replaces erased generics with underscores or
 * raises a parse error if used with is/as expressions
 *)
and fixup_type_arg (env : Emit_env.t) ~isas (hint : Aast.hint): Aast.hint =
  let erased_tparams = get_erased_tparams env in
  let rec aux (p, hint as _x : Aast.hint) =
    match hint with
    | Aast.Hoption h -> p, Aast.Hoption (aux h)
    | Aast.Hlike h -> p, Aast.Hlike (aux h)
    | Aast.Hfun (fr, ic, hl, pkl, pml, vh, h, mr) ->
      p, Aast.Hfun (fr, ic, List.map ~f:aux hl, pkl, pml, vh, aux h, mr)
    | Aast.Htuple hl -> p, Aast.Htuple (List.map ~f:aux hl)
    | Aast.Happly ((_, id), _)
      when List.mem ~equal:String.equal erased_tparams id ->
      if isas then
        Emit_fatal.raise_fatal_parse p
          "Erased generics are not allowed in is/as expressions"
      else
        (p, Aast.Happly ((p, "_"), []))
    | Aast.Happly (id, hl) -> p, Aast.Happly (id, List.map ~f:aux hl)
    | Aast.Hshape { Aast.nsi_allows_unknown_fields = uf; Aast.nsi_field_map = fm} ->
      p, Aast.Hshape { Aast.nsi_allows_unknown_fields = uf
                     ; Aast.nsi_field_map = List.map ~f:aux_sf fm
                  }
    | Aast.Haccess _ -> p, hint
    | Aast.Hsoft h -> p, Aast.Hsoft (aux h)
    | _ -> failwith "todo"
  and aux_sf sfi =
    { sfi with Aast.sfi_hint = aux sfi.Aast.sfi_hint } in
  aux hint

and emit_reified_arg env ~isas pos (hint : Aast.hint) =
  let hint = fixup_type_arg env ~isas hint in
  let scope = Emit_env.get_scope env in
  let f is_fun tparam acc =
    match tparam with
    | { A.tp_name = (_, id); A.tp_reified; _ } when not (tp_reified = A.Erased) -> SMap.add id is_fun acc
    | _ -> acc
  in
  let current_targs =
    List.fold_right (Ast_scope.Scope.get_fun_tparams scope)
      ~init:SMap.empty ~f:(f true)
  in
  let current_targs =
    List.fold_right (Ast_scope.Scope.get_class_tparams scope).A.c_tparam_list
      ~init:current_targs ~f:(f false)
  in
  let acc = ref (0, SMap.empty) in
  let visitor = object (_)
    inherit [_] A.iter as super
    method! on_hint_ _ h =
      let add_name name =
        let i, map = !acc in
        match SMap.get name current_targs with
        | Some _ when not (SMap.mem name map) ->
           acc := (i + 1, SMap.add name i map)
        | _ -> ()
      in
      match h with
      | A.Haccess (_, sids) ->
         List.iter sids (fun sid -> add_name (snd sid))
      | A.Happly ((_, name), h) ->
          add_name name;
          let _ = List.map ~f:(super#on_hint ()) h in
          ()
      | A.Habstr name ->
        add_name name
      | _ -> ();
      super#on_hint_ () h
  end in
  visitor#on_hint () hint;
  let count, targ_map = !acc in
  if count > 0 && not @@ reified_generics ()
  then Emit_fatal.raise_fatal_parse Pos.none "Reified generics are not allowed"
  else
  match snd hint with
  | Aast.Happly ((_, name), []) when SMap.mem name current_targs ->
    emit_reified_type env pos name, false
  | _ ->
    let ts = get_type_structure_for_hint env ~targ_map hint in
    let ts_list = if count = 0 then ts else
      (* Sort map from key 0 to count and convert each identified into cgetl *)
      let values =
        SMap.bindings targ_map
        |> List.sort ~compare:(fun (_, x) (_, y) -> Pervasives.compare x y)
        |> List.map ~f:(fun (v, _) -> emit_reified_type env pos v)
      in
      gather [
        gather values;
        ts;
      ]
    in
    gather [
      ts_list;
      instr_combine_and_resolve_type_struct (count + 1);
    ], (count = 0)

(* Emit arguments of a function call and inout setter for inout args *)
and emit_args_and_inout_setters env (args: A.expr list) =
  let aliases =
    if has_inout_args args
    then InoutLocals.collect_written_variables env args
    else SMap.empty in

  let emit_arg_and_inout_setter i (arg : A.expr) =
    match snd arg with
    (* inout $var *)
    | A.Callconv (Ast.Pinout, (_, A.Lvar (pos, id))) ->
      let local = get_local env (pos, (Local_id.get_name id)) in
      let not_in_try = not (Emit_env.is_in_try env) in
      let move_instrs =
        if not_in_try && (InoutLocals.should_move_local_value local aliases)
        then gather [ instr_null; instr_popl local ]
        else empty in
      gather [ instr_cgetl local; move_instrs ],
      instr_popl local
    (* inout $arr[...][...] *)
    | A.Callconv (Ast.Pinout, ((pos, _), A.Array_get (base_expr, opt_elem_expr))) ->
      let array_get_result =
        fst (emit_array_get_worker
          ~inout_param_info:(Some (i, aliases)) env pos
          QueryOp.InOut base_expr opt_elem_expr) in
      begin match array_get_result with
      | Array_get_regular instrs ->
        let setter_base =
          fst (emit_array_get ~no_final:true
            ~mode:MemberOpMode.Define
            env pos QueryOp.InOut base_expr opt_elem_expr) in
        let setter = gather [
          setter_base;
          instr_setm 0 (get_elem_member_key env 0 opt_elem_expr);
          instr_popc
        ] in
        instrs, setter
      | Array_get_inout { load; store } ->
        rebuild_load_store load store
      end
    (* unsupported inout *)
    | A.Callconv (Ast.Pinout, _) ->
      failwith "emit_arg_and_inout_setter: Unexpected inout expression type"

    (* by-ref annotated argument *)
    | A.Unop (Ast.Uref, expr) ->
      let pos = fst @@ fst expr in
      begin match snd expr with
      (* passed by reference *)
      | A.Array_get _ ->
        Emit_fatal.raise_fatal_parse pos
          "references of subscript expressions should not parse"
      | A.Class_get (cid, id) ->
        let env = { env with Emit_env.env_allows_array_append = true } in
        emit_class_get env QueryOp.CGet cid id
      | A.Lvar id ->
        emit_pos_then pos @@
        emit_local ~notice:Notice ~need_ref:true env id
      | A.Obj_get (obj, prop, nullflavor) ->
        let env = { env with Emit_env.env_allows_array_append = true } in
        fst (emit_obj_get ~arg_by_ref:true env pos QueryOp.Empty obj prop
          nullflavor)
      (* passed by value *)
      | _ -> emit_expr env expr
      end,
      empty

    (* regular argument *)
    | _ -> emit_expr env arg, empty
  in

  let rec aux i (args : A.expr list) = match args with
  | [] -> empty, empty
  | arg :: rem_args ->
    let this_arg, this_setter = emit_arg_and_inout_setter i arg in
    let rem_args, rem_setters = aux (i + 1) rem_args in
    gather [ this_arg; rem_args ],
    gather [ this_setter; rem_setters ]
  in

  let instr_args, instr_setters = aux 0 args in
  if has_inout_args args then
    let retval = Local.get_unnamed_local () in
    instr_args, gather [ instr_popl retval; instr_setters; instr_pushl retval ]
  else
    instr_args, empty

(* Create fcall_args for a given call *)
and get_fcall_args args uargs async_eager_label =
  let num_args = List.length args in
  let num_rets = List.fold_left args ~init:1
    ~f:(fun acc arg -> if is_inout_arg arg then acc + 1 else acc) in
  let flags = { default_fcall_flags with has_unpack = uargs <> [] } in
  let by_refs = List.map args expr_starts_with_ref in
  make_fcall_args ~flags ~num_rets ~by_refs ?async_eager_label num_args

(* Expression that appears in an object context, such as expr->meth(...) *)
and emit_object_expr env (expr : A.expr) =
  let (_, expr_) = expr in
  match expr_ with
  | A.Lvar(_, x) when is_local_this env x ->
    instr_this
  | _ -> emit_expr env expr

and is_inout_arg = function
  | _, A.Callconv (Ast.Pinout, _) -> true
  | _ -> false

and has_inout_args es =
  List.exists es ~f:is_inout_arg

and emit_call_lhs_and_fcall
  env (expr : A.expr) fcall_args (targs : Aast.targ list) inout_arg_positions =
  let ((pos, _), expr_) = expr in
  let has_inout_args = List.length inout_arg_positions <> 0 in
  let does_not_have_non_tparam_generics =
    not (has_non_tparam_generics env targs) in
  let reified_call_body name =
    gather [
      emit_reified_targs env pos targs;
      instr_reified_name name;
    ] in
  match expr_ with
  | A.Obj_get (obj, (_, A.String id), null_flavor)
  | A.Obj_get (obj, (_, A.Id (_, id)), null_flavor) ->
    let name = Hhbc_id.Method.from_ast_name id in
    let name =
      if has_inout_args
      then Hhbc_id.Method.add_suffix name
        (Emit_inout_helpers.inout_suffix inout_arg_positions)
      else name in
    let obj = emit_object_expr env obj in
    if does_not_have_non_tparam_generics then
      gather [ obj; instr_nulluninit; instr_nulluninit ],
      instr_fcallobjmethodd fcall_args name null_flavor
    else
      gather [ obj; instr_nulluninit; instr_nulluninit ],
      gather [
        emit_reified_targs env pos targs;
        instr_fcallobjmethodrd fcall_args name null_flavor
      ]
  | A.Obj_get (obj, method_expr, null_flavor) ->
    let obj = emit_object_expr env obj in
    let tmp = Local.get_unnamed_local () in
    gather [
      obj; instr_nulluninit; instr_nulluninit;
      emit_expr env method_expr;
      instr_popl tmp
    ],
    gather [
      instr_pushl tmp;
      instr_fcallobjmethod fcall_args null_flavor inout_arg_positions
    ]

  | A.Class_const (cid, (_, id)) ->
    let cexpr = class_id_to_class_expr ~resolve_self:false
      (Emit_env.get_scope env) cid in
    let method_id = Hhbc_id.Method.from_ast_name id in
    let method_id =
      if has_inout_args
      then Hhbc_id.Method.add_suffix method_id
        (Emit_inout_helpers.inout_suffix inout_arg_positions)
      else method_id in
    let method_id_string = Hhbc_id.Method.to_raw_string method_id in
    let cexpr = match cexpr with
      | Class_id (_, name) ->
        Option.value ~default:cexpr (get_reified_var_cexpr env pos name)
      | _ -> cexpr in
    begin match cexpr with
    (* Statically known *)
    | Class_id cid ->
      let fq_cid = Hhbc_id.Class.elaborate_id (Emit_env.get_namespace env) cid in
      let fq_cid_string = Hhbc_id.Class.to_raw_string fq_cid in
      Emit_symbol_refs.add_class fq_cid_string;
      if does_not_have_non_tparam_generics then
        gather [ instr_nulluninit; instr_nulluninit; instr_nulluninit ],
        instr_fcallclsmethodd fcall_args method_id fq_cid
      else
        gather [ instr_nulluninit; instr_nulluninit; instr_nulluninit ],
        gather [
          emit_reified_targs env pos targs;
          instr_fcallclsmethodrd fcall_args method_id fq_cid
        ]
    | Class_special clsref ->
      if does_not_have_non_tparam_generics then
        gather [ instr_nulluninit; instr_nulluninit; instr_nulluninit ],
        instr_fcallclsmethodsd fcall_args clsref method_id
      else
        gather [ instr_nulluninit; instr_nulluninit; instr_nulluninit ],
        gather [
          emit_reified_targs env pos targs;
          instr_fcallclsmethodsrd fcall_args clsref method_id
        ]
    | Class_expr expr ->
      let emit_fcall instr_meth = gather [
        instr_meth;
        emit_expr env expr;
        instr_clsrefgetc;
        instr_fcallclsmethod fcall_args []
      ] in
      if does_not_have_non_tparam_generics then
        gather [ instr_nulluninit; instr_nulluninit; instr_nulluninit ],
        emit_fcall (instr_string method_id_string)
      else
        gather [ instr_nulluninit; instr_nulluninit; instr_nulluninit ],
        emit_fcall (reified_call_body method_id_string)
    | Class_reified instrs ->
      (* TODO(T31677864): Implement reification here *)
      let tmp = Local.get_unnamed_local () in
      gather [
        instr_nulluninit; instr_nulluninit; instr_nulluninit;
        instrs; instr_popl tmp
      ],
      gather [
        instr_string method_id_string;
        instr_pushl tmp;
        instr_clsrefgetc;
        instr_fcallclsmethod fcall_args []
      ]
    end

  | A.Class_get (cid, e) ->
    let cexpr = class_id_to_class_expr ~resolve_self:false
                  (Emit_env.get_scope env) cid in
    let emit_meth_name () =
      match e with
      | A.CGstring (pos, id) ->
        emit_pos_then pos @@
        instr_cgetl (Local.Named id)
      | A.CGexpr e ->
        emit_expr env e in
    let cexpr = match cexpr with
      | Class_id (_, name) ->
        Option.value ~default:cexpr (get_reified_var_cexpr env pos name)
      | _ -> cexpr in
    begin match cexpr with
    | Class_id cid ->
      let tmp = Local.get_unnamed_local () in
      gather [
       instr_nulluninit; instr_nulluninit; instr_nulluninit;
       emit_meth_name (); instr_popl tmp
      ],
      gather [
        instr_pushl tmp;
        emit_known_class_id env cid;
        instr_fcallclsmethod fcall_args inout_arg_positions
      ]
    | Class_special clsref ->
      let tmp = Local.get_unnamed_local () in
      gather [
       instr_nulluninit; instr_nulluninit; instr_nulluninit;
       emit_meth_name (); instr_popl tmp
      ],
      gather [
        instr_pushl tmp;
        instr_fcallclsmethods fcall_args clsref
      ]
    | Class_expr expr ->
      let cls = Local.get_unnamed_local () in
      let meth = Local.get_unnamed_local () in
      gather [
        instr_nulluninit; instr_nulluninit; instr_nulluninit;
        emit_expr env expr;
        instr_popl cls;
        emit_meth_name ();
        instr_popl meth
      ],
      gather [
        instr_pushl meth;
        instr_pushl cls;
        instr_clsrefgetc;
        instr_fcallclsmethod fcall_args inout_arg_positions
      ]
    | Class_reified instrs ->
      let cls = Local.get_unnamed_local () in
      let meth = Local.get_unnamed_local () in
      gather [
        instr_nulluninit; instr_nulluninit; instr_nulluninit;
        instrs;
        instr_popl cls;
        emit_meth_name ();
        instr_popl meth
      ],
      gather [
        instr_pushl meth;
        instr_pushl cls;
        instr_clsrefgetc;
        instr_fcallclsmethod fcall_args inout_arg_positions
      ]
    end

  | A.Id (_, s as id) ->
    let fq_id =
      Hhbc_id.Function.elaborate_id_with_builtins (Emit_env.get_namespace env) id in
    let fq_id =
      let flags, num_args, _, _, _ = fcall_args in
      match SU.strip_global_ns s with
      | "min" when num_args = 2 && not flags.has_unpack ->
        Hhbc_id.Function.from_raw_string "__SystemLib\\min2"
      | "max" when num_args = 2 && not flags.has_unpack ->
        Hhbc_id.Function.from_raw_string  "__SystemLib\\max2"
      | _ -> fq_id in
    let fq_id = if has_inout_args
      then Hhbc_id.Function.add_suffix
        fq_id (Emit_inout_helpers.inout_suffix inout_arg_positions)
      else fq_id in
    if does_not_have_non_tparam_generics then
      gather [ instr_nulluninit; instr_nulluninit; instr_nulluninit ],
      instr_fcallfuncd fcall_args fq_id
    else
      gather [ instr_nulluninit; instr_nulluninit; instr_nulluninit ],
      gather [
        reified_call_body (Hhbc_id.Function.to_raw_string fq_id);
        instr_fcallfunc fcall_args inout_arg_positions
      ]
  | A.String s ->
    if does_not_have_non_tparam_generics then
      gather [ instr_nulluninit; instr_nulluninit; instr_nulluninit ],
      instr_fcallfuncd fcall_args (Hhbc_id.Function.from_raw_string s)
    else
      gather [ instr_nulluninit; instr_nulluninit; instr_nulluninit ],
      gather [
        reified_call_body s;
        instr_fcallfunc fcall_args inout_arg_positions
      ]
  | _ ->
    let tmp = Local.get_unnamed_local () in
    gather [
      instr_nulluninit; instr_nulluninit; instr_nulluninit;
      emit_expr env expr;
      instr_popl tmp
    ],
    gather [
      instr_pushl tmp;
      instr_fcallfunc fcall_args inout_arg_positions
    ]

and get_call_builtin_func_info lower_fq_id =
  match lower_fq_id with
  | "array_key_exists" -> Some (2, IMisc AKExists)
  | "hphp_array_idx" -> Some (3, IMisc ArrayIdx)
  | "intval" -> Some (1, IOp CastInt)
  | "boolval" -> Some (1, IOp CastBool)
  | "strval" -> Some (1, IOp CastString)
  | "floatval" | "doubleval" -> Some (1, IOp CastDouble)
  | "hh\\vec" -> Some (1, IOp CastVec)
  | "hh\\keyset" -> Some (1, IOp CastKeyset)
  | "hh\\dict" -> Some (1, IOp CastDict)
  | "hh\\varray" -> Some (1, IOp (if hack_arr_dv_arrs () then CastVec else CastVArray))
  | "hh\\darray" -> Some (1, IOp (if hack_arr_dv_arrs () then CastDict else CastDArray))
  | "hh\\global_empty" -> Some (1, IIsset EmptyG)
  | "hh\\global_get" -> Some (1, IGet CGetG)
  | "hh\\global_get_safe" -> Some (1, IGet CGetQuietG)
  | "hh\\global_isset" -> Some (1, IIsset IssetG)
  | _ -> None

(* TODO: work out what HHVM does special here *)
and emit_name_string env e =
  emit_expr env e

and emit_special_function env pos annot id (args : A.expr list) (uargs : A.expr list) default =
  let nargs = List.length args + List.length uargs in
  let fq_id =
    Hhbc_id.Function.elaborate_id_with_builtins (Emit_env.get_namespace env) (Pos.none, id) in
  (* Make sure that we do not treat a special function that is aliased as not
   * aliased *)
  let lower_fq_name =
    String.lowercase (Hhbc_id.Function.to_raw_string fq_id) in
  let hh_enabled = Emit_env.is_hh_syntax_enabled () in
  match lower_fq_name, args with
  | id, _ when id = SN.SpecialFunctions.echo ->
    let instrs = gather @@ List.mapi args begin fun i arg ->
         gather [
           emit_expr env arg;
           emit_pos pos;
           instr (IOp Print);
           if i = nargs-1 then empty else instr_popc
         ] end in
    Some instrs

  | "hh\\invariant", e::rest when hh_enabled ->
    let l = Label.next_regular () in
    let annot = (pos, snd annot) in
    let expr_id = annot, A.Id (pos, "\\hh\\invariant_violation") in
    Some (gather [
      (* Could use emit_jmpnz for better code *)
      emit_expr env e;
      instr_jmpnz l;
      emit_ignored_expr env (annot, A.Call (Aast.Cnormal, expr_id, [], rest, uargs));
      Emit_fatal.emit_fatal_runtime pos "invariant_violation";
      instr_label l;
      instr_null;
    ])

  | "assert", _ ->
    let l0 = Label.next_regular () in
    let l1 = Label.next_regular () in
    Some (gather [
      instr_string "zend.assertions";
      instr_fcallbuiltin 1 1 "ini_get";
      instr_int 0;
      instr_gt;
      instr_jmpz l0;
      default ();
      instr_jmp l1;
      instr_label l0;
      instr_true;
      instr_label l1;
    ])

  | ("class_exists" | "interface_exists" | "trait_exists" as id), arg1::_
    when nargs = 1 || nargs = 2 ->
    let class_kind =
      match id with
      | "class_exists" -> KClass
      | "interface_exists" -> KInterface
      | "trait_exists" -> KTrait
      | _ -> failwith "class_kind" in
    Some (gather [
      emit_name_string env arg1;
      instr (IOp CastString);
      if nargs = 1 then instr_true
      else gather [
        emit_expr env (List.nth_exn args 1);
        instr (IOp CastBool)
      ];
      instr (IMisc (OODeclExists class_kind))
    ])

  | ("exit" | "die"), _ when nargs = 0 || nargs = 1 ->
    Some (emit_exit env (List.hd args))

  | "hh\\fun", _ ->
    if nargs <> 1 then
      Emit_fatal.raise_fatal_runtime pos
        ("fun() expects exactly 1 parameter, " ^ (string_of_int nargs) ^
         " given")
    else begin match args with
      | [(_, A.String func_name)] ->
        let func_name = SU.strip_global_ns func_name in
        if Hhbc_options.emit_func_pointers !Hhbc_options.compiler_options
        then Some (instr_resolve_func @@ Hhbc_id.Function.from_raw_string func_name)
        else Some (instr_string func_name)
      | _ ->
        Emit_fatal.raise_fatal_runtime pos "Constant string expected in fun()"
    end

  | "hh\\inst_meth", _ ->
    begin match args with
      | [obj_expr; method_name] ->
        Some (gather [
          emit_expr env obj_expr;
          emit_expr env method_name;
          if Hhbc_options.emit_inst_meth_pointers !Hhbc_options.compiler_options
          then instr_resolve_obj_method
          else instr (ILitConst (NewVArray 2));
        ])
      | _ ->
        Emit_fatal.raise_fatal_runtime pos
          ("inst_meth() expects exactly 2 parameters, " ^
           (string_of_int nargs) ^ " given")
    end

  | "hh\\class_meth", _ ->
      begin match args with
        | [class_name; method_name] ->
          Some (gather [
            emit_expr env class_name;
            emit_expr env method_name;
            if Hhbc_options.emit_cls_meth_pointers !Hhbc_options.compiler_options
            then instr_resolve_cls_method
            else instr (ILitConst (NewVArray 2));
          ])
        | _ ->
          Emit_fatal.raise_fatal_runtime pos
            ("class_meth() expects exactly 2 parameters, " ^
             (string_of_int nargs) ^ " given")
      end

  | "hh\\global_set", _ ->
    begin match args with
      | [gkey; gvalue] -> Some (
        gather[
          emit_expr env gkey;
          emit_expr env gvalue;
          emit_pos pos;
          instr (IMutator SetG);
          instr_popc;
          instr_null
        ])
      | _ -> Emit_fatal.raise_fatal_runtime pos
            ("global_set() expects exactly 2 parameters, " ^
             (string_of_int nargs) ^ " given")
      end

  | "hh\\global_unset", _ ->
      begin match args with
      | [gkey] ->
        Some(gather [
          emit_expr env gkey;
          emit_pos pos;
          instr (IMutator UnsetG);
          instr_null
        ])
      | _ -> Emit_fatal.raise_fatal_runtime pos
            ("global_unset() expects exactly 1 parameter, " ^
             (string_of_int nargs) ^ " given")
      end

  | "__hhvm_internal_whresult", [_, A.Lvar (_, param)]
    when Emit_env.is_systemlib () ->
    Some (gather [
      instr_cgetl (Local.Named (Local_id.get_name param));
      instr_whresult;
    ])

  | "__hhvm_internal_newlikearrayl", [_, A.Lvar (_, param); _, A.Int n]
    when Emit_env.is_systemlib () ->
    Some (instr (ILitConst (NewLikeArrayL (Local.Named (Local_id.get_name param),
                                           int_of_string n))))

  | _ ->
    begin match args, istype_op lower_fq_name, is_isexp_op lower_fq_name with
    | [arg_expr], _, Some h when Emit_env.is_hh_syntax_enabled () ->
      (* T29079834:
       * Using this as a migration from is_{int,bool,etc} to is expressions *)
      Some (gather [
        emit_expr env arg_expr;
        emit_is env pos h
      ])
    | [(_, A.Lvar (_, arg_str as arg_id))], Some i, _
      when SN.Superglobals.is_superglobal (Local_id.get_name arg_str) ->
      Some (gather [
        emit_local ~notice:NoNotice ~need_ref:false env arg_id;
        emit_pos pos;
        instr (IIsset (IsTypeC i))
      ])
    | [(_, A.Lvar (arg_pos, arg_str))], Some i, _
      when not (is_local_this env arg_str) ->
      Some (instr (IIsset (IsTypeL (get_local env (arg_pos, (Local_id.get_name arg_str)), i))))
    | [arg_expr], Some i, _ ->
      Some (gather [
        emit_expr env arg_expr;
        emit_pos pos;
        instr (IIsset (IsTypeC i))
      ])
    | _ ->
      begin match get_call_builtin_func_info lower_fq_name with
      | Some (nargs, i) when nargs = List.length args ->
        Some (
          gather [
          emit_exprs env args;
          emit_pos pos;
          instr i
        ])
      | _ -> None
      end
    end

and get_inout_arg_positions args =
  List.filter_mapi args
    ~f:(fun i -> function
          | _, A.Callconv (Ast.Pinout, _) -> Some i
          | _ -> None)

and emit_call env pos expr (targs: Aast.targ list) (args : A.expr list) (uargs : A.expr list) async_eager_label =
  let (annot, expr_) = expr in
  (match expr_ with
    | A.Id (_, s) -> Emit_symbol_refs.add_function s
    | _ -> ());
  let fcall_args = get_fcall_args args uargs async_eager_label in
  let inout_arg_positions = get_inout_arg_positions args in
  let num_uninit = List.length inout_arg_positions in
  let default () = Scope.with_unnamed_locals @@ fun () ->
    let instr_lhs, instr_fcall = emit_call_lhs_and_fcall
      env expr fcall_args targs inout_arg_positions in
    let instr_args, instr_inout_setters =
      emit_args_and_inout_setters env args in
    let instr_uargs = match uargs with
      | [] -> empty
      | uargs :: _ -> emit_expr env uargs
    in
    empty,
    gather [
      gather @@ List.init num_uninit ~f:(fun _ -> instr_nulluninit);
      instr_lhs;
      instr_args;
      instr_uargs;
      emit_pos pos;
      instr_fcall;
      instr_inout_setters
    ],
    empty in

  match expr_, args with
  | A.Id (_, id), _ ->
    let special_fn_opt = emit_special_function env pos annot id args uargs default in
    begin match special_fn_opt with
    | Some instrs -> instrs
    | None -> default ()
    end
  | _ -> default ()

and emit_final_member_op stack_index op mk =
  match op with
  | LValOp.Set -> instr (IFinal (SetM (stack_index, mk)))
  | LValOp.SetOp op -> instr (IFinal (SetOpM (stack_index, op, mk)))
  | LValOp.IncDec op -> instr (IFinal (IncDecM (stack_index, op, mk)))
  | LValOp.Unset -> instr (IFinal (UnsetM (stack_index, mk)))

and emit_final_local_op pos op lid =
  emit_pos_then pos @@
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

and emit_final_static_op cid (prop : A.class_get_expr) op =
  match op with
  | LValOp.Set -> instr (IMutator (SetS 0))
  | LValOp.SetOp op -> instr (IMutator (SetOpS (op, 0)))
  | LValOp.IncDec op -> instr (IMutator (IncDecS (op, 0)))
  | LValOp.Unset ->
     let pos = match prop with
       | A.CGexpr ((pos, _), _) -> pos
       | A.CGstring (pos, _) -> pos in
    let cid = text_of_class_id cid in
    let id = text_of_prop prop in
    Emit_fatal.emit_fatal_runtime pos
      ("Attempt to unset static property " ^ cid ^ "::" ^ id)

(* Given a local $local and a list of integer array indices i_1, ..., i_n,
 * generate code to extract the value of $local[i_n]...[i_1]:
 *   BaseL $local Warn
 *   Dim Warn EI:i_n ...
 *   Dim Warn EI:i_2
 *   QueryM 0 CGet EI:i_1
 *)
and emit_array_get_fixed last_usage local indices =
  let base, stack_count =
    if last_usage then gather [
      instr_pushl local;
      instr_basec 0 MemberOpMode.Warn;
    ], 1
    else instr_basel local MemberOpMode.Warn, 0 in
  let indices =
    gather @@ List.rev_mapi indices
      begin fun i ix ->
        let mk = MemberKey.EI (Int64.of_int ix) in
        if i = 0
        then instr (IFinal (QueryM (stack_count, QueryOp.CGet, mk)))
        else instr (IBase (Dim (MemberOpMode.Warn, mk)))
      end in
  gather [
    base;
    indices;
  ]

and can_use_as_rhs_in_list_assignment (expr : A.expr_) =
  let open Tast in
  match expr with
  | Call (_, (_, Id (_, s)), _, _, _) when String.lowercase s = "echo" ->
    false
  | Lvar _ | Array_get _ | Obj_get _ | Class_get _ | PU_atom _
  | Call _ | New _ | Record _ | Expr_list _ | Yield _ | Cast _ | Eif _
  | Array _ | Varray _ | Darray _ | Collection _ | Clone _ | Unop _
  | As _ | Await _ -> true
  | Pipe (_, _, (_, r))
  | Binop ((Ast.Eq None), (_, List _), (_, r)) ->
    can_use_as_rhs_in_list_assignment r
  | Binop (Ast.Plus, _, _) | Binop (Ast.QuestionQuestion, _, _)
  | Binop (Ast.Eq _, _, _) | Class_const _ -> true
  (* Everything below is false *)
  | This | Any | ValCollection _ | KeyValCollection _ | ImmutableVar _
  | Dollardollar _ | Lplaceholder _| Fun_id _| Method_id (_, _) | Method_caller (_, _)
  | Smethod_id (_, _) | Special_func _ | Pair (_, _) | Assert _ | Typename _
  | Binop _ | Shape _ | Null | True | False | Omitted | Id _
  | Int _ | Float _ | String _ | String2 _
  | PrefixedString _ | Yield_break | Yield_from _ | Suspend _
  | InstanceOf _ | Is _ | BracedExpr _ | ParenthesizedExpr _
  | Efun _ | Lfun _ | Xml _ | Unsafe_expr _
  | Import _ | Callconv _ | List _ -> false
  | PU_identifier _ ->
    failwith "TODO(T35357243): Pocket Universes syntax must be erased by now"

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
and emit_lval_op_list ?(last_usage=false) (env : Emit_env.t) (outer_pos : Pos.t) local (indices : int list) (expr : A.expr) =
  let is_ltr = php7_ltr_assign () in
  match snd expr with
  | A.List exprs ->
    let last_non_omitted =
      (* last usage of the local will happen when processing last non-omitted
         element in the list - find it *)
      if last_usage
      then begin
        if is_ltr
        then
          exprs
          |> List.foldi ~init:None
            ~f:(fun i acc (_, v) -> if v = A.Omitted then acc else Some i)
        (* in right-to-left case result list will be reversed
           so we need to find first non-omitted expression *)
        else
          exprs
          |> List.findi ~f:(fun _ (_, v) -> v <> A.Omitted)
          |> Option.map ~f:fst
      end
      else None in
    let lhs_instrs, set_instrs =
      List.mapi exprs (fun i expr ->
        emit_lval_op_list ~last_usage:(Some i = last_non_omitted)
          env outer_pos local (i::indices) expr)
      |> List.unzip in
    gather lhs_instrs,
    gather (if not is_ltr then List.rev set_instrs else set_instrs)
  | A.Omitted -> empty, empty
  | _ ->
    (* Generate code to access the element from the array *)
    let access_instrs =
      match local, indices with
      | Some local, _ :: _ -> emit_array_get_fixed last_usage local indices
      | Some local, [] ->
        if last_usage then instr_pushl local
        else instr_cgetl local
      | None, _ -> instr_null
    in
    (* Generate code to assign to the lvalue *)
    (* Return pair: side effects to initialize lhs + assignment *)
    let lhs_instrs, rhs_instrs, set_op =
      emit_lval_op_nonlist_steps env outer_pos LValOp.Set expr access_instrs 1 in
    let lhs = if is_ltr then empty else lhs_instrs in
    let rest =
      gather [
        if is_ltr then lhs_instrs else empty;
        rhs_instrs;
        set_op;
        instr_popc;
      ]
    in
    lhs, rest

and expr_starts_with_ref = function
  | _, A.Unop (Ast.Uref, _) -> true
  | _ -> false

(* Emit code for an l-value operation *)
and emit_lval_op ?(null_coalesce_assignment=false) (env : Emit_env.t) pos op (expr1 : A.expr) opt_expr2 =
  match op, expr1, opt_expr2 with
  | LValOp.Set, _, Some e when expr_starts_with_ref e ->
    failwith "parser should not allow by-ref assignments"
  (* Special case for list destructuring, only on assignment *)
  | LValOp.Set, (_, A.List l), Some expr2 ->
    let instr_rhs = emit_expr env expr2 in
    let has_elements = List.exists l ~f: (function
     | _, A.Omitted -> false
     | _ -> true)
    in
    if not has_elements then instr_rhs else
    Scope.with_unnamed_local @@ fun local ->
    let loc = if can_use_as_rhs_in_list_assignment (snd expr2)
     then Some local else None in
    let instr_lhs, instr_assign = emit_lval_op_list env pos loc [] expr1 in
    (* before *)
    gather [ instr_lhs; instr_rhs; instr_popl local ],
    (* innner *)
    instr_assign,
    (* after *)
    instr_pushl local
  | _ ->
    Local.scope @@ fun () ->
    let rhs_instrs, rhs_stack_size =
      match opt_expr2 with
      | None -> empty, 0
      | Some (_, A.Yield af) ->
        let temp = Local.get_unnamed_local () in
        gather [
          emit_yield env pos af;
          instr_setl temp;
          instr_popc;
          instr_pushl temp;
        ], 1
      | Some ((pos, _), A.Unop (Ast.Uref, (_, A.Obj_get (_, _, Ast.OG_nullsafe)
                                  | _, A.Array_get ((_,
                                    A.Obj_get (_, _, Ast.OG_nullsafe)), _)))) ->
        Emit_fatal.raise_fatal_runtime
          pos "?-> is not allowed in write context"
      | Some e -> emit_expr env e, 1
    in
    emit_lval_op_nonlist ~null_coalesce_assignment env pos op expr1 rhs_instrs rhs_stack_size;

and emit_lval_op_nonlist ?(null_coalesce_assignment=false) env pos op e rhs_instrs rhs_stack_size =
  let (lhs, rhs, setop) =
    emit_lval_op_nonlist_steps ~null_coalesce_assignment env pos op e rhs_instrs rhs_stack_size
  in
  gather [
    lhs;
    rhs;
    setop;
  ]

and emit_lval_op_nonlist_steps ?(null_coalesce_assignment=false)
  (env : Emit_env.t) outer_pos op (expr : A.expr) rhs_instrs rhs_stack_size =
  let ((pos, _), expr_) = expr in
  let env =
  match op with
  (* Unbelieveably, $test[] += 5; is legal in PHP, but $test[] = $test[] + 5 is not *)
  | LValOp.Set
  | LValOp.SetOp _
  | LValOp.IncDec _ -> { env with Emit_env.env_allows_array_append = true }
  | _ -> env in
  match expr_ with
  | A.Lvar (name_pos, id) when SN.Superglobals.is_superglobal (Local_id.get_name id) ->
    emit_pos_then name_pos @@ instr_string @@ SU.Locals.strip_dollar (Local_id.get_name id),
    rhs_instrs,
    emit_final_global_op outer_pos op

  | A.Lvar ((_, str) as id) when is_local_this env str && is_incdec op ->
    emit_local ~notice:Notice ~need_ref:false env id,
    rhs_instrs,
    empty

  | A.Lvar (pos, name) when not (is_local_this env name) || op = LValOp.Unset ->
    empty,
    rhs_instrs,
    emit_final_local_op outer_pos op (get_local env (pos, (Local_id.get_name name)))

  | A.Array_get ((_, A.Lvar (_, x)), Some e) when (Local_id.get_name x) = SN.Superglobals.globals ->
    let final_global_op_instrs = emit_final_global_op pos op in
    if rhs_stack_size = 0
    then
      emit_expr env e,
      empty,
      final_global_op_instrs
    else
      let index_instrs, under_top = emit_first_expr env e in
      if under_top
      then
        empty,
        gather [
          rhs_instrs;
          index_instrs
        ],
        final_global_op_instrs
      else
        index_instrs,
        rhs_instrs,
        final_global_op_instrs
  | A.Array_get (_, None) when not (Emit_env.does_env_allow_array_append env) ->
      Emit_fatal.raise_fatal_runtime pos "Can't use [] for reading"
  | A.Array_get (base_expr, opt_elem_expr) ->
    let mode =
      match op with
      | LValOp.Unset -> MemberOpMode.Unset
      | _ -> MemberOpMode.Define in
    let elem_expr_instrs, elem_stack_size =
      emit_elem_instrs ~local_temp_kind:None ~null_coalesce_assignment env opt_elem_expr in
    let elem_expr_instrs =
      if null_coalesce_assignment then empty else elem_expr_instrs in
    let base_offset = elem_stack_size + rhs_stack_size in
    let base_expr_instrs_begin,
        base_expr_instrs_end,
        base_setup_instrs,
        base_stack_size =
      emit_base ~is_object:false ~notice:Notice ~null_coalesce_assignment env
        mode base_offset base_expr
    in
    let mk = get_elem_member_key ~null_coalesce_assignment env rhs_stack_size opt_elem_expr in
    let total_stack_size = elem_stack_size + base_stack_size in
    let final_instr =
      emit_pos_then pos @@
      emit_final_member_op total_stack_size op mk in
    gather [
      if null_coalesce_assignment then empty else base_expr_instrs_begin;
      elem_expr_instrs;
      base_expr_instrs_end;
    ],
    rhs_instrs,
    gather [
      emit_pos pos;
      base_setup_instrs;
      final_instr
    ]

  | A.Obj_get (e1, e2, null_flavor) ->
    if null_flavor = Ast.OG_nullsafe then
     Emit_fatal.raise_fatal_parse pos "?-> is not allowed in write context";
    let mode =
      match op with
      | LValOp.Unset -> MemberOpMode.Unset
      | _ -> MemberOpMode.Define in
    let mk, prop_expr_instrs, prop_stack_size =
      emit_prop_expr ~null_coalesce_assignment env null_flavor rhs_stack_size e2 in
    let prop_expr_instrs =
      if null_coalesce_assignment then empty else prop_expr_instrs in
    let base_offset = prop_stack_size + rhs_stack_size in
    let base_expr_instrs_begin,
        base_expr_instrs_end,
        base_setup_instrs,
        base_stack_size =
      emit_base
        ~notice:Notice ~is_object:true ~null_coalesce_assignment
        env mode base_offset e1
    in
    let total_stack_size = prop_stack_size + base_stack_size in
    let final_instr =
      emit_pos_then pos @@
      emit_final_member_op total_stack_size op mk in
    gather [
      if null_coalesce_assignment then empty else base_expr_instrs_begin;
      prop_expr_instrs;
      base_expr_instrs_end;
    ],
    rhs_instrs,
    gather [
      base_setup_instrs;
      final_instr
    ]

  | A.Class_get (cid, prop) ->
    let cexpr = class_id_to_class_expr ~resolve_self:false
      (Emit_env.get_scope env) cid in
    let final_instr =
      emit_pos_then pos @@
      emit_final_static_op cid prop op in
    of_pair @@ emit_class_expr env cexpr prop,
    rhs_instrs,
    final_instr

  | A.Unop (uop, e) ->
    empty,
    rhs_instrs,
    gather [
      emit_lval_op_nonlist env pos op e empty rhs_stack_size;
      from_unop uop
    ]

  | _ ->
    Emit_fatal.raise_fatal_parse pos "Can't use return value in write context"

and from_unop op =
  match op with
  | Ast.Utild -> instr (IOp BitNot)
  | Ast.Unot -> instr (IOp Not)
  | Ast.Uplus -> instr (IOp Add)
  | Ast.Uminus -> instr (IOp Sub)
  | Ast.Uincr | Ast.Udecr | Ast.Upincr | Ast.Updecr | Ast.Uref | Ast.Usilence ->
    failwith "this unary operation cannot be translated"

and emit_unop env pos op e =
  match op with
  | Ast.Utild ->
    gather [
      emit_expr env e;
      emit_pos_then pos @@ from_unop op
    ]
  | Ast.Unot ->
    gather [
      emit_expr env e;
      emit_pos_then pos @@ from_unop op
    ]
  | Ast.Uplus ->
    gather [
      emit_pos pos;
      instr (ILitConst (Int (Int64.zero)));
      emit_expr env e;
      emit_pos_then pos @@ from_unop op
    ]
  | Ast.Uminus ->
    gather [
      emit_pos pos;
      instr (ILitConst (Int (Int64.zero)));
      emit_expr env e;
      emit_pos_then pos @@ from_unop op
    ]
  | Ast.Uincr | Ast.Udecr | Ast.Upincr | Ast.Updecr ->
    emit_lval_op env pos (LValOp.IncDec (unop_to_incdec_op op)) e None
  | Ast.Uref ->
    failwith "Ast.Uref is used only for argument passing"
  | Ast.Usilence ->
    Local.scope @@ fun () ->
      let temp_local = Local.get_unnamed_local () in
      let done_label = Label.next_regular () in
      gather [
        emit_pos pos;
        instr_silence_start temp_local;
        instr_try_catch_begin;
        emit_expr env e;
        instr_jmp done_label;
        instr_try_catch_middle;
        emit_pos pos;
        instr_silence_end temp_local;
        instr_throw;
        instr_try_catch_end;
        instr_label done_label;
        emit_pos pos;
        instr_silence_end temp_local
      ]

and emit_exprs env (exprs : A.expr list) =
  match exprs with
  | [] -> empty
  | expr::exprs ->
    gather (emit_expr env expr ::
      List.map exprs (emit_expr env))
