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
open Emit_expression
open Emit_pos

module A = Ast
module H = Hhbc_ast
module TC = Hhas_type_constraint
module SN = Naming_special_names
module TFR = Try_finally_rewriter
module JT = Jump_targets
module Opts = Hhbc_options

(* Context for code generation. It would be more elegant to pass this
 * around in an environment parameter. *)
let verify_return = ref false
let default_return_value = ref instr_null
let default_dropthrough = ref None
let return_by_ref = ref false
let verify_out = ref empty
let num_out = ref 0
let function_pos = ref Pos.none
let set_num_out c = num_out := c
let set_verify_return b = verify_return := b
let set_default_return_value i = default_return_value := i
let set_default_dropthrough i = default_dropthrough := i
let set_return_by_ref b = return_by_ref := b
let set_verify_out i = verify_out := i
let set_function_pos p = function_pos := p

let emit_return ~need_ref env =
  TFR.emit_return
    ~need_ref
    ~verify_return:!verify_return
    ~verify_out:!verify_out
    ~num_out:!num_out
    ~in_finally_epilogue:false
    env

let emit_def_inline = function
  | A.Fun fd ->
    let has_inout_params =
      let r, _ =
        Emit_inout_helpers.extract_function_inout_or_ref_param_locations fd in
      Option.is_some r in
    Emit_pos.emit_pos_then (fd.Ast.f_span) @@
    let n = int_of_string (snd fd.Ast.f_name) in
    gather [
      instr_deffunc n;
      (if not has_inout_params then empty else instr_deffunc (n + 1))
    ]
  | A.Class cd ->
    let defcls_fn =
      if Emit_env.is_systemlib () then instr_defclsnop else instr_defcls in
    Emit_pos.emit_pos_then (fst cd.Ast.c_name) @@
    defcls_fn (int_of_string (snd cd.Ast.c_name))
  | A.Typedef td ->
    Emit_pos.emit_pos_then (fst td.Ast.t_id) @@
    instr_deftypealias (int_of_string (snd td.Ast.t_id))
  | _ ->
    failwith "Define inline: Invalid inline definition"

let emit_markup env s echo_expr_opt ~check_for_hashbang =
  let emit_ignored_call_expr f e =
    let p = Pos.none in
    let call_expr = p, A.Call ((p, A.Id (p, f)), [], [e], []) in
    emit_ignored_expr env call_expr
  in
  let emit_ignored_call_for_non_empty_string f s =
    if String.length s = 0 then empty
    else emit_ignored_call_expr f (Pos.none, A.String s)
  in
  let markup =
    if String.length s = 0
    then empty
    else
      let hashbang, tail =
        if check_for_hashbang
        then
          (* if markup text starts with #!
          - extract a line with hashbang - it will be emitted as a call
          to print_hashbang function
          - emit remaining part of text as regular markup *)
          let r = Str.regexp "^#!.*\n" in
          if Str.string_match r s 0
          then
            let cmd = Str.matched_string s in
            let tail = String_utils.lstrip s cmd in
            cmd, tail
          else "", s
        else "", s
      in
      gather [
        emit_ignored_call_for_non_empty_string
          "__SystemLib\\print_hashbang" hashbang;
        emit_ignored_call_for_non_empty_string SN.SpecialFunctions.echo tail
      ]
  in
  let echo =
    match echo_expr_opt with
    | Some e -> emit_ignored_call_expr SN.SpecialFunctions.echo e
    | None -> empty
  in
  gather [
    markup;
    echo
  ]

let get_level p op e =
  match Ast_utils.get_break_continue_level e with
  | Ast_utils.Level_ok (Some i) -> i
  | Ast_utils.Level_ok None -> 1
  | Ast_utils.Level_non_positive ->
    Emit_fatal.raise_fatal_parse
      p ("'" ^ op ^ "' operator accepts only positive numbers")
  | Ast_utils.Level_non_literal ->
    Emit_fatal.raise_fatal_parse
      p ("'" ^ op ^ "' with non-constant operand is not supported")

