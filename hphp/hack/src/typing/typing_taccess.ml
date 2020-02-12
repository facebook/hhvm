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
module MakeType = Typing_make_type

(* A guiding principle when expanding a type access C::T is that if C <: D and
   we know that D::T = X (represented by an Exact result below), then C::T is
   also X. So Exact is propagated down the <: relation, see `update_class_name`
   below where this behavior is encoded. *)

type context = {
  (* The T in the type access C::T *)
  id: Nast.sid;
  (* The expand environment as passed in by Typing_phase.localize *)
  ety_env: expand_env;
  (* A set of visited types used to avoid infinite loops during expansion. *)
  generics_seen: TySet.t;
  (* Whether or not an abstract type constant is allowed as the result. In the
     future, this boolean should disappear and abstract type constants should
     appear only in the class where they are defined. *)
  allow_abstract: bool;
  (* If set, abstract type constants will be expanded as type variables. This
     is a hack which should naturally go away when the semantics of abstract
     type constants is cleaned up. *)
  abstract_as_tyvar: bool;
  (* The origin of the extension. For example if TC is a generic parameter
     subject to the constraint TC as C and we would like to expand TC::T we
     will expand C::T with base set to `Some (Tgeneric "TC")` (and root set
     to C). If it is None the base is exactly the current root. *)
  base: locl_ty option;
  (* A callback for errors *)
  on_error: Errors.typing_error_callback;
}

(* The result of an expansion
   - Exact ty means that the expansion results precisely in 'ty'
   - Abstract (n0, [n1, n2, n3], bound) means that the result is a
     generic with name n0::T such that:
     n0::T as n1::T as n2::T as n3::T as bound *)
type result =
  | Exact of locl_ty
  | Abstract of string * string list * locl_ty option

exception NoTypeConst of (unit -> unit)

let raise_error error = raise_notrace @@ NoTypeConst error

let make_reason env r id root =
  Reason.Rtypeconst (r, id, Typing_print.error env root, get_reason root)

(* FIXME: It is bogus to use strings here and put them in Tgeneric; one
   possible problem is when a type parameter has a name which conflicts
   with a class name *)
let tp_name class_name id = class_name ^ "::" ^ snd id

(* A smart constructor for Abstract that also checks if the type we are
   creating is known to be equal to some other type *)
let make_abstract env id name namel bnd =
  let tp_name = tp_name name id in
  if not (Typing_set.is_empty (Env.get_equal_bounds env tp_name)) then
    (* If the resulting abstract type is exactly equal to something,
       mark the result as exact.
       For example, if we have the following
       abstract class Box {
         abstract const type T;
       }
       function addFiveToValue<T1 as Box>(T1 $x) : int where T1::T = int {
           return $x->get() + 5;
       }
       Here, $x->get() has type expr#1::T as T1::T (as Box::T).
       But T1::T is exactly equal to int, so $x->get() no longer needs
       to be expression dependent. Thus, $x->get() typechecks. *)
    Exact (MakeType.generic Reason.Rnone tp_name)
  else
    Abstract (name, namel, bnd)

(* Lookup a type constant in a class and return a result. A type constant has
   both a constraint type and assigned type. Which one we choose depends if
   the current root is the base (origin) of the expansion, or if it is an
   upper bound of the base. *)
