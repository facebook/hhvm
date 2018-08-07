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
open Instruction_sequence
open Ast_class_expr
open Emit_pos

module A = Ast
module H = Hhbc_ast
module TC = Hhas_type_constraint
module SN = Naming_special_names
module SU = Hhbc_string_utils
module ULS = Unique_list_string
module Opts = Hhbc_options

let inline_hhas_blocks_: (Hhas_asm.t SMap.t) ref = ref SMap.empty
let set_inline_hhas_blocks s = inline_hhas_blocks_ := s

let can_inline_gen_functions () =
  let opts = !Opts.compiler_options in
  Emit_env.is_hh_syntax_enabled () &&
  (Opts.enable_hiphop_syntax opts) &&
  (Opts.can_inline_gen_functions opts) &&
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
  | SetRef
  | SetOp of eq_op
  | IncDec of incdec_op
  | Unset
end

let jit_enable_rename_function () =
  Hhbc_options.jit_enable_rename_function !Hhbc_options.compiler_options

let is_local_this env id =
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
    { first_inout = max_int; last_write = min_int; num_uses = 0; }

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

  let add_write name i m = update name i add_write m
  let add_inout name i m = update name i add_inout m
  let add_use name i m = update name i add_use m

  let collect_written_variables env args =
    (* check value of the argument *)
    let rec handle_arg ~is_top i acc arg =
      match snd arg with
      (* inout $v *)
      | A.Callconv (A.Pinout, (_, A.Lvar (_, id)))
        when not (is_local_this env id) ->
        let acc = add_use id i acc in
        if is_top then add_inout id i acc else add_write id i acc
      (* &$v *)
      | A.Unop (A.Uref, (_, A.Lvar (_, id))) ->
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
    and collect_lvars_lhs i acc e =
      match snd e with
      | A.Lvar (_, id) when not (is_local_this env id) ->
        let acc = add_use id i acc in
        add_write id i acc
      | A.List exprs ->
        List.fold_left exprs ~f:(collect_lvars_lhs i) ~init:acc
      | _ -> acc

    (* descend into expression *)
    and dive i acc expr =
      let visitor = object(_)
        inherit [_] Ast_visitor.ast_visitor as super
        (* lhs op= _ *)
        method! on_binop acc bop l r =
          let acc =
            match bop with
            | A.Eq _ -> collect_lvars_lhs i acc l
            | _ -> acc in
          super#on_binop acc bop l r
        (* $i++ or $i-- *)
        method! on_unop acc op e =
          let acc =
            match op with
            | A.Uincr | A.Udecr -> collect_lvars_lhs i acc e
            | _ -> acc in
          super#on_unop acc op e
        (* $v *)
        method! on_lvar acc (_p, id) =
          let acc = add_use id 0 acc in
          super#on_lvar acc (_p, id)
        (* f(inout $v) or f(&$v) *)
        method! on_call acc _ _ args uargs =
          let f = handle_arg ~is_top:false i in
          let acc = List.fold_left args ~init:acc ~f in
          List.fold_left uargs ~init:acc ~f
        end in
      visitor#on_expr acc expr in
    List.foldi args ~f:(handle_arg ~is_top:true) ~init:SMap.empty

  (* determines if value of a local 'name' that appear in parameter 'i'
     should be saved to local because it might be overwritten later *)
  let should_save_local_value name i aliases =
    Option.value_map ~default:false ~f:(in_range i) (SMap.get name aliases)
  let should_move_local_value name aliases =
    Option.value_map ~default:true ~f:has_single_ref (SMap.get name aliases)
end

(* Describes what kind of value is intended to be stored in local *)
type stored_value_kind =
  | Value_kind_local
  | Value_kind_expression

(* represents sequence of instructions interleaved with temp locals.
   <i, None :: rest> - is emitted i :: <rest> (commonly used for final instructions in sequence)
   <i, Some (l, local_kind) :: rest> is emitted as

   i
   try-fault F {
     setl/popl l; depending on local_kind
     <rest>
   }
   unsetl l
   F: unset l
      unwind
   *)
type instruction_sequence_with_locals =
  (Instruction_sequence.t * (Local.t * stored_value_kind) option) list

(* converts instruction_sequence_with_locals to instruction_sequence.t *)
let rebuild_sequence s rest =
  let rec aux = function
  | [] -> rest ()
  | (i, None) :: xs -> gather [ i; aux xs ]
  | (i, Some (l, kind)) :: xs ->
    let fault_label = Label.next_fault () in
    let unset = instr_unsetl l in
    let set = if kind = Value_kind_expression then instr_setl l else instr_popl l in
    let try_block = gather [
      set;
      aux xs;
    ] in
    let fault_block = gather [ unset; instr_unwind; ] in
    gather [
      i;
      instr_try_fault fault_label try_block fault_block;
      unset;  ] in
  aux s

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

let is_special_function env e args =
  match snd e with
  | A.Id (_, s) ->
  begin
    let n = List.length args in
    match String.lowercase_ascii s with
    | "isset" -> n > 0
    | "__hhas_adata"
    | "empty" -> n = 1
    | "define" when is_global_namespace env ->
      begin match args with
      | [_, A.String _; _] -> true
      | _ -> false
      end
    | "eval" -> n = 1
    | "idx" -> not (jit_enable_rename_function ()) && (n = 2 || n = 3)
    | "class_alias" when is_global_namespace env ->
      begin
        match args with
        | [_, A.String _; _, A.String _]
        | [_, A.String _; _, A.String _; _] -> true
        | _ -> false
      end
   | _ -> false
  end
  | _ -> false

let optimize_null_check () =
  Hhbc_options.optimize_null_check !Hhbc_options.compiler_options

let hack_arr_compat_notices () =
  Hhbc_options.hack_arr_compat_notices !Hhbc_options.compiler_options

let hack_arr_dv_arrs () =
  Hhbc_options.hack_arr_dv_arrs !Hhbc_options.compiler_options

let php7_ltr_assign () =
  Hhbc_options.php7_ltr_assign !Hhbc_options.compiler_options

(* Strict binary operations; assumes that operands are already on stack *)
let from_binop op =
  let ints_overflow_to_ints =
    Hhbc_options.ints_overflow_to_ints !Hhbc_options.compiler_options in
  match op with
  | A.Plus -> instr (IOp (if ints_overflow_to_ints then Add else AddO))
  | A.Minus -> instr (IOp (if ints_overflow_to_ints then Sub else  SubO))
  | A.Star -> instr (IOp (if ints_overflow_to_ints then Mul else MulO))
  | A.Slash -> instr (IOp Div)
  | A.Eqeq -> instr (IOp Eq)
  | A.EQeqeq -> instr (IOp Same)
  | A.Starstar -> instr (IOp Pow)
  | A.Diff -> instr (IOp Neq)
  | A.Diff2 -> instr (IOp NSame)
  | A.Lt -> instr (IOp Lt)
  | A.Lte -> instr (IOp Lte)
  | A.Gt -> instr (IOp Gt)
  | A.Gte -> instr (IOp Gte)
  | A.Dot -> instr (IOp Concat)
  | A.Amp -> instr (IOp BitAnd)
  | A.Bar -> instr (IOp BitOr)
  | A.Ltlt -> instr (IOp Shl)
  | A.Gtgt -> instr (IOp Shr)
  | A.Cmp -> instr (IOp Cmp)
  | A.Percent -> instr (IOp Mod)
  | A.Xor -> instr (IOp BitXor)
  | A.LogXor -> instr (IOp Xor)
  | A.Eq _ -> failwith "assignment is emitted differently"
  | A.QuestionQuestion -> failwith "null coalescence is emitted differently"
  | A.AMpamp
  | A.BArbar ->
    failwith "short-circuiting operator cannot be generated as a simple binop"

let binop_to_eqop op =
  let ints_overflow_to_ints =
    Hhbc_options.ints_overflow_to_ints !Hhbc_options.compiler_options in
  match op with
  | A.Plus -> Some (if ints_overflow_to_ints then PlusEqual else PlusEqualO)
  | A.Minus -> Some (if ints_overflow_to_ints then MinusEqual else MinusEqualO)
  | A.Star -> Some (if ints_overflow_to_ints then MulEqual else MulEqualO)
  | A.Slash -> Some DivEqual
  | A.Starstar -> Some PowEqual
  | A.Amp -> Some AndEqual
  | A.Bar -> Some OrEqual
  | A.Xor -> Some XorEqual
  | A.Ltlt -> Some SlEqual
  | A.Gtgt -> Some SrEqual
  | A.Percent -> Some ModEqual
  | A.Dot -> Some ConcatEqual
  | _ -> None

let unop_to_incdec_op op =
  let ints_overflow_to_ints =
    Hhbc_options.ints_overflow_to_ints !Hhbc_options.compiler_options in
  match op with
  | A.Uincr -> if ints_overflow_to_ints then PreInc else PreIncO
  | A.Udecr -> if ints_overflow_to_ints then PreDec else PreDecO
  | A.Upincr -> if ints_overflow_to_ints then PostInc else PostIncO
  | A.Updecr -> if ints_overflow_to_ints then PostDec else PostDecO
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
  | _ -> None

(* T29079834: Using this for the is expressions migration *)
let is_isexp_op lower_fq_id =
  if not (Hhbc_options.enable_is_expr_primitive_migration !Hhbc_options.compiler_options)
  then None else
  let h n = Pos.none, A.Happly ((Pos.none, n), []) in
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

let extract_shape_field_name_pstring = function
  | A.SFlit_int s -> A.Int (snd s)
  | A.SFlit_str s ->
    Emit_type_constant.check_shape_key s;
    A.String (snd s)
  | A.SFclass_const ((pn, _) as id, p) -> A.Class_const ((pn, A.Id id), p)

let rec text_of_expr e = match e with
  (* Note we force string literals to become single-quoted, regardless of
     whether they were single- or double-quoted in the source. Gross. *)
  | p, A.String s -> (p, "'" ^ s ^ "'")
  | _, A.Id id | _, A.Lvar id -> id
  | _, A.Array_get ((p, A.Lvar (_, id)), Some e) ->
    (p, id ^ "[" ^ snd (text_of_expr e) ^ "]")
  | _ -> Pos.none, "unknown" (* TODO: get text of expression *)

let parse_include e =
  let strip_backslash p =
    let len = String.length p in
    if len > 0 && p.[0] = '/' then String.sub p 1 (len-1) else p in
  let rec split_var_lit = function
    | _, A.Binop (A.Dot, e1, e2) -> begin
      let v, l = split_var_lit e2 in
      if v = ""
      then let var, lit = split_var_lit e1 in var, lit ^ l
      else v, ""
    end
    | _, A.String lit -> "", lit
    | e -> snd (text_of_expr e), "" in
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
  | A.AFvalue e ->
    let add_instr =
      if expr_starts_with_ref e then instr_add_new_elemv else instr_to_add_new
    in
    gather [emit_expr ~need_ref:false env e; emit_pos pos; add_instr]
  | A.AFkvalue (k, v) ->
    let add_instr =
      if expr_starts_with_ref v then instr_add_elemv else instr_to_add
    in
    gather [
      emit_two_exprs env (fst k) k v;
      add_instr;
    ]

and get_local env (pos, str) =
  if str = SN.SpecialIdents.dollardollar
  then
    match Emit_env.get_pipe_var env with
    | None -> Emit_fatal.raise_fatal_runtime pos
      "Pipe variables must occur only in the RHS of pipe expressions"
    | Some v -> v
  else Local.Named str

and check_non_pipe_local e =
  match e with
  | _, A.Lvar (pos, str) when str = SN.SpecialIdents.dollardollar ->
    Emit_fatal.raise_fatal_parse pos
      "Cannot take indirect reference to a pipe variable"
  | _ -> ()

(*
and get_non_pipe_local (pos, str) =
  if str = SN.SpecialIdents.dollardollar
  then Emit_fatal.raise_fatal_parse pos
    "Cannot take indirect reference to a pipe variable"
  else Local.Named str
*)

and emit_local ~notice ~need_ref env ((pos, str) as id) =
  if SN.Superglobals.is_superglobal str
  then gather [
    instr_string (SU.Locals.strip_dollar str);
    emit_pos pos;
    instr (IGet (if need_ref then VGetG else CGetG))
  ]
  else
  let local = get_local env id in
  if is_local_this env str && not (Emit_env.get_needs_local_this env) then
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
and emit_first_expr env expr =
  match snd expr with
  | A.Lvar ((_, name) as id)
    when not ((is_local_this env name && not (Emit_env.get_needs_local_this env))
      || SN.Superglobals.is_superglobal name) ->
    instr_cgetl2 (get_local env id), true
  | _ ->
    emit_expr ~need_ref:false env expr, false

(* Special case for binary operations to make use of CGetL2 *)
and emit_two_exprs env outer_pos e1 e2 =
  let instrs1, is_under_top = emit_first_expr env e1 in
  let instrs2 = emit_expr ~need_ref:false env e2 in
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

and emit_is_null env e =
  match e with
  | (_, A.Lvar ((_, str) as id)) when not (is_local_this env str) ->
    instr_istypel (get_local env id) OpNull
  | _ ->
    gather [
      emit_expr ~need_ref:false env e;
      instr_istypec OpNull
    ]

