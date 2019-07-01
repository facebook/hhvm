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

module N = Nast
module SN = Naming_special_names
module Reason = Typing_reason
module Env = Typing_env
module ShapeMap = Nast.ShapeMap
module TySet = Typing_set
module Cls = Decl_provider.Class
module MakeType = Typing_make_type

(* This can be useful to debug type which blow up in size *)
let ty_size env ty =
  let ty_size_visitor = object
    inherit [int] Type_visitor.type_visitor as super
    method! on_type acc ty = 1 + super#on_type acc ty
    method! on_tvar acc r v =
      let _, ty = Env.expand_var env r v in
      match ty with
      | _, Tvar v' when v' = v -> acc
      | _ -> super#on_type acc ty
    end in
  ty_size_visitor#on_type 0 ty

(*****************************************************************************)
(* Importing what is necessary *)
(*****************************************************************************)
let not_implemented _ = failwith "Function not implemented"
type expand_typedef =
    expand_env -> Env.env -> Reason.t -> string -> locl ty list -> Env.env * locl ty
let (expand_typedef_ref : expand_typedef ref) = ref not_implemented
let expand_typedef x = !expand_typedef_ref x

type sub_type = Env.env -> locl ty -> locl ty -> Env.env
let (sub_type_ref: sub_type ref) = ref not_implemented
let sub_type x = !sub_type_ref x

type sub_string = Pos.t -> Env.env -> locl ty -> Env.env
let (sub_string_ref: sub_string ref) = ref not_implemented
let sub_string x = !sub_string_ref x

type is_sub_type_type =
  Env.env -> locl ty -> locl ty -> bool
let (is_sub_type_ref: is_sub_type_type ref) = ref not_implemented
let is_sub_type x = !is_sub_type_ref x
let (is_sub_type_for_union_ref: is_sub_type_type ref) = ref not_implemented
let is_sub_type_for_union x = !is_sub_type_for_union_ref x

type add_constraint = Pos.Map.key -> Env.env -> Ast.constraint_kind -> locl ty -> locl ty -> Env.env
let (add_constraint_ref: add_constraint ref) = ref not_implemented
let add_constraint x = !add_constraint_ref x

type expand_type_and_solve_type = Env.env -> description_of_expected:string -> Pos.t -> locl ty -> Env.env * locl ty
let (expand_type_and_solve_ref: expand_type_and_solve_type ref) = ref not_implemented
let expand_type_and_solve env ~description_of_expected = !expand_type_and_solve_ref env ~description_of_expected

type expand_typeconst =
  expand_env -> Env.env -> ?as_tyvar_with_cnstr:bool -> locl ty ->
  Nast.sid -> Env.env * locl ty
let (expand_typeconst_ref: expand_typeconst ref) = ref not_implemented
let expand_typeconst x = !expand_typeconst_ref x

type union = Env.env -> locl ty -> locl ty -> Env.env * locl ty
let (union_ref: union ref) = ref not_implemented
let union x = !union_ref x

type union_list = Env.env -> Reason.t -> locl ty list -> (Env.env * locl ty)
let (union_list_ref : union_list ref) = ref not_implemented
let union_list x = !union_list_ref x

type fold_union = Env.env -> Reason.t -> locl ty list -> Env.env * locl ty
let (fold_union_ref : fold_union ref) = ref not_implemented
let fold_union x = !fold_union_ref x

type simplify_unions =
  Env.env ->
  ?on_tyvar:(Env.env -> Reason.t -> Ident.t -> Env.env * locl ty) ->
  locl ty ->
  Env.env * locl ty
let (simplify_unions_ref : simplify_unions ref) = ref not_implemented
let simplify_unions x = !simplify_unions_ref x

type diff = locl ty -> locl ty -> locl ty
let (diff_ref : diff ref) = ref not_implemented
let diff x = !diff_ref x

type approx = ApproxUp | ApproxDown
type non = Env.env -> Reason.t -> locl ty -> approx:approx -> (Env.env * locl ty)
let (non_ref : non ref) = ref not_implemented
let non x = !non_ref x

type simplify_intersections = Env.env ->
  ?on_tyvar:(Env.env -> Reason.t -> int -> Env.env * locl ty) -> locl ty -> Env.env * locl ty
let (simplify_intersections_ref : simplify_intersections ref) = ref not_implemented
let simplify_intersections x = !simplify_intersections_ref x

