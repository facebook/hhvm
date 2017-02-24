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

module A = Ast
module N = Nast
module H = Hhbc_ast
module TC = Hhas_type_constraint
module SN = Naming_special_names

(* These are the three flavors of value that can live on the stack:
 *   C = cell
 *   R = ref
 *   A = classref
 *)
type flavor =
  | Flavor_C
  | Flavor_R
  | Flavor_A

(* The various from_X functions below take some kind of AST (expression,
 * statement, etc.) and produce what is logically a sequence of instructions.
 * This could simply be represented by a list, but then we would need to
 * use an accumulator to avoid the quadratic complexity associated with
 * repeated appending to a list. Instead, we simply build a tree of
 * instructions which can easily be flattened at the end.
 *)
type instr_seq =
| Instr_list of H.instruct list
| Instr_concat of instr_seq list

(* Some helper constructors *)
let instr x = Instr_list [x]
let instrs x = Instr_list x
let gather x = Instr_concat x
let empty = Instr_list []

let instr_jmp label = instr (H.IContFlow (H.Jmp label))
let instr_jmpz label = instr (H.IContFlow (H.JmpZ label))
let instr_jmpnz label = instr (H.IContFlow (H.JmpNZ label))
let instr_label label = instr (H.ILabel label)
let instr_unwind = instr H.(IContFlow Unwind)

let instr_false = instr (H.ILitConst H.False)
let instr_true = instr (H.ILitConst H.True)
let instr_setl_unnamed local =
  instr H.(IMutator (SetL (Local_unnamed local)))
let instr_unsetl_unnamed local =
  instr H.(IMutator (UnsetL (Local_unnamed local)))

let instr_popc = instr H.(IBasic PopC)

let rec instr_seq_to_list_aux sl result =
  match sl with
  | [] -> List.rev result
  | s::sl ->
    match s with
    | Instr_list instrl ->
      instr_seq_to_list_aux sl (List.rev_append instrl result)
    | Instr_concat sl' -> instr_seq_to_list_aux (sl' @ sl) result

let instr_seq_to_list t = instr_seq_to_list_aux [t] []

let instr_try_fault_no_catch fault_label try_body fault_body =
  let try_body = instr_seq_to_list try_body in
  let fault_body = instr_seq_to_list fault_body in
  let fl = H.IExceptionLabel (fault_label, H.FaultL) in
  instr (H.ITryFault (fault_label, try_body, fl :: fault_body))


(* Emit a comment in lieu of instructions for not-yet-implemented features *)
let emit_nyi description =
  instr (H.IComment ("NYI: " ^ description))


(* Strict binary operations; assumes that operands are already on stack *)
let from_binop op =
  match op with
  | A.Plus -> instr (H.IOp H.AddO)
  | A.Minus -> instr (H.IOp H.SubO)
  | A.Star -> instr (H.IOp H.MulO)
  | A.Slash -> instr (H.IOp H.Div)
  | A.Eqeq -> instr (H.IOp H.Eq)
  | A.EQeqeq -> instr (H.IOp H.Same)
  | A.Starstar -> instr (H.IOp H.Pow)
  | A.Diff -> instr (H.IOp H.Neq)
  | A.Diff2 -> instr (H.IOp H.NSame)
  | A.Lt -> instr (H.IOp H.Lt)
  | A.Lte -> instr (H.IOp H.Lte)
  | A.Gt -> instr (H.IOp H.Gt)
  | A.Gte -> instr (H.IOp H.Gte)
  | A.Dot -> instr (H.IOp H.Concat)
  | A.Amp -> instr (H.IOp H.BitAnd)
  | A.Bar -> instr (H.IOp H.BitOr)
  | A.Ltlt -> instr (H.IOp H.Shl)
  | A.Gtgt -> instr (H.IOp H.Shr)
  | A.Percent -> instr (H.IOp H.Mod)
  | A.Xor -> instr (H.IOp H.BitXor)
  | A.Eq _ -> emit_nyi "Eq"
  | A.AMpamp
  | A.BArbar ->
    failwith "short-circuiting operator cannot be generated as a simple binop"

let binop_to_eqop op =
  let open H in
  match op with
  | A.Plus -> Some PlusEqualO
  | A.Minus -> Some MinusEqualO
  | A.Star -> Some MulEqualO
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
  let open H in
  match op with
  | A.Uincr -> Some PreIncO
  | A.Udecr -> Some PreDecO
  | A.Upincr -> Some PostIncO
  | A.Updecr -> Some PostDecO
  | _ -> None

(* TODO: there are lots of ways of specifying the same type in a cast.
 * Sort this out!
 *)
let emit_cast (_, hint_) =
  let open H in
  match hint_ with
  | A.Happly((_, id), []) when id = SN.Typehints.int ->
    instr (IOp CastInt)
  | A.Happly((_, id), []) when id = SN.Typehints.bool ->
    instr (IOp CastBool)
  | A.Happly((_, id), []) when id = SN.Typehints.string ->
    instr (IOp CastString)
  | A.Happly((_, id), []) when id = SN.Typehints.object_cast ->
    instr (IOp CastObject)
  | A.Happly((_, id), []) when id = SN.Typehints.array ->
    instr (IOp CastArray)
  | A.Happly((_, id), []) when id = SN.Typehints.float ->
    instr (IOp CastDouble)
  | _ -> emit_nyi "cast type"