let rec emit_stmt env (pos, st_) =
  match st_ with
  | A.Let _ -> assert false (* Let statement is converted to assignment in closure convert *)
  | A.Expr (_, A.Yield_break) ->
    gather [
      instr_null;
      emit_return ~need_ref:false env;
    ]
  | A.Expr (_, A.Call ((_, A.Id (_, s)), _, exprl, []))
    when String.lowercase_ascii s = "unset" ->
    gather (List.map exprl (emit_unset_expr env))
  | A.Return (Some (inner_pos, A.Await e)) ->
    gather [
      emit_await env inner_pos e;
      Emit_pos.emit_pos pos;
      emit_return ~need_ref:false env;
    ]
  | A.Return (Some (_, A.Yield_from e)) ->
    gather [
      emit_yield_from_delegates env pos e;
      Emit_pos.emit_pos pos;
      emit_return ~need_ref:false env;
    ]
  | A.Expr (pos, A.Await e) ->
    begin match try_inline_genva_call env e GI_ignore_result with
    | Some r -> r
    | None ->
    gather [
      emit_await env pos e;
      instr_popc;
    ]
    end
  | A.Expr
    (_, A.Binop ((A.Eq None), ((_, A.List l) as e1), (await_pos, A.Await e_await))) ->
    begin match try_inline_genva_call env e_await (GI_list_assignment l) with
    | Some r -> r
    | None ->
    let has_elements =
      List.exists l ~f: (function
        | _, A.Omitted -> false
        | _ -> true)
    in
    if has_elements then
      Local.scope @@ fun () ->
        let awaited = emit_await env await_pos e_await in
        let temp = Local.get_unnamed_local () in
        gather [
          awaited;
          instr_setl temp;
          instr_popc;
          with_temp_local temp
          begin fun temp _ ->
            let prefix, block =
              emit_lval_op_list env pos (Some temp) [] e1 in
              gather [
                prefix;
                block
              ]
          end;
          instr_pushl temp;
          instr_popc;
        ]
    else
      Local.scope @@ fun () ->
        let temp = Local.get_unnamed_local () in
        gather [
          emit_await env await_pos e_await;
          instr_setl temp;
          instr_popc;
          instr_pushl temp;
          instr_popc;
        ]
    end
  | A.Expr (_, A.Binop (A.Eq None, e_lhs, (await_pos, A.Await e_await))) ->
    let result = Local.scope @@ fun () -> emit_await env await_pos e_await in
    Local.scope @@ fun () ->
      let temp = Local.get_unnamed_local () in
      let rhs_instrs = instr_pushl temp in
      let (lhs, rhs, setop) =
        emit_lval_op_nonlist_steps env pos LValOp.Set e_lhs rhs_instrs 1 in
      gather [
        result;
        instr_setl temp;
        instr_popc;
        with_temp_local temp (fun _ _ -> lhs);
        rhs;
        setop;
        instr_popc;
      ]
  | A.Expr (_, A.Yield_from e) ->
    gather [
      emit_yield_from_delegates env pos e;
      emit_pos pos;
      instr_popc;
    ]
  | A.Expr (pos, A.Binop (A.Eq None, e_lhs, (_, A.Yield_from e))) ->
    Local.scope @@ fun () ->
      let temp = Local.get_unnamed_local () in
      let rhs_instrs = instr_pushl temp in
      gather [
        emit_yield_from_delegates env pos e;
        instr_setl temp;
        instr_popc;
        emit_lval_op_nonlist env pos LValOp.Set e_lhs rhs_instrs 1;
        instr_popc;
      ]
  | A.Expr expr ->
    emit_ignored_expr ~pop_pos:pos env expr
  | A.Return None ->
    gather [
      instr_null;
      Emit_pos.emit_pos pos;
      emit_return ~need_ref:false env;
    ]
  | A.Return (Some expr) ->
    let need_ref = !return_by_ref in
    gather [
      emit_expr ~last_pos:pos ~need_ref env expr;
      if not need_ref then Emit_pos.emit_pos pos else empty;
      emit_return ~need_ref env;
    ]
  | A.GotoLabel (_, label) ->
    instr_label (Label.named label)
  | A.Goto (_, label) ->
    TFR.emit_goto ~in_finally_epilogue:false env label
  | A.Block b -> emit_stmts env b
  | A.If (condition, consequence, alternative) ->
    emit_if env pos condition consequence alternative
  | A.While (e, b) ->
    emit_while env e (pos, A.Block b)
  | A.Declare (is_block, e, b) ->
    emit_declare env is_block e b
  | A.Using {
      Ast.us_has_await = has_await;
      Ast.us_expr = e; Ast.us_block = b;
      Ast.us_is_block_scoped = is_block_scoped
    } ->
    emit_using env pos is_block_scoped has_await e (block_pos b, A.Block b)
  | A.Break level_opt ->
    emit_break env pos (get_level pos "break" level_opt)
  | A.Continue level_opt ->
    emit_continue env pos (get_level pos "continue" level_opt)
  | A.Do (b, e) ->
    emit_do env (pos, A.Block b) e
  | A.For (e1, e2, e3, b) ->
    emit_for env pos e1 e2 e3 (pos, A.Block b)
  | A.Throw e ->
    gather [
      emit_expr ~need_ref:false env e;
      Emit_pos.emit_pos pos;
      instr (IContFlow Throw);
    ]
  | A.Try (try_block, catch_list, finally_block) ->
    if (JT.get_function_has_goto ()) then
      TFR.fail_if_goto_from_try_to_finally try_block finally_block;
    if catch_list <> [] && finally_block <> [] then
      emit_stmt env (pos, A.Try([pos, A.Try (try_block, catch_list, [])], [], finally_block))
    else if catch_list <> [] then
      emit_try_catch env (pos, A.Block try_block) catch_list
    else
      emit_try_finally env pos (pos, A.Block try_block) (pos, A.Block finally_block)

  | A.Switch (e, cl) ->
    emit_switch env pos e cl
  | A.Foreach (collection, await_pos, iterator, block) ->
    emit_foreach env pos collection await_pos iterator (pos, A.Block block)
  | A.Def_inline def ->
    emit_def_inline def
  | A.Static_var es ->
    emit_static_var pos es
  | A.Global_var es ->
    emit_global_vars env pos es
  | A.Markup ((_, s), echo_expr_opt) ->
    emit_markup env s echo_expr_opt ~check_for_hashbang:false
    (* TODO: What do we do with unsafe? *)
  | A.Unsafe
  | A.Fallthrough
  | A.Noop -> empty

