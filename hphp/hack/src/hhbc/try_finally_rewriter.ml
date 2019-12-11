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
module JT = Jump_targets
module RGH = Reified_generics_helpers
module T = Aast

(* Collect list of Ret* and non rewritten Break/Continue instructions inside
   try body. *)
let collect_jump_instructions instrseq env =
  let jump_targets = Emit_env.get_jump_targets env in
  let get_label_id ~is_break level =
    match JT.get_target_for_level is_break level jump_targets with
    | JT.ResolvedRegular (target_label, _)
    | JT.ResolvedTryFinally { JT.target_label; _ } ->
      JT.get_id_for_label target_label
    | _ -> failwith "impossible"
  in
  let folder map i =
    match i with
    | ISpecialFlow (Break l) -> IMap.add (get_label_id ~is_break:true l) i map
    | ISpecialFlow (Continue l) ->
      IMap.add (get_label_id ~is_break:false l) i map
    | IContFlow (RetC | RetCSuspended | RetM _) ->
      IMap.add (JT.get_id_for_return ()) i map
    | ISpecialFlow (Goto l) ->
      IMap.add (JT.get_id_for_label (Label.named l)) i map
    | _ -> map
  in
  InstrSeq.fold_left instrseq ~init:IMap.empty ~f:folder

(* Delete Ret*, Break/Continue/Jmp(Named) instructions from the try body *)
let cleanup_try_body instrseq =
  let rewriter i =
    match i with
    | ISpecialFlow _
    | IContFlow (RetC | RetCSuspended | RetM _) ->
      None
    | _ -> Some i
  in
  InstrSeq.filter_map instrseq ~f:rewriter

let emit_jump_to_label l iters =
  match iters with
  | [] -> instr_jmp l
  | iters -> instr_iter_break l iters

let emit_save_label_id id =
  gather [instr_int id; instr_setl (Local.get_label_id_local ()); instr_popc]

let get_pos_for_error env =
  let aux_pos p =
    match p with
    | None -> Pos.none
    | Some p -> Pos.first_char_of_line p
  in
  let rec aux_scope scope =
    match scope with
    | Ast_scope.ScopeItem.Function fd :: _ -> aux_pos @@ Some fd.T.f_span
    (* For methods, it points to class not the method.. weird *)
    | Ast_scope.ScopeItem.Class cd :: _ -> aux_pos @@ Some cd.T.c_span
    | Ast_scope.ScopeItem.Method _ :: scope
    | Ast_scope.ScopeItem.Lambda _ :: scope
    | Ast_scope.ScopeItem.LongLambda _ :: scope ->
      aux_scope scope
    | _ -> aux_pos None
  in
  aux_scope @@ Emit_env.get_scope env

let fail_if_goto_from_try_to_finally try_block finally_block =
  let find_gotos_in block =
    let visitor =
      let state = ref [] in
      object
        inherit [_] Aast.iter

        method! on_Goto () label = state := label :: !state

        method state () = !state
      end
    in
    let _ = visitor#on_block () block in
    visitor#state ()
  in
  let goto_labels = find_gotos_in try_block in
  let fail_if_find_any_label_in block =
    let visitor =
      object
        inherit [_] Aast.iter

        method! on_GotoLabel () (_, label) =
          let label_opt = List.find ~f:(fun (_, l) -> l = label) goto_labels in
          match label_opt with
          | Some (p, _) ->
            Emit_fatal.raise_fatal_parse
              p
              "'goto' into finally statement is disallowed"
          | None -> ()
      end
    in
    visitor#on_block block
  in
  fail_if_find_any_label_in () finally_block