let rec from_expr expr =
  (* Note that this takes an Ast.expr, not a Nast.expr. *)
  let open H in
  match snd expr with
  | A.Float (_, litstr) ->
    instr (ILitConst (Double (float_of_string litstr)))
  | A.String (_, litstr) ->
    instr (ILitConst (String litstr))
  | A.Int (_, litstr) ->
    (* TODO deal with integer out of range *)
    instr (ILitConst (Int (Int64.of_string litstr)))
  | A.Null -> instr (ILitConst Null)
  | A.False -> instr (ILitConst False)
  | A.True -> instr (ILitConst True)
  | A.Lvar (_, x) when x = SN.SpecialIdents.this -> instr (IMisc This)
  | A.Lvar (_, x) -> instr (IGet (CGetL (Local_named x)))
  | A.Class_const ((_, cid), (_, id)) when id = SN.Members.mClass ->
    instr (ILitConst (String cid))
  | A.Unop (op, e) ->
    emit_unop op e
  | A.Binop (A.AMpamp, e1, e2) ->
    emit_logical_and e1 e2
  | A.Binop (A.BArbar, e1, e2) ->
    emit_logical_or e1 e2
  | A.Binop (A.Eq obop, e1, e2) ->
    emit_assignment obop e1 e2
  (* Special case to make use of CGetL2 *)
  | A.Binop (op, (_, A.Lvar (_, x)), e) ->
    gather [from_expr e; instr (IGet (CGetL2 (Local_named x))); from_binop op]
  | A.Binop (op, e1, e2) ->
    gather [from_expr e2; from_expr e1; from_binop op]
  | A.Pipe (e1, e2) ->
    emit_pipe e1 e2
  | A.InstanceOf (e1, (_, A.Id (_, id))) ->
    gather [from_expr e1; instr (IOp (InstanceOfD id))]
  | A.InstanceOf (e1, e2) ->
    gather [from_expr e1; from_expr e2; instr (IOp InstanceOf)]
  | A.NullCoalesce(e1, e2) ->
    let end_label = Label.get_next_label () in
    gather [
      emit_quiet_expr e1;
      instr (IBasic Dup);
      instr (IIsset (IsTypeC OpNull));
      instr (IOp Not);
      instr (IContFlow (JmpNZ end_label));
      instr (IBasic PopC);
      from_expr e2;
      instr (ILabel end_label);
    ]
  | A.Cast(hint, e) ->
    gather [from_expr e; emit_cast hint]
  | A.Eif (etest, Some etrue, efalse) ->
    let false_label = Label.get_next_label () in
    let end_label = Label.get_next_label () in
    gather [
      from_expr etest;
      instr (IContFlow (JmpZ false_label));
      from_expr etrue;
      instr (IContFlow (Jmp end_label));
      instr (ILabel false_label);
      from_expr efalse;
      instr (ILabel end_label)
    ]
  | A.Eif (etest, None, efalse) ->
    let end_label = Label.get_next_label () in
    gather [
      from_expr etest;
      instr (IBasic Dup);
      instr (IContFlow (JmpNZ end_label));
      instr (IBasic PopC);
      from_expr efalse;
      instr (ILabel end_label)
    ]
  | A.Expr_list es -> gather @@ List.map es ~f:from_expr
  | A.Call _ ->
    let instrs, flavor = emit_flavored_expr expr in
    gather [
      instrs;
      (* If the instruction has produced a ref then unbox it *)
      if flavor = Flavor_R then instr (IBasic UnboxR) else empty
    ]

  | A.New ((_, A.Id (_, id)), args, uargs) ->
      let nargs = List.length args + List.length uargs in
      gather [
        instr (ICall (FPushCtorD (nargs, id)));
        emit_args_and_call args uargs;
        instr (IBasic PopR)
      ]

  | _ ->
    emit_nyi "expression"

and emit_pipe e1 e2 =
  (* TODO: We need a local generator, like a label generator.
  For now, just use the label generator. *)
  let temp = Label.get_next_label () in
  let fault_label = Label.get_next_label () in
  gather [
    from_expr e1;
    instr_setl_unnamed temp;
    instr_popc;
    instr_try_fault_no_catch
      fault_label
      (from_expr e2) (* TODO: Rewrite $$ *)
      (gather [
        instr_unsetl_unnamed temp;
        instr_unwind ])
  ]

and emit_logical_and e1 e2 =
  let left_is_false = Label.get_next_label () in
  let right_is_true = Label.get_next_label () in
  let its_done = Label.get_next_label () in
  gather [
    from_expr e1;
    instr_jmpz left_is_false;
    from_expr e2;
    instr_jmpnz right_is_true;
    instr_label left_is_false;
    instr_false;
    instr_jmp its_done;
    instr_label right_is_true;
    instr_true;
    instr_label its_done ]

and emit_logical_or e1 e2 =
  let its_true = Label.get_next_label () in
  let its_done = Label.get_next_label () in
  gather [
    from_expr e1;
    instr_jmpnz its_true;
    from_expr e2;
    instr_jmpnz its_true;
    instr_false;
    instr_jmp its_done;
    instr_label its_true;
    instr_true;
    instr_label its_done ]

