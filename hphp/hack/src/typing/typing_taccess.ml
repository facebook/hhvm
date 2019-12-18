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
module Inter = Typing_intersection
module Reason = Typing_reason
module Env = Typing_env
module Log = Typing_log
module Phase = Typing_phase
module TySet = Typing_set
module TR = Typing_reactivity
module CT = Typing_subtype.ConditionTypes
module Cls = Decl_provider.Class

type result =
  | Exact of locl_ty
  (* The result of the expansion can be overwritten by sub-classes; this is
     the case where the type constant was declared using
     const type T as X = Y;
     Only Y is returned in that case since Y <: X *)
  | Default of Pos.t * locl_ty
  (* The result of the expansion is abstract; the name stored is of the form
     X::T where X is a class with an abstract type constant named T. Type
     constants can only have an upper bound, so one locl_ty is enough here. *)
  | Abstract of Pos.t * locl_ty option

exception NoTypeConst of (unit -> unit)

let raise_error error = raise_notrace @@ NoTypeConst error

type env = {
  tenv: Typing_env_types.env;
  choose_assigned_type: bool;
}

let empty_env env = { tenv = env; choose_assigned_type = true }

let make_reason env r id ty =
  Reason.Rtypeconst (r, id, Typing_print.error env ty, fst ty)

(* The function takes a "step" forward in the expansion. We look up the type
 * constant associated with the given class_name and create a new root type.
 * A type constant has both a constraint type and assigned type. Which one we
 * choose depends on if there are any dependent types set in the environment.
 * If the root type is not a dependent type then we choose the assigned type,
 * otherwise we choose the constraint type. If there is no constraint type then
 * we choose the assigned type.
 *)
let create_root_from_type_constant
    ety_env
    env
    ((root_ty_r, _) as root)
    class_pos
    class_name
    (tconst_pos, tconst)
    ~on_error (* ~as_tyvar_with_cnstr *)
              (* ~allow_abstract_tconst *) =
  let class_ =
    match Env.get_class env class_name with
    | None ->
      raise_error (fun () -> Errors.unbound_name_typing class_pos class_name)
    | Some c -> c
  in
  let typeconst =
    match Env.get_typeconst env class_ tconst with
    | Some tc -> tc
    | None ->
      raise_error (fun () ->
          Errors.smember_not_found
            `class_typeconst
            (Reason.to_pos root_ty_r)
            (Cls.pos class_, class_name)
            tconst
            `no_hint
            on_error)
  in
  let ty_name = class_name ^ "::" ^ tconst in
  let ety_env = { ety_env with from_class = None } in
  (* Check for cycles. We do this by combining the name of the current class
   * with the remaining ids that we need to expand. If we encounter the same
   * class name + ids that means we have entered a cycle.
   *)
  let type_expansions = (tconst_pos, ty_name) :: ety_env.type_expansions in
  ( if
    List.mem ~equal:String.equal (List.map ety_env.type_expansions snd) ty_name
  then
    let seen = List.rev_map type_expansions snd in
    raise_error (fun () ->
        Errors.cyclic_typeconst (fst typeconst.ttc_name) seen) );
  let ety_env = { ety_env with type_expansions } in
  let make_reason ?(root = ety_env.this_ty) env r =
    make_reason env r (tconst_pos, tconst) root
  in
  match typeconst with
  (* Concrete type constants *)
  | { ttc_type = Some ty; ttc_constraint = None; _ } ->
    let (env, (r, ty)) = Phase.localize ~ety_env env ty in
    (env, Exact (make_reason env r ~root, ty))
  | { ttc_type = Some ty; ttc_constraint = Some _; ttc_name = (tc_pos, _); _ }
    ->
    let (env, (r, ty)) = Phase.localize ~ety_env env ty in
    let ty = (make_reason env r ~root, ty) in
    if Cls.final class_ then
      (env, Exact ty)
    else
      (env, Default (tc_pos, ty))
  (* Abstract type constants with constraint *)
  | { ttc_constraint = Some cstr; ttc_name = (tc_pos, _); _ } ->
    let (env, cstr) = Phase.localize ~ety_env env cstr in
    (env, Abstract (tc_pos, Some cstr))
  (* Abstract type constant without constraint. *)
  | { ttc_name = (tc_pos, _); _ } -> (env, Abstract (tc_pos, None))

