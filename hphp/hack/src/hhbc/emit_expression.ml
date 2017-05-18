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
open Instruction_sequence
open Ast_class_expr

module A = Ast
module H = Hhbc_ast
module TC = Hhas_type_constraint
module SN = Naming_special_names
module CBR = Continue_break_rewriter
module SU = Hhbc_string_utils

(* When using the PassX instructions we need to emit the right kind *)
module PassByRefKind = struct
  type t = AllowCell | WarnOnCell | ErrorOnCell
end

(* Locals, array elements, and properties all support the same range of l-value
 * operations. *)
module LValOp = struct
  type t =
  | Set
  | SetOp of eq_op
  | IncDec of incdec_op
  | Unset
end

let special_functions =
  ["isset"; "empty"; "tuple"; "idx"; "hphp_array_idx"; "define"]

(* Context for code generation. It would be more elegant to pass this
 * around in an environment parameter. *)
let scope = ref Ast_scope.Scope.toplevel
let set_scope s = scope := s
let get_scope () = !scope

let namespace = ref Namespace_env.empty_with_default_popt
let get_namespace () = !namespace
let set_namespace ns = namespace := ns

let needs_local_this = ref false
let get_needs_local_this () = !needs_local_this
let set_needs_local_this n = needs_local_this := n

let optimize_null_check () =
  Hhbc_options.optimize_null_check !Hhbc_options.compiler_options

let optimize_cuf () =
  Hhbc_options.optimize_cuf !Hhbc_options.compiler_options

(* Emit a comment in lieu of instructions for not-yet-implemented features *)
let emit_nyi description =
  instr (IComment ("NYI: " ^ description))

let make_varray p es = p, A.Array (List.map es ~f:(fun e -> A.AFvalue e))
let make_kvarray p kvs =
  p, A.Array (List.map kvs ~f:(fun (k, v) -> A.AFkvalue (k, v)))

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
  | A.Eq _ -> emit_nyi "Eq"
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
  | A.Uincr -> Some (if ints_overflow_to_ints then PreInc else PreIncO)
  | A.Udecr -> Some (if ints_overflow_to_ints then PreDec else PreDecO)
  | A.Upincr -> Some (if ints_overflow_to_ints then PostInc else PostIncO)
  | A.Updecr -> Some (if ints_overflow_to_ints then PostDec else PostDecO)
  | _ -> None

let collection_type = function
  | "Vector"    -> 17
  | "Map"       -> 18
  | "Set"       -> 19
  | "Pair"      -> 20
  | "ImmVector" -> 21
  | "ImmMap"    -> 22
  | "ImmSet"    -> 23
  | x -> failwith ("unknown collection type '" ^ x ^ "'")

let istype_op id =
  match id with
  | "is_int" | "is_integer" | "is_long" -> Some OpInt
  | "is_bool" -> Some OpBool
  | "is_float" | "is_real" | "is_double" -> Some OpDbl
  | "is_string" -> Some OpStr
  | "is_array" -> Some OpArr
  | "is_object" -> Some OpObj
  | "is_null" -> Some OpNull
  | "is_scalar" -> Some OpScalar
  | "is_keyset" -> Some OpKeyset
  | "is_dict" -> Some OpDict
  | "is_vec" -> Some OpVec
  | _ -> None

(* See EmitterVisitor::getPassByRefKind in emitter.cpp *)
let get_passByRefKind expr =
  let open PassByRefKind in
  let rec from_non_list_assignment permissive_kind expr =
    match snd expr with
    | A.New _ | A.Lvar _ | A.Clone _ -> AllowCell
    | A.Binop(A.Eq None, (_, A.List _), e) ->
      from_non_list_assignment WarnOnCell e
    | A.Array_get(_, Some _) -> permissive_kind
    | A.Binop(A.Eq _, _, _) -> WarnOnCell
    | A.Unop((A.Uincr | A.Udecr), _) -> WarnOnCell
    | A.Unop((A.Usplat, _)) -> AllowCell
    | _ -> ErrorOnCell in
  from_non_list_assignment AllowCell expr

let get_queryMOpMode need_ref op =
  match op with
  | QueryOp.CGet -> MemberOpMode.Warn
  | QueryOp.Empty when need_ref -> MemberOpMode.Define
  | _ -> MemberOpMode.ModeNone

let is_special_function e =
  match e with
  | (_, A.Id(_, x))
    when List.mem special_functions x -> true
  | _ -> false

let is_local_this id =
  let scope = get_scope () in
  id = SN.SpecialIdents.this && Ast_scope.Scope.has_this scope

let extract_shape_field_name_pstring = function
  | A.SFlit p -> A.String p
  | A.SFclass_const (id, p) -> A.Class_const (id, p)

let extract_shape_field_name = function
  | A.SFlit (_, s)
  | A.SFclass_const (_, (_, s)) -> s

let rec expr_and_newc instr_to_add_new instr_to_add = function
  | A.AFvalue e ->
    gather [from_expr ~need_ref:false e; instr_to_add_new]
  | A.AFkvalue (k, v) ->
    gather [
      emit_two_exprs k v;
      instr_to_add
    ]

and emit_local need_ref x =
  let scope = get_scope () in
  if x = SN.SpecialIdents.this &&
    Ast_scope.Scope.has_this scope &&
    not (get_needs_local_this ()) then
    if need_ref then
      instr_vgetl (Local.Named x)
    else
      instr (IMisc (BareThis Notice))
  else
  if x = SN.Superglobals.globals
  then gather [
    instr_string (SU.Locals.strip_dollar x);
    instr (IGet CGetG)
  ]
  else if need_ref then
    instr_vgetl (Local.Named x)
  else
    instr_cgetl (Local.Named x)

(* Emit CGetL2 for local variables, and return true to indicate that
 * the result will be just below the top of the stack *)
and emit_first_expr e =
  match e with
  | (_, A.Lvar (_, local)) when not (is_local_this local) ->
    instr_cgetl2 (Local.Named local), true

  | _ ->
    from_expr ~need_ref:false e, false

(* Special case for binary operations to make use of CGetL2 *)
and emit_two_exprs e1 e2 =
  let instrs1, is_under_top = emit_first_expr e1 in
  let instrs2 = from_expr ~need_ref:false e2 in
  gather @@
    if is_under_top
    then [instrs2; instrs1]
    else [instrs1; instrs2]

and emit_is_null e =
  match e with
  | (_, A.Lvar (_, id)) when not (is_local_this id) ->
    instr_istypel (Local.Named id) OpNull
  | _ ->
    gather [
      from_expr ~need_ref:false e;
      instr_istypec OpNull
    ]

and emit_binop expr op e1 e2 =
  match op with
  | A.AMpamp | A.BArbar -> emit_short_circuit_op expr
  | A.Eq None -> emit_lval_op LValOp.Set e1 (Some e2)
  | A.Eq (Some obop) ->
    begin match binop_to_eqop obop with
    | None -> emit_nyi "illegal eq op"
    | Some op -> emit_lval_op (LValOp.SetOp op) e1 (Some e2)
    end
  | _ ->
    if not (optimize_null_check ())
    then gather [emit_two_exprs e1 e2; from_binop op]
    else
    match op with
    | A.EQeqeq when snd e2 = A.Null ->
      emit_is_null e1
    | A.EQeqeq when snd e1 = A.Null ->
      emit_is_null e2
    | A.Diff2 when snd e2 = A.Null ->
      gather [
        emit_is_null e1;
        instr_not
      ]
    | A.Diff2 when snd e1 = A.Null ->
      gather [
        emit_is_null e2;
        instr_not
      ]
    | _ ->
      gather [
        emit_two_exprs e1 e2;
        from_binop op
      ]

and emit_box_if_necessary need_ref instr =
  if need_ref then
    gather [
      instr;
      instr_box
    ]
  else
    instr

and emit_instanceof e1 e2 =
  match (e1, e2) with
  | (_, (_, A.Id id)) ->
    let id, _ = Hhbc_id.Class.elaborate_id (get_namespace ()) id in
    gather [
      from_expr ~need_ref:false e1;
      instr_instanceofd id ]
  | _ ->
    gather [
      from_expr ~need_ref:false e1;
      from_expr ~need_ref:false e2;
      instr_instanceof ]

