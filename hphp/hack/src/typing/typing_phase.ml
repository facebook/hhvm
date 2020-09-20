(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Common
open Typing_defs
open Typing_env_types
open Typing_dependent_type
module Env = Typing_env
module TUtils = Typing_utils
module TGenConstraint = Typing_generic_constraint
module Subst = Decl_subst
module MakeType = Typing_make_type
module Cls = Decl_provider.Class
module KindDefs = Typing_kinding_defs
module Kinding = Typing_kinding

(* Here is the general problem the delayed application of the phase solves.
 * Let's say you have a function that you want to operate generically across
 * phases. In most cases when you do this you can use the 'ty' GADT and locally
 * abstract types to write code in a phase agonistic way.
 *
 *  let yell_any: type a. a ty -> string = fun ty ->
 *    match ty with
 *    | _, Tany -> "Any"
 *    | _ -> ""
 *
 * Now let's add a function that works for all phases, but whose logic is phase
 * dependent. For this we can use 'phase_ty' ADT:
 *
 *  let yell_locl phase_ty =
 *     match phase_ty with
 *     | DeclTy ty -> ""
 *     | LoclTy ty -> "Locl"
 *
 * Let's say you want to write a function that has behavior that works across
 * phases, but needs to invoke a function that is phase dependent. Our options
 * are as follows.
 *
 *  let yell_any_or_locl phase_ty =
 *    let ans = yell_locl phase_ty in
 *    match phase_ty with
 *    | DeclTy ty -> ans ^ (yell_any ty)
 *    | LoclTy ty -> ans ^ (yell_any ty)
 *
 * This would lead to code duplication since we can't generically operate on the
 * underlying 'ty' GADT. If we want to eliminate this code duplication there are
 * two options.
 *
 *  let generic_ty: type a. phase_ty -> a ty = function
 *    | DeclTy ty -> ty
 *    | LoclTy ty -> ty
 *
 *  let yell_any_or_locl phase_ty =
 *    let ans = yell_locl phase_ty in
 *    ans ^ (yell_any (generic_ty phase_ty))
 *
 * generic_ty allows us to extract a generic value which we can use. This
 * approach is limiting because we lose all information about what phase 'a ty
 * is.
 *
 * The other approach is to pass in a function that goes from 'a ty -> phase_ty'
 *
 *  let yell_any_or_locl phase ty =
 *    let ans = yell_locl (phase ty) in
 *    ans ^ (yell_any ty)
 *
 * Here we can use 'ty' generically (without losing information about what phase
 * 'a ty' is), and we rely on the caller passing in an appropriate function that
 * converts into the 'phase_ty' when we need to hop into phase specific code.
 *)
let decl ty = DeclTy ty

let locl ty = LoclTy ty

type method_instantiation = {
  use_pos: Pos.t;
  use_name: string;
  explicit_targs: Tast.targ list;
}

let env_with_self ?pos ?(quiet = false) ?report_cycle env =
  let this_ty = mk (Reason.none, TUtils.this_of (Env.get_self env)) in
  {
    type_expansions =
      begin
        match report_cycle with
        | None -> []
        | Some (pos, id) -> [(true, pos, id)]
      end;
    substs = SMap.empty;
    this_ty;
    from_class = None;
    quiet;
    on_error =
      (match pos with
      | None -> Errors.unify_error
      | Some pos -> Errors.unify_error_at pos);
  }

(*****************************************************************************)
(* Transforms a declaration phase type into a localized type. This performs
 * common operations that are necessary for this operation, specifically:
 *   > Expand newtype/types
 *   > Resolves the "this" type
 *   > Instantiate generics
 *   > ...
 *
 * When keep track of additional information while localizing a type such as
 * what type defs were expanded to detect potentially recursive definitions..
 *)
(*****************************************************************************)

let rec localize ~ety_env env (dty : decl_ty) =
  Typing_log.log_localize ~level:1 dty
  @@
  let tvar_or_localize ~ety_env env r ty ~i =
    if
      GlobalOptions.tco_global_inference env.genv.tcopt
      && (is_any ty || is_tyvar ty)
    then
      Env.new_global_tyvar env ~i r
    else
      localize ~ety_env env ty
  in
  match deref dty with
  | (r, Terr) -> (env, TUtils.terr env r)
  | (r, Tany _) -> (env, mk (r, TUtils.tany env))
  | (r, Tvar _var) -> Env.new_global_tyvar env r
  | (r, ((Tnonnull | Tprim _ | Tdynamic) as x)) -> (env, mk (r, x))
  | (r, Tmixed) -> (env, MakeType.mixed r)
  | (r, Tthis) ->
    let ty =
      map_reason ety_env.this_ty ~f:(function
          | Reason.Rnone -> r
          | Reason.Rexpr_dep_type (_, pos, s) ->
            Reason.Rexpr_dep_type (r, pos, s)
          | reason ->
            if Option.is_some ety_env.from_class then
              reason
            else
              Reason.Rinstantiate (reason, SN.Typehints.this, r))
    in
    let (env, ty) =
      match ety_env.from_class with
      | Some cid -> ExprDepTy.make env cid ty
      | _ -> (env, ty)
    in
    (env, ty)
  | (r, Tarray (ty1, ty2)) ->
    let (env, ty) =
      match (ty1, ty2) with
      | (None, None) ->
        let tk = MakeType.arraykey Reason.(Rvarray_or_darray_key (to_pos r)) in
        let (env, tv) =
          if GlobalOptions.tco_global_inference env.genv.tcopt then
            Env.new_global_tyvar env r
          else
            (env, mk (r, TUtils.tany env))
        in
        (env, Tvarray_or_darray (tk, tv))
      | (Some tv, None) ->
        let (env, tv) = tvar_or_localize ~ety_env env r tv ~i:1 in
        (env, Tvarray tv)
      | (Some tk, Some tv) ->
        let (env, tk) = tvar_or_localize ~ety_env env r tk ~i:0 in
        let (env, tv) = tvar_or_localize ~ety_env env r tv ~i:1 in
        (env, Tdarray (tk, tv))
      | (None, Some _) -> failwith "Invalid array declaration type"
    in
    (env, mk (r, ty))
  | (r, Tdarray (tk, tv)) ->
    let (env, tk) = tvar_or_localize ~ety_env env r tk ~i:0 in
    let (env, tv) = tvar_or_localize ~ety_env env r tv ~i:1 in
    let ty = Tdarray (tk, tv) in
    (env, mk (r, ty))
  | (r, Tvarray tv) ->
    let (env, tv) = tvar_or_localize ~ety_env env r tv ~i:0 in
    let ty = Tvarray tv in
    (env, mk (r, ty))
  | (r, Tvarray_or_darray (tk, tv)) ->
    let (env, tk) = tvar_or_localize ~ety_env env r tk ~i:0 in
    let (env, tv) = tvar_or_localize ~ety_env env r tv ~i:1 in
    (env, MakeType.varray_or_darray r tk tv)
  | (r, Tgeneric (x, targs)) ->
    let localize_tgeneric ?replace_with name r =
      match (targs, replace_with, Env.get_pos_and_kind_of_generic env name) with
      | (_, _, Some (def_pos, kind)) ->
        let arg_kinds : KindDefs.Simple.named_kind list =
          KindDefs.Simple.from_full_kind kind
          |> KindDefs.Simple.get_named_parameter_kinds
        in
        let use_pos = Reason.to_pos r in
        begin
          match
            ( localize_tparams_by_kind
                ~ety_env
                ~def_pos
                ~use_pos
                env
                targs
                arg_kinds,
              replace_with )
          with
          | ((env, _), Some repl_ty) -> (env, mk (r, repl_ty))
          | ((env, locl_tyargs), None) ->
            (env, mk (r, Tgeneric (name, locl_tyargs)))
        end
      | ([], None, None) ->
        (* No kinding info, but also no type arguments. Just return Tgeneric *)
        (env, mk (r, Tgeneric (x, [])))
      | ([], Some repl_ty, None) -> (env, mk (r, repl_ty))
      | (_, _, None) ->
        (* No kinding info, but type arguments given. We don't know the kinds of the arguments,
          so we can't localize them. Not much we can do. *)
        (env, mk (Reason.none, Terr))
    in
    begin
      match SMap.find_opt x ety_env.substs with
      | Some x_ty ->
        let (env, x_ty) = Env.expand_type env x_ty in
        let r_inst = Reason.Rinstantiate (get_reason x_ty, x, r) in
        begin
          match (targs, get_node x_ty) with
          | (_ :: _, Tclass (((_, name) as id), _, [])) ->
            let class_info = Env.get_class env name in
            localize_class_instantiation ~ety_env env r_inst id targs class_info
          | (_ :: _, Tnewtype (id, [], _))
          | (_ :: _, Tunapplied_alias id) ->
            localize_typedef_instantiation ~ety_env env r_inst id targs
          | (_ :: _, Tgeneric (x', [])) -> localize_tgeneric x' r_inst
          | (_, ty_) -> (env, mk (r_inst, ty_))
        end
      | None -> localize_tgeneric x r
    end
  | (r, Toption ty) ->
    let (env, ty) = localize ~ety_env env ty in
    TUtils.union env (MakeType.null r) ty
  | (r, Tlike ty) ->
    let (env, ty) = localize ~ety_env env ty in
    let (env, lty) = TUtils.union env (MakeType.dynamic r) ty in
    (env, lty)
  | (r, Tfun ft) ->
    let pos = Reason.to_pos r in
    let (env, ft) = localize_ft ~ety_env ~def_pos:pos env ft in
    (env, mk (r, Tfun ft))
  | (r, Tapply ((_, x), [arg]))
    when String.equal x Naming_special_names.FB.cIncorrectType
         && Env.is_typedef env x ->
    localize ~ety_env env (mk (r, Tlike arg))
  | (r, Tapply (((_p, cid) as cls), argl)) ->
    begin
      match Env.get_class env cid with
      | None ->
        (* If it isn't a class, try fetching it as a typedef instead.
         * We attempt fetching as a class first since it is the common case.
         *)
        if Env.is_typedef env cid then
          localize_typedef_instantiation ety_env env r cid argl
        else
          localize_class_instantiation ~ety_env env r cls argl None
      | Some class_info ->
        localize_class_instantiation ~ety_env env r cls argl (Some class_info)
    end
  | (r, Ttuple tyl) ->
    let (env, tyl) = List.map_env env tyl (localize ~ety_env) in
    (env, mk (r, Ttuple tyl))
  | (r, Tunion tyl) ->
    let (env, tyl) = List.map_env env tyl (localize ~ety_env) in
    (env, mk (r, Tunion tyl))
  | (r, Tintersection tyl) ->
    let (env, tyl) = List.map_env env tyl (localize ~ety_env) in
    (env, mk (r, Tintersection tyl))
  | (r, Taccess (root_ty, ids)) ->
    (* Sometimes, Tthis and Tgeneric are not expanded to Tabstract, so we need
    to allow accessing abstract type constants here. *)
    let allow_abstract_tconst =
      match get_node root_ty with
      | Tthis
      | Tgeneric _ ->
        true
      | _ -> false
    in
    let (env, root_ty) = localize ~ety_env env root_ty in
    let root_pos = get_pos root_ty in
    let ((env, ty), _) =
      List.fold
        ids
        ~init:((env, root_ty), root_pos)
        ~f:(fun ((env, root_ty), root_pos) id ->
          ( TUtils.expand_typeconst
              ety_env
              env
              root_ty
              id
              ~root_pos
              ~on_error:ety_env.on_error
              ~allow_abstract_tconst,
            fst id ))
    in
    (* Elaborate reason with information about expression dependent types and
     * the original location of the Taccess type
     *)
    let elaborate_reason expand_reason =
      (* First convert into a string of root_ty::ID1::ID2::IDn *)
      let taccess_string =
        String.concat
          ~sep:"::"
          (Typing_print.full_strip_ns env root_ty :: List.map ~f:snd ids)
      in
      (* If the root is an expression dependent type, change the primary
       * reason to be for the full Taccess type to preserve the position where
       * the expression dependent type was derived from.
       *)
      let reason =
        match get_reason root_ty with
        | Reason.Rexpr_dep_type (_, p, e) -> Reason.Rexpr_dep_type (r, p, e)
        | _ -> r
      in
      Reason.Rtype_access (expand_reason, [(reason, taccess_string)])
    in
    let ty = map_reason ty ~f:elaborate_reason in
    (env, ty)
  | (r, Tshape (shape_kind, tym)) ->
    let (env, tym) = ShapeFieldMap.map_env (localize ~ety_env) env tym in
    (env, mk (r, Tshape (shape_kind, tym)))
  | (r, Tpu_access (dbase, enum_or_tyname)) ->
    (* Env.get_upper_bounds might not be populated every time localization
     * is called, but so far it is when it really matters, so we'll stick
     * with this approximation: if no upper bounds info is available,
     * localization will return a Tpu
     *)
    let guess_if_pu env tp targs =
      let upper_bounds = Env.get_upper_bounds env tp targs in
      let res =
        Typing_set.fold
          (fun bound res ->
            match get_node bound with
            | Tpu (_, _) -> true
            | _ -> res)
          upper_bounds
          false
      in
      res
    in
    let (env, base) = localize ~ety_env env dbase in
    (match deref base with
    | (r, Tgeneric (tp, targs)) ->
      let member = (Reason.to_pos r, tp) in
      if guess_if_pu env tp targs then
        (env, mk (r, Tpu_type_access (member, enum_or_tyname)))
      else
        (env, mk (r, Tpu (base, enum_or_tyname)))
    | (r, Tvar v) ->
      Typing_subtype_pocket_universes.get_tyvar_pu_access env r v enum_or_tyname
    | _ -> (env, mk (r, Tpu (base, enum_or_tyname))))

(* Localize type arguments for something whose kinds is [kind] *)
and localize_tparams_by_kind
    ~ety_env
    ~def_pos:_
    ~use_pos:_
    env
    (tyargs : decl_ty list)
    (nkinds : KindDefs.Simple.named_kind list) =
  let exp_len = List.length nkinds in
  let act_len = List.length tyargs in
  let length = min exp_len act_len in
  let (tyargs, nkinds) = (List.take tyargs length, List.take nkinds length) in
  let ((env, _), tyl) =
    List.map2_env (env, ety_env) tyargs nkinds localize_tparam_by_kind
  in
  (* Note that we removed superfluous type arguments, because we don't have a kind to localize
    them against.
    It would also be useful to fill in Terr for missing type arguments, but this breaks some
    checks on built-in collections that check the number of type arguments *after* localization. *)
  (env, tyl)

and localize_tparam_by_kind
    (env, ety_env) ty (nkind : KindDefs.Simple.named_kind) =
  match deref ty with
  | (r, Tapply ((_, x), _argl)) when String.equal x SN.Typehints.wildcard ->
    let (name, kind) = nkind in
    let is_higher_kinded = KindDefs.Simple.get_arity kind > 0 in
    let pos = get_reason ty |> Reason.to_pos in
    if is_higher_kinded then
      (* We don't support wildcards in place of HK type arguments *)
      ((env, ety_env), mk (Reason.none, Terr))
    else
      let full_kind_without_bounds =
        KindDefs.Simple.to_full_kind_without_bounds kind
      in
      let (env, new_name) =
        (* add without bounds, because we need to substitute inside them first,
           as done below *)
        Env.add_fresh_generic_parameter_by_kind
          env
          (snd name)
          full_kind_without_bounds
      in
      let ty_fresh = mk (r, Tgeneric (new_name, [])) in
      (* Substitute fresh type parameters for original formals in constraint *)
      let substs = SMap.add (snd name) ty_fresh ety_env.substs in
      let ety_env = { ety_env with substs } in
      let subst_and_add_localized_constraints env ck cstr_tys =
        Typing_set.fold
          (fun cstr_ty env ->
            let cstr_ty = Typing_kinding.Locl_Inst.instantiate substs cstr_ty in
            TUtils.add_constraint pos env ck ty_fresh cstr_ty)
          cstr_tys
          env
      in
      let env =
        match KindDefs.Simple.get_wilcard_bounds kind with
        | KindDefs.Simple.NonLocalized decl_cstrs ->
          List.fold_left decl_cstrs ~init:env ~f:(fun env (ck, ty) ->
              let (env, ty) = localize ~ety_env env ty in
              TUtils.add_constraint pos env ck ty_fresh ty)
        | KindDefs.Simple.Localized { wc_lower; wc_upper } ->
          let env =
            subst_and_add_localized_constraints
              env
              Ast_defs.Constraint_as
              wc_upper
          in
          let env =
            subst_and_add_localized_constraints
              env
              Ast_defs.Constraint_super
              wc_lower
          in
          env
      in
      ((env, ety_env), ty_fresh)
  | _ ->
    let (env, ty) = localize_with_kind ~ety_env env ty (snd nkind) in
    ((env, ety_env), ty)

and localize_class_instantiation ~ety_env env r sid tyargs class_info =
  let (pos, name) = sid in
  match class_info with
  | None ->
    (* Without class info, we don't know the kinds of the arguments.
       We assume they are non-HK types. *)
    let (env, tyl) = List.map_env env tyargs (localize ~ety_env) in
    (env, mk (r, Tclass (sid, Nonexact, tyl)))
  | Some class_info ->
    if Option.is_some (Cls.enum_type class_info) then
      (* if argl <> [], nastInitCheck would have raised an error *)
      match Typing_defs.has_expanded ety_env name with
      | Some _ ->
        Errors.cyclic_enum_constraint pos;
        (env, mk (r, Typing_utils.tany env))
      | None ->
        let type_expansions = (false, pos, name) :: ety_env.type_expansions in
        let ety_env = { ety_env with type_expansions } in
        let (env, cstr) =
          match Env.get_enum_constraint env name with
          (* If not specified, default bound is arraykey *)
          | None ->
            ( env,
              MakeType.arraykey (Reason.Rimplicit_upper_bound (pos, "arraykey"))
            )
          | Some ty -> localize ~ety_env env ty
        in
        (env, mk (r, Tnewtype (name, [], cstr)))
    else
      let tparams = Cls.tparams class_info in
      let nkinds = KindDefs.Simple.named_kinds_of_decl_tparams tparams in
      let (env, tyl) =
        if
          TypecheckerOptions.global_inference (Env.get_tcopt env)
          && (not (List.is_empty tparams))
          && List.is_empty tyargs
        then
          (* In this case we will infer the missing type parameters *)
          (* FIXME: I guess in global inference mode, we should just reject classes
             with missing type args if they have any HK parameters? *)
          localize_missing_tparams_class env r sid class_info
        else
          let def_pos = Cls.pos class_info in
          let use_pos = Reason.to_pos r in
          localize_tparams_by_kind ~ety_env ~def_pos ~use_pos env tyargs nkinds
      in
      (env, mk (r, Tclass (sid, Nonexact, tyl)))

and localize_typedef_instantiation ~ety_env env r type_name tyargs =
  match Env.get_typedef env type_name with
  | Some typedef_info ->
    let tparams = typedef_info.Typing_defs.td_tparams in
    let nkinds = KindDefs.Simple.named_kinds_of_decl_tparams tparams in
    let use_pos = Reason.to_pos r in
    let def_pos = typedef_info.Typing_defs.td_pos in
    let (env, tyargs) =
      localize_tparams_by_kind ~ety_env ~use_pos ~def_pos env tyargs nkinds
    in
    TUtils.expand_typedef ety_env env r type_name tyargs
  | None ->
    (* This must be unreachable. We only call localize_typedef_instantiation if we *know* that
       we have a typedef with typedef info at hand. *)
    failwith "Internal error: No info about typedef"

(* Localize a type with the given expected kind, which
   may either indicate a higher-kinded or fully applied type.
 *)
and localize_with_kind
    ~ety_env env (dty : decl_ty) (expected_kind : KindDefs.Simple.kind) =
  let is_newtype typedef =
    (* FIXME This function should be somewhere else *)
    match typedef.Typing_defs.td_vis with
    | Aast_defs.Transparent -> false
    | Aast_defs.Opaque -> true
  in
  let (r, dty_) = deref dty in
  let arity = KindDefs.Simple.get_arity expected_kind in
  if Int.( = ) arity 0 then
    (* Not higher-kinded *)
    localize ~ety_env env dty
  else
    match dty_ with
    | Tapply (((_pos, name) as id), []) ->
      begin
        match Env.get_class env name with
        | Some class_info ->
          let tparams = Cls.tparams class_info in
          let class_kind =
            KindDefs.Simple.type_with_params_to_simple_kind tparams
          in
          if Kinding.Simple.is_subkind env ~sub:class_kind ~sup:expected_kind
          then
            (env, mk (r, Tclass (id, Nonexact, [])))
          else
            (env, mk (Reason.none, Terr))
        | None ->
          begin
            match Env.get_typedef env name with
            | Some typedef ->
              if is_newtype typedef then
                (* The bound is unused until the newtype is fully applied, thus supplying dummy Tany *)
                ( env,
                  mk (r, Tnewtype (name, [], mk (Reason.none, make_tany ()))) )
              else
                (env, mk (r, Tunapplied_alias name))
            | None ->
              (* We are expected to localize a higher-kinded type, but are given an unknown class name.
                Not much we can do. *)
              (env, mk (Reason.none, Terr))
          end
      end
    | Tgeneric (name, []) ->
      begin
        match Env.get_pos_and_kind_of_generic env name with
        | Some (_, gen_kind) ->
          if
            Kinding.Simple.is_subkind
              env
              ~sub:(KindDefs.Simple.from_full_kind gen_kind)
              ~sup:expected_kind
          then
            (env, mk (r, Tgeneric (name, [])))
          else
            (env, mk (Reason.none, Terr))
        | None ->
          (* FIXME: Ideally, we would like to fail here, but sometimes we see type
            parameters without an entry in the environment. *)
          (env, mk (r, Tgeneric (name, [])))
      end
    | Tgeneric (_, _targs)
    | Tapply (_, _targs) ->
      (env, mk (Reason.none, Terr))
    | Tany _ -> (env, mk (r, make_tany ()))
    | Terr -> (env, mk (r, Terr))
    | _dty_ -> (env, mk (Reason.none, Terr))

(* Recursive localizations of function types do not make judgements about enforceability *)
and localize_possibly_enforced_ty ~ety_env env ety =
  let (env, et_type) = localize ~ety_env env ety.et_type in
  (env, { ety with et_type })

and localize_cstr_ty ~ety_env env ty tp_name =
  let (env, ty) = localize ~ety_env env ty in
  let ty =
    map_reason ty ~f:(fun r ->
        Reason.Rcstr_on_generics (Reason.to_pos r, tp_name))
  in
  (env, ty)

(* Localize an explicit type argument to a constructor or function. We
 * support the use of wildcards at the top level only *)
and localize_targ_with_kind
    ~check_well_kinded env hint (nkind : KindDefs.Simple.named_kind) =
  (* For explicit type arguments we support a wildcard syntax `_` for which
   * Hack will generate a fresh type variable *)
  let kind = snd nkind in
  match hint with
  | (_, Aast.Happly ((p, id), [])) when String.equal id SN.Typehints.wildcard ->
    let is_higher_kinded = KindDefs.Simple.get_arity kind > 0 in
    if is_higher_kinded then
      let () = Errors.wildcard_for_higher_kinded_type (fst hint) in
      (env, (mk (Reason.none, Terr), hint))
    else
      let (env, ty) = Env.fresh_type env p in
      (env, (ty, hint))
  | _ ->
    let ty = Decl_hint.hint env.decl_env hint in
    if check_well_kinded then Kinding.Simple.check_well_kinded env ty nkind;
    let (env, ty) = localize_with_self_and_kind env ty kind in
    (env, (ty, hint))

and localize_targ ~check_well_kinded env hint =
  let named_kind =
    KindDefs.Simple.with_dummy_name (KindDefs.Simple.fully_applied_type ())
  in
  localize_targ_with_kind ~check_well_kinded env hint named_kind

(* See signature in .mli file for details *)
and localize_targs_with_kinds
    ~check_well_kinded
    ~is_method
    ~def_pos
    ~use_pos
    ~use_name
    ?(tparaml = [])
    env
    named_kinds
    targl =
  let targ_count = List.length targl in
  let tparam_count =
    match List.length tparaml with
    | 0 -> List.length named_kinds
    | n -> n
  in
  (* If there are explicit type arguments but too few or too many then
   * report an error *)
  if Int.( <> ) targ_count 0 && Int.( <> ) tparam_count targ_count then
    if is_method then
      Errors.expected_tparam ~definition_pos:def_pos ~use_pos tparam_count None
    else
      Errors.type_arity
        use_pos
        def_pos
        ~expected:tparam_count
        ~actual:targ_count;

  (* Declare and localize the explicit type arguments *)
  let targl =
    if Int.( > ) targ_count tparam_count then
      List.take targl tparam_count
    else
      targl
  in
  let (env, explicit_targs) =
    List.map2_env
      env
      targl
      (List.take named_kinds targ_count)
      (localize_targ_with_kind ~check_well_kinded)
  in
  (* Generate fresh type variables for the remainder *)
  let (env, implicit_targs) =
    let mk_implicit_targ env (kind_name, kind) =
      let wildcard_hint =
        (use_pos, Aast.Happly ((Pos.none, SN.Typehints.wildcard), []))
      in
      if
        check_well_kinded
        && KindDefs.Simple.get_arity kind > 0
        && Int.( = ) targ_count 0
      then (
        (* We only throw an error if the user didn't provide any type arguments at all.
           Otherwise, if they provided some, but not all of them, n arity mismatch
           triggers earlier in this function, independently from higher-kindedness *)
        Errors.implicit_type_argument_for_higher_kinded_type
          ~use_pos
          ~def_pos:(fst kind_name)
          (snd kind_name);
        (env, (mk (Reason.none, Terr), wildcard_hint))
      ) else
        let (env, tvar) =
          Env.fresh_type_reason
            env
            (Reason.Rtype_variable_generics (use_pos, snd kind_name, use_name))
        in
        Typing_log.log_tparam_instantiation env use_pos (snd kind_name) tvar;
        (env, (tvar, wildcard_hint))
    in
    List.map_env env (List.drop named_kinds targ_count) mk_implicit_targ
  in
  let check_for_explicit_user_attribute tparam (_, hint) =
    let is_wildcard =
      match hint with
      | (_, Aast.Happly ((_, class_id), _))
        when String.equal class_id SN.Typehints.wildcard ->
        true
      | _ -> false
    in
    if
      Attributes.mem SN.UserAttributes.uaExplicit tparam.tp_user_attributes
      && is_wildcard
    then
      Errors.require_generic_explicit tparam.tp_name (fst hint)
  in
  List.iter2
    tparaml
    (explicit_targs @ implicit_targs)
    ~f:check_for_explicit_user_attribute
  |> ignore;
  (env, explicit_targs @ implicit_targs)

and localize_targs
    ~check_well_kinded ~is_method ~def_pos ~use_pos ~use_name env tparaml targl
    =
  let nkinds = KindDefs.Simple.named_kinds_of_decl_tparams tparaml in
  localize_targs_with_kinds
    ~check_well_kinded
    ~is_method
    ~def_pos
    ~use_pos
    ~use_name
    ~tparaml
    env
    nkinds
    targl

(* For the majority of cases when we localize a function type we instantiate
 * the function's type parameters to Tvars. There are two cases where we do not do this.

 * 1) In Typing_subtype.subtype_method. See the comment for that function for why
 * this is necessary.
 * 2) When the type arguments are explicitly specified, in which case we instantiate
 * the type parameters to the provided types.
 *)
and localize_ft
    ?(instantiation : method_instantiation option) ~ety_env ~def_pos env ft =
  (* If no explicit type parameters are provided, set the instantiated type parameter to
   * initially point to unresolved, so that it can grow and eventually be a subtype of
   * something like "mixed".
   * If explicit type parameters are provided, just instantiate tvarl to them.
   *)
  let (env, substs) =
    match instantiation with
    | Some { explicit_targs; use_name = _; use_pos } ->
      if
        (not (List.is_empty explicit_targs))
        && Int.( <> ) (List.length explicit_targs) (List.length ft.ft_tparams)
      then
        Errors.expected_tparam
          ~definition_pos:def_pos
          ~use_pos
          (List.length ft.ft_tparams)
          None;
      let tvarl = List.map ~f:fst explicit_targs in
      let ft_subst = Subst.make_locl ft.ft_tparams tvarl in
      (env, SMap.union ft_subst ety_env.substs)
    | None ->
      ( env,
        List.fold_left
          ft.ft_tparams
          ~f:
            begin
              fun subst t ->
              SMap.remove (snd t.tp_name) subst
            end
          ~init:ety_env.substs )
  in
  let ety_env = { ety_env with substs } in
  (* Localize the constraints for a type parameter declaration *)
  let rec localize_tparam ~nested env t =
    let (env, cstrl) =
      (* TODO(T70068435)
        For nested type parameters (i.e., type parameters of type parameters),
        we do not support constraints, yet. If nested type parameters do have
        constraints, this is reported earlier. We just throw them away here. *)
      if nested then
        (env, [])
      else
        List.map_env env t.tp_constraints (fun env (ck, ty) ->
            let (env, ty) = localize_cstr_ty ~ety_env env ty t.tp_name in
            let name_str = snd t.tp_name in
            (* In order to access type constants on generics on where clauses,
             we need to add the constraints from the type parameters into the
             environment before localizing the where clauses with them. Temporarily
             add them to the environment here, and reset the environment later. *)
            let env =
              match ck with
              | Ast_defs.Constraint_as -> Env.add_upper_bound env name_str ty
              | Ast_defs.Constraint_super -> Env.add_lower_bound env name_str ty
              | Ast_defs.Constraint_eq ->
                Env.add_upper_bound
                  (Env.add_lower_bound env name_str ty)
                  name_str
                  ty
            in
            (env, (ck, ty)))
    in
    let (env, tparams) =
      List.map_env env t.tp_tparams (localize_tparam ~nested:true)
    in
    (env, { t with tp_constraints = cstrl; tp_tparams = tparams })
  in
  let localize_where_constraint env (ty1, ck, ty2) =
    let (env, ty1) = localize ~ety_env env ty1 in
    let (env, ty2) = localize ~ety_env env ty2 in
    (env, (ty1, ck, ty2))
  in
  (* Grab and store the old tpenvs *)
  let old_tpenv = Env.get_tpenv env in
  let old_global_tpenv = env.global_tpenv in
  (* Always localize tparams so they are available for later Tast check *)
  let (env, tparams) =
    List.map_env env ft.ft_tparams (localize_tparam ~nested:false)
  in
  (* Localize the 'where' constraints *)
  let (env, where_constraints) =
    List.map_env env ft.ft_where_constraints localize_where_constraint
  in
  (* Remove the constraints we added for localizing where constraints  *)
  let env = Env.env_with_tpenv env old_tpenv in
  let env = Env.env_with_global_tpenv env old_global_tpenv in
  (* If we're instantiating the generic parameters then add a deferred
   * check that constraints are satisfied under the
   * substitution [ety_env.substs].
   *)
  let env =
    match instantiation with
    | Some { use_pos; _ } ->
      let env = check_tparams_constraints ~use_pos ~ety_env env ft.ft_tparams in
      let env =
        check_where_constraints
          ~in_class:false
          ~use_pos
          ~definition_pos:def_pos
          ~ety_env
          env
          ft.ft_where_constraints
      in
      env
    | None -> env
  in
  let (env, arity) =
    match ft.ft_arity with
    | Fvariadic ({ fp_type = var_ty; _ } as param) ->
      let (env, var_ty) = localize ~ety_env env var_ty.et_type in
      (* HHVM does not enforce types on vararg parameters yet *)
      ( env,
        Fvariadic
          { param with fp_type = { et_type = var_ty; et_enforced = false } } )
    | Fstandard as x -> (env, x)
  in
  let (env, params) =
    List.map_env env ft.ft_params (fun env param ->
        let (env, ty) =
          localize_possibly_enforced_ty ~ety_env env param.fp_type
        in
        (env, { param with fp_type = ty }))
  in
  let (env, implicit_params) =
    let (env, capability) =
      localize ~ety_env env ft.ft_implicit_params.capability
    in
    (env, { capability })
  in
  let (env, ret) = localize_possibly_enforced_ty ~ety_env env ft.ft_ret in
  let ft =
    set_ft_ftk
      ft
      ( if Option.is_some instantiation then
        FTKinstantiated_targs
      else
        FTKtparams )
  in
  ( env,
    {
      ft with
      ft_arity = arity;
      ft_params = params;
      ft_implicit_params = implicit_params;
      ft_ret = ret;
      ft_tparams = tparams;
      ft_where_constraints = where_constraints;
    } )

(* Given a list of generic parameters [tparams] and a substitution
 * in [ety_env.substs] whose domain is at least these generic parameters,
 * check that the types satisfy
 * the constraints on the corresponding generic parameter.
 *
 * Note that the constraints may contain occurrences of the generic
 * parameters, but the subsitution will be applied to them. e.g. if tparams is
 *   <Tu as MyCovariant<Tu>, Tv super Tu>
 * and ety_env.substs is
 *   Tu :-> C
 *   Tv :-> I
 * with
 *   class C extends MyContravariant<I> implements I { ... }
 * Then the constraints are satisfied, because
 *   C is a subtype of MyContravariant<C>
 *   I is a supertype of C
 *)
and check_tparams_constraints ~use_pos ~ety_env env tparams =
  let check_tparam_constraints env t =
    List.fold_left t.tp_constraints ~init:env ~f:(fun env (ck, ty) ->
        let (env, ty) = localize_cstr_ty ~ety_env env ty t.tp_name in
        match SMap.find_opt (snd t.tp_name) ety_env.substs with
        | Some x_ty ->
          Typing_log.(
            log_with_level env "generics" 1 (fun () ->
                log_types
                  use_pos
                  env
                  [
                    Log_head
                      ( "check_tparams_constraints: check_tparams_constraint",
                        [Log_type ("ty", ty); Log_type ("x_ty", x_ty)] );
                  ]));
          TGenConstraint.check_tparams_constraint
            env
            ~use_pos
            t.tp_name
            ck
            ty
            x_ty
        | None -> env)
  in
  List.fold_left tparams ~init:env ~f:check_tparam_constraints

and check_where_constraints
    ~in_class ~use_pos ~ety_env ~definition_pos env cstrl =
  List.fold_left cstrl ~init:env ~f:(fun env (ty1, ck, ty2) ->
      let (env, ty1) = localize ~ety_env env ty1 in
      let (env, ty2) = localize ~ety_env env ty2 in
      TGenConstraint.check_where_constraint
        ~in_class
        env
        ~use_pos
        ~definition_pos
        ck
        ty2
        ty1)

(* Performs no substitutions of generics and initializes Tthis to
 * Env.get_self env
 *)
and localize_with_self env ?pos ?(quiet = false) ?report_cycle ty =
  let ety_env = env_with_self env ?pos ~quiet ?report_cycle in
  localize env ty ~ety_env

(* Like localize_with_self, but uses the supplied kind, enabling support
   for higher-kinded types *)
and localize_with_self_and_kind env ?pos ?(quiet = false) ?report_cycle ty kind
    =
  let ety_env = env_with_self env ?pos ~quiet ?report_cycle in
  localize_with_kind ~ety_env env ty kind

and localize_possibly_enforced_with_self env ety =
  let (env, et_type) = localize_with_self env ety.et_type in
  (env, { ety with et_type })

and localize_hint_with_self env h =
  let h = Decl_hint.hint env.decl_env h in
  localize_with_self env h

and localize_hint ~ety_env env hint =
  let hint_ty = Decl_hint.hint env.decl_env hint in
  localize ~ety_env env hint_ty

(**
 * If a type annotation for a class is missing type arguments, create a type variable
 * for them and apply constraints.
 *
 * This is not used for local inference on constructors, but rather for global inference
 * for partial files where function signatures are allowed to contain types ommitting type
 * arguments.
 *)
and localize_missing_tparams_class env r sid class_ =
  let use_pos = Reason.to_pos r in
  let use_name = Utils.strip_ns (snd sid) in
  let ((env, _i), tyl) =
    List.fold_map (Cls.tparams class_) ~init:(env, 0) ~f:(fun (env, i) tparam ->
        let (env, ty) =
          Env.new_global_tyvar
            env
            ~i
            (Reason.Rtype_variable_generics
               (use_pos, snd tparam.tp_name, use_name))
        in
        ((env, i + 1), ty))
  in
  let c_ty = mk (r, Tclass (sid, Nonexact, tyl)) in
  let ety_env =
    {
      type_expansions = [];
      this_ty = c_ty;
      substs = Subst.make_locl (Cls.tparams class_) tyl;
      from_class = Some (Aast.CI sid);
      quiet = false;
      on_error = Errors.unify_error_at use_pos;
    }
  in
  let env =
    check_tparams_constraints ~use_pos ~ety_env env (Cls.tparams class_)
  in
  let env =
    check_where_constraints
      ~in_class:true
      ~use_pos
      ~definition_pos:(Cls.pos class_)
      ~ety_env
      env
      (Cls.where_constraints class_)
  in
  (env, tyl)

and localize_targs_and_check_constraints
    ~exact
    ~check_well_kinded
    ~check_constraints
    ~def_pos
    ~use_pos
    env
    class_id
    from_class
    tparaml
    hintl =
  let (env, type_argl) =
    localize_targs
      ~check_well_kinded
      ~is_method:false
      ~def_pos
      ~use_pos
      ~use_name:(Utils.strip_ns (snd class_id))
      env
      tparaml
      hintl
  in
  let targs_tys = List.map ~f:fst type_argl in
  let this_ty =
    mk (Reason.Rwitness (fst class_id), Tclass (class_id, exact, targs_tys))
  in
  let env =
    if check_constraints then
      let ety_env =
        {
          type_expansions = [];
          this_ty;
          substs = Subst.make_locl tparaml targs_tys;
          from_class = Some from_class;
          quiet = false;
          on_error = Errors.unify_error_at use_pos;
        }
      in
      check_tparams_constraints ~use_pos ~ety_env env tparaml
    else
      env
  in
  (env, this_ty, type_argl)

(* Add generic parameters to the environment, localize their bounds, and
 * transform these into a flat list of constraints of the form (ty1,ck,ty2)
 * where ck is as, super or =
 *)
let localize_generic_parameters_with_bounds
    ~ety_env (env : env) (tparams : decl_tparam list) =
  let env = Env.add_generic_parameters env tparams in
  let localize_bound
      env ({ tp_name = (pos, name); tp_constraints = cstrl; _ } : decl_tparam) =
    (* TODO(T70068435) This may have to be touched when adding support for constraints on HK
      types *)
    let tparam_ty = mk (Reason.Rwitness pos, Tgeneric (name, [])) in
    List.map_env env cstrl (fun env (ck, cstr) ->
        let (env, ty) = localize env cstr ~ety_env in
        (env, (tparam_ty, ck, ty)))
  in
  let (env, cstrss) = List.map_env env tparams localize_bound in
  (env, List.concat cstrss)

let localize_and_add_where_constraints ~ety_env (env : env) where_constraints =
  let add_constraint env (h1, ck, h2) =
    let (env, ty1) = localize env (Decl_hint.hint env.decl_env h1) ~ety_env in
    let (env, ty2) = localize env (Decl_hint.hint env.decl_env h2) ~ety_env in
    TUtils.add_constraint (fst h1) env ck ty1 ty2
  in
  List.fold_left where_constraints ~f:add_constraint ~init:env

(* Helper functions *)

let sub_type_decl env ty1 ty2 on_error =
  let (env, ty1) = localize_with_self env ty1 in
  let (env, ty2) = localize_with_self env ty2 in
  let env = TUtils.sub_type env ty1 ty2 on_error in
  env

let add_constraints p env constraints =
  let add_constraint env (ty1, ck, ty2) =
    Typing_utils.add_constraint p env ck ty1 ty2
  in
  List.fold_left constraints ~f:add_constraint ~init:env

let localize_and_add_generic_parameters pos env tparams =
  let ety_env = env_with_self env in
  let (env, constraints) =
    localize_generic_parameters_with_bounds env tparams ~ety_env
  in
  let env = add_constraints pos env constraints in
  env

let localize_and_add_ast_generic_parameters_and_where_constraints
    pos env tparams where_constraints =
  let tparams : decl_tparam list =
    List.map tparams ~f:(Decl_hint.aast_tparam_to_decl_tparam env.decl_env)
  in
  let env = localize_and_add_generic_parameters pos env tparams in
  let ety_env = env_with_self env in
  let env = localize_and_add_where_constraints ~ety_env env where_constraints in
  env

let () = TUtils.localize_with_self_ref := localize_with_self

let () = TUtils.localize_ref := localize

let () = TUtils.env_with_self_ref := env_with_self