(* The root of a type access is a type. When expanding a type access, this type
 * needs to resolve to the name of a class so we can look up if a given type
 * constant is defined in the class.
 *
 * We also need to track what expansions have already taken place to make sure
 * we do not recurse infinitely.
 *)
let rec expand
    ety_env
    env
    generics_seen
    ~as_tyvar_with_cnstr
    root
    id
    ~on_error
    ~allow_abstract_tconst =
  let (tenv, ((root_reason, root_ty) as root)) =
    Env.expand_type env.tenv root
  in
  (* The type constant itself may contain a 'this' type so we need to
   * change the this_ty in the environment to be the root as an
   * expression dependent type.
   *)
  let ety_env =
    let this_ty =
      match root with
      | _ when not env.choose_assigned_type -> ety_env.this_ty
      (* Legacy behaviour is to preserve exactness only on `this`
       * and not through `this::T` *)
      | (r, Tclass (cid, _, tyl)) -> (r, Tclass (cid, Nonexact, tyl))
      | _ -> root
    in
    { ety_env with this_ty }
  in
  let make_reason env r = make_reason env r id root in
  let make_ty env name = function
    | ty when env.choose_assigned_type -> (env, ty)
    (* If the generic in question is exactly equal to something, the
    expression dependent type collapses to that given type, since
    all constraints of the expression dependent type will get
    transferred to the lower type. *)
    (* For example, if we have the following
      abstract class Box {
        abstract const type T;
      }
      class IntBox { const type T = int; }
      function addFiveToValue<T1 as Box>(T1 $x) : int where T1::T = int {
          return $x->get() + 5;
      }
      Here, $x->get() has type expr#1::T as T1::T as Box::T.
      But T1::T is exactly equal to int, so $x->get() no longer needs
      to be expression dependent. Thus, $x->get() typechecks.
    *)
    | (_, Tgeneric s) as ty
      when not (Typing_set.is_empty (Env.get_equal_bounds env.tenv s)) ->
      (env, ty)
    | ty ->
      let ty_name = name ^ "::" ^ snd id in
      let new_ty = (make_reason env.tenv Reason.Rnone, Tgeneric ty_name) in
      let tenv = Env.add_upper_bound_global env.tenv ty_name ty in
      ({ env with tenv }, new_ty)
  in
  let env = { env with tenv } in
  let allow_abstract_tconst =
    match root_ty with
    | Tgeneric _
    | Tnewtype _
    | Tdependent _ ->
      true
    (* Hack: `self` in a trait is mistakenly replaced by the trait instead of
    the class using the trait, so if a trait is the root, it is likely because
    originally there was `self::T` written.
    TODO: T54081153 fix `self` in traits and clean this up *)
    | Tclass ((_, class_name), _, _)
      when match Env.get_class env.tenv class_name with
           | Some ci ->
             Ast_defs.(equal_class_kind (Decl_provider.Class.kind ci) Ctrait)
           | None -> false ->
      true
    | _ -> allow_abstract_tconst
  in
  let expand = expand ~allow_abstract_tconst in
  match root_ty with
  | Tany _
  | Terr ->
    (env, root)
  | Tdependent (DTcls _, ty)
  | Tnewtype (_, _, ty) ->
    expand ety_env env generics_seen ~on_error ~as_tyvar_with_cnstr ty id
  | Tclass ((class_pos, class_name), _, _) ->
    let make_abstract env tc_pos bndo =
      let name = class_name ^ "::" ^ snd id in
      if as_tyvar_with_cnstr then (
        let (env, tvar) = Env.fresh_invariant_type_var env (fst id) in
        Log.log_new_tvar_for_tconst_access env (fst id) tvar class_name (snd id);
        (env, tvar)
      ) else
        let reason = make_reason env (Reason.Rwitness tc_pos) in
        let ty = (reason, Tgeneric name) in
        let env =
          Option.fold bndo ~init:env ~f:(fun env bnd ->
              Env.add_upper_bound_global env name bnd)
        in
        (env, ty)
    in
    let (tenv, res) =
      create_root_from_type_constant
        ety_env
        env.tenv
        root
        class_pos
        class_name
        id
        ~on_error
    in
    let env = { env with tenv } in
    (match res with
    | Exact ty -> ({ env with choose_assigned_type = true }, ty)
    | Default (tc_pos, ty) ->
      if env.choose_assigned_type then
        (env, ty)
      else
        let (tenv, ty) = make_abstract env.tenv tc_pos (Some ty) in
        ({ env with tenv }, ty)
    | Abstract (tc_pos, bnd) ->
      if (not allow_abstract_tconst) && not ety_env.quiet then
        Errors.abstract_tconst_not_allowed (fst id) (tc_pos, snd id);
      let (tenv, ty) = make_abstract env.tenv tc_pos bnd in
      ({ env with tenv }, ty))
  | Tgeneric s ->
    let generics_seen = TySet.add root generics_seen in
    let env = { env with choose_assigned_type = false } in
    let upper_bounds = Env.get_upper_bounds env.tenv s in

    (* Ignore upper bounds that are equal to ones we've seen, to avoid
       an infinite loop *)
    let upper_bounds = TySet.diff upper_bounds generics_seen in
    (* We will attempt to look up the type constant for each upper bound.
      If at least one reports a type then we ignore the errors of any other
      failed look ups.
    *)
    let (env, tyl, errors) =
      List.fold
        ~init:(env, [], [])
        (TySet.elements upper_bounds)
        ~f:(fun (prev_env, tys, errors) ty ->
          try
            let (env, ty) =
              expand
                ety_env
                env
                generics_seen
                ~on_error
                ~as_tyvar_with_cnstr:false
                ty
                id
            in
            (* If ty here involves a type access, we have to use
            the current environment's dependent types. Otherwise,
            we throw away type access information.
          *)
            let (env, ty) = make_ty env s ty in
            ( { prev_env with tenv = env.tenv },
              (env.choose_assigned_type, ty) :: tys,
              errors )
          with NoTypeConst error -> (prev_env, tys, error :: errors))
    in
    begin
      match tyl with
      | [] ->
        raise_error
          begin
            match List.hd errors with
            | Some error -> error
            | None ->
              let (pos, tconst) = id in
              let ty = Typing_print.error env.tenv root in
              fun () ->
                Errors.non_object_member
                  ~is_method:false
                  tconst
                  (Reason.to_pos root_reason)
                  ty
                  pos
                  on_error
          end
      | (choose_assigned_type, ty) :: _ ->
        ({ env with choose_assigned_type }, ty)
    end
  | Tdependent (dep_ty, ty) ->
    let env = { env with choose_assigned_type = false } in
    let as_tyvar_with_cnstr = false in
    let (env, ty) =
      expand ety_env env generics_seen ~on_error ~as_tyvar_with_cnstr ty id
    in
    make_ty env (DependentKind.to_string dep_ty) ty
  | Tunion tyl ->
    let (env, tyl) =
      List.map_env env tyl ~f:(fun env ty ->
          expand ety_env env generics_seen ~on_error ~as_tyvar_with_cnstr ty id)
    in
    (env, (make_reason tenv Reason.Rnone, Tunion tyl))
  | Tintersection tyl ->
    let (env, tyl) =
      Typing_utils.run_on_intersection env tyl ~f:(fun env ty ->
          expand ety_env env generics_seen ~on_error ~as_tyvar_with_cnstr ty id)
    in
    let (tenv, ty) =
      Inter.intersect_list env.tenv (make_reason tenv Reason.Rnone) tyl
    in
    ({ env with tenv }, ty)
  | Tvar n ->
    let (tenv, ty) = Typing_subtype_tconst.get_tyvar_type_const env.tenv n id in
    ({ env with tenv }, ty)
  (* TODO(T36532263): Pocket Universes *)
  | Tpu (base, _)
  | Tpu_type_access (base, _, _, _) ->
    let reason = fst base in
    let pos = Reason.to_pos reason in
    raise_error (fun _ -> Errors.pu_expansion pos)
  | Tanon _
  | Tobject
  | Tnonnull
  | Tprim _
  | Tshape _
  | Ttuple _
  | Tarraykind _
  | Tfun _
  | Tdynamic
  | Toption _ ->
    let (pos, tconst) = id in
    let ty = Typing_print.error env.tenv root in
    raise_error (fun () ->
        Errors.non_object_member
          ~is_method:false
          tconst
          pos
          ty
          (Reason.to_pos root_reason)
          on_error)

