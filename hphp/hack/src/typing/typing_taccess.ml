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
open Utils

module Reason = Typing_reason
module Env = Typing_env
module Phase = Typing_phase
module TySet = Typing_set
module TR = Typing_reactivity
module CT = Typing_subtype.ConditionTypes

type env = {
  tenv : Env.env;
  ety_env : Phase.env;
  (* A trail of all the type constants we have expanded. Used primarily for
   * error reporting
   *)
  trail : dependent_type list;

  (* A list of dependent we have encountered while expanding a type constant.
   * After expanding a type constant we can choose either the assigned type or
   * the constrained type. If we choose the assigned type, the result will not
   * be expression dependent so this list will be set to empty. However, if it
   * is the constrained type then the final type will also be a dependent type.
   *)
  dep_tys : (Reason.t * dependent_type) list;
  (* The remaining type constants we need to expand *)
  ids : Nast.sid list;

  (* A list of generics we've seen while expanding. *)
  gen_seen : TySet.t;

  (* The identifiers for each class and typeconst pair seen while expanding,
   * along with the location where the typeconst was referenced. *)
  typeconsts_seen : (string * string * Pos.t) list;
}

let empty_env env ety_env ids = {
  tenv = env;
  ety_env = ety_env;
  trail = [];
  dep_tys = [];
  ids = ids;
  gen_seen = TySet.empty;
  typeconsts_seen = [];
}

let rec expand_with_env ety_env env reason root ids =
  let tenv, env, ty = expand_with_env_ ety_env env reason root ids in
  tenv, (env.ety_env, ty)

and expand_with_env_ ety_env env reason root ids =
  let env = empty_env env ety_env ids in
  let env, (root_r, root_ty) = expand env root in
  let trail = List.rev_map env.trail (compose strip_ns ExprDepTy.to_string) in
  let reason_func r =
    let r = match r with
      | Reason.Rexpr_dep_type (_, p, e) ->
          Reason.Rexpr_dep_type (root_r, p, e)
      | _ -> r in
    Reason.Rtype_access(reason, trail, r) in
  let ty = reason_func root_r, root_ty in
  let deps = List.map env.dep_tys (fun (x, y) -> reason_func x, y) in
  let tenv, ty = ExprDepTy.apply env.tenv deps ty in
  let tenv, ty =
    (* if type constant has type this::ID and method has associated condition type ROOTCOND_TY
       for the receiver - check if condition type has type constant at the same path.
       If yes - attach a condition type ROOTCOND_TY::ID to a result type *)
       begin match root, ids, TR.condition_type_from_reactivity (Env.env_reactivity tenv) with
       | (_, Tabstract (AKdependent (`static, []),
                       Some (_, Tabstract (AKdependent (`this, []), _)))),
         [_, id],
         Some cond_ty ->
         begin match CT.try_get_class_for_condition_type tenv cond_ty with
         | Some (_, cls) when SMap.mem id cls.tc_typeconsts ->
          let cond_ty = (Reason.none, Taccess (cond_ty, ids)) in
          Option.value (TR.try_substitute_type_with_condition tenv cond_ty ty)
            ~default:(tenv, ty)
         | _ ->  tenv, ty
         end
       | _ -> tenv, ty
       end in

  tenv, env, ty

and referenced_typeconsts tenv ety_env r (root, ids) =
  let tenv, (ety_env, root) = Phase.localize_with_env ~ety_env tenv root in
  let _, env, _ = expand_with_env_ ety_env tenv r root ids in
  List.rev env.typeconsts_seen

(* The root of a type access is a type. When expanding a type access this type
 * needs to resolve to the name of a class so we can look up if a given type
 * constant is defined in the class.
 *
 * We also need to track what expansions have already taken place to make sure
 * we do not recurse infinitely.
 *)
