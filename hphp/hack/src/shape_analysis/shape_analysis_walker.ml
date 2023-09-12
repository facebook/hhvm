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

(* In local mode, we deliberately poison inter-procedural entities to prevent
   the solver from yielding results that need access to other entities (e.g.,
   other functions, class definitions, constants, etc.).

   Currently, local mode is enabled only when we the shape analysis logger is
   enabled.
*)
let when_local_mode mode ~default f =
  match mode with
  | Local -> f ()
  | Global -> default

let dynamic_when_local ~origin pos env entity_ =
  when_local_mode env.mode ~default:env @@ fun () ->
  let constraint_ =
    { hack_pos = pos; origin; constraint_ = Has_dynamic_key entity_ }
  in
  Env.add_constraint env constraint_

let dynamic_always ~origin pos env entity_ =
  let constraint_ =
    { hack_pos = pos; origin; constraint_ = Has_dynamic_key entity_ }
  in
  Env.add_constraint env constraint_

let failwithpos pos msg =
  raise
  @@ Shape_analysis_exn (Error.mk @@ Format.asprintf "%a: %s" Pos.pp pos msg)

let not_yet_supported (env : env) pos msg =
  let msg = Error.mk @@ Format.asprintf "%a: Unsupported %s" Pos.pp pos msg in
  Env.add_error env msg

let failwith = failwithpos Pos.none

let pos_of_hint hint_opt =
  match hint_opt with
  | Some (pos, _hint) -> pos
  | None -> failwith "parameter hint is missing"

let might_be_dict tast_env ty =
  let open Typing_make_type in
  let open Typing_reason in
  let mixed = mixed Rnone in
  let dict_top = dict Rnone mixed mixed in
  let awaitable_dict_top = awaitable Rnone dict_top in
  let nothing = nothing Rnone in
  let dict_bottom = dict Rnone nothing nothing in
  let awaitable_dict_bottom = awaitable Rnone dict_bottom in
  let typing_env = Tast_env.tast_env_as_typing_env tast_env in
  let is_type_disjoint = Typing_subtype.is_type_disjoint typing_env in
  not
  @@ (is_type_disjoint ty dict_top
     && is_type_disjoint ty dict_bottom
     && is_type_disjoint ty awaitable_dict_top
     && is_type_disjoint ty awaitable_dict_bottom)

let is_dict tast_env ty =
  let open Typing_make_type in
  let open Typing_reason in
  let mixed = mixed Rnone in
  let dict_top = dict Rnone mixed mixed in
  let awaitable_dict_top = awaitable Rnone dict_top in
  let is_sub_type = Tast_env.is_sub_type tast_env in
  is_sub_type ty dict_top || is_sub_type ty awaitable_dict_top

let is_cow tast_env ty =
  let open Typing_make_type in
  let open Typing_reason in
  let mixed = mixed Rnone in
  let dict_bottom = dict Rnone mixed mixed in
  let vec_bottom = vec Rnone mixed in
  let keyset_bottom = keyset Rnone mixed in
  let cow_ty =
    Typing_make_type.union
      Typing_reason.Rnone
      [dict_bottom; vec_bottom; keyset_bottom]
  in
  Tast_env.is_sub_type tast_env ty cow_ty

let disjoint_from_traversable tast_env ty =
  let open Typing_make_type in
  let open Typing_reason in
  let mixed = mixed Rnone in
  let traversable_top = traversable Rnone mixed in
  let typing_env = Tast_env.tast_env_as_typing_env tast_env in
  Typing_subtype.is_type_disjoint typing_env traversable_top ty

let any_shape_can_flow tast_env ty =
  let open Typing_make_type in
  let open Typing_reason in
  let shape_top = open_shape Rnone Typing_defs.TShapeMap.empty in
  Tast_env.is_sub_type tast_env shape_top ty

let class_name_of_class_id pos tenv class_id =
  let open Aast in
  match class_id with
  | CIparent -> Tast_env.get_parent_id tenv
  | CIself -> Tast_env.get_self_id tenv
  | CIstatic -> Tast_env.get_self_id tenv
  | CIexpr (_, _, Lvar (_, _)) ->
    (* TODO(T135268910): handle `classname` / `new $c` *)
    None
  | CIexpr (_, _, e) ->
    failwithpos pos
    @@ Printf.sprintf
         "Unexpected class name expression: %s"
         (Aast_names_utils.expr_name e)
  | CI (_, id) -> Some id

