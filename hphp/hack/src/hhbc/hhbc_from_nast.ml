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

module A = Ast
module H = Hhbc_ast
module TC = Hhas_type_constraint
module SN = Naming_special_names
module CBR = Continue_break_rewriter

(* When using the PassX instructions we need to emit the right kind *)
module PassByRefKind = struct
  type t = AllowCell | WarnOnCell | ErrorOnCell
end

(* Emit a comment in lieu of instructions for not-yet-implemented features *)
let emit_nyi description =
  instr (IComment ("NYI: " ^ description))

(* Strict binary operations; assumes that operands are already on stack *)
let from_binop op =
  match op with
  | A.Plus -> instr (IOp AddO)
  | A.Minus -> instr (IOp SubO)
  | A.Star -> instr (IOp MulO)
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
  | A.Percent -> instr (IOp Mod)
  | A.Xor -> instr (IOp BitXor)
  | A.Eq _ -> emit_nyi "Eq"
  | A.AMpamp
  | A.BArbar ->
    failwith "short-circuiting operator cannot be generated as a simple binop"

let binop_to_eqop op =
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

let collection_type = function
  | "Vector"    -> 17
  | "Map"       -> 18
  | "Set"       -> 19
  | "Pair"      -> 20
  | "ImmVector" -> 21
  | "ImmMap"    -> 22
  | "ImmSet"    -> 23
  | x -> failwith ("unknown collection type '" ^ x ^ "'")

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
    | _ -> ErrorOnCell in
  from_non_list_assignment AllowCell expr

let extract_shape_field_name_pstring = function
  | A.SFlit p
  | A.SFclass_const (_, p) ->  p

let extract_shape_field_name = function
  | A.SFlit (_, s)
  | A.SFclass_const (_, (_, s)) -> s

let rec expr_and_newc instr_to_add_new instr_to_add = function
  | A.AFvalue e ->
    gather [from_expr e; instr_to_add_new]
  | A.AFkvalue (k, v) ->
    gather [from_expr k; from_expr v; instr_to_add]

and from_expr expr =
  (* Note that this takes an Ast.expr, not a Nast.expr. *)
  match snd expr with
  | A.Float (_, litstr) ->
    instr (ILitConst (Double litstr))
  | A.String (_, litstr) ->
    instr (ILitConst (String litstr))
  | A.Int (_, litstr) ->
    (* TODO deal with integer out of range *)
    instr (ILitConst (Int (Int64.of_string litstr)))
  | A.Null -> instr (ILitConst Null)
  | A.False -> instr (ILitConst False)
  | A.True -> instr (ILitConst True)
  | A.Lvar (_, x) when x = SN.SpecialIdents.this -> instr (IMisc This)
  | A.Lvar (_, x) -> instr_cgetl (Local.Named x)
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
    gather [from_expr e; instr_cgetl2 (Local.Named x); from_binop op]
  | A.Binop (op, e1, e2) ->
    gather [from_expr e1; from_expr e2; from_binop op]
  | A.Pipe (e1, e2) ->
    emit_pipe e1 e2
  | A.Dollardollar ->
    instr_cgetl2_pipe (* This will get rewritten into a temp local *)
  | A.InstanceOf (e1, (_, A.Id (_, id))) ->
    gather [from_expr e1; instr (IOp (InstanceOfD id))]
  | A.InstanceOf (e1, e2) ->
    gather [from_expr e1; from_expr e2; instr (IOp InstanceOf)]
  | A.NullCoalesce(e1, e2) ->
    let end_label = Label.next_regular () in
    gather [
      emit_quiet_expr e1;
      instr (IBasic Dup);
      instr (IIsset (IsTypeC OpNull));
      instr (IOp Not);
      instr_jmpnz end_label;
      instr (IBasic PopC);
      from_expr e2;
      instr_label end_label;
    ]
  | A.Cast(hint, e) ->
    gather [from_expr e; emit_cast hint]
  | A.Eif (etest, Some etrue, efalse) ->
    let false_label = Label.next_regular () in
    let end_label = Label.next_regular () in
    gather [
      from_expr etest;
      instr_jmpz false_label;
      from_expr etrue;
      instr_jmp end_label;
      instr_label false_label;
      from_expr efalse;
      instr_label end_label;
    ]
  | A.Eif (etest, None, efalse) ->
    let end_label = Label.next_regular () in
    gather [
      from_expr etest;
      instr (IBasic Dup);
      instr_jmpnz end_label;
      instr (IBasic PopC);
      from_expr efalse;
      instr_label end_label;
    ]
  | A.Expr_list es -> gather @@ List.map es ~f:from_expr
  | A.Call ((p, A.Id (_, "tuple")), es, _) ->
    (* Did you know that tuples are functions? *)
    let af_list = List.map es ~f:(fun e -> A.AFvalue e) in
    from_expr (p, A.Array af_list)
  | A.Call _ ->
    let instrs, flavor = emit_flavored_expr expr in
    gather [
      instrs;
      (* If the instruction has produced a ref then unbox it *)
      if flavor = Flavor.Ref then instr (IBasic UnboxR) else empty
    ]
  | A.New ((_, A.Id (_, id)), args, uargs) ->
      let nargs = List.length args + List.length uargs in
      gather [
        instr (ICall (FPushCtorD (nargs, id)));
        emit_args_and_call args uargs;
        instr (IBasic PopR)
      ]
  | A.Array es
  | A.Collection ((_, "dict"), es)
  | A.Collection ((_, "vec"), es)
  | A.Collection ((_, "keyset"), es) -> emit_collection expr es
  | A.Collection ((pos, "Set"), fields)  -> begin
    let collection_type = collection_type "Set" in
    match fields with
    | [] -> instr @@ ILitConst (NewCol collection_type)
    | _ -> gather
      [ from_expr (pos, A.Array fields)
      ; instr @@ ILitConst (ColFromArray collection_type)
      ]
  end
  | A.Collection ((_, "Pair"), fields) -> begin
    let collection_type = collection_type "Pair" in
    let values = List.fold_right
      fields
      ~f:(fun x acc ->
        expr_and_newc instr_col_add_new_elemc instr_col_add_new_elemc x::acc)
      ~init:[] in
    let newCol = instr @@ ILitConst (NewCol collection_type) in
    gather (newCol::values)
  end
  | A.Array_get(e1, Some e2) ->
    emit_array_get None e1 e2
  | A.Clone e ->
    gather [
      from_expr e;
      instr (IOp Clone)
    ]
  | A.Shape fl ->
    let are_values_all_literals =
      List.for_all fl ~f:(fun (_, e) -> is_literal e)
    in
    let p = fst expr in
    if are_values_all_literals then
      let fl =
        List.map fl
          ~f:(fun (fn, e) ->
                A.AFkvalue ((p,
                  A.String (extract_shape_field_name_pstring fn)), e))
      in
      from_expr (fst expr, A.Array fl)
    else
      let es = List.map fl ~f:(fun (_, e) -> from_expr e) in
      let keys = List.map fl ~f:(fun (fn, _) -> extract_shape_field_name fn) in
      gather [
        gather es;
        instr @@ ILitConst (NewStructArray keys);
      ]

  | A.Obj_get (expr, prop, nullflavor)  ->
    emit_obj_get expr prop nullflavor

  (* TODO *)
  | A.Collection ((_, type_str), _) ->
    emit_nyi @@ "collection: " ^ type_str
  | A.New _                     -> emit_nyi "new"
  | A.Yield_break               -> emit_nyi "yield_break"
  | A.Id _                      -> emit_nyi "id"
  | A.Id_type_arguments (_, _)  -> emit_nyi "id_type_arguments"
  | A.Lvarvar (_, _)            -> emit_nyi "lvarvar"
  | A.Array_get (_, _)          -> emit_nyi "array_get"
  | A.Class_get (_, _)          -> emit_nyi "class_get"
  | A.Class_const (_, _)        -> emit_nyi "class_const"
  | A.String2 _                 -> emit_nyi "string2"
  | A.Yield _                   -> emit_nyi "yield"
  | A.Await _                   -> emit_nyi "await"
  | A.List _                    -> emit_nyi "list"
  | A.Efun (_, _)               -> emit_nyi "efun"
  | A.Lfun _                    -> emit_nyi "lfun"
  | A.Xml (_, _, _)             -> emit_nyi "xml"
  | A.Unsafeexpr _              -> emit_nyi "unsafexpr"
  | A.Import (_, _)             -> emit_nyi "import"

