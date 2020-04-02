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
open Emit_expression
open Emit_pos
module A = Aast
module TC = Hhas_type_constraint
module SN = Naming_special_names
module TFR = Try_finally_rewriter
module JT = Jump_targets
module Opts = Hhbc_options

(* Context for code generation. It would be more elegant to pass this
 * around in an environment parameter. *)
let verify_return : Aast.hint option ref = ref None

let default_return_value = ref instr_null

let default_dropthrough = ref None

let verify_out = ref empty

let num_out = ref 0

let function_pos = ref Pos.none

let set_num_out c = num_out := c

let set_verify_return b = verify_return := b

let set_default_return_value i = default_return_value := i

let set_default_dropthrough i = default_dropthrough := i

let set_verify_out i = verify_out := i

let set_function_pos p = function_pos := p

let emit_return (env : Emit_env.t) =
  TFR.emit_return
    ~verify_return:!verify_return
    ~verify_out:!verify_out
    ~num_out:!num_out
    ~in_finally_epilogue:false
    env

let emit_markup env s ~check_for_hashbang =
  let emit_ignored_call_expr f e =
    let p = Pos.none in
    let call_expr =
      Tast_annotate.make
        (A.Call (Aast.Cnormal, Tast_annotate.make (A.Id (p, f)), [], [e], None))
    in
    emit_ignored_expr env call_expr
  in
  let emit_ignored_call_for_non_empty_string f s =
    if String.length s = 0 then
      empty
    else
      emit_ignored_call_expr f (Tast_annotate.make (A.String s))
  in
  let markup =
    if String.length s = 0 then
      empty
    else
      let tail =
        if check_for_hashbang then
          (* if markup text starts with #!
          - extract a line with hashbang - it will be emitted as a call
          to print_hashbang function
          - emit remaining part of text as regular markup *)
          let r = Str.regexp "^#!.*\n" in
          if Str.string_match r s 0 then
            let cmd = Str.matched_string s in
            String_utils.lstrip s cmd
          else
            s
        else
          s
      in
      emit_ignored_call_for_non_empty_string SN.SpecialFunctions.echo tail
  in
  markup

let get_level p op e =
  match A.get_break_continue_level e with
  | A.Level_ok (Some i) -> i
  | A.Level_ok None -> 1
  | A.Level_non_positive ->
    Emit_fatal.raise_fatal_parse
      p
      ("'" ^ op ^ "' operator accepts only positive numbers")
  | A.Level_non_literal ->
    Emit_fatal.raise_fatal_parse
      p
      ("'" ^ op ^ "' with non-constant operand is not supported")

let rec set_bytes_kind name =
  let re =
    Str.regexp_case_fold
      "^hh\\\\set_bytes\\(_rev\\|\\)_\\([a-z0-9]+\\)\\(_vec\\|\\)$"
  in
  if Str.string_match re name 0 then
    let op =
      if Str.matched_group 1 name = "_rev" then
        Reverse
      else
        Forward
    in
    let size = Str.matched_group 2 name in
    let is_vec = Str.matched_group 3 name = "_vec" in
    match (size, is_vec) with
    | ("string", false) -> Some (op, 1, true)
    | ("bool", _)
    | ("int8", _) ->
      Some (op, 1, is_vec)
    | ("int16", _) -> Some (op, 2, is_vec)
    | ("int32", _)
    | ("float32", _) ->
      Some (op, 4, is_vec)
    | ("int64", _)
    | ("float64", _) ->
      Some (op, 8, is_vec)
    | _ -> None
  else
    None