let to_marshallable_ty = Wipe_type_reason.wipe

let add_key_constraint
    ~(pos : Pos.t)
    ~(origin : int)
    ~certainty
    ~variety
    ~base_ty
    (((_, _, key), ty) : T.expr * Typing_defs.locl_ty)
    (env : env)
    entity : env =
  let add_key key =
    let ty = ty |> Tast_env.fully_expand env.tast_env |> to_marshallable_ty in
    let add_static_key env variety =
      let constraint_ = Static_key (variety, certainty, entity, key, ty) in
      Env.add_constraint env { hack_pos = pos; origin; constraint_ }
    in
    List.fold ~f:add_static_key variety ~init:env
  in
  if is_dict env.tast_env base_ty then
    match key with
    | A.String str -> add_key (Typing_defs.TSFlit_str (Pos_or_decl.none, str))
    | A.Class_const ((_, _, A.CI (_, class_name)), (_, const_name)) ->
      add_key
        (Typing_defs.TSFclass_const
           ((Pos_or_decl.none, class_name), (Pos_or_decl.none, const_name)))
    | _ ->
      let constraint_ = Has_dynamic_key entity in
      Env.add_constraint env { hack_pos = pos; origin; constraint_ }
  else
    env

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
    ((lhs_ty, lhs_pos, lval) : T.expr)
    (rhs : entity)
    (ty_rhs : Typing_defs.locl_ty) : env =
  let decorate origin constraint_ = { hack_pos = pos; origin; constraint_ } in
  match lval with
  | A.Lvar (_, lid) -> Env.set_local env lid rhs
  | A.Array_get ((ty, _, A.Lvar (_, lid)), ix_opt) ->
    let entity = Env.get_local env lid in
    begin
      match entity with
      | Some entity_ ->
        let (env, entity_) =
          if is_cow env.tast_env ty then
            (* Handle copy-on-write by creating a variable indirection *)
            let (env, entity_) = redirect ~pos ~origin env entity_ in
            let env = Env.set_local env lid (Some entity_) in
            (env, entity_)
          else
            (env, entity_)
        in
        let env =
          Option.fold ~init:env ix_opt ~f:(fun env ix ->
              add_key_constraint
                ~pos
                ~origin
                ~certainty:Definite
                ~variety:[Has]
                ~base_ty:ty
                (ix, ty_rhs)
                env
                entity_)
        in
        let env =
          Option.fold ~init:env rhs ~f:(fun env rhs_entity_ ->
              decorate __LINE__ (Subsets (rhs_entity_, entity_))
              |> Env.add_constraint env)
        in
        env
      | None ->
        (* We might end up here as a result of deadcode, such as a dictionary
           assignment after an unconditional break in a loop. In this
           situation, it is not meaningful to report a candidate. *)
        env
    end
  | A.Class_get (_, _, _)
  | A.Obj_get (_, _, _, _) ->
    (* Imprecise local handling so that false positives are invalidated *)
    let env =
      if any_shape_can_flow env.tast_env lhs_ty then
        env
      else
        Option.fold ~init:env ~f:(dynamic_when_local ~origin:__LINE__ pos) rhs
    in
    not_yet_supported env lhs_pos ("lvalue: " ^ Utils.expr_name lval)
  | _ -> not_yet_supported env lhs_pos ("lvalue: " ^ Utils.expr_name lval)