and emit_null_coalesce e1 e2 =
  let end_label = Label.next_regular () in
  gather [
    emit_quiet_expr e1;
    instr_dup;
    instr_istypec OpNull;
    instr_not;
    instr_jmpnz end_label;
    instr_popc;
    from_expr ~need_ref:false e2;
    instr_label end_label;
  ]

and emit_cast hint expr =
  let op =
    begin match hint with
    | A.Happly((_, id), []) ->
      let id = String.lowercase_ascii id in
      begin match id with
      | _ when id = SN.Typehints.int
            || id = SN.Typehints.integer -> instr (IOp CastInt)
      | _ when id = SN.Typehints.bool
            || id = SN.Typehints.boolean -> instr (IOp CastBool)
      | _ when id = SN.Typehints.string -> instr (IOp CastString)
      | _ when id = SN.Typehints.object_cast -> instr (IOp CastObject)
      | _ when id = SN.Typehints.array -> instr (IOp CastArray)
      | _ when id = SN.Typehints.real
            || id = SN.Typehints.double
            || id = SN.Typehints.float -> instr (IOp CastDouble)
      | _ -> emit_nyi "cast type"
      end
      (* TODO: unset *)
    | _ ->
      emit_nyi "cast type"
    end in
  gather [
    from_expr ~need_ref:false expr;
    op;
  ]

and emit_conditional_expression etest etrue efalse =
  match etrue with
  | Some etrue ->
    let false_label = Label.next_regular () in
    let end_label = Label.next_regular () in
    gather [
      emit_jmpz etest false_label;
      from_expr ~need_ref:false etrue;
      instr_jmp end_label;
      instr_label false_label;
      from_expr ~need_ref:false efalse;
      instr_label end_label;
    ]
  | None ->
    let end_label = Label.next_regular () in
    gather [
      from_expr ~need_ref:false etest;
      instr_dup;
      instr_jmpnz end_label;
      instr_popc;
      from_expr ~need_ref:false efalse;
      instr_label end_label;
    ]

and emit_new expr args uargs =
  let nargs = List.length args + List.length uargs in
  let cexpr, _ = expr_to_class_expr (get_scope ()) expr in
  match cexpr with
    (* Special case for statically-known class *)
  | Class_id id ->
    let fq_id, _id_opt =
      Hhbc_id.Class.elaborate_id (get_namespace ()) id in
    let push_instr = instr (ICall (FPushCtorD (nargs, fq_id))) in
    gather [
      push_instr;
      emit_args_and_call args uargs;
      instr_popr
    ]
  | _ ->
    gather [
      emit_class_expr cexpr;
      instr_fpushctor nargs 0;
      emit_args_and_call args uargs;
      instr_popr
    ]

and emit_clone expr =
  gather [
    from_expr ~need_ref:false expr;
    instr_clone;
  ]

and emit_shape expr fl =
  let p = fst expr in
  let fl =
    List.map fl
      ~f:(fun (fn, e) ->
            A.AFkvalue ((p, extract_shape_field_name_pstring fn), e))
  in
  from_expr ~need_ref:false (p, A.Array fl)

and emit_tuple p es =
  (* Did you know that tuples are functions? *)
  let af_list = List.map es ~f:(fun e -> A.AFvalue e) in
  from_expr ~need_ref:false (p, A.Array af_list)

and emit_call_expr need_ref expr =
  let instrs, flavor = emit_flavored_expr expr in
  gather [
    instrs;
    (* If the instruction has produced a ref then unbox it *)
    if flavor = Flavor.Ref then
      if need_ref then
        instr_boxr
      else
        instr_unboxr
    else
      empty
  ]

and emit_known_class_id id =
  let fq_id, _ = Hhbc_id.Class.elaborate_id (get_namespace ()) id in
  gather [
    instr_string (Hhbc_id.Class.to_raw_string fq_id);
    instr_clsrefgetc;
  ]

and emit_class_expr cexpr =
  match cexpr with
  | Class_static -> instr (IMisc (LateBoundCls 0))
  | Class_parent -> instr (IMisc (Parent 0))
  | Class_self -> instr (IMisc (Self 0))
  | Class_id id -> emit_known_class_id id
  | Class_expr (_, A.Lvar (_, id)) ->
    instr (IGet (ClsRefGetL (Local.Named id, 0)))
  | Class_expr expr -> gather [from_expr ~need_ref:false expr; instr_clsrefgetc]

and emit_class_get param_num_opt qop cid (_, id) =
  let cexpr, _ = expr_to_class_expr (get_scope ()) (id_to_expr cid) in
    gather [
      (* We need to strip off the initial dollar *)
      instr_string (SU.Locals.strip_dollar id);
      emit_class_expr cexpr;
      match (param_num_opt, qop) with
      | (None, QueryOp.CGet) -> instr_cgets
      | (None, QueryOp.Isset) -> instr_issets
      | (None, QueryOp.Empty) -> instr_emptys
      | (Some i, _) -> instr (ICall (FPassS (i, 0)))
    ]

(* Class constant <cid>::<id>.
 * We follow the logic for the Construct::KindOfClassConstantExpression
 * case in emitter.cpp
 *)
and emit_class_const cid (_, id) =
  let cexpr, _ = expr_to_class_expr (get_scope()) (id_to_expr cid) in
  match cexpr with
  | Class_id cid ->
    let fq_id, _id_opt =
      Hhbc_id.Class.elaborate_id (get_namespace ()) cid in
    if id = SN.Members.mClass
    then instr_string (Hhbc_id.Class.to_raw_string fq_id)
    else instr (ILitConst (ClsCnsD (Hhbc_id.Const.from_ast_name id, fq_id)))
  | _ ->
    gather [
      emit_class_expr cexpr;
      if id = SN.Members.mClass
      then instr (IMisc (ClsRefName 0))
      else instr (ILitConst (ClsCns (Hhbc_id.Const.from_ast_name id, 0)))
    ]

and emit_await e =
  let after_await = Label.next_regular () in
  gather [
    from_expr ~need_ref:false e;
    instr_dup;
    instr_istypec OpNull;
    instr_jmpnz after_await;
    instr_await;
    instr_label after_await;
  ]

and emit_yield = function
  | A.AFvalue e ->
    gather [
      from_expr ~need_ref:false e;
      instr_yield;
    ]
  | A.AFkvalue (e1, e2) ->
    gather [
      from_expr ~need_ref:false e1;
      from_expr ~need_ref:false e2;
      instr_yieldk;
    ]

and emit_string2 exprs =
  match exprs with
  | [e] ->
    gather [
      from_expr ~need_ref:false e;
      instr (IOp CastString)
    ]
  | e1::e2::es ->
    gather @@ [
      emit_two_exprs e1 e2;
      instr (IOp Concat);
      gather (List.map es (fun e ->
        gather [from_expr ~need_ref:false e; instr (IOp Concat)]))
    ]

  | [] -> failwith "String2 with zero arguments is impossible"


and emit_lambda fundef ids =
  (* Closure conversion puts the class number used for CreateCl in the "name"
   * of the function definition *)
  let class_num = int_of_string (snd fundef.A.f_name) in
  (* Horrid hack: use empty body for implicit closed vars, [Noop] otherwise *)
  let explicit_use = match fundef.A.f_body with [] -> false | _ -> true in
  gather [
    gather @@ List.map ids
      (fun (x, isref) ->
        instr (IGet (
          let lid = Local.Named (snd x) in
          if explicit_use
          then
            if isref then VGetL lid else CGetL lid
          else CUGetL lid)));
    instr (IMisc (CreateCl (List.length ids, class_num)))
  ]

and emit_id (p, s as id) =
  match s with
  | "__FILE__" -> instr (ILitConst File)
  | "__DIR__" -> instr (ILitConst Dir)
  | "__LINE__" ->
    (* If the expression goes on multi lines, we return the last line *)
    let _, line, _, _ = Pos.info_pos_extended p in
    instr_int line
  | "__NAMESPACE__" ->
    let ns = get_namespace () in
    instr_string (Option.value ~default:"" ns.Namespace_env.ns_name)
  | _ ->
    let fq_id, id_opt, contains_backslash =
      Hhbc_id.Const.elaborate_id (get_namespace ()) id in
    begin match id_opt with
    | Some id -> instr (ILitConst (CnsU (fq_id, id)))
    | None -> instr (ILitConst
        (if contains_backslash then CnsE fq_id else Cns fq_id))
    end