and emit_static_collection expr es =
  let a_label = Label.get_next_data_label () in
  (* Arrays can either contains values or key/value pairs *)
  let need_index = match snd expr with
    | A.Collection ((_, "vec"), _)
    | A.Collection ((_, "keyset"), _) -> false
    | _ -> true
  in
  let _, es =
    List.fold_left
      es
      ~init:(0, [])
      ~f:(fun (index, l) x ->
            (index + 1, match x with
            | A.AFvalue e when need_index ->
              literal_from_expr e :: Int (Int64.of_int index) :: l
            | A.AFvalue e ->
              literal_from_expr e :: l
            | A.AFkvalue (k,v) ->
              literal_from_expr v :: literal_from_expr k :: l)
          )
  in
  let es = List.rev es in
  let lit_constructor = match snd expr with
    | A.Array _ -> Array (a_label, es)
    | A.Collection ((_, "dict"), _) -> Dict (a_label, es)
    | A.Collection ((_, "vec"), _) -> Vec (a_label, es)
    | A.Collection ((_, "keyset"), _) -> Keyset (a_label, es)
    | _ -> failwith "impossible"
  in
  instr (ILitConst lit_constructor)

and emit_dynamic_collection expr es =
  let is_only_values =
    List.for_all es ~f:(function A.AFkvalue _ -> false | _ -> true)
  in
  let count = List.length es in
  if is_only_values then begin
    let lit_constructor = match snd expr with
      | A.Array _ -> NewPackedArray count
      | A.Collection ((_, "vec"), _) -> NewVecArray count
      | A.Collection ((_, "keyset"), _) -> NewKeysetArray count
      | _ -> failwith "impossible"
    in
    gather [
      gather @@
      List.map es
        ~f:(function A.AFvalue e -> from_expr e | _ -> failwith "impossible");
      instr @@ ILitConst lit_constructor;
    ]
  end else begin
    let lit_constructor = match snd expr with
      | A.Array _ -> NewMixedArray count
      | A.Collection ((_, "dict"), _) -> NewDictArray count
      | _ -> failwith "impossible"
    in
    gather @@
      (instr @@ ILitConst lit_constructor) ::
      (List.map es ~f:(expr_and_newc instr_add_new_elemc instr_add_elemc))
  end

and emit_collection expr es =
  let all_literal = List.for_all es
    ~f:(function A.AFvalue e -> is_literal e
               | A.AFkvalue (k,v) -> is_literal k && is_literal v)
  in
  if all_literal then
    emit_static_collection expr es
  else
    emit_dynamic_collection expr es

and emit_pipe e1 e2 =
  let temp = Local.get_unnamed_local () in
  let fault_label = Label.next_fault () in
  let rewrite_dollardollar e =
    let rewriter i =
      match i with
      | IGet (CGetL2 Local.Pipe) ->
        IGet (CGetL2 temp)
      | _ -> i in
    InstrSeq.map e ~f:rewriter in
  gather [
    from_expr e1;
    instr_setl temp;
    instr_popc;
    instr_try_fault
      fault_label
      (* try block *)
      (rewrite_dollardollar (from_expr e2))
      (* fault block *)
      (gather [
        instr_unsetl temp;
        instr_unwind ]);
    instr_unsetl temp
  ]

and emit_logical_and e1 e2 =
  let left_is_false = Label.next_regular () in
  let right_is_true = Label.next_regular () in
  let its_done = Label.next_regular () in
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
  let its_true = Label.next_regular () in
  let its_done = Label.next_regular () in
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
  match expr_ with
  | A.Lvar (_, x) ->
    instr_cgetquietl (Local.Named x)
  | _ ->
    from_expr expr