and emit_binop ~need_ref env pos op e1 e2 =
  let default () =
    emit_box_if_necessary pos need_ref @@ gather [
      emit_two_exprs env pos e1 e2;
      from_binop op
    ] in
  match op with
  | A.AMpamp | A.BArbar ->
    emit_box_if_necessary pos need_ref @@
      emit_short_circuit_op env pos (A.Binop (op, e1, e2))
  | A.Eq None ->
    emit_lval_op ~need_ref env pos LValOp.Set e1 (Some e2)
  | A.Eq (Some A.QuestionQuestion) ->
    emit_box_if_necessary pos need_ref @@
      emit_null_coalesce_assignment ~need_ref env pos e1 e2
  | A.Eq (Some obop) ->
    begin match binop_to_eqop obop with
    | None -> failwith "illegal eq op"
    | Some op -> emit_lval_op ~need_ref env pos (LValOp.SetOp op) e1 (Some e2)
    end
  | A.QuestionQuestion ->
    emit_box_if_necessary pos need_ref @@
      let end_label = Label.next_regular () in
      gather [
        fst (emit_quiet_expr env pos e1);
        instr_dup;
        instr_istypec OpNull;
        instr_not;
        instr_jmpnz end_label;
        instr_popc;
        emit_expr ~need_ref:false env e2;
        instr_label end_label;
      ]
  | _ ->
    if not (optimize_null_check ())
    then default ()
    else
    match op with
    | A.EQeqeq when snd e2 = A.Null ->
      emit_box_if_necessary pos need_ref @@ emit_is_null env e1
    | A.EQeqeq when snd e1 = A.Null ->
      emit_box_if_necessary pos need_ref @@ emit_is_null env e2
    | A.Diff2 when snd e2 = A.Null ->
      emit_box_if_necessary pos need_ref @@ gather [
        emit_is_null env e1;
        instr_not
      ]
    | A.Diff2 when snd e1 = A.Null ->
      emit_box_if_necessary pos need_ref @@ gather [
        emit_is_null env e2;
        instr_not
      ]
    | _ ->
      default ()

and emit_box_if_necessary pos need_ref instr =
  if need_ref then
    gather [
      instr;
      emit_pos pos;
      instr_box
    ]
  else
    instr

and emit_instanceof env pos e1 e2 =
  match (e1, e2) with
  | (_, (_, A.Id _)) ->
    let lhs = emit_expr ~need_ref:false env e1 in
    let from_class_ref instrs =
      gather [
        lhs;
        emit_pos pos;
        instrs;
        instr_instanceof;
      ] in
    let scope = Emit_env.get_scope env in
    begin match expr_to_class_expr ~resolve_self:true scope e2 with
    | Class_static ->
      from_class_ref @@ gather [
        instr_fcallbuiltin 0 0 "get_called_class";
        instr_unboxr_nop;
      ]
    | Class_parent ->
      from_class_ref @@ gather [
        instr_parent;
        instr_clsrefname;
      ]
    | Class_self ->
      from_class_ref @@ gather [
        instr_self;
        instr_clsrefname;
      ]
    | Class_id name ->
      let n, _ =
        Hhbc_id.Class.elaborate_id (Emit_env.get_namespace env) name in
      gather [
        lhs;
        instr_instanceofd n;
      ]
    | Class_expr _
    | Class_unnamed_local _ ->
      failwith "cannot get this shape from from A.Id"
    end
  | _ ->
    gather [
      emit_expr ~need_ref:false env e1;
      emit_expr ~need_ref:false env e2;
      instr_instanceof ]

and emit_as env pos e h is_nullable =
  if is_nullable then begin
    Local.scope @@ fun () ->
      let local = Local.get_unnamed_local () in
      let true_label = Label.next_regular () in
      let done_label = Label.next_regular () in
      gather [
        emit_expr ~need_ref:false env e;
        instr_setl local;
        (* (e is h) ? e : null *)
        emit_is env pos h;
        instr_jmpnz true_label;
        instr_null;
        instr_unsetl local;
        instr_jmp done_label;
        instr_label true_label;
        instr_pushl local;
        instr_label done_label;
      ]
  end else begin
    let namespace = Emit_env.get_namespace env in
    let tv = Emit_type_constant.hint_to_type_constant
      ~tparams:[] ~namespace h in
    gather [
      emit_expr ~need_ref:false env e;
      instr_astypestruct @@ Emit_adata.get_array_identifier tv
    ] end

and emit_is env _pos h =
  let namespace = Emit_env.get_namespace env in
  let ts = Emit_type_constant.hint_to_type_constant
    ~tparams:[] ~namespace h in
  instr_istypestruct @@ Emit_adata.get_array_identifier ts

and emit_cast env pos hint expr =
  let op =
    begin match hint with
    | A.Happly((_, id), []) ->
      let id = String.lowercase_ascii id in
      begin match id with
      | _ when id = SN.Typehints.int
            || id = SN.Typehints.integer -> instr (IOp CastInt)
      | _ when id = SN.Typehints.bool
            || id = SN.Typehints.boolean -> instr (IOp CastBool)
      | _ when id = SN.Typehints.string ||
               id = "binary" -> instr (IOp CastString)
      | _ when id = SN.Typehints.object_cast -> instr (IOp CastObject)
      | _ when id = SN.Typehints.array -> instr (IOp CastArray)
      | _ when id = SN.Typehints.real
            || id = SN.Typehints.double
            || id = SN.Typehints.float -> instr (IOp CastDouble)
      | _ when id = "unset" -> gather [ instr_popc; instr_null ]
      | _ -> Emit_fatal.raise_fatal_parse pos ("Invalid cast type: " ^ id)
      end
    | _ ->
      Emit_fatal.raise_fatal_parse pos "Invalid cast type"
    end in
  gather [
    emit_expr ~last_pos:pos ~need_ref:false env expr;
    emit_pos pos;
    op;
  ]

and emit_conditional_expression env pos etest etrue efalse =
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
        emit_expr ~need_ref:false env etrue;
        emit_pos pos;
        instr_jmp end_label
      ]
      else empty
      end;
      (* only emit false branch if false_label is used *)
      begin if r.is_label_used
      then gather [
        instr_label false_label;
        emit_expr ~need_ref:false env efalse;
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
      emit_expr ~last_pos:pos ~need_ref:false env etest;
      instr_dup;
      instr_jmpnz end_label;
      instr_popc;
      emit_expr ~need_ref:false env efalse;
      instr_label end_label;
    ]

and emit_new env pos expr args uargs =
  let nargs = List.length args + List.length uargs in
  let cexpr = expr_to_class_expr ~resolve_self:true
    (Emit_env.get_scope env) expr in
  match cexpr with
    (* Special case for statically-known class *)
  | Class_id id ->
    let fq_id, _id_opt =
      Hhbc_id.Class.elaborate_id (Emit_env.get_namespace env) id in
    Emit_symbol_refs.add_class (Hhbc_id.Class.to_raw_string fq_id);
    gather [
      emit_pos pos;
      instr_fpushctord nargs fq_id;
      emit_args_and_call env pos args uargs;
      instr_popr
      ]
  | Class_static ->
    gather [
      emit_pos pos;
      instr_fpushctors nargs SpecialClsRef.Static;
      emit_args_and_call env pos args uargs;
      instr_popr
      ]
  | Class_self ->
    gather [
      emit_pos pos;
      instr_fpushctors nargs SpecialClsRef.Self;
      emit_args_and_call env pos args uargs;
      instr_popr
      ]
  | Class_parent ->
    gather [
      emit_pos pos;
      instr_fpushctors nargs SpecialClsRef.Parent;
      emit_args_and_call env pos args uargs;
      instr_popr
      ]
  | _ ->
    gather [
      emit_load_class_ref env pos cexpr;
      instr_fpushctor nargs 0;
      emit_args_and_call env pos args uargs;
      instr_popr
    ]

and emit_new_anon env pos cls_idx args uargs =
  let nargs = List.length args + List.length uargs in
  gather [
    instr_defcls cls_idx;
    instr_fpushctori nargs cls_idx;
    emit_args_and_call env pos args uargs;
    instr_popr
    ]

and emit_clone env expr =
  gather [
    emit_expr ~need_ref:false env expr;
    instr_clone;
  ]

and emit_shape env expr fl =
  let p = fst expr in
  let fl =
    List.map fl
             ~f:(fun (fn, e) ->
                   ((p, extract_shape_field_name_pstring fn), e))
  in
  emit_expr ~need_ref:false env (p, A.Darray fl)

and emit_call_expr ?last_pos ~need_ref env expr =
  let instrs, flavor = emit_flavored_expr env expr in
  let pos = Option.value ~default:(fst expr) last_pos in
  gather [
    instrs;
    (* Box/unbox as needed *)
    match need_ref, flavor with
    | false, Flavor.Ref -> emit_pos_then pos instr_unbox
    | false, Flavor.ReturnVal -> emit_pos_then pos instr_unboxr
    | true, Flavor.Cell -> emit_pos_then pos instr_box
    | true, Flavor.ReturnVal -> emit_pos_then pos instr_boxr
    | _ -> empty
  ]

and emit_known_class_id env id =
  let fq_id, _ = Hhbc_id.Class.elaborate_id (Emit_env.get_namespace env) id in
  Emit_symbol_refs.add_class (Hhbc_id.Class.to_raw_string fq_id);
  gather [
    instr_string (Hhbc_id.Class.to_raw_string fq_id);
    instr_clsrefgetc;
  ]

and emit_load_class_ref env pos cexpr =
  emit_pos_then pos @@
  match cexpr with
  | Class_static -> instr (IMisc (LateBoundCls 0))
  | Class_parent -> instr (IMisc (Parent 0))
  | Class_self -> instr (IMisc (Self 0))
  | Class_id id -> emit_known_class_id env id
  | Class_unnamed_local l -> instr (IGet (ClsRefGetL (l, 0)))
  | Class_expr expr ->
    begin match snd expr with
    | A.Lvar ((_, id) as pos_id)
      when id <> SN.SpecialIdents.this || (Emit_env.get_needs_local_this env) ->
      let local = get_local env pos_id in
      instr (IGet (ClsRefGetL (local, 0)))
    | _ ->
      gather [
        emit_pos pos;
        emit_expr ~need_ref:false env expr;
        instr_clsrefgetc
      ]
    end

and emit_load_class_const env pos cexpr id =
  (* TODO(T21932293): HHVM does not match Zend here.
   * Eventually remove this to match PHP7 *)
  match Ast_scope.Scope.get_class (Emit_env.get_scope env) with
  | Some cd when cd.A.c_kind = A.Ctrait
              && cexpr = Class_self
              && SU.is_class id ->
    emit_pos_then pos @@
    instr_string @@ SU.strip_global_ns @@ snd cd.A.c_name
  | _ ->
    let load_const =
      if SU.is_class id
      then instr (IMisc (ClsRefName 0))
      else instr (ILitConst (ClsCns (Hhbc_id.Const.from_ast_name id, 0)))
    in
    gather [
      emit_load_class_ref env pos cexpr;
      load_const
    ]

and emit_class_expr ?(null_coalesce_assignment=false) env cexpr prop =
  match cexpr with
  | Class_expr ((pos, (A.BracedExpr _ |
                     A.Dollar _ |
                     A.Call _ |
                     A.Lvar (_, "$this") |
                     A.Binop _ |
                     A.Class_get _)) as e) ->
    (* if class is stored as dollar or braced expression (computed dynamically)
       it needs to be stored in unnamed local and eventually cleaned.
       Here we don't use stash_in_local because shape of the code generated
       for class case is different (PopC / UnsetL is the part of try block) *)
    let cexpr_local =
      Local.scope @@ fun () -> emit_expr ~need_ref:false env e in
    empty,
    Local.scope @@ fun () ->
      let temp = Local.get_unnamed_local () in
      let instrs = emit_class_expr env (Class_unnamed_local temp) prop in
      let fault_label = Label.next_fault () in
      let block =
        instr_try_fault
          fault_label
          (* try block *)
          (gather [
            instr_popc;
            of_pair @@ instrs;
            instr_unsetl temp
          ])
          (* fault block *)
          (gather [
            instr_unsetl temp;
            emit_pos pos;
            instr_unwind ]) in
      gather [
        cexpr_local;
        instr_setl temp;
        block
      ]
  | _ ->
  let load_prop, load_prop_first =
    match prop with
    | pos, A.Id (_, id) ->
      emit_pos_then pos @@
      instr_string id, true
    | pos, A.Lvar (_, id) ->
      emit_pos_then pos @@
      instr_string (SU.Locals.strip_dollar id), true
    | _, A.Dollar (_, A.Lvar _ as e) ->
      emit_expr ~need_ref:false env e, false
      (* The outer dollar just says "class property" *)
    | _, A.Dollar e | e ->
      emit_expr ~need_ref:false env e, true
  in
  let load_cls_ref = emit_load_class_ref env (fst prop) cexpr in
  let load_prop = if null_coalesce_assignment then empty else load_prop in
  if load_prop_first then load_prop, load_cls_ref
  else load_cls_ref, load_prop

and emit_class_get env qop need_ref cid prop =
  let cexpr = expr_to_class_expr ~resolve_self:false
    (Emit_env.get_scope env) cid
  in
  gather [
    of_pair @@ emit_class_expr env cexpr prop;
    match qop with
    | QueryOp.CGet -> if need_ref then instr_vgets else instr_cgets
    | QueryOp.CGetQuiet -> failwith "emit_class_get: CGetQuiet"
    | QueryOp.Isset -> instr_issets
    | QueryOp.Empty -> instr_emptys
    | QueryOp.InOut -> failwith "emit_class_get: InOut"
  ]