and emit_break env pos level =
  TFR.emit_break_or_continue ~is_break:true ~in_finally_epilogue:false env pos level

and emit_continue env pos level =
  TFR.emit_break_or_continue ~is_break:false ~in_finally_epilogue:false env pos level

and get_instrs r = r.Emit_expression.instrs

and emit_if env pos condition consequence alternative =
  match alternative with
  | []
  | [_, A.Noop] ->
    let done_label = Label.next_regular () in
    gather [
      get_instrs @@ emit_jmpz env condition done_label;
      emit_stmts env consequence;
      instr_label done_label;
    ]
  | _ ->
    let alternative_label = Label.next_regular () in
    let done_label = Label.next_regular () in
    let consequence_instr = emit_stmts env consequence in
    let alternative_instr = emit_stmts env alternative in
    gather [
      get_instrs @@ emit_jmpz env condition alternative_label;
      consequence_instr;
      emit_pos pos;
      instr_jmp done_label;
      instr_label alternative_label;
      alternative_instr;
      instr_label done_label;
    ]

and emit_global_vars env p es =
  let emit_global_var (_, e) =
    match e with
    | A.Id (_, name) when name.[0] = '$' ->
      if SN.Superglobals.is_superglobal name
      then empty
      else
        gather [
          instr_string (SU.Locals.strip_dollar name);
          instr_vgetg;
          instr_bindl @@ Local.Named name;
          instr_popv;
        ]
    | A.Dollar e ->
      let rec emit_inner e =
        match snd e with
        | A.Lvar (name_pos, id) ->
          if SN.Superglobals.is_superglobal id then
            gather [
              Emit_pos.emit_pos name_pos;
              instr_string (SU.Locals.strip_dollar id);
              Emit_pos.emit_pos (fst e);
              instr_cgetg;
            ]
          else
            instr_cgetl (Local.Named id)
        | A.Dollar e ->
          gather [emit_inner e; Emit_pos.emit_pos p; instr_cgetn]
        | _ ->
          emit_expr ~need_ref:false env e in
      gather [
        emit_inner e;
        Emit_pos.emit_pos p;
        instr_dup;
        instr_vgetg;
        instr_bindn;
        instr_popv;
      ]
    | _ ->
      failwith "Global var - impossible"
  in
  (* Deduplicate global variable declarations *)
  let _, instrs = List.fold es ~init:([], [])
    ~f:begin fun (seen, instrs)  e ->
      match snd e with
      | A.Id (_, name) when List.mem seen name ->
        seen, instrs
      | A.Id (_, name) ->
        name::seen, (emit_global_var e)::instrs
      | _ ->
        seen, (emit_global_var e)::instrs
      end in
  Emit_pos.emit_pos_then p @@ gather (List.rev instrs)

and emit_static_var pos es =
  let emit_static_var_single e =
    match snd e with
    | A.Lvar (_, name)
    | A.Binop (A.Eq _, (_, A.Lvar (_, name)), _) ->
      gather [
        Emit_pos.emit_pos pos;
        instr_static_loc_init name
      ]
    | _ -> failwith "Static var - impossible"
  in
  gather @@ List.map es ~f:emit_static_var_single

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
  gather [
    get_instrs @@ emit_jmpz env e break_label;
    instr_label start_label;
    (Emit_env.do_in_loop_body break_label cont_label env b emit_stmt);
    instr_label cont_label;
    get_instrs @@ emit_jmpnz env e start_label;
    instr_label break_label;
  ]