let create_root_from_type_constant ctx env root (class_pos, class_name) =
  let { id = (id_pos, id_name) as id; _ } = ctx in
  let class_ =
    match Env.get_class env class_name with
    | None ->
      raise_error (fun () -> Errors.unbound_name_typing class_pos class_name)
    | Some c -> c
  in
  let typeconst =
    match Env.get_typeconst env class_ id_name with
    | Some tc -> tc
    | None ->
      raise_error (fun () ->
          Errors.smember_not_found
            `class_typeconst
            (get_pos root)
            (Cls.pos class_, class_name)
            id_name
            `no_hint
            ctx.on_error)
  in
  let name = tp_name class_name id in
  let type_expansions = (id_pos, name) :: ctx.ety_env.type_expansions in
  ( if
    List.mem ~equal:String.equal (List.map ctx.ety_env.type_expansions snd) name
  then
    let seen = List.rev_map type_expansions snd in
    raise_error (fun () ->
        Errors.cyclic_typeconst (fst typeconst.ttc_name) seen) );
  let drop_exact ty =
    (* Legacy behavior is to preserve exactness only on `this` and not
       through `this::T` *)
    match deref ty with
    | (r, Tclass (cid, _, tyl)) -> mk (r, Tclass (cid, Nonexact, tyl))
    | _ -> ty
  in
  let ety_env =
    let from_class = None in
    let this_ty = drop_exact (Option.value ctx.base ~default:root) in
    { ctx.ety_env with from_class; type_expansions; this_ty }
  in
  let make_abstract env bnd =
    ( if (not ctx.allow_abstract) && not ety_env.quiet then
      let tc_pos = fst typeconst.ttc_name in
      Errors.abstract_tconst_not_allowed id_pos (tc_pos, id_name) );
    (* TODO(T59448452): this treatment of abstract type constants is unsound *)
    make_abstract env id class_name [] bnd
  in
  match typeconst with
  (* Concrete type constants *)
  | { ttc_type = Some ty; ttc_constraint = None; _ } ->
    let (env, ty) = Phase.localize ~ety_env env ty in
    let (r, ty) = deref ty in
    (env, Exact (mk (make_reason env r id root, ty)))
  (* A type constant with default can be seen as abstract or exact, depending
     on the root and base of the access. *)
  | { ttc_type = Some ty; ttc_constraint = Some _; _ } ->
    let (env, ty) = Phase.localize ~ety_env env ty in
    let (r, ty) = deref ty in
    let ty = mk (make_reason env r id root, ty) in
    if Cls.final class_ || Option.is_none ctx.base then
      (env, Exact ty)
    else
      (env, make_abstract env (Some ty))
  (* Abstract type constants with constraint *)
  | { ttc_constraint = Some cstr; _ } ->
    let (env, cstr) = Phase.localize ~ety_env env cstr in
    (env, make_abstract env (Some cstr))
  (* Abstract type constant without constraint. *)
  | _ -> (env, make_abstract env None)

let rec type_of_result ctx env root res =
  let { id = (id_pos, id_name) as id; _ } = ctx in
  let type_with_bound env as_tyvar name bnd =
    if as_tyvar then (
      let (env, tvar) = Env.fresh_invariant_type_var env id_pos in
      Log.log_new_tvar_for_tconst_access env id_pos tvar name id_name;
      (env, tvar)
    ) else
      let generic_name = tp_name name id in
      let reason = make_reason env Reason.Rnone id root in
      let ty = MakeType.generic reason generic_name in
      let env =
        Option.fold bnd ~init:env ~f:(fun env bnd ->
            (* TODO(T59317869): play well with flow sensitivity *)
            Env.add_upper_bound_global env generic_name bnd)
      in
      (env, ty)
  in
  match res with
  | Exact ty -> (env, ty)
  | Abstract (name, name' :: namel, bnd) ->
    let res' = Abstract (name', namel, bnd) in
    let (env, ty) = type_of_result ctx env root res' in
    type_with_bound env false name (Some ty)
  | Abstract (name, [], bnd) ->
    type_with_bound env ctx.abstract_as_tyvar name bnd

let update_class_name env id new_name = function
  | Exact _ as res -> res
  | Abstract (name, namel, bnd) ->
    make_abstract env id new_name (name :: namel) bnd