let emit_goto ~in_finally_epilogue env label =
  let err_pos = get_pos_for_error env in
  match SMap.find_opt label @@ JT.get_labels_in_function () with
  | None ->
    Emit_fatal.raise_fatal_parse err_pos
    @@ "'goto' to undefined label '"
    ^ label
    ^ "'"
  | Some in_using ->
    let named_label = Label.named label in
    (* CONSIDER: we don't need to assign state id for label
      for cases when it is not necessary, i.e. when jump target is in the same
      scope. HHVM does not do this today, do the same for compatibility reasons *)
    let label_id = JT.get_id_for_label named_label in
    let jump_targets = Emit_env.get_jump_targets env in
    begin
      match JT.find_goto_target jump_targets label with
      | JT.ResolvedGoto_label iters ->
        let preamble =
          if not in_finally_epilogue then
            empty
          else
            instr_unsetl @@ Local.get_label_id_local ()
        in
        gather [preamble; emit_jump_to_label named_label iters]
      | JT.ResolvedGoto_finally
          { JT.rgf_finally_start_label; JT.rgf_iterators_to_release } ->
        let preamble =
          if in_finally_epilogue then
            empty
          else
            emit_save_label_id label_id
        in
        gather
          [
            preamble;
            emit_jump_to_label rgf_finally_start_label rgf_iterators_to_release;
            (* emit goto as an indicator for try/finally rewriter to generate
            finally epilogue, try/finally rewriter will remove it. *)
            instr_goto label;
          ]
      | JT.ResolvedGoto_goto_from_finally ->
        Emit_fatal.raise_fatal_runtime
          err_pos
          "Goto to a label outside a finally block is not supported"
      | JT.ResolvedGoto_goto_invalid_label ->
        let message =
          if in_using then
            "'goto' into or across using statement is disallowed"
          else
            "'goto' into loop or switch statement is disallowed"
        in
        Emit_fatal.raise_fatal_parse err_pos message
    end

let emit_return ~verify_return ~verify_out ~num_out ~in_finally_epilogue env =
  (* check if there are try/finally region *)
  let jump_targets = Emit_env.get_jump_targets env in
  match JT.get_closest_enclosing_finally_label jump_targets with
  (* no finally blocks, but there might be some iterators that should be
      released before exit - do it *)
  | None ->
    let verify_return_instr () =
      match verify_return with
      | None -> empty
      | Some h ->
        let h = RGH.convert_awaitable env h |> RGH.remove_erased_generics env in
        (match RGH.has_reified_type_constraint env h with
        | RGH.NoConstraint -> empty
        | RGH.NotReified -> instr_verifyRetTypeC
        | RGH.MaybeReified ->
          gather
            [
              Emit_expression.get_type_structure_for_hint
                ~targ_map:SMap.empty
                ~tparams:[]
                h;
              instr_verifyRetTypeTS;
            ]
        | RGH.DefinitelyReified ->
          let check = gather [instr_dup; instr_istypec OpNull] in
          RGH.simplify_verify_type env Pos.none check h instr_verifyRetTypeTS)
    in
    let release_iterators_instr =
      let iterators_to_release = JT.collect_iterators jump_targets in
      gather @@ List.map iterators_to_release ~f:instr_iterfree
    in
    if in_finally_epilogue then
      let load_retval_instr = instr_cgetl (Local.get_retval_local ()) in
      gather
        [
          load_retval_instr;
          verify_return_instr ();
          verify_out;
          release_iterators_instr;
          ( if num_out <> 0 then
            instr_retm (num_out + 1)
          else
            instr_retc );
        ]
    else
      gather
        [
          verify_return_instr ();
          verify_out;
          release_iterators_instr;
          ( if num_out <> 0 then
            instr_retm (num_out + 1)
          else
            instr_retc );
        ]
  (* ret is in finally block and there might be iterators to release -
    jump to finally block via Jmp *)
  | Some (target_label, iterators_to_release) ->
    let preamble =
      if in_finally_epilogue then
        empty
      else
        let save_state = emit_save_label_id (JT.get_id_for_return ()) in
        let save_retval =
          gather [instr_setl (Local.get_retval_local ()); instr_popc]
        in
        gather [save_state; save_retval]
    in
    gather
      [
        preamble;
        emit_jump_to_label target_label iterators_to_release;
        (* emit ret instr as an indicator for try/finally rewriter to generate
        finally epilogue, try/finally rewriter will remove it. *)
        instr_retc;
      ]