and expr_ (env : env) ((ty, pos, e) : T.expr) : env * entity =
  let decorate ~origin constraint_ = { hack_pos = pos; origin; constraint_ } in
  let mode = env.mode in
  let dynamic_when_local = dynamic_when_local pos in
  let dynamic_always = dynamic_always pos in
  match e with
  | A.Int _
  | A.Float _
  | A.String _
  | A.True
  | A.False
  | A.Null ->
    (env, None)
  | A.Tuple values
  | A.Varray (_, values)
  | A.ValCollection (_, _, values) ->
    (* TODO(T131709581): This is an approximation where we identify the the
       surrounding collection with whatever might be inside. *)
    let collection_entity_ = Env.fresh_var () in
    let collection_entity = Some collection_entity_ in
    let add_value env value =
      let (env, value_entity) = expr_ env value in
      Option.fold ~init:env value_entity ~f:(fun env value_entity_ ->
          let constraint_ =
            decorate ~origin:__LINE__
            @@ Subsets (value_entity_, collection_entity_)
          in
          Env.add_constraint env constraint_)
    in
    let env = List.fold ~init:env ~f:add_value values in
    (env, collection_entity)
  | A.Darray (_, key_value_pairs)
  | A.KeyValCollection ((_, A.Dict), _, key_value_pairs) ->
    let entity_ = Literal pos in
    let entity = Some entity_ in
    let constraint_ = decorate ~origin:__LINE__ @@ Marks (Allocation, pos) in
    let env = Env.add_constraint env constraint_ in
    let handle_key_value env (key, ((val_ty, _, _) as value)) : env =
      let (env, _key_entity) = expr_ env key in
      let (env, val_entity) = expr_ env value in
      let env =
        (* TODO(T131709581): This is an approximation where we identify the the
           surrounding collection with whatever might be inside. *)
        Option.fold ~init:env val_entity ~f:(fun env val_entity_ ->
            decorate ~origin:__LINE__ @@ Subsets (val_entity_, entity_)
            |> Env.add_constraint env)
      in
      Option.fold
        ~init:env
        ~f:
          (add_key_constraint
             ~pos
             ~origin:__LINE__
             ~certainty:Definite
             ~variety:[Has]
             ~base_ty:ty
             (key, val_ty))
        entity
    in
    let env = List.fold ~init:env ~f:handle_key_value key_value_pairs in
    (env, entity)
  | A.KeyValCollection (_, _, key_value_pairs) ->
    (* TODO(T131709581): This is an approximation where we identify the the
       surrounding collection with whatever might be inside. *)
    let entity_ = Env.fresh_var () in
    let entity = Some entity_ in
    let handle_key_value env (key, value) : env =
      let (env, _key_entity) = expr_ env key in
      let (env, val_entity) = expr_ env value in
      Option.fold ~init:env val_entity ~f:(fun env val_entity_ ->
          decorate ~origin:__LINE__ @@ Subsets (val_entity_, entity_)
          |> Env.add_constraint env)
    in
    let env = List.fold ~init:env ~f:handle_key_value key_value_pairs in
    (env, entity)
  | A.Array_get (((base_ty, _, _) as base), Some ix) ->
    let (env, base_entity) = expr_ env base in
    let (env, _entity_ix) = expr_ env ix in
    let env =
      Option.fold
        ~init:env
        ~f:
          (add_key_constraint
             ~pos
             ~origin:__LINE__
             ~certainty:Definite
             ~variety:[Has; Needs]
               (*TODO(T136668856): consider only generating a `Needs` constraint here, and propagating `Needs` forward *)
             ~base_ty
             (ix, ty))
        base_entity
    in
    (* TODO(T131709581): Returning the collection is an approximation where we
       identify the the surrounding collection with whatever might be inside. *)
    let entity =
      if disjoint_from_traversable env.tast_env ty then
        None
      else
        base_entity
    in
    (env, entity)
  | A.Lvar (_, lid) ->
    let entity = Env.get_local env lid in
    (env, entity)
  | A.(Binop { bop = Ast_defs.Eq None; lhs = e1; rhs = (ty_rhs, _, _) as e2 })
    ->
    let (env, entity_rhs) = expr_ env e2 in
    let env = assign pos __LINE__ env e1 entity_rhs ty_rhs in
    (env, None)
  | A.(Call { func = (_, _, Id (_, idx)); args; _ })
    when String.equal idx SN.FB.idx -> begin
    (* Currently treating idx expressions with and without default value in the same way.
       Essentially following the case for A.Array_get after extracting the right data. *)
    match args with
    | [(_, ((base_ty, _, _) as base)); (_, ix)]
    | [(_, ((base_ty, _, _) as base)); (_, ix); _] ->
      let (env, entity_exp) = expr_ env base in
      let (env, _entity_ix) = expr_ env ix in
      let env =
        Option.fold
          ~init:env
          ~f:
            (add_key_constraint
               ~pos
               ~origin:__LINE__
               ~certainty:Maybe
               ~variety:[Has; Needs]
               ~base_ty
               (ix, ty))
          entity_exp
      in
      (env, None)
    | _ ->
      let env =
        not_yet_supported env pos ("idx expression: " ^ Utils.expr_name e)
      in
      (env, None)
  end
  | A.New (class_id, targs, args, unpacked_arg, _instantiation) ->
    (* What is new object creation but a call to a static method call to a
       class constructor? *)
    let func = (ty, pos, A.Class_const (class_id, (pos, "__construct"))) in
    let args = List.map ~f:(fun arg -> (Ast_defs.Pnormal, arg)) args in
    let call_expr = (ty, pos, A.(Call { func; targs; args; unpacked_arg })) in
    expr_ env call_expr
  | A.(Call { func = (base_ty, _, lhs) as base; args; unpacked_arg; _ }) ->
    let lhs_is_obj_get =
      match lhs with
      | A.Obj_get _ -> true
      | _ -> false
    in
    let param_tys =
      match Typing_defs.get_node base_ty with
      | Typing_defs.Tfun ft ->
        List.map
          ~f:(fun param -> param.Typing_defs.fp_type.Typing_defs.et_type)
          ft.Typing_defs.ft_params
      | _ -> []
    in
    let handle_arg arg_idx env (param_kind, ((_ty, pos, _exp) as arg)) =
      let (env, arg_entity) = expr_ env arg in
      let param_ty_opt = List.nth param_tys arg_idx in
      let env =
        let be_conservative =
          Option.value_map
            ~default:true
            param_ty_opt
            ~f:(Fn.non @@ any_shape_can_flow env.tast_env)
        in
        if be_conservative then
          let fold_env f = Option.fold ~init:env arg_entity ~f in
          if lhs_is_obj_get then
            (* Because HIPS doesn't know about objects yet (T139375375). Note that this isn't
               as conservative as it could be because of function and method pointers *)
            fold_env @@ dynamic_always ~origin:__LINE__
          else
            (* During local mode we cannot know what happens to the entity, so we
               conservatively assume there is a dynamic access. *)
            fold_env @@ dynamic_when_local ~origin:__LINE__
        else
          env
      in
      let (env, arg_entity) =
        match param_kind with
        | Ast_defs.Pinout _ -> begin
          (* When we have an inout parameter, we sever the connection between
             what goes into the parameter and what comes out.

             Once again in local mode, we do not know what happened to the
             dictionary, so we assume it was dynamically accessed. *)
          match arg with
          | (_, _, A.Lvar (_, lid)) ->
            let arg_entity_ = Env.fresh_var () in
            let arg_entity = Some arg_entity_ in
            let env = Env.set_local env lid arg_entity in
            let env = dynamic_when_local ~origin:__LINE__ env arg_entity_ in
            (env, arg_entity)
          | (_, pos, _) ->
            let env = not_yet_supported env pos "inout argument" in
            (env, arg_entity)
        end
        | Ast_defs.Pnormal -> (env, arg_entity)
      in
      match arg_entity with
      | Some arg_entity_ -> begin
        match base with
        | (_, _, A.Id (_, f_id)) when String.equal f_id SN.Hips.inspect ->
          let constraint_ = decorate ~origin:__LINE__ @@ Marks (Debug, pos) in
          let env = Env.add_constraint env constraint_ in
          let constraint_ =
            decorate ~origin:__LINE__ @@ Subsets (arg_entity_, Literal pos)
          in
          let env = Env.add_constraint env constraint_ in
          env
        | (_, _, A.Id (_, f_id)) ->
          (* TODO: inout parameters need special treatment inter-procedurally *)
          let inter_constraint_ =
            decorate ~origin:__LINE__
            @@ HT.ArgLike (((pos, f_id), HT.Index arg_idx), arg_entity_)
          in
          Env.add_inter_constraint env inter_constraint_
        | _ -> env
      end
      | None -> env
    in
    (* Handle the bast of the call *)
    let (env, _base_entity) =
      match base with
      | (_, _, A.Id _) ->
        (* Use of identifiers inside function calls is not compositional.
           This could be cleaned up... *)
        (env, None)
      | _ -> expr_ env base
    in
    (* Handle the vanilla arguments *)
    let env = List.foldi ~f:handle_arg ~init:env args in
    (* Handle the unpaced argument (e.g., ...$args) *)
    let env =
      Option.value_map
        ~default:env
        ~f:(fun exp ->
          let idx = List.length args + 1 in
          handle_arg idx env (Ast_defs.Pnormal, exp))
        unpacked_arg
    in
    (* Handle the return. *)
    let return_entity = Env.fresh_var () in
    let env =
      match base with
      (* TODO: handle function calls through variables *)
      | (_, _, A.Id (_, f_id)) when not @@ String.equal f_id SN.Hips.inspect ->
        let constraint_ =
          decorate ~origin:__LINE__
          @@ HT.ArgLike (((pos, f_id), HT.Return), return_entity)
        in
        Env.add_inter_constraint env constraint_
      | _ -> env
    in
    let env =
      when_local_mode mode ~default:env @@ fun () ->
      let constraint_ =
        decorate ~origin:__LINE__ @@ Has_dynamic_key return_entity
      in
      Env.add_constraint env constraint_
    in
    (env, Some return_entity)
  | A.Await e -> expr_ env e
  | A.As (e, _ty, _) -> expr_ env e
  | A.Is (e, _ty) ->
    (* `is` expressions always evaluate to bools, so we discard the entity. *)
    let (env, _) = expr_ env e in
    (env, None)
  | A.Unop
      ( Ast_defs.(
          ( Utild | Unot | Uplus | Uminus | Uincr | Udecr | Upincr | Updecr
          | Usilence )),
        e1 ) ->
    (* Adding support for unary operations *)
    let (env, _) = expr_ env e1 in
    (env, None)
  | A.Eif (cond, then_expr_opt, else_expr) ->
    eif ~pos env cond then_expr_opt else_expr
  | A.(
      Binop
        {
          bop = Ast_defs.QuestionQuestion;
          lhs = nullable_expr;
          rhs = else_expr;
        }) ->
    eif ~pos env nullable_expr None else_expr
  | A.(
      Binop
        {
          bop =
            Ast_defs.(
              ( Plus | Minus | Star | Slash | Eqeq | Eqeqeq | Starstar | Diff
              | Diff2 | Ampamp | Barbar | Lt | Lte | Gt | Gte | Dot | Amp | Bar
              | Ltlt | Gtgt | Percent | Xor | Cmp ));
          lhs = e1;
          rhs = e2;
        }) ->
    (* Adding support for binary operations. Currently not covering
       "Ast_defs.Eq Some _" *)
    let (env, _) = expr_ env e1 in
    let (env, _) = expr_ env e2 in
    (env, None)
  | A.Id name ->
    let entity__ =
      {
        HT.ident_pos = fst name;
        HT.class_name_opt = None;
        HT.const_name = snd name;
      }
    in
    let entity_ = Inter (HT.ConstantIdentifier entity__) in
    let env = dynamic_when_local ~origin:__LINE__ env entity_ in
    let constr_ =
      {
        hack_pos = fst name;
        origin = __LINE__;
        constraint_ = HT.ConstantIdentifier entity__;
      }
    in
    let env = Env.add_inter_constraint env constr_ in
    (env, Some entity_)
  | A.Class_const ((_, ident_pos, class_id), (_, const_name)) ->
    let class_name_opt =
      class_name_of_class_id ident_pos env.tast_env class_id
    in
    let entity__ = { HT.ident_pos; HT.class_name_opt; HT.const_name } in
    let entity_ = Inter (HT.ConstantIdentifier entity__) in
    let env = dynamic_when_local ~origin:__LINE__ env entity_ in
    let constr_ =
      {
        hack_pos = ident_pos;
        origin = __LINE__;
        constraint_ = HT.ConstantIdentifier entity__;
      }
    in
    let env = Env.add_inter_constraint env constr_ in
    (env, Some entity_)
  | A.Class_get (_, _, _)
  | A.Obj_get (_, _, _, _) ->
    let env = not_yet_supported env pos ("expression: " ^ Utils.expr_name e) in
    (* Imprecise local handling so that false positives are invalidated *)
    when_local_mode mode ~default:(env, None) @@ fun () ->
    let entity_ = Env.fresh_var () in
    let constraint_ = decorate ~origin:__LINE__ @@ Has_dynamic_key entity_ in
    let env = Env.add_constraint env constraint_ in
    (env, Some entity_)
  | _ ->
    let env = not_yet_supported env pos ("expression: " ^ Utils.expr_name e) in
    (env, None)