and emit_declare env is_block (p, e) b =
  (* TODO: We are ignoring the directive (e) here?? *)
  let errors =
    match e with
    | A.Binop (A.Eq None, (_, A.Id (_, "strict_types")), _) when is_block ->
      Emit_fatal.emit_fatal_runtime
        p "strict_types declaration must not use block mode"
    | _ -> empty
  in
  gather [ errors; emit_stmts env b ]

and emit_using env pos is_block_scoped has_await e b =
  match snd e with
  | A.Expr_list es ->
    emit_stmt env @@ List.fold_right es
      ~f:(fun e acc ->
        fst e, A.Using {
          Ast.us_has_await = has_await;
          Ast.us_is_block_scoped = is_block_scoped;
          Ast.us_expr = e;
          Ast.us_block = [acc];
        })
      ~init:b
  | _ ->
    Local.scope @@ begin fun () ->
    let local, preamble = match snd e with
      | A.Binop (A.Eq None, (_, A.Lvar (_, id)), _)
      | A.Lvar (_, id) ->
        Local.Named id, gather [
          emit_expr ~need_ref:false env e;
          Emit_pos.emit_pos (fst b);
          instr_popc;
        ]
      | _ ->
        let l = Local.get_unnamed_local () in
        l, gather [
          emit_expr ~need_ref:false env e;
          instr_setl l;
          instr_popc
        ]
    in
    let finally_start = Label.next_regular () in
    let finally_end = Label.next_regular () in
    let body = Emit_env.do_in_using_body finally_start env b emit_stmt in
    let jump_instructions = TFR.collect_jump_instructions body env in
    let body =
      if IMap.is_empty jump_instructions then body
      else TFR.cleanup_try_body body
    in
    let fn_name = Hhbc_id.Method.from_raw_string @@
      if has_await then "__disposeAsync" else "__dispose"
    in
    let epilogue =
      if has_await then
        gather [
          instr_unboxr;
          instr_await;
          instr_popc
        ]
      else
        instr_popr
    in
    let finally = gather [
        instr_cgetl local;
        instr_fpushobjmethodd 0 fn_name A.OG_nullthrows;
        instr_fcall 0 false 1;
        epilogue;
        if is_block_scoped then instr_unsetl local else empty;
      ]
    in
    let finally_epilogue =
      TFR.emit_finally_epilogue
        env pos ~verify_return:!verify_return ~verify_out:!verify_out ~num_out:!num_out
        jump_instructions finally_end
    in
    let exn_local = Local.get_unnamed_local () in
    let after_catch = Label.next_regular() in
    let middle =
      if is_empty_block b then empty
      else gather [
        instr_try_catch_begin;
          body;
          instr_jmp after_catch;
        instr_try_catch_middle;
          emit_pos (fst b);
          make_finally_catch exn_local finally;
          emit_pos pos;
        instr_try_catch_end;
        instr_label after_catch;
      ]
    in
    gather [
      preamble;
      middle;
      instr_label finally_start;
      finally;
      finally_epilogue;
      instr_label finally_end;
    ]
    end

and emit_do env b e =
  let cont_label = Label.next_regular () in
  let break_label = Label.next_regular () in
  let start_label = Label.next_regular () in
  gather [
    instr_label start_label;
    (Emit_env.do_in_loop_body break_label cont_label env b emit_stmt);
    instr_label cont_label;
    get_instrs @@ emit_jmpnz env e start_label;
    instr_label break_label;
  ]

and emit_for env p e1 e2 e3 b =
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
      get_instrs (if jmpz then emit_jmpz env cond label else emit_jmpnz env cond label)
    in
    let rec expr_list h tl =
      match tl with
      | [] -> [final @@ (Pos.none, A.Expr_list [h])]
      | h1 :: t1 -> emit_ignored_expr env ~pop_pos:p h :: expr_list h1 t1
    in
    match e2 with
    | _, A.Expr_list [] -> if jmpz then empty else instr_jmp label
    | _, A.Expr_list (h::t) -> gather @@ expr_list h t
    | cond -> final cond
  in
  gather [
    emit_ignored_expr env ~pop_pos:p e1;
    emit_cond ~jmpz:true break_label;
    instr_label start_label;
    (Emit_env.do_in_loop_body break_label cont_label env b emit_stmt);
    instr_label cont_label;
    emit_ignored_expr env ~pop_pos:p e3;
    emit_cond ~jmpz:false start_label;
    instr_label break_label;
  ]

