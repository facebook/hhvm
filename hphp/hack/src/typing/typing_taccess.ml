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
module Reason = Typing_reason
module Env = Typing_env
module Log = Typing_log
module Phase = Typing_phase
module TySet = Typing_set
module Cls = Decl_provider.Class
module MakeType = Typing_make_type

(* A guiding principle when expanding a type access C::T is that if C <: D and
   we know that D::T = X (represented by an Exact result below), then C::T is
   also X. So Exact is propagated down the <: relation, see `update_class_name`
   below where this behavior is encoded. *)

type context = {
  id: pos_id;  (** The T in the type access C::T *)
  root_pos: Pos_or_decl.t;
  ety_env: expand_env;
      (** The expand environment as passed in by Typing_phase.localize *)
  generics_seen: TySet.t;
      (** A set of visited types used to avoid infinite loops during expansion. *)
  allow_abstract: bool;
      (** Whether or not an abstract type constant is allowed as the result. In the
          future, this boolean should disappear and abstract type constants should
          appear only in the class where they are defined. *)
  abstract_as_tyvar_at_pos: Pos.t option;
      (** If set, abstract type constants will be expanded as type variables. This
          is a hack which should naturally go away when the semantics of abstract
          type constants is cleaned up. *)
  base: locl_ty option;
      (** The origin of the extension. For example if TC is a generic parameter
          subject to the constraint TC as C and we would like to expand TC::T we
          will expand C::T with base set to `Some (Tgeneric "TC")` (and root set
          to C). If it is None the base is exactly the current root. *)
}

(** The result of an expansion
   - Missing err means that the type constant is not present, with error function
     to be called if we need to report this
   - Exact ty means that the expansion results precisely in 'ty'
   - Abstract (n0, [n1, n2, n3], lower_bound, upper_bound) means that the result is a
     generic with name n0::T such that:
     n0::T as n1::T as n2::T as n3::T as upper_bound super lower_bound *)
type result =
  | Missing of (unit -> unit)
  | Exact of locl_ty
  | Abstract of abstract

and abstract = {
  name: string;
  names: string list;
  lower_bounds: TySet.t;
  upper_bounds: TySet.t;
}

let make_reason env id root r =
  Reason.Rtypeconst (r, id, Typing_print.error env root, get_reason root)

(* FIXME: It is bogus to use strings here and put them in Tgeneric; one
   possible problem is when a type parameter has a name which conflicts
   with a class name *)
let tp_name class_name id = class_name ^ "::" ^ snd id

(** A smart constructor for Abstract that also checks if the type we are
    creating is known to be equal to some other type *)
let abstract_or_exact env id ({ name; _ } as abstr) =
  let tp_name = tp_name name id in
  if not (Typing_set.is_empty (Env.get_equal_bounds env tp_name [])) then
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
    Abstract abstr

(** Lookup a type constant in a class and return a result. A type constant has
    both a constraint type and assigned type. Which one we choose depends if
    the current root is the base (origin) of the expansion, or if it is an
    upper bound of the base. *)