and emit_quiet_expr (_, expr_ as expr) =
  let open H in
  match expr_ with
  | A.Lvar (_, x) ->
    instr (IGet (CGetQuietL (Local_named x)))
  | _ ->
    from_expr expr

and emit_arg i ((_, expr_) as e) =
  match expr_ with
  | A.Lvar (_, x) ->
    instr H.(ICall (FPassL (Param_unnamed i, Local_named x)))
  | _ ->
    let instrs, flavor = emit_flavored_expr e in
    gather [
      instrs;
      instr H.(ICall (if flavor = Flavor_R then FPassR (Param_unnamed i)
                      else FPassCE (Param_unnamed i)));
    ]

and emit_pop flavor =
  match flavor with
  | Flavor_R -> instr H.(IBasic PopR)
  | Flavor_C -> instr H.(IBasic PopC)
  | Flavor_A -> instr H.(IBasic PopA)

and emit_ignored_expr e =
  let instrs, flavor = emit_flavored_expr e in
  gather [
    instrs;
    emit_pop flavor;
  ]

(* Emit code to construct the argument frame and then make the call *)
and emit_args_and_call args uargs =
  let all_args = args @ uargs in
  let nargs = List.length all_args in
  gather [
    gather (List.mapi all_args emit_arg);
    if uargs = []
    then instr H.(ICall (FCall nargs))
    else instr H.(ICall (FCallUnpack nargs))
  ]

and emit_call_lhs (_, expr_) nargs =
  let open H in
  match expr_ with
  | A.Obj_get (obj, (_, A.Id (_, id)), null_flavor) ->
    gather [
      from_expr obj;
      instr (ICall (FPushObjMethodD (nargs, id, null_flavor)));
    ]

  | A.Class_const ((_, cid), (_, id)) when cid = SN.Classes.cStatic ->
    instrs [
      ILitConst (String id);
      IMisc LateBoundCls;
      ICall (FPushClsMethod nargs);
    ]

  | A.Class_const ((_, cid), (_, id)) ->
    instr (ICall (FPushClsMethodD (nargs, id, cid)))

  | A.Id (_, id) ->
    instr (ICall (FPushFuncD (nargs, id)))

  | _ ->
    emit_nyi "call lhs expression"

and emit_call (_, expr_ as expr) args uargs =
  let nargs = List.length args + List.length uargs in
  match expr_ with
  | A.Id (_, id) when id = SN.SpecialFunctions.echo ->
    let instrs = gather @@ List.mapi args begin fun i arg ->
         gather [
           from_expr arg;
           instr H.(IOp Print);
           if i = nargs-1 then empty else emit_pop Flavor_C
         ] end in
    instrs, Flavor_C

  | A.Obj_get _ | A.Class_const _ | A.Id _ ->
    gather [
      emit_call_lhs expr nargs;
      emit_args_and_call args uargs;
    ], Flavor_R

  | _ ->
    emit_nyi "call expression", Flavor_C

(* Emit code for an expression that might leave a cell or reference on the
 * stack. Return which flavor it left.
 *)
and emit_flavored_expr (_, expr_ as expr) =
  match expr_ with
  | A.Call (e, args, uargs) ->
    emit_call e args uargs
  | _ ->
    from_expr expr, Flavor_C

and literal_from_named_expr expr =
  match snd expr with
  | N.Float (_, litstr) -> H.Double (float_of_string litstr)
  | N.String (_, litstr) -> H.String litstr
  | N.Int (_, litstr) -> H.Int (Int64.of_string litstr)
  | N.Null -> H.Null
  | N.False -> H.False
  | N.True -> H.True
  (* TODO: HHVM does not allow <<F(2+2)>> in an attribute, but Hack does, and
   this seems reasonable to allow. Right now this will crash if given an
   expression rather than a literal in here.  In particular, see what unary
   minus does; do we allow it on a literal int? We should. *)
  | _ -> failwith (Printf.sprintf
    "Expected a literal expression in literal_from_named_expr, got %s"
    (N.expr_to_string (snd expr)))

and literals_from_named_exprs exprs =
  List.map exprs literal_from_named_expr

(* Emit code for an l-value, returning instructions and the location that
 * must be set. For now, this is just a local. *)
and emit_lval (_, expr_) =
  match expr_ with
  | A.Lvar id -> empty, H.Local_named (snd id)
  | _ -> emit_nyi "lval expression", H.Local_unnamed 0

and emit_assignment obop e1 e2 =
  let instrs1, lval = emit_lval e1 in
  let instrs2 = from_expr e2 in
  let open H in
  gather [instrs1; instrs2;
    match obop with
    | None -> instr (IMutator (SetL lval))
    | Some bop ->
      match binop_to_eqop bop with
      | None -> emit_nyi "op-assignment"
      | Some eqop -> instr (IMutator (SetOpL (lval, eqop)))
    ]

and emit_unop op e =
  let open H in
  match op with
  | A.Utild -> gather [from_expr e; instr (IOp BitNot)]
  | A.Unot -> gather [from_expr e; instr (IOp Not)]
  | A.Uplus -> gather
    [instr (ILitConst (Int (Int64.zero)));
    from_expr e;
    instr (IOp AddO)]
  | A.Uminus -> gather
    [instr (ILitConst (Int (Int64.zero)));
    from_expr e;
    instr (IOp SubO)]
  | A.Uincr | A.Udecr | A.Upincr | A.Updecr ->
    let instrs, lval = emit_lval e in
    gather [instrs;
      match unop_to_incdec_op op with
      | None -> emit_nyi "incdec"
      | Some incdec_op -> instr (IMutator (IncDecL (lval, incdec_op)))]
  | A.Uref ->
    emit_nyi "references"