(* Class constant <cid>::<id>.
 * We follow the logic for the Construct::KindOfClassConstantExpression
 * case in emitter.cpp
 *)
and emit_class_const env pos cid (_, id) =
  let cexpr = expr_to_class_expr ~resolve_self:true
    (Emit_env.get_scope env) cid in
  match cexpr with
  | Class_id cid ->
    emit_class_const_impl env cid id
  | _ ->
    emit_load_class_const env pos cexpr id

and emit_class_const_impl env cid id =
  let fq_id, _id_opt =
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
      emit_expr ~last_pos:pos ~need_ref:false env e;
      emit_pos pos;
      instr_yield;
    ]
  | A.AFkvalue (e1, e2) ->
    gather [
      emit_expr ~last_pos:pos ~need_ref:false env e1;
      emit_expr ~need_ref:false env e2;
      emit_pos pos;
      instr_yieldk;
    ]

and emit_execution_operator env pos exprs =
  let instrs =
    match exprs with
    (* special handling of ``*)
    | [_, A.String "" as e] -> emit_expr ~need_ref:false env e
    | _ ->  emit_string2 env pos exprs in
  gather [
    instr_fpushfuncd 1 (Hhbc_id.Function.from_raw_string "shell_exec");
    instrs;
    instr_fcall 1 false 1;
  ]

and emit_string2 env pos exprs =
  match exprs with
  | [e] ->
    gather [
      emit_expr ~last_pos:pos ~need_ref:false env e;
      emit_pos pos;
      instr (IOp CastString)
    ]
  | e1::e2::es ->
    gather @@ [
      emit_two_exprs env (fst e1) e1 e2;
      emit_pos pos;
      instr (IOp Concat);
      gather (List.map es (fun e ->
        gather [emit_expr ~need_ref:false env e;
          emit_pos pos; instr (IOp Concat)]))
    ]

  | [] -> failwith "String2 with zero arguments is impossible"


and emit_lambda env fundef ids =
  (* Closure conversion puts the class number used for CreateCl in the "name"
   * of the function definition *)
  let fundef_name = snd fundef.A.f_name in
  let class_num = int_of_string fundef_name in
  let explicit_use = SSet.mem fundef_name (Emit_env.get_explicit_use_set ()) in
  gather [
    gather @@ List.map ids
      (fun (x, isref) ->
        instr (IGet (
          let lid = get_local env x in
          if explicit_use
          then
            if isref then VGetL lid else CGetL lid
          else CUGetL lid)));
    instr (IMisc (CreateCl (List.length ids, class_num)))
  ]

and emit_id env (p, s as id) =
  let s = String.uppercase_ascii s in
  match s with
  | "__FILE__" -> instr (ILitConst File)
  | "__DIR__" -> instr (ILitConst Dir)
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
    let fq_id, id_opt, contains_backslash =
      Hhbc_id.Const.elaborate_id (Emit_env.get_namespace env) id in
    begin match id_opt with
    | Some id ->
      Emit_symbol_refs.add_constant (Hhbc_id.Const.to_raw_string fq_id);
      Emit_symbol_refs.add_constant id;
      emit_pos_then p @@
      instr (ILitConst (CnsU (fq_id, id)))
    | None ->
      Emit_symbol_refs.add_constant (snd id);
      emit_pos_then p @@
      instr (ILitConst
        (if contains_backslash then CnsE fq_id else Cns fq_id))
    end

and rename_xhp (p, s) = (p, SU.Xhp.mangle s)

and emit_xhp env p id attributes children =
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
  let create_spread p id = (p, "...$" ^ string_of_int(id)) in
  let convert_attr (spread_id, attrs) = function
    | A.Xhp_simple (name, v) ->
        let attr = (A.SFlit_str name, v) in
        (spread_id, attr::attrs)
    | A.Xhp_spread e ->
        let (p, _) = e in
        let attr = (A.SFlit_str (create_spread p spread_id), e) in
        (spread_id + 1, attr::attrs) in
  let (_, attributes) = List.fold_left ~f:convert_attr ~init:(0, []) attributes in
  let attribute_map = p, A.Shape (List.rev attributes) in
  let children_vec = p, A.Varray children in
  let filename = p, A.Id (p, "__FILE__") in
  let line = p, A.Id (p, "__LINE__") in
  let renamed_id = rename_xhp id in
  Emit_symbol_refs.add_class (snd renamed_id);
  emit_expr ~need_ref:false env @@
    (p, A.New (
      (p, A.Id renamed_id),
      [],
      [attribute_map ; children_vec ; filename ; line],
      []))

and emit_import env pos flavor e =
  let inc = parse_include e in
  Emit_symbol_refs.add_include inc;
  let e, import_op = match flavor with
    | A.Include -> e, IIncludeEvalDefine Incl
    | A.Require -> e, IIncludeEvalDefine Req
    | A.IncludeOnce -> e, IIncludeEvalDefine InclOnce
    | A.RequireOnce ->
      let include_roots = Hhbc_options.include_roots !Hhbc_options.compiler_options in
      match Hhas_symbol_refs.resolve_to_doc_root_relative inc ~include_roots with
        | Hhas_symbol_refs.DocRootRelative path ->
          (pos, A.String path), IIncludeEvalDefine ReqDoc
        | _ -> e, IIncludeEvalDefine ReqOnce
  in
  gather [
    emit_expr ~need_ref:false env e;
    emit_pos pos;
    instr import_op;
  ]