and emit_switch env pos scrutinee_expr cl =
  if List.is_empty cl
  then emit_ignored_expr env scrutinee_expr
  else
  stash_in_local env pos scrutinee_expr
  begin fun local break_label ->
  (* If there is no default clause, add an empty one at the end *)
  let is_default c = match c with A.Default _ -> true | _ -> false in
  let cl, has_default =
    match List.count cl is_default with
    | 0 -> cl @ [A.Default []], false
    | 1 -> cl, true
    | _ -> Emit_fatal.raise_fatal_runtime
      pos "Switch statements may only contain one 'default' clause." in
  (* "continue" in a switch in PHP has the same semantics as break! *)
  let cl =
    Emit_env.do_in_switch_body break_label env cl @@
      fun env _ -> List.map cl ~f:(emit_case env)
  in
  let bodies = gather @@ List.map cl ~f:snd in
  let default_label_to_shift =
    if has_default
    then List.find_map cl ~f: (fun ((e, l), _) ->
      if Option.is_none e then Some l else None)
    else None in
  let init = gather @@ List.map cl
    ~f: begin fun x ->
          let (e_opt, l) = fst x in
          match e_opt with
          | None ->
            (* jmp to default case should be emitted as the
            very last 'else' case so do not emit it if it appear in the
            middle of emitted if/elseif clauses *)
            if Option.is_none default_label_to_shift
            then instr_jmp l
            else empty
          | Some e ->
            (* Special case for simple scrutinee *)
            match scrutinee_expr with
            | _, A.Lvar _ ->
              let eq_expr = pos, A.Binop (A.Eqeq, scrutinee_expr, e) in
              gather [
                emit_expr ~need_ref:false env eq_expr;
                instr_jmpnz l
              ]
            | _ ->
              gather [
                instr_cgetl local;
                emit_expr ~need_ref:false env e;
                instr_eq;
                instr_jmpnz l]
        end
  in
  gather [
    init;
    Option.value_map default_label_to_shift ~default:empty ~f:instr_jmp;
    bodies;
  ]
  end

and block_pos b =
  let bpos = List.map b fst in
  let valid_pos = List.filter bpos (fun e -> e <> Pos.none) in
  if valid_pos = [] then Pos.none
  else Pos.btw (List.hd_exn valid_pos) (List.last_exn valid_pos)

and emit_catch env pos end_label (catch_type, (_, catch_local), b) =
    (* Note that this is a "regular" label; we're not going to branch to
    it directly in the event of an exception. *)
    let next_catch = Label.next_regular () in
    let id, _ = Hhbc_id.Class.elaborate_id
      (Emit_env.get_namespace env) catch_type in
    gather [
      instr_dup;
      instr_instanceofd id;
      instr_jmpz next_catch;
      instr_setl (Local.Named catch_local);
      instr_popc;
      emit_stmt env (Pos.none, A.Block b);
      Emit_pos.emit_pos pos;
      instr_jmp end_label;
      instr_label next_catch;
    ]

and emit_catches env pos catch_list end_label =
  gather (List.map catch_list ~f:(emit_catch env pos end_label))

and is_empty_block b =
  match b with
  | _, A.Block l -> List.for_all ~f:is_empty_block l
  | _, A.Noop -> true
  | _ -> false

and emit_try_catch env try_block catch_list =
  Local.scope @@ fun () ->
    emit_try_catch_ env try_block catch_list

and emit_try_catch_ env try_block catch_list =
  if is_empty_block try_block then empty
  else
  let end_label = Label.next_regular () in
  let (pos, _) = try_block in
  let try_env = Emit_env.with_try env in
  gather [
    instr_try_catch_begin;
    emit_stmt try_env try_block;
    Emit_pos.emit_pos pos;
    instr_jmp end_label;
    instr_try_catch_middle;
    emit_catches env pos catch_list end_label;
    instr_throw;
    instr_try_catch_end;
    instr_label end_label;
  ]

and emit_try_finally env pos try_block finally_block =
  Local.scope @@ fun () ->
    emit_try_finally_ env pos try_block finally_block

and emit_try_finally_ env pos try_block finally_block =
  let make_finally_body () =
    Emit_env.do_in_finally_body env finally_block emit_stmt
  in
  if is_empty_block try_block then make_finally_body ()
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
    Emit_env.do_in_try_body finally_start try_env try_block emit_stmt in
  let jump_instructions =
    TFR.collect_jump_instructions try_body env
  in
  let try_body =
    if IMap.is_empty jump_instructions then try_body
    else TFR.cleanup_try_body try_body
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
    finally_body
    |> Label_rewriter.clone_with_fresh_regular_labels
    |> strip_fault_bodies
  in

  (* (3) Finally epilogue *)

  let finally_epilogue =
    TFR.emit_finally_epilogue
      env pos ~verify_return:!verify_return ~verify_out:!verify_out ~num_out:!num_out
      jump_instructions finally_end
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
  let after_catch = Label.next_regular() in
  let middle = gather [
    instr_try_catch_begin;
      try_body;
      instr_jmp after_catch;
    instr_try_catch_middle;
      emit_pos enclosing_span;
      make_finally_catch exn_local finally_body_for_catch;
    instr_try_catch_end;
    instr_label after_catch;
  ]
  in
  (* Put it all together. *)
  gather [
    middle;
    instr_label finally_start;
    Emit_pos.emit_pos (fst finally_block);
    finally_body;
    finally_epilogue;
    instr_label finally_end;
  ]