and emit_condition_loop begin_label break_label =
  instr H.(IContFlow (JmpNZ begin_label)),
  instr H.(IContFlow (JmpZ break_label))

and from_exprs exprs =
  gather (List.map exprs from_expr)

(* Returns the set of instructions and
 * whether a continue or a break is needed
 * Note about labels regarding continue and break:
 * HHVM generates a separate condition block for continue to jump to,
 * hence creates an additional label however, it also names this label
 * at the time of creation. But we do it recursively, in order to have
 * parity with HHVM we tread along a flag that tells us whether the
 * label is already used for continue or whether it is safe to use it
 * for other purposes *)
and from_stmt ~continue_label ~break_label verify_return st =
  let open H in
  match st with
  | A.Expr expr ->
    emit_ignored_expr expr, (false, false)
  | A.Return (_, None) ->
    instrs [
      ILitConst Null;
      IContFlow RetC;
    ], (false, false)
  | A.Return (_,  Some expr) ->
    gather [
      from_expr expr;
      (if verify_return then instr (IMisc VerifyRetTypeC) else empty);
      instr (IContFlow RetC);
    ], (false, false)
  | A.Block b -> from_stmts ~continue_label ~break_label verify_return b
  | A.If (e, b1, b2) ->
    let l0 = Label.get_next_label () in
    let l1 = Label.get_next_label () in
    let jmp0, jmp1 =
      instr H.(IContFlow (JmpZ l0)), instr H.(IContFlow (Jmp l1))
    in
    let b1, (need_cont1, need_break1) =
      from_stmt ~continue_label ~break_label verify_return (A.Block b1)
    in
    let b2, (need_cont2, need_break2) =
      from_stmt ~continue_label ~break_label verify_return (A.Block b2)
    in
    gather [
      from_expr e;
      jmp0;
      b1;
      jmp1;
      instr (ILabel l0);
      b2;
      instr (ILabel l1);
    ], (need_cont1 || need_cont2, need_break1 || need_break2)
  | A.While (e, b) ->
    let l0 = Label.get_next_label () in
    let l1 = Label.get_next_label () in
    let cond = from_expr e in
    let body, (need_cont, _) =
      from_stmt
        ~continue_label:(Some l1)
        ~break_label:(Some l0)
        verify_return
        (A.Block b)
    in
    let jmp_to_begin, jmp_to_end, begin_label, cont_label =
      if need_cont
      then
        let l2 = Label.get_next_label () in
        let jmp_to_begin, jmp_to_end = emit_condition_loop l2 l0 in
        jmp_to_begin, jmp_to_end, instr (ILabel l2), instr (ILabel l1)
      else
        let jmp_to_begin, jmp_to_end = emit_condition_loop l1 l0 in
        jmp_to_begin, jmp_to_end, instr (ILabel l1), empty
    in
    gather [
      cond;
      jmp_to_end;
      begin_label;
      body;
      cont_label;
      cond;
      jmp_to_begin;
      instr (ILabel l0);
    ], (false, false)
  (* TODO: Break takes an argument *)
  | A.Break _ ->
    begin match break_label with
    | Some i -> instr H.(IContFlow (Jmp i))
    | None -> failwith "There is nowhere to break"
    end, (false, true)
  | A.Continue _ ->
  (* TODO: Continue takes an argument *)
    begin match continue_label with
    | Some i -> instr H.(IContFlow (Jmp i))
    | None -> failwith "There is nowhere to continue"
    end, (true, false)
  | A.Do (b, e) ->
    let l0 = Label.get_next_label () in
    let l1 = Label.get_next_label () in
    let body, (need_cont, need_break) =
      from_stmt
        ~continue_label:(Some l0)
        ~break_label:(Some l1)
        verify_return
        (A.Block b)
    in
    let jmp_to_begin, begin_label, continue_label, break_label =
      if need_cont && need_break
      then
        let l2 = Label.get_next_label () in
        let jmp_to_begin = instr H.(IContFlow (JmpNZ l2)) in
        jmp_to_begin, instr (ILabel l2), instr (ILabel l0), instr (ILabel l1)
      else if need_cont
      then
        let jmp_to_begin = instr H.(IContFlow (JmpNZ l1)) in
        jmp_to_begin, instr (ILabel l1), instr (ILabel l0), empty
      else if need_break
      then
        (* TODO: Begin and break labels are switched
         * as we have already assigned l1 to break *)
        let jmp_to_begin = instr H.(IContFlow (JmpNZ l0)) in
        jmp_to_begin, instr (ILabel l0), empty, instr (ILabel l1)
      else
        (* TODO: We are wasting a label here since
         * we created l1 but not actually use it *)
        instr H.(IContFlow (JmpNZ l0)), instr (ILabel l0), empty, empty
    in
    gather [
      begin_label;
      body;
      continue_label;
      from_expr e;
      jmp_to_begin;
      break_label;
    ], (false, false)
  | A.For (e1, e2, e3, b) ->
    let l0 = Label.get_next_label () in
    let l1 = Label.get_next_label () in
    let cond = from_expr e2 in
    let body, (need_cont, _) =
      from_stmt
        ~continue_label:(Some l1)
        ~break_label:(Some l0)
        verify_return
        (A.Block b)
    in
    let jmp_to_begin, jmp_to_end, begin_label, cont_label =
      if need_cont
      then
        let l2 = Label.get_next_label () in
        let jmp_to_begin, jmp_to_end = emit_condition_loop l2 l0 in
        jmp_to_begin, jmp_to_end, instr (ILabel l2), instr (ILabel l1)
      else
        let jmp_to_begin, jmp_to_end = emit_condition_loop l1 l0 in
        jmp_to_begin, jmp_to_end, instr (ILabel l1), empty
    in
    gather [
      emit_ignored_expr e1;
      cond;
      jmp_to_end;
      begin_label;
      body;
      cont_label;
      emit_ignored_expr e3;
      cond;
      jmp_to_begin;
      instr (ILabel l0);
    ], (false, false)
  | A.Throw e ->
    gather [
      from_expr e;
      instr H.(IContFlow Throw);
    ], (false, false)
  | A.Try (tb, cl, fb) ->
    let catch_exists = List.length cl <> 0 in
    let finally_exists = fb <> [] in

    let l0 = Label.get_next_label () in
    let new_continue_label =
      if finally_exists then Some l0 else continue_label
    in
    let new_break_label =
      if finally_exists then Some l0 else break_label
    in
    (* TODO: Combine needs of catch and try block *)
    let try_body, needs =
      from_stmt
        ~continue_label:new_continue_label
        ~break_label:new_break_label
        verify_return
        (A.Block tb)
    in
    let try_body = gather [try_body; instr (IContFlow (Jmp l0));] in
    let try_body_list = instr_seq_to_list try_body in
    let catches =
      List.map cl
      ~f:(from_catch
          ~continue_label:new_continue_label
          ~break_label:new_break_label
          verify_return
          l0)
    in
    let catch_meta_data = List.rev @@ List.map catches ~f:snd in
    let catch_blocks =
      List.map catches ~f:(fun x -> instr_seq_to_list @@ fst x)
    in
    let catch_instr = List.concat catch_blocks in
    (* TODO: What happens to finally needs? *)
    let finally_body, _ =
      from_stmt
        ~continue_label
        ~break_label
        verify_return
        (A.Block fb)
    in
    let fault_body =
      if finally_exists
      then
        instr_seq_to_list @@ gather [
          (* TODO: What are these unnamed locals? *)
          instr H.(IMutator (UnsetL (Local_unnamed 0)));
          instr H.(IMutator (UnsetL (Local_unnamed 0)));
          finally_body;
          instr H.(IContFlow Unwind);
        ]

      else []
    in
    let init =
      if finally_exists && catch_exists
      then
        let f1 = Label.get_next_label () in
        let rest =
          ITryCatch (catch_meta_data, try_body_list)
          :: catch_instr
        in
        let f1_label = IExceptionLabel (f1, FaultL) in
        instr (ITryFault (f1, rest, f1_label :: fault_body))
      else if finally_exists
      then
        let f1 = Label.get_next_label () in
        let f1_label = IExceptionLabel (f1, FaultL) in
        instr (ITryFault (f1, try_body_list, f1_label :: fault_body))
      else if catch_exists
      then
        instrs @@ ITryCatch (catch_meta_data, try_body_list) :: catch_instr
      else
        failwith "Impossible for finally and catch to both not exist"
    in
    gather [
      init;
      instr (ILabel l0);
      finally_body;
    ], needs
  | A.Static_var _
  | A.Switch _
  | A.Foreach _ ->
    emit_nyi "statement", (false, false)
  (* TODO: What do we do with unsafe? *)
  | A.Unsafe
  | A.Fallthrough
  | A.Noop -> empty, (false, false)