and emit_stmt env (pos, stmt) =
  match stmt with
  | A.Expr (_, A.Yield_break) -> gather [instr_null; emit_return env]
  | A.Expr (((pos, _), A.Call (_, (_, A.Id (_, s)), _, exprl, None)) as expr) ->
    let s = Hhbc_id.Function.(from_ast_name s |> to_raw_string) in
    if String.lowercase s = "unset" then
      gather (List.map exprl (emit_unset_expr env))
    else (
      match set_bytes_kind s with
      | Some kind ->
        let exprl =
          match exprl with
          | (_, A.Callconv (Ast_defs.Pinout, e)) :: t -> e :: t
          | _ -> exprl
        in
        emit_set_range_expr env pos s kind exprl
      | None -> emit_ignored_expr ~pop_pos:pos env expr
    )
  | A.Expr (ann, A.Await e) -> gather [emit_await env (fst ann) e; instr_popc]
  | A.Expr
      ( _,
        A.Binop
          ( Ast_defs.Eq None,
            ((_, A.List l) as e1),
            ((await_pos, _), A.Await e_await) ) ) ->
    let awaited_instrs = emit_await env await_pos e_await in
    let has_elements =
      List.exists l ~f:(function
          | (_, A.Omitted) -> false
          | _ -> true)
    in
    if has_elements then
      Scope.with_unnamed_local @@ fun temp ->
      (* before *)
      ( gather [awaited_instrs; instr_popl temp],
        (* inner *)
        of_pair @@ emit_lval_op_list env pos (Some temp) [] e1,
        (* after *)
        instr_unsetl temp )
    else
      gather [awaited_instrs; instr_popc]
  | A.Expr
      (_, A.Binop (Ast_defs.Eq None, e_lhs, ((await_pos, _), A.Await e_await)))
    ->
    emit_await_assignment env await_pos e_lhs e_await
  | A.Expr (_, A.Yield_from e) ->
    gather [emit_yield_from_delegates env pos e; emit_pos pos; instr_popc]
  | A.Expr ((pos, _), A.Binop (Ast_defs.Eq None, e_lhs, (_, A.Yield_from e))) ->
    Local.scope @@ fun () ->
    let temp = Local.get_unnamed_local () in
    let rhs_instrs = instr_pushl temp in
    gather
      [
        emit_yield_from_delegates env pos e;
        instr_setl temp;
        instr_popc;
        emit_lval_op_nonlist env pos LValOp.Set e_lhs rhs_instrs 1;
        instr_popc;
      ]
  | A.Expr expr -> emit_ignored_expr ~pop_pos:pos env expr
  | A.Return (Some ((inner_pos, _), A.Await e)) ->
    gather [emit_await env inner_pos e; Emit_pos.emit_pos pos; emit_return env]
  | A.Return (Some (_, A.Yield_from e)) ->
    gather
      [
        emit_yield_from_delegates env pos e;
        Emit_pos.emit_pos pos;
        emit_return env;
      ]
  | A.Return None -> gather [instr_null; Emit_pos.emit_pos pos; emit_return env]
  | A.Return (Some expr) ->
    gather [emit_expr env expr; Emit_pos.emit_pos pos; emit_return env]
  | A.GotoLabel (_, label) -> instr_label (Label.named label)
  | A.Goto (_, label) -> TFR.emit_goto ~in_finally_epilogue:false env label
  | A.Block b -> emit_stmts env b
  | A.If (condition, consequence, alternative) ->
    emit_if env pos condition consequence alternative
  | A.While (e, b) -> emit_while env e b
  | A.Using
      A.
        {
          us_has_await = has_await;
          us_expr = e;
          us_block = b;
          us_is_block_scoped = is_block_scoped;
        } ->
    emit_using env pos is_block_scoped has_await e (block_pos b) b
  | A.Break -> emit_break env pos
  | A.Continue -> emit_continue env pos
  | A.Do (b, e) -> emit_do env b e
  | A.For (e1, e2, e3, b) -> emit_for env pos e1 e2 e3 b
  | A.Throw ((_, _) as expr) ->
    gather [emit_expr env expr; Emit_pos.emit_pos pos; instr (IContFlow Throw)]
  | A.Try (try_block, catch_list, finally_block) ->
    if JT.get_function_has_goto () then
      TFR.fail_if_goto_from_try_to_finally try_block finally_block;

    (* TFR.fail_if_goto_from_try_to_finally try_block finally_block;*)
    if catch_list <> [] && finally_block <> [] then
      emit_stmt
        env
        ( pos,
          A.Try ([(pos, A.Try (try_block, catch_list, []))], [], finally_block)
        )
    else if catch_list <> [] then
      emit_try_catch env pos try_block catch_list
    else
      emit_try_finally env pos try_block finally_block
  | A.Switch (e, cl) -> emit_switch env pos e cl
  | A.Foreach (collection, iterator, block) ->
    emit_foreach env pos collection iterator block
  | A.Awaitall (el, b) -> emit_awaitall env pos el b
  | A.Markup (_, s) -> emit_markup env s ~check_for_hashbang:false
  | A.Fallthrough
  | A.Noop ->
    empty

and emit_break env pos =
  TFR.emit_break_or_continue ~is_break:true ~in_finally_epilogue:false env pos 1

and emit_continue env pos =
  TFR.emit_break_or_continue
    ~is_break:false
    ~in_finally_epilogue:false
    env
    pos
    1

and emit_temp_break env pos level =
  TFR.emit_break_or_continue
    ~is_break:true
    ~in_finally_epilogue:false
    env
    pos
    level

and emit_temp_continue env pos level =
  TFR.emit_break_or_continue
    ~is_break:false
    ~in_finally_epilogue:false
    env
    pos
    level

and get_instrs r = r.Emit_expression.instrs