(* Emit code for e1[e2].
 * If param_num_opt = Some i
 * then this is the i'th parameter to a function
 *)
and emit_array_get param_num_opt e1 e2 =
  let base_instrs, n = emit_base param_num_opt e1 in
  let mk, stack_size =
    match snd e2 with
      (* Special case for local index *)
    | A.Lvar (_, x) -> MemberKey.EL (Local.Named x), n
      (* Special case for literal integer index *)
    | A.Int (_, litstr) -> MemberKey.EI (Int64.of_string litstr), n
      (* Special case for literal string index *)
    | A.String (_, litstr) -> MemberKey.ET litstr, n
      (* General case *)
    | _ -> MemberKey.EC, n+1 in
  let final_instr =
    instr (IFinal (
      match param_num_opt with
      | None -> QueryM (stack_size, QueryOp.CGet, mk)
      | Some i -> FPassM (i, stack_size, mk)
    )) in
  match mk with
  | MemberKey.EC ->
    gather [
      from_expr e2;
      base_instrs;
      final_instr
    ]
  | _ ->
    gather [
      base_instrs;
      final_instr
    ]

(* TODO: Nullflavor is currently being ignored, start using it *)
and emit_obj_get expr prop _ =
  let base_instrs, n = emit_base None expr in
  let mk, stack_size =
    match snd prop with
    | A.Id (_, litstr) -> MemberKey.PT litstr, n
    | A.Lvar (_, x) -> MemberKey.PL (Local.Named x), n
      (* TODO: Work out when to use MemberKey.QT  *)
    | _ -> MemberKey.PC, n+1 in
    let final_instr =
      instr (IFinal (
        (* TODO: Work out when this should be FPassM *)
        match None with
        | None -> QueryM (stack_size, QueryOp.CGet, mk)
        | Some id -> FPassM (id, stack_size, mk)
      )) in
      gather [
        if mk = MemberKey.PC then from_expr prop else empty;
        base_instrs;
        final_instr
      ]

(* Emit instructions to construct base for `expr`, and also return
 * the stack size for subsequent query operations *)
and emit_base param_num_opt (_, expr_ as expr) =
  match expr_ with
  | A.Lvar (_, x) when x = SN.SpecialIdents.this ->
    gather [
     instr (IMisc CheckThis);
     instr (IBase BaseH)
    ], 0
  | A.Lvar (_, x) ->
    instr (IBase (
      match param_num_opt with
      | None -> BaseL (Local.Named x, MemberOpMode.Warn)
      | Some i -> FPassBaseL (i, Local.Named x)
      )), 0
  | _ ->
    let instrs, flavor = emit_flavored_expr expr in
    gather [
      instrs;
      instr (IBase (if flavor = Flavor.Ref then BaseR 0 else BaseC 0))
    ], 1

and instr_fpass kind i =
  match kind with
  | PassByRefKind.AllowCell -> instr (ICall (FPassC i))
  | PassByRefKind.WarnOnCell -> instr (ICall (FPassCW i))
  | PassByRefKind.ErrorOnCell -> instr (ICall (FPassCE i))

and instr_fpassr i = instr (ICall (FPassR i))

and emit_arg i ((_, expr_) as e) =
  match expr_ with
  | A.Lvar (_, x) -> instr_fpassl i (Local.Named x)

  | A.Array_get(e1, Some e2) ->
    emit_array_get (Some i) e1 e2

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

(* Emit code to construct the argument frame and then make the call *)
and emit_args_and_call args uargs =
  let all_args = args @ uargs in
  let nargs = List.length all_args in
  gather [
    gather (List.mapi all_args emit_arg);
    if uargs = []
    then instr (ICall (FCall nargs))
    else instr (ICall (FCallUnpack nargs))
  ]

and emit_call_lhs (_, expr_) nargs =
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
           instr (IOp Print);
           if i = nargs-1 then empty else instr_popc
         ] end in
    instrs, Flavor.Cell

  | A.Obj_get _ | A.Class_const _ | A.Id _ ->
    gather [
      emit_call_lhs expr nargs;
      emit_args_and_call args uargs;
    ], Flavor.Ref

  | _ ->
    emit_nyi "call expression", Flavor.Cell

(* Emit code for an expression that might leave a cell or reference on the
 * stack. Return which flavor it left.
 *)
and emit_flavored_expr (_, expr_ as expr) =
  match expr_ with
  | A.Call (e, args, uargs) ->
    emit_call e args uargs
  | _ ->
    from_expr expr, Flavor.Cell

and is_literal expr =
  match snd expr with
  | A.Float _
  | A.String _
  | A.Int _
  | A.Null
  | A.False
  | A.True -> true
  | _ -> false

and literal_from_expr expr =
  match snd expr with
  | A.Float (_, litstr) -> Double litstr
  | A.String (_, litstr) -> String litstr
  | A.Int (_, litstr) -> Int (Int64.of_string litstr)
  | A.Null -> Null
  | A.False -> False
  | A.True -> True
  (* TODO: HHVM does not allow <<F(2+2)>> in an attribute, but Hack does, and
   this seems reasonable to allow. Right now this will crash if given an
   expression rather than a literal in here.  In particular, see what unary
   minus does; do we allow it on a literal int? We should. *)
   | _ -> failwith "Expected a literal expression"

and literals_from_exprs_with_index exprs =
  List.rev @@ snd @@
  List.fold_left
    exprs
    ~init:(0, [])
    ~f:(fun (index, l) e ->
      (index + 1, literal_from_expr e :: Int (Int64.of_int index) :: l))

(* Emit code for an l-value, returning instructions and the location that
 * must be set. For now, this is just a local. *)
and emit_lval (_, expr_) =
  match expr_ with
  | A.Lvar (_, id) -> empty, Local.Named id
  | _ -> emit_nyi "lval expression", Local.Unnamed 0

and emit_assignment obop e1 e2 =
  let instrs1, lval = emit_lval e1 in
  let instrs2 = from_expr e2 in
  gather [instrs1; instrs2;
    match obop with
    | None -> instr (IMutator (SetL lval))
    | Some bop ->
      match binop_to_eqop bop with
      | None -> emit_nyi "op-assignment"
      | Some eqop -> instr (IMutator (SetOpL (lval, eqop)))
    ]