and from_stmts ~continue_label ~break_label verify_return stl =
  let results =
    List.map stl (from_stmt ~continue_label ~break_label verify_return)
  in
  let instrs = List.map results fst in
  let needs = List.map results snd in
  let need_cont = List.exists ~f:(fst) needs in
  let need_break = List.exists ~f:(snd) needs in
  gather (instrs), (need_cont, need_break)

and from_catch
  ~continue_label
  ~break_label
  verify_return end_label ((_, id1), (_, id2), b) =
  let open H in
  let cl = Label.get_next_label () in
  (* TODO: what to do with needs? *)
  let body, _ =
    from_stmt
      ~continue_label
      ~break_label
      verify_return
      (A.Block b)
  in
  gather [
    instr (IExceptionLabel (cl, CatchL));
    instr (IMisc Catch);
    instr H.(IMutator (SetL (Local_named id2)));
    instr H.(IBasic PopC);
    body;
    instr H.(IContFlow (Jmp end_label));
  ], (cl, id1)

let fmt_prim x =
  match x with
  | N.Tvoid   -> "HH\\void"
  | N.Tint    -> "HH\\int"
  | N.Tbool   -> "HH\\bool"
  | N.Tfloat  -> "HH\\float"
  | N.Tstring -> "HH\\string"
  | N.Tnum    -> "HH\\num"
  | N.Tresource -> "HH\\resource"
  | N.Tarraykey -> "HH\\arraykey"
  | N.Tnoreturn -> "HH\\noreturn"

