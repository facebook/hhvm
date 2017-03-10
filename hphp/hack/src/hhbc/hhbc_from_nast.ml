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

(* Locals, array elements, and properties all support the same range of l-value
 * operations. *)
module LValOp = struct
  type t =
  | Set
  | SetOp of eq_op
  | IncDec of incdec_op
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
    gather [
      emit_two_exprs k v;
      instr_to_add
    ]

and from_local x =
  if x = SN.SpecialIdents.this then instr_this
  else instr_cgetl (Local.Named x)

and emit_two_exprs e1 e2 =
  (* Special case to make use of CGetL2 *)
  match e1 with
  | (_, A.Lvar (_, local)) ->
    gather [
      from_expr e2;
      instr_cgetl2 (Local.Named local);
    ]
  | _ ->
    gather [
      from_expr e1;
      from_expr e2;
    ]

and emit_binop op e1 e2 =
  match op with
  | A.AMpamp -> emit_logical_and e1 e2
  | A.BArbar -> emit_logical_or e1 e2
  | A.Eq None -> emit_lval_op LValOp.Set e1 (Some e2)
  | A.Eq (Some obop) ->
    begin match binop_to_eqop obop with
    | None -> emit_nyi "illegal eq op"
    | Some op -> emit_lval_op (LValOp.SetOp op) e1 (Some e2)
    end
  | _ ->
    gather [
      emit_two_exprs e1 e2;
      from_binop op
    ]

and emit_instanceof e1 e2 =
  match (e1, e2) with
  | (_, (_, A.Id (_, id))) ->
    gather [
      from_expr e1;
      instr_instanceofd id ]
  | _ ->
    gather [
      from_expr e1;
      from_expr e2;
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
    from_expr e2;
    instr_label end_label;
  ]

(* TODO: there are lots of ways of specifying the same type in a cast.
 * Sort this out!
 *)
and emit_cast hint expr =
  let op = match hint with
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
  | _ -> emit_nyi "cast type" in
  gather [
    from_expr expr;
    op;
  ]

and emit_conditional_expression etest etrue efalse =
  match etrue with
  | Some etrue ->
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
  | None ->
    let end_label = Label.next_regular () in
    gather [
      from_expr etest;
      instr_dup;
      instr_jmpnz end_label;
      instr_popc;
      from_expr efalse;
      instr_label end_label;
    ]

and emit_new_id id args uargs =
  let nargs = List.length args + List.length uargs in
  gather [
    instr_fpushctord nargs id;
    emit_args_and_call args uargs;
    instr_popr;
  ]

and emit_new typename args uargs =
  match typename with
  | A.Id (_, id) -> emit_new_id id args uargs
  | _ -> emit_nyi "new" (* TODO *)

and emit_clone expr =
  gather [
    from_expr expr;
    instr_clone;
  ]

and emit_shape expr fl =
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
      instr_newstructarray keys;
    ]

and emit_named_collection expr pos name fields =
  match name with
  | "dict" | "vec" | "keyset" -> emit_collection expr fields
  | "Set" -> begin
    let collection_type = collection_type "Set" in
    match fields with
    | [] -> instr_newcol collection_type
    | _ -> gather [
      from_expr (pos, A.Array fields);
      instr_colfromarray collection_type ]
    end
  | "Pair" -> begin
    let collection_type = collection_type "Pair" in
    let values = gather @@ List.fold_right
      fields
      ~f:(fun x acc ->
        expr_and_newc instr_col_add_new_elemc instr_col_add_new_elemc x::acc)
      ~init:[] in
    gather [
      instr_newcol collection_type;
      values ]
  end
  | _ -> emit_nyi @@ "collection: " ^ name (* TODO: Are there more? *)

and emit_tuple p es =
  (* Did you know that tuples are functions? *)
  let af_list = List.map es ~f:(fun e -> A.AFvalue e) in
  from_expr (p, A.Array af_list)

and emit_call_expr expr =
  let instrs, flavor = emit_flavored_expr expr in
  gather [
    instrs;
    (* If the instruction has produced a ref then unbox it *)
    if flavor = Flavor.Ref then instr_unboxr else empty
  ]

and emit_class_const cid id =
  if id = SN.Members.mClass then instr_string cid
  else emit_nyi "class_const" (* TODO *)