and emit_break_or_continue ~is_break ~in_finally_epilogue env pos level =
  let jump_targets = Emit_env.get_jump_targets env in
  match JT.get_target_for_level ~is_break level jump_targets with
  | JT.NotFound -> Emit_fatal.emit_fatal_for_break_continue pos level
  | JT.ResolvedRegular (target_label, iterators_to_release) ->
    let preamble =
      if in_finally_epilogue && level = 1 then
        instr_unsetl @@ Local.get_label_id_local ()
      else
        empty
    in
    gather
      [
        preamble;
        Emit_pos.emit_pos pos;
        emit_jump_to_label target_label iterators_to_release;
      ]
  | JT.ResolvedTryFinally
      {
        JT.target_label;
        JT.finally_label;
        JT.iterators_to_release;
        JT.adjusted_level;
      } ->
    let preamble =
      if not in_finally_epilogue then
        emit_save_label_id (JT.get_id_for_label target_label)
      else
        empty
    in
    gather
      [
        preamble;
        emit_jump_to_label finally_label iterators_to_release;
        Emit_pos.emit_pos pos;
        (* emit break/continue instr as an indicator for try/finally rewriter
         to generate finally epilogue - try/finally rewriter will remove it. *)
        ( if is_break then
          instr_break adjusted_level
        else
          instr_continue adjusted_level );
      ]

let emit_finally_epilogue
    ~verify_return
    ~verify_out
    ~num_out
    (env : Emit_env.t)
    pos
    jump_instructions
    finally_end =
  let emit_instr i =
    match i with
    | IContFlow (RetM _)
    | IContFlow RetC
    | IContFlow RetCSuspended ->
      emit_return
        ~verify_return
        ~verify_out
        ~num_out
        ~in_finally_epilogue:true
        env
    | ISpecialFlow (Break l) ->
      emit_break_or_continue ~is_break:true ~in_finally_epilogue:true env pos l
    | ISpecialFlow (Continue l) ->
      emit_break_or_continue ~is_break:false ~in_finally_epilogue:true env pos l
    | ISpecialFlow (Goto l) -> emit_goto ~in_finally_epilogue:true env l
    | _ ->
      failwith
      @@ "unexpected instruction: "
      ^ "only Ret* or Break/Continue/Jmp(Named) are expected"
  in
  match IMap.elements jump_instructions with
  | [] -> empty
  | [(_, h)] ->
    gather
      [
        Emit_pos.emit_pos pos;
        instr_issetl (Local.get_label_id_local ());
        instr_jmpz finally_end;
        emit_instr h;
      ]
  | (max_id, _) :: _ as lst ->
    (* mimic HHVM behavior:
      in some cases ids can be non-consequtive - this might happen i.e. return statement
       appear in the block and it was assigned a high id before.
       ((3, Return), (1, Break), (0, Continue))
       In thid case generate switch as
       switch  (id) {
          L0 -> handler for continue
          L1 -> handler for break
          FinallyEnd -> empty
          L3 -> handler for return
       }
       *)
    (* This function builds a list of labels and jump targets for switch.
    It is possible that cases ids are not consequtive
      [L1,L2,L4]. Vector of labels in switch should be dense so we need to
      fill holes with a label that points to the end of finally block
      [End, L1, L2, End, L4]
    *)
    let rec aux n instructions labels bodies =
      match instructions with
      | [] when n >= 0 ->
        aux (n - 1) instructions (finally_end :: labels) (empty :: bodies)
      | [] -> (labels, bodies)
      | (id, instruction) :: t ->
        if id = n then
          let label = Label.next_regular () in
          let body = gather [instr_label label; emit_instr instruction] in
          aux (n - 1) t (label :: labels) (body :: bodies)
        else
          aux (n - 1) instructions (finally_end :: labels) (empty :: bodies)
    in
    (* lst is already sorted - IMap.bindings took care of it *)
    (* TODO: add is_sorted assert to make sure this behavior is preserved *)
    let (labels, bodies) = aux max_id lst [] [] in
    let labels = labels in
    gather
      [
        Emit_pos.emit_pos pos;
        instr_issetl (Local.get_label_id_local ());
        instr_jmpz finally_end;
        instr_cgetl (Local.get_label_id_local ());
        instr_switch labels;
        gather bodies;
      ]

(*
TODO: This codegen is unnecessarily complex.  Basically we are generating

IsSetL temp
JmpZ   finally_end
CGetL  temp
Switch Unbounded 0 <L4 L5>
L5:
UnsetL temp
Jmp LContinue
L4:
UnsetL temp
Jmp LBreak

Two problems immediately come to mind. First, why is the unset in every case,
instead of after the CGetL?  Surely the unset doesn't modify the stack.
Second, now we have a jump-to-jump situation.

Would it not make more sense to instead say

IsSetL temp
JmpZ   finally_end
CGetL  temp
UnsetL temp
Switch Unbounded 0 <LBreak LContinue>

?
*)