(* TODO *)
let fmt_name s = s

let extract_shape_fields smap =
  let get_pos =
    function A.SFlit (p, _) | A.SFclass_const ((p, _), _) -> p in
  List.sort (fun (k1, _) (k2, _) -> Pos.compare (get_pos k1) (get_pos k2))
    (Nast.ShapeMap.elements smap)

(* Produce the "userType" bit of the annotation *)
let rec fmt_hint (_, h) =
  match h with
  | N.Hany -> ""
  | N.Hmixed -> "HH\\mixed"
  | N.Hthis -> "HH\\this"
  | N.Hprim prim -> fmt_prim prim
  | N.Habstr s -> fmt_name s

  | N.Happly ((_, s), []) -> fmt_name s
  | N.Happly ((_, s), args) ->
    fmt_name s ^ "<" ^ String.concat ", " (List.map args fmt_hint) ^ ">"

  | N.Hfun (args, _, ret) ->
    "(function (" ^ String.concat ", " (List.map args fmt_hint) ^ "): " ^
      fmt_hint ret ^ ")"

  | N.Htuple hs ->
    "(" ^ String.concat ", " (List.map hs fmt_hint) ^ ")"

  | N.Haccess (h, accesses) ->
    fmt_hint h ^ "::" ^ String.concat "::" (List.map accesses snd)

  | N.Hoption t -> "?" ^ fmt_hint t

  | N.Harray (None, None) -> "array"
  | N.Harray (Some h, None) -> "array<" ^ fmt_hint h ^ ">"
  | N.Harray (Some h1, Some h2) ->
    "array<" ^ fmt_hint h1 ^ ", " ^ fmt_hint h2 ^ ">"
  | N.Harray _ -> failwith "bogus array"
  | N.Hshape smap ->
    let fmt_field = function
      | A.SFlit (_, s) -> "'" ^ s ^ "'"
      | A.SFclass_const ((_, s1), (_, s2)) -> fmt_name s1 ^ "::" ^ s2
    in
    let shape_fields =
      List.map ~f:(fun (k, h) -> fmt_field k ^ "=>" ^ fmt_hint h)
        (extract_shape_fields smap) in
    "HH\\shape(" ^ String.concat ", " shape_fields ^ ")"

let rec hint_to_type_constraint tparams (_, h) =
match h with
| N.Hany | N.Hmixed | N.Hfun (_, _, _) | N.Hthis
| N.Hprim N.Tvoid ->
  TC.make None []

| N.Hprim prim ->
  let tc_name = Some (fmt_prim prim) in
  let tc_flags = [TC.HHType] in
  TC.make tc_name tc_flags

| N.Haccess (_, _) ->
  let tc_name = Some "" in
  let tc_flags = [TC.HHType; TC.ExtendedHint; TC.TypeConstant] in
  TC.make tc_name tc_flags

(* Need to differentiate between type params and classes *)
| N.Habstr s | N.Happly ((_, s), _) ->
  if List.mem tparams s then
    let tc_name = Some "" in
    let tc_flags = [TC.HHType; TC.ExtendedHint; TC.TypeVar] in
    TC.make tc_name tc_flags
  else
    let tc_name = Some s in
    let tc_flags = [TC.HHType] in
    TC.make tc_name tc_flags

(* Shapes and tuples are just arrays *)
| N.Harray (_, _) | N.Hshape _ |  N.Htuple _ ->
  let tc_name = Some "array" in
  let tc_flags = [TC.HHType] in
  TC.make tc_name tc_flags

| N.Hoption t ->
  let tc = hint_to_type_constraint tparams t in
  let tc_name = TC.name tc in
  let tc_flags = TC.flags tc in
  let tc_flags = List.dedup
    ([TC.Nullable; TC.HHType; TC.ExtendedHint] @ tc_flags) in
  TC.make tc_name tc_flags

let hint_to_type_info ~always_extended tparams h =
  let tc = hint_to_type_constraint tparams h in
  let tc_name = TC.name tc in
  let tc_flags = TC.flags tc in
  let tc_flags =
    if always_extended && tc_name != None
    then List.dedup (TC.ExtendedHint :: tc_flags)
    else tc_flags in
  let type_info_user_type = Some (fmt_hint h) in
  let type_info_type_constraint = TC.make tc_name tc_flags in
  Hhas_type_info.make type_info_user_type type_info_type_constraint

let hints_to_type_infos ~always_extended tparams hints =
  let mapper hint = hint_to_type_info always_extended tparams hint in
  List.map hints mapper

let from_param tparams p =
  let param_name = p.N.param_name in
  let param_type_info = Option.map p.N.param_hint
    (hint_to_type_info ~always_extended:false tparams) in
  Hhas_param.make param_name param_type_info

let has_type_constraint ti =
  match ti with
  | Some ti when (Hhas_type_info.has_type_constraint ti) -> true
  | _ -> false

let emit_method_prolog params =
  gather H.(List.filter_map params (fun p ->
    let param_type_info = Hhas_param.type_info p in
    let param_name = Hhas_param.name p in
    if has_type_constraint param_type_info
    then Some (instr (IMisc (VerifyParamType (Param_named param_name))))
    else None))

