(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open Typing_defs

module N = Nast
module SN = Naming_special_names
module Reason = Typing_reason
module Env = Typing_env
module ShapeMap = Nast.ShapeMap
module TySet = Typing_set

(* This can be useful to debug type which blow up in size *)
let ty_size ty =
  let ty_size_visitor = object
    inherit [int] Type_visitor.type_visitor as super
    method! on_type acc ty = 1 + super#on_type acc ty
    end in
  ty_size_visitor#on_type 0 ty

(*****************************************************************************)
(* Importing what is necessary *)
(*****************************************************************************)
let not_implemented _ = failwith "Function not implemented"
type expand_typedef =
    expand_env -> Env.env -> Reason.t -> string -> locl ty list -> Env.env * ety
let (expand_typedef_ref : expand_typedef ref) = ref not_implemented
let expand_typedef x = !expand_typedef_ref x

(* Options to check for while unifying *)
type unify_options = {
  (* If follow_bounds=false, only match generic parameters with themselves.
   * If follow_bounds=true, look in lower and upper bounds of generic parameters,
   * for example, to unify T and t if there are bounds T as t and T super t.
   *)
  follow_bounds : bool;
  (* Whether we simplify our reason information after encountering a unification
  error *)
  simplify_errors : bool;
}


let default_unify_opt = {
  follow_bounds = true;
  simplify_errors = true;
}

let unify_fake ?(opts=default_unify_opt) = not_implemented opts

type unify =
  ?opts:unify_options -> Env.env -> locl ty -> locl ty -> Env.env * locl ty
let (unify_ref: unify ref) = ref unify_fake
let unify ?(opts=default_unify_opt) = !unify_ref ~opts

type sub_type = Env.env -> locl ty -> locl ty -> Env.env
let (sub_type_ref: sub_type ref) = ref not_implemented
let sub_type x = !sub_type_ref x

type is_sub_type_type = Env.env -> locl ty -> locl ty -> bool
let (is_sub_type_ref: is_sub_type_type ref) = ref not_implemented
let is_sub_type x = !is_sub_type_ref x

type add_constraint = Pos.Map.key -> Env.env -> Ast.constraint_kind -> locl ty -> locl ty -> Env.env
let (add_constraint_ref: add_constraint ref) = ref not_implemented
let add_constraint x = !add_constraint_ref x

type expand_typeconst =
  expand_env -> Env.env -> Reason.t -> locl ty -> Nast.sid list ->
  Env.env * ety
let (expand_typeconst_ref: expand_typeconst ref) = ref not_implemented
let expand_typeconst x = !expand_typeconst_ref x

(* Convenience function for creating `this` types *)
let this_of ty = Tabstract (AKdependent (`this, []), Some ty)

let is_void_type_of_null env =
  TypecheckerOptions.experimental_feature_enabled
    (Env.get_options env)
    TypecheckerOptions.experimental_void_is_type_of_null

(*****************************************************************************)
(* Returns true if a type is optional *)
(*****************************************************************************)

let rec is_option env ty =
  let _, ety = Env.expand_type env ty in
  match snd ety with
  | Toption _ -> true
  | Tunresolved tyl ->
      List.exists tyl (is_option env)
  | _ -> false

let ensure_option env r ty = if is_option env ty then ty else (r, Toption ty)

let rec is_option_non_mixed env ty =
  let _, ety = Env.expand_type env ty in
  match snd ety with
  | Toption (_, Tnonnull) -> false
  | Toption _ -> true
  | Tprim Nast.Tvoid -> is_void_type_of_null env
  | Tunresolved tyl ->
      List.exists tyl (is_option_non_mixed env)
  | _ -> false

let is_shape_field_optional env { sft_optional; sft_ty } =
  let optional_shape_field_enabled =
    not @@
      TypecheckerOptions.experimental_feature_enabled
        (Env.get_options env)
        TypecheckerOptions.experimental_disable_optional_and_unknown_shape_fields in

  if optional_shape_field_enabled then
    sft_optional
  else
    is_option env sft_ty || sft_optional

let is_class ty = match snd ty with
  | Tclass _ -> true
  | _ -> false

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
      | _ ->
        iter seen env (TySet.add ty acc) tyl
  in
    let env, resl = iter SSet.empty env TySet.empty [ty] in
    env, TySet.elements resl


(*****************************************************************************)
(* Dynamicism  *)
(*****************************************************************************)
let rec find_dynamic env tyl =
  match tyl with
   | [] -> None
   | ty::tys ->
    begin match Env.expand_type env ty with
    | (_, (_, Tdynamic)) ->
      Some ty
    | (_, (_, Tunresolved tyl)) -> find_dynamic env (tys@tyl)
    | _ -> find_dynamic env tys end


let is_dynamic env ty =
  find_dynamic env [ty] <> None

(*****************************************************************************)
(* Check if type is any or a variant thereof  *)
(*****************************************************************************)

let rec is_any env ty =
  match Env.expand_type env ty with
  | (_, (_, (Tany | Terr))) -> true
  | (_, (_, Tunresolved tyl)) -> List.for_all tyl (is_any env)
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
    | _, Tclass ((_, cid), _) -> cid::acc
    | _, (Toption ty | Tabstract (_, Some ty)) -> aux seen acc ty
    | _, Tunresolved tys -> List.fold tys ~init:acc ~f:(aux seen)
    | _, Tabstract (AKgeneric name, None) when not (List.mem seen name) ->
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
    | Nonreactive -> "non-reactive" in
  aux r

(*****************************************************************************)
(* Unification error *)
(*****************************************************************************)
let uerror r1 ty1 r2 ty2 =
  let ty1 = Typing_print.error ty1 in
  let ty2 = Typing_print.error ty2 in
  Errors.unify_error
    (Reason.to_string ("This is " ^ ty1) r1)
    (Reason.to_string ("It is incompatible with " ^ ty2) r2)

(* We attempt to simplify the unification error to see if it can be
 * explained without referring to dependent types.
 *)
let simplified_uerror env ty1 ty2 =
  (* Need this check to ensure we don't enter an infinite loop *)
  let simplify = match snd ty1, snd ty2 with
    | Tabstract (AKdependent (`static, []), _), Tclass _
    | Tclass _, Tabstract (AKdependent (`static, []), _) -> false
    | Tabstract (AKdependent _, Some _), _
    | _, Tabstract (AKdependent _, Some _) -> true
    | Tabstract(AKgeneric s, _), _ ->
      let base_ty1 = get_base_type env ty1 in
      AbstractKind.is_generic_dep_ty s &&
      not (ty_equal ty1 base_ty1)
    | _, Tabstract(AKgeneric s, _) ->
      let base_ty2 = get_base_type env ty2 in
      AbstractKind.is_generic_dep_ty s &&
      not (ty_equal ty2 base_ty2)
    | _, _ -> false in

  let opts = {
    follow_bounds = true;
    simplify_errors = false;
  } in
  (* We unify the base types to see if that produces an error, if not then
   * we use the standard unification error
   *)
  if simplify then
    Errors.must_error
      (fun _ ->
          ignore @@ unify ~opts env (get_base_type env ty1) (get_base_type env ty2)
      )
      (fun _ -> uerror (fst ty1) (snd ty1) (fst ty2) (snd ty2))
  else
    uerror (fst ty1) (snd ty1) (fst ty2) (snd ty2)

(* Find the first defined position in a list of types *)
let rec find_pos p_default tyl =
  match tyl with
  | [] -> p_default
  | (r, _) :: rl ->
      let p = Reason.to_pos r in
      if p = Pos.none
      then find_pos p_default rl
      else p

(*****************************************************************************)
(* Applies a function to 2 shapes simultaneously, raises an error if
 * the second argument has less fields than the first.
 *)
(*****************************************************************************)

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
    | None when is_shape_field_optional env shape_field_type_1 ->
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
    | _, Class_const ((_, CI (x, _)), y) -> Ok (Ast.SFclass_const (x, y))
    | _, Class_const ((_, CIself), y) ->
      let _, c_ty = Env.get_self env in
      (match c_ty with
      | Tclass (sid, _) ->
        Ok (Ast.SFclass_const(sid, y))
      | _ ->
        Error `Expected_class)
    | _ -> Error `Invalid_shape_field_name

let maybe_shape_field_name env field =
  match shape_field_name_ env field with
    | Ok x -> Some x
    | Error _ -> None

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
    (* flatten Tunresolved[Tunresolved[...]] *)
    | (_, Tunresolved tyl) -> tyl @ acc
    | (_, Tany) -> acc
    | _ -> ty :: acc in
  env, res

let rec member_inter env ty tyl acc =
  match tyl with
  | [] -> env, ty :: acc
  | x :: rl ->
      Errors.try_
        begin fun () ->
          let env, ty = unify env x ty in
          env, List.rev_append acc (ty :: rl)
        end
        begin fun _ ->
          member_inter env ty rl (x :: acc)
        end

and normalize_inter env tyl1 tyl2 =
  match tyl1 with
  | [] -> env, tyl2
  | x :: rl ->
      let env, tyl2 = member_inter env x tyl2 [] in
      normalize_inter env rl tyl2

let normalize_inter env tyl1 tyl2 =
  if List.length tyl1 + List.length tyl2 > 100
  then
    (* normalization is O(len(tyl1) * len(tyl2)), so just appending is
     * a significant perf win here *)
    env, (List.rev_append tyl1 tyl2)
  else
  begin match find_dynamic env tyl1 with
    | Some ty -> env, [ty]
    | None ->
      begin match find_dynamic env tyl2 with
      | Some ty -> env, [ty]
      | None ->
        (* TODO this should probably pass through the uenv *)
        normalize_inter env tyl1 tyl2
      end
  end

let rec push_option_out env ty =
  let is_option = function
    | _, Toption _ -> true
    | _ -> false in
  let env, ty = Env.expand_type env ty in
  match ty with
  | r, Toption ty ->
    let env, ty = push_option_out env ty in
    env, if is_option ty then ty else (r, Toption ty)
  | r, Tprim N.Tvoid when is_void_type_of_null env ->
    env, (r, Toption (r, Tany))
  | r, Tunresolved tyl ->
    let env, tyl = List.map_env env tyl push_option_out in
    if List.exists tyl is_option then
      let (env, tyl), r' =
        List.fold_right tyl ~f:begin fun ty ((env, tyl), r) ->
          match ty with
          | r', Toption ty' -> flatten_unresolved env ty' tyl, r'
          | _ -> flatten_unresolved env ty tyl, r
          end ~init:((env, []), r) in
      env, (r', Toption (r, Tunresolved tyl))
    else
      let env, tyl =
        List.fold_right tyl ~f:begin fun ty (env, tyl) ->
          flatten_unresolved env ty tyl
          end ~init:(env, []) in
      env, (r, Tunresolved tyl)
  | r, Tabstract (ak, _) ->
    begin match get_concrete_supertypes env ty with
    | env, [ty'] ->
      let env, ty' = push_option_out env ty' in
      (match ty' with
      | r', Toption ty' -> env, (r', Toption (r, Tabstract (ak, Some ty')))
      | _ -> env, ty)
    | env, _ -> env, ty
    end
  | _, (Terr | Tany | Tmixed | Tnonnull | Tarraykind _ | Tprim _ | Tvar _
    | Tclass (_, _) | Ttuple _ | Tanon (_, _) | Tfun _
    | Tobject | Tshape _ | Tdynamic) -> env, ty

(**
 * Strips away all Toption that we possible can in a type, expanding type
 * variables along the way, turning ?T -> T. This exists to avoid ??T when
 * we wrap a type in Toption while typechecking.
 *)
let non_null env ty =
  let env, ty = push_option_out env ty in
  match ty with
  | _, Toption ty' -> env, ty'
  | _ -> env, ty

(*****************************************************************************)
(* *)
(*****************************************************************************)

let in_var env ty =
  let x = Env.fresh () in
  let env = Env.add env x ty in
  env, (fst ty, Tvar x)

let unresolved_tparam env (_, (pos, _), _, _) =
  let reason = Reason.Rwitness pos in
  in_var env (reason, Tunresolved [])

(*****************************************************************************)
(*****************************************************************************)

(* Try to unify all the types in a intersection *)
let rec fold_unresolved env ty =
  let env, ety = Env.expand_type env ty in
  match ety with
  | r, Tunresolved [] -> env, (r, Tany)
  | _, Tunresolved [x] -> fold_unresolved env x
  | _, Tunresolved (x :: rl) ->
      (try
        let env, acc =
          List.fold_left rl ~f:begin fun (env, acc) ty ->
            Errors.try_ (fun () -> unify env acc ty) (fun _ -> raise Exit)
          end ~init:(env, x) in
        env, acc
      with Exit ->
        env, ty
      )
  | _ -> env, ty

(*****************************************************************************)
(* *)
(*****************************************************************************)

let string_of_visibility = function
  | Vpublic  -> "public"
  | Vprivate _ -> "private"
  | Vprotected _ -> "protected"

let unresolved env ty =
  let env, ety = Env.expand_type env ty in
  match ety with
  | _, Tunresolved _ -> in_var env ety
  | _ -> in_var env (fst ty, Tunresolved [ty])

let unwrap_class_hint = function
  | (_, N.Happly ((pos, class_name), type_parameters)) ->
      pos, class_name, type_parameters
  | p, N.Habstr _ ->
      Errors.expected_class ~suffix:" or interface but got a generic" p;
      Pos.none, "", []
  | p, _ ->
      Errors.expected_class ~suffix:" or interface" p;
      Pos.none, "", []

let unwrap_class_type = function
  | r, Tapply (name, tparaml) -> r, name, tparaml
  | _,
    (
      Terr
      | Tdynamic
      | Tany
      | Tmixed
      | Tnonnull
      | Tarray (_, _)
      | Tdarray (_, _)
      | Tvarray _
      | Tvarray_or_darray _
      | Tgeneric _
      | Toption _
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
  class_ty.tc_final &&
    List.for_all
      class_ty.tc_tparams
      ~f:(begin function
          (Ast.Invariant | Ast.Covariant), _, _, _ -> true
          | _, _, _, _ -> false
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
        | AKshape fdm -> ShapeMap.fold (fun _ (tk, tv) acc ->
            merge
              (this#on_type acc tk)
              (this#on_type acc tv)
          ) fdm acc
        | AKtuple fields ->
          IMap.fold (fun _ ty acc -> this#on_type acc ty) fields acc
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
  fp_rx_condition = None;
}

let fun_mutable user_attributes =
  Attributes.mem SN.UserAttributes.uaMutable user_attributes

let desugar_mixed r = Toption (r, Tnonnull)

let tany env =
  let dynamic_view_enabled =
    TypecheckerOptions.dynamic_view (Typing_env.get_tcopt env) in
  if dynamic_view_enabled then Tdynamic else Tany

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
    if is_sub_type env ty ty' && not (HasTany.check ty)
    then try_intersect env ty tyl'
    else
    if is_sub_type env ty' ty && not (HasTany.check ty')
    then try_intersect env ty' tyl'
    else ty' :: try_intersect env ty tyl'
in
  if HasTany.check fty
  then (try_intersect env fty untyped_ftys, ftys)
  else (untyped_ftys, try_intersect env fty ftys)