(** Expand a type constant access like A::T
If as_tyvar_with_cnstr is set, then return a fresh type variable which has
the same constraints as type constant T in A. Otherwise, return an
AKGeneric("A::T"). *)
let expand_with_env
    ety_env
    env
    ?(ignore_errors = false)
    ?(as_tyvar_with_cnstr = false)
    root
    id
    ~on_error
    ~allow_abstract_tconst =
  let (env, ty) =
    let env = empty_env env in
    try
      expand
        ety_env
        env
        TySet.empty
        ~as_tyvar_with_cnstr
        root
        id
        ~on_error
        ~allow_abstract_tconst
    with NoTypeConst error ->
      if not ignore_errors then error ();
      ( env,
        (make_reason env.tenv Reason.Rnone id root, Typing_utils.terr env.tenv)
      )
  in
  let tenv = env.tenv in
  (* if type constant has type this::ID and method has associated condition type ROOTCOND_TY
     for the receiver - check if condition type has type constant at the same path.
     If yes - attach a condition type ROOTCOND_TY::ID to a result type *)
  match
    ( root,
      id,
      TR.condition_type_from_reactivity (Typing_env_types.env_reactivity tenv)
    )
  with
  | ((_, Tdependent (DTthis, _)), (_, tconst), Some cond_ty) ->
    begin
      match CT.try_get_class_for_condition_type tenv cond_ty with
      | Some (_, cls) when Cls.has_typeconst cls tconst ->
        let cond_ty = (Reason.Rwitness (fst id), Taccess (cond_ty, [id])) in
        Option.value
          (TR.try_substitute_type_with_condition tenv cond_ty ty)
          ~default:(tenv, ty)
      | _ -> (tenv, ty)
    end
  | _ -> (tenv, ty)