let tparams_to_strings tparams =
  List.map tparams (fun (_, (_, s), _) -> s)

(* TODO: This function is grossly inefficient,
 * I wonder how we can do it better *)
let rec emit_fault_instructions stmt_instrs =
  let emit_fault_instruction_aux = function
    | H.ITryFault (_, il, fault) ->
      gather [emit_fault_instructions @@ instrs il; instrs fault;]
    | H.ITryCatch (_, il) -> emit_fault_instructions @@ instrs il
    | _ -> empty
  in
  let instr_list = instr_seq_to_list stmt_instrs in
  gather @@ List.map instr_list ~f:emit_fault_instruction_aux

let from_body tparams params ret b =
  let params = List.map params (from_param tparams) in
  let return_type_info = Option.map ret
    (hint_to_type_info ~always_extended:true tparams) in
  let verify_return = has_type_constraint return_type_info in
  let stmt_instrs, _ =
    from_stmts ~continue_label:None ~break_label:None verify_return b.N.fub_ast
  in
  let ret_instrs =
    match List.last b.N.fub_ast with Some (A.Return _) -> empty | _ ->
    instrs [H.ILitConst H.Null; H.IContFlow H.RetC]
  in
  let fault_instrs = emit_fault_instructions stmt_instrs in
  let body_instrs = gather [
    emit_method_prolog params;
    stmt_instrs;
    ret_instrs;
    fault_instrs;
  ] in
  body_instrs, params, return_type_info

(* In order to be efficient,
 * keep around a set to check for membership and a list for order *)
let rec extract_decl_vars set l body_instrs =
  let open H in
  let add_if_not_exists set l s =
    if SSet.mem s set then (set, l) else (SSet.add s set, s::l)
  in
  let extract_decl_vars_aux set l = function
    | IMutator (SetL (Local_named s)) -> add_if_not_exists set l s
    | ITryFault (_, il, _)
    | ITryCatch (_, il) -> extract_decl_vars set l il
    | _ -> (set, l)
  in
  List.fold_left
    body_instrs
    ~init:(set, l)
    ~f:(fun (set, l) i -> extract_decl_vars_aux set l i)

let from_fun_ : Nast.fun_ -> Hhas_function.t option =
  fun nast_fun ->
  Label.reset_label ();
  let function_name = Litstr.to_string @@ snd nast_fun.Nast.f_name in
  match nast_fun.N.f_body with
  | N.NamedBody _ ->
    None
  | N.UnnamedBody b ->
    let tparams = tparams_to_strings nast_fun.N.f_tparams in
    let body_instrs, function_params, function_return_type =
      from_body tparams nast_fun.N.f_params nast_fun.N.f_ret b in
    let function_body = instr_seq_to_list body_instrs in
    let (_, function_decl_vars) =
      extract_decl_vars SSet.empty [] function_body
    in
    (* In order to be efficient,
     * we cons everything, hence the need to reverse *)
    let function_decl_vars = List.rev function_decl_vars in
    Some (Hhas_function.make
      function_name
      function_params
      function_return_type
      function_body
      function_decl_vars)

let from_functions nast_functions =
  Core.List.filter_map nast_functions from_fun_

let from_attribute : Nast.user_attribute -> Hhas_attribute.t =
  fun nast_attr ->
  let attribute_name = Litstr.to_string @@ snd nast_attr.Nast.ua_name in
  let attribute_arguments = literals_from_named_exprs nast_attr.Nast.ua_params in
  Hhas_attribute.make attribute_name attribute_arguments

let from_attributes nast_attributes =
  (* The list of attributes is reversed in the Nast. *)
  List.map (List.rev nast_attributes) from_attribute

let from_method : bool -> Nast.class_ -> Nast.method_ -> Hhas_method.t option =
  fun method_is_static nast_class nast_method ->
  let class_tparams = tparams_to_strings (fst nast_class.N.c_tparams) in
  let method_name = Litstr.to_string @@ snd nast_method.Nast.m_name in
  let method_is_abstract = nast_method.Nast.m_abstract in
  (* TODO: An abstract method must generate
.method [protected abstract] <"HH\\void" N  > protected_abstract_method() {
String "Cannot call abstract method AbstractClass::protected_abstract_method()"
Fatal RuntimeOmitFrame
} *)
  let method_is_final = nast_method.Nast.m_final in
  let method_is_private = nast_method.Nast.m_visibility = Nast.Private in
  let method_is_protected = nast_method.Nast.m_visibility = Nast.Protected in
  let method_is_public = nast_method.Nast.m_visibility = Nast.Public in
  let method_attributes = from_attributes nast_method.Nast.m_user_attributes in
  match nast_method.N.m_body with
  | N.NamedBody _ ->
    None
  | N.UnnamedBody b ->
    let method_tparams = tparams_to_strings nast_method.N.m_tparams in
    let tparams = class_tparams @ method_tparams in
    let body_instrs, method_params, method_return_type =
      from_body tparams nast_method.N.m_params nast_method.N.m_ret b in
    let method_body = instr_seq_to_list body_instrs in
    let m = Hhas_method.make
      method_attributes
      method_is_protected
      method_is_public
      method_is_private
      method_is_static
      method_is_final
      method_is_abstract
      method_name
      method_params
      method_return_type
      method_body in
    Some m