and emit_call_isset_expr env outer_pos (pos, expr_ as expr) =
  match expr_ with
  | A.Array_get ((_, A.Lvar (_, x)), Some e) when x = SN.Superglobals.globals ->
    gather [
      emit_expr ~need_ref:false env e;
      emit_pos outer_pos;
      instr_issetg
    ]
  | A.Array_get (base_expr, opt_elem_expr) ->
    fst (emit_array_get ~need_ref:false env pos QueryOp.Isset base_expr opt_elem_expr)
  | A.Class_get (cid, id)  ->
    emit_class_get env QueryOp.Isset false cid id
  | A.Obj_get (expr, prop, nullflavor) ->
    fst (emit_obj_get ~need_ref:false env pos QueryOp.Isset expr prop nullflavor)
  | A.Lvar (_, n) when SN.Superglobals.is_superglobal n ->
    gather [
      emit_pos outer_pos;
      instr_string @@ SU.Locals.strip_dollar n;
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
  | A.Lvar id ->
    emit_pos_then outer_pos @@
    instr (IIsset (IssetL (get_local env id)))
  | A.Dollar e ->
    gather [
      emit_expr ~need_ref:false env e;
      instr_issetn
    ]
  | _ ->
    gather [
      emit_expr ~need_ref:false env expr;
      instr_istypec OpNull;
      instr_not
    ]

and emit_call_empty_expr env outer_pos (pos, expr_ as expr) =
  match expr_ with
  | A.Array_get((_, A.Lvar (_, x)), Some e) when x = SN.Superglobals.globals ->
    gather [
      emit_expr ~need_ref:false env e;
      emit_pos outer_pos;
      instr_emptyg
    ]
  | A.Array_get(base_expr, opt_elem_expr) ->
    fst (emit_array_get ~need_ref:false env pos QueryOp.Empty base_expr opt_elem_expr)
  | A.Class_get (cid, id) ->
    emit_class_get env QueryOp.Empty false cid id
  | A.Obj_get (expr, prop, nullflavor) ->
    fst (emit_obj_get ~need_ref:false env pos QueryOp.Empty expr prop nullflavor)
  | A.Lvar(_, id) when SN.Superglobals.is_superglobal id ->
    gather [
      instr_string @@ SU.Locals.strip_dollar id;
      emit_pos outer_pos;
      instr_emptyg
    ]
  | A.Lvar id ->
    if not (is_local_this env (snd id)) ||
      Emit_env.get_needs_local_this env
    then
      emit_pos_then outer_pos @@
      instr_emptyl (get_local env id)
    else
      gather [
        emit_pos pos;
        instr (IMisc (BareThis NoNotice));
        emit_pos outer_pos;
        instr_not
      ]
  | A.Dollar e ->
    gather [
      emit_expr ~last_pos:outer_pos ~need_ref:false env e;
      emit_pos outer_pos;
      instr_emptyn
    ]
  | _ ->
    gather [
      emit_expr ~need_ref:false env expr;
      instr_not
    ]

and emit_unset_expr env expr =
  emit_lval_op_nonlist env (fst expr) LValOp.Unset expr empty 0

and emit_call_isset_exprs env pos exprs =
  match exprs with
  | [] -> Emit_fatal.raise_fatal_parse
    pos "cannot call isset without any arguments"
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

and emit_exit env expr_opt =
  gather [
    (match expr_opt with
      | None -> instr_int 0
      | Some e -> emit_expr ~need_ref:false env e);
    instr_exit;
  ]

and emit_idx env pos es =
  let default = if List.length es = 2 then instr_null else empty in
  gather [
    emit_exprs env pos es;
    emit_pos pos;
    default;
    instr_idx;
  ]

and emit_define env pos s e =
  gather [
    emit_expr ~need_ref:false env e;
    emit_pos pos;
    instr_defcns s;
  ]

and emit_eval env pos e =
  gather [
    emit_expr ~need_ref:false env e;
    emit_pos pos;
    instr_eval;
  ]

and emit_xhp_obj_get_raw env pos e s nullflavor =
  let fn_name = pos, A.Obj_get (e, (pos, A.Id (pos, "getAttribute")), nullflavor) in
  let args = [pos, A.String (SU.Xhp.clean s)] in
  fst (emit_call env pos fn_name args [])

and emit_xhp_obj_get ~need_ref env pos e s nullflavor =
  gather [
    emit_xhp_obj_get_raw env pos e s nullflavor;
    emit_pos pos;
    if need_ref then instr_boxr else instr_unboxr
  ]

and emit_get_class_no_args () =
  gather [
    instr_fpushfuncd 0 (Hhbc_id.Function.from_raw_string "get_class");
    instr_fcall 0 false 1;
    instr_unboxr
  ]

and emit_class_alias es =
  let c1, c2 = match es with
    | (_, A.String c1) :: (_, A.String c2) :: _ -> c1, c2
    | _ -> failwith "emit_class_alias: impossible"
  in
  let default = if List.length es = 2 then instr_true else instr_string c2 in
  gather [
    default;
    instr_alias_cls c1 c2
  ]

and try_inline_gen_call env e =
  if not (can_inline_gen_functions ()) then None
  else match snd e with
  | A.Call ((_, A.Id (_, s)), _, [arg], [])
    when String.lowercase_ascii (SU.strip_global_ns s) = "gena"->
    Some (inline_gena_call env arg)
  | _ ->
    try_inline_genva_call env e GI_expression

and try_inline_genva_call env e inline_context =
  if not (can_inline_gen_functions ()) then None
  else match e with
  | pos, A.Call ((_, A.Id (_, s)), _, args, uargs)
    when String.lowercase_ascii (SU.strip_global_ns s) = "genva"->
    try_inline_genva_call_ env pos args uargs inline_context
  | _ -> None

and try_fault b f =
  let label = Label.next_fault () in
  let body = b () in
  let fault = f () in
  instr_try_fault label body fault

and unset_in_fault temps b =
  try_fault b @@ fun () ->
    gather [
      gather @@ List.map temps ~f:instr_unsetl;
      instr_unwind
    ]

(* emits iteration over the ~collection where loop body is
   produced by ~f *)
and emit_iter ~collection f = Local.scope @@ fun () ->
  let loop_end = Label.next_regular () in
  let key_local = Local.get_unnamed_local () in
  let value_local = Local.get_unnamed_local () in
  let iter = Iterator.get_iterator () in
  let iter_init = gather [
    collection;
    instr_iterinitk iter loop_end value_local key_local;
  ] in
  let loop_next = Label.next_regular () in
  let iterate =
    (* try-fault to release temp locals *)
    unset_in_fault [value_local; key_local] @@ begin fun () ->
      (* try-fault to release iterator *)
      try_fault
        begin fun () ->
          gather [
            instr_label loop_next;
            f value_local key_local;
            instr_iternextk iter loop_next value_local key_local;
            instr_label loop_end;
            instr_unsetl value_local;
            instr_unsetl key_local;
          ]
        end
        begin fun () ->
          gather [
            instr_iterfree iter;
            instr_unwind;
          ]
        end
      end in
  Iterator.free_iterator ();
  gather [
    iter_init;
    iterate;
  ]

and inline_gena_call env arg = Local.scope @@ fun () ->
  (* convert input to array *)
  let load_array = emit_expr ~need_ref:false env arg in
  let arr_local = Local.get_unnamed_local () in
  gather [
    load_array;
    if hack_arr_dv_arrs () then instr_cast_dict else instr_cast_darray;
    instr_setl arr_local;
    instr_popc;
    begin
      unset_in_fault [arr_local] @@ fun () ->
        gather [
          instr_fpushclsmethodd 1
            (Hhbc_id.Method.from_raw_string
               (if hack_arr_dv_arrs () then "fromDict" else "fromDArray"))
            (Hhbc_id.Class.from_raw_string "HH\\AwaitAllWaitHandle");
          instr_cgetl arr_local;
          instr_fcall 1 false 1;
          instr_unboxr;
          instr_await;
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
          end;
        ]
    end;
    instr_pushl arr_local;
  ]

and try_inline_genva_call_ env pos args uargs inline_context =
  let args_count = List.length args in
  let is_valid_list_assignment l =
    Core_list.findi l ~f:(fun i (_, x) -> i >= args_count && x <> A.Omitted)
    |> Option.is_none in
  let emit_list_assignment lhs rhs =
    let rec combine lhs rhs =
      (* ensure that list of values on left hand side and right hand size
         has the same length *)
      match lhs, rhs with
      | l :: lhs, r :: rhs -> (l, r) :: combine lhs rhs
      (* left hand size is smaller - pad with omitted expression *)
      | [], r :: rhs -> ((Pos.none, A.Omitted), r) :: combine [] rhs
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
    let reify = gather @@ Core_list.map rhs ~f:begin fun l ->
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
      gather @@ Core_list.map pairs
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
  Local.scope @@ begin fun () ->
  let load_args =
    gather @@ Core_list.map args ~f:begin fun arg ->
      emit_expr ~need_ref:false env arg
    end in
  let reserved_locals =
    List.init args_count (fun _ -> Local.get_unnamed_local ()) in
  let reserved_locals_reversed =
    List.rev reserved_locals in
  let init_locals =
    gather @@ Core_list.map reserved_locals_reversed ~f:begin fun l ->
      gather [
        instr_setl l;
        instr_popc;
      ]
    end in
  let await_and_process_results =
    unset_in_fault reserved_locals @@ begin fun () ->
      let await_all =
        gather [
          instr_awaitall (Some ((List.hd_exn reserved_locals_reversed), args_count));
          instr_popc;
        ] in
      let process_results =
        let reify ~pop_result =
          gather @@ Core_list.map reserved_locals ~f:begin fun l ->
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
      gather [
        await_all;
        process_results;
      ]
    end in
  let result =
    gather [
      load_args;
      init_locals;
      await_and_process_results;
    ] in
  Some result
  end

and emit_await env pos e =
  begin match try_inline_gen_call env e with
  | Some r -> r
  | None ->
    let after_await = Label.next_regular () in
    gather [
      emit_expr ~need_ref:false env e;
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
  | A.Pinout ->
    failwith "emit_callconv: This should have been caught at emit_arg"

and emit_inline_hhas s =
  match SMap.get s !inline_hhas_blocks_ with
  | Some asm ->
    let instrs =
      Label_rewriter.clone_with_fresh_regular_labels @@ Hhas_asm.instrs asm in
    (* TODO: handle case when code after inline hhas is unreachable
      i.e. fallthrough return should not be emitted *)
    begin match get_estimated_stack_depth instrs with
    | 0 -> gather [ instrs; instr_null ]
      | 1 -> instrs
    | _ ->
      Emit_fatal.raise_fatal_runtime Pos.none
        "Inline assembly expressions should leave the stack unchanged, \
        or push exactly one cell onto the stack."
    end
  | None ->
    failwith @@ "impossible: cannot find parsed inline hhas for '" ^ s ^ "'"

and emit_expr env ?last_pos ~need_ref (pos, expr_ as expr) =
  match expr_ with
  | A.Call ((_, A.Id (_, "__hhas_adata")), _, [ (_, A.String _) ], [])
  | A.Float _ | A.String _ | A.Int _ | A.Null | A.False | A.True ->
    let v = Ast_constant_folder.expr_to_typed_value (Emit_env.get_namespace env) expr in
    emit_pos_then pos @@
    emit_box_if_necessary pos need_ref @@
    instr (ILitConst (TypedValue v))
  | A.PrefixedString (_, e)
  | A.ParenthesizedExpr e ->
    emit_expr ~need_ref env e
  | A.Lvar id ->
    gather [
      emit_pos (Option.value ~default:pos last_pos);
      emit_local ~notice:Notice ~need_ref env id
    ]
  | A.Class_const (cid, id) ->
    emit_box_if_necessary pos need_ref @@ emit_class_const env pos cid id
  | A.Unop (op, e) ->
    emit_unop ~need_ref env pos op e
  | A.Binop (op, e1, e2) ->
    emit_binop ~need_ref env pos op e1 e2
  | A.Pipe (e1, e2) ->
    emit_box_if_necessary pos need_ref @@ emit_pipe env pos e1 e2
  | A.InstanceOf (e1, e2) ->
    emit_box_if_necessary pos need_ref @@ emit_instanceof env pos e1 e2
  | A.Is (e, h) ->
    emit_box_if_necessary pos need_ref @@ gather [
      emit_expr ~need_ref:false env e;
      emit_is env pos h;
    ]
  | A.As (e, h, is_nullable) ->
    emit_box_if_necessary pos need_ref @@ emit_as env pos e h is_nullable
  | A.Cast((_, hint), e) ->
    emit_box_if_necessary pos need_ref @@ emit_cast env pos hint e
  | A.Eif (etest, etrue, efalse) ->
    emit_box_if_necessary pos need_ref @@
      emit_conditional_expression env pos etest etrue efalse
  | A.Expr_list es -> gather @@ List.map es ~f:(emit_expr ~need_ref:false env)
  | A.Array_get((_, A.Lvar (_, x)), Some e) when x = SN.Superglobals.globals ->
    gather [
      emit_expr ~need_ref:false env e;
      emit_pos pos;
      instr (IGet (if need_ref then VGetG else CGetG))
    ]
  | A.Array_get(base_expr, opt_elem_expr) ->
    let query_op = if need_ref then QueryOp.Empty else QueryOp.CGet in
    fst (emit_array_get ~need_ref env pos query_op base_expr opt_elem_expr)
  | A.Obj_get (expr, prop, nullflavor) ->
    let query_op = if need_ref then QueryOp.Empty else QueryOp.CGet in
    fst (emit_obj_get ~need_ref env pos query_op expr prop nullflavor)

  | A.Call ((_, A.Id (_, id)), _, exprs, [])
    when String.lowercase_ascii id = "isset" ->
    emit_box_if_necessary pos need_ref @@ emit_call_isset_exprs env pos exprs
  | A.Call ((_, A.Id (_, id)), _, [expr], [])
    when String.lowercase_ascii id = "empty" ->
    emit_box_if_necessary pos need_ref @@ emit_call_empty_expr env pos expr
  | A.Call ((_, A.Id (_, id)), _, ([_; _] | [_; _; _] as es), _)
    when  String.lowercase_ascii id = "idx" && not (jit_enable_rename_function ()) ->
    emit_box_if_necessary pos need_ref @@ emit_idx env pos es
  | A.Call ((_, A.Id (_, id)), _, [(_, A.String s); e], _)
    when String.lowercase_ascii id = "define" && is_global_namespace env ->
    emit_box_if_necessary pos need_ref @@ emit_define env pos s e
  | A.Call ((_, A.Id (_, id)), _, [expr], _) when String.lowercase_ascii id = "eval" ->
    emit_box_if_necessary pos need_ref @@ emit_eval env pos expr
  | A.Call ((_, A.Id (_, "class_alias")), _, es, _)
    when is_global_namespace env ->
    emit_pos_then pos @@
    emit_box_if_necessary pos need_ref @@ emit_class_alias es
  | A.Call ((_, A.Id (_, "get_class")), _, [], _) ->
    emit_box_if_necessary pos need_ref @@ emit_get_class_no_args ()
  | A.Call ((_, A.Id (_, s)), _, es, _)
    when (String.lowercase_ascii s = "exit" || String.lowercase_ascii s = "die") ->
    emit_pos_then pos @@
    emit_exit env (List.hd es)
  | A.Call _
  (* execution operator is compiled as call to `shell_exec` and should
     be handled in the same way *)
  | A.Execution_operator _ ->
    emit_call_expr ?last_pos ~need_ref env expr
  | A.New (typeexpr, _, args, uargs) ->
    emit_box_if_necessary pos need_ref @@ emit_new env pos typeexpr args uargs
  | A.NewAnonClass (args, uargs, { A.c_name = (_, cls_name); _ }) ->
    let cls_idx = int_of_string cls_name in
    emit_box_if_necessary pos need_ref @@ emit_new_anon env pos cls_idx args uargs
  | A.Array es ->
    emit_pos_then pos @@
    emit_box_if_necessary pos need_ref @@ emit_collection env expr es
  | A.Darray es ->
    emit_pos_then pos @@
    let es2 = List.map ~f:(fun (e1, e2) -> A.AFkvalue (e1, e2)) es in
    let darray_e = fst expr, A.Darray es in
    emit_box_if_necessary pos need_ref @@ emit_collection env darray_e es2
  | A.Varray es ->
    emit_pos_then pos @@
    let es2 = List.map ~f:(fun e -> A.AFvalue e) es in
    let varray_e = fst expr, A.Varray es in
    emit_box_if_necessary pos need_ref @@ emit_collection env varray_e es2
  | A.Collection ((pos, name), fields) ->
    emit_box_if_necessary pos need_ref
      @@ emit_named_collection env expr pos name fields
  | A.Clone e ->
    emit_pos_then pos @@
    emit_box_if_necessary pos need_ref @@ emit_clone env e
  | A.Shape fl ->
    emit_pos_then pos @@
    emit_box_if_necessary pos need_ref @@ emit_shape env expr fl
  | A.Await e -> emit_await env pos e
  | A.Yield e -> emit_yield env pos e
  | A.Yield_break ->
    failwith "yield break should be in statement position"
  | A.Yield_from _ -> failwith "complex yield_from expression"
  | A.Lfun _ ->
    failwith "expected Lfun to be converted to Efun during closure conversion"
  | A.Efun (fundef, ids) ->
    emit_pos_then pos @@
    emit_box_if_necessary pos need_ref @@ emit_lambda env fundef ids
  | A.Class_get (cid, id)  ->
    emit_class_get env QueryOp.CGet need_ref cid id
  | A.String2 es ->
    emit_box_if_necessary pos need_ref @@ emit_string2 env pos es
  | A.BracedExpr e -> emit_expr ~need_ref:false env e
  | A.Dollar e ->
    check_non_pipe_local e;
    let instr = emit_expr ?last_pos ~need_ref:false env e in
    gather [
      instr;
      emit_pos (Option.value ~default:pos last_pos);
      if need_ref then instr_vgetn else instr_cgetn
    ]
  | A.Id id ->
    emit_pos_then pos @@
    emit_box_if_necessary pos need_ref @@ emit_id env id
  | A.Xml (id, attributes, children) ->
    emit_box_if_necessary pos need_ref @@
      emit_xhp env (fst expr) id attributes children
  | A.Callconv (kind, e) ->
    emit_box_if_necessary pos need_ref @@ emit_callconv env kind e
  | A.Import (flavor, e) ->
      emit_box_if_necessary pos need_ref @@ emit_import env pos flavor e
  | A.Omitted -> empty
  | A.Unsafeexpr _ ->
    failwith "Unsafe expression should be removed during closure conversion"
  | A.Suspend _ ->
    failwith "Codegen for 'suspend' operator is not supported"
  | A.List _ ->
    failwith "List destructor can only be used as an lvar"

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

and emit_value_only_collection env pos es constructor =
  let limit = max_array_elem_on_stack () in
  let inline exprs =
    gather
    [gather @@ List.map exprs
      ~f:(function
        (* Drop the keys *)
        | A.AFkvalue (_, e)
        | A.AFvalue e -> emit_expr ~need_ref:false env e);
      emit_pos pos;
      instr @@ ILitConst (constructor @@ List.length exprs)]
  in
  let outofline exprs =
    gather @@
    List.map exprs
      ~f:(function
        (* Drop the keys *)
        | A.AFkvalue (_, e)
        | A.AFvalue e -> gather [emit_expr ~need_ref:false env e; instr_add_new_elemc])
  in
  match (List.groupi ~break:(fun i _ _ -> i = limit) es) with
    | [] -> empty
    | x1 :: [] -> inline x1
    | x1 :: x2 :: _ -> gather [inline x1; outofline x2]

and emit_keyvalue_collection name env pos es constructor =
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

and emit_struct_array env pos es ctor =
  let es =
    List.map es
      ~f:(function A.AFkvalue (k, v) ->
            let ns = Emit_env.get_namespace env in
            (* TODO: Consider reusing folded keys from is_struct_init *)
            begin match snd @@ Ast_constant_folder.fold_expr ns k with
            | A.String s -> s, emit_expr ~need_ref:false env v
            | _ -> failwith "impossible"
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
and is_packed_init ?(hack_arr_compat=true) es =
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

and is_struct_init env es allow_numerics =
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
  && num_keys != 0

(* transform_to_collection argument keeps track of
 * what collection to transform to *)
and emit_dynamic_collection env (pos, expr_) es =
  let count = List.length es in
  match expr_ with
  | A.Collection ((_, "vec"), _) ->
    emit_value_only_collection env pos es (fun n -> NewVecArray n)
  | A.Collection ((_, "keyset"), _) ->
    emit_value_only_collection env pos es (fun n -> NewKeysetArray n)
  | A.Collection ((_, "dict"), _) ->
     if is_struct_init env es true then
       emit_struct_array env pos es instr_newstructdict
     else
       emit_keyvalue_collection "dict" env pos es (NewDictArray count)
  | A.Collection ((_, name), _)
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

and emit_named_collection env expr pos name fields =
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
    gather [
      emit_collection env (pos, A.Collection ((pos, "vec"), fields)) fields;
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
        | A.AFvalue e -> emit_expr ~need_ref:false env e
        | _ -> failwith "impossible Pair argument"));
      instr (ILitConst NewPair);
    ]
  | _ -> failwith @@ "collection: " ^ name ^ " does not exist"

and is_php_array = function
 | _, A.Array _ -> true
 | _, A.Varray _ -> not (hack_arr_dv_arrs ())
 | _, A.Darray _ -> not (hack_arr_dv_arrs ())
 | _ -> false

and emit_collection ?(transform_to_collection) env expr es =
  match Ast_constant_folder.expr_to_opt_typed_value
          ~allow_maps:true
          ~restrict_keys:(not @@ is_php_array expr)
          (Emit_env.get_namespace env)
          expr
  with
  | Some tv ->
    emit_static_collection ~transform_to_collection (fst expr) tv
  | None ->
    emit_dynamic_collection env expr es

and emit_pipe env pos e1 e2 =
  stash_in_local ~always_stash:true env pos e1
  begin fun temp _break_label ->
  let env = Emit_env.with_pipe_var temp env in
  emit_expr ~need_ref:false env e2
  end

(* Emit code that is equivalent to
 *   <code for expr>
 *   JmpZ label
 * Generate specialized code in case expr is statically known, and for
 * !, && and || expressions
 *)
and emit_jmpz env (pos, expr_ as expr) label: emit_jmp_result =
  let with_pos i = emit_pos_then pos i in
  let opt = optimize_null_check () in
  match Ast_constant_folder.expr_to_opt_typed_value (Emit_env.get_namespace env) expr with
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
    | A.Unop(A.Unot, e) ->
      emit_jmpnz env e label
    | A.Binop(A.BArbar, e1, e2) ->
      let skip_label = Label.next_regular () in
      let r1 = emit_jmpnz env e1 skip_label in
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
    | A.Binop(A.AMpamp, e1, e2) ->
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
    | A.Binop(A.EQeqeq, e, (_, A.Null))
    | A.Binop(A.EQeqeq, (_, A.Null), e) when opt ->
      { instrs = with_pos @@ gather [
          emit_is_null env e;
          instr_jmpz label;
        ];
        is_fallthrough = true;
        is_label_used = true; }
    | A.Binop(A.Diff2, e, (_, A.Null))
    | A.Binop(A.Diff2, (_, A.Null), e) when opt ->
      { instrs = with_pos @@ gather [
          emit_is_null env e;
          instr_jmpnz label;
        ];
        is_fallthrough = true;
        is_label_used = true; }
    | _ ->
      { instrs = with_pos @@ gather [
          emit_expr ~need_ref:false env expr;
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
and emit_jmpnz env (pos, expr_ as expr) label: emit_jmp_result =
  let with_pos i = emit_pos_then pos i in
  let opt = optimize_null_check () in
  match Ast_constant_folder.expr_to_opt_typed_value (Emit_env.get_namespace env) expr with
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
    | A.Unop(A.Unot, e) ->
      emit_jmpz env e label
    | A.Binop(A.BArbar, e1, e2) ->
      let r1 = emit_jmpnz env e1 label in
      if not r1.is_fallthrough then r1
      else
        let r2 = emit_jmpnz env e2 label in
        { instrs = with_pos @@ gather [ r1.instrs; r2.instrs ];
          is_fallthrough = r2.is_fallthrough;
          is_label_used = r1.is_label_used || r2.is_label_used }
    | A.Binop(A.AMpamp, e1, e2) ->
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
        let r2 = emit_jmpnz env e2 label in
        { instrs = with_pos @@ gather [
            r1.instrs;
            r2.instrs;
            optional r1.is_label_used [instr_label skip_label]
          ];
          is_fallthrough = r2.is_fallthrough || r1.is_label_used;
          is_label_used = r2.is_label_used }
      end
    | A.Binop(A.EQeqeq, e, (_, A.Null))
    | A.Binop(A.EQeqeq, (_, A.Null), e) when opt ->
      { instrs = with_pos @@ gather [
          emit_is_null env e;
          instr_jmpnz label;
        ];
        is_fallthrough = true;
        is_label_used = true; }
    | A.Binop(A.Diff2, e, (_, A.Null))
    | A.Binop(A.Diff2, (_, A.Null), e) when opt ->
      { instrs = with_pos @@ gather [
          emit_is_null env e;
          instr_jmpz label;
        ];
        is_fallthrough = true;
        is_label_used = true; }
    | _ ->
      { instrs = with_pos @@ gather [
          emit_expr ~need_ref:false env expr;
          instr_jmpnz label;
        ];
        is_fallthrough = true;
        is_label_used = true; }
    end

and emit_short_circuit_op env pos expr =
  let its_true = Label.next_regular () in
  let its_done = Label.next_regular () in
  let r1 = emit_jmpnz env (pos, expr) its_true in
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

and emit_null_coalesce_assignment ~need_ref env pos e1 e2 =
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
    emit_lval_op ~null_coalesce_assignment:true ~need_ref env pos LValOp.Set e1 (Some e2);
    instr_label end_label;
  ]

and emit_quiet_expr ?(null_coalesce_assignment=false) env pos (_, expr_ as expr) =
  match expr_ with
  | A.Lvar (name_pos, name) when name = SN.Superglobals.globals ->
    gather [
      emit_pos name_pos;
      instr_string (SU.Locals.strip_dollar name);
      emit_pos pos;
      instr (IGet CGetQuietG)
    ], None
  | A.Lvar ((_, name) as id) when not (is_local_this env name) ->
    instr_cgetquietl (get_local env id), None
  | A.Dollar e ->
    gather [
      emit_expr ~need_ref:false env e;
      emit_pos pos;
      instr_cgetquietn
    ], None
  | A.Array_get((_, A.Lvar (_, x)), Some e) when x = SN.Superglobals.globals ->
    gather [
      emit_expr ~need_ref:false env e;
      emit_pos pos;
      instr (IGet CGetQuietG)
    ], None
  | A.Array_get(base_expr, opt_elem_expr) ->
    emit_array_get ~null_coalesce_assignment ~need_ref:false
      env pos QueryOp.CGetQuiet base_expr opt_elem_expr
  | A.Obj_get (expr, prop, nullflavor) ->
    emit_obj_get ~null_coalesce_assignment ~need_ref:false
      env pos QueryOp.CGetQuiet expr prop nullflavor
  | _ ->
    emit_expr ~need_ref:false env expr, None

(* returns instruction that will represent setter for $base[local] where
   is_base is true when result cell is base for another subscript operator and
   false when it is final left hand side of the assignment *)
and emit_store_for_simple_base ~is_base env pos elem_stack_size base_expr local =
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
and get_local_temp_kind ~is_base inout_param_info env e_opt =
  match e_opt, inout_param_info with
  (* not inout case - no need to save *)
  | _, None -> None
  (* local that will later be overwritten *)
  | Some (_, A.Lvar (_, id)), Some (i, aliases)
    when InoutLocals.should_save_local_value id i aliases -> Some Value_kind_local
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

and emit_array_get ?(null_coalesce_assignment=false) ?(no_final=false) ?mode ~need_ref
  env outer_pos qop base_expr opt_elem_expr =
  let result =
    emit_array_get_worker ~null_coalesce_assignment ~no_final ?mode ~need_ref ~inout_param_info:None
    env outer_pos qop base_expr opt_elem_expr in
  match result with
  | Array_get_regular i, querym_n_unpopped -> i, querym_n_unpopped
  | Array_get_inout _, _ -> failwith "unexpected inout"

and emit_array_get_worker ?(null_coalesce_assignment=false) ?(no_final=false) ?mode
  ~need_ref ~inout_param_info
  env outer_pos qop base_expr opt_elem_expr =
  (* Disallow use of array(..)[] *)
  match base_expr, opt_elem_expr with
  | (pos, A.Array _), None ->
    Emit_fatal.raise_fatal_parse pos "Can't use array() as base in write context"
  | (pos, _), None when not (Emit_env.does_env_allow_array_append env)->
    Emit_fatal.raise_fatal_runtime pos "Can't use [] for reading"
  | _ ->
  let local_temp_kind =
    get_local_temp_kind ~is_base:false inout_param_info env opt_elem_expr in
  let mode =
    if null_coalesce_assignment then MemberOpMode.Warn
    else Option.value mode ~default:(get_queryMOpMode need_ref qop) in
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
      if need_ref then
        VGetM (total_stack_size, mk)
      else if null_coalesce_assignment then begin
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
      emit_store_for_simple_base ~is_base:false env outer_pos elem_stack_size
      base_expr local in
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
    let store =  gather [
      base.store;
      instr_setm 0 mk;
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
    ] in
    Array_get_inout { load; store }
  in
  instr, !querym_n_unpopped

(* Emit code for e1->e2 or e1?->e2 or isset(e1->e2).
 *)
and emit_obj_get ?(null_coalesce_assignment=false) ~need_ref env pos qop expr prop null_flavor =
  match snd expr with
  | A.Lvar (pos, id)
    when id = SN.SpecialIdents.this && null_flavor = A.OG_nullsafe ->
    Emit_fatal.raise_fatal_parse
      pos "?-> is not allowed with $this"
  | _ ->
    begin match snd prop with
    | A.Id (_, s) when SU.Xhp.is_xhp s ->
      emit_xhp_obj_get ~need_ref env pos expr s null_flavor, None
    | _ ->
      let mode =
        if null_coalesce_assignment then MemberOpMode.Warn
        else get_queryMOpMode need_ref qop in
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
          if need_ref then
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

and is_special_class_constant_accessed_with_class_id env (_, cName) id =
  (* TODO(T21932293): HHVM does not match Zend here.
   * Eventually remove this to match PHP7 *)
  SU.is_class id &&
  (not (SU.is_self cName || SU.is_parent cName || SU.is_static cName)
  || (Ast_scope.Scope.is_in_trait (Emit_env.get_scope env)) && SU.is_self cName)

and emit_elem_instrs env ~local_temp_kind ?(null_coalesce_assignment=false) opt_elem_expr =
  match opt_elem_expr with
  (* These all have special inline versions of member keys *)
  | Some (_, (A.Int _ | A.String _)) -> empty, 0
  | Some (_, (A.Lvar ((_, id) as pid))) when not (is_local_this env id) ->
    if Option.is_some local_temp_kind
    then instr_cgetquietl (get_local env pid), 0
    else if null_coalesce_assignment then instr_cgetl (get_local env pid), 1
    else empty, 0
  | Some (_, (A.Class_const ((_, A.Id cid), (_, id))))
    when is_special_class_constant_accessed_with_class_id env cid id -> empty, 0
  | Some expr -> emit_expr ~need_ref:false env expr, 1
  | None -> empty, 0

(* Get the member key for an array element expression: the `elem` in
 * expressions of the form `base[elem]`.
 * If the array element is missing, use the special key `W`.
 *)
and get_elem_member_key ?(null_coalesce_assignment=false) env stack_index opt_expr =
  match opt_expr with
  (* Special case for local *)
  | Some (_, A.Lvar id) when not (is_local_this env (snd id)) ->
    if null_coalesce_assignment then MemberKey.EC stack_index
    else MemberKey.EL (get_local env id)
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
  | Some (_, (A.Class_const ((_, A.Id (p, cName as cid)), (_, id))))
    when is_special_class_constant_accessed_with_class_id env cid id ->
    (* Special case for self::class in traits *)
    (* TODO(T21932293): HHVM does not match Zend here.
     * Eventually remove this to match PHP7 *)
    let cName =
      match SU.is_self cName,
            Ast_scope.Scope.get_class (Emit_env.get_scope env)
      with
      | true, Some cd -> SU.strip_global_ns @@ snd cd.A.c_name
      | _ -> cName
    in
    let fq_id, _ =
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
  stack_index prop_expr =
  let mk =
    match snd prop_expr with
    | A.Id ((_, name) as id) when String_utils.string_starts_with name "$" ->
      MemberKey.PL (get_local env id)
    (* Special case for known property name *)
    | A.Id (_, id)
    | A.String id ->
      let pid = Hhbc_id.Prop.from_ast_name id in
      begin match null_flavor with
      | Ast.OG_nullthrows -> MemberKey.PT pid
      | Ast.OG_nullsafe -> MemberKey.QT pid
      end
    | A.Lvar ((_, name) as id) when not (is_local_this env name) ->
      MemberKey.PL (get_local env id)
    (* General case *)
    | _ ->
      MemberKey.PC stack_index
  in
  (* For nullsafe access, insist that property is known *)
  begin match mk with
  | MemberKey.PL _ | MemberKey.PC _ ->
    if null_flavor = A.OG_nullsafe then
      Emit_fatal.raise_fatal_parse (fst prop_expr)
        "?-> can only be used with scalar property names"
  | _ -> ()
  end;
  match mk with
  | MemberKey.PC _ ->
    mk, emit_expr ~need_ref:false env prop_expr, 1
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

and emit_base ~is_object ~notice ?(null_coalesce_assignment=false) env mode base_offset e =
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
  (pos, expr_ as expr) =
  let base_mode =
    if mode = MemberOpMode.InOut then MemberOpMode.Warn else mode in
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
   | A.Lvar (name_pos, x) when SN.Superglobals.is_superglobal x ->
     emit_default
       (emit_pos_then name_pos @@ instr_string (SU.Locals.strip_dollar x))
       empty
       (instr (IBase (BaseGC (base_offset, base_mode))))
       1

   | A.Lvar (thispos, x) when is_object && x = SN.SpecialIdents.this ->
     emit_default
       (emit_pos_then thispos @@ instr (IMisc CheckThis))
       empty
       (instr (IBase BaseH))
       0

   | A.Lvar ((_, str) as id)
     when not (is_local_this env str) || Emit_env.get_needs_local_this env ->
     let v = get_local env id in
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

   | A.Lvar id ->
     emit_default
       (emit_local ~notice ~need_ref:false env id)
       empty
       (instr (IBase (BaseC (base_offset, base_mode))))
       1

   | A.Array_get((_, A.Lvar (_, x)), Some (_, A.Lvar y))
     when x = SN.Superglobals.globals ->
     let v = get_local env y in
     emit_default
       empty
       empty
       (instr (IBase (BaseGL (v, base_mode))))
       0

   | A.Array_get((_, A.Lvar (_, x)), Some e) when x = SN.Superglobals.globals ->
     let elem_expr_instrs = emit_expr ~need_ref:false env e in
     emit_default
       elem_expr_instrs
       empty
       (instr (IBase (BaseGC (base_offset, base_mode))))
       1
   (* $a[] can not be used as the base of an array get unless as an lval *)
   | A.Array_get(_, None) when not (Emit_env.does_env_allow_array_append env) ->
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
         (emit_xhp_obj_get_raw env pos base_expr s null_flavor)
         empty
         (gather [ instr_baser base_offset base_mode ])
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

   | A.Class_get(cid, (_, A.Dollar (_, A.Lvar id))) ->
     let cexpr = expr_to_class_expr ~resolve_self:false
       (Emit_env.get_scope env) cid in
     (* special case for $x->$$y: use BaseSL *)
     emit_default
       (emit_load_class_ref env pos cexpr)
       empty
       (emit_pos_then pos @@
       instr_basesl (get_local env id) base_mode)
       0
   | A.Class_get(cid, prop) ->
     let cexpr = expr_to_class_expr ~resolve_self:false
       (Emit_env.get_scope env) cid in
     let cexpr_begin, cexpr_end = emit_class_expr env cexpr prop in
     emit_default
       cexpr_begin
       cexpr_end
       (instr_basesc base_offset base_mode)
       1
   | A.Dollar (_, A.Lvar id as e) ->
     check_non_pipe_local e;
     let local = get_local env id in
     emit_default
       empty
       empty
       (emit_pos_then pos @@ instr_basenl local base_mode)
       0
   | A.Dollar e ->
     let base_expr_instrs = emit_expr ~need_ref:false env e in
     emit_default
       base_expr_instrs
       empty
       (emit_pos_then pos @@ instr_basenc base_offset base_mode)
       1
   | _ ->
     let base_expr_instrs, flavor = emit_flavored_expr env expr in
     emit_default
       (if binary_assignment_rhs_starts_with_ref expr
       then gather [base_expr_instrs; instr_unbox]
       else base_expr_instrs)
       empty
       (emit_pos_then pos @@
       instr (IBase (if flavor = Flavor.ReturnVal
                     then BaseR (base_offset, base_mode) else BaseC (base_offset, base_mode))))
       1

and use_pass_by_ref_hint () =
  Emit_env.is_hh_syntax_enabled () && not (Emit_env.is_systemlib ())

and get_pass_by_ref_hint expr =
  if expr_starts_with_ref expr then Ref else Cell

and strip_ref e =
  match snd e with
  | A.Unop (A.Uref, e) -> e
  | _ -> e

and emit_ignored_expr env ?(pop_pos = Pos.none) e =
  match snd e with
  | A.Expr_list es -> gather @@ List.map ~f:(emit_ignored_expr env ~pop_pos) es
  | _ ->
    let instrs, flavor = emit_flavored_expr ~last_pos:pop_pos env e in
    gather [
      instrs;
      emit_pos_then pop_pos @@ instr_pop flavor;
    ]

(* Emit code to construct the argument frame and then make the call *)
and emit_args_and_call env call_pos args uargs =
  let args_count = List.length args in
  let all_args = args @ uargs in
  let aliases =
    if has_inout_args args
    then InoutLocals.collect_written_variables env args
    else SMap.empty in
  let use_hint = use_pass_by_ref_hint () in
  let throw_on_mismatch = use_hint &&
    Hhbc_options.throw_on_call_by_ref_annotation_mismatch
    !Hhbc_options.compiler_options in

  let rec aux i rem_args inout_setters =
    match rem_args with
    | [] ->
      let use_unpack = (uargs != []) in
      let num_inout = List.length inout_setters in
      let nargs = List.length args in
      let instr_enforce_hint =
        if throw_on_mismatch && (args != [])
        then instr_fthrow_on_ref_mismatch (List.map args expr_starts_with_ref)
        else empty
      in
      gather [
        (* emit call*)
        emit_pos call_pos;
        instr_enforce_hint;
        instr_fcall nargs use_unpack (num_inout + 1);
        (* propagate inout values back *)
        if List.is_empty inout_setters
        then empty
        else begin
          let local = Local.get_unnamed_local () in
          gather [
            Emit_inout_helpers.emit_list_set_for_inout_call local
              (List.rev inout_setters)
          ]
        end; ]

    | (_, A.Callconv (A.Pinout, expr)) :: rest -> begin
      let pos, expr_ = strip_ref expr in
      match expr_ with
      | A.Lvar (name_pos, s) ->
        let inout_setters =
          (instr_setl @@ Local.Named s) :: inout_setters in
        let not_in_try = not (Emit_env.is_in_try env) in
        let move_instrs =
          if not_in_try && (InoutLocals.should_move_local_value s aliases)
          then gather [ instr_null; instr_popl @@ Local.Named s ]
          else empty in
        gather [
          emit_pos name_pos;
          instr_cgetl @@ Local.Named s;
          move_instrs;
          aux (i + 1) rest inout_setters
        ]
      | A.Array_get (base_expr, opt_elem_expr) -> begin
        let array_get_result =
          fst (emit_array_get_worker ~need_ref:false
            ~inout_param_info:(Some (i, aliases)) env pos
            QueryOp.InOut base_expr opt_elem_expr) in
        match array_get_result with
        | Array_get_regular instrs ->
          let setter =
            let base =
              fst (emit_array_get ~no_final:true ~need_ref:false
                ~mode:MemberOpMode.Define
                env pos QueryOp.InOut base_expr opt_elem_expr) in
            gather [
              base;
              instr_setm 0 (get_elem_member_key env 0 opt_elem_expr);
            ] in
          gather [
            instrs;
            aux (i + 1) rest (setter :: inout_setters) ]
        | Array_get_inout { load; store } ->
          rebuild_sequence load @@ begin fun () ->
            aux (i + 1) rest (store :: inout_setters)
          end
        end
      | _ -> failwith "emit_args_and_call: Unexpected inout expression type"
      end

    | expr :: rest ->
      let next c = gather [ c; aux (i + 1) rest inout_setters ] in next @@
      let param_pos, _ = expr in
      let hint = if use_hint then get_pass_by_ref_hint expr else Any in
      let expr = strip_ref expr in
      if i >= args_count then
        emit_expr ~need_ref:false env expr
      else
      if throw_on_mismatch && hint = Cell then
        emit_expr ~need_ref:false env expr
      else if throw_on_mismatch && hint = Ref then
        match snd expr with
        | A.Lvar _
        | A.Dollar _
        | A.Array_get _
        | A.Obj_get _
        | A.Class_get _
        | A.Binop (A.Eq None, (_, A.List _), (_, A.Lvar _)) ->
          emit_expr_as_ref env expr;
        | _ ->
          let instrs, flavor = emit_flavored_expr env ~last_pos:call_pos expr in
          if flavor != Flavor.ReturnVal then instrs else
          gather [
            instrs;
            emit_pos param_pos;
            instr_boxr
          ]
      else
      let emit_ref_cond instrs_init instrs_by_val instrs_by_ref =
        let by_ref_label = Label.next_regular () in
        let done_label = Label.next_regular () in
        gather [
          instrs_init;
          emit_pos param_pos;
          instr_fis_param_by_ref i hint;
          instr_jmpnz by_ref_label;
          instrs_by_val;
          instr_jmp done_label;
          instr_label by_ref_label;
          instrs_by_ref;
          instr_label done_label
        ]
      in
      let pos, expr_ = expr in
      match expr_ with
      | A.Lvar (name_pos, x) when SN.Superglobals.is_superglobal x ->
        emit_ref_cond
          (emit_pos_then name_pos @@ instr_string (SU.Locals.strip_dollar x))
          instr_cgetg instr_vgetg
      | A.Lvar ((_, str) as id)
        when not (is_local_this env str) || Emit_env.get_needs_local_this env ->
        let local = get_local env id in
        emit_ref_cond empty (instr_cgetl local) (instr_vgetl local)
      | A.Dollar e ->
        check_non_pipe_local e;
        emit_ref_cond (emit_expr ~need_ref:false env e) instr_cgetn instr_vgetn
      | A.Array_get ((_, A.Lvar (_, x)), Some e)
        when x = SN.Superglobals.globals ->
        emit_ref_cond (emit_expr ~need_ref:false env e) instr_cgetg instr_vgetg
      | A.Array_get (base_expr, opt_elem_expr) ->
        let env = { env with Emit_env.env_allows_array_append = true } in
        emit_ref_cond empty
          (fst (emit_array_get ~need_ref:false env pos QueryOp.CGet base_expr
            opt_elem_expr))
          (fst (emit_array_get ~need_ref:true env pos QueryOp.Empty base_expr
            opt_elem_expr))
      | A.Obj_get (e1, e2, nullflavor) ->
        emit_ref_cond empty
          (fst (emit_obj_get ~need_ref:false env pos QueryOp.CGet e1 e2 nullflavor))
          (fst (emit_obj_get ~need_ref:true env pos QueryOp.Empty e1 e2 nullflavor))
      | A.Class_get (cid, prop) ->
        emit_ref_cond empty
          (emit_class_get env QueryOp.CGet false cid prop)
          (emit_class_get env QueryOp.CGet true cid prop)
      | A.Binop (A.Eq None, (_, A.List _ as e), (_, A.Lvar id)) ->
        let local = get_local env id in
        let lhs_instrs, set_instrs =
          emit_lval_op_list env pos (Some local) [] e in
        emit_ref_cond
          (gather [ lhs_instrs; set_instrs ])
          (instr_cgetl local) (instr_vgetl local)
      | _ ->
        let instrs, flavor = emit_flavored_expr env ~last_pos:call_pos expr in
        match flavor with
        | Flavor.Ref -> emit_ref_cond instrs instr_unbox empty
        | Flavor.ReturnVal -> emit_ref_cond instrs instr_unboxr instr_boxr
        | Flavor.Cell ->
          gather [
            instrs;
            emit_pos param_pos;
            instr_fis_param_by_ref i hint;
            instr_popc;
          ]
  in
  Local.scope @@ fun () -> aux 0 all_args []

(* Expression that appears in an object context, such as expr->meth(...) *)
and emit_object_expr env ?last_pos (_, expr_ as expr) =
  match expr_ with
  | A.Lvar(_, x) when is_local_this env x ->
    instr_this
  | _ -> emit_expr ?last_pos ~need_ref:false env expr

and emit_call_lhs_with_this env instrs = Local.scope @@ fun () ->
  let id = Pos.none, SN.SpecialIdents.this in
  let temp = Local.get_unnamed_local () in
  gather [
    emit_local ~notice:Notice ~need_ref:false env id;
    instr_setl temp;
    with_temp_local temp
    begin fun temp _ -> gather [
      instr_popc;
      instrs;
      instr (IGet (ClsRefGetL (temp, 0)));
      instr_unsetl temp;
    ]
    end
  ]

and has_inout_args es =
  List.exists es ~f:(function _, A.Callconv (A.Pinout, _) -> true | _ -> false)

and emit_call_lhs env outer_pos (pos, expr_ as expr) nargs has_splat inout_arg_positions =
  let has_inout_args = List.length inout_arg_positions <> 0 in
  match expr_ with
  | A.Obj_get (obj, (_, A.Id ((_, str) as id)), null_flavor)
    when str.[0] = '$' ->
    gather [
      emit_object_expr ~last_pos:outer_pos env obj;
      instr_cgetl (get_local env id);
      instr_fpushobjmethod nargs null_flavor inout_arg_positions;
    ]
  | A.Obj_get (obj, (_, A.String id), null_flavor)
  | A.Obj_get (obj, (_, A.Id (_, id)), null_flavor) ->
    let name = Hhbc_id.Method.from_ast_name id in
    let name =
      if has_inout_args
      then Hhbc_id.Method.add_suffix name
        (Emit_inout_helpers.inout_suffix inout_arg_positions)
      else name in
    gather [
      emit_object_expr env ~last_pos:outer_pos obj;
      emit_pos outer_pos;
      instr_fpushobjmethodd nargs name null_flavor;
    ]
  | A.Obj_get(obj, method_expr, null_flavor) ->
    gather [
      emit_pos outer_pos;
      emit_object_expr env ~last_pos:outer_pos obj;
      emit_expr ~need_ref:false env method_expr;
      instr_fpushobjmethod nargs null_flavor inout_arg_positions;
    ]

  | A.Class_const (cid, (_, id)) ->
    let cexpr = expr_to_class_expr ~resolve_self:false
      (Emit_env.get_scope env) cid in
    let method_id = Hhbc_id.Method.from_ast_name id in
    let method_id =
      if has_inout_args
      then Hhbc_id.Method.add_suffix method_id
        (Emit_inout_helpers.inout_suffix inout_arg_positions)
      else method_id in
    begin match cexpr with
    (* Statically known *)
    | Class_id cid ->
      let fq_cid, _ = Hhbc_id.Class.elaborate_id (Emit_env.get_namespace env) cid in
      Emit_symbol_refs.add_class (Hhbc_id.Class.to_raw_string fq_cid);
      instr_fpushclsmethodd nargs method_id fq_cid
    | Class_static ->
      instr_fpushclsmethodsd nargs SpecialClsRef.Static method_id
    | Class_self ->
      instr_fpushclsmethodsd nargs SpecialClsRef.Self method_id
    | Class_parent ->
      instr_fpushclsmethodsd nargs SpecialClsRef.Parent method_id
    | Class_expr (_, A.Lvar (_, x)) when x = SN.SpecialIdents.this ->
       let method_name = Hhbc_id.Method.to_raw_string method_id in
       gather [
         emit_call_lhs_with_this env @@ instr_string method_name;
         instr_fpushclsmethod nargs []
       ]
    | _ ->
       let method_name = Hhbc_id.Method.to_raw_string method_id in
       gather [
         of_pair @@ emit_class_expr env cexpr (Pos.none, A.Id (Pos.none, method_name));
         instr_fpushclsmethod nargs []
       ]
    end

  | A.Class_get (cid, e) ->
    let cexpr = expr_to_class_expr ~resolve_self:false
      (Emit_env.get_scope env) cid in
    let expr_instrs = emit_expr ~need_ref:false env e in
    begin match cexpr with
    | Class_static ->
       gather [expr_instrs;
         emit_pos outer_pos; instr_fpushclsmethods nargs SpecialClsRef.Static]
    | Class_self ->
       gather [expr_instrs;
         emit_pos outer_pos; instr_fpushclsmethods nargs SpecialClsRef.Self]
    | Class_parent ->
       gather [expr_instrs;
         emit_pos outer_pos; instr_fpushclsmethods nargs SpecialClsRef.Parent]
    | Class_expr (_, A.Lvar (_, x)) when x = SN.SpecialIdents.this ->
       gather [
        emit_call_lhs_with_this env expr_instrs;
        emit_pos outer_pos;
        instr_fpushclsmethod nargs inout_arg_positions
       ]
    | _ ->
       gather [
        expr_instrs;
        emit_load_class_ref env pos cexpr;
        emit_pos outer_pos;
        instr_fpushclsmethod nargs inout_arg_positions
       ]
    end

  | A.Id (_, s as id)->
    let fq_id, id_opt =
      Hhbc_id.Function.elaborate_id_with_builtins (Emit_env.get_namespace env) id in
    let fq_id, id_opt =
      match id_opt, SU.strip_global_ns s with
      | None, "min" when nargs = 2 && not has_splat ->
        Hhbc_id.Function.from_raw_string "__SystemLib\\min2", None
      | None, "max" when nargs = 2 && not has_splat ->
        Hhbc_id.Function.from_raw_string  "__SystemLib\\max2", None
      | _ -> fq_id, id_opt in
    let fq_id = if has_inout_args
      then Hhbc_id.Function.add_suffix
        fq_id (Emit_inout_helpers.inout_suffix inout_arg_positions)
      else fq_id in
    emit_pos_then outer_pos @@
    begin match id_opt with
    | Some id -> instr (ICall (FPushFuncU (nargs, fq_id, id)))
    | None -> instr (ICall (FPushFuncD (nargs, fq_id)))
    end
  | A.String s ->
    emit_pos_then outer_pos @@
    instr_fpushfuncd nargs (Hhbc_id.Function.from_raw_string s)
  | _ ->
    gather [
      emit_expr ~need_ref:false env expr;
      emit_pos outer_pos;
      instr_fpushfunc nargs inout_arg_positions
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
  | _ -> None

(* TODO: work out what HHVM does special here *)
and emit_name_string env e =
  emit_expr ~need_ref:false env e

and emit_special_function env pos id args uargs default =
  let nargs = List.length args + List.length uargs in
  let fq_id, _ =
    Hhbc_id.Function.elaborate_id_with_builtins (Emit_env.get_namespace env) (Pos.none, id) in
  (* Make sure that we do not treat a special function that is aliased as not
   * aliased *)
  let lower_fq_name =
    String.lowercase_ascii (Hhbc_id.Function.to_raw_string fq_id) in
  let hh_enabled = Emit_env.is_hh_syntax_enabled () in
  match lower_fq_name, args with
  | id, _ when id = SN.SpecialFunctions.echo ->
    let instrs = gather @@ List.mapi args begin fun i arg ->
         gather [
           emit_expr ~need_ref:false env arg;
           emit_pos pos;
           instr (IOp Print);
           if i = nargs-1 then empty else instr_popc
         ] end in
    Some (instrs, Flavor.Cell)

  | "array_slice", [
    _, A.Call ((_, A.Id (_, s)), _, [], []); (_, A.Int _ as count)
    ] when not (jit_enable_rename_function ())
           && String.lowercase_ascii @@ SU.strip_ns s = "func_get_args"->
    let p = Pos.none in
    Some (emit_call env pos (p,
        A.Id (p, "\\__SystemLib\\func_slice_args")) [count] [])

  | "hh\\asm", [_, A.String s] ->
    Some (emit_inline_hhas s, Flavor.Cell)

  | "hh\\invariant", e::rest when hh_enabled ->
    let l = Label.next_regular () in
    let expr_id = pos, A.Id (pos, "\\hh\\invariant_violation") in
    Some (gather [
      (* Could use emit_jmpnz for better code *)
      emit_expr ~need_ref:false env e;
      instr_jmpnz l;
      emit_ignored_expr env (pos, A.Call (expr_id, [], rest, uargs));
      Emit_fatal.emit_fatal_runtime pos "invariant_violation";
      instr_label l;
      instr_null;
    ], Flavor.Cell)

  | "assert", _ ->
    let l0 = Label.next_regular () in
    let l1 = Label.next_regular () in
    Some (gather [
      instr_string "zend.assertions";
      instr_fcallbuiltin 1 1 "ini_get";
      instr_unboxr_nop;
      instr_int 0;
      instr_gt;
      instr_jmpz l0;
      fst @@ default ();
      instr_unboxr;
      instr_jmp l1;
      instr_label l0;
      instr_true;
      instr_label l1;
    ], Flavor.Cell)

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
        emit_expr ~need_ref:false env (List.nth_exn args 1);
        instr (IOp CastBool)
      ];
      instr (IMisc (OODeclExists class_kind))
    ], Flavor.Cell)

  | ("exit" | "die"), _ when nargs = 0 || nargs = 1 ->
    Some (emit_exit env (List.hd args), Flavor.Cell)

  | "hh\\fun", _ ->
    if nargs <> 1 then
      Emit_fatal.raise_fatal_runtime pos
        ("fun() expects exactly 1 parameter, " ^ (string_of_int nargs) ^
         " given")
    else begin match args with
      | [(pos, A.String func_name)] ->
        let func_id, id_opt = Hhbc_id.Function.elaborate_id_with_builtins
          (Emit_env.get_namespace env) (pos, func_name) in
        let func_id = Option.value_map id_opt
          ~default:func_id ~f:Hhbc_id.Function.from_raw_string in
        Some (instr_resolve_func func_id, Flavor.Cell)
      | _ ->
        Emit_fatal.raise_fatal_runtime pos "Constant string expected in fun()"
    end

  | "hh\\inst_meth", _ ->
    begin match args with
      | [obj_expr; method_name] ->
        Some (gather [
          emit_expr ~need_ref:false env obj_expr;
          emit_expr ~need_ref:false env method_name;
          instr_resolve_obj_method;
        ], Flavor.Cell)
      | _ ->
        Emit_fatal.raise_fatal_runtime pos
          ("inst_meth() expects exactly 2 parameters, " ^
           (string_of_int nargs) ^ " given")
    end

  | "hh\\class_meth", _ ->
      begin match args with
        | [class_name; method_name] ->
          Some (gather [
            emit_expr ~need_ref:false env class_name;
            emit_expr ~need_ref:false env method_name;
            instr_resolve_cls_method;
          ], Flavor.Cell)
        | _ ->
          Emit_fatal.raise_fatal_runtime pos
            ("class_meth() expects exactly 2 parameters, " ^
             (string_of_int nargs) ^ " given")
      end

  | _ ->
    begin match args, istype_op lower_fq_name, is_isexp_op lower_fq_name with
    | [arg_expr], _, Some h when Emit_env.is_hh_syntax_enabled () ->
      (* T29079834:
       * Using this as a migration from is_{int,bool,etc} to is expressions *)
      Some (gather [
        emit_expr ~need_ref:false env arg_expr;
        emit_is env pos h
      ], Flavor.Cell)
    | [(_, A.Lvar (_, arg_str as arg_id))], Some i, _
      when SN.Superglobals.is_superglobal arg_str ->
      Some (gather [
        emit_local ~notice:NoNotice ~need_ref:false env arg_id;
        emit_pos pos;
        instr (IIsset (IsTypeC i))
      ], Flavor.Cell)
    | [(_, A.Lvar (_, arg_str as arg_id))], Some i, _
      when not (is_local_this env arg_str) ->
      Some (instr (IIsset (IsTypeL (get_local env arg_id, i))), Flavor.Cell)
    | [arg_expr], Some i, _ ->
      Some (gather [
        emit_expr ~need_ref:false env arg_expr;
        emit_pos pos;
        instr (IIsset (IsTypeC i))
      ], Flavor.Cell)
    | _ ->
      begin match get_call_builtin_func_info lower_fq_name with
      | Some (nargs, i) when nargs = List.length args ->
        Some (
          gather [
          emit_exprs env pos args;
          emit_pos pos;
          instr i
        ], Flavor.Cell)
      | _ -> None
      end
    end