and emit_if env pos condition consequence alternative =
  match alternative with
  | []
  | [(_, A.Noop)] ->
    let done_label = Label.next_regular () in
    gather
      [
        get_instrs @@ emit_jmpz env condition done_label;
        emit_stmts env consequence;
        instr_label done_label;
      ]
  | _ ->
    let alternative_label = Label.next_regular () in
    let done_label = Label.next_regular () in
    let consequence_instr = emit_stmts env consequence in
    let alternative_instr = emit_stmts env alternative in
    gather
      [
        get_instrs @@ emit_jmpz env condition alternative_label;
        consequence_instr;
        emit_pos pos;
        instr_jmp done_label;
        instr_label alternative_label;
        alternative_instr;
        instr_label done_label;
      ]

and emit_await_assignment env pos lval e =
  match snd lval with
  | A.Lvar id when not (is_local_this env (snd id)) ->
    gather
      [
        emit_await env pos e;
        emit_pos pos;
        instr_popl (get_local env (pos, Local_id.get_name (snd id)));
      ]
  | _ ->
    let awaited_instrs = emit_await env pos e in
    Scope.with_unnamed_local @@ fun temp ->
    let rhs_instrs = instr_pushl temp in
    let (lhs, rhs, setop) =
      emit_lval_op_nonlist_steps env pos LValOp.Set lval rhs_instrs 1
    in
    (* before *)
    ( gather [awaited_instrs; instr_popl temp],
      (* inner *)
      lhs,
      (* after *)
      gather [rhs; setop; instr_popc] )

and emit_awaitall env pos el b =
  match el with
  | [] -> empty
  | [(lvar, e)] -> emit_awaitall_single env pos lvar e b
  | _ -> emit_awaitall_multi env el b

and emit_awaitall_single env pos lval e b =
  Scope.with_unnamed_locals @@ fun () ->
  let load_arg = emit_await env pos e in
  let (load, unset) =
    match lval with
    | Some (_, str) ->
      let l = Local.init_unnamed_local_for_tempname (Local_id.get_name str) in
      (instr_popl l, instr_unsetl l)
    | None -> (instr_popc, empty)
  in
  (* before *)
  (gather [load_arg; load], (* inner *)
                            emit_stmts env b, (* after *)
                                              unset)

and emit_awaitall_multi env el b =
  Scope.with_unnamed_locals @@ fun () ->
  let load_args =
    gather @@ List.map el ~f:(fun (_, arg) -> emit_expr env arg)
  in
  let locals =
    List.map el ~f:(fun (lvar, _) ->
        match lvar with
        | None -> Local.get_unnamed_local ()
        | Some (_, str) ->
          Local.init_unnamed_local_for_tempname (Local_id.get_name str))
  in
  let init_locals = gather @@ List.rev_map locals ~f:instr_popl in
  let await_all = gather [instr_awaitall_list locals; instr_popc] in
  let unpack =
    gather
    @@ List.map locals ~f:(fun l ->
           let label_done = Label.next_regular () in
           gather
             [
               instr_pushl l;
               instr_dup;
               instr_istypec OpNull;
               instr_jmpnz label_done;
               instr_whresult;
               instr_label label_done;
               instr_popl l;
             ])
  in
  let block = emit_stmts env b in
  let unset_locals = gather @@ List.map locals ~f:instr_unsetl in
  (* before *)
  ( gather [load_args; init_locals],
    (* inner *)
    gather [await_all; unpack; block],
    (* after *)
    unset_locals )

and emit_while env e b =
  let break_label = Label.next_regular () in
  let cont_label = Label.next_regular () in
  let start_label = Label.next_regular () in
  (* TODO: This is *bizarre* codegen for a while loop.
  It would be better to generate this as
  instr_label continue_label;
  emit_expr e;
  instr_jmpz break_label;
  body;
  instr_jmp continue_label;
  instr_label break_label;
  *)
  gather
    [
      get_instrs @@ emit_jmpz env e break_label;
      instr_label start_label;
      Emit_env.do_in_loop_body break_label cont_label env b emit_stmts;
      instr_label cont_label;
      get_instrs @@ emit_jmpnz env (fst e) (snd e) start_label;
      instr_label break_label;
    ]