let rec expand ctx env root =
  let (env, root) = Env.expand_type env root in
  let make_reason env = make_reason env Reason.Rnone ctx.id root in
  match get_node root with
  | Tany _
  | Terr ->
    (env, Exact root)
  | Tdependent (DTcls name, ty)
  | Tnewtype (name, _, ty) ->
    let ctx =
      let base = Some (Option.value ctx.base ~default:root) in
      let allow_abstract = true in
      { ctx with base; allow_abstract }
    in
    let (env, res) = expand ctx env ty in
    let name = Printf.sprintf "<cls#%s>" name in
    (env, update_class_name env ctx.id name res)
  | Tclass (cls, _, _) ->
    let allow_abstract =
      match Env.get_class env (snd cls) with
      | Some ci
        when Ast_defs.(equal_class_kind (Decl_provider.Class.kind ci) Ctrait) ->
        (* Hack: `self` in a trait is mistakenly replaced by the trait instead
           of the class using the trait, so if a trait is the root, it is
           likely because originally there was `self::T` written.
           TODO(T54081153): fix `self` in traits and clean this up *)
        true
      | _ -> ctx.allow_abstract
    in
    let ctx = { ctx with allow_abstract } in
    create_root_from_type_constant ctx env root cls
  | Tgeneric s ->
    let ctx =
      let generics_seen = TySet.add root ctx.generics_seen in
      let base = Some (Option.value ctx.base ~default:root) in
      let allow_abstract = true in
      let abstract_as_tyvar = false in
      { ctx with generics_seen; base; allow_abstract; abstract_as_tyvar }
    in
    let rec last_res res err = function
      | [] -> (res, err)
      | ty :: tys ->
        let (res', err) =
          try (Some (expand ctx env ty), err)
          with NoTypeConst err -> (res, err)
        in
        (* The strategy here is to take the last result. It is necessary for
           poor reasons, unfortunately. Because `type_of_result` bogusly uses
           `Env.add_upper_bound_global`, local type refinement information can
           leak outside its scope. To remain consistent with the previous
           version of the type access algorithm wrt this bug, we pick the last
           result. See T59317869.
           The test test/typecheck/tconst/type_refinement_stress.php monitors
           the situation here. *)
        last_res (Option.first_some res' res) err tys
    in
    let err () =
      let (pos, tconst) = ctx.id in
      let ty = Typing_print.error env root in
      Errors.non_object_member
        ~is_method:false
        tconst
        (get_pos root)
        ty
        pos
        ctx.on_error
    in
    (* Ignore seen bounds to avoid infinite loops *)
    let upper_bounds =
      TySet.diff (Env.get_upper_bounds env s) ctx.generics_seen
    in
    (match last_res None err (TySet.elements upper_bounds) with
    | (Some (env, res), _) -> (env, update_class_name env ctx.id s res)
    | (None, err) -> raise_error err)
  | Tdependent (dep_ty, ty) ->
    let ctx =
      let base = Some (Option.value ctx.base ~default:root) in
      let allow_abstract = true in
      let abstract_as_tyvar = false in
      { ctx with base; allow_abstract; abstract_as_tyvar }
    in
    let (env, res) = expand ctx env ty in
    (env, update_class_name env ctx.id (DependentKind.to_string dep_ty) res)
  | Tunion tyl ->
    (* TODO(T58839232): accesses on unions are unsound *)
    let (env, tyl) =
      List.map_env env tyl ~f:(fun env ty ->
          let (env, res) = expand ctx env ty in
          type_of_result ctx env root res)
    in
    let ty = MakeType.union (make_reason env) tyl in
    (env, Exact ty)
  | Tintersection tyl ->
    let (env, tyl) =
      Typing_utils.run_on_intersection env tyl ~f:(fun env ty ->
          let (env, res) = expand ctx env ty in
          type_of_result ctx env root res)
    in
    let (env, ty) = Inter.intersect_list env (make_reason env) tyl in
    (env, Exact ty)
  | Tvar n ->
    let (env, ty) = Typing_subtype_tconst.get_tyvar_type_const env n ctx.id in
    (env, Exact ty)
  (* TODO(T36532263): Pocket Universes *)
  | Tpu (base, _)
  | Tpu_type_access (base, _, _, _) ->
    let pos = get_pos base in
    raise_error (fun _ -> Errors.pu_expansion pos)
  | Tobject
  | Tnonnull
  | Tprim _
  | Tshape _
  | Ttuple _
  | Tarraykind _
  | Tfun _
  | Tdynamic
  | Toption _ ->
    let (pos, tconst) = ctx.id in
    let ty = Typing_print.error env root in
    raise_error (fun () ->
        Errors.non_object_member
          ~is_method:false
          tconst
          pos
          ty
          (get_pos root)
          ctx.on_error)

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
    try
      let ctx =
        {
          id;
          ety_env;
          base = None;
          generics_seen = TySet.empty;
          allow_abstract = allow_abstract_tconst;
          abstract_as_tyvar = as_tyvar_with_cnstr;
          on_error;
        }
      in
      let (env, res) = expand ctx env root in
      type_of_result ctx env root res
    with NoTypeConst error ->
      if not ignore_errors then error ();
      let reason = make_reason env Reason.Rnone id root in
      (env, Typing_utils.terr env reason)
  in
  (* If type constant has type this::ID and method has associated condition
     type ROOTCOND_TY for the receiver - check if condition type has type
     constant at the same path.  If yes - attach a condition type
     ROOTCOND_TY::ID to a result type *)
  match
    ( deref root,
      id,
      TR.condition_type_from_reactivity (Typing_env_types.env_reactivity env) )
  with
  | ((_, Tdependent (DTthis, _)), (_, tconst), Some cond_ty) ->
    begin
      match CT.try_get_class_for_condition_type env cond_ty with
      | Some (_, cls) when Cls.has_typeconst cls tconst ->
        let cond_ty = mk (Reason.Rwitness (fst id), Taccess (cond_ty, [id])) in
        Option.value
          (TR.try_substitute_type_with_condition env cond_ty ty)
          ~default:(env, ty)
      | _ -> (env, ty)
    end
  | _ -> (env, ty)

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
              match get_node ty with
              | Tclass ((_, class_name), _, _) ->
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