and expand env (root_reason, root_ty as root) =
  match env.ids with
  | [] ->
      env, root
  | head::tail -> begin match root_ty with
      | Tany | Terr -> env, root
      | Tabstract (AKdependent (`cls _, []), Some ty)
      | Tabstract (AKnewtype (_, _), Some ty) | Toption ty -> expand env ty
      | Tclass ((class_pos, class_name), _) ->
          let env, ty =
            create_root_from_type_constant
              env class_pos class_name root head in
          expand { env with ids = tail } ty
      | Tabstract (AKgeneric s, _) ->
        let dep_ty = generic_to_dep_ty s in
        let env =
          { env with
            dep_tys = (root_reason, dep_ty)::env.dep_tys;
            gen_seen = TySet.add root env.gen_seen;
            } in
        let upper_bounds = Env.get_upper_bounds env.tenv s in
        (* Ignore upper bounds that are equal to ones we've seen, to avoid
          an infinite loop

          let upper_bounds = upper_bounds - env.gen_seen
        *)
        let upper_bounds = TySet.diff upper_bounds env.gen_seen in
        let env, tyl = List.map_env env (TySet.elements upper_bounds)
          begin fun prev_env ty ->
          let env, ty = expand env ty in
          (* If ty here involves a type access, we have to use
            the current environment's dependent types. Otherwise,
            we throw away type access information.
          *)
          let tenv, ty = ExprDepTy.apply env.tenv env.dep_tys ty in
          { prev_env with tenv }, ty
        end in
        begin match tyl with
        | [] ->
          let pos, tconst = head in
          let ty = Typing_print.error root_ty in
          Errors.non_object_member tconst (Reason.to_pos root_reason) ty pos;
          env, (root_reason, Terr)
        | ty::_ ->
          { env with dep_tys = [] }, ty
        end
      | Tabstract (AKdependent dep_ty, Some ty) ->
          let env =
            { env with
              dep_tys = (root_reason, dep_ty)::env.dep_tys } in
          expand env ty
      | Tunresolved tyl ->
          let env, tyl = List.map_env env tyl begin fun prev_env ty ->
            let env, ty = expand env ty in
            (* If ty here involves a type access, we have to use
              the current environment's dependent types. Otherwise,
              we throw away type access information.
            *)
            let tenv, ty = ExprDepTy.apply env.tenv env.dep_tys ty in
            { prev_env with tenv }, ty
          end in
          { env with dep_tys = [] } , (root_reason, Tunresolved tyl)
      | Tvar _ ->
          let tenv, ty =
            Env.expand_type env.tenv root in
          let env = { env with tenv = tenv } in
          expand env ty
      | Tanon _ | Tobject | Tmixed | Tnonnull | Tprim _ | Tshape _ | Ttuple _
      | Tarraykind _ | Tfun _ | Tabstract (_, _)  | Tdynamic ->
          let pos, tconst = head in
          let ty = Typing_print.error root_ty in
          Errors.non_object_member tconst (Reason.to_pos root_reason) ty pos;
          env, (root_reason, Terr)
     end
 and generic_to_dep_ty s =
  let regexp = Str.regexp "::" in
  let res = Str.split regexp s in
  match res with
  | name::tys -> `cls name, tys
  | [] -> `cls s, []
(* The function takes a "step" forward in the expansion. We look up the type
 * constant associated with the given class_name and create a new root type.
 * A type constant has both a constraint type and assigned type. Which one we
 * choose depends on if there are any dependent types set in the environment.
 * If the root type is not a dependent type then we choose the assigned type,
 * otherwise we choose the constraint type. If there is no constraint type then
 * we choose the assigned type.
 *)
and create_root_from_type_constant env class_pos class_name root_ty (pos, tconst) =
  match get_typeconst env class_pos class_name pos tconst with
  | None -> env, (fst root_ty, Typing_utils.tany env.tenv)
  | Some (env, typeconst) ->
      let env =
        { env with
          trail = (`cls class_name, List.map env.ids snd)::env.trail } in
      (* The type constant itself may contain a 'this' type so we need to
       * change the this_ty in the environment to be the root as an
       * expression dependent type.
       *)
      let tenv, new_this = ExprDepTy.apply env.tenv env.dep_tys root_ty in
      let env = { env with tenv } in
      let ety_env =
        { env.ety_env with
          this_ty = new_this;
          from_class = None; } in
      begin
        match typeconst with
        | { ttc_type = Some ty; _ }
            when typeconst.ttc_constraint = None || env.dep_tys = [] ->
            let tenv, ty = Phase.localize ~ety_env env.tenv ty in
            { env with dep_tys = []; tenv = tenv }, ty
        | {ttc_constraint = Some cstr; _} ->
            let tenv, cstr = Phase.localize ~ety_env env.tenv cstr in
            let dep_ty = Reason.Rwitness (fst typeconst.ttc_name),
                         (`cls class_name, [tconst]) in
            (* Append the name of the expanded type constant to each dependent
             * type.
             *)
            let dep_tys =
              List.map env.dep_tys (fun (r, (d, s)) -> r, (d, s @ [tconst])) in
            { env with dep_tys = dep_ty::dep_tys; tenv = tenv }, cstr
        | _ ->
            let ty =
              Reason.Rwitness (fst typeconst.ttc_name),
              Tabstract (AKgeneric (class_name^"::"^tconst), None) in
              let dep_tys =
              List.map env.dep_tys (fun (r, (d, s)) -> r, (d, s @ [tconst])) in
            { env with dep_tys = dep_tys }, ty
      end

(* Looks up the type constant within the given class. This also checks for
 * potential cycles by examining the expansions we have already performed.
 *)
and get_typeconst env class_pos class_name pos tconst =
  try
    let class_ = match Env.get_class env.tenv class_name with
      | None ->
          Errors.unbound_name_typing class_pos class_name;
          raise Exit
      | Some c -> c in
    let typeconst = match Env.get_typeconst env.tenv class_ tconst with
      | None ->
          Errors.smember_not_found
            `class_typeconst pos (class_.tc_pos, class_name) tconst `no_hint;
          raise Exit
      | Some tc -> tc in
    let tc_tuple = (class_.tc_name, snd typeconst.ttc_name, pos) in
    let env = {env with typeconsts_seen = tc_tuple :: env.typeconsts_seen} in
    (* Check for cycles. We do this by combining the name of the current class
     * with the remaining ids that we need to expand. If we encounter the same
     * class name + ids that means we have entered a cycle.
     *)
    let cur_tconst = `cls class_name, List.map env.ids snd in
    let seen = ExprDepTy.to_string cur_tconst in
    let type_expansions = (pos, seen)::env.ety_env.type_expansions in
    if List.mem ~equal:(=) (List.map env.ety_env.type_expansions snd) seen then
      begin
        let seen = List.rev_map type_expansions snd in
        Errors.cyclic_typeconst (fst typeconst.ttc_name) seen;
        raise Exit
      end;
    (* If the class is final then we do not need to create dependent types
     * because the type constant cannot be overridden by a child class
     *)
    let env =
      { env with
        ety_env = { env.ety_env with type_expansions };
        dep_tys = if class_.tc_final then []  else env.dep_tys;
      } in
    Some (env, typeconst)
  with
    Exit -> None

(*****************************************************************************)
(* Exporting *)
(*****************************************************************************)

let () = Typing_utils.expand_typeconst_ref := expand_with_env