and rename_xhp (p, s) = (p, SU.Xhp.mangle s)

and emit_xhp p id attributes children =
  (* Translate into a constructor call. The arguments are:
   *  1) shape-like array of attributes
   *  2) vec-like array of children
   *  3) filename, for debugging
   *  4) line number, for debugging
   *)
  let convert_xml_attr (name, v) = (A.SFlit name, v) in
  let attributes = List.map ~f:convert_xml_attr attributes in
  let attribute_map = p, A.Shape attributes in
  let children_vec = make_varray p children in
  let filename = p, A.Id (p, "__FILE__") in
  let line = p, A.Id (p, "__LINE__") in
  from_expr ~need_ref:false @@
    (p, A.New (
      (p, A.Id (rename_xhp id)),
      [attribute_map ; children_vec ; filename ; line],
      []))

and emit_import flavor e =
  let import_instr = match flavor with
    | A.Include -> instr @@ IIncludeEvalDefine Incl
    | A.Require -> instr @@ IIncludeEvalDefine Req
    | A.IncludeOnce -> instr @@ IIncludeEvalDefine InclOnce
    | A.RequireOnce -> instr @@ IIncludeEvalDefine ReqOnce
  in
  gather [
    from_expr ~need_ref:false e;
    import_instr;
  ]

and emit_lvarvar need_ref n (_, id) =
  gather [
    instr_cgetl (Local.Named id);
    if need_ref then
      let prefix =
        if n = 1 then empty
        else gather @@ List.replicate ~num:(n - 1) instr_cgetn
      in
      gather [
        prefix;
        instr_vgetn
      ]
    else
      gather @@ List.replicate ~num:n instr_cgetn
  ]

and emit_call_isset_expr (_, expr_ as expr) =
  match expr_ with
  | A.Array_get ((_, A.Lvar (_, x)), Some e) when x = SN.Superglobals.globals ->
    gather [
      from_expr ~need_ref:false e;
      instr (IIsset IssetG)
    ]
  | A.Array_get (base_expr, opt_elem_expr) ->
    emit_array_get false None QueryOp.Isset base_expr opt_elem_expr
  | A.Class_get (cid, id)  ->
    emit_class_get None QueryOp.Isset cid id
  | A.Obj_get (expr, prop, nullflavor) ->
    emit_obj_get false None QueryOp.Isset expr prop nullflavor
  | A.Lvar(_, id) when is_local_this id ->
    gather [
      instr (IMisc (BareThis NoNotice));
      instr_istypec OpNull;
      instr_not
    ]
  | A.Lvar(_, id) ->
    instr (IIsset (IssetL (Local.Named id)))
  | _ ->
    gather [
      from_expr ~need_ref:false expr;
      instr_istypec OpNull;
      instr_not
    ]

and emit_call_empty_expr (_, expr_ as expr) =
  match expr_ with
  | A.Array_get((_, A.Lvar (_, x)), Some e) when x = SN.Superglobals.globals ->
    gather [
      from_expr ~need_ref:false e;
      instr (IIsset EmptyG)
    ]
  | A.Array_get(base_expr, opt_elem_expr) ->
    emit_array_get false None QueryOp.Empty base_expr opt_elem_expr
  | A.Class_get (cid, id) ->
    emit_class_get None QueryOp.Empty cid id
  | A.Obj_get (expr, prop, nullflavor) ->
    emit_obj_get false None QueryOp.Empty expr prop nullflavor
  | A.Lvar(_, id) when not (is_local_this id) ->
    instr (IIsset (EmptyL (Local.Named id)))
  | _ ->
    gather [
      from_expr ~need_ref:false expr;
      instr_not
    ]

and emit_unset_expr expr =
  emit_lval_op_nonlist LValOp.Unset expr empty 0