let create_root_from_type_constant ctx env root (_class_pos, class_name) class_
    =
  let { id = (id_pos, id_name) as id; _ } = ctx in
  match Env.get_typeconst env class_ id_name with
  | None ->
    ( env,
      Missing
        (fun () ->
          Errors.smember_not_found_
            `class_typeconst
            id_pos
            (Cls.pos class_, class_name)
            id_name
            `no_hint
            ctx.ety_env.on_error) )
  | Some typeconst ->
    let name = tp_name class_name id in
    let (ety_env, has_cycle) =
      Typing_defs.add_type_expansion_check_cycles ctx.ety_env (id_pos, name)
    in
    let ctx = { ctx with ety_env } in
    (match has_cycle with
    | Some initial_taccess_pos_opt ->
      Option.iter initial_taccess_pos_opt ~f:(fun initial_taccess_pos ->
          let seen =
            Typing_defs.Type_expansions.ids ctx.ety_env.type_expansions
          in
          Errors.cyclic_typeconst initial_taccess_pos seen);
      (* This is a cycle through a type constant that we are using *)
      (env, Missing (fun () -> ()))
    | None ->
      let drop_exact ty =
        (* Legacy behavior is to preserve exactness only on `this` and not
       through `this::T` *)
        map_ty ty ~f:(function
            | Tclass (cid, _, tyl) -> Tclass (cid, Nonexact, tyl)
            | ty -> ty)
      in
      let ety_env =
        {
          ctx.ety_env with
          this_ty = drop_exact (Option.value ctx.base ~default:root);
        }
      in
      let abstract_or_exact env ~lower_bounds ~upper_bounds =
        ( if not ctx.allow_abstract then
          let tc_pos = fst typeconst.ttc_name in
          Errors.abstract_tconst_not_allowed
            id_pos
            (tc_pos, id_name)
            ctx.ety_env.on_error );
        (* TODO(T59448452): this treatment of abstract type constants is unsound *)
        abstract_or_exact
          env
          id
          { name = class_name; names = []; lower_bounds; upper_bounds }
      in
      let to_tys env = function
        | Some ty ->
          let (env, ty) = Phase.localize ~ety_env env ty in
          (env, TySet.singleton ty)
        | None -> (env, TySet.empty)
      in
      (* Don't report errors in expanded definition or constraint.
       * These will have been reported at the definition site already. *)
      let ety_env = { ety_env with on_error = Errors.ignore_error } in
      (match typeconst with
      (* Concrete type constants *)
      | { ttc_type = Some ty; ttc_as_constraint = None; _ } ->
        let (env, ty) = Phase.localize ~ety_env env ty in
        let ty = map_reason ty ~f:(make_reason env id root) in
        (env, Exact ty)
      (* A type constant with default can be seen as abstract or exact, depending
       * on the root and base of the access. *)
      | {
       ttc_type = Some ty;
       ttc_as_constraint = Some _;
       ttc_super_constraint;
       _;
      } ->
        let (env, ty) = Phase.localize ~ety_env env ty in
        let ty = map_reason ty ~f:(make_reason env id root) in
        if Cls.final class_ || Option.is_none ctx.base then
          (env, Exact ty)
        else
          let (env, lower_bounds) = to_tys env ttc_super_constraint in
          let upper_bounds = TySet.singleton ty in
          (env, abstract_or_exact env ~lower_bounds ~upper_bounds)
      (* Abstract type constant without constraint. *)
      | { ttc_as_constraint = None; ttc_super_constraint = None; _ } ->
        ( env,
          abstract_or_exact
            env
            ~lower_bounds:TySet.empty
            ~upper_bounds:TySet.empty )
      (* Abstract type constants with constraint *)
      | { ttc_as_constraint = upper; ttc_super_constraint = lower; _ } ->
        let (env, lower_bounds) = to_tys env lower in
        let (env, upper_bounds) = to_tys env upper in
        (env, abstract_or_exact env ~lower_bounds ~upper_bounds)))

(* Cheap intersection operation. Do not call Typing_intersection.intersect
 * as this calls into subtype which in turn calls into expand_with_env below
 *)
let intersect ty1 ty2 =
  if equal_locl_ty ty1 ty2 then
    ty1
  else
    MakeType.intersection (get_reason ty1) [ty1; ty2]

(* Cheap union operation. Do not call Typing_union.union
 * as this calls into subtype which in turn calls into expand_with_env below
 *)
let union ty1 ty2 =
  if equal_locl_ty ty1 ty2 then
    ty1
  else
    MakeType.union (get_reason ty1) [ty1; ty2]

(* Given the results of projecting a type constant from types t1 , ... , tn,
 * determine the result of projecting a type constant from type (t1 | ... | tn).
 * If the type constant is missing from any of the disjuncts, then it's treated
 * as missing from the union.
 *)