and from_expr expr =
  (* Note that this takes an Ast.expr, not a Nast.expr. *)
  match snd expr with
  | A.Float (_, litstr) -> instr_double litstr
  | A.String (_, litstr) -> instr_string litstr
  (* TODO deal with integer out of range *)
  | A.Int (_, litstr) -> instr_int_of_string litstr
  | A.Null -> instr_null
  | A.False -> instr_false
  | A.True -> instr_true
  | A.Lvar (_, x) -> from_local x
  | A.Class_const ((_, cid), (_, id)) -> emit_class_const cid id
  | A.Unop (op, e) -> emit_unop op e
  | A.Binop (op, e1, e2) -> emit_binop op e1 e2
  | A.Pipe (e1, e2) -> emit_pipe e1 e2
  | A.Dollardollar -> instr_cgetl2 Local.Pipe
  | A.InstanceOf (e1, e2) -> emit_instanceof e1 e2
  | A.NullCoalesce (e1, e2) -> emit_null_coalesce e1 e2
  | A.Cast((_, hint), e) -> emit_cast hint e
  | A.Eif (etest, etrue, efalse) ->
    emit_conditional_expression etest etrue efalse
  | A.Expr_list es -> gather @@ List.map es ~f:from_expr
  | A.Call ((p, A.Id (_, "tuple")), es, _) -> emit_tuple p es
  | A.Call _ -> emit_call_expr expr
  | A.New ((_, typename), args, uargs) -> emit_new typename args uargs
  | A.Array es -> emit_collection expr es
  | A.Collection ((pos, name), fields) ->
    emit_named_collection expr pos name fields
  | A.Array_get(base_expr, opt_elem_expr) ->
    emit_array_get None base_expr opt_elem_expr
  | A.Clone e -> emit_clone e
  | A.Shape fl -> emit_shape expr fl
  | A.Obj_get (expr, prop, nullflavor) -> emit_obj_get None expr prop nullflavor
  (* TODO *)
  | A.Yield_break               -> emit_nyi "yield_break"
  | A.Id _                      -> emit_nyi "id"
  | A.Id_type_arguments (_, _)  -> emit_nyi "id_type_arguments"
  | A.Lvarvar (_, _)            -> emit_nyi "lvarvar"
  | A.Class_get (_, _)          -> emit_nyi "class_get"
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
  stash_in_local e1
  begin fun temp _break_label ->
  let rewrite_dollardollar e =
    let rewriter i =
      match i with
      | IGet (CGetL2 Local.Pipe) ->
        IGet (CGetL2 temp)
      | _ -> i in
    InstrSeq.map e ~f:rewriter in
  rewrite_dollardollar (from_expr e2)
  end

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
and emit_array_get param_num_opt base_expr opt_elem_expr =
  let elem_expr_instrs, elem_stack_size = emit_elem_instrs opt_elem_expr in
  let base_expr_instrs, base_setup_instrs, base_stack_size =
    emit_base MemberOpMode.Warn elem_stack_size param_num_opt base_expr in
  let mk = get_elem_member_key 0 opt_elem_expr in
  let total_stack_size = elem_stack_size + base_stack_size in
  let final_instr =
    instr (IFinal (
      match param_num_opt with
      | None -> QueryM (total_stack_size, QueryOp.CGet, mk)
      | Some i -> FPassM (i, total_stack_size, mk)
    )) in
  gather [
    base_expr_instrs;
    elem_expr_instrs;
    base_setup_instrs;
    final_instr
  ]

(* Emit code for e1->e2 or e1?->e2.
 * If param_num_opt = Some i
 * then this is the i'th parameter to a function
 *)
