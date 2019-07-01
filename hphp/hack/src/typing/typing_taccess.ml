(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Common
open Typing_defs
open Typing_dependent_type

module Inter = Typing_intersection
module Reason = Typing_reason
module Env = Typing_env
module Log = Typing_log
module Phase = Typing_phase
module TySet = Typing_set
module TR = Typing_reactivity
module CT = Typing_subtype.ConditionTypes
module Cls = Decl_provider.Class

exception NoTypeConst of (unit -> unit)

let raise_error error = raise_notrace @@ NoTypeConst error

type env = {
  tenv : Env.env;
  ety_env : Phase.env;

  (* A list of dependent we have encountered while expanding a type constant.
   * After expanding a type constant we can choose either the assigned type or
   * the constrained type. If we choose the assigned type, the result will not
   * be expression dependent so this list will be set to empty. However, if it
   * is the constrained type then the final type will also be a dependent type.
   *)
  dep_tys : (Reason.t * dependent_type) list;

  (* A list of generics we've seen while expanding. *)
  gen_seen : TySet.t;
}

let empty_env env ety_env = {
  tenv = env;
  ety_env;
  dep_tys = [];
  gen_seen = TySet.empty;
}

let make_reason env r id ty =
  Reason.Rtypeconst (
    r,
    id,
    Typing_print.error env ty,
    fst ty
  )

(** Expand a type constant access like A::T
If as_tyvar_with_cnstr is set, then return a fresh type variable which has
the same constraints as type constant T in A. Otherwise, return an
AKGeneric("A::T"). *)
let rec expand_with_env ety_env env ?(as_tyvar_with_cnstr = false) root id =
  let tenv, _, ty =
    expand_with_env_ ety_env env ~as_tyvar_with_cnstr root id in
  tenv, ty

and expand_with_env_ ety_env env ~as_tyvar_with_cnstr root id =
  let env = empty_env env ety_env in
  let env, ty =
    try expand env ~as_tyvar_with_cnstr root id
    with NoTypeConst error ->
      error ();
      env, (make_reason env.tenv Reason.Rnone id root, Typing_utils.terr env.tenv)
  in
  let tenv, ty = ExprDepTy.apply env.tenv env.dep_tys [snd id] ty in
  let tenv, ty =
    (* if type constant has type this::ID and method has associated condition type ROOTCOND_TY
       for the receiver - check if condition type has type constant at the same path.
       If yes - attach a condition type ROOTCOND_TY::ID to a result type *)
       begin match root, id, TR.condition_type_from_reactivity (Env.env_reactivity tenv) with
       | (_, Tabstract (AKdependent (`this), _)),
         (_, tconst),
         Some cond_ty ->
         begin match CT.try_get_class_for_condition_type tenv cond_ty with
         | Some (_, cls) when Cls.has_typeconst cls tconst ->
          let cond_ty = (Reason.Rwitness (fst id), Taccess (cond_ty, [id])) in
          Option.value (TR.try_substitute_type_with_condition tenv cond_ty ty)
            ~default:(tenv, ty)
         | _ ->  tenv, ty
         end
       | _ -> tenv, ty
       end in

  tenv, env, ty

and referenced_typeconsts env ety_env (root, ids) =
  let env, root = Phase.localize ~ety_env env root in
  List.fold ids ~init:((env, root), []) ~f:begin fun ((env, root), acc) (pos, tconst) ->
    let env, tyl = Typing_utils.get_concrete_supertypes env root in
    let acc = List.fold tyl ~init:acc ~f:begin fun acc ty ->
      match snd ty with
      | Typing_defs.Tclass ((_, class_name), _, _) ->
        let (>>=) = Option.(>>=) in
        Option.value ~default:acc begin
          Typing_env.get_class env class_name >>= fun class_ ->
          Typing_env.get_typeconst env class_ tconst >>= fun typeconst ->
          Some ((typeconst.Typing_defs.ttc_origin, tconst, pos) :: acc)
        end
      | _ -> acc
    end in
    expand_with_env ety_env env ~as_tyvar_with_cnstr:false root (pos, tconst), acc
  end
  |> snd

(* The root of a type access is a type. When expanding a type access this type
 * needs to resolve to the name of a class so we can look up if a given type
 * constant is defined in the class.
 *
 * We also need to track what expansions have already taken place to make sure
 * we do not recurse infinitely.
 *)
and expand env ~as_tyvar_with_cnstr root id =
  let expand env ty = expand env ~as_tyvar_with_cnstr ty id in
  let tenv, (root_reason, root_ty as root) = Env.expand_type env.tenv root in
  (* The type constant itself may contain a 'this' type so we need to
   * change the this_ty in the environment to be the root as an
   * expression dependent type.
   *)
  let update_this_ty env =
    let this_ty = match root with
    | _ when env.dep_tys <> [] -> env.ety_env.this_ty
    (* Legacy behaviour is to preserve exactness only on `this`
     * and not through `this::T` *)
    | r, Tclass (cid, _, tyl) -> r, Tclass (cid, Nonexact, tyl)
    | _ -> root
    in
    { env with ety_env = { env.ety_env with this_ty } }
  in
  let env = update_this_ty { env with tenv } in
  let make_reason env r = make_reason env r id root in
  let env = { env with tenv = tenv } in
  let expand_and_apply_dep env ty =
    let env, ty = expand env ty in
    (* If ty here involves a type access, we have to use
      the current environment's dependent types. Otherwise,
      we throw away type access information.
    *)
    let tenv, ty = ExprDepTy.apply env.tenv env.dep_tys [snd id] ty in
    { env with tenv }, ty in
  match root_ty with
  | Tany | Terr -> env, root
  | Tabstract (AKdependent (`cls _), Some ty)
  | Tabstract (AKnewtype (_, _), Some ty) -> expand env ty
  | Tclass ((class_pos, class_name), _, _) ->
    create_root_from_type_constant
      env root class_pos class_name
      id ~as_tyvar_with_cnstr
  | Tabstract (AKgeneric s, _) ->
    let dep_ty = `cls s in
    let env =
      { env with
        dep_tys = (make_reason tenv Reason.Rnone, dep_ty)::env.dep_tys;
        gen_seen = TySet.add root env.gen_seen;
      }
    in
    let upper_bounds = Env.get_upper_bounds env.tenv s in
    (* Ignore upper bounds that are equal to ones we've seen, to avoid
      an infinite loop

      let upper_bounds = upper_bounds - env.gen_seen
    *)
    let upper_bounds = TySet.diff upper_bounds env.gen_seen in
    (* We will attempt to look up the type constant for each upper bound.
      If at least one reports a type then we ignore the errors of any other
      failed look ups.
    *)
    let env, tyl, errors =
      List.fold ~init:(env, [], []) (TySet.elements upper_bounds)
        ~f:begin fun (prev_env, tys, errors) ty ->
        try
          let env, ty = expand env ty in
          (* If ty here involves a type access, we have to use
            the current environment's dependent types. Otherwise,
            we throw away type access information.
          *)
          let tenv, ty = ExprDepTy.apply env.tenv env.dep_tys [snd id] ty in
          { prev_env with tenv }, ty::tys, errors
        with
          NoTypeConst error -> prev_env, tys, error::errors
    end in
    begin match tyl with
    | [] ->
      raise_error begin match List.hd errors with
      | Some error -> error
      | None ->
        let pos, tconst = id in
        let ty = Typing_print.error env.tenv root in
        (fun () ->
          Errors.non_object_member tconst (Reason.to_pos root_reason) ty pos
        )
      end
    | ty::_ ->
      { env with dep_tys = [] }, ty
    end
  | Tabstract (AKdependent dep_ty, Some ty) ->
    let env =
      { env with
        dep_tys = (make_reason tenv Reason.Rnone, dep_ty)::env.dep_tys;
      }
    in
    expand env ty
  | Tunion tyl ->
    let env, tyl = List.map_env env tyl ~f:expand_and_apply_dep in
    { env with dep_tys = [] } , (make_reason tenv Reason.Rnone, Tunion tyl)
  | Tintersection tyl ->
    let env, tyl = Typing_utils.run_on_intersection env tyl ~f:expand_and_apply_dep in
    let tenv, ty = Inter.intersect_list env.tenv root_reason tyl in
    { env with dep_tys = []; tenv } , ty
  | Tvar n ->
    let tenv, ty = Typing_subtype_tconst.get_tyvar_type_const env.tenv n id in
    { env with tenv }, ty
  | Tanon _ | Tobject | Tnonnull | Tprim _ | Tshape _ | Ttuple _
  | Tarraykind _ | Tfun _ | Tabstract (_, _)  | Tdynamic | Toption _ ->
    let pos, tconst = id in
    let ty = Typing_print.error env.tenv root in
    raise_error (fun () ->
      Errors.non_object_member tconst pos ty (Reason.to_pos root_reason)
    )

(* The function takes a "step" forward in the expansion. We look up the type
 * constant associated with the given class_name and create a new root type.
 * A type constant has both a constraint type and assigned type. Which one we
 * choose depends on if there are any dependent types set in the environment.
 * If the root type is not a dependent type then we choose the assigned type,
 * otherwise we choose the constraint type. If there is no constraint type then
 * we choose the assigned type.
 *)
and create_root_from_type_constant env root class_pos class_name
  (pos, tconst) ~as_tyvar_with_cnstr =
  let env, typeconst =
    get_typeconst env class_pos class_name pos tconst ~as_tyvar_with_cnstr
  in
  let ety_env =
    { env.ety_env with
      from_class = None; } in
  let make_reason ?(root = ety_env.this_ty) env r = make_reason env r (pos, tconst) root in
  match typeconst with
  | { ttc_type = Some ty; _ }
      when typeconst.ttc_constraint = None || env.dep_tys = [] ->
      let tenv, (r, ty) = Phase.localize ~ety_env env.tenv ty in
      { env with dep_tys = []; tenv = tenv },
      (make_reason tenv r ~root, ty)
  | {ttc_constraint = Some cstr; _} ->
      let tenv, cstr = Phase.localize ~ety_env env.tenv cstr in
      let dep_ty =
        make_reason tenv (Reason.Rwitness (fst typeconst.ttc_name)),
        `cls class_name
      in
      let tenv, ty =
        if as_tyvar_with_cnstr then
          let tenv, tvar = Env.fresh_invariant_type_var tenv pos in
          Log.log_new_tvar_for_tconst_access tenv pos tvar class_name tconst;
          let tenv = Typing_utils.sub_type tenv tvar cstr in
          tenv, tvar
        else tenv, cstr in
      { env with dep_tys = dep_ty::env.dep_tys; tenv }, ty
  | _ ->
      let tenv, (r, ty) =
        if as_tyvar_with_cnstr then
          let tenv, tvar = Env.fresh_invariant_type_var env.tenv pos in
          Log.log_new_tvar_for_tconst_access tenv pos tvar class_name tconst;
          tenv, tvar
        else
          let reason = Reason.Rwitness (fst typeconst.ttc_name) in
          let ty = (reason, Tabstract (AKgeneric (class_name^"::"^tconst), None)) in
          env.tenv, ty in
      { env with tenv },
      (make_reason tenv r, ty)

(* Looks up the type constant within the given class. This also checks for
 * potential cycles by examining the expansions we have already performed.
 *)
and get_typeconst env class_pos class_name pos tconst ~as_tyvar_with_cnstr =
  let class_ = match Env.get_class env.tenv class_name with
    | None ->
        raise_error (fun () -> Errors.unbound_name_typing class_pos class_name)
    | Some c -> c in
  let typeconst = match Env.get_typeconst env.tenv class_ tconst with
    | None ->
        raise_error (fun () ->
          if not as_tyvar_with_cnstr then
            Errors.smember_not_found
              `class_typeconst pos ((Cls.pos class_), class_name) tconst `no_hint)
    | Some tc -> tc in
  (* Check for cycles. We do this by combining the name of the current class
   * with the remaining ids that we need to expand. If we encounter the same
   * class name + ids that means we have entered a cycle.
   *)
  let cur_tconst = `cls class_name, [tconst] in
  let seen = ExprDepTy.to_string cur_tconst in
  let type_expansions = (pos, seen)::env.ety_env.type_expansions in
  if List.mem ~equal:(=) (List.map env.ety_env.type_expansions snd) seen then
    begin
      let seen = List.rev_map type_expansions snd in
      raise_error (fun () -> Errors.cyclic_typeconst (fst typeconst.ttc_name) seen)
    end;
  (* If the class is final then we do not need to create dependent types
   * because the type constant cannot be overridden by a child class
   *)
  { env with
    ety_env = { env.ety_env with type_expansions };
    dep_tys = if (Cls.final class_) then []  else env.dep_tys;
  },
  typeconst

(*****************************************************************************)
(* Exporting *)
(*****************************************************************************)

let () = Typing_utils.expand_typeconst_ref := expand_with_env
