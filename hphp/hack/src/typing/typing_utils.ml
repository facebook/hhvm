(*
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
open Typing_env_types
module SN = Naming_special_names
module Reason = Typing_reason
module Env = Typing_env
module ShapeMap = Aast.ShapeMap
module TySet = Typing_set
module Cls = Decl_provider.Class
module MakeType = Typing_make_type

(* This can be useful to debug type which blow up in size *)
let ty_size env ty =
  let ty_size_visitor =
    object
      inherit [int] Type_visitor.locl_type_visitor as super

      method! on_type acc ty = 1 + super#on_type acc ty

      method! on_tvar acc r v =
        let (_, ty) = Env.expand_var env r v in
        match ty with
        | (_, Tvar v') when v' = v -> acc
        | _ -> super#on_type acc ty
    end
  in
  ty_size_visitor#on_type 0 ty

(*****************************************************************************)
(* Importing what is necessary *)
(*****************************************************************************)
let not_implemented s _ =
  failwith (Printf.sprintf "Function %s not implemented" s)

type expand_typedef =
  expand_env -> env -> Reason.t -> string -> locl_ty list -> env * locl_ty

let (expand_typedef_ref : expand_typedef ref) =
  ref (not_implemented "expand_typedef")

let expand_typedef x = !expand_typedef_ref x

type sub_type =
  env -> locl_ty -> locl_ty -> Errors.typing_error_callback -> env

let (sub_type_ref : sub_type ref) = ref (not_implemented "sub_type")

let sub_type x = !sub_type_ref x

type is_sub_type_type = env -> locl_ty -> locl_ty -> bool

(*let (is_sub_type_ref: is_sub_type_type ref) = ref not_implemented*)
let (is_sub_type_for_union_ref : is_sub_type_type ref) =
  ref (not_implemented "is_sub_type_for_union")

let is_sub_type_for_union x = !is_sub_type_for_union_ref x

type add_constraint =
  Pos.Map.key -> env -> Ast_defs.constraint_kind -> locl_ty -> locl_ty -> env

let (add_constraint_ref : add_constraint ref) =
  ref (not_implemented "add_constraint")

let add_constraint x = !add_constraint_ref x

type expand_typeconst =
  expand_env ->
  env ->
  ?as_tyvar_with_cnstr:bool ->
  locl_ty ->
  Aast.sid ->
  env * locl_ty

let (expand_typeconst_ref : expand_typeconst ref) =
  ref (not_implemented "expand_typeconst")

let expand_typeconst x = !expand_typeconst_ref x

type union = env -> locl_ty -> locl_ty -> env * locl_ty

let (union_ref : union ref) = ref (not_implemented "union")

let union x = !union_ref x

type union_list = env -> Reason.t -> locl_ty list -> env * locl_ty

let (union_list_ref : union_list ref) = ref (not_implemented "union_list")

let union_list x = !union_list_ref x

type fold_union = env -> Reason.t -> locl_ty list -> env * locl_ty

let (fold_union_ref : fold_union ref) = ref (not_implemented "fold_union")

let fold_union x = !fold_union_ref x

type simplify_unions =
  env ->
  ?on_tyvar:(env -> Reason.t -> Ident.t -> env * locl_ty) ->
  locl_ty ->
  env * locl_ty

let (simplify_unions_ref : simplify_unions ref) =
  ref (not_implemented "simplify_unions")

let simplify_unions x = !simplify_unions_ref x

type approx =
  | ApproxUp
  | ApproxDown

type non = env -> Reason.t -> locl_ty -> approx:approx -> env * locl_ty

let (non_ref : non ref) = ref (not_implemented "non")

let non x = !non_ref x

type simplify_intersections =
  env ->
  ?on_tyvar:(env -> Reason.t -> int -> env * locl_ty) ->
  locl_ty ->
  env * locl_ty

let (simplify_intersections_ref : simplify_intersections ref) =
  ref (not_implemented "simplify_intersections")

let simplify_intersections x = !simplify_intersections_ref x

type localize_with_self = env -> decl_ty -> env * locl_ty

let (localize_with_self_ref : localize_with_self ref) =
  ref (not_implemented "localize_with_self")

let localize_with_self x = !localize_with_self_ref x

(* Convenience function for creating `this` types *)
let this_of ty = Tabstract (AKdependent DTthis, Some ty)

(*****************************************************************************)
(* Returns true if a type is optional *)
(*****************************************************************************)

let is_option env ty =
  let null = MakeType.null Reason.Rnone in
  is_sub_type_for_union env null ty

let is_mixed env ty =
  let mixed = MakeType.mixed Reason.Rnone in
  is_sub_type_for_union env mixed ty

let is_nothing env ty =
  let nothing = MakeType.nothing Reason.Rnone in
  is_sub_type_for_union env ty nothing

let ensure_option env r ty =
  if is_option env ty then
    ty
  else
    (r, Toption ty)

(* Grab all supertypes of a given type, recursively *)
let get_all_supertypes env ty =
  let rec iter seen env acc tyl =
    match tyl with
    | [] -> (env, acc)
    | ty :: tyl ->
      let (env, ty) = Env.expand_type env ty in
      (match snd ty with
      | Tabstract (_, Some ty) -> iter seen env (TySet.add ty acc) (ty :: tyl)
      | Tabstract (AKgeneric n, _) ->
        if SSet.mem n seen then
          iter seen env acc tyl
        else
          iter
            (SSet.add n seen)
            env
            acc
            (TySet.elements (Env.get_upper_bounds env n) @ tyl)
      | _ -> iter seen env (TySet.add ty acc) tyl)
  in
  let (env, resl) = iter SSet.empty env TySet.empty [ty] in
  (env, TySet.elements resl)

(*****************************************************************************
 * Get the "as" constraints from an abstract type or generic parameter, or
 * return the type itself if there is no "as" constraint.
 * In the case of a generic parameter whose "as" constraint is another
 * generic parameter, repeat the process until a type is reached that is not
 * a generic parameter. Don't loop on cycles.
 * (For example, function foo<Tu as Tv, Tv as Tu>(...))
 *****************************************************************************)
let get_concrete_supertypes env ty =
  let rec iter seen env acc tyl =
    match tyl with
    | [] -> (env, acc)
    | ty :: tyl ->
      let (env, ty) = Env.expand_type env ty in
      (match snd ty with
      (* Enums with arraykey upper bound are treated as "abstract" *)
      | Tabstract (AKnewtype (cid, _), Some (_, Tprim Aast.Tarraykey))
        when Env.is_enum env cid ->
        iter seen env acc tyl
      (* Don't expand enums or newtype; just return the type itself *)
      | Tabstract ((AKnewtype _ | AKdependent _), Some ty) ->
        iter seen env (TySet.add ty acc) tyl
      | Tabstract (_, Some ty) -> iter seen env acc (ty :: tyl)
      | Tabstract (AKgeneric n, _) ->
        if SSet.mem n seen then
          iter seen env acc tyl
        else
          iter
            (SSet.add n seen)
            env
            acc
            (TySet.elements (Env.get_upper_bounds env n) @ tyl)
      | Tabstract (_, None) -> iter seen env acc tyl
      | Tunion tyl' ->
        let tys = TySet.of_list tyl' in
        begin
          match TySet.elements tys with
          | [ty] -> iter seen env acc (ty :: tyl)
          | _ -> iter seen env (TySet.add ty acc) tyl
        end
      | _ -> iter seen env (TySet.add ty acc) tyl)
  in
  let (env, resl) = iter SSet.empty env TySet.empty [ty] in
  (env, TySet.elements resl)

(* Try running function on each concrete supertype in turn. Return all
 * successful results
 *)
let try_over_concrete_supertypes env ty f =
  let (env, tyl) = get_concrete_supertypes env ty in
  (* If there is just a single result then don't swallow errors *)
  match tyl with
  | [ty] -> [f env ty]
  | _ ->
    let rec iter_over_types env resl tyl =
      match tyl with
      | [] -> resl
      | ty :: tyl ->
        Errors.try_
          (fun () -> iter_over_types env (f env ty :: resl) tyl)
          (fun _ -> iter_over_types env resl tyl)
    in
    iter_over_types env [] tyl

(** Run a function on an intersection represented by a list of types.
Similarly to try_over_concrete_supertypes, we stay liberal with errors:
discard the result of any run which has produced an error.
If all runs have produced an error, gather all errors and results and add errors. *)
let run_on_intersection :
    'env -> f:('env -> locl_ty -> 'env * 'a) -> locl_ty list -> 'env * 'a list
    =
 fun env ~f tyl ->
  let (env, resl_errors) =
    List.map_env env tyl ~f:(fun env ty ->
        Errors.try_with_result
          (fun () ->
            let (env, res) = f env ty in
            (env, (res, None)))
          (fun (_, (res, _)) err -> (env, (res, Some err))))
  in
  let valid_resl =
    List.filter resl_errors ~f:(fun (_, err) -> Option.is_none err)
    |> List.map ~f:fst
  in
  let resl =
    if not (List.is_empty valid_resl) then
      valid_resl
    else (
      List.iter resl_errors ~f:(fun (_, err) ->
          Option.iter err ~f:Errors.add_error);
      List.map ~f:fst resl_errors
    )
  in
  (env, resl)

(*****************************************************************************)
(* Dynamicism  *)
(*****************************************************************************)
let is_dynamic env ty =
  let dynamic = MakeType.dynamic Reason.Rnone in
  (is_sub_type_for_union env dynamic ty && not (is_mixed env ty))
  || (is_sub_type_for_union env ty dynamic && not (is_nothing env ty))

(*****************************************************************************)
(* Check if type is any or a variant thereof  *)
(*****************************************************************************)

let rec is_any env ty =
  match Env.expand_type env ty with
  | (_, (_, (Tany _ | Terr))) -> true
  | (_, (_, Tunion tyl)) -> List.for_all tyl (is_any env)
  | (_, (_, Tintersection tyl)) -> List.exists tyl (is_any env)
  | _ -> false

(*****************************************************************************)
(* Gets the base type of an abstract type *)
(*****************************************************************************)

let rec get_base_type env ty =
  match snd ty with
  | Tabstract (AKnewtype (classname, _), _)
    when classname = SN.Classes.cClassname ->
    ty
  (* If we have an expression dependent type and it only has one super
    type, we can treat it similarly to AKdependent _, Some ty  *)
  | Tabstract (AKgeneric n, _) when AbstractKind.is_generic_dep_ty n ->
    begin
      match TySet.elements (Env.get_upper_bounds env n) with
      | ty2 :: _ when ty_equal ty ty2 -> ty
      (* If it's exactly equal, then the base ty is just this one *)
      | ty :: _ ->
        if TySet.mem ty (Env.get_lower_bounds env n) then
          ty
        else
          get_base_type env ty
      | [] -> ty
    end
  | Tabstract (AKnewtype (cid, _), Some (_, Tprim Aast.Tarraykey))
    when Env.is_enum env cid ->
    ty
  | Tabstract _ ->
    begin
      match get_concrete_supertypes env ty with
      (* If the type is exactly equal, we don't want to recurse *)
      | (_, ty2 :: _) when ty_equal ty ty2 -> ty
      | (_, ty :: _) -> get_base_type env ty
      | (_, []) -> ty
    end
  | _ -> ty

(*****************************************************************************)
(* Given some class type or unresolved union of class types, return the
 * identifiers of all classes the type may represent.
 *
 * Intended for uses like constructing call graphs and finding references, where
 * we have the statically known class type of some runtime value or class ID and
 * we would like the name of that class. *)
(*****************************************************************************)
let get_class_ids env ty =
  let rec aux seen acc = function
    | (_, Tclass ((_, cid), _, _)) -> cid :: acc
    | (_, (Toption ty | Tabstract (_, Some ty))) -> aux seen acc ty
    | (_, Tunion tys)
    | (_, Tintersection tys) ->
      List.fold tys ~init:acc ~f:(aux seen)
    | (_, Tabstract (AKgeneric name, None))
      when not (List.mem ~equal:( = ) seen name) ->
      let seen = name :: seen in
      let upper_bounds = Env.get_upper_bounds env name in
      TySet.fold (fun ty acc -> aux seen acc ty) upper_bounds acc
    | _ -> acc
  in
  List.rev (aux [] [] (Typing_expand.fully_expand env ty))

(*****************************************************************************)
(* Reactivity *)
(*****************************************************************************)

let reactivity_to_string env r =
  let cond_reactive prefix t =
    let str = Typing_print.full_decl (Env.get_tcopt env) t in
    prefix ^ " (condition type: " ^ str ^ ")"
  in
  let rec aux r =
    match r with
    | Reactive None -> "reactive"
    | Reactive (Some ty) -> cond_reactive "conditionally reactive" ty
    | Shallow None -> "shallow reactive"
    | Shallow (Some ty) -> cond_reactive "conditionally shallow reactive" ty
    | Local None -> "local reactive"
    | Local (Some ty) -> cond_reactive "conditionally local reactive" ty
    | MaybeReactive n -> "maybe (" ^ aux n ^ ")"
    | Nonreactive -> "non-reactive"
    | RxVar _ -> "maybe reactive"
  in
  aux r

let get_printable_shape_field_name = Env.get_shape_field_name

let shape_field_name_ env field =
  Aast.(
    match field with
    | (p, Int name) -> Ok (Ast_defs.SFlit_int (p, name))
    | (p, String name) -> Ok (Ast_defs.SFlit_str (p, name))
    | (_, Class_const ((_, CI x), y)) -> Ok (Ast_defs.SFclass_const (x, y))
    | (_, Class_const ((_, CIself), y)) ->
      let (_, c_ty) = Env.get_self env in
      (match c_ty with
      | Tclass (sid, _, _) -> Ok (Ast_defs.SFclass_const (sid, y))
      | _ -> Error `Expected_class)
    | _ -> Error `Invalid_shape_field_name)

let shape_field_name env (p, field) =
  match shape_field_name_ env (p, field) with
  | Ok x -> Some x
  | Error `Expected_class ->
    Errors.expected_class p;
    None
  | Error `Invalid_shape_field_name ->
    Errors.invalid_shape_field_name p;
    None

(*****************************************************************************)
(* Try to unify all the types in a intersection *)
(*****************************************************************************)
let flatten_unresolved env ty acc =
  let (env, ety) = Env.expand_type env ty in
  let res =
    match ety with
    (* flatten Tunion[Tunion[...]] *)
    | (_, Tunion tyl) -> tyl @ acc
    | (_, Tany _) -> acc
    | _ -> ty :: acc
  in
  (env, res)

(*****************************************************************************)
(*****************************************************************************)

(* Try to unify all the types in a union *)
let rec fold_unresolved env ty =
  let (env, ety) = Env.expand_type env ty in
  match ety with
  | (r, Tunion []) -> (env, (r, Typing_defs.make_tany ()))
  | (_, Tunion [x]) -> fold_unresolved env x
  (* We don't want to use unification if new_inference is set.
   * Just return the type unchanged: better would be to remove redundant
   * elements, but let's postpone that until we have an improved
   * representation of unions.
   *)
  | _ -> (env, ety)

(*****************************************************************************)
(* *)
(*****************************************************************************)

let string_of_visibility = function
  | Vpublic -> "public"
  | Vprivate _ -> "private"
  | Vprotected _ -> "protected"

let unwrap_class_type = function
  | (r, Tapply (name, tparaml)) -> (r, name, tparaml)
  | ( _,
      ( Terr | Tdynamic | Tany _ | Tmixed | Tnonnull | Tnothing
      | Tarray (_, _)
      | Tdarray (_, _)
      | Tvarray _ | Tvarray_or_darray _ | Tgeneric _ | Toption _ | Tlike _
      | Tprim _ | Tfun _ | Ttuple _ | Tshape _
      | Taccess (_, _)
      | Tthis | Tpu_access _ | Tvar _ ) ) ->
    raise @@ Invalid_argument "unwrap_class_type got non-class"

let try_unwrap_class_type x = Option.try_with (fun () -> unwrap_class_type x)

let class_is_final_and_not_contravariant class_ty =
  Cls.final class_ty
  && List.for_all (Cls.tparams class_ty) ~f:(function
         | { tp_variance = Ast_defs.Invariant | Ast_defs.Covariant; _ } -> true
         | _ -> false)

(*****************************************************************************)
(* Check if a type is not fully constrained *)
(*****************************************************************************)

module HasTany : sig
  val check : locl_ty -> bool

  val check_why : locl_ty -> Reason.t option
end = struct
  let merge x y = Option.merge x y (fun x _ -> x)

  let visitor =
    object (this)
      inherit [Reason.t option] Type_visitor.locl_type_visitor

      method! on_tany _ r = Some r

      method! on_tarraykind acc r akind =
        match akind with
        | AKany -> Some r
        | AKempty -> acc
        | AKvarray_or_darray ty
        | AKvarray ty
        | AKvec ty ->
          this#on_type acc ty
        | AKdarray (tk, tv)
        | AKmap (tk, tv) ->
          merge (this#on_type acc tk) (this#on_type acc tv)
    end

  let check_why ty = visitor#on_type None ty

  let check ty = Option.is_some (check_why ty)
end

(*****************************************************************************)
(* Function parameters *)
(*****************************************************************************)

let default_fun_param ?(pos = Pos.none) ty : 'a fun_param =
  {
    fp_pos = pos;
    fp_name = None;
    fp_type = { et_type = ty; et_enforced = false };
    fp_kind = FPnormal;
    fp_accept_disposable = false;
    fp_mutability = None;
    fp_rx_annotation = None;
  }

let fun_mutable user_attributes =
  let rec go = function
    | [] -> None
    | { Aast.ua_name = (_, n); _ } :: _ when n = SN.UserAttributes.uaMutable ->
      Some Param_borrowed_mutable
    | { Aast.ua_name = (_, n); _ } :: _
      when n = SN.UserAttributes.uaMaybeMutable ->
      Some Param_maybe_mutable
    | _ :: tl -> go tl
  in
  go user_attributes

let tany = Env.tany

let decl_tany = Env.decl_tany

let terr env =
  let dynamic_view_enabled =
    TypecheckerOptions.dynamic_view (Typing_env.get_tcopt env)
  in
  if dynamic_view_enabled then
    Tdynamic
  else
    Terr

(* Hacked version of Typing_subtype.try_intersect for collecting function types *)
let add_function_type env fty logged =
  let (untyped_ftys, ftys) = logged in
  let rec try_intersect env ty tyl =
    match tyl with
    | [] -> [ty]
    | ty' :: tyl' ->
      if is_sub_type_for_union env ty ty' && not (HasTany.check ty) then
        try_intersect env ty tyl'
      else if is_sub_type_for_union env ty' ty && not (HasTany.check ty') then
        try_intersect env ty' tyl'
      else
        ty' :: try_intersect env ty tyl'
  in
  if HasTany.check fty then
    (try_intersect env fty untyped_ftys, ftys)
  else
    (untyped_ftys, try_intersect env fty ftys)

type class_get_pu =
  ?from_class:Nast.class_id_ ->
  env ->
  locl_ty ->
  string ->
  env * (expand_env * pu_enum_type) option

let (class_get_pu_ref : class_get_pu ref) =
  ref (fun ?from_class:_ -> not_implemented "Typing_utils.class_get_pu")

let class_get_pu ?from_class env ty name =
  !class_get_pu_ref ?from_class env ty name
