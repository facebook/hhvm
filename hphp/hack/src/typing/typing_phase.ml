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
module Env = Typing_env
module TUtils = Typing_utils
module TGenConstraint = Typing_generic_constraint
module Subst = Decl_subst
module MakeType = Typing_make_type
module Cls = Decl_provider.Class
module KindDefs = Typing_kinding_defs
module Kinding = Typing_kinding
module SN = Naming_special_names

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

let rec localize ~(ety_env : expand_env) env (dty : decl_ty) =
  (fun ((env, ty_err_opt), ty) ->
    let (env, ty) = Typing_log.log_localize ~level:1 ety_env dty (env, ty) in
    ((env, ty_err_opt), ty))
  @@
  let tvar_or_localize ~ety_env env r ty ~i =
    if
      TypecheckerOptions.global_inference env.genv.tcopt
      && (is_any ty || is_tyvar ty)
    then
      let (env, tv) = Env.new_global_tyvar env ~i r in
      ((env, None), tv)
    else
      localize ~ety_env env ty
  in
  let r = get_reason dty |> Typing_reason.localize in
  match get_node dty with
  | Terr -> ((env, None), TUtils.terr env r)
  | Trefinement (root, cr) -> localize_refinement ~ety_env env r root cr
  | Tany _ -> ((env, None), mk (r, TUtils.tany env))
  | Tvar _var ->
    let (env, tv) = Env.new_global_tyvar env r in
    ((env, None), tv)
  | (Tnonnull | Tprim _ | Tdynamic) as x -> ((env, None), mk (r, x))
  | Tmixed -> ((env, None), MakeType.mixed r)
  | Tthis ->
    let ty =
      map_reason ety_env.this_ty ~f:(function
          | Reason.Rnone -> r
          | Reason.Rexpr_dep_type (_, pos, s) ->
            Reason.Rexpr_dep_type (r, pos, s)
          | reason -> Reason.Rinstantiate (reason, SN.Typehints.this, r))
    in
    ((env, None), ty)
  | Tvec_or_dict (tk, tv) ->
    let ((env, e1), tk) = tvar_or_localize ~ety_env env r tk ~i:0 in
    let ((env, e2), tv) = tvar_or_localize ~ety_env env r tv ~i:1 in
    let ty = Tvec_or_dict (tk, tv) in
    let ty_err_opt = Option.merge e1 e2 ~f:Typing_error.both in
    ((env, ty_err_opt), mk (r, ty))
  | Tgeneric (x, targs) ->
    let localize_tgeneric ?replace_with name r =
      match (targs, replace_with, Env.get_pos_and_kind_of_generic env name) with
      | (_, _, Some (_def_pos, kind)) ->
        let arg_kinds : KindDefs.Simple.named_kind list =
          KindDefs.Simple.from_full_kind kind
          |> KindDefs.Simple.get_named_parameter_kinds
        in
        begin
          match
            ( localize_targs_by_kind
                ~ety_env:{ ety_env with expand_visible_newtype = true }
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
        ((env, None), mk (r, Tgeneric (x, [])))
      | ([], Some repl_ty, None) -> ((env, None), mk (r, repl_ty))
      | (_ :: _, _, None) ->
        (* No kinding info, but type arguments given. We don't know the kinds of the arguments,
           so we can't localize them. Not much we can do. *)
        ((env, None), mk (Reason.none, Terr))
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
            localize_typedef_instantiation
              ~ety_env
              env
              r_inst
              (get_reason dty)
              id
              targs
              (Env.get_typedef env id)
          | (_ :: _, Tgeneric (x', [])) -> localize_tgeneric x' r_inst
          | (_, ty_) -> ((env, None), mk (r_inst, ty_))
        end
      | None -> localize_tgeneric x r
    end
  | Toption ty ->
    let ((env, ty_err_opt), ty) = localize ~ety_env env ty in
    (* Calling into the union module here would cost 2% perf regression on a full init,
     * so we use this lightweight version instead. *)
    let union_null env ty =
      let rec null_is_subtype_of ty =
        match get_node ty with
        | Toption _
        | Tprim Aast.Tnull ->
          true
        | Tunion tyl -> List.exists tyl ~f:null_is_subtype_of
        | Tintersection tyl -> List.for_all tyl ~f:null_is_subtype_of
        | _ -> false
      in
      if null_is_subtype_of ty then
        (env, ty)
      else
        Typing_make_type.nullable_locl r ty
        |> TUtils.wrap_union_inter_ty_in_var env r
    in
    let (env, ty) = union_null env ty in
    ((env, ty_err_opt), ty)
  | Tlike ty ->
    let ((env, ty_err_opt), ty) = localize ~ety_env env ty in
    let (env, lty) = TUtils.union env (MakeType.dynamic r) ty in
    ((env, ty_err_opt), lty)
  | Tfun ft ->
    let pos = Reason.to_pos r in
    let (env, ft) = localize_ft ~ety_env ~def_pos:pos env ft in
    (env, mk (r, Tfun ft))
  | Tapply ((_, x), [arg])
    when String.equal x Naming_special_names.FB.cIncorrectType
         && Env.is_typedef env x ->
    localize ~ety_env env (mk (get_reason dty, Tlike arg))
  | Tapply ((_, x), [arg])
    when String.equal x Naming_special_names.HH.FIXME.tTanyMarker ->
    if TypecheckerOptions.enable_sound_dynamic (Env.get_tcopt env) then
      localize ~ety_env env arg
    else
      ((env, None), mk (r, Typing_utils.tany env))
  | Tapply ((_, x), [arg])
    when String.equal x Naming_special_names.HH.FIXME.tPoisonMarker ->
    let decl_ty =
      if TypecheckerOptions.enable_sound_dynamic (Env.get_tcopt env) then
        mk (get_reason dty, Tlike arg)
      else
        arg
    in
    localize ~ety_env env decl_ty
  | Tapply (((_p, cid) as cls), argl) ->
    begin
      match Env.get_class_or_typedef env cid with
      | Some (Env.ClassResult class_info) ->
        localize_class_instantiation ~ety_env env r cls argl (Some class_info)
      | Some (Env.TypedefResult typedef_info) ->
        localize_typedef_instantiation
          ~ety_env
          env
          r
          (get_reason dty)
          cid
          argl
          (Some typedef_info)
      | None -> localize_class_instantiation ~ety_env env r cls argl None
    end
  | Ttuple tyl ->
    let (env, tyl) =
      List.map_env_ty_err_opt
        env
        tyl
        ~f:(localize ~ety_env)
        ~combine_ty_errs:Typing_error.multiple_opt
    in
    (env, mk (r, Ttuple tyl))
  | Tunion tyl ->
    let ((env, ty_err_opt), tyl) =
      List.map_env_ty_err_opt
        env
        tyl
        ~f:(localize ~ety_env)
        ~combine_ty_errs:Typing_error.union_opt
    in
    let (env, ty) = Typing_union.union_list env r tyl in
    ((env, ty_err_opt), ty)
  | Tintersection tyl ->
    let ((env, ty_err_opt), tyl) =
      List.map_env_ty_err_opt
        env
        tyl
        ~f:(localize ~ety_env)
        ~combine_ty_errs:Typing_error.multiple_opt
    in
    let (env, ty) = Typing_intersection.intersect_list env r tyl in
    ((env, ty_err_opt), ty)
  | Taccess (root_ty, id) ->
    let rec allow_abstract_tconst ty =
      match get_node ty with
      | Tthis
      | Tgeneric _ ->
        (*
         * When the root of an access is 'this', abstract type constants
         * are allowed and localized as rigid type variables (Tgeneric).
         * This happens when typing generic code in an abstract class
         * that deals with data whose type is going to be set later in
         * derived classes.
         *
         * In case the root is a generic, we also accept accesses to
         * abstract constant to type check the dangerous and ubiquitous
         * pattern:
         *
         *   function get<TBox as Box, T>(TBox $foo) : T where T = TBox::T
         *                                                         ^^^^^^^
         *)
        true
      | Taccess (ty, _) -> allow_abstract_tconst ty
      | _ -> false
    in
    let allow_abstract_tconst = allow_abstract_tconst root_ty in
    let ((env, e1), root_ty) = localize ~ety_env env root_ty in
    let ((env, e2), ty) =
      TUtils.expand_typeconst ety_env env root_ty id ~allow_abstract_tconst
    in
    (* Elaborate reason with information about expression dependent types and
     * the original location of the Taccess type
     *)
    let elaborate_reason expand_reason =
      let taccess_string =
        lazy (Typing_print.full_strip_ns env root_ty ^ "::" ^ snd id)
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
    let ty_err_opt = Option.merge e1 e2 ~f:Typing_error.both in
    ((env, ty_err_opt), ty)
  | Tshape (shape_kind, tym) ->
    let (env, tym) =
      ShapeFieldMap.map_env_ty_err_opt
        (localize ~ety_env)
        env
        tym
        ~combine_ty_errs:Typing_error.multiple_opt
    in
    (env, mk (r, Tshape (shape_kind, tym)))
  | Tnewtype (name, tyl, ty) ->
    if Env.is_enum env name then
      failwith "should not happen"
    else
      let td =
        Utils.unsafe_opt
        @@ Decl_provider.get_typedef (Typing_env.get_ctx env) name
      in
      let should_expand =
        Typing_env.is_typedef_visible
          env
          ~expand_visible_newtype:ety_env.expand_visible_newtype
          ~name
          td
      in
      if should_expand then
        Decl_typedef_expand.expand_typedef
          ~force_expand:true
          (Typing_env.get_ctx env)
          (get_reason dty)
          name
          tyl
        |> localize ~ety_env env
      else
        let ((env, e1), ty) = localize ~ety_env env ty in
        let ((env, e2), tyl) =
          List.map_env_ty_err_opt
            env
            tyl
            ~f:(localize ~ety_env)
            ~combine_ty_errs:Typing_error.multiple_opt
        in
        let ty_err_opt = Option.merge e1 e2 ~f:Typing_error.both in
        ((env, ty_err_opt), mk (r, Tnewtype (name, tyl, ty)))

(* Localize type arguments for something whose kinds is [kind] *)
and localize_targs_by_kind
    ~ety_env
    env
    (tyargs : decl_ty list)
    (nkinds : KindDefs.Simple.named_kind list) =
  let exp_len = List.length nkinds in
  let act_len = List.length tyargs in
  let length = min exp_len act_len in
  let (tyargs, nkinds) = (List.take tyargs length, List.take nkinds length) in
  let ((env, ty_errs, _), tyl) =
    List.map2_env
      (env, [], ety_env)
      tyargs
      nkinds
      ~f:(fun (env, ty_errs, ety_env) x y ->
        let ((env, ty_err_opt, ety_env), res) =
          localize_targ_by_kind (env, ety_env) x y
        in
        let ty_errs =
          Option.value_map ty_err_opt ~default:ty_errs ~f:(fun e ->
              e :: ty_errs)
        in
        ((env, ty_errs, ety_env), res))
  in
  let ty_err_opt = Typing_error.multiple_opt ty_errs in
  (* Note that we removed superfluous type arguments, because we don't have a kind to localize
     them against.
     It would also be useful to fill in Terr for missing type arguments, but this breaks some
     checks on built-in collections that check the number of type arguments *after* localization. *)
  ((env, ty_err_opt), tyl)

and localize_targ_by_kind (env, ety_env) ty (nkind : KindDefs.Simple.named_kind)
    =
  match deref ty with
  | (r, Tapply ((pos, x), _argl)) when String.equal x SN.Typehints.wildcard ->
    let r = Typing_reason.localize r in
    let (name, kind) = nkind in
    let is_higher_kinded = KindDefs.Simple.get_arity kind > 0 in
    if is_higher_kinded then
      (* We don't support wildcards in place of HK type arguments *)
      ((env, None, ety_env), mk (Reason.none, Terr))
    else
      let full_kind_without_bounds =
        KindDefs.Simple.to_full_kind_without_bounds kind
      in
      let (env, new_name) =
        (* add without bounds, because we need to substitute inside them first,
           as done below *)
        Env.add_fresh_generic_parameter_by_kind
          env
          pos
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
            TUtils.add_constraint env ck ty_fresh cstr_ty ety_env.on_error)
          cstr_tys
          env
      in
      let (env, ty_errs) =
        match KindDefs.Simple.get_wilcard_bounds kind with
        | KindDefs.Simple.NonLocalized decl_cstrs ->
          List.fold_left
            decl_cstrs
            ~init:(env, [])
            ~f:(fun (env, ty_errs) (ck, ty) ->
              let ((env, ty_err_opt), ty) = localize ~ety_env env ty in
              let ty_errs =
                Option.value_map
                  ~default:ty_errs
                  ~f:(fun e -> e :: ty_errs)
                  ty_err_opt
              in
              let env =
                TUtils.add_constraint env ck ty_fresh ty ety_env.on_error
              in
              (env, ty_errs))
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
          (env, [])
      in
      let ty_err_opt = Typing_error.multiple_opt ty_errs in
      ((env, ty_err_opt, ety_env), ty_fresh)
  | _ ->
    let ((env, ty_err_opt), ty) =
      localize_with_kind ~ety_env env ty (snd nkind)
    in
    ((env, ty_err_opt, ety_env), ty)

and localize_class_instantiation ~ety_env env r sid tyargs class_info =
  let (pos, name) = sid in
  match class_info with
  | None ->
    (* Without class info, we don't know the kinds of the arguments.
       We assume they are non-HK types. *)
    let (env, tyl) =
      List.map_env_ty_err_opt
        env
        tyargs
        ~f:(localize ~ety_env:{ ety_env with expand_visible_newtype = true })
        ~combine_ty_errs:Typing_error.multiple_opt
    in
    (env, mk (r, Tclass (sid, nonexact, tyl)))
  | Some class_info ->
    if Option.is_some (Cls.enum_type class_info) then
      let (ety_env, has_cycle) =
        Typing_defs.add_type_expansion_check_cycles ety_env (pos, name)
      in
      match has_cycle with
      | Some _ ->
        let ty_err_opt =
          Option.map
            ety_env.on_error
            ~f:
              Typing_error.(
                fun on_error ->
                  apply_reasons ~on_error
                  @@ Secondary.Cyclic_enum_constraint pos)
        in
        ((env, ty_err_opt), mk (r, Typing_utils.tany env))
      | None ->
        if Ast_defs.is_c_enum_class (Cls.kind class_info) then
          (* Enum classes no longer has the ambiguity between the type of
           * the enum set and the type of elements, so the enum class
           * itself is seen as a Tclass
           *)
          ((env, None), mk (r, Tclass (sid, nonexact, [])))
        else
          let (env, cstr) =
            match Env.get_enum_constraint env name with
            (* If not specified, default bound is arraykey *)
            | None ->
              ( (env, None),
                MakeType.arraykey
                  (Reason.Rimplicit_upper_bound (pos, "arraykey")) )
            | Some ty -> localize ~ety_env env ty
          in
          (env, mk (r, Tnewtype (name, [], cstr)))
    else
      let tparams = Cls.tparams class_info in
      let nkinds = KindDefs.Simple.named_kinds_of_decl_tparams tparams in
      let ((env, err), tyl) =
        if
          TypecheckerOptions.global_inference (Env.get_tcopt env)
          && (not (List.is_empty tparams))
          && List.is_empty tyargs
        then
          (* In this case we will infer the missing type parameters *)
          (* FIXME: I guess in global inference mode, we should just reject
           * classes with missing type args if they have any HK parameters ?
           *)
          localize_missing_tparams_class_for_global_inference
            env
            r
            sid
            class_info
        else
          localize_targs_by_kind
            ~ety_env:{ ety_env with expand_visible_newtype = true }
            env
            tyargs
            nkinds
      in
      (* Hide the class type if its internal and outside of the module *)
      if Typing_modules.is_class_visible env class_info then
        ((env, err), mk (r, Tclass (sid, nonexact, tyl)))
      else
        let callee_module =
          match Cls.get_module class_info with
          | Some m -> m
          | None ->
            failwith
              "Internal error: module must exist for class to be not visible"
        in
        let new_r =
          Reason.Ropaque_type_from_module (Cls.pos class_info, callee_module, r)
        in
        let cstr = MakeType.mixed new_r in
        ((env, err), mk (new_r, Tnewtype (name, [], cstr)))

and localize_typedef_instantiation
    ~ety_env env r decl_r type_name tyargs typedef_info =
  match typedef_info with
  | Some typedef_info ->
    if TypecheckerOptions.use_type_alias_heap (Env.get_tcopt env) then
      Decl_typedef_expand.expand_typedef
        (Typing_env.get_ctx env)
        decl_r
        type_name
        tyargs
      |> localize ~ety_env env
    else
      let tparams = typedef_info.Typing_defs.td_tparams in
      let nkinds = KindDefs.Simple.named_kinds_of_decl_tparams tparams in
      let ((env, e1), tyargs) =
        localize_targs_by_kind
          ~ety_env:{ ety_env with expand_visible_newtype = true }
          env
          tyargs
          nkinds
      in
      let ((env, e2), lty) =
        TUtils.expand_typedef ety_env env r type_name tyargs
      in
      let ty_err_opt = Option.merge e1 e2 ~f:Typing_error.both in
      ((env, ty_err_opt), lty)
  | None ->
    (* This must be unreachable. We only call localize_typedef_instantiation if we *know* that
       we have a typedef with typedef info at hand. *)
    failwith "Internal error: No info about typedef"

(* Localize a type with the given expected kind, which
   may either indicate a higher-kinded or fully applied type.
 *)
and localize_with_kind
    ~ety_env env (dty : decl_ty) (expected_kind : KindDefs.Simple.kind) =
  let (r, dty_) = deref dty in
  let r = Typing_reason.localize r in
  let arity = KindDefs.Simple.get_arity expected_kind in
  if Int.( = ) arity 0 then
    (* Not higher-kinded *)
    localize ~ety_env env dty
  else
    match dty_ with
    | Tapply (((_pos, name) as id), []) ->
      begin
        match Env.get_class_or_typedef env name with
        | Some (Env.ClassResult class_info) ->
          let tparams = Cls.tparams class_info in
          let classish_kind =
            KindDefs.Simple.type_with_params_to_simple_kind tparams
          in
          if Kinding.Simple.is_subkind env ~sub:classish_kind ~sup:expected_kind
          then
            ((env, None), mk (r, Tclass (id, nonexact, [])))
          else
            ((env, None), mk (Reason.none, Terr))
        | Some (Env.TypedefResult typedef) ->
          if Typing_env.is_typedef_visible env ~name typedef then
            ((env, None), mk (r, Tunapplied_alias name))
          else
            (* The bound is unused until the newtype is fully applied, thus supplying dummy Tany *)
            ( (env, None),
              mk (r, Tnewtype (name, [], mk (Reason.none, make_tany ()))) )
        | None ->
          (* We are expected to localize a higher-kinded type, but are given an unknown class name.
                Not much we can do. *)
          ((env, None), mk (Reason.none, Terr))
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
            ((env, None), mk (r, Tgeneric (name, [])))
          else
            ((env, None), mk (Reason.none, Terr))
        | None ->
          (* FIXME: Ideally, we would like to fail here, but sometimes we see type
             parameters without an entry in the environment. *)
          ((env, None), mk (r, Tgeneric (name, [])))
      end
    | Tgeneric (_, _targs)
    | Tapply (_, _targs) ->
      ((env, None), mk (Reason.none, Terr))
    | Tany _ -> ((env, None), mk (r, make_tany ()))
    | Terr -> ((env, None), mk (r, Terr))
    | _dty_ -> ((env, None), mk (Reason.none, Terr))

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

(* For the majority of cases when we localize a function type we instantiate
 * the function's type parameters to Tvars. There are two cases where we do not do this.

 * 1) In Typing_subtype.subtype_method. See the comment for that function for why
 * this is necessary.
 * 2) When the type arguments are explicitly specified, in which case we instantiate
 * the type parameters to the provided types.
 *)
and localize_ft
    ?(instantiation : method_instantiation option)
    ~ety_env
    ~def_pos
    env
    (ft : decl_ty fun_type) =
  let ((env, arity_ty_err_opt), substs) =
    match instantiation with
    | Some { explicit_targs; use_name = _; use_pos } ->
      let ty_err_opt =
        if
          (not (List.is_empty explicit_targs))
          && Int.( <> ) (List.length explicit_targs) (List.length ft.ft_tparams)
        then
          Some
            Typing_error.(
              primary
              @@ Primary.Expected_tparam
                   {
                     decl_pos = def_pos;
                     pos = use_pos;
                     n = List.length ft.ft_tparams;
                   })
        else
          None
      in
      let tvarl = List.map ~f:fst explicit_targs in
      let ft_subst = Subst.make_locl ft.ft_tparams tvarl in
      ((env, ty_err_opt), SMap.union ft_subst ety_env.substs)
    | None -> ((env, None), ety_env.substs)
  in
  let ety_env = { ety_env with substs } in
  (* Localize the constraints for a type parameter declaration *)
  let rec localize_tparam ~nested env t =
    let ((env, e1), cstrl) =
      (* TODO(T70068435)
         For nested type parameters (i.e., type parameters of type parameters),
         we do not support constraints, yet. If nested type parameters do have
         constraints, this is reported earlier. We just throw them away here. *)
      if nested then
        ((env, None), [])
      else
        List.map_env_ty_err_opt
          env
          t.tp_constraints
          ~combine_ty_errs:Typing_error.multiple_opt
          ~f:(fun env (ck, ty) ->
            let ((env, ty_err_opt), ty) =
              localize_cstr_ty ~ety_env env ty t.tp_name
            in
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
            ((env, ty_err_opt), (ck, ty)))
    in
    let ((env, e2), tparams) =
      List.map_env_ty_err_opt
        env
        t.tp_tparams
        ~f:(localize_tparam ~nested:true)
        ~combine_ty_errs:Typing_error.multiple_opt
    in
    let ty_err_opt = Option.merge e1 e2 ~f:Typing_error.both in
    ((env, ty_err_opt), { t with tp_constraints = cstrl; tp_tparams = tparams })
  in
  let localize_where_constraint env (ty1, ck, ty2) =
    let ((env, e1), ty1) = localize ~ety_env env ty1 in
    let ((env, e2), ty2) = localize ~ety_env env ty2 in
    let ty_err_opt = Option.merge e1 e2 ~f:Typing_error.both in
    ((env, ty_err_opt), (ty1, ck, ty2))
  in
  (* Grab and store the old tpenvs *)
  let old_tpenv = Env.get_tpenv env in
  let old_global_tpenv = env.tpenv in
  (* Always localize tparams so they are available for later Tast check *)
  let ((env, tparam_ty_err_opt), tparams) =
    List.map_env_ty_err_opt
      env
      ft.ft_tparams
      ~f:(localize_tparam ~nested:false)
      ~combine_ty_errs:Typing_error.multiple_opt
  in
  (* Localize the 'where' constraints *)
  let ((env, where_cstr_ty_err_opt), where_constraints) =
    List.map_env_ty_err_opt
      env
      ft.ft_where_constraints
      ~f:localize_where_constraint
      ~combine_ty_errs:Typing_error.multiple_opt
  in
  (* Remove the constraints we added for localizing where constraints  *)
  let env = Env.env_with_tpenv env old_tpenv in
  let env = Env.env_with_global_tpenv env old_global_tpenv in
  (* If we're instantiating the generic parameters then add a deferred
   * check that constraints are satisfied under the
   * substitution [ety_env.substs].
   *)
  let (env, gen_param_ty_err_opt) =
    match instantiation with
    | Some { use_pos; _ } ->
      let (env, e1) =
        check_tparams_constraints ~use_pos ~ety_env env ft.ft_tparams
      in
      let (env, e2) =
        check_where_constraints
          ~in_class:false
          ~use_pos
          ~definition_pos:def_pos
          ~ety_env
          env
          ft.ft_where_constraints
      in
      let ty_err_opt = Option.merge e1 e2 ~f:Typing_error.both in
      (env, ty_err_opt)
    | None -> (env, None)
  in
  let variadic_index =
    if get_ft_variadic ft then
      List.length ft.ft_params - 1
    else
      -1
  in
  let (((env, _), variadic_params_ty_err_opt), params) =
    List.map_env_ty_err_opt
      (env, 0)
      ft.ft_params
      ~f:(fun (env, i) param ->
        let ((env, ty_err_opt), ty) =
          localize_possibly_enforced_ty ~ety_env env param.fp_type
        in
        (* HHVM does not enforce types on vararg parameters yet *)
        let ty =
          if i = variadic_index then
            { et_type = ty.et_type; et_enforced = Unenforced }
          else
            ty
        in
        (((env, i + 1), ty_err_opt), { param with fp_type = ty }))
      ~combine_ty_errs:Typing_error.multiple_opt
  in
  let ((env, implicit_params_ty_err_opt), implicit_params) =
    let (env, capability) =
      match ft.ft_implicit_params.capability with
      | CapTy c ->
        let (env, ty) = localize ~ety_env env c in
        (env, CapTy ty)
      | CapDefaults p -> ((env, None), CapDefaults p)
    in
    (env, { capability })
  in
  let ((env, ret_ty_err_opt), ret) =
    localize_possibly_enforced_ty ~ety_env env ft.ft_ret
  in
  let ft =
    set_ft_ftk
      ft
      (if Option.is_some instantiation then
        FTKinstantiated_targs
      else
        FTKtparams)
  in
  let ty_err_opt =
    Typing_error.multiple_opt
    @@ List.filter_map
         ~f:Fn.id
         [
           arity_ty_err_opt;
           tparam_ty_err_opt;
           where_cstr_ty_err_opt;
           gen_param_ty_err_opt;
           variadic_params_ty_err_opt;
           implicit_params_ty_err_opt;
           ret_ty_err_opt;
         ]
  in
  ( (env, ty_err_opt),
    {
      ft with
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
  let check_tparam_constraints (env, ty_errs) t =
    match SMap.find_opt (snd t.tp_name) ety_env.substs with
    | Some ty ->
      List.fold_left
        t.tp_constraints
        ~init:(env, ty_errs)
        ~f:(fun (env, ty_errs) (ck, cstr_ty) ->
          let ((env, e1), cstr_ty) =
            localize_cstr_ty ~ety_env env cstr_ty t.tp_name
          in
          Typing_log.(
            log_with_level env "generics" ~level:1 (fun () ->
                log_types
                  (Pos_or_decl.of_raw_pos use_pos)
                  env
                  [
                    Log_head
                      ( "check_tparams_constraints: check_tparams_constraint",
                        [Log_type ("cstr_ty", cstr_ty); Log_type ("ty", ty)] );
                  ]));
          let (env, e2) =
            TGenConstraint.check_tparams_constraint env ~use_pos ck ~cstr_ty ty
          in
          let ty_err_opt = Option.merge e1 e2 ~f:Typing_error.both in
          let ty_errs =
            Option.value_map
              ~default:ty_errs
              ~f:(fun e -> e :: ty_errs)
              ty_err_opt
          in
          (env, ty_errs))
    | None -> (env, ty_errs)
  in
  let (env, ty_errs) =
    List.fold_left tparams ~init:(env, []) ~f:check_tparam_constraints
  in
  (env, Typing_error.multiple_opt ty_errs)

and check_where_constraints
    ~in_class ~use_pos ~ety_env ~definition_pos env cstrl =
  let (env, ty_errs) =
    List.fold_left
      cstrl
      ~init:(env, [])
      ~f:(fun (env, ty_errs) (ty, ck, cstr_ty) ->
        let ((env, e1), ty) = localize ~ety_env env ty in
        let ((env, e2), cstr_ty) = localize ~ety_env env cstr_ty in
        let (env, e3) =
          TGenConstraint.check_where_constraint
            ~in_class
            env
            ~use_pos
            ~definition_pos
            ck
            ~cstr_ty
            ty
        in
        let ty_err_opt =
          Typing_error.multiple_opt @@ List.filter_map ~f:Fn.id [e1; e2; e3]
        in
        let ty_errs =
          Option.value_map
            ~default:ty_errs
            ~f:(fun e -> e :: ty_errs)
            ty_err_opt
        in
        (env, ty_errs))
  in
  (env, Typing_error.multiple_opt ty_errs)

(**
 * If a type annotation for a class is missing type arguments, create a type variable
 * for them and apply constraints.
 *
 * This is not used for local inference on constructors, but rather for global inference
 * for partial files where function signatures are allowed to contain types ommitting type
 * arguments.
 *)
and localize_missing_tparams_class_for_global_inference env r sid class_ =
  let use_name = Utils.strip_ns (snd sid) in
  let tparams = Cls.tparams class_ in
  let ((env, _i), tyl) =
    List.fold_map tparams ~init:(env, 0) ~f:(fun (env, i) tparam ->
        let (env, ty) =
          Env.new_global_tyvar
            env
            ~i
            (Reason.Rglobal_type_variable_generics
               (Reason.to_pos r, snd tparam.tp_name, use_name))
        in
        ((env, i + 1), ty))
  in
  let c_ty = mk (r, Tclass (sid, nonexact, tyl)) in
  let ety_env =
    {
      empty_expand_env with
      this_ty = c_ty;
      substs = Subst.make_locl tparams tyl;
      on_error = Some (Typing_error.Reasons_callback.unify_error_at Pos.none);
    }
  in
  let (env, e1) =
    check_tparams_constraints ~use_pos:Pos.none ~ety_env env tparams
  in
  let constraints = Cls.where_constraints class_ in
  let (env, e2) =
    if List.is_empty constraints then
      (env, None)
    else
      check_where_constraints
        ~in_class:true
        ~use_pos:Pos.none
        ~definition_pos:(Cls.pos class_)
        ~ety_env
        env
        constraints
  in
  let ty_err_opt = Option.merge e1 e2 ~f:Typing_error.both in
  ((env, ty_err_opt), tyl)

and localize_refinement ~ety_env env r root decl_cr =
  let mk_unsupported_err () =
    let pos = Reason.to_pos r in
    Option.map ety_env.on_error ~f:(fun on_error ->
        Typing_error.(
          apply_reasons ~on_error (Secondary.Unsupported_class_refinement pos)))
  in
  let ((env, ty_err_opt), root) = localize ~ety_env env root in
  match get_node root with
  | Tclass (cid, Nonexact cr, tyl) ->
    let both_err e1 e2 = Option.merge e1 e2 ~f:Typing_error.both in
    let ((env, ty_err_opt), cr) =
      Class_refinement.fold_type_refs
        decl_cr
        ~init:((env, ty_err_opt), cr)
        ~f:(fun id tr ((env, ty_err_opt), cr) ->
          match tr with
          | TRexact ty ->
            let ((env, ty_err_opt'), ty) = localize ~ety_env env ty in
            let cr = Class_refinement.add_type_ref id (TRexact ty) cr in
            ((env, both_err ty_err_opt ty_err_opt'), cr)
          | TRloose _ ->
            let err = mk_unsupported_err () in
            ((env, both_err ty_err_opt err), cr))
    in
    ((env, ty_err_opt), mk (r, Tclass (cid, Nonexact cr, tyl)))
  | _ -> ((env, mk_unsupported_err ()), TUtils.terr env r)

(* Like localize_no_subst, but uses the supplied kind, enabling support
   for higher-kinded types *)
let localize_no_subst_and_kind env ~on_error ty kind =
  let ety_env =
    Option.value_map
      ~default:empty_expand_env
      ~f:empty_expand_env_with_on_error
      on_error
  in
  localize_with_kind ~ety_env env ty kind

(** Localize an explicit type argument to a constructor or function. We
    support the use of wildcards at the top level only *)
let localize_targ_with_kind
    ~check_well_kinded env hint (nkind : KindDefs.Simple.named_kind) =
  (* For explicit type arguments we support a wildcard syntax `_` for which
   * Hack will generate a fresh type variable *)
  let kind = snd nkind in
  match hint with
  | (_, Aast.Happly ((p, id), [])) when String.equal id SN.Typehints.wildcard ->
    let is_higher_kinded = KindDefs.Simple.get_arity kind > 0 in
    if is_higher_kinded then
      let ty_err = Typing_error.(primary @@ Primary.HKT_wildcard (fst hint)) in
      ((env, Some ty_err), (mk (Reason.none, Terr), hint))
    else
      let (env, ty) = Env.fresh_type env p in
      ((env, None), (ty, hint))
  | (hint_pos, _) ->
    let ty = Decl_hint.hint env.decl_env hint in
    if check_well_kinded then
      Kinding.Simple.check_well_kinded ~in_signature:false env ty nkind;
    let (env, ty) =
      localize_no_subst_and_kind
        env
        ~on_error:
          (Some (Typing_error.Reasons_callback.invalid_type_hint hint_pos))
        ty
        kind
    in
    (env, (ty, hint))

let localize_targ ~check_well_kinded env hint =
  let named_kind =
    KindDefs.Simple.with_dummy_name (KindDefs.Simple.fully_applied_type ())
  in
  localize_targ_with_kind ~check_well_kinded env hint named_kind

(* See signature in .mli file for details *)
let localize_targs_with_kinds
    ~check_well_kinded
    ~is_method
    ~def_pos
    ~use_pos
    ~use_name
    ?(check_explicit_targs = true)
    ?(tparaml = [])
    env
    named_kinds
    targl =
  let targ_count = List.length targl in
  let generated_tparam_count =
    List.count
      ~f:(fun t -> SN.Coeffects.is_generated_generic (snd t.tp_name))
      tparaml
  in
  let tparam_count =
    match List.length tparaml with
    | 0 -> List.length named_kinds
    | n -> n
  in
  let explicit_tparam_count = tparam_count - generated_tparam_count in

  let checking_rewritten_call () =
    (* Typing_phase expands the case of targl=[] to a list of wildcards matching the
     * length of tparaml, but some `if` condition checks retype already typed
     * expressions, so we get the generated list instead of what the user wrote
     * TODO(coeffects) attempt to remove Tast.to_nast_expr calls *)
    generated_tparam_count > 0
    && targ_count = tparam_count
    && List.for_all
         ~f:(function
           | a ->
             (match a with
             | (_, Aast.Happly ((_, n), _)) ->
               String.equal n SN.Typehints.wildcard
             | _ -> false))
         targl
  in
  (* If there are explicit type arguments but too few or too many then
   * report an error *)
  let open Int in
  let arity_ty_err_opt =
    if
      not
        (targ_count = 0
        || targ_count = explicit_tparam_count
        || checking_rewritten_call ())
    then
      if is_method then
        Some
          Typing_error.(
            primary
            @@ Primary.Expected_tparam
                 {
                   decl_pos = def_pos;
                   pos = use_pos;
                   n = explicit_tparam_count;
                 })
      else
        Some
          Typing_error.(
            primary
            @@ Primary.Type_arity_mismatch
                 {
                   pos = use_pos;
                   decl_pos = def_pos;
                   expected = tparam_count;
                   actual = targ_count;
                 })
    else
      None
  in

  (* Declare and localize the explicit type arguments *)
  let targl =
    if targ_count > tparam_count then
      List.take targl tparam_count
    else
      targl
  in
  let ((env, ty_errs), explicit_targs) =
    List.map2_env
      (env, [])
      targl
      (List.take named_kinds targ_count)
      ~f:(fun (env, ty_errs) x y ->
        let ((env, ty_err_opt), res) =
          localize_targ_with_kind ~check_well_kinded env x y
        in
        let ty_errs =
          Option.value_map ty_err_opt ~default:ty_errs ~f:(fun e ->
              e :: ty_errs)
        in
        ((env, ty_errs), res))
  in
  let explicit_targ_ty_err_opt = Typing_error.multiple_opt ty_errs in
  (* Generate fresh type variables for the remainder *)
  let ((env, implicit_targ_ty_err_opt), implicit_targs) =
    let mk_implicit_targ env (kind_name, kind) =
      let wildcard_hint =
        (use_pos, Aast.Happly ((Pos.none, SN.Typehints.wildcard), []))
      in
      if
        check_well_kinded
        && KindDefs.Simple.get_arity kind > 0
        && targ_count = 0
      then
        (* We only throw an error if the user didn't provide any type arguments at all.
           Otherwise, if they provided some, but not all of them, n arity mismatch
           triggers earlier in this function, independently from higher-kindedness *)
        let ty_err =
          Typing_error.(
            primary
            @@ Primary.HKT_implicit_argument
                 {
                   pos = use_pos;
                   decl_pos = fst kind_name;
                   param_name = snd kind_name;
                 })
        in
        ((env, Some ty_err), (mk (Reason.none, Terr), wildcard_hint))
      else
        let (env, tvar) =
          Env.fresh_type_reason
            env
            use_pos
            (Reason.Rtype_variable_generics (use_pos, snd kind_name, use_name))
        in
        Typing_log.log_tparam_instantiation env use_pos (snd kind_name) tvar;
        ((env, None), (tvar, wildcard_hint))
    in
    List.map_env_ty_err_opt
      env
      (List.drop named_kinds targ_count)
      ~f:mk_implicit_targ
      ~combine_ty_errs:Typing_error.multiple_opt
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
      let (decl_pos, param_name) = tparam.tp_name in
      Some
        Typing_error.(
          primary
          @@ Primary.Require_generic_explicit
               { decl_pos; param_name; pos = fst hint })
    else
      None
  in

  let ua_ty_err_opt =
    if check_explicit_targs then
      Typing_error.multiple_opt
      @@ List.fold2_exn
           tparaml
           (explicit_targs @ implicit_targs)
           ~init:[]
           ~f:(fun ty_errs tp targ ->
             Option.value_map ~default:ty_errs ~f:(fun e -> e :: ty_errs)
             @@ check_for_explicit_user_attribute tp targ)
    else
      None
  in
  let ty_err_opt =
    Typing_error.multiple_opt
    @@ List.filter_map
         ~f:Fn.id
         [
           arity_ty_err_opt;
           explicit_targ_ty_err_opt;
           implicit_targ_ty_err_opt;
           ua_ty_err_opt;
         ]
  in
  ((env, ty_err_opt), explicit_targs @ implicit_targs)

let localize_targs
    ~check_well_kinded
    ~is_method
    ~def_pos
    ~use_pos
    ~use_name
    ?(check_explicit_targs = true)
    env
    tparaml
    targl =
  let nkinds = KindDefs.Simple.named_kinds_of_decl_tparams tparaml in
  localize_targs_with_kinds
    ~check_well_kinded
    ~is_method
    ~def_pos
    ~use_pos
    ~use_name
    ~tparaml
    ~check_explicit_targs
    env
    nkinds
    targl

(* Performs no substitutions of generics and initializes Tthis to
 * Env.get_self env
 *)
let localize_no_subst_ env ~on_error ?report_cycle ty =
  let ety_env =
    {
      empty_expand_env with
      type_expansions =
        Typing_defs.Type_expansions.empty_w_cycle_report ~report_cycle;
      on_error;
    }
  in
  localize env ty ~ety_env

let localize_hint_no_subst env ~ignore_errors ?report_cycle h =
  let (pos, _) = h in
  let h = Decl_hint.hint env.decl_env h in
  localize_no_subst_
    env
    ~on_error:
      (if ignore_errors then
        None
      else
        Some (Typing_error.Reasons_callback.invalid_type_hint pos))
    ?report_cycle
    h

let localize_no_subst env ~ignore_errors ty =
  localize_no_subst_
    env
    ~on_error:
      (if ignore_errors then
        None
      else
        Some
          (Typing_error.Reasons_callback.invalid_type_hint
             (Pos_or_decl.unsafe_to_raw_pos @@ get_pos ty)))
    ty

let localize_possibly_enforced_no_subst env ~ignore_errors ety =
  let (env, et_type) = localize_no_subst env ~ignore_errors ety.et_type in
  (env, { ety with et_type })

let localize_targs_and_check_constraints
    ~exact
    ~check_well_kinded
    ~def_pos
    ~use_pos
    ?(check_explicit_targs = true)
    env
    class_id
    r
    tparaml
    hintl =
  let ((env, e1), type_argl) =
    localize_targs
      ~check_well_kinded
      ~is_method:false
      ~def_pos
      ~use_pos
      ~use_name:(Utils.strip_ns (snd class_id))
      ~check_explicit_targs
      env
      tparaml
      hintl
  in
  let targs_tys = List.map ~f:fst type_argl in
  let this_ty =
    mk (r, Tclass (Positioned.of_raw_positioned class_id, exact, targs_tys))
  in
  let ety_env =
    {
      empty_expand_env with
      this_ty;
      substs = Subst.make_locl tparaml targs_tys;
      on_error = Some (Typing_error.Reasons_callback.unify_error_at use_pos);
    }
  in
  let (env, e2) = check_tparams_constraints ~use_pos ~ety_env env tparaml in
  let ty_err_opt = Option.merge e1 e2 ~f:Typing_error.both in
  ((env, ty_err_opt), this_ty, type_argl)

(* Add generic parameters to the environment, localize their bounds, and
 * transform these into a flat list of constraints of the form (ty1,ck,ty2)
 * where ck is as, super or =
 *)
let localize_and_add_generic_parameters_with_bounds
    ~ety_env (env : env) (tparams : decl_tparam list) =
  let env = Env.add_generic_parameters env tparams in
  let localize_bound
      env ({ tp_name = (pos, name); tp_constraints = cstrl; _ } : decl_tparam) =
    (* TODO(T70068435) This may have to be touched when adding support for constraints on HK
       types *)
    let tparam_ty = mk (Reason.Rwitness_from_decl pos, Tgeneric (name, [])) in
    List.map_env_ty_err_opt
      env
      cstrl
      ~f:(fun env (ck, cstr) ->
        let (env, ty) = localize env cstr ~ety_env in
        (env, (tparam_ty, ck, ty)))
      ~combine_ty_errs:Typing_error.multiple_opt
  in
  let ((env, ty_err_opt), cstrss) =
    List.map_env_ty_err_opt
      env
      tparams
      ~f:localize_bound
      ~combine_ty_errs:Typing_error.multiple_opt
  in
  let add_constraint env (ty1, ck, ty2) =
    Typing_utils.add_constraint env ck ty1 ty2 ety_env.on_error
  in
  let env = List.fold_left (List.concat cstrss) ~f:add_constraint ~init:env in
  (env, ty_err_opt)

let localize_and_add_where_constraints ~ety_env (env : env) where_constraints =
  let localize_and_add_constraint (env, ty_errs) (ty1, ck, ty2) =
    let ((env, e1), ty1) = localize env ty1 ~ety_env in
    let ((env, e2), ty2) = localize env ty2 ~ety_env in
    let env = TUtils.add_constraint env ck ty1 ty2 ety_env.on_error in
    let ty_errs =
      Option.(
        value_map ~default:ty_errs ~f:(fun e -> e :: ty_errs)
        @@ merge e1 e2 ~f:Typing_error.both)
    in
    (env, ty_errs)
  in
  let (env, ty_errs) =
    List.fold_left
      where_constraints
      ~f:localize_and_add_constraint
      ~init:(env, [])
  in
  (env, Typing_error.multiple_opt ty_errs)

(* Helper functions *)

let sub_type_decl env ty1 ty2 on_error =
  let ((env, e1), ty1) = localize_no_subst env ~ignore_errors:true ty1 in
  let ((env, e2), ty2) = localize_no_subst env ~ignore_errors:true ty2 in
  let (env, e3) = TUtils.sub_type env ty1 ty2 on_error in
  let ty_err_opt =
    Typing_error.multiple_opt @@ List.filter_map ~f:Fn.id [e1; e2; e3]
  in
  (env, ty_err_opt)

let localize_and_add_generic_parameters_and_where_constraints
    ~ety_env env tparams where_constraints =
  let (env, e1) =
    localize_and_add_generic_parameters_with_bounds env tparams ~ety_env
  in
  let (env, e2) =
    localize_and_add_where_constraints env where_constraints ~ety_env
  in
  (env, Option.merge e1 e2 ~f:Typing_error.both)

let localize_and_add_ast_generic_parameters_and_where_constraints
    env
    ~ignore_errors
    (tparams : Nast.tparam list)
    (where_constraints : Aast_defs.where_constraint_hint list) =
  let tparams : decl_tparam list =
    List.map tparams ~f:(Decl_hint.aast_tparam_to_decl_tparam env.decl_env)
  in
  let where_constraints : decl_where_constraint list =
    List.map where_constraints ~f:(fun (h1, ck, h2) ->
        (Decl_hint.hint env.decl_env h1, ck, Decl_hint.hint env.decl_env h2))
  in
  let ety_env =
    if ignore_errors then
      empty_expand_env
    else
      empty_expand_env_with_on_error
        (Env.invalid_type_hint_assert_primary_pos_in_current_decl env)
  in
  localize_and_add_generic_parameters_and_where_constraints
    ~ety_env
    env
    tparams
    where_constraints

let () = TUtils.localize_no_subst_ref := localize_no_subst

let () = TUtils.localize_ref := localize