let rec union_results err rl =
  match rl with
  | [] -> Missing err
  | [r] -> r
  | r1 :: rl ->
    let r2 = union_results err rl in

    (* Union is defined iff both are defined concretely *)
    (match (r1, r2) with
    | (Missing err, _)
    | (_, Missing err) ->
      Missing err
    (* In essence, this says (C | D)::TP = (C::TP) | (D::TP) *)
    | (Exact ty1, Exact ty2) -> Exact (union ty1 ty2)
    (* We don't support projecting through any other kind of union *)
    | _ -> Missing err)

(* Given the results of projecting a type constant from types t1, ..., tn,
 * determine the result of projecting a type constant from type (t1 & ... & tn).
 *)
let rec intersect_results err rl =
  match rl with
  (* Essentially, this is `mixed`. *)
  | [] -> Missing err
  | [r] -> r
  | r1 :: rs ->
    let r2 = intersect_results err rs in
    (match (r1, r2) with
    | (Missing _, r)
    | (r, Missing _) ->
      r
    (* In essence, we're saying (C & D)::TP = (C::TP) & (D::TP) *)
    | (Exact ty1, Exact ty2) -> Exact (intersect ty1 ty2)
    (* Here, we merge the bounds on abstract type constants. Effectively this is intersection. *)
    | ( Abstract { name; names; lower_bounds = lowerl1; upper_bounds = upperl1 },
        Abstract { lower_bounds = lowerl2; upper_bounds = upperl2; _ } ) ->
      Abstract
        {
          name;
          names;
          lower_bounds = TySet.union lowerl1 lowerl2;
          upper_bounds = TySet.union upperl1 upperl2;
        }
    (* Exact type overrides abstract type: the bound on abstract type will be checked
    * against the exact type at implementation site. *)
    | (Abstract _, Exact ty)
    | (Exact ty, Abstract _) ->
      Exact ty)

let rec type_of_result ~ignore_errors ctx env root res =
  let type_with_bound env as_tyvar name ~lower_bounds ~upper_bounds =
    match as_tyvar with
    | Some tyvar_pos ->
      let (env, tvar) = Env.fresh_invariant_type_var env tyvar_pos in
      Log.log_new_tvar_for_tconst_access env tyvar_pos tvar name ctx.id;
      (env, tvar)
    | None ->
      let generic_name = tp_name name ctx.id in
      let reason = make_reason env ctx.id root Reason.Rnone in
      let ty = MakeType.generic reason generic_name in
      let env =
        TySet.fold
          (fun bnd env ->
            (* TODO(T59317869): play well with flow sensitivity *)
            Env.add_upper_bound_global env generic_name bnd)
          upper_bounds
          env
      in
      let env =
        TySet.fold
          (fun bnd env ->
            (* TODO(T59317869): play well with flow sensitivity *)
            Env.add_lower_bound_global env generic_name bnd)
          lower_bounds
          env
      in
      (env, ty)
  in
  match res with
  | Exact ty -> (env, ty)
  | Abstract { name; names = name' :: namel; lower_bounds; upper_bounds } ->
    let res' =
      Abstract { name = name'; names = namel; lower_bounds; upper_bounds }
    in
    let (env, ty) = type_of_result ~ignore_errors ctx env root res' in
    type_with_bound
      env
      None
      name
      ~lower_bounds
      ~upper_bounds:(TySet.singleton ty)
  | Abstract { name; names = []; lower_bounds; upper_bounds } ->
    type_with_bound
      env
      ctx.abstract_as_tyvar_at_pos
      name
      ~lower_bounds
      ~upper_bounds
  | Missing err ->
    if not ignore_errors then err ();
    let reason = make_reason env ctx.id root Reason.Rnone in
    (env, Typing_utils.terr env reason)

let update_class_name env id new_name = function
  | (Exact _ | Missing _) as res -> res
  | Abstract ({ name; names; _ } as abstr) ->
    abstract_or_exact
      env
      id
      { abstr with name = new_name; names = name :: names }

let rec expand ctx env root : _ * result =
  let (env, root) = Env.expand_type env root in
  let err () =
    let (pos, tconst) = ctx.id in
    let ty = Typing_print.error env root in
    Errors.non_object_member_read_
      ~kind:`class_typeconst
      tconst
      pos
      ty
      (get_pos root)
      ctx.ety_env.on_error
  in

  match get_node root with
  | Tany _
  | Terr ->
    (env, Exact root)
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
    begin
      match Env.get_class env (snd cls) with
      | None -> (env, Missing (fun () -> ()))
      | Some ci ->
        (* Hack: `self` in a trait is mistakenly replaced by the trait instead
           of the class using the trait, so if a trait is the root, it is
           likely because originally there was `self::T` written.
           TODO(T54081153): fix `self` in traits and clean this up *)
        let allow_abstract =
          Ast_defs.(equal_class_kind (Decl_provider.Class.kind ci) Ctrait)
          || ctx.allow_abstract
        in

        let ctx = { ctx with allow_abstract } in
        create_root_from_type_constant ctx env root cls ci
    end
  | Tvarray _
  | Tdarray _
  | Tvec_or_dict _
  | Tvarray_or_darray _ ->
    let { id = (_, tconst); _ } = ctx in
    (match tconst with
    (* harcode the constants for coeffects until HAM finishes *)
    | "C"
    | "CMut" ->
      (* TODO point to something real with this reason *)
      (env, Exact (Typing_make_type.mixed Reason.Rnone))
    | _ -> (env, Missing err))
  | Tgeneric (s, tyargs) ->
    let ctx =
      let generics_seen = TySet.add root ctx.generics_seen in
      let base = Some (Option.value ctx.base ~default:root) in
      let allow_abstract = true in
      let abstract_as_tyvar_at_pos = None in
      { ctx with generics_seen; base; allow_abstract; abstract_as_tyvar_at_pos }
    in

    (* Ignore seen bounds to avoid infinite loops *)
    let upper_bounds =
      TySet.elements
        (TySet.diff (Env.get_upper_bounds env s tyargs) ctx.generics_seen)
    in
    let (env, resl) = List.map_env env upper_bounds (expand ctx) in
    let res = intersect_results err resl in
    (env, update_class_name env ctx.id s res)
  | Tdependent (dep_ty, ty) ->
    let ctx =
      let base = Some (Option.value ctx.base ~default:root) in
      let allow_abstract = true in
      let abstract_as_tyvar_at_pos = None in
      { ctx with base; allow_abstract; abstract_as_tyvar_at_pos }
    in
    let (env, res) = expand ctx env ty in
    (env, update_class_name env ctx.id (DependentKind.to_string dep_ty) res)
  | Tintersection tyl ->
    (* Terrible hack (compatible with previous behaviour) that first attempts to project off the
     * non-type-variable conjunects. If this doesn't succeed, then try the type variable
     * conjunects, which will cause type-const constraints to be added to the type variables.
     *)
    let (tyl_vars, tyl_nonvars) =
      List.partition_tf tyl ~f:(fun t ->
          let (_env, t) = Env.expand_type env t in
          is_tyvar t)
    in
    let (env, resl) = List.map_env env tyl_nonvars (expand ctx) in
    let result = intersect_results err resl in
    begin
      match result with
      | Missing _ ->
        let (env, resl) = List.map_env env tyl_vars (expand ctx) in
        (env, intersect_results err resl)
      | _ -> (env, result)
    end
  | Tunion tyl ->
    let (env, resl) = List.map_env env tyl (expand ctx) in
    let result = union_results err resl in
    (env, result)
  | Tvar n ->
    let (env, ty) =
      Typing_subtype_tconst.get_tyvar_type_const
        env
        n
        ctx.id
        ~on_error:ctx.ety_env.on_error
    in
    (env, Exact ty)
  | Tunapplied_alias _ ->
    Typing_defs.error_Tunapplied_alias_in_illegal_context ()
  | Taccess _
  | Tobject
  | Tnonnull
  | Tprim _
  | Tshape _
  | Ttuple _
  | Tfun _
  | Tdynamic
  | Toption _ ->
    (env, Missing err)

(** Expands a type constant access like A::T to its definition. *)
let expand_with_env
    (ety_env : expand_env)
    env
    ?(ignore_errors = false)
    ?(as_tyvar_with_cnstr = None)
    root
    (id : pos_id)
    ~root_pos
    ~allow_abstract_tconst =
  let (env, ty) =
    Log.log_type_access ~level:1 root id
    @@
    let ctx =
      {
        id;
        ety_env;
        base = None;
        generics_seen = TySet.empty;
        allow_abstract = allow_abstract_tconst;
        abstract_as_tyvar_at_pos = as_tyvar_with_cnstr;
        root_pos;
      }
    in
    let (env, res) = expand ctx env root in
    type_of_result ~ignore_errors ctx env root res
  in
  (env, ty)

(* This is called with non-nested type accesses e.g. this::T1::T2 is
 * represented by (this, [T1; T2])
 *)
let referenced_typeconsts env ety_env (root, ids) =
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
            ~as_tyvar_with_cnstr:None
            root
            (Pos_or_decl.of_raw_pos pos, tconst)
            ~root_pos:(get_pos root)
            ~allow_abstract_tconst:true,
          acc )
      end
  |> snd

(*****************************************************************************)
(* Exporting *)
(*****************************************************************************)

let () = Typing_utils.expand_typeconst_ref := expand_with_env