and emit_obj_get param_num_opt expr prop null_flavor =
  let prop_expr_instrs, prop_stack_size = emit_prop_instrs prop in
  let base_expr_instrs, base_setup_instrs, base_stack_size =
    emit_base MemberOpMode.Warn prop_stack_size param_num_opt expr in
  let mk = get_prop_member_key null_flavor 0 prop in
  let total_stack_size = prop_stack_size + base_stack_size in
  let final_instr =
    instr (IFinal (
      match param_num_opt with
      | None -> QueryM (total_stack_size, QueryOp.CGet, mk)
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
  | Some (_, (A.Lvar _ | A.Int _ | A.String _)) -> empty, 0
  | Some expr -> from_expr expr, 1
  | None -> empty, 0

and emit_prop_instrs (_, expr_ as expr) =
  match expr_ with
  (* These all have special inline versions of member keys *)
  | A.Lvar _ | A.Id _ -> empty, 0
  | _ -> from_expr expr, 1

(* Get the member key for an array element expression: the `elem` in
 * expressions of the form `base[elem]`.
 * If the array element is missing, use the special key `W`.
 *)
and get_elem_member_key stack_index opt_expr =
  match opt_expr with
  (* Special case for local *)
  | Some (_, A.Lvar (_, x)) -> MemberKey.EL (Local.Named x)
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
    begin match null_flavor with
    | Ast.OG_nullthrows -> MemberKey.PT str
    | Ast.OG_nullsafe -> MemberKey.QT str
    end
  (* Special case for local *)
  | (_, A.Lvar (_, x)) -> MemberKey.PL (Local.Named x)
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
   match expr_ with
   | A.Lvar (_, x) when x = SN.SpecialIdents.this ->
     instr (IMisc CheckThis),
     instr (IBase BaseH),
     0

   | A.Lvar (_, x) ->
     empty,
     instr (IBase (
       match param_num_opt with
       | None -> BaseL (Local.Named x, mode)
       | Some i -> FPassBaseL (i, Local.Named x)
       )),
     0

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
  | A.Lvar (_, x) -> instr_fpassl i (Local.Named x)

  | A.Array_get(base_expr, opt_elem_expr) ->
    emit_array_get (Some i) base_expr opt_elem_expr

  | A.Obj_get(e1, e2, nullflavor) ->
    emit_obj_get (Some i) e1 e2 nullflavor

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

and emit_final_member_op stack_index op mk =
  match op with
  | LValOp.Set -> instr (IFinal (SetM (stack_index, mk)))
  | LValOp.SetOp op -> instr (IFinal (SetOpM (stack_index, op, mk)))
  | LValOp.IncDec op -> instr (IFinal (IncDecM (stack_index, op, mk)))

and emit_final_local_op op lid =
  match op with
  | LValOp.Set -> instr (IMutator (SetL lid))
  | LValOp.SetOp op -> instr (IMutator (SetOpL (lid, op)))
  | LValOp.IncDec op -> instr (IMutator (IncDecL (lid, op)))

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
      | Some e -> from_expr e, 1 in
      emit_lval_op_nonlist op expr1 rhs_instrs rhs_stack_size

and emit_lval_op_nonlist op expr1 rhs_instrs rhs_stack_size =
  match expr1 with
  | (_, A.Lvar (_, id)) ->
    gather [
      rhs_instrs;
      emit_final_local_op op (Local.Named id)
    ]

  | (_, A.Array_get(base_expr, opt_elem_expr)) ->
    let elem_expr_instrs, elem_stack_size = emit_elem_instrs opt_elem_expr in
    let base_offset = elem_stack_size + rhs_stack_size in
    let base_expr_instrs, base_setup_instrs, base_stack_size =
      emit_base MemberOpMode.Define base_offset None base_expr in
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

  | (_, A.Obj_get(e1, e2, null_flavor)) ->
    let prop_expr_instrs, prop_stack_size = emit_prop_instrs e2 in
    let base_offset = prop_stack_size + rhs_stack_size in
    let base_expr_instrs, base_setup_instrs, base_stack_size =
      emit_base MemberOpMode.Define base_offset None e1 in
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

  | _ ->
    gather [
      emit_nyi "lval expression";
      rhs_instrs;
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
    begin match unop_to_incdec_op op with
    | None -> emit_nyi "incdec"
    | Some incdec_op ->
      emit_lval_op (LValOp.IncDec incdec_op) e None
    end
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

and stash_in_local ?(leave_on_stack=false) e f =
  let break_label = Label.next_regular () in
  match e with
  | (_, A.Lvar (_, id)) ->
    gather [
      f (Local.Named id) break_label;
      instr_label break_label;
    ]
  | _ ->
    let temp = Local.get_unnamed_local () in
    let fault_label = Label.next_fault () in
    gather [
      from_expr e;
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

and from_switch scrutinee_expr cl =
  stash_in_local scrutinee_expr
  begin fun local break_label ->
  (* "continue" in a switch in PHP has the same semantics as break! *)
  let cl = List.map cl ~f:from_case in
  let bodies = gather @@ List.map cl ~f:snd in
  let init = gather @@ List.map cl
    ~f: begin fun x ->
          let (e_opt, l) = fst x in
          match e_opt with
          | None ->
            instr_jmp l
          | Some e ->
            (* Special case for simple scrutinee *)
            match scrutinee_expr with
            | (_, A.Lvar _) ->
              gather [from_expr e; instr_cgetl2 local; instr_eq; instr_jmpnz l]
            | _ ->
              gather [instr_cgetl local; from_expr e; instr_eq; instr_jmpnz l]
        end
  in
  let instrs = gather [
    init;
    bodies;
  ] in
  CBR.rewrite_in_switch instrs break_label
  end

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