and emit_using
    (env : Emit_env.t)
    pos
    is_block_scoped
    has_await
    (e : Tast.expr)
    block_pos
    (b : Tast.block) =
  match snd e with
  | A.Expr_list es ->
    emit_stmts env
    @@ List.fold_right
         es
         ~f:(fun e acc ->
           let ((p, _), _) = e in
           [
             ( p,
               A.Using
                 {
                   A.us_has_await = has_await;
                   A.us_is_block_scoped = is_block_scoped;
                   A.us_expr = e;
                   A.us_block = acc;
                 } );
           ])
         ~init:b
  | _ ->
    Local.scope @@ fun () ->
    let (local, preamble) =
      match snd e with
      | A.Binop (Ast_defs.Eq None, (_, A.Lvar (_, id)), _)
      | A.Lvar (_, id) ->
        ( Local.Named (Local_id.get_name id),
          gather [emit_expr env e; Emit_pos.emit_pos block_pos; instr_popc] )
      | _ ->
        let l = Local.get_unnamed_local () in
        (l, gather [emit_expr env e; instr_setl l; instr_popc])
    in
    let finally_start = Label.next_regular () in
    let finally_end = Label.next_regular () in
    let body = Emit_env.do_in_using_body finally_start env b emit_stmts in
    let jump_instructions = TFR.collect_jump_instructions body env in
    let body =
      if IMap.is_empty jump_instructions then
        body
      else
        TFR.cleanup_try_body body
    in
    let fn_name =
      Hhbc_id.Method.from_raw_string
      @@
      if has_await then
        "__disposeAsync"
      else
        "__dispose"
    in
    let emit_finally () =
      let (epilogue, async_eager_label) =
        if has_await then
          let after_await = Label.next_regular () in
          ( gather [instr_await; instr_label after_await; instr_popc],
            Some after_await )
        else
          (instr_popc, None)
      in
      gather
        [
          instr_cgetl local;
          instr_nulluninit;
          instr_nulluninit;
          instr_fcallobjmethodd
            (make_fcall_args
               ?async_eager_label
               ?context:(Emit_env.get_call_context env)
               0)
            fn_name
            Obj_null_throws;
          epilogue;
          ( if is_block_scoped then
            instr_unsetl local
          else
            empty );
        ]
    in
    let finally_epilogue =
      TFR.emit_finally_epilogue
        env
        pos
        ~verify_return:!verify_return
        ~verify_out:!verify_out
        ~num_out:!num_out
        jump_instructions
        finally_end
    in
    let exn_local = Local.get_unnamed_local () in
    let middle =
      if is_empty_block b then
        empty
      else
        create_try_catch
          ~skip_throw:true
          body
          (gather
             [
               emit_pos block_pos;
               make_finally_catch exn_local (emit_finally ());
               emit_pos pos;
             ])
    in
    gather
      [
        preamble;
        middle;
        instr_label finally_start;
        emit_finally ();
        finally_epilogue;
        instr_label finally_end;
      ]

and emit_do env b e =
  let cont_label = Label.next_regular () in
  let break_label = Label.next_regular () in
  let start_label = Label.next_regular () in
  gather
    [
      instr_label start_label;
      Emit_env.do_in_loop_body break_label cont_label env b emit_stmts;
      instr_label cont_label;
      get_instrs @@ emit_jmpnz env (fst e) (snd e) start_label;
      instr_label break_label;
    ]

and emit_for (env : Emit_env.t) p (e1 : Tast.expr) e2 e3 b =
  let break_label = Label.next_regular () in
  let cont_label = Label.next_regular () in
  let start_label = Label.next_regular () in
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
  let emit_cond ~jmpz label =
    let final cond =
      get_instrs
        ( if jmpz then
          emit_jmpz env cond label
        else
          emit_jmpnz env (fst cond) (snd cond) label )
    in
    let rec expr_list h tl =
      match tl with
      | [] ->
        [
          final
          @@ ( ( Pos.none,
                 Typing_defs.mk (Typing_reason.none, Typing_defs.make_tany ())
               ),
               A.Expr_list [h] );
        ]
      | h1 :: t1 -> emit_ignored_expr env ~pop_pos:p h :: expr_list h1 t1
    in
    match e2 with
    | (_, A.Expr_list []) ->
      if jmpz then
        empty
      else
        instr_jmp label
    | (_, A.Expr_list (h :: t)) -> gather @@ expr_list h t
    | cond -> final cond
  in
  gather
    [
      emit_ignored_expr env e1;
      emit_cond ~jmpz:true break_label;
      instr_label start_label;
      Emit_env.do_in_loop_body break_label cont_label env b emit_stmts;
      instr_label cont_label;
      emit_ignored_expr env e3;
      emit_cond ~jmpz:false start_label;
      instr_label break_label;
    ]