and get_inout_arg_positions args =
  List.filter_mapi args
    ~f:(fun i -> function
          | _, A.Callconv (A.Pinout, _) -> Some i
          | _ -> None)

and emit_call env pos (_, expr_ as expr) args uargs =
  (match expr_ with
    | A.Id (_, s) -> Emit_symbol_refs.add_function s
    | _ -> ());
  let nargs = List.length args + List.length uargs in
  let inout_arg_positions = get_inout_arg_positions args in
  let num_uninit = List.length inout_arg_positions in
  let default () =
    let flavor = if List.is_empty inout_arg_positions then
      Flavor.ReturnVal else Flavor.Cell in
    gather [
      gather @@ List.init num_uninit ~f:(fun _ -> instr_nulluninit);
      emit_call_lhs
        env pos expr nargs (not (List.is_empty uargs)) inout_arg_positions;
      emit_args_and_call env pos args uargs;
    ], flavor in

  match expr_, args with
  | A.Id (_, id), _ ->
    let special_fn_opt = emit_special_function env pos id args uargs default in
    begin match special_fn_opt with
    | Some (instrs, flavor) -> instrs, flavor
    | None -> default ()
    end
  | _ -> default ()


(* Emit code for an expression that might leave a cell or reference on the
 * stack. Return which flavor it left.
 *)