and emit_call_isset_exprs exprs =
  match exprs with
  | [] -> emit_nyi "isset()"
  | [expr] -> emit_call_isset_expr expr
  | _ ->
    let n = List.length exprs in
    let its_done = Label.next_regular () in
      gather [
        gather @@
        List.mapi exprs
        begin fun i expr ->
          gather [
            emit_call_isset_expr expr;
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

and emit_exit expr_opt =
  gather [
    (match expr_opt with
      | None -> instr_int 0
      | Some e -> from_expr ~need_ref:false e);
    instr_exit;
  ]

and is_valid_idx ~is_array es =
  let n = List.length es in
  if is_array then n = 3 else n = 2 || n = 3

and emit_idx ~is_array es =
  let default = if List.length es = 2 then instr_null else empty in
  gather [
    from_exprs es;
    default;
    if is_array then instr_array_idx else instr_idx;
  ]

and from_expr expr ~need_ref =
  (* Note that this takes an Ast.expr, not a Nast.expr. *)
  match snd expr with
  | A.Float (_, litstr) ->
    emit_box_if_necessary need_ref @@ instr_double litstr
  | A.String (_, litstr) ->
    emit_box_if_necessary need_ref @@ instr_string litstr
  (* TODO deal with integer out of range *)
  | A.Int (_, litstr) ->
    emit_box_if_necessary need_ref @@ instr_int_of_string litstr
  | A.Null ->
    emit_box_if_necessary need_ref @@ instr_null
  | A.False ->
    emit_box_if_necessary need_ref @@ instr_false
  | A.True ->
    emit_box_if_necessary need_ref @@ instr_true
  | A.Lvar (_, x) ->
    emit_local need_ref x
  | A.Class_const (cid, id) ->
    emit_class_const cid id
  | A.Unop (op, e) ->
    emit_unop need_ref op e
  | A.Binop (op, e1, e2) ->
    emit_box_if_necessary need_ref @@ emit_binop expr op e1 e2
  | A.Pipe (e1, e2) ->
    emit_box_if_necessary need_ref @@ emit_pipe e1 e2
  | A.Dollardollar ->
    instr_cgetl2 Local.Pipe
  | A.InstanceOf (e1, e2) ->
    emit_box_if_necessary need_ref @@ emit_instanceof e1 e2
  | A.NullCoalesce (e1, e2) ->
    emit_box_if_necessary need_ref @@ emit_null_coalesce  e1 e2
  | A.Cast((_, hint), e) ->
    emit_box_if_necessary need_ref @@ emit_cast hint e
  | A.Eif (etest, etrue, efalse) ->
    emit_box_if_necessary need_ref @@
      emit_conditional_expression etest etrue efalse
  | A.Expr_list es -> gather @@ List.map es ~f:(from_expr ~need_ref:false)
  | A.Array_get((_, A.Lvar (_, x)), Some e) when x = SN.Superglobals.globals ->
    gather [
      from_expr ~need_ref:false e;
      instr (IGet (if need_ref then VGetG else CGetG))
    ]
  | A.Array_get((_, A.Lvarvar (n, (_, x))), Some e)
    when x = SN.Superglobals.globals ->
    gather [
      from_expr ~need_ref:false e;
      instr (IGet CGetG);
      if n = 1 then
        empty
      else
        gather @@ List.replicate (n - 1) instr_cgetn;
      if need_ref then
        instr_vgetn
      else
        instr_cgetn
    ]
  | A.Array_get(base_expr, opt_elem_expr) ->
    let query_op = if need_ref then QueryOp.Empty else QueryOp.CGet in
    emit_array_get need_ref None query_op base_expr opt_elem_expr
  | A.Obj_get (expr, prop, nullflavor) ->
    let query_op = if need_ref then QueryOp.Empty else QueryOp.CGet in
    emit_obj_get need_ref None query_op expr prop nullflavor
  | A.Call ((_, A.Id (_, "isset")), exprs, []) ->
    emit_box_if_necessary need_ref @@ emit_call_isset_exprs exprs
  | A.Call ((_, A.Id (_, "empty")), [expr], []) ->
    emit_box_if_necessary need_ref @@ emit_call_empty_expr expr
  | A.Call ((p, A.Id (_, "tuple")), es, _) ->
    emit_box_if_necessary need_ref @@ emit_tuple p es
  | A.Call ((_, A.Id (_, "idx")), es, _) when is_valid_idx ~is_array:false es ->
    emit_box_if_necessary need_ref @@ emit_idx ~is_array:false es
  | A.Call ((_, A.Id (_, "hphp_array_idx")), es, _)
    when is_valid_idx ~is_array:true es ->
    emit_box_if_necessary need_ref @@  emit_idx ~is_array:true es
  | A.Call ((_, A.Id (_, "define")), [(_, A.String s); expr2], _) ->
    emit_box_if_necessary need_ref @@ gather [
      from_expr ~need_ref:false expr2;
      instr (IIncludeEvalDefine
        (DefCns (Hhbc_id.Const.from_raw_string (snd s))))
    ]
  | A.Call _ -> emit_call_expr need_ref expr
  | A.New (typeexpr, args, uargs) ->
    emit_box_if_necessary need_ref @@ emit_new typeexpr args uargs
  | A.Array es ->
    emit_box_if_necessary need_ref @@ emit_collection expr es
  | A.Darray es ->
    es
      |> List.map ~f:(fun (e1, e2) -> A.AFkvalue (e1, e2))
      |> emit_collection expr
  | A.Varray es ->
    es
      |> List.map ~f:(fun e -> A.AFvalue e)
      |> emit_collection expr
  | A.Collection ((pos, name), fields) ->
    emit_box_if_necessary need_ref @@ emit_named_collection expr pos name fields
  | A.Clone e ->
    emit_box_if_necessary need_ref @@ emit_clone e
  | A.Shape fl ->
    emit_box_if_necessary need_ref @@ emit_shape expr fl
  | A.Await e -> emit_await e
  | A.Yield e -> emit_yield e
  | A.Yield_break ->
    failwith "yield break should be in statement position"
  | A.Lfun _ ->
    failwith "expected Lfun to be converted to Efun during closure conversion"
  | A.Efun (fundef, ids) -> emit_lambda fundef ids
  | A.Class_get (cid, id)  -> emit_class_get None QueryOp.CGet cid id
  | A.String2 es -> emit_string2 es
  | A.Unsafeexpr e ->
    let instr = from_expr ~need_ref:false e in
    if need_ref then
      gather [
        instr;
        instr_vgetn
      ]
    else
      gather [
        instr;
        instr_cgetn
      ]
  | A.Id id -> emit_id id
  | A.Xml (id, attributes, children) ->
    emit_xhp (fst expr) id attributes children
  | A.Import (flavor, e) -> emit_import flavor e
  | A.Lvarvar (n, id) -> emit_lvarvar need_ref n id
  | A.Id_type_arguments (id, _) -> emit_id id
  | A.List _ ->
    failwith "List destructor can only be used as an lvar"

and emit_static_collection ~transform_to_collection tv =
  let lit_constructor =
    match tv with
    | Typed_value.Array _ -> Array tv
    | Typed_value.Dict _ -> Dict tv
    | Typed_value.Vec _ -> Vec tv
    | Typed_value.Keyset _ -> Keyset tv
    | _ -> failwith "emit_static_collection: unexpected collection type"
  in
  let transform_instr =
    match transform_to_collection with
    | Some n -> instr_colfromarray n
    | None -> empty
  in
  gather [
    instr (ILitConst lit_constructor);
    transform_instr;
  ]

(* transform_to_collection argument keeps track of
 * what collection to transform to *)
and emit_dynamic_collection ~transform_to_collection expr es =
  let is_only_values =
    List.for_all es ~f:(function A.AFkvalue _ -> false | _ -> true)
  in
  let are_all_keys_strings =
    List.for_all es ~f:(function A.AFkvalue ((_, A.String (_, _)), _) -> true
                               | _ -> false)
  in
  let is_array = match snd expr with A.Array _ -> true | _ -> false in
  let count = List.length es in
  if is_only_values && transform_to_collection = None then begin
    let lit_constructor = match snd expr with
      | A.Array _ -> NewPackedArray count
      | A.Collection ((_, "vec"), _) -> NewVecArray count
      | A.Collection ((_, "keyset"), _) -> NewKeysetArray count
      | _ ->
        failwith "emit_dynamic_collection (values only): unexpected expression"
    in
    gather [
      gather @@
      List.map es
        ~f:(function
          | A.AFvalue e -> from_expr ~need_ref:false e
          | _ -> failwith "impossible");
      instr @@ ILitConst lit_constructor;
    ]
  end else if are_all_keys_strings && is_array then begin
    let es =
      List.map es
        ~f:(function
          | A.AFkvalue ((_, A.String (_, s)), v) ->
            s, from_expr ~need_ref:false v
          | _ -> failwith "impossible")
    in
    gather [
      gather @@ List.map es ~f:snd;
      instr_newstructarray @@ List.map es ~f:fst;
    ]
  end else begin
    let lit_constructor = match snd expr with
      | A.Array _ -> NewMixedArray count
      | A.Collection ((_, ("dict" | "Set" | "ImmSet" | "Map" | "ImmMap")), _) ->
        NewDictArray count
      | _ -> failwith "emit_dynamic_collection: unexpected expression"
    in
    let transform_instr =
      match transform_to_collection with
      | Some n -> instr_colfromarray n
      | None -> empty
    in
    let add_elem_instr =
      if transform_to_collection = None
      then instr_add_new_elemc
      else gather [instr_dup; instr_add_elemc]
    in
    gather [
      instr (ILitConst lit_constructor);
      gather (List.map es ~f:(expr_and_newc add_elem_instr instr_add_elemc));
      transform_instr;
    ]
  end

and emit_named_collection expr pos name fields =
  match name with
  | "dict" | "vec" | "keyset" -> emit_collection expr fields
  | "Vector" | "ImmVector" ->
    let collection_type = collection_type name in
    if fields = []
    then instr_newcol collection_type
    else
    gather [
      emit_collection (pos, A.Collection ((pos, "vec"), fields)) fields;
      instr_colfromarray collection_type;
    ]
  | "Set" | "ImmSet" ->
    let collection_type = collection_type name in
    if fields = []
    then instr_newcol collection_type
    else
      emit_dynamic_collection
        ~transform_to_collection:(Some collection_type)
        expr
        fields
  | "Map" | "ImmMap" ->
    let collection_type = collection_type name in
    if fields = []
    then instr_newcol collection_type
    else
      emit_collection
        ~transform_to_collection:collection_type
        expr
        fields
  | "Pair" ->
    gather [
      gather (List.map fields (function
        | A.AFvalue e -> from_expr ~need_ref:false e
        | _ -> failwith "impossible Pair argument"));
      instr (ILitConst NewPair);
    ]
  | _ -> failwith @@ "collection: " ^ name ^ " does not exist"

and emit_collection ?(transform_to_collection) expr es =
  match Ast_constant_folder.expr_to_opt_typed_value (get_namespace ()) expr with
  | Some tv ->
    emit_static_collection ~transform_to_collection tv
  | None ->
    emit_dynamic_collection ~transform_to_collection expr es

and emit_pipe e1 e2 =
  stash_in_local e1
  begin fun temp _break_label ->
  let rewrite_dollardollar e =
    let rewriter i =
      match i with
      | IGet (CGetL2 Local.Pipe) ->
        IGet (CGetL2 temp)
      | _ -> i in
    InstrSeq.map e ~f:rewriter in
  rewrite_dollardollar (from_expr ~need_ref:false e2)
  end

(* Emit code that is equivalent to
 *   <code for expr>
 *   JmpZ label
 * Generate specialized code in case expr is statically known, and for
 * !, && and || expressions
 *)
and emit_jmpz (_, expr_ as expr) label =
  let opt = optimize_null_check () in
  match Ast_constant_folder.expr_to_opt_typed_value (get_namespace()) expr with
  | Some v ->
    if Typed_value.to_bool v then empty else instr_jmp label
  | None ->
    match expr_ with
    | A.Unop(A.Unot, e) ->
      emit_jmpnz e label
    | A.Binop(A.BArbar, e1, e2) ->
      let skip_label = Label.next_regular () in
      gather [
        emit_jmpnz e1 skip_label;
        emit_jmpz e2 label;
        instr_label skip_label;
      ]
    | A.Binop(A.AMpamp, e1, e2) ->
      gather [
        emit_jmpz e1 label;
        emit_jmpz e2 label;
      ]
    | A.Binop(A.EQeqeq, e, (_, A.Null))
    | A.Binop(A.EQeqeq, (_, A.Null), e) when opt ->
      gather [
        emit_is_null e;
        instr_jmpz label
      ]
    | A.Binop(A.Diff2, e, (_, A.Null))
    | A.Binop(A.Diff2, (_, A.Null), e) when opt ->
      gather [
        emit_is_null e;
        instr_jmpnz label
      ]
    | _ ->
      gather [
        from_expr ~need_ref:false expr;
        instr_jmpz label
      ]

(* Emit code that is equivalent to
 *   <code for expr>
 *   JmpNZ label
 * Generate specialized code in case expr is statically known, and for
 * !, && and || expressions
 *)
and emit_jmpnz (_, expr_ as expr) label =
  let opt = optimize_null_check () in
  match Ast_constant_folder.expr_to_opt_typed_value (get_namespace ()) expr with
  | Some v ->
    if Typed_value.to_bool v then instr_jmp label else empty
  | None ->
    match expr_ with
    | A.Unop(A.Unot, e) ->
      emit_jmpz e label
    | A.Binop(A.BArbar, e1, e2) ->
      gather [
        emit_jmpnz e1 label;
        emit_jmpnz e2 label;
      ]
    | A.Binop(A.AMpamp, e1, e2) ->
      let skip_label = Label.next_regular () in
      gather [
        emit_jmpz e1 skip_label;
        emit_jmpnz e2 label;
        instr_label skip_label;
      ]
    | A.Binop(A.EQeqeq, e, (_, A.Null))
    | A.Binop(A.EQeqeq, (_, A.Null), e) when opt ->
      gather [
        emit_is_null e;
        instr_jmpnz label
      ]
    | A.Binop(A.Diff2, e, (_, A.Null))
    | A.Binop(A.Diff2, (_, A.Null), e) when opt ->
      gather [
        emit_is_null e;
        instr_jmpz label
      ]
    | _ ->
      gather [
        from_expr ~need_ref:false expr;
        instr_jmpnz label
      ]

and emit_short_circuit_op expr =
  let its_true = Label.next_regular () in
  let its_done = Label.next_regular () in
  gather [
    emit_jmpnz expr its_true;
    instr_false;
    instr_jmp its_done;
    instr_label its_true;
    instr_true;
    instr_label its_done ]

and emit_quiet_expr (_, expr_ as expr) =
  match expr_ with
  | A.Lvar (_, x) when not (is_local_this x) ->
    instr_cgetquietl (Local.Named x)
  | _ ->
    from_expr ~need_ref:false expr

(* Emit code for e1[e2] or isset(e1[e2]).
 * If param_num_opt = Some i
 * then this is the i'th parameter to a function
 *)
and emit_array_get need_ref param_num_opt qop base_expr opt_elem_expr =
  let mode = get_queryMOpMode need_ref qop in
  let elem_expr_instrs, elem_stack_size = emit_elem_instrs opt_elem_expr in
  let base_expr_instrs, base_setup_instrs, base_stack_size =
    emit_base mode elem_stack_size param_num_opt base_expr in
  let mk = get_elem_member_key 0 opt_elem_expr in
  let total_stack_size = elem_stack_size + base_stack_size in
  let final_instr =
    instr (IFinal (
      match param_num_opt with
      | None ->
        if need_ref then
          VGetM (total_stack_size, mk)
        else
          QueryM (total_stack_size, qop, mk)
      | Some i -> FPassM (i, total_stack_size, mk)
    )) in
  gather [
    base_expr_instrs;
    elem_expr_instrs;
    base_setup_instrs;
    final_instr
  ]

(* Emit code for e1->e2 or e1?->e2 or isset(e1->e2).
 * If param_num_opt = Some i
 * then this is the i'th parameter to a function
 *)
and emit_obj_get need_ref param_num_opt qop expr prop null_flavor =
  let mode = get_queryMOpMode need_ref qop in
  let prop_expr_instrs, prop_stack_size = emit_prop_instrs prop in
  let base_expr_instrs, base_setup_instrs, base_stack_size =
    emit_base mode prop_stack_size param_num_opt expr in
  let mk = get_prop_member_key null_flavor 0 prop in
  let total_stack_size = prop_stack_size + base_stack_size in
  let final_instr =
    instr (IFinal (
      match param_num_opt with
      | None ->
        if need_ref then
          VGetM (total_stack_size, mk)
        else
          QueryM (total_stack_size, qop, mk)
      | Some i -> FPassM (i, total_stack_size, mk)
    )) in
  gather [
    base_expr_instrs;
    prop_expr_instrs;
    base_setup_instrs;
    final_instr
  ]

and emit_elem_instrs opt_elem_expr =
  match opt_elem_expr with
  (* These all have special inline versions of member keys *)
  | Some (_, (A.Int _ | A.String _)) -> empty, 0
  | Some (_, (A.Lvar (_, id))) when not (is_local_this id) -> empty, 0
  | Some expr -> from_expr ~need_ref:false expr, 1
  | None -> empty, 0

and emit_prop_instrs (_, expr_ as expr) =
  match expr_ with
  (* These all have special inline versions of member keys *)
  | A.Lvar (_, id) when not (is_local_this id) -> empty, 0
  | A.Id _ -> empty, 0
  | _ -> from_expr ~need_ref:false expr, 1

(* Get the member key for an array element expression: the `elem` in
 * expressions of the form `base[elem]`.
 * If the array element is missing, use the special key `W`.
 *)
and get_elem_member_key stack_index opt_expr =
  match opt_expr with
  (* Special case for local *)
  | Some (_, A.Lvar (_, x)) when not (is_local_this x) ->
    MemberKey.EL (Local.Named x)
  (* Special case for literal integer *)
  | Some (_, A.Int (_, str)) -> MemberKey.EI (Int64.of_string str)
  (* Special case for literal string *)
  | Some (_, A.String (_, str)) -> MemberKey.ET str
  (* General case *)
  | Some _ -> MemberKey.EC stack_index
  (* ELement missing (so it's array append) *)
  | None -> MemberKey.W

(* Get the member key for a property *)
and get_prop_member_key null_flavor stack_index prop_expr =
  match prop_expr with
  (* Special case for known property name *)
  | (_, A.Id (_, str)) ->
    let pid = Hhbc_id.Prop.from_ast_name str in
    begin match null_flavor with
    | Ast.OG_nullthrows -> MemberKey.PT pid
    | Ast.OG_nullsafe -> MemberKey.QT pid
    end
  | (_, A.Lvar (_, x)) when not (is_local_this x) ->
    MemberKey.PL (Local.Named x)
  (* General case *)
  | _ -> MemberKey.PC stack_index

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
and emit_base mode base_offset param_num_opt (_, expr_ as expr) =
   let base_mode =
     match mode with
     | MemberOpMode.Unset -> MemberOpMode.ModeNone
     | _ -> mode in
   match expr_ with
   | A.Lvar (_, x) when SN.Superglobals.is_superglobal x ->
     instr_string (SU.Locals.strip_dollar x),
     instr (IBase (
     match param_num_opt with
     | None -> BaseGC (base_offset, base_mode)
     | Some i -> FPassBaseGC (i, base_offset)
     )),
     1

   | A.Lvar (_, x) when x = SN.SpecialIdents.this ->
     instr (IMisc CheckThis),
     instr (IBase BaseH),
     0

   | A.Lvar (_, x) ->
     empty,
     instr (IBase (
       match param_num_opt with
       | None -> BaseL (Local.Named x, base_mode)
       | Some i -> FPassBaseL (i, Local.Named x)
       )),
     0
   | A.Array_get((_, A.Lvar (_, x)), Some (_, A.Lvar (_, y)))
     when x = SN.Superglobals.globals ->
     empty,
     instr (IBase (
       match param_num_opt with
       | None -> BaseGL (Local.Named y, base_mode)
       | Some i -> FPassBaseGL (i, Local.Named y)
       )),
     0

   | A.Array_get((_, A.Lvar (_, x)), Some e) when x = SN.Superglobals.globals ->
     let elem_expr_instrs = from_expr ~need_ref:false e in
     elem_expr_instrs,
     instr (IBase (
     match param_num_opt with
     | None -> BaseGC (base_offset, base_mode)
     | Some i -> FPassBaseGC (i, base_offset)
     )),
   1

   | A.Array_get(base_expr, opt_elem_expr) ->
     let elem_expr_instrs, elem_stack_size = emit_elem_instrs opt_elem_expr in
     let base_expr_instrs, base_setup_instrs, base_stack_size =
       emit_base mode (base_offset + elem_stack_size) param_num_opt base_expr in
     let mk = get_elem_member_key base_offset opt_elem_expr in
     let total_stack_size = base_stack_size + elem_stack_size in
     gather [
       base_expr_instrs;
       elem_expr_instrs;
     ],
     gather [
       base_setup_instrs;
       instr (IBase (
         match param_num_opt with
         | None -> Dim (mode, mk)
         | Some i -> FPassDim (i, mk)
       ))
     ],
     total_stack_size

   | A.Obj_get(base_expr, prop_expr, null_flavor) ->
     let prop_expr_instrs, prop_stack_size = emit_prop_instrs prop_expr in
     let base_expr_instrs, base_setup_instrs, base_stack_size =
       emit_base mode (base_offset + prop_stack_size) param_num_opt base_expr in
     let mk = get_prop_member_key null_flavor base_offset prop_expr in
     let total_stack_size = prop_stack_size + base_stack_size in
     let final_instr =
       instr (IBase (
         match param_num_opt with
         | None -> Dim (mode, mk)
         | Some i -> FPassDim (i, mk)
       )) in
     gather [
       base_expr_instrs;
       prop_expr_instrs;
     ],
     gather [
       base_setup_instrs;
       final_instr
     ],
     total_stack_size

   | A.Class_get(cid, (_, id)) ->
     let prop_expr_instrs =
       instr_string (SU.Locals.strip_dollar id) in
     let cexpr, _ = expr_to_class_expr (get_scope ()) (id_to_expr cid) in
     gather [
       prop_expr_instrs;
       emit_class_expr cexpr
     ],
     gather [
       instr_basesc base_offset
     ],
     1
   | A.Lvarvar (1, (_, id)) ->
     empty,
     instr_basenl (Local.Named id) base_mode,
     0
   | A.Lvarvar (n, (_, id)) ->
     let base_expr_instrs =
       instr_cgetl (Local.Named id)::
       List.replicate (n - 1) instr_cgetn
     in
     gather base_expr_instrs,
     instr_basenc base_offset base_mode,
     1

   | _ ->
     let base_expr_instrs, flavor = emit_flavored_expr expr in
     base_expr_instrs,
     instr (IBase (if flavor = Flavor.Ref
                   then BaseR base_offset else BaseC base_offset)),
     1

and instr_fpass kind i =
  match kind with
  | PassByRefKind.AllowCell -> instr (ICall (FPassC i))
  | PassByRefKind.WarnOnCell -> instr (ICall (FPassCW i))
  | PassByRefKind.ErrorOnCell -> instr (ICall (FPassCE i))

and instr_fpassr i = instr (ICall (FPassR i))

and emit_arg i ((_, expr_) as e) =
  match expr_ with
  | A.Lvar (_, x) when SN.Superglobals.is_superglobal x ->
    gather [
      instr_string (SU.Locals.strip_dollar x);
      instr (ICall (FPassG i))
    ]

  | A.Lvar (_, x)
    when not (is_local_this x) || get_needs_local_this () ->
    instr_fpassl i (Local.Named x)

  | A.Array_get((_, A.Lvar (_, x)), Some e) when x = SN.Superglobals.globals ->
    gather [
      from_expr ~need_ref:false e;
      instr (ICall (FPassG i))
    ]

  | A.Array_get(base_expr, opt_elem_expr) ->
    emit_array_get false (Some i) QueryOp.CGet base_expr opt_elem_expr

  | A.Obj_get(e1, e2, nullflavor) ->
    emit_obj_get false (Some i) QueryOp.CGet e1 e2 nullflavor

  | A.Class_get(cid, id) ->
    emit_class_get (Some i) QueryOp.CGet cid id

  | _ ->
    let instrs, flavor = emit_flavored_expr e in
    gather [
      instrs;
      if flavor = Flavor.Ref
      then instr_fpassr i
      else instr_fpass (get_passByRefKind e) i
    ]

and emit_ignored_expr e =
  let instrs, flavor = emit_flavored_expr e in
  gather [
    instrs;
    instr_pop flavor;
  ]

and is_splatted = function
  | _, A.Unop (A.Usplat, _) -> true
  | _ -> false

(* Emit code to construct the argument frame and then make the call *)
and emit_args_and_call args uargs =
  let all_args = args @ uargs in
  let is_splatted = List.exists ~f:is_splatted args in
  let nargs = List.length all_args in
  gather [
    gather (List.mapi all_args emit_arg);
    if uargs = [] && not is_splatted
    then instr (ICall (FCall nargs))
    else instr (ICall (FCallUnpack nargs))
  ]

(* Expression that appears in an object context, such as expr->meth(...) *)
and emit_object_expr (_, expr_ as expr) =
  match expr_ with
  | A.Lvar(_, x) when is_local_this x ->
    instr_this
  | _ -> from_expr ~need_ref:false expr

and emit_call_lhs (_, expr_ as expr) nargs =
  match expr_ with
  | A.Obj_get (obj, (_, A.Id (_, id)), null_flavor) when id.[0] = '$' ->
    gather [
      emit_object_expr obj;
      instr_cgetl (Local.Named id);
      instr (ICall (FPushObjMethod (nargs, null_flavor)));
    ]
  | A.Obj_get (obj, (_, A.Id (_, id)), null_flavor) ->
    gather [
      emit_object_expr obj;
      instr (ICall (FPushObjMethodD
        (nargs, Hhbc_id.Method.from_ast_name id, null_flavor)));
    ]
  | A.Obj_get(obj, method_expr, null_flavor) ->
    gather [
      emit_object_expr obj;
      from_expr ~need_ref:false method_expr;
      instr (ICall (FPushObjMethod (nargs, null_flavor)));
    ]

  | A.Class_const (cid, (_, id)) ->
    let cexpr, forward = expr_to_class_expr (get_scope ()) (id_to_expr cid) in
    let method_id = Hhbc_id.Method.from_ast_name id in
    if forward then
      gather [
        instr_string (Hhbc_id.Method.to_raw_string method_id);
        emit_class_expr cexpr;
        instr (ICall (FPushClsMethodF (nargs, 0)));
      ]
    else begin match cexpr with
    (* Statically known *)
    | Class_id cid ->
      let fq_cid, _ = Hhbc_id.Class.elaborate_id (get_namespace ()) cid in
      instr (ICall (FPushClsMethodD (nargs, method_id, fq_cid)))
    | _ ->
      gather [
        instr_string (Hhbc_id.Method.to_raw_string method_id);
        emit_class_expr cexpr;
        instr (ICall (FPushClsMethod (nargs, 0)));
      ]
    end

  | A.Class_get (cid, (_, id)) when id.[0] = '$' ->
    let cexpr, _ = expr_to_class_expr (get_scope ()) (id_to_expr cid) in
    gather [
      emit_local false id;
      emit_class_expr cexpr;
      instr (ICall (FPushClsMethod (nargs, 0)))
    ]

  | A.Id id ->
    let fq_id, id_opt =
      Hhbc_id.Function.elaborate_id (get_namespace ()) id in
    begin match id_opt with
    | Some id -> instr (ICall (FPushFuncU (nargs, fq_id, id)))
    | None -> instr (ICall (FPushFuncD (nargs, fq_id)))
    end

  | _ ->
    gather [
      from_expr ~need_ref:false expr;
      instr (ICall (FPushFunc nargs))
    ]

(* Retuns whether the function is a call_user_func function,
  min args, max args *)
and get_call_user_func_info = function
  | "call_user_func" -> (true, 1, max_int)
  | "call_user_func_array" -> (true, 2, 2)
  | "forward_static_call" -> (true, 1, max_int)
  | "forward_static_call_array"  -> (true, 2, 2)
  | "fb_call_user_func_safe" -> (true, 1, max_int)
  | "fb_call_user_func_array_safe" -> (true, 2, 2)
  | "fb_call_user_func_safe_return" -> (true, 2, max_int)
  | _ -> (false, 0, 0)

and is_call_user_func id num_args =
  let (is_fn, min_args, max_args) = get_call_user_func_info id in
  is_fn && num_args >= min_args && num_args <= max_args

and emit_call_user_func_args i expr =
  gather [
    from_expr ~need_ref:false expr;
    instr_fpass PassByRefKind.AllowCell i;
  ]

and emit_call_user_func id arg args =
  let return_default, args = match id with
    | "fb_call_user_func_safe_return" ->
      begin match args with
        | [] -> failwith "fb_call_user_func_safe_return - requires default arg"
        | a :: args -> from_expr ~need_ref:false a, args
      end
    | _ -> empty, args
  in
  let num_params = List.length args in
  let begin_instr = match id with
    | "forward_static_call"
    | "forward_static_call_array" -> instr_fpushcuff num_params
    | "fb_call_user_func_safe"
    | "fb_call_user_func_array_safe" ->
      gather [instr_null; instr_fpushcuf_safe num_params]
    | "fb_call_user_func_safe_return" ->
      gather [return_default; instr_fpushcuf_safe num_params]
    | _ -> instr_fpushcuf num_params
  in
  let call_instr = match id with
    | "call_user_func_array"
    | "forward_static_call_array"
    | "fb_call_user_func_array_safe" -> instr (ICall FCallArray)
    | _ -> instr (ICall (FCall num_params))
  in
  let end_instr = match id with
    | "fb_call_user_func_safe_return" -> instr (ICall CufSafeReturn)
    | "fb_call_user_func_safe"
    | "fb_call_user_func_array_safe" -> instr (ICall CufSafeArray)
    | _ -> empty
  in
  let flavor = match id with
    | "fb_call_user_func_safe"
    | "fb_call_user_func_array_safe" -> Flavor.Cell
    | _ -> Flavor.Ref
  in
  gather [
    from_expr ~need_ref:false arg;
    begin_instr;
    gather (List.mapi args emit_call_user_func_args);
    call_instr;
    end_instr;
  ], flavor

and emit_call (_, expr_ as expr) args uargs =
  let nargs = List.length args + List.length uargs in
  let default () =
    gather [
      emit_call_lhs expr nargs;
      emit_args_and_call args uargs;
    ], Flavor.Ref in
  match expr_ with
  | A.Id (_, id) when id = SN.SpecialFunctions.echo ->
    let instrs = gather @@ List.mapi args begin fun i arg ->
         gather [
           from_expr ~need_ref:false arg;
           instr (IOp Print);
           if i = nargs-1 then empty else instr_popc
         ] end in
    instrs, Flavor.Cell

  | A.Id (_, id) when
    (optimize_cuf ()) && (is_call_user_func id (List.length args)) ->
    if List.length uargs != 0 then
    failwith "Using argument unpacking for a call_user_func is not supported";
    begin match args with
      | [] -> failwith "call_user_func - needs a name"
      | arg :: args ->
        emit_call_user_func id arg args
    end

  | A.Id (_, "intval") when List.length args = 1 ->
    let e = List.hd_exn args in
    gather [
      from_expr ~need_ref:false e;
      instr (IOp CastInt)
    ], Flavor.Cell

  | A.Id (_, "strval") when List.length args = 1 ->
    let e = List.hd_exn args in
    gather [
      from_expr ~need_ref:false e;
      instr (IOp CastString)
    ], Flavor.Cell

  | A.Id (_, "boolval") when List.length args = 1 ->
    let e = List.hd_exn args in
    gather [
      from_expr ~need_ref:false e;
      instr (IOp CastBool)
    ], Flavor.Cell

  | A.Id (_, "floatval") when List.length args = 1 ->
    let e = List.hd_exn args in
    gather [
      from_expr ~need_ref:false e;
      instr (IOp CastDouble)
    ], Flavor.Cell

  | A.Id (_, "vec") when List.length args = 1 ->
    let e = List.hd_exn args in
    gather [
      from_expr ~need_ref:false e;
      instr (IOp CastVec)
    ], Flavor.Cell

  | A.Id (_, "keyset") when List.length args = 1 ->
    let e = List.hd_exn args in
    gather [
      from_expr ~need_ref:false e;
      instr (IOp CastKeyset)
    ], Flavor.Cell

  | A.Id (_, "dict") when List.length args = 1 ->
    let e = List.hd_exn args in
    gather [
      from_expr ~need_ref:false e;
      instr (IOp CastDict)
    ], Flavor.Cell

  | A.Id (_, "invariant") when List.length args > 0 ->
    let e = List.hd_exn args in
    let rest = List.tl_exn args in
    let l = Label.next_regular () in
    let p = Pos.none in
    let id = p, A.Id (p, "hh\\invariant_violation") in
    gather [
      emit_jmpnz e l;
      emit_ignored_expr (p, A.Call (id, rest, uargs));
      instr_string "invariant_violation";
      instr (IOp (Fatal FatalOp.Runtime));
      instr_label l;
      instr_null;
    ], Flavor.Cell

  | A.Id (_, "assert") ->
    let l0 = Label.next_regular () in
    let l1 = Label.next_regular () in
    gather [
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
    ], Flavor.Cell

  | A.Id (_, id) ->
    begin match args, istype_op id with
    | [(_, A.Lvar (_, arg_id))], Some i when not (is_local_this arg_id) ->
      instr (IIsset (IsTypeL (Local.Named arg_id, i))),
      Flavor.Cell
    | [arg_expr], Some i ->
      gather [
        from_expr ~need_ref:false arg_expr;
        instr (IIsset (IsTypeC i))
      ], Flavor.Cell
    | _ -> default ()
    end
  | _ -> default ()


(* Emit code for an expression that might leave a cell or reference on the
 * stack. Return which flavor it left.
 *)
and emit_flavored_expr (_, expr_ as expr) =
  match expr_ with
  | A.Call (e, args, uargs) when not (is_special_function e) ->
    emit_call e args uargs
  | _ ->
    from_expr ~need_ref:false expr, Flavor.Cell

and emit_final_member_op stack_index op mk =
  match op with
  | LValOp.Set -> instr (IFinal (SetM (stack_index, mk)))
  | LValOp.SetOp op -> instr (IFinal (SetOpM (stack_index, op, mk)))
  | LValOp.IncDec op -> instr (IFinal (IncDecM (stack_index, op, mk)))
  | LValOp.Unset -> instr (IFinal (UnsetM (stack_index, mk)))

and emit_final_local_op op lid =
  match op with
  | LValOp.Set -> instr (IMutator (SetL lid))
  | LValOp.SetOp op -> instr (IMutator (SetOpL (lid, op)))
  | LValOp.IncDec op -> instr (IMutator (IncDecL (lid, op)))
  | LValOp.Unset -> instr (IMutator (UnsetL lid))

and emit_final_global_op op =
  match op with
  | LValOp.Set -> instr (IMutator SetG)
  | LValOp.SetOp op -> instr (IMutator (SetOpG op))
  | LValOp.IncDec op -> instr (IMutator (IncDecG op))
  | LValOp.Unset -> instr (IMutator UnsetG)

and emit_final_static_op cid id op =
  match op with
  | LValOp.Set -> instr (IMutator (SetS 0))
  | LValOp.SetOp op -> instr (IMutator (SetOpS (op, 0)))
  | LValOp.IncDec op -> instr (IMutator (IncDecS (op, 0)))
  | LValOp.Unset ->
    gather [
      instr_string ("Attempt to unset static property " ^ cid ^ "::" ^ id);
      instr (IOp (Fatal FatalOp.Runtime))
    ]

(* Given a local $local and a list of integer array indices i_1, ..., i_n,
 * generate code to extract the value of $local[i_n]...[i_1]:
 *   BaseL $local Warn
 *   Dim Warn EI:i_n ...
 *   Dim Warn EI:i_2
 *   QueryM 0 CGet EI:i_1
 *)
and emit_array_get_fixed local indices =
  gather (
    instr (IBase (BaseL (local, MemberOpMode.Warn))) ::
    List.rev_mapi indices (fun i ix ->
      let mk = MemberKey.EI (Int64.of_int ix) in
      if i = 0
      then instr (IFinal (QueryM (0, QueryOp.CGet, mk)))
      else instr (IBase (Dim (MemberOpMode.Warn, mk))))
      )

(* Generate code for each lvalue assignment in a list destructuring expression.
 * Lvalues are assigned right-to-left, regardless of the nesting structure. So
 *     list($a, list($b, $c)) = $d
 * and list(list($a, $b), $c) = $d
 * will both assign to $c, $b and $a in that order.
 *)
 and emit_lval_op_list local indices expr =
  match expr with
  | (_, A.List exprs) ->
    gather @@
    List.rev @@
    List.mapi exprs (fun i expr -> emit_lval_op_list local (i::indices) expr)
  | _ ->
    (* Generate code to access the element from the array *)
    let access_instrs = emit_array_get_fixed local indices in
    (* Generate code to assign to the lvalue *)
    let assign_instrs = emit_lval_op_nonlist LValOp.Set expr access_instrs 1 in
    gather [
      assign_instrs;
      instr_popc
    ]

(* Emit code for an l-value operation *)
and emit_lval_op op expr1 opt_expr2 =
  match op, expr1, opt_expr2 with
    (* Special case for list destructuring, only on assignment *)
    | LValOp.Set, (_, A.List _), Some expr2 ->
      stash_in_local ~leave_on_stack:true expr2
      begin fun local _break_label ->
        emit_lval_op_list local [] expr1
      end
    | _ ->
      let rhs_instrs, rhs_stack_size =
        match opt_expr2 with
        | None -> empty, 0
        | Some e -> from_expr ~need_ref:false e, 1 in
      emit_lval_op_nonlist op expr1 rhs_instrs rhs_stack_size

and emit_lval_op_nonlist op (_, expr_) rhs_instrs rhs_stack_size =
  match expr_ with
  | A.Lvar (_, id) when SN.Superglobals.is_superglobal id ->
    gather [
      instr_string @@ SU.Locals.strip_dollar id;
      rhs_instrs;
      emit_final_global_op op;
    ]

  | A.Lvar (_, id) when not (is_local_this id) ->
    gather [
      rhs_instrs;
      emit_final_local_op op (Local.Named id)
    ]

  | A.Lvarvar (n, (_, id)) ->
    (* TODO: if the rhs is lvarvar then use cgetl2 *)
    gather [
      instr_cgetl (Local.Named id);
      gather @@ List.replicate ~num:(n-1) instr_cgetn;
      rhs_instrs;
      instr_setn;
    ]

  | A.Array_get ((_, A.Lvar (_, x)), Some e) when x = SN.Superglobals.globals ->
    let final_global_op_instrs = emit_final_global_op op in
    if rhs_stack_size = 0
    then gather [from_expr ~need_ref:false e; final_global_op_instrs]
    else
      let index_instrs, under_top = emit_first_expr e in
      if under_top
      then gather [rhs_instrs; index_instrs; final_global_op_instrs]
      else gather [index_instrs; rhs_instrs; final_global_op_instrs]

  | A.Array_get (base_expr, opt_elem_expr) ->
    let mode =
      match op with
      | LValOp.Unset -> MemberOpMode.Unset
      | _ -> MemberOpMode.Define in
    let elem_expr_instrs, elem_stack_size = emit_elem_instrs opt_elem_expr in
    let base_offset = elem_stack_size + rhs_stack_size in
    let base_expr_instrs, base_setup_instrs, base_stack_size =
      emit_base mode base_offset None base_expr in
    let mk = get_elem_member_key rhs_stack_size opt_elem_expr in
    let total_stack_size = elem_stack_size + base_stack_size in
    let final_instr = emit_final_member_op total_stack_size op mk in
    gather [
      base_expr_instrs;
      elem_expr_instrs;
      rhs_instrs;
      base_setup_instrs;
      final_instr
    ]

  | A.Obj_get (e1, e2, null_flavor) ->
    let mode =
      match op with
      | LValOp.Unset -> MemberOpMode.Unset
      | _ -> MemberOpMode.Define in
    let prop_expr_instrs, prop_stack_size = emit_prop_instrs e2 in
      let base_offset = prop_stack_size + rhs_stack_size in
    let base_expr_instrs, base_setup_instrs, base_stack_size =
      emit_base mode base_offset None e1 in
    let mk = get_prop_member_key null_flavor rhs_stack_size e2 in
    let total_stack_size = prop_stack_size + base_stack_size in
    let final_instr = emit_final_member_op total_stack_size op mk in
    gather [
      base_expr_instrs;
      prop_expr_instrs;
      rhs_instrs;
      base_setup_instrs;
      final_instr
    ]

  | A.Class_get (cid, (_, id)) ->
    let prop_expr_instrs =
      instr_string (SU.Locals.strip_dollar id) in
    let cexpr, _ = expr_to_class_expr (get_scope ()) (id_to_expr cid) in
    let final_instr = emit_final_static_op (snd cid) id op in
    gather [
      prop_expr_instrs;
      emit_class_expr cexpr;
      rhs_instrs;
      final_instr
    ]

  | A.Unop (uop, e) ->
    gather [
      rhs_instrs;
      emit_lval_op_nonlist op e empty rhs_stack_size;
      from_unop uop;
    ]

  | _ ->
    gather [
      emit_nyi "lval expression";
      rhs_instrs;
    ]

and from_unop op =
  let ints_overflow_to_ints =
    Hhbc_options.ints_overflow_to_ints !Hhbc_options.compiler_options
  in
  match op with
  | A.Utild -> instr (IOp BitNot)
  | A.Unot -> instr (IOp Not)
  | A.Uplus -> instr (IOp (if ints_overflow_to_ints then Add else AddO))
  | A.Uminus -> instr (IOp (if ints_overflow_to_ints then Sub else SubO))
  | A.Uincr | A.Udecr | A.Upincr | A.Updecr | A.Uref | A.Usplat ->
    emit_nyi "unop - probably does not need translation"

and emit_unop need_ref op e =
  let unop_instr = from_unop op in
  match op with
  | A.Utild ->
    emit_box_if_necessary need_ref @@ gather [
      from_expr ~need_ref:false e; unop_instr
    ]
  | A.Unot ->
    emit_box_if_necessary need_ref @@ gather [
      from_expr ~need_ref:false e; unop_instr
    ]
  | A.Uplus ->
    emit_box_if_necessary need_ref @@ gather
    [instr (ILitConst (Int (Int64.zero)));
    from_expr ~need_ref:false e;
    unop_instr]
  | A.Uminus ->
    emit_box_if_necessary need_ref @@ gather
    [instr (ILitConst (Int (Int64.zero)));
    from_expr ~need_ref:false e;
    unop_instr]
  | A.Uincr | A.Udecr | A.Upincr | A.Updecr ->
    begin match unop_to_incdec_op op with
    | None -> emit_nyi "incdec"
    | Some incdec_op ->
      let instr = emit_lval_op (LValOp.IncDec incdec_op) e None in
      emit_box_if_necessary need_ref instr
    end
  | A.Uref ->
    (*TODO: add support for references e*)
    emit_nyi "references"
  | A.Usplat ->
    from_expr ~need_ref:false e

and from_exprs exprs =
  gather (List.map exprs (from_expr ~need_ref:false))

(* Generate code to evaluate `e`, and, if necessary, store its value in a
 * temporary local `temp` (unless it is itself a local). Then use `f` to
 * generate code that uses this local and branches or drops through to
 * `break_label`:
 *    temp := e
 *    <code generated by `f temp break_label`>
 *  break_label:
 *    push `temp` on stack if `leave_on_stack` is true.
 *)
and stash_in_local ?(leave_on_stack=false) e f =
  let break_label = Label.next_regular () in
  match e with
  | (_, A.Lvar (_, id)) when not (is_local_this id) ->
    gather [
      f (Local.Named id) break_label;
      instr_label break_label;
      if leave_on_stack then instr_cgetl (Local.Named id) else empty;
    ]
  | _ ->
    let temp = Local.get_unnamed_local () in
    let fault_label = Label.next_fault () in
    gather [
      from_expr ~need_ref:false e;
      instr_setl temp;
      instr_popc;
      instr_try_fault
        fault_label
        (* try block *)
        (f temp break_label)
        (* fault block *)
        (gather [
          instr_unsetl temp;
          instr_unwind ]);
      instr_label break_label;
      if leave_on_stack then instr_pushl temp else instr_unsetl temp
    ]
