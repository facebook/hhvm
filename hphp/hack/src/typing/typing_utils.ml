(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core
open Typing_defs

module N = Nast
module SN = Naming_special_names
module Reason = Typing_reason
module Env = Typing_env
module ShapeMap = Nast.ShapeMap
module TySet = Typing_set

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

type expand_typeconst =
  expand_env -> Env.env -> Reason.t -> locl ty -> Nast.sid list ->
  Env.env * ety
let (expand_typeconst_ref: expand_typeconst ref) = ref not_implemented
let expand_typeconst x = !expand_typeconst_ref x

(* Convenience function for creating `this` types *)
let this_of ty = Tabstract (AKdependent (`this, []), Some ty)

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

let is_shape_field_optional env { sft_optional; sft_ty } =
  let optional_shape_field_enabled =
    TypecheckerOptions.experimental_feature_enabled
      (Env.get_options env)
      TypecheckerOptions.experimental_optional_shape_field in

  if optional_shape_field_enabled then
    sft_optional
  else
    is_option env sft_ty

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
        else iter (SSet.add n seen) env acc (Env.get_upper_bounds env n @ tyl)
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
 * (For example, functon foo<Tu as Tv, Tv as Tu>(...))
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
        else iter (SSet.add n seen) env acc (Env.get_upper_bounds env n @ tyl)

      | Tabstract (_, None) ->
        iter seen env acc tyl
      | _ ->
        iter seen env (TySet.add ty acc) tyl
  in
    let env, resl = iter SSet.empty env TySet.empty [ty] in
    env, TySet.elements resl
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
    begin match Env.get_upper_bounds env n with
    | ty2::_ when ty_equal ty ty2 -> ty
    | ty::_ -> get_base_type env ty
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
  (* Need this check to ensure we don't enter an infiinite loop *)
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

let process_class_id = function
  | Nast.CI (c, _) ->
    Decl_hooks.dispatch_class_id_hook c None;
  | _ -> ()

let process_static_find_ref cid mid =
  match cid with
  | Nast.CI (c, _) ->
    Decl_hooks.dispatch_class_id_hook c (Some mid);
  | _ -> ()

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
  field that is missing in the second (on_missing_optional_field).

  The unset fields of the first shape (empty if it is fully known) must be a
  subset of the unset fields of the second shape. An error will be reported
  otherwise.

  If the first shape has an optional field that is not present and not
  explicitly unset in the second shape (i.e. the second shape is partially known
  and the field is not listed in its unset_fields), then
  Error.missing_optional_field will be emitted.

  This is used in subtyping and unification. When subtyping, the first and
  second fields are respectively the supertype and subtype.

  Example of Error.missing_optional_field for subtyping:
    s = shape('x' => int, ...)
    t = shape('x' => int, ?'z' => bool)
    $s = shape('x' => 1, 'z' => 2)
    $s is a subtype of s
    $s is not a subtype of t
    If s is a subtype of t, then by transitivity, $s is a subtype of t
      --> contradiction
    We prevent this by making sure that (apply_changes ... t s) fails because
    t has an optional field 'z' that is not unset by s.
*)
let apply_shape ~on_common_field ~on_missing_optional_field (env, acc)
  (r1, fields_known1, fdm1) (r2, fields_known2, fdm2) =
  begin match fields_known1, fields_known2 with
    | FieldsFullyKnown, FieldsPartiallyKnown _  ->
        let pos1 = Reason.to_pos r1 in
        let pos2 = Reason.to_pos r2 in
        Errors.shape_fields_unknown pos2 pos1
    | FieldsPartiallyKnown unset_fields1,
      FieldsPartiallyKnown unset_fields2 ->
        ShapeMap.iter begin fun name unset_pos ->
          match ShapeMap.get name unset_fields2 with
            | Some _ -> ()
            | None ->
                let pos2 = Reason.to_pos r2 in
                Errors.shape_field_unset unset_pos pos2
                  (get_printable_shape_field_name name);
        end unset_fields1
    | _ -> ()
  end;
  ShapeMap.fold begin fun name shape_field_type_1 (env, acc) ->
    match ShapeMap.get name fdm2 with
    | None when is_shape_field_optional env shape_field_type_1 ->
        let can_omit = match fields_known2 with
          | FieldsFullyKnown -> true
          | FieldsPartiallyKnown unset_fields ->
              ShapeMap.mem name unset_fields in
        if can_omit then
          on_missing_optional_field (env, acc) name shape_field_type_1
        else
          let pos1 = Reason.to_pos r1 in
          let pos2 = Reason.to_pos r2 in
          Errors.missing_optional_field pos2 pos1
            (get_printable_shape_field_name name);
        (env, acc)
    | None ->
        let pos1 = Reason.to_pos r1 in
        let pos2 = Reason.to_pos r2 in
        Errors.missing_field pos2 pos1 (get_printable_shape_field_name name);
        (env, acc)
    | Some shape_field_type_2 ->
        on_common_field (env, acc) name shape_field_type_1 shape_field_type_2
  end fdm1 (env, acc)

let shape_field_name_ env field =
  let open Nast in match field with
    | String name -> Result.Ok (Ast.SFlit name)
    | Class_const (CI (x, _), y) -> Result.Ok (Ast.SFclass_const (x, y))
    | Class_const (CIself, y) ->
      let _, c_ty = Env.get_self env in
      (match c_ty with
      | Tclass (sid, _) ->
        Result.Ok (Ast.SFclass_const(sid, y))
      | _ ->
        Result.Error `Expected_class)
    | _ -> Result.Error `Invalid_shape_field_name

let maybe_shape_field_name env field =
  match shape_field_name_ env field with
    | Result.Ok x -> Some x
    | Result.Error _ -> None

let shape_field_name env p field =
  match shape_field_name_ env field with
    | Result.Ok x -> Some x
    | Result.Error `Expected_class ->
        Errors.expected_class p;
        None
    | Result.Error `Invalid_shape_field_name ->
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
    (* TODO this should probably pass through the uenv *)
    normalize_inter env tyl1 tyl2

(*****************************************************************************)
(* *)
(*****************************************************************************)

let in_var env ty =
  let res = Env.fresh_type() in
  let env, res = unify env ty res in
  env, res

let unresolved_tparam env (_, (pos, _), _) =
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
      | Tany
      | Tmixed
      | Tarray (_, _)
      | Tdarray (_, _)
      | Tvarray _
      | Tdarray_or_varray _
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
          (Ast.Invariant | Ast.Covariant), _, _ -> true
          | _, _, _ -> false
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
        | AKdarray_or_varray ty
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