and emit_flavored_expr env ?last_pos (pos, expr_ as expr) =
  match expr_ with
  | A.Call (e, _, args, uargs)
    when not (is_special_function env e args) ->
    let instrs, flavor = emit_call env pos e args uargs in
    emit_pos_then pos instrs, flavor
  | A.Execution_operator es ->
    emit_execution_operator env pos es, Flavor.ReturnVal
  | _ ->
    let need_ref = binary_assignment_rhs_starts_with_ref expr in
    let flavor = if need_ref then Flavor.Ref else Flavor.Cell in
    emit_expr ?last_pos ~need_ref env expr, flavor

and emit_final_member_op stack_index op mk =
  match op with
  | LValOp.Set -> instr (IFinal (SetM (stack_index, mk)))
  | LValOp.SetRef -> instr (IFinal (BindM (stack_index, mk)))
  | LValOp.SetOp op -> instr (IFinal (SetOpM (stack_index, op, mk)))
  | LValOp.IncDec op -> instr (IFinal (IncDecM (stack_index, op, mk)))
  | LValOp.Unset -> instr (IFinal (UnsetM (stack_index, mk)))

and emit_final_local_op pos op lid =
  emit_pos_then pos @@
  match op with
  | LValOp.Set -> instr (IMutator (SetL lid))
  | LValOp.SetRef -> instr (IMutator (BindL lid))
  | LValOp.SetOp op -> instr (IMutator (SetOpL (lid, op)))
  | LValOp.IncDec op -> instr (IMutator (IncDecL (lid, op)))
  | LValOp.Unset -> instr (IMutator (UnsetL lid))