and emit_unop op e =
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

and from_exprs exprs =
  gather (List.map exprs from_expr)

and from_stmt st =
  match st with
  | A.Expr expr ->
    emit_ignored_expr expr
  | A.Return (_, None) ->
    gather [
      instr_null;
      instr_retc;
    ]
  | A.Return (_,  Some expr) ->
    gather [
      from_expr expr;
      instr_retc;
    ]
  | A.Block b -> from_stmts b
  | A.If (condition, consequence, alternative) ->
    from_if condition (A.Block consequence) (A.Block alternative)
  | A.While (e, b) ->
    from_while e (A.Block b)
  | A.Break _ ->
    instr_break 1 (* TODO: Break takes an argument *)
  | A.Continue _ ->
    instr_continue 1 (* TODO: Continue takes an argument *)
  | A.Do (b, e) ->
    from_do (A.Block b) e
  | A.For (e1, e2, e3, b) ->
    from_for e1 e2 e3 (A.Block b)
  | A.Throw e ->
    gather [
      from_expr e;
      instr (IContFlow Throw);
    ]
  | A.Try (try_block, catch_list, finally_block) ->
    if catch_list <> [] && finally_block <> [] then
      from_stmt (A.Try([A.Try (try_block, catch_list, [])], [], finally_block))
    else if catch_list <> [] then
      from_try_catch (A.Block try_block) catch_list
    else
      from_try_finally (A.Block try_block) (A.Block finally_block)

  | A.Switch (e, cl) ->
    from_switch e cl
  | A.Foreach (collection, await_pos, iterator, block) ->
    from_foreach (await_pos <> None) collection iterator
      (A.Block block)
  | A.Static_var _ ->
    emit_nyi "statement"
  (* TODO: What do we do with unsafe? *)
  | A.Unsafe
  | A.Fallthrough
  | A.Noop -> empty

and from_if condition consequence alternative =
  let alternative_label = Label.next_regular () in
  let done_label = Label.next_regular () in
  gather [
    from_expr condition;
    instr_jmpz alternative_label;
    from_stmt consequence;
    instr_jmp done_label;
    instr_label alternative_label;
    from_stmt alternative;
    instr_label done_label;
  ]

and from_while e b =
  let break_label = Label.next_regular () in
  let cont_label = Label.next_regular () in
  let start_label = Label.next_regular () in
  let cond = from_expr e in
  (* TODO: This is *bizarre* codegen for a while loop.
  It would be better to generate this as
  instr_label continue_label;
  from_expr e;
  instr_jmpz break_label;
  body;
  instr_jmp continue_label;
  instr_label break_label;
  *)
  let instrs = gather [
    cond;
    instr_jmpz break_label;
    instr_label start_label;
    from_stmt b;
    instr_label cont_label;
    cond;
    instr_jmpnz start_label;
    instr_label break_label;
  ] in
  CBR.rewrite_in_loop instrs cont_label break_label

and from_do b e =
  let cont_label = Label.next_regular () in
  let break_label = Label.next_regular () in
  let start_label = Label.next_regular () in
  let instrs = gather [
    instr_label start_label;
    from_stmt b;
    instr_label cont_label;
    from_expr e;
    instr_jmpnz start_label;
    instr_label break_label;
  ] in
  CBR.rewrite_in_loop instrs cont_label break_label

and from_for e1 e2 e3 b =
  let break_label = Label.next_regular () in
  let cont_label = Label.next_regular () in
  let start_label = Label.next_regular () in
  let cond = from_expr e2 in
  (* TODO: this is bizarre codegen for a "for" loop.
     This should be codegen'd as
     emit_ignored_expr initializer;
     instr_label start_label;
     from_expr condition;
     instr_jmpz break_label;
     body;
     instr_label continue_label;
     emit_ignored_expr increment;
     instr_jmp start_label;
     instr_label break_label;
  *)
  let instrs = gather [
    emit_ignored_expr e1;
    cond;
    instr_jmpz break_label;
    instr_label start_label;
    from_stmt b;
    instr_label cont_label;
    emit_ignored_expr e3;
    cond;
    instr_jmpnz start_label;
    instr_label break_label;
  ] in
  CBR.rewrite_in_loop instrs cont_label break_label

and from_switch e cl =
  let switched = from_expr e in
  let end_label = Label.next_regular () in
  (* "continue" in a switch in PHP has the same semantics as break! *)
  let cl = List.map cl ~f:from_case in
  let bodies = gather @@ List.map cl ~f:snd in
  let init = gather @@ List.map cl
    ~f: begin fun x ->
          let (e_opt, l) = fst x in
          match e_opt with
          | None -> instr_jmp l
          | Some e ->
            gather [from_expr e; switched; instr_eq; instr_jmpnz l]
        end
  in
  let instrs = gather [
    init;
    bodies;
    instr_label end_label;
  ] in
  CBR.rewrite_in_switch instrs end_label

and from_catch end_label ((_, catch_type), (_, catch_local), b) =
    (* Note that this is a "regular" label; we're not going to branch to
    it directly in the event of an exception. *)
    let next_catch = Label.next_regular () in
    gather [
      instr_dup;
      instr_instanceofd catch_type;
      instr_jmpz next_catch;
      instr_setl (Local.Named catch_local);
      instr_popc;
      from_stmt (A.Block b);
      instr_jmp end_label;
      instr_label next_catch;
    ]

and from_catches catch_list end_label =
  gather (List.map catch_list ~f:(from_catch end_label))

and from_try_catch try_block catch_list =
  let end_label = Label.next_regular () in
  let catch_label = Label.next_catch () in
  let try_body = gather [
    from_stmt try_block;
    instr_jmp end_label;
  ] in
  gather [
    instr_try_catch_begin catch_label;
    try_body;
    instr_try_catch_end;
    instr_label catch_label;
    instr_catch;
    from_catches catch_list end_label;
    instr_throw;
    instr_label end_label;
  ]