and make_finally_catch exn_local finally_body =
  let after_catch = Label.next_regular() in
  gather [
    instr_popl exn_local;
    instr_unsetl (Local.get_label_id_local ());
    instr_unsetl (Local.get_retval_local ());
    instr_try_catch_begin;
      finally_body;
      instr_jmp after_catch;
    instr_try_catch_middle;
      instr_pushl exn_local;
      instr_chain_faults;
      instr_throw;
    instr_try_catch_end;
    instr_label after_catch;
    instr_pushl exn_local;
    instr_throw;
  ]

and is_mutable_iterator iterator =
  match iterator with
  | A.As_kv (_, (_, A.Unop(A.Uref, _)))
  | A.As_v (_, A.Unop(A.Uref, _)) -> true
  | _ -> false

and get_id_of_simple_lvar_opt ~is_key v =
  match v with
  | A.Lvar (pos, str) when str = SN.SpecialIdents.this ->
    Emit_fatal.raise_fatal_parse pos "Cannot re-assign $this"
  | A.Unop (A.Uref, (_, A.Lvar (pos, _))) when is_key ->
    Emit_fatal.raise_fatal_parse pos "Key element cannot be a reference"
  | A.Lvar (_, id) | A.Unop (A.Uref, (_, A.Lvar (_, id)))
    when not (SN.Superglobals.is_superglobal id) -> Some id
  | _ -> None

and emit_load_list_elements env path vs =
  let preamble, load_value =
    List.mapi ~f:(emit_load_list_element env path) vs
    |> List.unzip
  in
  List.concat preamble, List.concat load_value

and emit_load_list_element env path i v =
  let query_value = gather [
    gather @@ List.rev path;
    instr_querym 0 QueryOp.CGet (MemberKey.EI (Int64.of_int i));
  ]
  in
  match v with
  | _, A.Lvar (_, id) ->
    let load_value = gather [
      query_value;
      instr_setl (Local.Named id);
      instr_popc
    ]
    in
    [], [load_value]
  | _, A.Dollar (_, A.Lvar (_, id)) ->
    let local = Local.Named id in
    [empty], [gather [
      query_value;
      instr_cgetl2 local;
      instr_setn;
      instr_popc
    ]]
  | _, A.Dollar e ->
    [emit_expr ~need_ref:false env e],
    [gather [query_value; instr_setn; instr_popc]]
  | _, A.List exprs ->
    let dim_instr =
      instr_dim MemberOpMode.Warn (MemberKey.EI (Int64.of_int i))
    in
    emit_load_list_elements env (dim_instr::path) exprs
  | pos, _ ->
    let set_instrs = emit_lval_op_nonlist env pos LValOp.Set v query_value 1 in
    let load_value = [set_instrs; instr_popc] in
    [], [gather load_value]

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
and emit_iterator_key_value_storage env iterator =
  match iterator with
  | A.As_kv (((_, k) as expr_k), ((_, v) as expr_v)) ->
    begin match get_id_of_simple_lvar_opt ~is_key:true k,
                get_id_of_simple_lvar_opt ~is_key:false v with
    | Some key_id, Some value_id ->
      let key_local = Local.Named key_id in
      let value_local = Local.Named value_id in
      Some key_local, value_local, [], []
    | _ ->
      let key_local = Local.get_unnamed_local () in
      let value_local = Local.get_unnamed_local () in
      let key_preamble, key_load =
        emit_iterator_lvalue_storage env expr_k key_local in
      let value_preamble, value_load =
        emit_iterator_lvalue_storage env expr_v value_local
      in

      (* HHVM prepends code to initialize non-plain, non-list foreach-key
         to the value preamble - do the same to minimize diffs *)
      let key_preamble, value_preamble =
        match k with
        | A.List _ -> key_preamble, value_preamble
        | _ -> [], (gather key_preamble) :: value_preamble
      in
      Some key_local, value_local,
      (gather key_preamble)::key_load,
      (gather value_preamble)::value_load
    end
  | A.As_v ((_, v) as expr_v) ->
    begin match get_id_of_simple_lvar_opt ~is_key:false v with
    | Some value_id ->
      let value_local = Local.Named value_id in
      None, value_local, [], []
    | None ->
      let value_local = Local.get_unnamed_local () in
      let value_preamble, value_load =
        emit_iterator_lvalue_storage env expr_v value_local in
      None, value_local, [], value_preamble @ value_load
    end