and emit_final_named_local_op pos op =
  match op with
  | LValOp.Set -> emit_pos_then pos @@ instr (IMutator SetN)
  | LValOp.SetRef -> instr (IMutator BindN)
  | LValOp.SetOp op -> instr (IMutator (SetOpN op))
  | LValOp.IncDec op -> instr (IMutator (IncDecN op))
  | LValOp.Unset -> instr (IMutator UnsetN)

and emit_final_global_op pos op =
  match op with
  | LValOp.Set -> emit_pos_then pos @@ instr (IMutator SetG)
  | LValOp.SetRef -> instr (IMutator BindG)
  | LValOp.SetOp op -> instr (IMutator (SetOpG op))
  | LValOp.IncDec op -> instr (IMutator (IncDecG op))
  | LValOp.Unset -> emit_pos_then pos @@ instr (IMutator UnsetG)

and emit_final_static_op cid prop op =
  match op with
  | LValOp.Set -> instr (IMutator (SetS 0))
  | LValOp.SetRef -> instr (IMutator (BindS 0))
  | LValOp.SetOp op -> instr (IMutator (SetOpS (op, 0)))
  | LValOp.IncDec op -> instr (IMutator (IncDecS (op, 0)))
  | LValOp.Unset ->
    let cid = text_of_expr cid in
    let id = text_of_expr prop in
    Emit_fatal.emit_fatal_runtime (fst id)
      ("Attempt to unset static property " ^ snd cid ^ "::" ^ snd id)

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