and from_try_finally try_block finally_block =
  (*
  We need to generate four things:
  (1) the try-body, which will be followed by
  (2) the normal-continuation finally body, and
  (3) an epilogue to the finally body that deals with finally-blocked
      break and continue
  (4) the exceptional-continuation fault body.
  *)

  (* (1) Try body

  The try body might have un-rewritten continues and breaks which
  branch to a label outside of the try. This means that we must
  first run the normal-continuation finally, and then branch to the
  appropriate label.

  We do this by running a rewriter which turns continues and breaks
  inside the try body into setting temp_local to an integer which indicates
  what action the finally must perform when it is finished, followed by a
  jump directly to the finally.
  *)
  let try_body = from_stmt try_block in
  let temp_local = Local.get_unnamed_local () in
  let finally_start = Label.next_regular () in
  let finally_end = Label.next_regular () in
  let cont_and_break = CBR.get_continues_and_breaks try_body in
  let try_body = CBR.rewrite_in_try_finally
    try_body cont_and_break temp_local finally_start in

  (* (2) Finally body

  Note that this is used both in the normal-continuation and
  exceptional-continuation cases; we generate the same code twice.

  TODO: We might consider changing the codegen so that the finally block
  is only generated once. We could do this by making the fault block set a
  temp local to -1, and then branch to the finally block. In the finally block
  epilogue it can check to see if the local is -1, and if so, issue an unwind
  instruction.

  It is illegal to have a continue or break which branches out of a finally.
  Unfortunately we at present do not detect this at parse time; rather, we
  generate an exception at run-time by rewriting continue and break
  instructions found inside finally blocks.

  TODO: If we make this illegal at parse time then we can remove this pass.
  *)
  let finally_body = from_stmt finally_block in
  let finally_body = CBR.rewrite_in_finally finally_body in

  (* (3) Finally epilogue *)

  let finally_epilogue =
    CBR.emit_finally_epilogue cont_and_break temp_local finally_end in

  (* (4) Fault body

  We now emit the fault body; it is just cleanup code for the temp_local,
  a copy of the finally body (without the branching epilogue, since we are
  going to unwind rather than branch), and an unwind instruction.

  TODO: The HHVM emitter sometimes emits seemingly spurious
  unset-unnamed-local instructions into the fault block.  These look
  like bugs in the emitter. Investigate; if they are bugs in the HHVM
  emitter, get them fixed there. If not, get a clear explanation of
  what they are for and why they are required.
  *)

  let cleanup_local =
    if cont_and_break = [] then empty else instr_unsetl temp_local in
  let fault_body = gather [
      cleanup_local;
      finally_body;
      instr_unwind;
    ] in
  let fault_label = Label.next_fault () in
  (* Put it all together. *)
  gather [
    instr_try_fault fault_label try_body fault_body;
    instr_label finally_start;
    finally_body;
    finally_epilogue;
    instr_label finally_end;
  ]

and get_foreach_lvalue e =
  match e with
  | A.Lvar (_, x) -> Some (Local.Named x)
  | _ -> None

and get_foreach_key_value iterator =
  match iterator with
  | A.As_kv ((_, k), (_, v)) ->
    begin match get_foreach_lvalue v with
    | None -> None
    | Some v_lid ->
      match get_foreach_lvalue k with
      | None -> None
      | Some k_lid ->
        Some (Some k_lid, v_lid)
    end
  | A.As_v (_, v) ->
    begin match get_foreach_lvalue v with
    | None -> None
    | Some lid -> Some (None, lid)
    end

and from_foreach _has_await collection iterator block =
  (* TODO: await *)
  (* TODO: generate .numiters based on maximum nesting depth *)
  (* TODO: We need to be able to process arbitrary lvalues in the key, value
     pair. This will require writing a preamble into the block, in the general
     case. For now we just support locals. *)
  let iterator_number = Iterator.get_iterator () in
  let fault_label = Label.next_fault () in
  let loop_continue_label = Label.next_regular () in
  let loop_break_label = Label.next_regular () in
  match get_foreach_key_value iterator with
  | None -> emit_nyi "foreach codegen does not support arbitrary lvalues yet"
  | Some (k, v) ->
    let init, next = match k with
    | Some k ->
      let init = instr_iterinitk iterator_number loop_break_label k v in
      let cont = instr_iternextk iterator_number loop_continue_label k v in
      init, cont
    | None ->
      let init = instr_iterinit iterator_number loop_break_label v in
      let cont = instr_iternext iterator_number loop_continue_label v in
      init, cont in
    let body = from_stmt block in
    let result = gather [
      from_expr collection;
      init;
      instr_try_fault
        fault_label
        (* try body *)
        (gather [
          instr_label loop_continue_label;
          body;
          next
        ])
        (* fault body *)
        (gather [
          instr_iterfree iterator_number;
          instr_unwind ]);
      instr_label loop_break_label
    ] in
    CBR.rewrite_in_loop
      result loop_continue_label loop_break_label

and from_stmts stl =
  let results = List.map stl from_stmt in
  gather results

and from_case c =
  let l = Label.next_regular () in
  let b = match c with
    | A.Default b
    | A.Case (_, b) ->
        from_stmt (A.Block b)
  in
  let e = match c with
    | A.Case (e, _) -> Some e
    | _ -> None
  in
  (e, l), gather [instr_label l; b]

(* TODO *)
let fmt_name s = s

let fmt_name_or_prim x =
  match x with
  | "void" -> "HH\\void"
  | "int" -> "HH\\int"
  | "bool" -> "HH\\bool"
  | "float" -> "HH\\float"
  | "string" -> "HH\\string"
  | "num" -> "HH\\num"
  | "resource" -> "HH\\resource"
  | "arraykey" -> "HH\\arraykey"
  | "noreturn" -> "HH\\noreturn"
  | "mixed" -> "HH\\mixed"
  | "this" -> "HH\\this"
  | _ -> fmt_name x