and emit_switch (env : Emit_env.t) pos scrutinee_expr cl =
  let (instr_init, instr_free, emit_check_case) =
    match snd scrutinee_expr with
    | A.Lvar _ ->
      (* Special case for simple scrutinee *)
      ( empty,
        empty,
        fun case_expr case_handler_label ->
          let ((pos, _), _) = case_expr in
          gather
            [
              emit_two_exprs env pos scrutinee_expr case_expr;
              instr_eq;
              instr_jmpnz case_handler_label;
            ] )
    | _ ->
      ( emit_expr env scrutinee_expr,
        instr_popc,
        fun case_expr case_handler_label ->
          let next_case_label = Label.next_regular () in
          let ((pos, _), _) = case_expr in
          gather
            [
              instr_dup;
              emit_expr env case_expr;
              emit_pos pos;
              instr_eq;
              instr_jmpz next_case_label;
              instr_popc;
              instr_jmp case_handler_label;
              instr_label next_case_label;
            ] )
  in
  (* "continue" in a switch in PHP has the same semantics as break! *)
  let break_label = Label.next_regular () in
  let has_default =
    List.exists
      ~f:(function
        | A.Default _ -> true
        | _ -> false)
      cl
  in
  let (cl, emit_exhaustiveness) =
    if has_default then
      (cl, false)
    else
      let rec aux cl =
        match cl with
        | [] ->
          failwith "impossible - switch statements must have at least one case"
        | [A.Default _] -> failwith "impossible - there shouldn't be a default"
        | [A.Case (e, b)] -> [A.Case (e, b @ [(Pos.none, A.Break)])]
        | c :: cl -> c :: aux cl
      in
      (* Add a default so that we can emit the warning/exception *)
      (aux cl @ [A.Default (pos, [])], true)
  in
  let cl =
    Emit_env.do_in_switch_body break_label env cl @@ fun env _ ->
    List.map cl ~f:(emit_case ~emit_exhaustiveness env)
  in
  let instr_bodies = gather @@ List.map cl ~f:snd in
  let default_label =
    let default_labels =
      List.filter_map cl ~f:(fun ((e, l), _) ->
          if Option.is_none e then
            Some l
          else
            None)
    in
    match default_labels with
    | [] -> break_label
    | [l] -> l
    | _ ->
      Emit_fatal.raise_fatal_runtime
        pos
        "Switch statements may only contain one 'default' clause."
  in
  let instr_check_cases =
    gather
    @@ List.map cl ~f:(function
           (* jmp to default case should be emitted as the very last 'else' case *)
           | ((None, _), _) -> empty
           | ((Some e, l), _) -> emit_check_case e l)
  in
  gather
    [
      instr_init;
      instr_check_cases;
      instr_free;
      instr_jmp default_label;
      instr_bodies;
      instr_label break_label;
    ]

and block_pos b =
  let bpos = List.map b fst in
  let valid_pos = List.filter bpos (fun e -> e <> Pos.none) in
  if valid_pos = [] then
    Pos.none
  else
    Pos.btw (List.hd_exn valid_pos) (List.last_exn valid_pos)

and emit_catch
    (env : Emit_env.t) pos end_label ((_, catch_type), (_, catch_local), b) =
  (* Note that this is a "regular" label; we're not going to branch to
    it directly in the event of an exception. *)
  let next_catch = Label.next_regular () in
  let id = Hhbc_id.Class.from_ast_name catch_type in
  gather
    [
      instr_dup;
      instr_instanceofd id;
      instr_jmpz next_catch;
      instr_setl (Local.Named (Local_id.get_name catch_local));
      instr_popc;
      emit_stmt env (Pos.none, A.Block b);
      Emit_pos.emit_pos pos;
      instr_jmp end_label;
      instr_label next_catch;
    ]

and emit_catches (env : Emit_env.t) pos catch_list end_label =
  gather (List.map catch_list ~f:(emit_catch env pos end_label))

and is_empty_block b =
  List.for_all
    ~f:(function
      | (_, A.Noop) -> true
      | _ -> false)
    b

and emit_try_catch (env : Emit_env.t) pos try_block catch_list =
  Local.scope @@ fun () -> emit_try_catch_ env pos try_block catch_list

and emit_try_catch_ (env : Emit_env.t) pos try_block catch_list =
  if is_empty_block try_block then
    empty
  else
    let end_label = Label.next_regular () in
    let try_env = Emit_env.with_try env in
    create_try_catch
      ~opt_done_label:end_label
      (gather [emit_stmts try_env try_block; Emit_pos.emit_pos pos])
      (emit_catches env pos catch_list end_label)

and emit_try_finally env pos try_block finally_block =
  Local.scope @@ fun () -> emit_try_finally_ env pos try_block finally_block