and eif ~pos env cond then_expr_opt else_expr =
  let (cond_env, cond_entity) = expr_ env cond in
  let base_env = Env.reset_constraints cond_env in
  let (then_env, then_entity) =
    match then_expr_opt with
    | Some then_expr ->
      let base_env = Env.refresh ~pos ~origin:__LINE__ base_env in
      expr_ base_env then_expr
    | None -> (cond_env, cond_entity)
  in
  let (else_env, else_entity) =
    let base_env = Env.refresh ~pos ~origin:__LINE__ base_env in
    expr_ base_env else_expr
  in
  let env = Env.union ~pos ~origin:__LINE__ env then_env else_env in
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

let expr (env : env) (e : T.expr) : env = expr_ env e |> fst

let rec switch
    ~pos
    (parent_locals : lenv)
    (env : env)
    (cases : ('ex, 'en) A.case list)
    (dfl : ('ex, 'en) A.default_case option) : env =
  let initialize_next_cont env =
    let env = Env.restore_conts_from env ~from:parent_locals [Cont.Next] in
    let env = Env.refresh ~pos ~origin:__LINE__ env in
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

and foreach_pattern env collection_ent = function
  | A.As_v (_, _, A.Lvar (_, lid))
  | A.As_kv (_, (_, _, A.Lvar (_, lid)))
  | A.Await_as_v (_, (_, _, A.Lvar (_, lid)))
  | A.Await_as_kv (_, _, (_, _, A.Lvar (_, lid))) ->
    Env.set_local env lid collection_ent
  | _ -> env

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
    let then_env =
      let base_env = Env.refresh ~pos ~origin:__LINE__ base_env in
      block base_env then_bl
    in
    let else_env =
      let base_env = Env.refresh ~pos ~origin:__LINE__ base_env in
      block base_env else_bl
    in
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
  | A.Foreach (collection_exp, pattern, bl) ->
    let (env, collection_ent) = expr_ env collection_exp in
    Env.stash_and_do env [Cont.Continue; Cont.Break] @@ fun env ->
    let env =
      Env.save_and_merge_next_in_cont ~pos ~origin:__LINE__ env Cont.Continue
    in
    let env_before_iteration = Env.refresh ~pos ~origin:__LINE__ env in
    let env_after_iteration =
      let env = foreach_pattern env_before_iteration collection_ent pattern in
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
        [Cont.Continue; Cont.Break; Cont.Next]
    in
    env
  | A.Block (_, statements) -> block env statements
  | A.Noop
  | A.AssertEnv _
  | A.Markup _ ->
    env
  | _ -> not_yet_supported env pos ("statement: " ^ Utils.stmt_name stmt)