(* Emit code for either the key or value l-value operation in foreach await.
 * `indices` is the initial prefix of the array indices ([0] for key or [1] for
 * value) that is prepended onto the indices needed for list destructuring
 *)
and emit_foreach_await_lvalue_storage env expr1 indices local =
  let instrs1, instrs2 = emit_lval_op_list env (fst expr1) (Some local) indices expr1 in
    gather [
      instrs1;
      instrs2;
    ]

(* Emit code for the value and possibly key l-value operation in a foreach
 * await statement. `local` is the temporary into which the result of invoking
 * the `next` method has been stored. For example:
 *   foreach (foo() await as $a->f => list($b[0], $c->g)) { ... }
 * Here, we need to construct l-value operations that access the [0] (for $a->f)
 * and [1;0] (for $b[0]) and [1;1] (for $c->g) indices of the array returned
 * from the `next` method.
 *)
and emit_foreach_await_key_value_storage env iterator local =
  match iterator with
  | A.As_kv (expr_k, expr_v) ->
    let key_instrs = emit_foreach_await_lvalue_storage env expr_k [0] local in
    let value_instrs = emit_foreach_await_lvalue_storage env expr_v [1] local in
    gather [key_instrs; value_instrs]

  | A.As_v expr_v ->
    emit_foreach_await_lvalue_storage env expr_v [1] local

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
  | pos, A.Call _ ->
    Emit_fatal.raise_fatal_parse pos "Can't use return value in write context"
  | _, A.List exprs ->
    let preamble, load_values =
      emit_load_list_elements env [instr_basel local MemberOpMode.Warn] exprs
    in
    let load_values = [
      gather @@ (List.rev load_values);
      instr_unsetl local
    ]
    in
    preamble, load_values
  | pos, x ->
    match x with
    | A.Unop (A.Uref, e) ->
      let (lhs, rhs, set_op) =
        emit_lval_op_nonlist_steps env pos LValOp.SetRef e (instr_vgetl local) 1
      in
      [lhs], [
        rhs;
        set_op;
        instr_popv;
        instr_unsetl local
      ]
    | _ ->
      let (lhs, rhs, set_op) =
        emit_lval_op_nonlist_steps env pos LValOp.Set v (instr_cgetl local) 1
      in
      [lhs], [
        rhs;
        set_op;
        instr_popc;
        instr_unsetl local
      ]

and wrap_non_empty_block_in_fault prefix block fault_block =
  match block with
  | [] -> prefix
  | block ->
    instr_try_fault
      (Label.next_fault())
      (gather @@ prefix::block)
      fault_block

and emit_foreach env pos collection await_pos iterator block =
  Local.scope @@ fun () ->
    match await_pos with
    | None -> emit_foreach_ env pos collection iterator block
    | Some pos -> emit_foreach_await env pos collection iterator block

and emit_foreach_await env pos collection iterator block =
  let next_label = Label.next_regular () in
  let exit_label = Label.next_regular () in
  let iter_temp_local = Local.get_unnamed_local () in
  let collection_expr = emit_expr ~need_ref:false env collection in
  let result_temp_local = Local.get_unnamed_local () in
  let next_meth = Hhbc_id.Method.from_raw_string "next" in
  let set_key_and_value =
    emit_foreach_await_key_value_storage env iterator result_temp_local in
  gather [
    collection_expr;
    instr_setl iter_temp_local;
    with_temp_local iter_temp_local begin fun _ _ -> gather [
      instr_instanceofd (Hhbc_id.Class.from_raw_string "HH\\AsyncIterator");
      instr_jmpnz next_label;
      Emit_fatal.emit_fatal_runtime pos
        "Unable to iterate non-AsyncIterator asynchronously";
      instr_label next_label;
      instr_cgetl iter_temp_local;
      instr_fpushobjmethodd 0 next_meth A.OG_nullthrows;
      instr_fcall 0 false 1;
      instr_unboxr;
      instr_await;
      instr_setl result_temp_local;
      instr_popc;
      instr_istypel result_temp_local OpNull;
      instr_jmpnz exit_label;
      with_temp_local result_temp_local begin fun _ _ -> set_key_and_value end;
      instr_unsetl result_temp_local;
      (Emit_env.do_in_loop_body exit_label next_label env block emit_stmt);
      emit_pos pos;
      instr_jmp next_label;
      instr_label exit_label;
      instr_unsetl result_temp_local;
    ] end;
    instr_unsetl iter_temp_local;
  ]