and emit_try_finally_ env pos try_block finally_block =
  let make_finally_body () =
    Emit_env.do_in_finally_body env finally_block emit_stmts
  in
  if is_empty_block try_block then
    make_finally_body ()
  else
    (*
  We need to generate four things:
  (1) the try-body, which will be followed by
  (2) the normal-continuation finally body, and
  (3) an epilogue to the finally body that deals with finally-blocked
      break and continue
  (4) the exceptional-continuation catch body.
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
    let finally_start = Label.next_regular () in
    let finally_end = Label.next_regular () in
    let enclosing_span = Ast_scope.Scope.get_span env.Emit_env.env_scope in
    let try_env = Emit_env.with_try env in
    let try_body =
      Emit_env.do_in_try_body finally_start try_env try_block emit_stmts
    in
    let jump_instructions = TFR.collect_jump_instructions try_body env in
    let try_body =
      if IMap.is_empty jump_instructions then
        try_body
      else
        TFR.cleanup_try_body try_body
    in

    (* (2) Finally body

  Note that this is used both in the normal-continuation and
  exceptional-continuation cases; we generate the same code twice.

  TODO: We might consider changing the codegen so that the finally block
  is only generated once. We could do this by making the catch block set a
  temp local to -1, and then branch to the finally block. In the finally block
  epilogue it can check to see if the local is -1, and if so, issue an unwind
  instruction.

  It is illegal to have a continue or break which branches out of a finally.
  Unfortunately we at present do not detect this at parse time; rather, we
  generate an exception at run-time by rewriting continue and break
  instructions found inside finally blocks.

  TODO: If we make this illegal at parse time then we can remove this pass.
  *)
    let exn_local = Local.get_unnamed_local () in
    let finally_body = make_finally_body () in
    let finally_body_for_catch =
      finally_body |> Label_rewriter.clone_with_fresh_regular_labels
    in
    (* (3) Finally epilogue *)
    let finally_epilogue =
      TFR.emit_finally_epilogue
        env
        pos
        ~verify_return:!verify_return
        ~verify_out:!verify_out
        ~num_out:!num_out
        jump_instructions
        finally_end
    in

    (* (4) Catch body

  We now emit the catch body; it is just cleanup code for the temp_local,
  a copy of the finally body (without the branching epilogue, since we are
  going to unwind rather than branch), and an unwind instruction.

  TODO: The HHVM emitter sometimes emits seemingly spurious
  unset-unnamed-local instructions into the catch block.  These look
  like bugs in the emitter. Investigate; if they are bugs in the HHVM
  emitter, get them fixed there. If not, get a clear explanation of
  what they are for and why they are required.
  *)
    let middle =
      create_try_catch
        ~skip_throw:true
        try_body
        (gather
           [
             emit_pos enclosing_span;
             make_finally_catch exn_local finally_body_for_catch;
           ])
    in
    (* Put it all together. *)
    gather
      [
        middle;
        instr_label finally_start;
        Emit_pos.emit_pos pos;
        finally_body;
        finally_epilogue;
        instr_label finally_end;
      ]

and make_finally_catch exn_local finally_body =
  gather
    [
      instr_popl exn_local;
      instr_unsetl (Local.get_label_id_local ());
      instr_unsetl (Local.get_retval_local ());
      create_try_catch
        finally_body
        (gather [instr_pushl exn_local; instr_chain_faults]);
      instr_pushl exn_local;
      instr_throw;
    ]

and get_id_of_simple_lvar_opt v =
  match v with
  | A.Lvar (pos, id) when Local_id.get_name id = SN.SpecialIdents.this ->
    Emit_fatal.raise_fatal_parse pos "Cannot re-assign $this"
  | A.Lvar (_, id)
    when not
           ( SN.Superglobals.is_superglobal (Local_id.get_name id)
           || Local_id.get_name id = SN.Superglobals.globals ) ->
    Some (Local_id.get_name id)
  | _ -> None

and emit_load_list_elements env path vs =
  let (preamble, load_value) =
    List.mapi ~f:(emit_load_list_element env path) vs |> List.unzip
  in
  (List.concat preamble, List.concat load_value)

and emit_load_list_element env path i v =
  let query_value =
    gather
      [
        gather @@ List.rev path;
        instr_querym 0 QueryOp.CGet (MemberKey.EI (Int64.of_int i));
      ]
  in
  match v with
  | (_, A.Lvar (_, id)) ->
    let load_value =
      gather
        [
          query_value;
          instr_setl (Local.Named (Local_id.get_name id));
          instr_popc;
        ]
    in
    ([], [load_value])
  | (_, A.List exprs) ->
    let dim_instr =
      instr_dim MemberOpMode.Warn (MemberKey.EI (Int64.of_int i))
    in
    emit_load_list_elements env (dim_instr :: path) exprs
  | ((pos, _), _) ->
    let set_instrs = emit_lval_op_nonlist env pos LValOp.Set v query_value 1 in
    let load_value = [set_instrs; instr_popc] in
    ([], [gather load_value])

(* Assigns a location to store values for foreach-key and foreach-value and
   creates a code to populate them.
   NOT suitable for foreach (... await ...) which uses different code-gen
   Returns: key_local_opt * value_local * key_preamble * value_preamble
   where:
   - key_local_opt - local variable to store a foreach-key value if it is
     declared
   - value_local - local variable to store a foreach-value
   - key_preamble - list of instructions to populate foreach-key
   - value_preamble - list of instructions to populate foreach-value
   *)
and emit_iterator_key_value_storage env iterator :
    Hhbc_ast.local_id option * Hhbc_ast.local_id * Instruction_sequence.t =
  match iterator with
  | A.As_kv (((_, k) as expr_k), ((_, v) as expr_v)) ->
    begin
      match (get_id_of_simple_lvar_opt k, get_id_of_simple_lvar_opt v) with
      | (Some key_id, Some value_id) ->
        let key_local = Local.Named key_id in
        let value_local = Local.Named value_id in
        (Some key_local, value_local, empty)
      | _ ->
        let key_local = Local.get_unnamed_local () in
        let value_local = Local.get_unnamed_local () in
        let (key_preamble, key_load) =
          emit_iterator_lvalue_storage env expr_k key_local
        in
        let (value_preamble, value_load) =
          emit_iterator_lvalue_storage env expr_v value_local
        in
        (* HHVM prepends code to initialize non-plain, non-list foreach-key
         to the value preamble - do the same to minimize diffs *)
        let (key_preamble, value_preamble) =
          match k with
          | A.List _ -> (key_preamble, value_preamble)
          | _ -> ([], gather key_preamble :: value_preamble)
        in
        ( Some key_local,
          value_local,
          gather
            [
              gather value_preamble;
              gather value_load;
              gather key_preamble;
              gather key_load;
            ] )
    end
  | A.As_v ((_, v) as expr_v) ->
    begin
      match get_id_of_simple_lvar_opt v with
      | Some value_id ->
        let value_local = Local.Named value_id in
        (None, value_local, empty)
      | None ->
        let value_local = Local.get_unnamed_local () in
        let (value_preamble, value_load) =
          emit_iterator_lvalue_storage env expr_v value_local
        in
        (None, value_local, gather [gather value_preamble; gather value_load])
    end
  | _ -> failwith "emit_iterator_key_value_storage with iterator using await"

(* Emit code for either the key or value l-value operation in foreach await.
 * `indices` is the initial prefix of the array indices ([0] for key or [1] for
 * value) that is prepended onto the indices needed for list destructuring
 *
 * TODO: we don't need unnamed local if the target is a local
 *)
and emit_foreach_await_lvalue_storage
    (env : Emit_env.t) (expr1 : Tast.expr) indices keep_on_stack =
  let ((pos, _), _) = expr1 in
  Scope.with_unnamed_local @@ fun local ->
  (* before *)
  ( instr_popl local,
    (* inner *)
    of_pair @@ emit_lval_op_list env pos (Some local) indices expr1,
    (* after *)
    if keep_on_stack then
      instr_pushl local
    else
      instr_unsetl local )

(* Emit code for the value and possibly key l-value operation in a foreach
 * await statement. The result of invocation of the `next` method has been
 * stored on top of the stack. For example:
 *   foreach (foo() await as $a->f => list($b[0], $c->g)) { ... }
 * Here, we need to construct l-value operations that access the [0] (for $a->f)
 * and [1;0] (for $b[0]) and [1;1] (for $c->g) indices of the array returned
 * from the `next` method.
 *)
and emit_foreach_await_key_value_storage (env : Emit_env.t) iterator =
  match iterator with
  | A.Await_as_kv (_, expr_k, expr_v)
  | A.As_kv (expr_k, expr_v) ->
    let key_instrs = emit_foreach_await_lvalue_storage env expr_k [0] true in
    let value_instrs = emit_foreach_await_lvalue_storage env expr_v [1] false in
    gather [key_instrs; value_instrs]
  | A.Await_as_v (_, expr_v)
  | A.As_v expr_v ->
    emit_foreach_await_lvalue_storage env expr_v [1] false

(*Generates a code to initialize a given foreach-* value.
  Returns: preamble * load_code
  where:
  - preamble - preparation part that should be executed before the loading
  - load_code - instructions to actually populate the value.
  This split is necessary to reflect the way how HHVM loads values.
  as an example for the code
    list($$$a, $$b, $$$c)
  preamble part will include code that pushes cells for $$a and $$c on the stack
  load_code will be executed assuming that stack is prepopulated:
    [$aa, $$c] <- top
  *)
and emit_iterator_lvalue_storage env v local =
  match v with
  | ((pos, _), A.Call _) ->
    Emit_fatal.raise_fatal_parse pos "Can't use return value in write context"
  | (_, A.List exprs) ->
    let (preamble, load_values) =
      emit_load_list_elements env [instr_basel local MemberOpMode.Warn] exprs
    in
    let load_values = [gather @@ List.rev load_values; instr_unsetl local] in
    (preamble, load_values)
  | ((pos, _), _) ->
    let (lhs, rhs, set_op) =
      emit_lval_op_nonlist_steps env pos LValOp.Set v (instr_cgetl local) 1
    in
    ([lhs], [rhs; set_op; instr_popc; instr_unsetl local])

and emit_foreach env pos collection iterator block =
  Local.scope @@ fun () ->
  match iterator with
  | A.As_kv _
  | A.As_v _ ->
    emit_foreach_ env pos collection iterator block
  | A.Await_as_kv (pos, _, _)
  | A.Await_as_v (pos, _) ->
    emit_foreach_await env pos collection iterator block

and emit_foreach_await env pos collection iterator block =
  let instr_collection = emit_expr env collection in
  Scope.with_unnamed_local @@ fun iter_temp_local ->
  let input_is_async_iterator_label = Label.next_regular () in
  let next_label = Label.next_regular () in
  let exit_label = Label.next_regular () in
  let pop_and_exit_label = Label.next_regular () in
  let async_eager_label = Label.next_regular () in
  let next_meth = Hhbc_id.Method.from_raw_string "next" in
  (* before *)
  ( gather
      [
        instr_collection;
        instr_dup;
        instr_instanceofd (Hhbc_id.Class.from_raw_string "HH\\AsyncIterator");
        instr_jmpnz input_is_async_iterator_label;
        Emit_fatal.emit_fatal_runtime
          pos
          "Unable to iterate non-AsyncIterator asynchronously";
        instr_label input_is_async_iterator_label;
        instr_popl iter_temp_local;
      ],
    (* inner *)
    gather
      [
        instr_label next_label;
        instr_cgetl iter_temp_local;
        instr_nulluninit;
        instr_nulluninit;
        instr_fcallobjmethodd
          (make_fcall_args ~async_eager_label 0)
          next_meth
          Obj_null_throws;
        instr_await;
        instr_label async_eager_label;
        instr_dup;
        instr_istypec OpNull;
        instr_jmpnz pop_and_exit_label;
        emit_foreach_await_key_value_storage env iterator;
        Emit_env.do_in_loop_body exit_label next_label env block emit_stmts;
        emit_pos pos;
        instr_jmp next_label;
        instr_label pop_and_exit_label;
        instr_popc;
        instr_label exit_label;
      ],
    (* after *)
    instr_unsetl iter_temp_local )

and emit_foreach_ env pos collection iterator block =
  let instr_collection = emit_expr env collection in
  Scope.with_unnamed_locals_and_iterators @@ fun () ->
  let iter_id = Iterator.get_iterator () in
  let loop_break_label = Label.next_regular () in
  let loop_continue_label = Label.next_regular () in
  let loop_head_label = Label.next_regular () in
  let (key_id, val_id, preamble) =
    emit_iterator_key_value_storage env iterator
  in
  let iter_args = { iter_id; key_id; val_id } in
  let init = instr_iterinit iter_args loop_break_label in
  let next = instr_iternext iter_args loop_head_label in
  let body =
    Emit_env.do_in_loop_body
      loop_break_label
      loop_continue_label
      env
      ~iter:iter_id
      block
      emit_stmts
  in
  ( gather [instr_collection; emit_pos (Tast_annotate.get_pos collection); init],
    gather
      [
        instr_label loop_head_label;
        preamble;
        body;
        instr_label loop_continue_label;
        emit_pos pos;
        next;
      ],
    gather [instr_label loop_break_label] )

and emit_yield_from_delegates env pos e =
  let iterator_number = Iterator.get_iterator () in
  let loop_label = Label.next_regular () in
  gather
    [
      emit_expr env e;
      emit_pos pos;
      instr_contAssignDelegate iterator_number;
      create_try_catch
        (gather
           [
             instr_null;
             instr_label loop_label;
             instr_contEnterDelegate;
             instr_yieldFromDelegate iterator_number loop_label;
           ])
        (instr_contUnsetDelegate_free iterator_number);
      instr_contUnsetDelegate_ignore iterator_number;
    ]

and emit_stmts env stl =
  let results = List.map stl (emit_stmt env) in
  gather results

and emit_case ~emit_exhaustiveness (env : Emit_env.t) c =
  let l = Label.next_regular () in
  let (b, e) =
    match c with
    | A.Default (pos, []) when emit_exhaustiveness ->
      (Emit_pos.emit_pos_then pos instr_throw_non_exhaustive_switch, None)
    | _ ->
      let (b, e) =
        match c with
        | A.Default (_, b) -> (b, None)
        | A.Case (e, b) -> (b, Some e)
      in
      (emit_stmt env (Pos.none, A.Block b), e)
  in
  ((e, l), gather [instr_label l; b])

let emit_dropthrough_return env =
  match !default_dropthrough with
  | Some instrs -> instrs
  | _ ->
    Emit_pos.emit_pos_then (Pos.last_char !function_pos)
    @@ gather [!default_return_value; emit_return env]

let rec emit_final_statement env s =
  match snd s with
  | A.Throw _
  | A.Return _
  | A.Goto _
  | A.Expr (_, A.Yield_break) ->
    emit_stmt env s
  | A.Block b -> emit_final_statements env b
  | _ -> gather [emit_stmt env s; emit_dropthrough_return env]

and emit_final_statements env b =
  match b with
  | [] -> emit_dropthrough_return env
  | [s] -> emit_final_statement env s
  | s :: b ->
    let i1 = emit_stmt env s in
    let i2 = emit_final_statements env b in
    gather [i1; i2]