let from_methods method_is_static nast_class nast_methods =
  Core.List.filter_map nast_methods (from_method method_is_static nast_class)

let is_interface nast_class =
  nast_class.N.c_kind = Ast.Cinterface

let default_constructor nast_class =
  let method_attributes = [] in
  let method_name = "86ctor" in
  let method_body = instr_seq_to_list (gather [
    instr (H.ILitConst H.Null);
    instr (H.IContFlow H.RetC)
  ]) in
  let method_is_abstract = is_interface nast_class in
  let method_is_final = false in
  let method_is_private = false in
  let method_is_protected = false in
  let method_is_public = true in
  let method_is_static = false in
  let method_params = [] in
  let method_return_type = None in
  Hhas_method.make
    method_attributes
    method_is_protected
    method_is_public
    method_is_private
    method_is_static
    method_is_final
    method_is_abstract
    method_name
    method_params
    method_return_type
    method_body

let add_constructor nast_class class_methods =
  match nast_class.N.c_constructor with
  | None -> (default_constructor nast_class) :: class_methods
  | Some nast_ctor ->
    begin
      match from_method false nast_class nast_ctor with
      | None -> class_methods
      | Some m -> m :: class_methods
    end

let from_extends tparams extends =
  (* TODO: This prints out "extends <"\\Mammal" "\\Mammal" hh_type >"
  instead of "extends Mammal" -- figure out how to have it produce the
  simpler form in this clause.
  *)
  match extends with
  | [] -> None
  | h :: _ -> Some (hint_to_type_info ~always_extended:false tparams h)

let from_implements tparams implements =
  (* TODO: This prints out "implements <"\\IFoo" "\\IFoo" hh_type >"
  instead of "implements IFoo" -- figure out how to have it produce the
  simpler form in this clause.
  *)
  hints_to_type_infos ~always_extended:false tparams implements

let from_property property_is_static class_var =
  (* TODO: xhp, type, initializer *)
  (* TODO: Hack allows a property to be marked final, which is nonsensical.
  HHVM does not allow this.  Fix this in the Hack parser? *)
  let property_name = Litstr.to_string @@ snd class_var.N.cv_id in
  let property_is_private = class_var.N.cv_visibility = N.Private in
  let property_is_protected = class_var.N.cv_visibility = N.Protected in
  let property_is_public = class_var.N.cv_visibility = N.Public in
  Hhas_property.make
    property_is_private
    property_is_protected
    property_is_public
    property_is_static
    property_name

let from_properties property_is_static class_vars =
  List.map class_vars (from_property property_is_static)

let from_constant (_hint, name, const_init) =
  (* The type hint is omitted. *)
  match const_init with
  | None -> None (* Abstract constants are omitted *)
  | Some _init ->
    (* TODO: Deal with the initializer *)
    let constant_name = Litstr.to_string @@ snd name in
    Some (Hhas_constant.make constant_name)

let from_constants nast_constants =
  Core.List.filter_map nast_constants from_constant

let from_type_constant nast_type_constant =
  match nast_type_constant.N.c_tconst_type with
  | None -> None (* Abstract type constants are omitted *)
  | Some _init ->
    (* TODO: Deal with the initializer *)
    let type_constant_name = Litstr.to_string @@
      snd nast_type_constant.N.c_tconst_name in
    Some (Hhas_type_constant.make type_constant_name)

let from_type_constants nast_type_constants =
  Core.List.filter_map nast_type_constants from_type_constant

let from_class : Nast.class_ -> Hhas_class.t =
  fun nast_class ->
  let class_attributes = from_attributes nast_class.N.c_user_attributes in
  let class_name = Litstr.to_string @@ snd nast_class.N.c_name in
  let class_is_trait = nast_class.N.c_kind = Ast.Ctrait in
  let class_is_enum = nast_class.N.c_kind = Ast.Cenum in
  let class_is_interface = is_interface nast_class in
  let class_is_abstract = nast_class.N.c_kind = Ast.Cabstract in
  let class_is_final =
    nast_class.N.c_final || class_is_trait || class_is_enum in
  let tparams = tparams_to_strings (fst nast_class.N.c_tparams) in
  let class_base =
    if class_is_interface then None
    else from_extends tparams nast_class.N.c_extends in
  let implements =
    if class_is_interface then nast_class.N.c_extends
    else nast_class.N.c_implements in
  let class_implements = from_implements tparams implements in
  let instance_methods = from_methods false nast_class nast_class.N.c_methods in
  let static_methods =
    from_methods true nast_class nast_class.N.c_static_methods in
  let class_methods = instance_methods @ static_methods in
  let class_methods = add_constructor nast_class class_methods in
  let instance_properties = from_properties false nast_class.N.c_vars in
  let static_properties = from_properties true nast_class.N.c_static_vars in
  let class_properties = static_properties @ instance_properties in
  let class_constants = from_constants nast_class.N.c_consts in
  let class_type_constants = from_type_constants nast_class.N.c_typeconsts in
  (* TODO: uses, xhp attr uses, xhp category *)
  Hhas_class.make
    class_attributes
    class_base
    class_implements
    class_name
    class_is_final
    class_is_abstract
    class_is_interface
    class_is_trait
    class_is_enum
    class_methods
    class_properties
    class_constants
    class_type_constants

let from_classes nast_classes =
  Core.List.map nast_classes from_class