(* Produce the "userType" bit of the annotation *)
let rec fmt_hint (_, h) =
  match h with
  | A.Happly ((_, s), []) -> fmt_name_or_prim s
  | A.Happly ((_, s), args) ->
    fmt_name_or_prim s ^ "<" ^ String.concat ", " (List.map args fmt_hint) ^ ">"

  | A.Hfun (args, _, ret) ->
    "(function (" ^ String.concat ", " (List.map args fmt_hint) ^ "): " ^
      fmt_hint ret ^ ")"

  | A.Htuple hs ->
    "(" ^ String.concat ", " (List.map hs fmt_hint) ^ ")"

  | A.Haccess (h1, h2, accesses) ->
    String.concat "::" (List.map (h1::h2::accesses) snd)

  | A.Hoption t -> "?" ^ fmt_hint t

  | A.Hshape smap ->
    let fmt_field = function
      | A.SFlit (_, s) -> "'" ^ s ^ "'"
      | A.SFclass_const ((_, s1), (_, s2)) -> fmt_name s1 ^ "::" ^ s2
    in
    let format_shape_field ({ A.sf_name; A.sf_hint; _ }) =
      fmt_field sf_name ^ "=>" ^ fmt_hint sf_hint in
    let shape_fields =
      List.map ~f:format_shape_field smap in
    "HH\\shape(" ^ String.concat ", " shape_fields ^ ")"

let rec hint_to_type_constraint tparams (_, h) =
match h with
| A.Happly ((_, ("mixed" | "this" | "void")), []) ->
  TC.make None []

| A.Hfun (_, _, _) ->
  TC.make None []

| A.Haccess _ ->
  let tc_name = Some "" in
  let tc_flags = [TC.HHType; TC.ExtendedHint; TC.TypeConstant] in
  TC.make tc_name tc_flags

(* Need to differentiate between type params and classes *)
| A.Happly ((_, s), _) ->
  if List.mem tparams s then
    let tc_name = Some "" in
    let tc_flags = [TC.HHType; TC.ExtendedHint; TC.TypeVar] in
    TC.make tc_name tc_flags
  else
    let tc_name = Some (fmt_name_or_prim s) in
    let tc_flags = [TC.HHType] in
    TC.make tc_name tc_flags

(* Shapes and tuples are just arrays *)
| A.Hshape _ |  A.Htuple _ ->
  let tc_name = Some "array" in
  let tc_flags = [TC.HHType] in
  TC.make tc_name tc_flags

| A.Hoption t ->
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
  let param_name = snd p.A.param_id in
  let param_type_info = Option.map p.A.param_hint
    (hint_to_type_info ~always_extended:false tparams) in
  let param_default_value = Option.map p.A.param_expr
    ~f:(fun e -> Label.next_default_arg (), e)
  in
  Hhas_param.make param_name param_type_info param_default_value

let has_type_constraint ti =
  match ti with
  | Some ti when (Hhas_type_info.has_type_constraint ti) -> true
  | _ -> false

let emit_method_prolog params =
  gather (List.filter_map params (fun p ->
    let param_type_info = Hhas_param.type_info p in
    let param_name = Hhas_param.name p in
    if has_type_constraint param_type_info
    then Some (instr (IMisc (VerifyParamType (Param_named param_name))))
    else None))

let emit_param_default_value_setter params =
  let setters = List.filter_map params (fun p ->
    let param_name = Hhas_param.name p in
    let dvo = Hhas_param.default_value p in
    Option.map dvo (fun (l, e) ->
      gather [
        instr_label l;
        from_expr e;
        instr_setl (Local.Named param_name);
        instr_popc;
      ]) )
  in
  if List.length setters = 0
  then empty, empty
  else
    let l = Label.next_regular () in
    instr_label l, gather [gather setters; instr_jmpns l]

let tparams_to_strings tparams =
  List.map tparams (fun (_, (_, s), _) -> s)

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
let emit_fault_instructions instrseq =
  let rec aux instrseq acc =
    match instrseq with
    | Instr_try_fault (try_body, fault_body) ->
      aux try_body (fault_body :: acc)
    | Instr_list _ -> acc
    | Instr_concat ([]) -> acc
    | Instr_concat (h :: t) -> aux (Instr_concat t) (aux h acc) in
  gather (aux instrseq [])

let verify_returns body =
  let rewriter i =
    match i with
    | IContFlow RetC ->
      [ (IMisc VerifyRetTypeC); (IContFlow RetC) ]
    | _ -> [ i ] in
  InstrSeq.flat_map body ~f:rewriter

let from_body tparams params ret b =
  Label.reset_label ();
  Local.reset_local ();
  Iterator.reset_iterator ();
  let params = List.map params (from_param tparams) in
  let return_type_info = Option.map ret
    (hint_to_type_info ~always_extended:true tparams) in
  let stmt_instrs = from_stmts b in
  let stmt_instrs =
    if has_type_constraint return_type_info then
      verify_returns stmt_instrs
    else
      stmt_instrs in
  let ret_instrs =
    match List.last b with Some (A.Return _) -> empty | _ ->
    gather [instr_null; instr_retc]
  in
  let fault_instrs = emit_fault_instructions stmt_instrs in
  let begin_label, default_value_setters =
    emit_param_default_value_setter params
  in
  let body_instrs = gather [
    begin_label;
    emit_method_prolog params;
    stmt_instrs;
    ret_instrs;
    default_value_setters;
    fault_instrs;
  ] in
  body_instrs, params, return_type_info

let extract_decl_vars instrseq =
  let module ULS = Unique_list_string in
  (* TODO: Both reads and writes need to go into decl vars *)
  (* TODO: Default arguments should not be in declvars *)
  let folder uniq_list instruction =
    match instruction with
    | IMutator (SetL (Local.Named s)) -> ULS.add uniq_list s
    | _ -> uniq_list in
  let decl_vars = InstrSeq.fold_left instrseq ~init:ULS.empty ~f:folder in
  List.rev (ULS.items decl_vars)

(* Create a map from defined labels to instruction offset *)
let create_label_to_offset_map instrseq =
  snd @@
  InstrSeq.fold_left instrseq ~init:(0, IMap.empty) ~f:(fun (i, m) instr ->
    begin match instr with
    | ILabel (Label.Regular l) -> (i, IMap.add l i m)
    | _        -> (i + 1, m)
    end)