and block (env : env) : T.block -> env = List.fold ~init:env ~f:stmt

let decl_hint mode kind tast_env ((ty, hint) : T.type_hint) :
    decorated_constraints * entity =
  let hint_pos = pos_of_hint hint in
  let entity_ =
    match kind with
    | `Parameter (id, idx) -> Inter (HT.ParamLike ((hint_pos, id), idx))
    | `Return id -> Inter (HT.ParamLike ((hint_pos, id), HT.Return))
  in
  let decorate ~origin constraint_ =
    { hack_pos = hint_pos; origin; constraint_ }
  in
  let inter_constraints =
    match kind with
    | `Parameter (id, idx) ->
      DecoratedInterConstraintSet.singleton
      @@ decorate ~origin:__LINE__
      @@ HT.ParamLike ((hint_pos, id), idx)
    | `Return id ->
      DecoratedInterConstraintSet.singleton
      @@ decorate ~origin:__LINE__
      @@ HT.ParamLike ((hint_pos, id), HT.Return)
  in
  let kind =
    match kind with
    | `Parameter _ -> Parameter
    | `Return _ -> Return
  in
  let constraints =
    if might_be_dict tast_env ty then
      DecoratedConstraintSet.singleton
      @@ decorate ~origin:__LINE__
      @@ Marks (kind, hint_pos)
    else
      DecoratedConstraintSet.empty
  in
  let constraints =
    when_local_mode mode ~default:constraints @@ fun () ->
    let invalidation_constraint =
      decorate ~origin:__LINE__ @@ Has_dynamic_key entity_
    in
    DecoratedConstraintSet.add invalidation_constraint constraints
  in
  ((constraints, inter_constraints), Some entity_)

