(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Shape_analysis_types
module Cont = Typing_continuations
module A = Aast
module T = Tast
module SN = Naming_special_names
module Env = Shape_analysis_env
module Utils = Aast_names_utils
module HT = Hips_types

let join ~pos ~origin (env : env) (left : entity_) (right : entity_) :
    env * entity_ =
  let join = Env.fresh_var () in
  let decorate constraint_ = { hack_pos = pos; origin; constraint_ } in
  let add_constraint env c = Env.add_constraint env @@ decorate c in
  let constraints = [Subsets (left, join); Subsets (right, join)] in
  let env = List.fold ~f:add_constraint ~init:env constraints in
  (env, join)

let when_tast_check tast_env ~default f =
  let tcopt = Tast_env.get_tcopt tast_env |> TypecheckerOptions.log_levels in
  match SMap.find_opt "shape_analysis" tcopt with
  | Some level when level > 0 -> f ()
  | _ -> default

let failwithpos pos msg =
  raise @@ Shape_analysis_exn (Format.asprintf "%a: %s" Pos.pp pos msg)

let failwith = failwithpos Pos.none

let collect_analysis_targets :
    Provider_context.t -> Tast.program -> potential_targets =
  let reducer =
    object (this)
      inherit [_] Tast_visitor.reduce as super

      method zero = { expressions_to_modify = []; hints_to_modify = [] }

      method plus
          { expressions_to_modify = es1; hints_to_modify = hs1 }
          { expressions_to_modify = es2; hints_to_modify = hs2 } =
        { expressions_to_modify = es1 @ es2; hints_to_modify = hs1 @ hs2 }

      method! on_expr env ((_, pos, exp_proper) as exp) =
        let expressions_to_modify =
          match exp_proper with
          | A.Darray _ -> [pos]
          | A.KeyValCollection (A.Dict, _, _) -> [pos]
          | _ -> []
        in
        let accumulator = { expressions_to_modify; hints_to_modify = [] } in
        this#plus accumulator (super#on_expr env exp)

      method! on_hint env ((pos, hint_proper) as hint) =
        let hints_to_modify =
          match hint_proper with
          | A.Happly ((_, id), _) when String.equal id SN.Collections.cDict ->
            [pos]
          | _ -> []
        in
        let accumulator = { expressions_to_modify = []; hints_to_modify } in
        this#plus accumulator (super#on_hint env hint)
    end
  in
  reducer#go

(* Is the type a suitable dict that can be coverted into shape. For the moment,
   that's only the case if the key is a string. *)
let rec is_suitable_target_ty tast_env ty =
  let ty = Tast_env.fully_expand tast_env ty in
  match Typing_defs.get_node ty with
  | Typing_defs.Tclass ((_, id), _, [ty])
    when String.equal id SN.Classes.cAwaitable ->
    is_suitable_target_ty tast_env ty
  | Typing_defs.Tclass ((_, id), _, [key_ty; _])
    when String.equal id SN.Collections.cDict ->
    Tast_env.can_subtype
      tast_env
      key_ty
      (Typing_make_type.arraykey Typing_reason.Rnone)
    || Typing_utils.is_nothing (Tast_env.tast_env_as_typing_env tast_env) key_ty
  | _ -> false

(* Extract position of a dictionary hint. This requires searching for the
   dictionary hint recursively in the case of awaitables. *)
let dict_pos_of_hint hint_opt =
  let rec go (pos, hint) =
    match hint with
    | A.Happly ((_, id), [_; _]) when String.equal id SN.Collections.cDict ->
      pos
    | A.Happly ((_, id), [hint]) when String.equal id SN.Classes.cAwaitable ->
      go hint
    | _ -> failwithpos pos "seeked position of unsuitable parameter hint"
  in
  match hint_opt with
  | Some hint -> go hint
  | None -> failwith "parameter hint is missing"

let add_key_constraint
    (hack_pos : Pos.t)
    (origin : int)
    (env : env)
    entity
    (((_, _, key), ty) : T.expr * Typing_defs.locl_ty)
    ~is_optional : env =
  match entity with
  | Some entity ->
    begin
      match key with
      | A.String str ->
        let key = Typing_defs.TSFlit_str (Pos_or_decl.none, str) in
        let ty = Tast_env.fully_expand env.tast_env ty in
        let constraint_ = Has_static_key (Base, entity, key, ty) in
        let env = Env.add_constraint env { hack_pos; origin; constraint_ } in
        if is_optional then
          let constraint_ = Has_optional_key (entity, key) in
          Env.add_constraint env { hack_pos; origin; constraint_ }
        else
          env
      | _ ->
        let constraint_ = Has_dynamic_key entity in
        Env.add_constraint env { hack_pos; origin; constraint_ }
    end
  | None -> env

let redirect ~pos ~origin (env : env) (entity_ : entity_) : env * entity_ =
  let var = Env.fresh_var () in
  let constraint_ = Subsets (entity_, var) in
  let decorated_constraint = { hack_pos = pos; origin; constraint_ } in
  let env = Env.add_constraint env decorated_constraint in
  (env, var)

let rec assign
    (pos : Pos.t)
    (origin : int)
    (env : env)
    ((_, lhs_pos, lval) : T.expr)
    (rhs : entity)
    (ty_rhs : Typing_defs.locl_ty) : env =
  match lval with
  | A.Lvar (_, lid) -> Env.set_local env lid rhs
  | A.Array_get ((_, _, A.Lvar (_, lid)), Some ix) ->
    let entity = Env.get_local env lid in
    begin
      match entity with
      | Some entity_ ->
        let (env, current_assignment) = redirect ~pos ~origin env entity_ in
        let env =
          add_key_constraint
            pos
            origin
            env
            (Some current_assignment)
            (ix, ty_rhs)
            ~is_optional:false
        in
        (* Handle copy-on-write by creating a variable indirection *)
        let (env, var) = redirect ~pos ~origin env current_assignment in
        Env.set_local env lid (Some var)
      | None ->
        (* We might end up here as a result of deadcode, such as a dictionary
           assignment after an unconditional break in a loop. In this
           situation, it is not meaningful to report a candidate. *)
        env
    end
  | _ -> failwithpos lhs_pos ("Unsupported lvalue: " ^ Utils.expr_name lval)

and expr_ (env : env) ((ty, pos, e) : T.expr) : env * entity =
  let decorate ~origin constraint_ = { hack_pos = pos; origin; constraint_ } in
  match e with
  | A.Int _
  | A.Float _
  | A.String _
  | A.True
  | A.False ->
    (env, None)
  | A.Darray (_, key_value_pairs)
  | A.KeyValCollection (A.Dict, _, key_value_pairs) ->
    let entity_ = Literal pos in
    let entity = Some entity_ in
    let constraint_ = decorate ~origin:__LINE__ @@ Marks (Allocation, pos) in
    let env = Env.add_constraint env constraint_ in
    let add_key_constraint env (key, ((ty, _, _) as value)) : env =
      let (env, _key_entity) = expr_ env key in
      let (env, _val_entity) = expr_ env value in
      add_key_constraint pos __LINE__ env entity (key, ty) ~is_optional:false
    in
    let env = List.fold ~init:env ~f:add_key_constraint key_value_pairs in
    (env, entity)
  | A.Array_get (base, Some ix) ->
    let (env, entity_exp) = expr_ env base in
    let (env, _entity_ix) = expr_ env ix in
    let env =
      add_key_constraint pos __LINE__ env entity_exp (ix, ty) ~is_optional:false
    in
    (env, None)
  | A.Lvar (_, lid) ->
    let entity = Env.get_local env lid in
    (env, entity)
  | A.Binop (Ast_defs.Eq None, e1, ((ty_rhs, _, _) as e2)) ->
    let (env, entity_rhs) = expr_ env e2 in
    let env = assign pos __LINE__ env e1 entity_rhs ty_rhs in
    (env, None)
  | A.Call ((_, _, A.Id (_, idx)), _targs, args, _unpacked)
    when String.equal idx SN.FB.idx ->
    (* Currently treating idx expressions with and without default value in the same way.
       Essentially following the case for A.Array_get after extracting the right data. *)
    begin
      match args with
      | [(_, base); (_, ix)]
      | [(_, base); (_, ix); _] ->
        let (env, entity_exp) = expr_ env base in
        let (env, _entity_ix) = expr_ env ix in
        let env =
          add_key_constraint
            pos
            __LINE__
            env
            entity_exp
            (ix, ty)
            ~is_optional:true
        in
        (env, None)
      | _ -> failwithpos pos ("Unsupported idx expression: " ^ Utils.expr_name e)
    end
  | A.Call ((_, _, A.Id (_, f_id)), _targs, args, _unpacked) ->
    let expr_arg arg_idx env (_param_kind, ((_ty, pos, _exp) as arg)) =
      let (env, arg_entity) = expr_ env arg in
      match arg_entity with
      | Some arg_entity_ ->
        let env =
          if String.equal f_id SN.Hips.inspect then
            let constraint_ = decorate ~origin:__LINE__ @@ Marks (Debug, pos) in
            Env.add_constraint env constraint_
          else
            let inter_constraint_ =
              decorate ~origin:__LINE__
              @@ HT.Arg ((f_id, arg_idx, pos), arg_entity_)
            in
            Env.add_inter_constraint env inter_constraint_
        in
        (* TODO(T128046165) Generate and add inter-procedural constraints *)
        let new_entity_ = Literal pos in
        let env =
          when_tast_check env.tast_env ~default:env @@ fun () ->
          let constraint_ =
            decorate ~origin:__LINE__ @@ Has_dynamic_key new_entity_
          in
          Env.add_constraint env constraint_
        in
        let constraint_ =
          decorate ~origin:__LINE__ @@ Subsets (arg_entity_, new_entity_)
        in
        let env = Env.add_constraint env constraint_ in
        env
      | None -> env
    in
    let env = List.foldi ~f:expr_arg ~init:env args in
    (env, None)
  | A.Eif (cond, Some then_expr, else_expr) ->
    let (parent_env, _cond_entity) = expr_ env cond in
    let base_env = Env.reset_constraints parent_env in
    let (then_env, then_entity) = expr_ base_env then_expr in
    let (else_env, else_entity) = expr_ base_env else_expr in
    let env = Env.union ~pos ~origin:__LINE__ parent_env then_env else_env in
    (* Create a join point entity. It is pretty much Option.marge except that
       that function doesn't allow threading state (`env`) through *)
    let (env, entity) =
      match (then_entity, else_entity) with
      | (Some then_entity_, Some else_entity_) ->
        let (env, join) =
          join ~pos ~origin:__LINE__ env then_entity_ else_entity_
        in
        (env, Some join)
      | (None, Some _) -> (env, else_entity)
      | (_, _) -> (env, then_entity)
    in
    (env, entity)
  | A.Eif (cond, None, else_expr) ->
    let (env, cond_entity) = expr_ env cond in
    let (env, else_entity) = expr_ env else_expr in
    (* Create a join point entity. It is pretty much Option.marge except that
       that function doesn't allow threading state (`env`) through *)
    let (env, entity) =
      match (cond_entity, else_entity) with
      | (Some then_entity_, Some else_entity_) ->
        let (env, join) =
          join ~pos ~origin:__LINE__ env then_entity_ else_entity_
        in
        (env, Some join)
      | (None, Some _) -> (env, else_entity)
      | (_, _) -> (env, cond_entity)
    in
    (env, entity)
  | A.Await e -> expr_ env e
  | A.As (e, _ty, _) -> expr_ env e
  | A.Is (e, _ty) ->
    (* `is` expressions always evaluate to bools, so we discard the entity. *)
    let (env, _) = expr_ env e in
    (env, None)
  | A.Unop
      ( ( Ast_defs.Utild | Ast_defs.Unot | Ast_defs.Uplus | Ast_defs.Uminus
        | Ast_defs.Uincr | Ast_defs.Udecr | Ast_defs.Upincr | Ast_defs.Updecr
        | Ast_defs.Usilence ),
        e1 ) ->
    (* Adding support for unary operations *)
    let (env, _) = expr_ env e1 in
    (env, None)
  | A.Binop
      ( ( Ast_defs.Plus | Ast_defs.Minus | Ast_defs.Star | Ast_defs.Slash
        | Ast_defs.Eqeq | Ast_defs.Eqeqeq | Ast_defs.Starstar | Ast_defs.Diff
        | Ast_defs.Diff2 | Ast_defs.Ampamp | Ast_defs.Barbar | Ast_defs.Lt
        | Ast_defs.Lte | Ast_defs.Gt | Ast_defs.Gte | Ast_defs.Dot
        | Ast_defs.Amp | Ast_defs.Bar | Ast_defs.Ltlt | Ast_defs.Gtgt
        | Ast_defs.Percent | Ast_defs.Xor | Ast_defs.Cmp ),
        e1,
        e2 ) ->
    (* Adding support for binary operations. Currently not covering "Ast_defs.Eq Some _" and "Ast_defs.QuestionQuestion"  *)
    let (env, _) = expr_ env e1 in
    let (env, _) = expr_ env e2 in
    (env, None)
  | _ -> failwithpos pos ("Unsupported expression: " ^ Utils.expr_name e)

let expr (env : env) (e : T.expr) : env = expr_ env e |> fst

let rec switch
    ~pos
    (parent_locals : lenv)
    (env : env)
    (cases : ('ex, 'en) A.case list)
    (dfl : ('ex, 'en) A.default_case option) : env =
  let initialize_next_cont env =
    let env = Env.restore_conts_from env ~from:parent_locals [Cont.Next] in
    let env =
      Env.update_next_from_conts
        ~pos
        ~origin:__LINE__
        env
        [Cont.Next; Cont.Fallthrough]
    in
    Env.drop_cont env Cont.Fallthrough
  in
  let handle_case env (e, b) =
    let env = initialize_next_cont env in
    let env = expr env e in
    block env b
  in
  let handle_default_case env dfl =
    dfl
    |> Option.fold ~init:env ~f:(fun env (_, b) ->
           let env = initialize_next_cont env in
           block env b)
  in
  let env = List.fold ~init:env ~f:handle_case cases in
  let env = handle_default_case env dfl in
  env

and stmt (env : env) ((pos, stmt) : T.stmt) : env =
  let decorate ~origin constraint_ = { hack_pos = pos; origin; constraint_ } in
  match stmt with
  | A.Expr e -> expr env e
  | A.Return None -> env
  | A.Return (Some e) ->
    let (env, entity) = expr_ env e in
    begin
      match (entity, env.return) with
      | (Some entity_, Some return_) ->
        let constraint_ = Subsets (entity_, return_) in
        let decorated_constraint = decorate ~origin:__LINE__ constraint_ in
        let env = Env.add_constraint env decorated_constraint in
        env
      | _ -> env
    end
  | A.If (cond, then_bl, else_bl) ->
    let parent_env = expr env cond in
    let base_env = Env.reset_constraints parent_env in
    let then_env = block base_env then_bl in
    let else_env = block base_env else_bl in
    Env.union ~pos ~origin:__LINE__ parent_env then_env else_env
  | A.Switch (cond, cases, dfl) ->
    let env = expr env cond in
    (* NB: A 'continue' inside a 'switch' block is equivalent to a 'break'.
     * See the note in
     * http://php.net/manual/en/control-structures.continue.php *)
    Env.stash_and_do env [Cont.Continue; Cont.Break] @@ fun env ->
    let parent_locals = env.lenv in
    let env = switch ~pos parent_locals env cases dfl in
    Env.update_next_from_conts
      ~pos
      ~origin:__LINE__
      env
      [Cont.Continue; Cont.Break; Cont.Next]
  | A.Fallthrough ->
    Env.move_and_merge_next_in_cont ~pos ~origin:__LINE__ env Cont.Fallthrough
  | A.Continue ->
    Env.move_and_merge_next_in_cont ~pos ~origin:__LINE__ env Cont.Continue
  | A.Break ->
    Env.move_and_merge_next_in_cont ~pos ~origin:__LINE__ env Cont.Break
  | A.While (cond, bl) ->
    Env.stash_and_do env [Cont.Continue; Cont.Break] @@ fun env ->
    let env =
      Env.save_and_merge_next_in_cont ~pos ~origin:__LINE__ env Cont.Continue
    in
    let env_before_iteration = Env.refresh ~pos ~origin:__LINE__ env in
    let env_after_iteration =
      let env = expr env_before_iteration cond in
      let env = block env bl in
      env
    in
    let env =
      Env.loop_continuation
        ~pos
        ~origin:__LINE__
        Cont.Next
        ~env_before_iteration
        ~env_after_iteration
    in
    let env =
      Env.update_next_from_conts
        ~pos
        ~origin:__LINE__
        env
        [Cont.Continue; Cont.Next]
    in
    let env = expr env cond in
    let env =
      Env.update_next_from_conts
        ~pos
        ~origin:__LINE__
        env
        [Cont.Break; Cont.Next]
    in
    env
  | A.Block statements -> block env statements
  | A.Noop
  | A.AssertEnv _
  | A.Markup _ ->
    env
  | _ -> failwithpos pos ("Unsupported statement: " ^ Utils.stmt_name stmt)

and block (env : env) : T.block -> env = List.fold ~init:env ~f:stmt

let decl_hint kind tast_env ((ty, hint) : T.type_hint) :
    constraint_ decorated list * entity =
  if is_suitable_target_ty tast_env ty then
    let hint_pos = dict_pos_of_hint hint in
    let entity_ =
      match kind with
      | `Parameter (id, idx) -> Inter (HT.Param (id, idx, hint_pos))
      | `Return -> Literal hint_pos
    in
    let kind =
      match kind with
      | `Parameter _ -> Parameter
      | `Return -> Return
    in
    let decorate ~origin constraint_ =
      { hack_pos = hint_pos; origin; constraint_ }
    in
    let marker_constraint =
      decorate ~origin:__LINE__ @@ Marks (kind, hint_pos)
    in
    let constraints = [marker_constraint] in
    let constraints =
      when_tast_check tast_env ~default:constraints @@ fun () ->
      let invalidation_constraint =
        decorate ~origin:__LINE__ @@ Has_dynamic_key entity_
      in
      invalidation_constraint :: constraints
    in
    (constraints, Some entity_)
  else
    ([], None)

let init_params id tast_env (params : T.fun_param list) :
    constraint_ decorated list * entity LMap.t =
  let add_param
      idx
      (constraints, lmap)
      A.{ param_name; param_type_hint; param_is_variadic; _ } =
    if param_is_variadic then
      (* TODO(T125878781): Handle variadic paramseters *)
      (constraints, lmap)
    else
      let (new_constraints, entity) =
        decl_hint (`Parameter (id, idx)) tast_env param_type_hint
      in
      let param_lid = Local_id.make_unscoped param_name in
      let lmap = LMap.add param_lid entity lmap in
      let constraints = new_constraints @ constraints in
      (constraints, lmap)
  in
  List.foldi ~f:add_param ~init:([], LMap.empty) params

let callable id tast_env params ~return body : decorated_constraints =
  let (param_constraints, param_env) = init_params id tast_env params in
  let (return_constraints, return) = decl_hint `Return tast_env return in
  let constraints = return_constraints @ param_constraints in
  let env = Env.init tast_env constraints [] param_env ~return in
  let env = block env body.A.fb_ast in
  (env.constraints, env.inter_constraints)

let program (ctx : Provider_context.t) (tast : Tast.program) :
    decorated_constraints SMap.t =
  let def (def : T.def) : (string * decorated_constraints) list =
    let tast_env = Tast_env.def_env ctx def in
    match def with
    | A.Fun fd ->
      let A.{ f_body; f_name = (_, id); f_params; f_ret; _ } = fd.A.fd_fun in
      [(id, callable id tast_env f_params ~return:f_ret f_body)]
    | A.Class A.{ c_methods; c_name = (_, class_name); _ } ->
      let handle_method
          A.{ m_body; m_name = (_, method_name); m_params; m_ret; _ } =
        let id = class_name ^ "::" ^ method_name in
        (id, callable id tast_env m_params ~return:m_ret m_body)
      in
      List.map ~f:handle_method c_methods
    | _ -> failwith "A definition is not yet handled"
  in
  List.concat_map ~f:def tast |> SMap.of_list