let lookup_def l defs =
  match IMap.get l defs with
  | None -> failwith "lookup_def: label missing"
  | Some ix -> ix

(* Generate new labels for all labels referenced in instructions, in the
 * order that the instructions appear. Also record which labels are
 *)
let create_label_ref_map defs instrseq =
  snd @@
  InstrSeq.fold_left instrseq ~init:(0, (ISet.empty, IMap.empty))
    ~f:(fun acc instr ->
    let process_ref (n, (used, refs) as acc) l =
      let l = Label.id l in
      let ix = lookup_def l defs in
      match IMap.get ix refs with
      (* This is the first time we've seen a reference to a label for
       * this instruction offset, so generate a new label *)
      | None -> (n + 1, (ISet.add l used, IMap.add ix n refs))
      (* We already have a label for this instruction offset *)
      | Some _ -> acc in
    match instr with
    | IIterator (IterInit (_, l, _))
    | IIterator (IterInitK (_, l, _, _))
    | IIterator (WIterInit (_, l, _))
    | IIterator (WIterInitK (_, l, _, _))
    | IIterator (MIterInit (_, l, _))
    | IIterator (MIterInitK (_, l, _, _))
    | IIterator (IterNext (_, l, _))
    | IIterator (IterNextK (_, l, _, _))
    | IIterator (WIterNext (_, l, _))
    | IIterator (WIterNextK (_, l, _, _))
    | IIterator (MIterNext (_, l, _))
    | IIterator (MIterNextK (_, l, _, _))
    | IContFlow (Jmp l | JmpNS l | JmpZ l | JmpNZ l) ->
      process_ref acc l
    | IContFlow (Switch (_, _, ls)) ->
      List.fold_left ls ~f:process_ref ~init:acc
    | IContFlow (SSwitch pairs) ->
      List.fold_left pairs ~f:(fun acc (_,l) -> process_ref acc l) ~init:acc
    (* TODO: other uses of Label.t in instructions:
      DecodeCufIter
      IterBreak
     *)
    | _ -> acc)

(* Relabel the instruction sequence so that
 *   1. No instruction is preceded by more than one label
 *   2. No label is unreferenced
 *   3. References to labels occur in strict label number order, starting at 0
 *)
let relabel_instrseq instrseq =
  let defs = create_label_to_offset_map instrseq in
  let used, refs = create_label_ref_map defs instrseq in
  let relabel l =
    let l = Label.id l in
    let ix = lookup_def l defs in
    match IMap.get ix refs with
    | None -> failwith "relabel_instrseq: offset not in refs"
    | Some l' -> Label.Regular l' in
  InstrSeq.filter_map instrseq ~f:(fun instr ->
    match instr with
    | IIterator (IterInit (id, l, v)) ->
      Some (IIterator (IterInit (id, relabel l, v)))
    | IIterator (IterInitK (id, l, k, v)) ->
      Some (IIterator (IterInitK (id, relabel l, k, v)))
    | IIterator (WIterInit (id, l, v)) ->
      Some (IIterator (WIterInit (id, relabel l, v)))
    | IIterator (WIterInitK (id, l, k, v)) ->
      Some (IIterator (WIterInitK (id, relabel l, k, v)))
    | IIterator (MIterInit (id, l, v)) ->
      Some (IIterator (MIterInit (id, relabel l, v)))
    | IIterator (MIterInitK (id, l, k, v)) ->
      Some (IIterator (MIterInitK (id, relabel l, k, v)))
    | IIterator (IterNext (id, l, v)) ->
      Some (IIterator (IterNext (id, relabel l, v)))
    | IIterator (IterNextK (id, l, k, v)) ->
      Some (IIterator (IterNextK (id, relabel l, k, v)))
    | IIterator (WIterNext (id, l, v)) ->
      Some (IIterator (WIterNext (id, relabel l, v)))
    | IIterator (WIterNextK (id, l, k, v)) ->
      Some (IIterator (WIterNextK (id, relabel l, k, v)))
    | IIterator (MIterNext (id, l, v)) ->
      Some (IIterator (MIterNext (id, relabel l, v)))
    | IIterator (MIterNextK (id, l, k, v)) ->
      Some (IIterator (MIterNextK (id, relabel l, k, v)))
    | IContFlow (Jmp l)   -> Some (IContFlow (Jmp (relabel l)))
    | IContFlow (JmpNS l) -> Some (IContFlow (JmpNS (relabel l)))
    | IContFlow (JmpZ l)  -> Some (IContFlow (JmpZ (relabel l)))
    | IContFlow (JmpNZ l) -> Some (IContFlow (JmpNZ (relabel l)))
    | IContFlow (Switch (k, n, ll)) ->
      Some (IContFlow (Switch (k, n, List.map ll relabel)))
    | IContFlow (SSwitch pairs) ->
      Some (IContFlow (SSwitch
        (List.map pairs (fun (id,l) -> (id, relabel l)))))
    (* TODO: other uses of Label.t in instructions:
      DecodeCufIter
      IterBreak
     *)
    | ILabel (Label.Regular l) ->
      (* TODO: Write test cases for things like catch and fault labels followed
      by loop start labels. *)
      if ISet.mem l used then
        let ix = lookup_def l defs in
        begin match IMap.get ix refs with
        | Some l' -> Some (ILabel (Label.Regular l'))
        | None -> None
        end
      else None
    | _ -> Some instr)

let from_fun_ : A.fun_ -> Hhas_function.t option =
  fun ast_fun ->
  let function_name = Litstr.to_string @@ snd ast_fun.A.f_name in
  match ast_fun.A.f_body with
  | b ->
    let tparams = tparams_to_strings ast_fun.A.f_tparams in
    let body_instrs, function_params, function_return_type =
      from_body tparams ast_fun.A.f_params ast_fun.A.f_ret b in
    let body_instrs = relabel_instrseq body_instrs in
    let function_decl_vars = extract_decl_vars body_instrs in
    let function_body = instr_seq_to_list body_instrs in
    Some (Hhas_function.make
      function_name
      function_params
      function_return_type
      function_body
      function_decl_vars)