let init_params mode id tast_env (params : T.fun_param list) :
    decorated_constraints * entity LMap.t =
  let add_param
      (idx : int)
      ((intra_constraints, inter_constraints), lmap)
      A.{ param_name; param_type_hint; param_is_variadic; _ } =
    if param_is_variadic then
      (* TODO(T125878781): Handle variadic paramseters *)
      ((intra_constraints, inter_constraints), lmap)
    else
      let ((new_intra_constraints, new_inter_constraints), entity) =
        decl_hint mode (`Parameter (id, HT.Index idx)) tast_env param_type_hint
      in
      let param_lid = Local_id.make_unscoped param_name in
      let lmap = LMap.add param_lid entity lmap in
      let intra_constraints =
        DecoratedConstraintSet.union new_intra_constraints intra_constraints
      in
      let inter_constraints =
        DecoratedInterConstraintSet.union
          new_inter_constraints
          inter_constraints
      in
      ((intra_constraints, inter_constraints), lmap)
  in
  List.foldi
    ~f:add_param
    ~init:
      ( (DecoratedConstraintSet.empty, DecoratedInterConstraintSet.empty),
        LMap.empty )
    params

let callable mode id tast_env params ~return body =
  (* TODO(T130457262): inout parameters should have the entity of their final
     binding flow back into them. *)
  let ((param_intra_constraints, param_inter_constraints), param_env) =
    init_params mode id tast_env params
  in
  let ((return_intra_constraints, return_inter_constraints), return) =
    decl_hint mode (`Return id) tast_env return
  in
  let intra_constraints =
    DecoratedConstraintSet.union
      return_intra_constraints
      param_intra_constraints
  in
  let inter_constraints =
    DecoratedInterConstraintSet.union
      return_inter_constraints
      param_inter_constraints
  in
  let env =
    Env.init mode tast_env intra_constraints inter_constraints param_env ~return
  in
  let env = block env body.A.fb_ast in
  ((env.constraints, env.inter_constraints), env.errors)