and can_use_as_rhs_in_list_assignment expr =
  match expr with
  | A.Call ((_, A.Id (_, s)), _, _, _) when String.lowercase_ascii s = "echo" ->
    false
  | A.Lvar _
  | A.Dollar _
  | A.Array_get _
  | A.Obj_get _
  | A.Class_get _
  | A.Call _
  | A.New _
  | A.Expr_list _
  | A.Yield _
  | A.Cast _
  | A.Eif _
  | A.Array _
  | A.Varray _
  | A.Darray _
  | A.Collection _
  | A.Clone _
  | A.Unop _
  | A.Await _ -> true
  | A.Pipe (_, (_, r))
  | A.Binop ((A.Eq None), (_, A.List _), (_, r)) ->
    can_use_as_rhs_in_list_assignment r
  | A.Binop (A.Plus, _, _)
  | A.Binop (A.QuestionQuestion, _, _)
  | A.Binop (A.Eq _, _, _) -> true
  | _ -> false


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
and emit_lval_op_list ?(last_usage=false) env outer_pos local indices expr =
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
          |> Core_list.foldi ~init:None
            ~f:(fun i acc (_, v) -> if v = A.Omitted then acc else Some i)
        (* in right-to-left case result list will be reversed
           so we need to find first non-omitted expression *)
        else
          exprs
          |> Core_list.findi ~f:(fun _ (_, v) -> v <> A.Omitted)
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
  | _, A.Unop (A.Uref, _) -> true
  | _ -> false

and binary_assignment_rhs_starts_with_ref = function
  | _, A.Binop (A.Eq None, _, e) when expr_starts_with_ref e -> true
  | _ -> false

(* Emit code for an l-value operation *)
and emit_lval_op ?(null_coalesce_assignment=false) ~need_ref env pos op expr1 opt_expr2 =
  let op, make_ref =
    match op, opt_expr2 with
    | LValOp.Set, Some e when expr_starts_with_ref e -> LValOp.SetRef, true
    | _ -> op, false
  in
  match op, expr1, opt_expr2 with
    (* Special case for list destructuring, only on assignment *)
    | LValOp.Set, (_, A.List l), Some expr2 ->
      let has_elements =
        List.exists l ~f: (function
          | _, A.Omitted -> false
          | _ -> true)
      in
      if has_elements then
        stash_in_local_with_prefix ~need_ref ~always_stash:(php7_ltr_assign ())
          ~leave_on_stack:true env pos expr2
        begin fun local _break_label ->
          let local =
            if can_use_as_rhs_in_list_assignment (snd expr2) then
              Some local
            else
              None
          in
            emit_lval_op_list env pos local [] expr1
        end
      else
        emit_expr ~need_ref env expr2
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
          | Some (pos, A.Unop (A.Uref, (_, A.Obj_get (_, _, A.OG_nullsafe)
                                    | _, A.Array_get ((_,
                                      A.Obj_get (_, _, A.OG_nullsafe)), _)))) ->
            Emit_fatal.raise_fatal_runtime
              pos "?-> is not allowed in write context"
          | Some e -> emit_expr ~need_ref:make_ref env e, 1
        in
        gather [
          emit_lval_op_nonlist ~null_coalesce_assignment env pos op expr1 rhs_instrs rhs_stack_size;
          match need_ref, make_ref with
            | false, true -> emit_pos_then pos instr_unbox
            | true, false -> emit_pos_then pos instr_box
            | _ -> empty
        ]

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
  env outer_pos op (pos, expr_) rhs_instrs rhs_stack_size =
  let env =
  match op with
  (* Unbelieveably, $test[] += 5; is legal in PHP, but $test[] = $test[] + 5 is not *)
  | LValOp.SetRef
  | LValOp.Set
  | LValOp.SetOp _
  | LValOp.IncDec _ -> { env with Emit_env.env_allows_array_append = true }
  | _ -> env in
  let handle_dollar e final_op =
    match e with
      _, A.Lvar id ->
      let instruction =
        let local = (get_local env id) in
        match op with
        | LValOp.Unset | LValOp.IncDec _ -> instr_cgetl local
        | _ -> instr_cgetl2 local
      in
      empty,
      rhs_instrs,
      gather [
        emit_pos outer_pos;
        instruction;
        final_op op
      ]
    | _ ->

      let instrs = emit_expr ~need_ref:false env e in
      instrs,
      rhs_instrs,
      final_op op
  in
  match expr_ with
  | A.Lvar (name_pos, id) when SN.Superglobals.is_superglobal id ->
    emit_pos_then name_pos @@ instr_string @@ SU.Locals.strip_dollar id,
    rhs_instrs,
    emit_final_global_op outer_pos op

  | A.Lvar ((_, str) as id) when is_local_this env str && is_incdec op ->
    emit_local ~notice:Notice ~need_ref:false env id,
    rhs_instrs,
    empty

  | A.Lvar id when not (is_local_this env (snd id)) || op = LValOp.Unset ->
    empty,
    rhs_instrs,
    emit_final_local_op outer_pos op (get_local env id)

  | A.Dollar e ->
    handle_dollar e (emit_final_named_local_op pos)

  | A.Array_get ((_, A.Lvar (_, x)), Some e) when x = SN.Superglobals.globals ->
    let final_global_op_instrs = emit_final_global_op pos op in
    if rhs_stack_size = 0
    then
      emit_expr ~need_ref:false env e,
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
      emit_base ~notice:Notice ~is_object:false ~null_coalesce_assignment env
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
    if null_flavor = A.OG_nullsafe then
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
    let cexpr = expr_to_class_expr ~resolve_self:false
      (Emit_env.get_scope env) cid in
    begin match snd prop with
    | A.Dollar (_, A.Lvar _ as e) ->
      let final_instr = emit_final_static_op cid prop op in
      begin match op with
      | LValOp.IncDec _ ->
         emit_load_class_ref env pos cexpr,
         rhs_instrs,
         gather [
           emit_expr ~need_ref:false env e;
           final_instr
         ]
      | _ ->
        let instrs, under_top = emit_first_expr env e in
        if under_top
        then
          emit_load_class_ref env pos cexpr,
          rhs_instrs,
          gather [instrs; final_instr]
        else
          gather [instrs; emit_load_class_ref env pos cexpr],
          rhs_instrs,
          final_instr
      end
    | _ ->
      let final_instr =
        emit_pos_then pos @@
        emit_final_static_op cid prop op in
      of_pair @@ emit_class_expr ~null_coalesce_assignment env cexpr prop,
      rhs_instrs,
      final_instr
    end

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
  let ints_overflow_to_ints =
    Hhbc_options.ints_overflow_to_ints !Hhbc_options.compiler_options
  in
  match op with
  | A.Utild -> instr (IOp BitNot)
  | A.Unot -> instr (IOp Not)
  | A.Uplus -> instr (IOp (if ints_overflow_to_ints then Add else AddO))
  | A.Uminus -> instr (IOp (if ints_overflow_to_ints then Sub else SubO))
  | A.Uincr | A.Udecr | A.Upincr | A.Updecr | A.Uref | A.Usilence ->
    failwith "this unary operation cannot be translated"

and emit_expr_as_ref env e =
  emit_expr ~need_ref:true { env with Emit_env.env_allows_array_append = true} e

and emit_unop ~need_ref env pos op e =
  match op with
  | A.Utild ->
    emit_box_if_necessary pos need_ref @@ gather [
      emit_expr ~last_pos:pos ~need_ref:false env e;
      emit_pos_then pos @@ from_unop op
    ]
  | A.Unot ->
    emit_box_if_necessary pos need_ref @@ gather [
      emit_expr ~last_pos:pos ~need_ref:false env e;
      emit_pos_then pos @@ from_unop op
    ]
  | A.Uplus ->
    emit_box_if_necessary pos need_ref @@ gather [
      emit_pos pos;
      instr (ILitConst (Int (Int64.zero)));
      emit_expr ~last_pos:pos ~need_ref:false env e;
      emit_pos_then pos @@ from_unop op
    ]
  | A.Uminus ->
    emit_box_if_necessary pos need_ref @@ gather [
      emit_pos pos;
      instr (ILitConst (Int (Int64.zero)));
      emit_expr ~last_pos:pos ~need_ref:false env e;
      emit_pos_then pos @@ from_unop op
    ]
  | A.Uincr | A.Udecr | A.Upincr | A.Updecr ->
    emit_lval_op ~need_ref env pos (LValOp.IncDec (unop_to_incdec_op op)) e None
  | A.Uref -> emit_expr_as_ref env e
  | A.Usilence ->
    Local.scope @@ fun () ->
      let enclosing_span = Ast_scope.Scope.get_span env.Emit_env.env_scope in
      let fault_label = Label.next_fault () in
      let temp_local = Local.get_unnamed_local () in
      let cleanup = instr_silence_end temp_local in
      let body =
        gather [emit_expr ~need_ref:false env e; emit_pos pos; cleanup] in
      let fault = gather [emit_pos enclosing_span; cleanup; instr_unwind] in
      emit_box_if_necessary pos need_ref @@ gather [
        emit_pos pos;
        instr_silence_start temp_local;
        instr_try_fault fault_label body fault
      ]

and emit_exprs env pos exprs =
  match exprs with
  | [] -> empty
  | expr::exprs ->
    gather (emit_expr ~last_pos:pos ~need_ref:false env expr ::
      List.map exprs (emit_expr ~last_pos:pos ~need_ref:false env))

(* allows to create a block of code that will
- get a fresh temporary local
- be wrapped in a try/fault where fault will clean temporary from the previous
  bulletpoint*)
and with_temp_local temp f =
  let _, block =
    with_temp_local_with_prefix temp (fun temp label -> empty, f temp label) in
  block

(* similar to with_temp_local with addition that
  function 'f' that creates result block of code can generate an
  additional prefix instruction sequence that should be
  executed before the result block *)
and with_temp_local_with_prefix temp f =
  let break_label = Label.next_regular () in
  let prefix, block = f temp break_label in
  if is_empty block then prefix, block
  else
    let fault_label = Label.next_fault () in
    prefix,
    gather [
      instr_try_fault
        fault_label
        (* try block *)
        block
        (* fault block *)
        (gather [
          instr_unsetl temp;
          instr_unwind ]);
      instr_label break_label;
    ]

(* Similar to stash_in_local with addition that function that
   creates a block of code can yield a prefix instrution
  that will be executed as the first instruction in the result instruction set *)
and stash_in_local_with_prefix ~need_ref ?(always_stash=false)
                   ?(leave_on_stack=false)
                   ?(always_stash_this=false) env pos e f =
  match e with
  | (_, A.Lvar id) when not always_stash
    && not (is_local_this env (snd id) &&
    ((Emit_env.get_needs_local_this env) || always_stash_this)) ->
    let break_label = Label.next_regular () in
    let prefix_instr, result_instr =
      f (get_local env id) break_label in
    gather [
      prefix_instr;
      result_instr;
      instr_label break_label;
      if leave_on_stack then
        (if need_ref then instr_vgetl else instr_cgetl) (get_local env id)
      else
        empty;
    ]
  | _ ->
    let generate_value =
      Local.scope @@ fun () -> emit_expr ~need_ref env e in
    Local.scope @@ fun () ->
      let temp = Local.get_unnamed_local () in
      let prefix_instr, result_instr =
        with_temp_local_with_prefix temp f in
      gather [
        prefix_instr;
        generate_value;
        if need_ref then gather [
          instr_bindl temp;
          instr_popv;
          result_instr;
          emit_pos pos;
          if leave_on_stack then instr_vgetl temp else empty;
          instr_unsetl temp
        ] else gather [
          instr_setl temp;
          instr_popc;
          result_instr;
          emit_pos pos;
          if leave_on_stack then instr_pushl temp else instr_unsetl temp
        ]
      ]
(* Generate code to evaluate `e`, and, if necessary, store its value in a
 * temporary local `temp` (unless it is itself a local). Then use `f` to
 * generate code that uses this local and branches or drops through to
 * `break_label`:
 *    temp := e
 *    <code generated by `f temp break_label`>
 *  break_label:
 *    push `temp` on stack if `leave_on_stack` is true.
 *)
and stash_in_local ?(always_stash=false) ?(leave_on_stack=false)
                   ?(always_stash_this=false) env pos e f =
  stash_in_local_with_prefix ~need_ref:false ~always_stash ~leave_on_stack
    ~always_stash_this env pos e (fun temp label -> empty, f temp label)