and emit_foreach_ env pos collection iterator block =
  let enclosing_span = Ast_scope.Scope.get_span env.Emit_env.env_scope in
  let iterator_number = Iterator.get_iterator () in
  let fault_label = Label.next_fault () in
  let loop_break_label = Label.next_regular () in
  let loop_continue_label = Label.next_regular () in
  let loop_head_label = Label.next_regular () in
  let mutable_iter = is_mutable_iterator iterator in
  let key_local_opt, value_local, key_preamble, value_preamble =
    emit_iterator_key_value_storage env iterator
  in
  let fault_block_local local = gather [
    emit_pos enclosing_span;
    instr_unsetl local;
    instr_unwind
  ]
  in
  let init, next, preamble = match key_local_opt with
  | Some (key_local) ->
    let initf, nextf =
      if mutable_iter then instr_miterinitk, instr_miternextk
      else instr_iterinitk, instr_iternextk
    in
    let init = initf iterator_number loop_break_label value_local key_local in
    let cont = nextf iterator_number loop_head_label value_local key_local in
    let preamble =
      wrap_non_empty_block_in_fault
        (instr_label loop_head_label)
        value_preamble
        (fault_block_local value_local)
    in
    let preamble =
      wrap_non_empty_block_in_fault
        preamble
        key_preamble
        (fault_block_local key_local)
    in
    init, cont, preamble
  | None ->
    let initf, nextf =
      if mutable_iter then instr_miterinit, instr_miternext
      else instr_iterinit, instr_iternext
    in
    let init = initf iterator_number loop_break_label value_local in
    let cont = nextf iterator_number loop_head_label value_local in
    let preamble =
      wrap_non_empty_block_in_fault
        (instr_label loop_head_label)
        value_preamble
        (fault_block_local value_local)
    in
    init, cont, preamble
  in

  let body =
    Emit_env.do_in_loop_body loop_break_label loop_continue_label env
      ~iter:(mutable_iter, iterator_number) block emit_stmt in
  let result = gather [
    emit_expr ~last_pos:pos ~need_ref:mutable_iter env collection;
    init;
    instr_try_fault
      fault_label
      (* try body *)
      (gather [
        preamble;
        body;
        instr_label loop_continue_label;
        Emit_pos.emit_pos pos;
        next
      ])
      (* fault body *)
      (gather [
        Emit_pos.emit_pos enclosing_span;
        if mutable_iter then instr_miterfree iterator_number
        else instr_iterfree iterator_number;
        instr_unwind ]);
    instr_label loop_break_label
  ] in
  Iterator.free_iterator ();
  result

and emit_yield_from_delegates env pos e =
  let iterator_number = Iterator.get_iterator () in
  let loop_label = Label.next_regular () in
  let fault_label = Label.next_fault () in
  let body =
    gather [
      instr_null;
      instr_label loop_label;
      instr_contEnterDelegate;
      instr_yieldFromDelegate iterator_number loop_label;
    ]
  in
  let fault_body =
    gather [
      emit_pos (Ast_scope.Scope.get_span env.Emit_env.env_scope);
      instr_contUnsetDelegate_free iterator_number;
      instr_unwind;
    ]
  in
  gather [
    emit_expr ~need_ref:false env e;
    emit_pos pos;
    instr_contAssignDelegate iterator_number;
    instr_try_fault fault_label body fault_body;
    instr_contUnsetDelegate_ignore iterator_number;
  ]

and emit_stmts env stl =
  let results = List.map stl (emit_stmt env) in
  gather results

and emit_case env c =
  let l = Label.next_regular () in
  let b = match c with
    | A.Default b
    | A.Case (_, b) ->
        emit_stmt env (Pos.none, A.Block b)
  in
  let e = match c with
    | A.Case (e, _) -> Some e
    | _ -> None
  in
  (e, l), gather [instr_label l; b]

let emit_dropthrough_return env =
  match !default_dropthrough with
  | Some instrs -> instrs
  | _ ->
    Emit_pos.emit_pos_then (Pos.last_char !function_pos) @@
    gather [!default_return_value; emit_return ~need_ref:false env]

let rec emit_final_statement env s =
  match snd s with
  | A.Throw _ | A.Return _ | A.Goto _
  | A.Expr (_, A.Yield_break) ->
    emit_stmt env s
  | A.Block b ->
    emit_final_statements env b
  | _ ->
    gather [
      emit_stmt env s;
      emit_dropthrough_return env
    ]

and emit_final_statements env b =
  match b with
  | [] -> emit_dropthrough_return env
  | [s] -> emit_final_statement env s
  | s::b ->
    let i1 = emit_stmt env s in
    let i2 = emit_final_statements env b in
    gather [i1; i2]