let referenced_typeconsts env ety_env (root, ids) ~on_error =
  let (env, root) = Phase.localize ~ety_env env root in
  List.fold
    ids
    ~init:((env, root), [])
    ~f:
      begin
        fun ((env, root), acc) (pos, tconst) ->
        let (env, tyl) = Typing_utils.get_concrete_supertypes env root in
        let acc =
          List.fold tyl ~init:acc ~f:(fun acc ty ->
              let (env, ty) = Env.expand_type env ty in
              match snd ty with
              | Typing_defs.Tclass ((_, class_name), _, _) ->
                let ( >>= ) = Option.( >>= ) in
                Option.value
                  ~default:acc
                  ( Typing_env.get_class env class_name >>= fun class_ ->
                    Typing_env.get_typeconst env class_ tconst
                    >>= fun typeconst ->
                    Some ((typeconst.Typing_defs.ttc_origin, tconst, pos) :: acc)
                  )
              | _ -> acc)
        in
        ( expand_with_env
            ety_env
            env
            ~as_tyvar_with_cnstr:false
            root
            (pos, tconst)
            ~on_error
            ~allow_abstract_tconst:true,
          acc )
      end
  |> snd

(*****************************************************************************)
(* Exporting *)
(*****************************************************************************)

let () = Typing_utils.expand_typeconst_ref := expand_with_env
