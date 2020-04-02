(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs_flags
include Typing_defs_core

let xhp_attr_to_ce_flags xa =
  match xa with
  | None -> 0x0
  | Some { xa_tag; xa_has_default } ->
    Int.bit_or
      ( if xa_has_default then
        ce_flags_xa_has_default
      else
        0x0 )
    @@
    (match xa_tag with
    | None -> ce_flags_xa_tag_none
    | Some Required -> ce_flags_xa_tag_required
    | Some Lateinit -> ce_flags_xa_tag_lateinit)

let get_ce_flags_xhp_attr flags =
  let tag_flags = Int.bit_and ce_flags_xa_tag_mask flags in
  if Int.equal tag_flags 0 then
    None
  else
    Some
      {
        xa_has_default = is_set ce_flags_xa_has_default flags;
        xa_tag =
          ( if Int.equal tag_flags ce_flags_xa_tag_none then
            None
          else if Int.equal tag_flags ce_flags_xa_tag_required then
            Some Required
          else
            Some Lateinit );
      }

let get_ft_return_disposable ft = is_set ft.ft_flags ft_flags_return_disposable

let get_ft_returns_void_to_rx ft =
  is_set ft.ft_flags ft_flags_returns_void_to_rx

let get_ft_returns_mutable ft = is_set ft.ft_flags ft_flags_returns_mutable

let get_ft_is_coroutine ft = is_set ft.ft_flags ft_flags_is_coroutine

let get_ft_async ft = is_set ft.ft_flags ft_flags_async

let get_ft_generator ft = is_set ft.ft_flags ft_flags_generator

let get_ft_ftk ft =
  if is_set ft.ft_flags ft_flags_instantiated_targs then
    FTKinstantiated_targs
  else
    FTKtparams

let set_ft_ftk ft ftk =
  {
    ft with
    ft_flags =
      set_bit
        ft_flags_instantiated_targs
        (match ftk with
        | FTKinstantiated_targs -> true
        | FTKtparams -> false)
        ft.ft_flags;
  }

let get_ft_fun_kind ft =
  if get_ft_is_coroutine ft then
    Ast_defs.FCoroutine
  else
    match (get_ft_async ft, get_ft_generator ft) with
    | (false, false) -> Ast_defs.FSync
    | (true, false) -> Ast_defs.FAsync
    | (false, true) -> Ast_defs.FGenerator
    | (true, true) -> Ast_defs.FAsyncGenerator

let get_ft_param_mutable ft =
  match Int.bit_and ft.ft_flags 0xC0 with
  | 0x0 -> None
  | 0x40 -> Some Param_owned_mutable
  | 0x80 -> Some Param_borrowed_mutable
  | 0xC0 -> Some Param_maybe_mutable
  | _ -> failwith "get_ft_param_mutable"

let param_mutable_to_flags m =
  match m with
  | None -> 0x0
  | Some Param_owned_mutable -> 0x40
  | Some Param_borrowed_mutable -> 0x80
  | Some Param_maybe_mutable -> 0xC0

let fun_kind_to_flags kind =
  match kind with
  | Ast_defs.FSync -> 0
  | Ast_defs.FAsync -> ft_flags_async
  | Ast_defs.FGenerator -> ft_flags_generator
  | Ast_defs.FAsyncGenerator -> Int.bit_or ft_flags_async ft_flags_generator
  | Ast_defs.FCoroutine -> ft_flags_is_coroutine

let make_ft_flags
    kind param_mutable ~return_disposable ~returns_mutable ~returns_void_to_rx =
  let flags =
    Int.bit_or (param_mutable_to_flags param_mutable) (fun_kind_to_flags kind)
  in
  let flags = set_bit ft_flags_return_disposable return_disposable flags in
  let flags = set_bit ft_flags_returns_mutable returns_mutable flags in
  let flags = set_bit ft_flags_returns_void_to_rx returns_void_to_rx flags in
  flags

type class_elt = {
  ce_visibility: visibility;
  ce_type: decl_ty Lazy.t;
  ce_origin: string;  (** identifies the class from which this elt originates *)
  ce_deprecated: string option;
  ce_pos: Pos.t Lazy.t;
  ce_flags: int;
}

and fun_elt = {
  fe_deprecated: string option;
  fe_type: decl_ty;
  fe_decl_errors: Errors.t option;
  fe_pos: Pos.t;
}

and class_const = {
  cc_synthesized: bool;
  cc_abstract: bool;
  cc_pos: Pos.t;
  cc_type: decl_ty;
  cc_expr: Nast.expr option;
  cc_origin: string;
      (** identifies the class from which this const originates *)
}

(** The position is that of the hint in the `use` / `implements` AST node
 * that causes a class to have this requirement applied to it. E.g.
 *
 * ```
 * class Foo {}
 *
 * interface Bar {
 *   require extends Foo; <- position of the decl_phase ty
 * }
 *
 * class Baz extends Foo implements Bar { <- position of the `implements`
 * }
 * ```
 *)
and requirement = Pos.t * decl_ty

and class_type = {
  tc_need_init: bool;
  tc_members_fully_known: bool;
      (** Whether the typechecker knows of all (non-interface) ancestors
       * and thus known all accessible members of this class *)
  tc_abstract: bool;
  tc_final: bool;
  tc_const: bool;
  tc_ppl: bool;
      (** True when the class is annotated with the __PPL attribute. *)
  tc_deferred_init_members: SSet.t;
      (** When a class is abstract (or in a trait) the initialization of
       * a protected member can be delayed *)
  tc_kind: Ast_defs.class_kind;
  tc_is_xhp: bool;
  tc_has_xhp_keyword: bool;
  tc_is_disposable: bool;
  tc_name: string;
  tc_pos: Pos.t;
  tc_tparams: decl_tparam list;
  tc_where_constraints: decl_where_constraint list;
  tc_consts: class_const SMap.t;
  tc_typeconsts: typeconst_type SMap.t;
  tc_pu_enums: pu_enum_type SMap.t;
  tc_props: class_elt SMap.t;
  tc_sprops: class_elt SMap.t;
  tc_methods: class_elt SMap.t;
  tc_smethods: class_elt SMap.t;
  tc_construct: class_elt option * consistent_kind;
      (** the consistent_kind represents final constructor or __ConsistentConstruct *)
  tc_ancestors: decl_ty SMap.t;
      (** This includes all the classes, interfaces and traits this class is
       * using. *)
  tc_req_ancestors: requirement list;
  tc_req_ancestors_extends: SSet.t;  (** the extends of req_ancestors *)
  tc_extends: SSet.t;
  tc_enum_type: enum_type option;
  tc_sealed_whitelist: SSet.t option;
  tc_decl_errors: Errors.t option;
}

and typeconst_abstract_kind =
  | TCAbstract of decl_ty option
  | TCPartiallyAbstract
  | TCConcrete

and typeconst_type = {
  ttc_abstract: typeconst_abstract_kind;
  ttc_name: Nast.sid;
  ttc_constraint: decl_ty option;
  ttc_type: decl_ty option;
  ttc_origin: string;
  ttc_enforceable: Pos.t * bool;
  ttc_reifiable: Pos.t option;
}

and pu_enum_type = {
  tpu_name: Nast.sid;
  tpu_is_final: bool;
  tpu_case_types: (Nast.sid * Aast.reify_kind) SMap.t;
  tpu_case_values: (Nast.sid * decl_ty) SMap.t;
  tpu_members: pu_member_type SMap.t;
}

and pu_member_type = {
  tpum_atom: Nast.sid;
  tpum_types: (Nast.sid * decl_ty) SMap.t;
  tpum_exprs: Nast.sid SMap.t;
}

and enum_type = {
  te_base: decl_ty;
  te_constraint: decl_ty option;
}

and record_field_req =
  | ValueRequired
  | HasDefaultValue

and record_def_type = {
  rdt_name: Nast.sid;
  rdt_extends: Nast.sid option;
  rdt_fields: (Nast.sid * record_field_req) list;
  rdt_abstract: bool;
  rdt_pos: Pos.t;
  rdt_errors: Errors.t option;
}

and typedef_type = {
  td_pos: Pos.t;
  td_vis: Aast.typedef_visibility;
  td_tparams: decl_tparam list;
  td_constraint: decl_ty option;
  td_type: decl_ty;
  td_decl_errors: Errors.t option;
}

and decl_tparam = decl_ty tparam

and locl_tparam = locl_ty tparam

and decl_where_constraint = decl_ty where_constraint

and locl_where_constraint = locl_ty where_constraint

type phase_ty =
  | DeclTy of decl_ty
  | LoclTy of locl_ty

type deserialization_error =
  | Wrong_phase of string
      (** The type was valid, but some component thereof was a decl_ty when we
          expected a locl_phase ty, or vice versa. *)
  | Not_supported of string
      (** The specific type or some component thereof is not one that we support
          deserializing, usually because not enough information was serialized to be
          able to deserialize it again. *)
  | Deserialization_error of string
      (** The input JSON was invalid for some reason. *)

(** Tracks information about how a type was expanded *)
type expand_env = {
  type_expansions: (Pos.t * string) list;
      (** A list of the type defs and type access we have expanded thus far. Used
       * to prevent entering into a cycle when expanding these types
       *)
  substs: locl_ty SMap.t;
  this_ty: locl_ty;
  from_class: Nast.class_id_ option;
      (** The class that the type is extracted from. Used for creating expression
       * dependent types for type constants.
       *)
  quiet: bool;
  on_error: Errors.typing_error_callback;
      (** If what we are localizing or expanding comes from the decl heap for
          example, then some errors must be silenced since they must have already been
          raised when first typechecking whatever we have fetched from the heap.
          Setting {!quiet} to true will silence those errors.
          T54121530 aims at offering a better mechanism. *)
}

let get_var t =
  match get_node t with
  | Tvar v -> Some v
  | _ -> None

let get_var_i t =
  match t with
  | LoclType t -> get_var t
  | ConstraintType _ -> None

let is_tyvar t = Option.is_some (get_var t)

let is_var_v t v =
  match get_node t with
  | Tvar v' when Ident.equal v v' -> true
  | _ -> false

let is_generic t =
  match get_node t with
  | Tgeneric _ -> true
  | _ -> false

let is_dynamic t =
  match get_node t with
  | Tdynamic -> true
  | _ -> false

let is_nonnull t =
  match get_node t with
  | Tnonnull -> true
  | _ -> false

let is_fun t =
  match get_node t with
  | Tfun _ -> true
  | _ -> false

let is_any t =
  match get_node t with
  | Tany _ -> true
  | _ -> false

let is_generic_equal_to n t =
  match get_node t with
  | Tgeneric n' when String.equal n n' -> true
  | _ -> false

let is_prim p t =
  match get_node t with
  | Tprim p' when Aast.equal_tprim p p' -> true
  | _ -> false

let is_union t =
  match get_node t with
  | Tunion _ -> true
  | _ -> false

let is_constraint_type_union t =
  match deref_constraint_type t with
  | (_, TCunion _) -> true
  | _ -> false

let show_phase_ty _ = "<phase_ty>"

let pp_phase_ty _ _ = Printf.printf "%s\n" "<phase_ty>"

let is_locl_type = function
  | LoclType _ -> true
  | _ -> false

let has_expanded { type_expansions; _ } x =
  List.exists type_expansions (function
      | (_, x') when String.equal x x' -> true
      | _ -> false)

let reason = function
  | LoclType t -> fst (deref t)
  | ConstraintType t -> fst (deref_constraint_type t)

let is_constraint_type = function
  | ConstraintType _ -> true
  | LoclType _ -> false

let is_union_or_inter_type (ty : locl_ty) =
  (* do not expand type here! *)
  match get_node ty with
  | Toption _
  | Tunion _
  | Tintersection _ ->
    true
  | Terr
  | Tnonnull
  | Tdynamic
  | Tobject
  | Tany _
  | Tprim _
  | Tfun _
  | Ttuple _
  | Tshape _
  | Tpu_type_access _
  | Tpu _
  | Tvar _
  | Tnewtype _
  | Tdependent _
  | Tgeneric _
  | Tclass _
  | Tarraykind _ ->
    false

module InternalType = struct
  let get_var t =
    match t with
    | LoclType t -> get_var t
    | ConstraintType _ -> None

  let is_var_v t ~v =
    match t with
    | LoclType t -> is_var_v t v
    | ConstraintType _ -> false

  let is_not_var_v t ~v = not @@ is_var_v t ~v
end

(* The identifier for this *)
let this = Local_id.make_scoped "$this"

(* This should be the ONLY way that Tany is constructed anywhere in the
 * codebase. *)
let make_tany () = Tany TanySentinel.value

let arity_min ft_arity : int =
  match ft_arity with
  | Fstandard min
  | Fvariadic (min, _)
  | Fellipsis (min, _) ->
    min

let get_param_mode callconv =
  match callconv with
  | Some Ast_defs.Pinout -> FPinout
  | None -> FPnormal

module DependentKind = struct
  let to_string = function
    | DTthis -> SN.Typehints.this
    | DTcls c -> c
    | DTexpr i ->
      let display_id = Reason.get_expr_display_id i in
      "<expr#" ^ string_of_int display_id ^ ">"

  let is_generic_dep_ty s = String_utils.is_substring "::" s
end

module ShapeFieldMap = struct
  include Nast.ShapeMap

  let map_and_rekey shape_map key_f value_f =
    let f_over_shape_field_type ({ sft_ty; _ } as shape_field_type) =
      { shape_field_type with sft_ty = value_f sft_ty }
    in
    Nast.ShapeMap.map_and_rekey shape_map key_f f_over_shape_field_type

  let map_env f env shape_map =
    let f_over_shape_field_type env _key ({ sft_ty; _ } as shape_field_type) =
      let (env, sft_ty) = f env sft_ty in
      (env, { shape_field_type with sft_ty })
    in
    Nast.ShapeMap.map_env f_over_shape_field_type env shape_map

  let map f shape_map = map_and_rekey shape_map (fun x -> x) f

  let iter f shape_map =
    let f_over_shape_field_type shape_map_key { sft_ty; _ } =
      f shape_map_key sft_ty
    in
    Nast.ShapeMap.iter f_over_shape_field_type shape_map

  let iter_values f = iter (fun _ -> f)
end

module ShapeFieldList = struct
  include Hh_core.List

  let map_env env xs ~f =
    let f_over_shape_field_type env ({ sft_ty; _ } as shape_field_type) =
      let (env, sft_ty) = f env sft_ty in
      (env, { shape_field_type with sft_ty })
    in
    Hh_core.List.map_env env xs ~f:f_over_shape_field_type
end

(*****************************************************************************)
(* Suggest mode *)
(*****************************************************************************)

(* Set to true when we are trying to infer the missing type hints. *)
let is_suggest_mode = ref false

(* Ordinal value for type constructor, for localized types *)
let ty_con_ordinal ty_ =
  match ty_ with
  | Tany _
  | Terr ->
    0
  | Toption t ->
    begin
      match get_node t with
      | Tnonnull -> 1
      | _ -> 4
    end
  | Tnonnull -> 2
  | Tdynamic -> 3
  | Tprim _ -> 5
  | Tfun _ -> 6
  | Ttuple _ -> 7
  | Tshape _ -> 8
  | Tvar _ -> 9
  | Tnewtype _ -> 10
  | Tgeneric _ -> 11
  | Tdependent _ -> 12
  | Tunion _ -> 13
  | Tintersection _ -> 14
  | Tobject -> 15
  | Tclass _ -> 16
  | Tarraykind _ -> 17
  | Tpu _ -> 18
  | Tpu_type_access _ -> 19

(* Ordinal value for type constructor, for decl types *)
let decl_ty_con_ordinal ty_ =
  match ty_ with
  | Tany _
  | Terr ->
    0
  | Tthis -> 1
  | Tapply _ -> 2
  | Tgeneric _ -> 3
  | Taccess _ -> 4
  | Tarray _ -> 5
  | Tdarray _ -> 6
  | Tvarray _ -> 7
  | Tvarray_or_darray _ -> 8
  | Tmixed -> 9
  | Tlike _ -> 10
  | Tnonnull -> 11
  | Tdynamic -> 12
  | Toption _ -> 13
  | Tprim _ -> 14
  | Tfun _ -> 15
  | Ttuple _ -> 16
  | Tshape _ -> 17
  | Tpu_access _ -> 18
  | Tvar _ -> 19
  | Tunion _ -> 20
  | Tintersection _ -> 21

let array_kind_con_ordinal ak =
  match ak with
  | AKvarray _ -> 1
  | AKdarray _ -> 3
  | AKvarray_or_darray _ -> 4

(* Compare two types syntactically, ignoring reason information and other
 * small differences that do not affect type inference behaviour. This
 * comparison function can be used to construct tree-based sets of types,
 * or to compare two types for "exact" equality.
 * Note that this function does *not* expand type variables, or type
 * aliases.
 * But if ty_compare ty1 ty2 = 0, then the types must not be distinguishable
 * by any typing rules.
 *
 * TODO(T52611361): Make this comparison exhaustive on ty1 to remove the _ catchall
 *)
let rec ty__compare ?(normalize_lists = false) ty_1 ty_2 =
  let rec ty__compare ty_1 ty_2 =
    match (ty_1, ty_2) with
    | (Tprim ty1, Tprim ty2) -> compare ty1 ty2
    | (Toption ty, Toption ty2) -> ty_compare ty ty2
    | (Tfun fty, Tfun fty2) -> tfun_compare fty fty2
    | (Tunion tyl1, Tunion tyl2)
    | (Tintersection tyl1, Tintersection tyl2)
    | (Ttuple tyl1, Ttuple tyl2) ->
      tyl_compare ~sort:normalize_lists ~normalize_lists tyl1 tyl2
    | (Tgeneric n1, Tgeneric n2) -> String.compare n1 n2
    | (Tnewtype (id, tyl, cstr1), Tnewtype (id2, tyl2, cstr2)) ->
      begin
        match String.compare id id2 with
        | 0 ->
          (match tyl_compare ~sort:false tyl tyl2 with
          | 0 -> ty_compare cstr1 cstr2
          | n -> n)
        | n -> n
      end
    | (Tdependent (d1, cstr1), Tdependent (d2, cstr2)) ->
      begin
        match compare d1 d2 with
        | 0 -> ty_compare cstr1 cstr2
        | n -> n
      end
    (* An instance of a class or interface, ty list are the arguments *)
    | (Tclass (id, exact, tyl), Tclass (id2, exact2, tyl2)) ->
      begin
        match String.compare (snd id) (snd id2) with
        | 0 ->
          begin
            match tyl_compare ~sort:false tyl tyl2 with
            | 0 -> compare_exact exact exact2
            | n -> n
          end
        | n -> n
      end
    | (Tarraykind ak1, Tarraykind ak2) -> array_kind_compare ak1 ak2
    | (Tshape (shape_kind1, fields1), Tshape (shape_kind2, fields2)) ->
      begin
        match compare_shape_kind shape_kind1 shape_kind2 with
        | 0 ->
          List.compare
            (fun (k1, v1) (k2, v2) ->
              match Ast_defs.ShapeField.compare k1 k2 with
              | 0 -> shape_field_type_compare v1 v2
              | n -> n)
            (Nast.ShapeMap.elements fields1)
            (Nast.ShapeMap.elements fields2)
        | n -> n
      end
    | (Tvar v1, Tvar v2) -> compare v1 v2
    | _ -> ty_con_ordinal ty_1 - ty_con_ordinal ty_2
  and shape_field_type_compare sft1 sft2 =
    match ty_compare sft1.sft_ty sft2.sft_ty with
    | 0 -> compare sft1.sft_optional sft2.sft_optional
    | n -> n
  and tfun_compare fty1 fty2 =
    match possibly_enforced_ty_compare fty1.ft_ret fty2.ft_ret with
    | 0 ->
      begin
        match ft_params_compare fty1.ft_params fty2.ft_params with
        | 0 ->
          compare
            (fty1.ft_arity, fty1.ft_reactive, fty1.ft_flags)
            (fty2.ft_arity, fty2.ft_reactive, fty2.ft_flags)
        | n -> n
      end
    | n -> n
  and array_kind_compare ak1 ak2 =
    match (ak1, ak2) with
    | (AKdarray (ty1, ty2), AKdarray (ty3, ty4))
    | (AKvarray_or_darray (ty1, ty2), AKvarray_or_darray (ty3, ty4)) ->
      tyl_compare ~sort:false [ty1; ty2] [ty3; ty4]
    | (AKvarray ty1, AKvarray ty2) -> ty_compare ty1 ty2
    | _ -> array_kind_con_ordinal ak1 - array_kind_con_ordinal ak2
  and ty_compare ty1 ty2 = ty__compare (get_node ty1) (get_node ty2) in
  ty__compare ty_1 ty_2

and ty_compare ?(normalize_lists = false) ty1 ty2 =
  ty__compare ~normalize_lists (get_node ty1) (get_node ty2)

and tyl_compare ~sort ?(normalize_lists = false) tyl1 tyl2 =
  let (tyl1, tyl2) =
    if sort then
      (List.sort ty_compare tyl1, List.sort ty_compare tyl2)
    else
      (tyl1, tyl2)
  in
  List.compare (ty_compare ~normalize_lists) tyl1 tyl2

and possibly_enforced_ty_compare ?(normalize_lists = false) ety1 ety2 =
  match ty_compare ~normalize_lists ety1.et_type ety2.et_type with
  | 0 -> Bool.compare ety1.et_enforced ety2.et_enforced
  | n -> n

and ft_params_compare ?(normalize_lists = false) params1 params2 =
  let rec ft_params_compare params1 params2 =
    List.compare ft_param_compare params1 params2
  and ft_param_compare param1 param2 =
    match
      possibly_enforced_ty_compare
        ~normalize_lists
        param1.fp_type
        param2.fp_type
    with
    | 0 ->
      compare
        (param1.fp_kind, param1.fp_accept_disposable, param1.fp_mutability)
        (param2.fp_kind, param2.fp_accept_disposable, param2.fp_mutability)
    | n -> n
  in
  ft_params_compare params1 params2

let tyl_equal tyl1 tyl2 = Int.equal 0 @@ tyl_compare ~sort:false tyl1 tyl2

let class_id_con_ordinal cid =
  match cid with
  | Aast.CIparent -> 0
  | Aast.CIself -> 1
  | Aast.CIstatic -> 2
  | Aast.CIexpr _ -> 3
  | Aast.CI _ -> 4

let class_id_compare cid1 cid2 =
  match (cid1, cid2) with
  | (Aast.CIexpr _e1, Aast.CIexpr _e2) -> 0
  | (Aast.CI (_, id1), Aast.CI (_, id2)) -> String.compare id1 id2
  | _ -> class_id_con_ordinal cid2 - class_id_con_ordinal cid1

let class_id_equal cid1 cid2 = Int.equal (class_id_compare cid1 cid2) 0

let has_member_compare ~normalize_lists hm1 hm2 =
  let ty_compare = ty_compare ~normalize_lists in
  let { hm_name = (_, m1); hm_type = ty1; hm_class_id = cid1 } = hm1 in
  let { hm_name = (_, m2); hm_type = ty2; hm_class_id = cid2 } = hm2 in
  match compare m1 m2 with
  | 0 ->
    (match ty_compare ty1 ty2 with
    | 0 -> class_id_compare cid1 cid2
    | comp -> comp)
  | comp -> comp

let destructure_compare ~normalize_lists d1 d2 =
  let {
    d_required = tyl1;
    d_optional = tyl_opt1;
    d_variadic = ty_opt1;
    d_kind = e1;
  } =
    d1
  in
  let {
    d_required = tyl2;
    d_optional = tyl_opt2;
    d_variadic = ty_opt2;
    d_kind = e2;
  } =
    d2
  in
  match tyl_compare ~normalize_lists ~sort:false tyl1 tyl2 with
  | 0 ->
    (match tyl_compare ~normalize_lists ~sort:false tyl_opt1 tyl_opt2 with
    | 0 ->
      (match Option.compare ty_compare ty_opt1 ty_opt2 with
      | 0 -> compare_destructure_kind e1 e2
      | comp -> comp)
    | comp -> comp)
  | comp -> comp

let constraint_ty_con_ordinal cty =
  match cty with
  | Thas_member _ -> 0
  | Tdestructure _ -> 1
  | TCunion _ -> 2
  | TCintersection _ -> 3

let rec constraint_ty_compare ?(normalize_lists = false) ty1 ty2 =
  let (_, ty1) = deref_constraint_type ty1 in
  let (_, ty2) = deref_constraint_type ty2 in
  match (ty1, ty2) with
  | (Thas_member hm1, Thas_member hm2) ->
    has_member_compare ~normalize_lists hm1 hm2
  | (Tdestructure d1, Tdestructure d2) ->
    destructure_compare ~normalize_lists d1 d2
  | (TCunion (lty1, cty1), TCunion (lty2, cty2))
  | (TCintersection (lty1, cty1), TCintersection (lty2, cty2)) ->
    let comp1 = ty_compare ~normalize_lists lty1 lty2 in
    if not @@ Int.equal comp1 0 then
      comp1
    else
      constraint_ty_compare ~normalize_lists cty1 cty2
  | (_, (Thas_member _ | Tdestructure _ | TCunion _ | TCintersection _)) ->
    constraint_ty_con_ordinal ty2 - constraint_ty_con_ordinal ty1

let constraint_ty_equal ?(normalize_lists = false) ty1 ty2 =
  Int.equal (constraint_ty_compare ~normalize_lists ty1 ty2) 0

let ty_equal ?(normalize_lists = false) ty1 ty2 =
  Int.equal 0 (ty_compare ~normalize_lists ty1 ty2)

let equal_internal_type ty1 ty2 =
  match (ty1, ty2) with
  | (LoclType ty1, LoclType ty2) -> ty_equal ~normalize_lists:true ty1 ty2
  | (ConstraintType ty1, ConstraintType ty2) ->
    constraint_ty_equal ~normalize_lists:true ty1 ty2
  | (_, (LoclType _ | ConstraintType _)) -> false

let equal_locl_ty ty1 ty2 = ty_equal ty1 ty2

let equal_locl_ty_ ty_1 ty_2 = Int.equal 0 (ty__compare ty_1 ty_2)

let equal_locl_fun_arity ft1 ft2 =
  match (ft1.ft_arity, ft2.ft_arity) with
  | (Fstandard min1, Fstandard min2) ->
    Int.equal min1 min2
    && Int.equal (List.length ft1.ft_params) (List.length ft2.ft_params)
  | (Fvariadic (min1, param1), Fvariadic (min2, param2)) ->
    Int.equal min1 min2 && Int.equal 0 (ft_params_compare [param1] [param2])
  | (Fellipsis (min1, pos1), Fellipsis (min2, pos2)) ->
    Int.equal min1 min2 && Pos.equal pos1 pos2
  | (Fstandard _, (Fvariadic _ | Fellipsis _))
  | (Fvariadic _, (Fstandard _ | Fellipsis _))
  | (Fellipsis _, (Fstandard _ | Fvariadic _)) ->
    false

let is_type_no_return = equal_locl_ty_ (Tprim Aast.Tnoreturn)

let make_function_type_rxvar param_ty =
  match deref param_ty with
  | (r, Tfun tfun) -> mk (r, Tfun { tfun with ft_reactive = RxVar None })
  | (r, Toption t) ->
    begin
      match deref t with
      | (r1, Tfun tfun) ->
        mk (r, Toption (mk (r1, Tfun { tfun with ft_reactive = RxVar None })))
      | _ -> param_ty
    end
  | _ -> param_ty

let rec equal_decl_ty_ ty_1 ty_2 =
  match (ty_1, ty_2) with
  | (Tany _, Tany _) -> true
  | (Terr, Terr) -> true
  | (Tthis, Tthis) -> true
  | (Tmixed, Tmixed) -> true
  | (Tnonnull, Tnonnull) -> true
  | (Tdynamic, Tdynamic) -> true
  | (Tapply (id1, tyl1), Tapply (id2, tyl2)) ->
    Aast.equal_sid id1 id2 && equal_decl_tyl tyl1 tyl2
  | (Tgeneric s1, Tgeneric s2) -> String.equal s1 s2
  | (Taccess (ty1, idl1), Taccess (ty2, idl2)) ->
    equal_decl_ty ty1 ty2 && List.equal ~equal:Aast.equal_sid idl1 idl2
  | (Tarray (tk1, tv1), Tarray (tk2, tv2)) ->
    Option.equal equal_decl_ty tk1 tk2 && Option.equal equal_decl_ty tv1 tv2
  | (Tdarray (tk1, tv1), Tdarray (tk2, tv2)) ->
    equal_decl_ty tk1 tk2 && equal_decl_ty tv1 tv2
  | (Tvarray ty1, Tvarray ty2) -> equal_decl_ty ty1 ty2
  | (Tvarray_or_darray (tk1, tv1), Tvarray_or_darray (tk2, tv2)) ->
    Option.equal equal_decl_ty tk1 tk2 && equal_decl_ty tv1 tv2
  | (Tlike ty1, Tlike ty2) -> equal_decl_ty ty1 ty2
  | (Tprim ty1, Tprim ty2) -> Aast.equal_tprim ty1 ty2
  | (Toption ty, Toption ty2) -> equal_decl_ty ty ty2
  | (Tfun fty1, Tfun fty2) -> equal_decl_fun_type fty1 fty2
  | (Tunion tyl1, Tunion tyl2)
  | (Tintersection tyl1, Tintersection tyl2)
  | (Ttuple tyl1, Ttuple tyl2) ->
    equal_decl_tyl tyl1 tyl2
  | (Tshape (shape_kind1, fields1), Tshape (shape_kind2, fields2)) ->
    equal_shape_kind shape_kind1 shape_kind2
    && List.equal
         ~equal:(fun (k1, v1) (k2, v2) ->
           Ast_defs.ShapeField.equal k1 k2 && equal_shape_field_type v1 v2)
         (Nast.ShapeMap.elements fields1)
         (Nast.ShapeMap.elements fields2)
  | (Tpu_access (ty1, id1, _), Tpu_access (ty2, id2, _)) ->
    equal_decl_ty ty1 ty2 && Aast.equal_sid id1 id2
  | (Tvar v1, Tvar v2) -> Ident.equal v1 v2
  | (Tany _, _)
  | (Terr, _)
  | (Tthis, _)
  | (Tapply _, _)
  | (Tgeneric _, _)
  | (Taccess _, _)
  | (Tarray _, _)
  | (Tdarray _, _)
  | (Tvarray _, _)
  | (Tvarray_or_darray _, _)
  | (Tmixed, _)
  | (Tlike _, _)
  | (Tnonnull, _)
  | (Tdynamic, _)
  | (Toption _, _)
  | (Tprim _, _)
  | (Tfun _, _)
  | (Ttuple _, _)
  | (Tshape _, _)
  | (Tpu_access _, _)
  | (Tvar _, _)
  | (Tunion _, _)
  | (Tintersection _, _) ->
    false

and equal_decl_ty ty1 ty2 = equal_decl_ty_ (get_node ty1) (get_node ty2)

and equal_shape_field_type sft1 sft2 =
  equal_decl_ty sft1.sft_ty sft2.sft_ty
  && Bool.equal sft1.sft_optional sft2.sft_optional

and equal_decl_fun_arity ft1 ft2 =
  match (ft1.ft_arity, ft2.ft_arity) with
  | (Fstandard min1, Fstandard min2) ->
    Int.equal min1 min2
    && Int.equal (List.length ft1.ft_params) (List.length ft2.ft_params)
  | (Fvariadic (min1, param1), Fvariadic (min2, param2)) ->
    Int.equal min1 min2 && equal_decl_ft_params [param1] [param2]
  | (Fellipsis (min1, pos1), Fellipsis (min2, pos2)) ->
    Int.equal min1 min2 && Pos.equal pos1 pos2
  | (Fstandard _, (Fvariadic _ | Fellipsis _))
  | (Fvariadic _, (Fstandard _ | Fellipsis _))
  | (Fellipsis _, (Fstandard _ | Fvariadic _)) ->
    false

and equal_decl_fun_type fty1 fty2 =
  equal_decl_possibly_enforced_ty fty1.ft_ret fty2.ft_ret
  && equal_decl_ft_params fty1.ft_params fty2.ft_params
  && equal_decl_fun_arity fty1 fty2
  && equal_reactivity fty1.ft_reactive fty2.ft_reactive
  && Int.equal fty1.ft_flags fty2.ft_flags

and equal_reactivity r1 r2 =
  match (r1, r2) with
  | (Nonreactive, Nonreactive) -> true
  | (Local ty1, Local ty2)
  | (Shallow ty1, Shallow ty2)
  | (Reactive ty1, Reactive ty2)
  | (Pure ty1, Pure ty2) ->
    Option.equal equal_decl_ty ty1 ty2
  | (MaybeReactive r1, MaybeReactive r2) -> equal_reactivity r1 r2
  | (RxVar r1, RxVar r2) -> Option.equal equal_reactivity r1 r2
  | _ -> false

and equal_param_rx_annotation pa1 pa2 =
  match (pa1, pa2) with
  | (Param_rx_var, Param_rx_var) -> true
  | (Param_rx_if_impl ty1, Param_rx_if_impl ty2) -> equal_decl_ty ty1 ty2
  | (Param_rx_var, Param_rx_if_impl _)
  | (Param_rx_if_impl _, Param_rx_var) ->
    false

and equal_decl_tyl tyl1 tyl2 = List.equal ~equal:equal_decl_ty tyl1 tyl2

and equal_decl_possibly_enforced_ty ety1 ety2 =
  equal_decl_ty ety1.et_type ety2.et_type
  && Bool.equal ety1.et_enforced ety2.et_enforced

and equal_decl_fun_param param1 param2 =
  equal_decl_possibly_enforced_ty param1.fp_type param2.fp_type
  && equal_param_mode param1.fp_kind param2.fp_kind
  && Bool.equal param1.fp_accept_disposable param2.fp_accept_disposable
  && Option.equal
       equal_param_mutability
       param1.fp_mutability
       param2.fp_mutability

and equal_decl_ft_params params1 params2 =
  List.equal ~equal:equal_decl_fun_param params1 params2

let equal_typeconst_abstract_kind ak1 ak2 =
  match (ak1, ak2) with
  | (TCAbstract ty1, TCAbstract ty2) -> Option.equal equal_decl_ty ty1 ty2
  | (TCPartiallyAbstract, TCPartiallyAbstract) -> true
  | (TCConcrete, TCConcrete) -> true
  | (TCAbstract _, _)
  | (TCPartiallyAbstract, _)
  | (TCConcrete, _) ->
    false

let equal_enum_type et1 et2 =
  equal_decl_ty et1.te_base et2.te_base
  && Option.equal equal_decl_ty et1.te_constraint et2.te_constraint

let equal_decl_where_constraint c1 c2 =
  let (tya1, ck1, tyb1) = c1 in
  let (tya2, ck2, tyb2) = c2 in
  equal_decl_ty tya1 tya2
  && Ast_defs.equal_constraint_kind ck1 ck2
  && equal_decl_ty tyb1 tyb2

let equal_decl_tparam tp1 tp2 =
  Ast_defs.equal_variance tp1.tp_variance tp2.tp_variance
  && Ast_defs.equal_id tp1.tp_name tp2.tp_name
  && List.equal
       ~equal:
         (Tuple.T2.equal ~eq1:Ast_defs.equal_constraint_kind ~eq2:equal_decl_ty)
       tp1.tp_constraints
       tp2.tp_constraints
  && Aast.equal_reify_kind tp1.tp_reified tp2.tp_reified
  && List.equal
       ~equal:Nast.equal_user_attribute
       tp1.tp_user_attributes
       tp2.tp_user_attributes

let equal_typedef_type tt1 tt2 =
  Pos.equal tt1.td_pos tt2.td_pos
  && Aast.equal_typedef_visibility tt1.td_vis tt2.td_vis
  && List.equal ~equal:equal_decl_tparam tt1.td_tparams tt2.td_tparams
  && Option.equal equal_decl_ty tt1.td_constraint tt2.td_constraint
  && equal_decl_ty tt1.td_type tt2.td_type

let equal_fun_elt fe1 fe2 =
  Option.equal String.equal fe1.fe_deprecated fe2.fe_deprecated
  && equal_decl_ty fe1.fe_type fe2.fe_type
  && Pos.equal fe1.fe_pos fe2.fe_pos

let get_ce_abstract ce = is_set ce_flags_abstract ce.ce_flags

let get_ce_final ce = is_set ce_flags_final ce.ce_flags

let get_ce_override ce = is_set ce_flags_override ce.ce_flags

let get_ce_lsb ce = is_set ce_flags_lsb ce.ce_flags

let get_ce_memoizelsb ce = is_set ce_flags_memoizelsb ce.ce_flags

let get_ce_synthesized ce = is_set ce_flags_synthesized ce.ce_flags

let get_ce_const ce = is_set ce_flags_const ce.ce_flags

let get_ce_lateinit ce = is_set ce_flags_lateinit ce.ce_flags

let get_ce_dynamicallycallable ce =
  is_set ce_flags_dynamicallycallable ce.ce_flags

let make_ce_flags
    ~xhp_attr
    ~abstract
    ~final
    ~override
    ~lsb
    ~memoizelsb
    ~synthesized
    ~const
    ~lateinit
    ~dynamicallycallable =
  let flags = 0 in
  let flags = set_bit ce_flags_abstract abstract flags in
  let flags = set_bit ce_flags_final final flags in
  let flags = set_bit ce_flags_override override flags in
  let flags = set_bit ce_flags_lsb lsb flags in
  let flags = set_bit ce_flags_memoizelsb memoizelsb flags in
  let flags = set_bit ce_flags_synthesized synthesized flags in
  let flags = set_bit ce_flags_const const flags in
  let flags = set_bit ce_flags_lateinit lateinit flags in
  let flags = set_bit ce_flags_dynamicallycallable dynamicallycallable flags in
  let flags = Int.bit_or flags (xhp_attr_to_ce_flags xhp_attr) in
  flags