let from_functions ast_functions =
  List.filter_map ast_functions from_fun_

let from_attribute_base attribute_name arguments =
  let attribute_arguments = literals_from_exprs_with_index arguments in
  Hhas_attribute.make attribute_name attribute_arguments

let from_attribute : A.user_attribute -> Hhas_attribute.t =
  fun ast_attr ->
  let attribute_name = Litstr.to_string @@ snd ast_attr.A.ua_name in
  from_attribute_base attribute_name ast_attr.A.ua_params

let from_attributes ast_attributes =
  (* The list of attributes is reversed in the A. *)
  List.map (List.rev ast_attributes) from_attribute

let from_method : A.class_ -> A.method_ -> Hhas_method.t option =
  fun ast_class ast_method ->
  let class_tparams = tparams_to_strings ast_class.A.c_tparams in
  let method_name = Litstr.to_string @@ snd ast_method.A.m_name in
  let method_is_abstract = List.mem ast_method.A.m_kind A.Abstract in
  let method_is_final = List.mem ast_method.A.m_kind A.Final in
  let method_is_private = List.mem ast_method.A.m_kind A.Private in
  let method_is_protected = List.mem ast_method.A.m_kind A.Protected in
  let method_is_public = List.mem ast_method.A.m_kind A.Public in
  let method_is_static = List.mem ast_method.A.m_kind A.Static in
  let method_attributes = from_attributes ast_method.A.m_user_attributes in
  match ast_method.A.m_body with
  | b ->
    let method_tparams = tparams_to_strings ast_method.A.m_tparams in
    let tparams = class_tparams @ method_tparams in
    let body_instrs, method_params, method_return_type =
      from_body tparams ast_method.A.m_params ast_method.A.m_ret b in
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

let from_methods ast_class ast_methods =
  List.filter_map ast_methods (from_method ast_class)

let is_interface ast_class =
  ast_class.A.c_kind = Ast.Cinterface

let default_constructor ast_class =
  let method_attributes = [] in
  let method_name = "86ctor" in
  let method_body = instr_seq_to_list (gather [
    instr (ILitConst Null);
    instr (IContFlow RetC)
  ]) in
  let method_is_abstract = is_interface ast_class in
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

let from_class_var cv_kind_list class_var =
  let (_, (_, cv_name), _) = class_var in
  (* TODO: xhp, type, initializer *)
  (* TODO: Hack allows a property to be marked final, which is nonsensical.
  HHVM does not allow this.  Fix this in the Hack parser? *)
  let property_name = Litstr.to_string @@ cv_name in
  let property_is_private = List.mem cv_kind_list A.Private in
  let property_is_protected = List.mem cv_kind_list A.Protected in
  let property_is_public = List.mem cv_kind_list A.Public in
  let property_is_static = List.mem cv_kind_list A.Static in
  Hhas_property.make
    property_is_private
    property_is_protected
    property_is_public
    property_is_static
    property_name

let from_constant (_hint, name, const_init) =
  (* The type hint is omitted. *)
  match const_init with
  | None -> None (* Abstract constants are omitted *)
  | Some _init ->
    (* TODO: Deal with the initializer *)
    let constant_name = Litstr.to_string @@ snd name in
    Some (Hhas_constant.make constant_name)

let from_constants ast_constants =
  List.filter_map ast_constants from_constant

let from_type_constant ast_type_constant =
  match ast_type_constant.A.tconst_type with
  | None -> None (* Abstract type constants are omitted *)
  | Some _init ->
    (* TODO: Deal with the initializer *)
    let type_constant_name = Litstr.to_string @@
      snd ast_type_constant.A.tconst_name in
    Some (Hhas_type_constant.make type_constant_name)

let from_type_constants ast_type_constants =
  List.filter_map ast_type_constants from_type_constant

let from_class_elt_method ast_class elt =
  match elt with
  | A.Method m -> from_method ast_class m
  | _ -> None

let from_class_elt_classvars elt =
  match elt with
  | A.ClassVars (kind_list, _, cvl) ->
    List.map cvl (from_class_var kind_list)
  | _ -> []

let from_class_elt_constants elt =
  match elt with
  | A.Const(hint_opt, l) ->
    List.filter_map l (fun (id, e) -> from_constant (hint_opt, id, Some e))
  | _ -> []

let from_class_elt_typeconsts elt =
  match elt with
  | A.TypeConst tc -> from_type_constant tc
  | _ -> None

let from_class : A.class_ -> Hhas_class.t =
  fun ast_class ->
  let class_attributes = from_attributes ast_class.A.c_user_attributes in
  let class_name = Litstr.to_string @@ snd ast_class.A.c_name in
  let class_is_trait = ast_class.A.c_kind = Ast.Ctrait in
  let class_is_enum = ast_class.A.c_kind = Ast.Cenum in
  let class_is_interface = is_interface ast_class in
  let class_is_abstract = ast_class.A.c_kind = Ast.Cabstract in
  let class_is_final =
    ast_class.A.c_final || class_is_trait || class_is_enum in
  let tparams = tparams_to_strings ast_class.A.c_tparams in
  let class_base =
    if class_is_interface then None
    else from_extends tparams ast_class.A.c_extends in
  let implements =
    if class_is_interface then ast_class.A.c_extends
    else ast_class.A.c_implements in
  let class_implements = from_implements tparams implements in
  let class_body = ast_class.A.c_body in
  let has_constructor = List.exists class_body
    (fun elt -> match elt with
                | A.Method { A.m_name; _} -> snd m_name = SN.Members.__construct
                | _ -> false) in
  let class_methods =
    if has_constructor then [] else [default_constructor ast_class] in
  let class_methods =
    List.filter_map class_body (from_class_elt_method ast_class)
    @ class_methods in
  let class_properties = List.concat_map class_body from_class_elt_classvars in
  let class_constants = List.concat_map class_body from_class_elt_constants in
  let class_type_constants =
    List.filter_map class_body from_class_elt_typeconsts in
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

let from_classes ast_classes =
  List.map ast_classes from_class