let marker_constraint_of ~hack_pos (marker_pos : Pos.t) : constraint_ decorated
    =
  { hack_pos; origin = __LINE__; constraint_ = Marks (Constant, marker_pos) }

let constant_constraint_of
    ~hack_pos (constant_pos : Pos.t) (constant_name : string) :
    inter_constraint_ decorated =
  {
    hack_pos;
    origin = __LINE__;
    constraint_ = HT.Constant (constant_pos, constant_name);
  }

let subset_constraint_of
    ~hack_pos (ent1 : entity_) ~constant_pos (constant_name : string) :
    constraint_ decorated =
  {
    hack_pos;
    origin = __LINE__;
    constraint_ =
      Subsets (ent1, Inter (HT.Constant (constant_pos, constant_name)));
  }

let initial_constraint_of ~hack_pos (ent : entity_) :
    inter_constraint_ decorated =
  { hack_pos; origin = __LINE__; constraint_ = HT.ConstantInitial ent }

let program mode (ctx : Provider_context.t) (tast : Tast.program) =
  let def (def : T.def) : (string * (decorated_constraints * Error.t list)) list
      =
    let tast_env = Tast_env.def_env ctx def in
    match def with
    | A.Fun fd ->
      let (_, id) = fd.A.fd_name in
      let A.{ f_body; f_params; f_ret; _ } = fd.A.fd_fun in
      [(id, callable mode id tast_env f_params ~return:f_ret f_body)]
    | A.Class A.{ c_kind = Ast_defs.Cenum; _ } ->
      (* There is nothing to analyse in an enum definition *)
      []
    | A.Class
        A.
          {
            c_methods;
            c_name = (_, class_name);
            c_consts;
            c_extends;
            c_kind =
              Ast_defs.(Cclass Concrete | Cclass Abstract | Cinterface | Ctrait);
            _;
          } ->
      let handle_method
          A.{ m_body; m_name = (_, method_name); m_params; m_ret; _ } =
        let id = class_name ^ "::" ^ method_name in
        (id, callable mode id tast_env m_params ~return:m_ret m_body)
      in
      let handle_constant A.{ cc_type; cc_id; cc_kind; _ } =
        let id = class_name ^ "::" ^ snd cc_id in
        let hint_pos = pos_of_hint cc_type in
        let (env, ent) =
          let empty_env =
            Env.init
              mode
              tast_env
              DecoratedConstraintSet.empty
              DecoratedInterConstraintSet.empty
              ~return:None
              LMap.empty
          in
          match cc_kind with
          | A.CCAbstract initial_expr_opt ->
            (match initial_expr_opt with
            | Some initial_expr -> expr_ empty_env initial_expr
            | None -> (empty_env, None))
          | A.CCConcrete initial_expr -> expr_ empty_env initial_expr
        in
        let marker_constraint =
          marker_constraint_of ~hack_pos:hint_pos hint_pos
        in
        let env = Env.add_constraint env marker_constraint in
        let constant_constraint =
          constant_constraint_of ~hack_pos:(fst cc_id) hint_pos id
        in
        let env = Env.add_inter_constraint env constant_constraint in
        let env =
          match ent with
          | Some ent_ ->
            let subset_constr =
              subset_constraint_of
                ~hack_pos:(fst cc_id)
                ent_
                ~constant_pos:hint_pos
                id
            in
            let initial_constr =
              initial_constraint_of ~hack_pos:(fst cc_id) ent_
            in
            let env = Env.add_constraint env subset_constr in
            Env.add_inter_constraint env initial_constr
          | None -> env
        in
        (id, ((env.constraints, env.inter_constraints), env.errors))
      in
      let handle_extends class_hint =
        match class_hint with
        | (pos, A.Happly (class_id_of_extends, _)) ->
          let extends_constr =
            {
              hack_pos = pos;
              origin = __LINE__;
              constraint_ = HT.ClassExtends class_id_of_extends;
            }
          in
          let empty_env =
            Env.init
              mode
              tast_env
              DecoratedConstraintSet.empty
              DecoratedInterConstraintSet.empty
              ~return:None
              LMap.empty
          in
          let env = Env.add_inter_constraint empty_env extends_constr in
          Some
            (class_name, ((env.constraints, env.inter_constraints), env.errors))
        | _ -> None
      in
      List.map ~f:handle_method c_methods
      @ List.map ~f:handle_constant c_consts
      @ List.filter_map ~f:handle_extends c_extends
    | A.Constant A.{ cst_name; cst_value; cst_type; _ } ->
      let hint_pos = pos_of_hint cst_type in
      let env =
        Env.init
          mode
          tast_env
          DecoratedConstraintSet.empty
          DecoratedInterConstraintSet.empty
          ~return:None
          LMap.empty
      in
      let (env, ent) = expr_ env cst_value in
      let marker_constraint =
        marker_constraint_of ~hack_pos:hint_pos hint_pos
      in
      let env = Env.add_constraint env marker_constraint in
      let constant_constraint =
        constant_constraint_of ~hack_pos:(fst cst_name) hint_pos (snd cst_name)
      in
      let env = Env.add_inter_constraint env constant_constraint in
      let env =
        match ent with
        | Some ent_ ->
          let subset_constr =
            subset_constraint_of
              ~hack_pos:(fst cst_name)
              ent_
              ~constant_pos:hint_pos
              (snd cst_name)
          in
          let initial_constr =
            initial_constraint_of ~hack_pos:(fst cst_name) ent_
          in
          let env = Env.add_constraint env subset_constr in
          Env.add_inter_constraint env initial_constr
        | None -> env
      in
      [(snd cst_name, ((env.constraints, env.inter_constraints), env.errors))]
    | _ -> failwith "A definition is not yet handled"
  in
  List.concat_map ~f:def tast |> SMap.of_list