type localize_with_self = Env.env -> decl ty -> Env.env * locl ty
let (localize_with_self_ref : localize_with_self ref) = ref not_implemented
let localize_with_self x = !localize_with_self_ref x

type coerce_type =
  Pos.t ->
  ?sub_fn:(Pos.t -> Reason.ureason -> Env.env -> locl ty -> locl ty -> Env.env) ->
  Reason.ureason -> Env.env -> locl ty -> ?ty_expect_decl: decl ty -> locl ty -> Env.env
let (coerce_type_ref : coerce_type ref) = ref not_implemented
let coerce_type x = !coerce_type_ref x

type can_coerce = Env.env -> ?ur:Reason.ureason -> locl ty -> ?ty_expect_decl: decl ty -> locl ty ->
  Env.env option
let (can_coerce_ref : can_coerce ref) = ref not_implemented
let can_coerce x = !can_coerce_ref x

(* Convenience function for creating `this` types *)
let this_of ty = Tabstract (AKdependent `this, Some ty)

(*****************************************************************************)
(* Returns true if a type is optional *)
(*****************************************************************************)

let is_option env ty =
  let null =  MakeType.null Reason.Rnone in
  is_sub_type_for_union env null ty

let is_mixed env ty =
  let mixed = MakeType.mixed Reason.Rnone in
  is_sub_type_for_union env mixed ty

let is_stringish env ty =
  let stringish = MakeType.class_type Reason.Rnone SN.Classes.cStringish [] in
  is_sub_type env ty stringish

let is_object env ty =
  is_sub_type env ty (Reason.Rnone, Tobject)

let ensure_option env r ty = if is_option env ty then ty else (r, Toption ty)

(* Grab all supertypes of a given type, recursively *)
let get_all_supertypes env ty =
  let rec iter seen env acc tyl =
    match tyl with
    | [] -> env, acc
    | ty::tyl ->
      let env, ty = Env.expand_type env ty in
      match snd ty with
      | Tabstract (_, Some ty) ->
        iter seen env (TySet.add ty acc) (ty::tyl)

      | Tabstract (AKgeneric n, _) ->
        if SSet.mem n seen
        then iter seen env acc tyl
        else iter (SSet.add n seen) env acc
          (TySet.elements (Env.get_upper_bounds env n) @ tyl)
      | _ ->
        iter seen env (TySet.add ty acc) tyl
  in
   let env, resl = iter SSet.empty env TySet.empty [ty] in
   env, TySet.elements resl

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
    | [] -> env, acc
    | ty::tyl ->
      let env, ty = Env.expand_type env ty in
      match snd ty with
      (* Don't expand enums or newtype; just return the type itself *)
      | Tabstract ((AKnewtype _ | AKenum _ | AKdependent _), Some ty) ->
        iter seen env (TySet.add ty acc) tyl

      | Tabstract (_, Some ty) ->
        iter seen env acc (ty::tyl)

      | Tabstract (AKgeneric n, _) ->
        if SSet.mem n seen
        then iter seen env acc tyl
        else iter (SSet.add n seen) env acc
          (TySet.elements (Env.get_upper_bounds env n) @ tyl)

      | Tabstract (_, None) ->
        iter seen env acc tyl
      | Tunion tyl' ->
        let tys = TySet.of_list tyl' in
        begin match TySet.elements tys with
        | [ty] -> iter seen env acc (ty::tyl)
        | _ -> iter seen env (TySet.add ty acc) tyl
        end
      | _ ->
        iter seen env (TySet.add ty acc) tyl
  in
    let env, resl = iter SSet.empty env TySet.empty [ty] in
    env, TySet.elements resl

(* Try running function on each concrete supertype in turn. Return all
 * successful results
 *)
let try_over_concrete_supertypes env ty f =
  let env, tyl = get_concrete_supertypes env ty in
  (* If there is just a single result then don't swallow errors *)
  match tyl with
  | [ty] ->
    [f env ty]
  | _ ->
    let rec iter_over_types env resl tyl =
      match tyl with
        [] -> resl
      | ty::tyl ->
        Errors.try_
          (fun () -> iter_over_types env (f env ty::resl) tyl)
          (fun _ -> iter_over_types env resl tyl) in
  iter_over_types env [] tyl

(** Run a function on an intersection represented by a list of types.
Similarly to try_over_concrete_supertypes, we stay liberal with errors:
discard the result of any run which has produced an error.
If all runs have produced an error, gather all errors and results and add errors. *)
let run_on_intersection :
  'env -> f:('env -> locl ty -> 'env * 'a) -> locl ty list -> 'env * 'a list
= fun env ~f tyl ->
  let env, resl_errors = List.map_env env tyl ~f:(fun env ty ->
    Errors.try_with_result
      (fun () ->
        let env, res = f env ty in
        env, (res, None))
      (fun (_, (res, _)) err -> env, (res, Some err))) in
  let valid_resl = List.filter resl_errors ~f:(fun (_, err) -> Option.is_none err) |>
    List.map ~f:fst in
  let resl = if not (List.is_empty valid_resl) then valid_resl else begin
    List.iter resl_errors ~f:(fun (_, err) -> Option.iter err ~f:Errors.add_error);
    List.map ~f:fst resl_errors end in
  env, resl

(*****************************************************************************)
(* Dynamicism  *)
(*****************************************************************************)
let is_dynamic env ty =
  let dynamic = MakeType.dynamic Reason.Rnone in
  is_sub_type_for_union env dynamic ty && not (is_mixed env ty)

let is_hack_collection env ty =
  is_sub_type env ty
    (MakeType.const_collection Reason.Rnone (MakeType.mixed Reason.Rnone))

(*****************************************************************************)
(* Check if type is any or a variant thereof  *)
(*****************************************************************************)

let rec is_any env ty =
  match Env.expand_type env ty with
  | (_, (_, (Tany | Terr))) -> true
  | (_, (_, Tunion tyl)) -> List.for_all tyl (is_any env)
  | (_, (_, Tintersection tyl)) -> List.exists tyl (is_any env)
  | _ -> false

(*****************************************************************************)
(* Gets the base type of an abstract type *)
(*****************************************************************************)

let rec get_base_type env ty =
  match snd ty with
  | Tabstract (AKnewtype (classname, _), _) when
      classname = SN.Classes.cClassname -> ty
  (* If we have an expression dependent type and it only has one super
    type, we can treat it similarly to AKdependent _, Some ty  *)
  | Tabstract (AKgeneric n, _) when AbstractKind.is_generic_dep_ty n ->
    begin match TySet.elements (Env.get_upper_bounds env n) with
    | ty2::_ when ty_equal ty ty2 -> ty
    (* If it's exactly equal, then the base ty is just this one *)
    | ty::_ ->
      if TySet.mem ty (Env.get_lower_bounds env n)
      then ty else get_base_type env ty
    | [] -> ty
    end
  | Tabstract _ ->
    begin match get_concrete_supertypes env ty with
    (* If the type is exactly equal, we don't want to recurse *)
    | _, ty2::_ when ty_equal ty ty2 -> ty
    | _, ty::_ -> get_base_type env ty
    | _, [] -> ty
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
    | _, Tclass ((_, cid), _, _) -> cid::acc
    | _, (Toption ty | Tabstract (_, Some ty)) -> aux seen acc ty
    | _, Tunion tys | _, Tintersection tys -> List.fold tys ~init:acc ~f:(aux seen)
    | _, Tabstract (AKgeneric name, None) when not (List.mem ~equal:(=) seen name) ->
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
    let str = Typing_print.full env t in
    prefix ^ " (condition type: " ^ str ^ ")" in
  let rec aux r =
    match r with
    | Reactive None -> "reactive"
    | Reactive (Some ty) -> cond_reactive "conditionally reactive" ty
    | Shallow None -> "shallow reactive"
    | Shallow (Some ty) -> cond_reactive "conditionally shallow reactive" ty
    | Local None -> "local reactive"
    | Local (Some ty) -> cond_reactive "conditionally local reactive" ty
    | MaybeReactive n -> "maybe (" ^ (aux n) ^ ")"
    | Nonreactive -> "non-reactive"
    | RxVar _ -> "maybe reactive" in
  aux r

(*****************************************************************************)
(* Unification error *)
(*****************************************************************************)
let uerror env r1 ty1 r2 ty2 =
  let ty1 = Typing_print.with_blank_tyvars (fun () -> Typing_print.full_strip_ns env (r1,ty1)) in
  let ty2 = Typing_print.with_blank_tyvars (fun () -> Typing_print.full_strip_ns env (r2,ty2)) in
  let ty1, ty2 =
    if String.equal ty1 ty2 then
      "exactly the type " ^ ty1, "the nonexact type " ^ ty2
    else ty1, ty2
  in
  let left = Reason.to_string ("Expected " ^ ty1) r1 in
  let right = Reason.to_string ("But got " ^ ty2) r2 in
  match (r1, r2) with
  | Reason.Rcstr_on_generics (p, tparam), _ | _, Reason.Rcstr_on_generics (p, tparam) ->
    Errors.violated_constraint p tparam left right
  | _ -> Errors.unify_error left right

let get_printable_shape_field_name = Env.get_shape_field_name

(* Traverses two shapes structurally, parameterized by functions to run on
  common fields (on_common_field) and when the first shape has an optional
  field that is missing in the second (on_missing_omittable optional_field
  and on_missing_non_omittable_optional_field).

  This is used in subtyping and unification. When subtyping, the first and
  second fields are respectively the supertype and subtype.

  The unset fields of the first shape (empty if it is fully known) must be a
  subset of the unset fields of the second shape. An error will be reported
  otherwise.

  If the first shape has an optional field, we distinguish two cases:

    - the field may be omitted in the second shape because the shape is
    closed or because the field is explicitly unset in it;

    - the field may not be omitted (i.e., the second shape is open and
    the field is not explicitly unset in it).

  The first case corresponds roughly to the rule

      shape() <: shape(?'x' => t),

  saying that a closed shape can be widened with optional fields of any
  types.  The second case corresponds to the rule

      shape(...) <: shape(?'x' => mixed, ...),

  i.e., an open shape can be widened with optional fields of type mixed
  only.

*)
let apply_shape
  ~on_common_field
  ~on_missing_omittable_optional_field
  ~on_missing_non_omittable_optional_field
  ~on_error
  (env, acc)
  (r1, fields_known1, fdm1)
  (r2, fields_known2, fdm2) =
  let (env, acc) =
  begin match fields_known1, fields_known2 with
    | FieldsFullyKnown, FieldsFullyKnown ->
        (* If both shapes are FieldsFullyKnown, then we must ensure that the
           supertype shape knows about every single field that could possibly
           be present in the subtype shape. *)
        ShapeMap.fold begin fun name _ (env, acc) ->
          if not @@ ShapeMap.mem name fdm1 then
            let pos1 = Reason.to_pos r1 in
            let pos2 = Reason.to_pos r2 in
            on_error (env,acc) (fun () -> Errors.unknown_field_disallowed_in_shape
              pos1
              pos2
              (get_printable_shape_field_name name))
          else (env, acc)
        end fdm2 (env, acc)
    | FieldsFullyKnown, FieldsPartiallyKnown _  ->
        let pos1 = Reason.to_pos r1 in
        let pos2 = Reason.to_pos r2 in
        on_error (env, acc) (fun () -> Errors.shape_fields_unknown pos2 pos1)
    | FieldsPartiallyKnown unset_fields1,
      FieldsPartiallyKnown unset_fields2 ->
        ShapeMap.fold begin fun name unset_pos (env, acc) ->
          match ShapeMap.get name unset_fields2 with
            | Some _ -> (env, acc)
            | None ->
                let pos2 = Reason.to_pos r2 in
                on_error (env, acc) (fun () -> Errors.shape_field_unset unset_pos pos2
                  (get_printable_shape_field_name name))
        end unset_fields1 (env, acc)
    | _ -> (env, acc)
  end in
  ShapeMap.fold begin fun name shape_field_type_1 (env, acc) ->
    match ShapeMap.get name fdm2 with
    | None when shape_field_type_1.sft_optional ->
        let can_omit = match fields_known2 with
          | FieldsFullyKnown -> true
          | FieldsPartiallyKnown unset_fields ->
              ShapeMap.mem name unset_fields in
        let on_missing_optional_field =
          if can_omit
          then on_missing_omittable_optional_field
          else on_missing_non_omittable_optional_field in
        on_missing_optional_field (env, acc) name shape_field_type_1
    | None ->
        let pos1 = Reason.to_pos r1 in
        let pos2 = Reason.to_pos r2 in
        on_error (env, acc) (fun () ->
          Errors.missing_field pos2 pos1 (get_printable_shape_field_name name))
    | Some shape_field_type_2 ->
        on_common_field (env, acc) name shape_field_type_1 shape_field_type_2
  end fdm1 (env, acc)

let shape_field_name_ env field =
  let open Nast in match field with
    | p, Int name -> Ok (Ast.SFlit_int (p, name))
    | p, String name -> Ok (Ast.SFlit_str (p, name))
    | _, Class_const ((_, CI (x)), y) -> Ok (Ast.SFclass_const (x, y))
    | _, Class_const ((_, CIself), y) ->
      let _, c_ty = Env.get_self env in
      (match c_ty with
      | Tclass (sid, _, _) ->
        Ok (Ast.SFclass_const(sid, y))
      | _ ->
        Error `Expected_class)
    | _ -> Error `Invalid_shape_field_name

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
  let env, ety = Env.expand_type env ty in
  let res = match ety with
    (* flatten Tunion[Tunion[...]] *)
    | (_, Tunion tyl) -> tyl @ acc
    | (_, Tany) -> acc
    | _ -> ty :: acc in
  env, res

let rec push_option_out pos env ty =
  let is_option = function
    | _, Toption _ -> true
    | _ -> false in
  let env, ty = Env.expand_type env ty in
  match ty with
  | r, Toption ty ->
    let env, ty = push_option_out pos env ty in
    env, if is_option ty then ty else (r, Toption ty)
  | r, Tprim N.Tnull ->
    let ty = (r, Tunion []) in
    env, (r, Toption ty)
  | r, Tunion tyl ->
    let env, tyl = List.map_env env tyl (push_option_out pos) in
    if List.exists tyl is_option then
      let (env, tyl), r' =
        List.fold_right tyl ~f:begin fun ty ((env, tyl), r) ->
          match ty with
          | r', Toption ty' -> flatten_unresolved env ty' tyl, r'
          | _ -> flatten_unresolved env ty tyl, r
          end ~init:((env, []), r) in
      env, (r', Toption (r, Tunion tyl))
    else
      let env, tyl =
        List.fold_right tyl ~f:begin fun ty (env, tyl) ->
          flatten_unresolved env ty tyl
          end ~init:(env, []) in
      env, (r, Tunion tyl)
  | r, Tintersection tyl ->
    let env, tyl = List.map_env env tyl (push_option_out pos) in
    if List.for_all tyl is_option then
      let r', tyl = List.fold_map tyl ~init:Reason.none
        ~f:(fun r' ty -> match ty with r, Toption ty -> r, ty | _ -> r', ty) in
      env, (r', Toption (r, Tintersection tyl))
    else
      env, (r, Tintersection tyl)
  | r, Tabstract (ak, _) ->
    begin match get_concrete_supertypes env ty with
    | env, [ty'] ->
      let env, ty' = push_option_out pos env ty' in
      (match ty' with
      | r', Toption ty' -> env, (r', Toption (r, Tabstract (ak, Some ty')))
      | _ -> env, ty)
    | env, _ -> env, ty
    end
  (* Solve type variable to lower bound if it's manifestly nullable *)
  | _, Tvar var ->
    let rec has_null env ty =
      match snd (Env.expand_type env ty) with
      | _, Tprim Nast.Tnull -> true
      | _, Toption _ -> true
      | _, Tabstract (_, Some ty) -> has_null env ty
      | _ -> false in
    let lower_bounds = Typing_set.elements (Typing_env.get_tyvar_lower_bounds env var) in
    if List.exists lower_bounds (has_null env)
    then begin
      let env, ty = expand_type_and_solve env
        ~description_of_expected:"a value of known type" pos ty in
      push_option_out pos env ty
    end
    else env, ty
  | _, (Terr | Tany | Tnonnull | Tarraykind _ | Tprim _
    | Tclass _ | Ttuple _ | Tanon _ | Tfun _
    | Tobject | Tshape _ | Tdynamic) -> env, ty

(**
 * Strips away all Toption that we possible can in a type, expanding type
 * variables along the way, turning ?T -> T. This exists to avoid ??T when
 * we wrap a type in Toption while typechecking.
 *)
let non_null env pos ty =
  let env, ty = push_option_out pos env ty in
  match ty with
  | _, Toption ty' -> env, ty'
  | _ -> env, ty

(*****************************************************************************)
(*****************************************************************************)

(* Try to unify all the types in a union *)
let rec fold_unresolved env ty =
  let env, ety = Env.expand_type env ty in
  match ety with
  | r, Tunion [] -> env, (r, Tany)
  | _, Tunion [x] -> fold_unresolved env x
  (* We don't want to use unification if new_inference is set.
   * Just return the type unchanged: better would be to remove redundant
   * elements, but let's postpone that until we have an improved
   * representation of unions.
   *)
  | _ -> env, ety

(*****************************************************************************)
(* *)
(*****************************************************************************)

let string_of_visibility = function
  | Vpublic  -> "public"
  | Vprivate _ -> "private"
  | Vprotected _ -> "protected"

let unwrap_class_type = function
  | r, Tapply (name, tparaml) -> r, name, tparaml
  | _,
    (
      Terr
      | Tdynamic
      | Tany
      | Tmixed
      | Tnonnull
      | Tnothing
      | Tarray (_, _)
      | Tdarray (_, _)
      | Tvarray _
      | Tvarray_or_darray _
      | Tgeneric _
      | Toption _
      | Tlike _
      | Tprim _
      | Tfun _
      | Ttuple _
      | Tshape _
      | Taccess (_, _)
      | Tthis
    ) ->
      raise @@ Invalid_argument "unwrap_class_type got non-class"

let try_unwrap_class_type x = Option.try_with (fun () -> unwrap_class_type x)

let class_is_final_and_not_contravariant class_ty =
  Cls.final class_ty &&
    List.for_all
      (Cls.tparams class_ty)
      ~f:(begin function
          | { tp_variance = (Ast.Invariant | Ast.Covariant); _ } -> true
          | _ -> false
          end)

(*****************************************************************************)
(* Check if a type is not fully constrained *)
(*****************************************************************************)

module HasTany : sig
  val check: locl ty -> bool
  val check_why: locl ty -> Reason.t option
end = struct

  let merge x y = Option.merge x y (fun x _ -> x)

  let visitor =
    object(this)
      inherit [Reason.t option] Type_visitor.type_visitor
      method! on_tany _ r = Some r
      method! on_tarray acc r ty1_opt ty2_opt =
        (* Check for array without its type parameters specified *)
        match ty1_opt, ty2_opt with
        | None, None -> Some r
        | _ -> merge
            (Option.fold ~f:this#on_type ~init:acc ty1_opt)
            (Option.fold ~f:this#on_type ~init:acc ty2_opt)
      method! on_tarraykind acc r akind =
        match akind with
        | AKany -> Some r
        | AKempty -> acc
        | AKvarray_or_darray ty
        | AKvarray ty
        | AKvec ty -> this#on_type acc ty
        | AKdarray (tk, tv)
        | AKmap (tk, tv) -> merge
            (this#on_type acc tk)
            (this#on_type acc tv)
    end
  let check_why ty = visitor#on_type None ty

  let check ty = Option.is_some (check_why ty)
end

(*****************************************************************************)
(* Function parameters *)
(*****************************************************************************)

let default_fun_param ?(pos=Pos.none) ty : 'a fun_param = {
  fp_pos = pos;
  fp_name = None;
  fp_type = ty;
  fp_kind = FPnormal;
  fp_accept_disposable = false;
  fp_mutability = None;
  fp_rx_annotation = None;
}

let fun_mutable user_attributes =
  let rec go = function
  | [] -> None
  | { Nast.ua_name = (_, n); _ } :: _ when n = SN.UserAttributes.uaMutable ->
    Some Param_borrowed_mutable
  | { Nast.ua_name = (_, n); _ } :: _ when n = SN.UserAttributes.uaMaybeMutable ->
    Some Param_maybe_mutable
  | _ :: tl -> go tl in
  go user_attributes

let tany = Env.tany
let terr env =
  let dynamic_view_enabled =
    TypecheckerOptions.dynamic_view (Typing_env.get_tcopt env) in
  if dynamic_view_enabled then Tdynamic else Terr

(* Hacked version of Typing_subtype.try_intersect for collecting function types *)
let add_function_type env fty logged =
  let (untyped_ftys, ftys) = logged in
  let rec try_intersect env ty tyl =
  match tyl with
  | [] -> [ty]
  | ty'::tyl' ->
    if is_sub_type_for_union env ty ty' && not (HasTany.check ty)
    then try_intersect env ty tyl'
    else
    if is_sub_type_for_union env ty' ty && not (HasTany.check ty')
    then try_intersect env ty' tyl'
    else ty' :: try_intersect env ty tyl'
in
  if HasTany.check fty
  then (try_intersect env fty untyped_ftys, ftys)
  else (untyped_ftys, try_intersect env fty ftys)
