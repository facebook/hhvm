(*
 * Copyrighd (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs_flags
include Typing_defs_core

(** Origin of Class Constant References:
    In order to be able to detect cycle definitions like
    class C {
      const int A = D::A;
    }
    class D {
      const int A = C::A;
    }
    we need to remember which constants were used during initialization.

    Currently the syntax of constants allows direct references to another class
    like D::A, or self references using self::A.

    class_const_from encodes the origin (class vs self).
 *)
type class_const_from =
  | Self
  | From of string
[@@deriving eq, show]

(** Class Constant References:
    In order to be able to detect cycle definitions like
    class C {
      const int A = D::A;
    }
    class D {
      const int A = C::A;
    }
    we need to remember which constants were used during initialization.

    Currently the syntax of constants allows direct references to another class
    like D::A, or self references using self::A.
 *)
type class_const_ref = class_const_from * string [@@deriving eq, show]

module CCR = struct
  type t = class_const_ref [@@deriving show]

  (* We're deriving the compare function by hand to sync it with the rust code.
   * In the decl parser I need to sort these class_const_from in the same
   * way. I used `cargo expand` to see how Cargo generated they Ord/PartialOrd
   * functions.
   *)
  let compare_class_const_from ccf0 ccf1 =
    match (ccf0, ccf1) with
    | (Self, Self) -> 0
    | (From lhs, From rhs) -> String.compare lhs rhs
    | (Self, From _) -> -1
    | (From _, Self) -> 1

  let compare (ccf0, s0) (ccf1, s1) =
    match compare_class_const_from ccf0 ccf1 with
    | 0 -> String.compare s0 s1
    | x -> x
end

module CCRSet = struct
  include Caml.Set.Make (CCR)

  type t_as_list = CCR.t list [@@deriving show]

  let pp fmt set = pp_t_as_list fmt (elements set)

  let show set = show_t_as_list (elements set)
end

type const_decl = {
  cd_pos: Pos_or_decl.t;
  cd_type: decl_ty;
}
[@@deriving show]

type class_elt = {
  ce_visibility: ce_visibility;
  ce_type: decl_ty Lazy.t;
  ce_origin: string;  (** identifies the class from which this elt originates *)
  ce_deprecated: string option;
  ce_pos: Pos_or_decl.t Lazy.t;  (** pos of the type of the elt *)
  ce_flags: Typing_defs_flags.ClassElt.t;
}
[@@deriving show]

type fun_elt = {
  fe_deprecated: string option;
  fe_module: Ast_defs.id option;
  fe_internal: bool;  (** Top-level functions have limited visibilities *)
  fe_type: decl_ty;
  fe_pos: Pos_or_decl.t;
  fe_php_std_lib: bool;
  fe_support_dynamic_type: bool;
  fe_no_auto_dynamic: bool;
}
[@@deriving show]

(* TODO(T71787342) This is temporary. It is necessary because legacy decl
 * uses Typing_defs types to represent folded inherited members. This will
 * be removed when shallow decl ships as this information is only necessary
 * there. *)
type class_const_kind =
  | CCAbstract of bool (* has default *)
  | CCConcrete
[@@deriving eq, show]

type class_const = {
  cc_synthesized: bool;
  cc_abstract: class_const_kind;
  cc_pos: Pos_or_decl.t;
  cc_type: decl_ty;
  cc_origin: string;
      (** identifies the class from which this const originates *)
  cc_refs: class_const_ref list;
      (** references to the constants used in the initializer *)
}
[@@deriving show]

type module_reference =
  | MRGlobal
  | MRPrefix of string
  | MRExact of string
[@@deriving show]

type module_def_type = {
  mdt_pos: Pos_or_decl.t;
  mdt_exports: module_reference list option;
  mdt_imports: module_reference list option;
}
[@@deriving show]

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
type requirement = Pos_or_decl.t * decl_ty

and abstract_typeconst = {
  atc_as_constraint: decl_ty option;
  atc_super_constraint: decl_ty option;
  atc_default: decl_ty option;
}

and concrete_typeconst = { tc_type: decl_ty }

and partially_abstract_typeconst = {
  patc_constraint: decl_ty;
  patc_type: decl_ty;
}

and typeconst =
  | TCAbstract of abstract_typeconst
  | TCConcrete of concrete_typeconst

and typeconst_type = {
  ttc_synthesized: bool;
  ttc_name: pos_id;
  ttc_kind: typeconst;
  ttc_origin: string;
  ttc_enforceable: Pos_or_decl.t * bool;
      (** If the typeconst had the <<__Enforceable>> attribute on its
          declaration, this will be [(position_of_declaration, true)].

          In legacy decl, the second element of the tuple will also be true if
          the typeconst overrides some parent typeconst which had the
          <<__Enforceable>> attribute. In that case, the position will point to
          the declaration of the parent typeconst.

          In shallow decl, this is not the case--there is no overriding behavior
          modeled here, and the second element will only be true when the
          declaration of this typeconst had the attribute.

          When the second element of the tuple is false, the position will be
          [Pos_or_decl.none].

          To manage the difference between legacy and shallow decl, use
          [Typing_classes_heap.Api.get_typeconst_enforceability] rather than
          accessing this field directly. *)
  ttc_reifiable: Pos_or_decl.t option;
  ttc_concretized: bool;
  ttc_is_ctx: bool;
}

and enum_type = {
  te_base: decl_ty;
  te_constraint: decl_ty option;
  te_includes: decl_ty list;
}
[@@deriving show]

type typedef_type = {
  td_module: Ast_defs.id option;
  td_pos: Pos_or_decl.t;
  td_vis: Ast_defs.typedef_visibility;
  td_tparams: decl_tparam list;
  td_as_constraint: decl_ty option;
  td_super_constraint: decl_ty option;
  td_type: decl_ty;
  td_is_ctx: bool;
  td_attributes: user_attribute list;
  td_internal: bool;
  td_docs_url: string option;
}
[@@deriving show]

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
[@@deriving show]

module Type_expansions : sig
  (** A list of the type defs and type access we have expanded thus far. Used
      to prevent entering into a cycle when expanding these types. *)
  type t

  val empty : t

  (** If we are expanding the RHS of a type definition, [report_cycle] contains
      the position and id of the LHS. This way, if the RHS expands at some point
      to the LHS id, we are able to report a cycle. *)
  val empty_w_cycle_report : report_cycle:(Pos.t * string) option -> t

  (** Returns:
    - [None] if there was no cycle
    - [Some None] if there was a cycle which did not involve the first
      type expansion, i.e. error reporting should be done elsewhere
    - [Some (Some pos)] if there was a cycle involving the first type
      expansion in which case an error should be reported at [pos]. *)
  val add_and_check_cycles :
    t -> Pos_or_decl.t * string -> t * Pos.t option option

  val ids : t -> string list

  val positions : t -> Pos_or_decl.t list
end = struct
  type t = {
    report_cycle: (Pos.t * string) option;
        (** If we are expanding the RHS of a type definition, [report_cycle] contains
            the position and id of the LHS. This way, if the RHS expands at some point
            to the LHS id, we are able to report a cycle. *)
    expansions: (Pos_or_decl.t * string) list;
        (** A list of the type defs and type access we have expanded thus far. Used
            to prevent entering into a cycle when expanding these types. *)
  }

  let empty_w_cycle_report ~report_cycle = { report_cycle; expansions = [] }

  let empty = empty_w_cycle_report ~report_cycle:None

  let add { report_cycle; expansions } exp =
    { report_cycle; expansions = exp :: expansions }

  let has_expanded { report_cycle; expansions } x =
    match report_cycle with
    | Some (p, x') when String.equal x x' -> Some (Some p)
    | Some _
    | None ->
      List.find_map expansions ~f:(function
          | (_, x') when String.equal x x' -> Some None
          | _ -> None)

  let add_and_check_cycles exps (p, id) =
    let has_cycle = has_expanded exps id in
    let exps = add exps (p, id) in
    (exps, has_cycle)

  let as_list { report_cycle; expansions } =
    (report_cycle
    |> Option.map ~f:(Tuple2.map_fst ~f:Pos_or_decl.of_raw_pos)
    |> Option.to_list)
    @ List.rev expansions

  let ids exps = as_list exps |> List.map ~f:snd

  let positions exps = as_list exps |> List.map ~f:fst
end

(** Tracks information about how a type was expanded *)
type expand_env = {
  type_expansions: Type_expansions.t;
  expand_visible_newtype: bool;
      (** Allow to expand visible `newtype`, i.e. opaque types defined in the current file.
          True by default. *)
  substs: locl_ty SMap.t;
  this_ty: locl_ty;
      (** The type that is substituted for `this` in signatures. It should be
       * set to an expression dependent type if appropriate
       *)
  on_error: Typing_error.Reasons_callback.t option;
}

let empty_expand_env =
  {
    type_expansions = Type_expansions.empty;
    expand_visible_newtype = true;
    substs = SMap.empty;
    this_ty =
      mk (Reason.none, Tgeneric (Naming_special_names.Typehints.this, []));
    on_error = None;
  }

let empty_expand_env_with_on_error on_error =
  { empty_expand_env with on_error = Some on_error }

let add_type_expansion_check_cycles env exp =
  let (type_expansions, has_cycle) =
    Type_expansions.add_and_check_cycles env.type_expansions exp
  in
  let env = { env with type_expansions } in
  (env, has_cycle)

let get_var t =
  match get_node t with
  | Tvar v -> Some v
  | _ -> None

let get_class_type t =
  match get_node t with
  | Tclass (id, exact, tyl) -> Some (id, exact, tyl)
  | _ -> None

let get_var_i t =
  match t with
  | LoclType t -> get_var t
  | ConstraintType _ -> None

let is_tyvar t = Option.is_some (get_var t)

let is_tyvar_i t = Option.is_some (get_var_i t)

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
  (* TODO(T69551141) handle type arguments *)
  match get_node t with
  | Tgeneric (n', _tyargs) when String.equal n n' -> true
  | _ -> false

let is_prim p t =
  match get_node t with
  | Tprim p' when Aast.equal_tprim p p' -> true
  | _ -> false

let is_union t =
  match get_node t with
  | Tunion _ -> true
  | _ -> false

let is_neg t =
  match get_node t with
  | Tneg _ -> true
  | _ -> false

let is_constraint_type_union t =
  match deref_constraint_type t with
  | (_, TCunion _) -> true
  | _ -> false

let is_has_member t =
  match deref_constraint_type t with
  | (_, Thas_member _) -> true
  | _ -> false

let show_phase_ty _ = "<phase_ty>"

let pp_phase_ty _ _ = Printf.printf "%s\n" "<phase_ty>"

let is_locl_type = function
  | LoclType _ -> true
  | _ -> false

let reason = function
  | LoclType t -> get_reason t
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
  | Tnonnull
  | Tneg _
  | Tdynamic
  | Tany _
  | Tprim _
  | Tfun _
  | Ttuple _
  | Tshape _
  | Tvar _
  | Tnewtype _
  | Tdependent _
  | Tgeneric _
  | Tclass _
  | Tunapplied_alias _
  | Tvec_or_dict _
  | Taccess _ ->
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

let arity_min ft : int =
  let a = List.count ~f:(fun fp -> not (get_fp_has_default fp)) ft.ft_params in
  if get_ft_variadic ft then
    a - 1
  else
    a

let get_param_mode callconv =
  match callconv with
  | Ast_defs.Pinout _ -> FPinout
  | Ast_defs.Pnormal -> FPnormal

module DependentKind = struct
  let to_string = function
    | DTexpr i ->
      let display_id = Reason.get_expr_display_id i in
      "<expr#" ^ string_of_int display_id ^ ">"

  let is_generic_dep_ty s =
    String.is_substring ~substring:"::" s
    || String.equal s Naming_special_names.Typehints.this
end

let rec is_denotable ty =
  match get_node ty with
  | Tunion [] (* Possible encodings of nothing *)
  | Tintersection [] (* Possible encodings of mixed *)
  | Tnonnull
  | Tdynamic
  | Tprim _
  | Tnewtype _ ->
    true
  | Tunion [ty; ty'] -> begin
    match (get_node ty, get_node ty') with
    | (Tprim Aast.(Tfloat | Tstring), Tprim Aast.Tint)
    | (Tprim Aast.Tint, Tprim Aast.(Tfloat | Tstring)) ->
      true
    | _ -> false
  end
  | Tclass (_, _, ts)
  | Ttuple ts ->
    List.for_all ~f:is_denotable ts
  | Tvec_or_dict (tk, tv) -> is_denotable tk && is_denotable tv
  | Taccess (ty, _) -> is_denotable ty
  | Tshape (_, _, sm) ->
    TShapeMap.for_all (fun _ { sft_ty; _ } -> is_denotable sft_ty) sm
  | Tfun { ft_params; ft_ret; _ } ->
    is_denotable ft_ret.et_type
    && List.for_all ft_params ~f:(fun { fp_type; _ } ->
           is_denotable fp_type.et_type)
  | Toption ty -> is_denotable ty
  | Tgeneric (nm, _) ->
    not
      (DependentKind.is_generic_dep_ty nm
      || String.is_substring ~substring:"#" nm)
  | Tunion _
  | Tintersection _
  | Tneg _
  | Tany _
  | Tvar _
  | Tdependent _
  | Tunapplied_alias _ ->
    false

let same_type_origin orig1 orig2 =
  match orig1 with
  | Missing_origin -> false
  | _ -> equal_type_origin orig1 orig2

module ShapeFieldMap = struct
  include TShapeMap

  let map_and_rekey shape_map key_f value_f =
    let f_over_shape_field_type ({ sft_ty; _ } as shape_field_type) =
      { shape_field_type with sft_ty = value_f sft_ty }
    in
    TShapeMap.map_and_rekey shape_map key_f f_over_shape_field_type

  let map_env f env shape_map =
    let f_over_shape_field_type env _key ({ sft_ty; _ } as shape_field_type) =
      let (env, sft_ty) = f env sft_ty in
      (env, { shape_field_type with sft_ty })
    in
    TShapeMap.map_env f_over_shape_field_type env shape_map

  let map_env_ty_err_opt f env shape_map ~combine_ty_errs =
    let f_over_shape_field_type env _key ({ sft_ty; _ } as shape_field_type) =
      let (env, sft_ty) = f env sft_ty in
      (env, { shape_field_type with sft_ty })
    in
    TShapeMap.map_env_ty_err_opt
      f_over_shape_field_type
      env
      shape_map
      ~combine_ty_errs

  let map f shape_map = map_and_rekey shape_map (fun x -> x) f

  let iter f shape_map =
    let f_over_shape_field_type shape_map_key { sft_ty; _ } =
      f shape_map_key sft_ty
    in
    TShapeMap.iter f_over_shape_field_type shape_map

  let iter_values f = iter (fun _ -> f)
end

module ShapeFieldList = struct
  include Common.List

  let map_env env xs ~f =
    let f_over_shape_field_type env ({ sft_ty; _ } as shape_field_type) =
      let (env, sft_ty) = f env sft_ty in
      (env, { shape_field_type with sft_ty })
    in
    Common.List.map_env env xs ~f:f_over_shape_field_type
end

(*****************************************************************************)
(* Suggest mode *)
(*****************************************************************************)

(* Set to true when we are trying to infer the missing type hints. *)
let is_suggest_mode = ref false

(* Ordinal value for type constructor *)
let ty_con_ordinal_ : type a. a ty_ -> int = function
  (* only decl constructors *)
  | Tthis -> 100
  | Tapply _ -> 101
  | Tmixed -> 102
  | Tlike _ -> 103
  | Trefinement _ -> 104
  (* exist in both phases *)
  | Tany _ -> 0
  | Toption t -> begin
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
  | Tgeneric _ -> 11
  | Tunion _ -> 13
  | Tintersection _ -> 14
  | Taccess _ -> 24
  | Tvec_or_dict _ -> 25
  (* only locl constructors *)
  | Tunapplied_alias _ -> 200
  | Tnewtype _ -> 201
  | Tdependent _ -> 202
  | Tclass _ -> 204
  | Tneg _ -> 205

let compare_neg_type neg1 neg2 =
  match (neg1, neg2) with
  | (Neg_prim tp1, Neg_prim tp2) -> Aast.compare_tprim tp1 tp2
  | (Neg_class c1, Neg_class c2) -> String.compare (snd c1) (snd c2)
  | (Neg_prim _, Neg_class _) -> -1
  | (Neg_class _, Neg_prim _) -> 1

(* Compare two types syntactically, ignoring reason information and other
 * small differences that do not affect type inference behaviour. This
 * comparison function can be used to construct tree-based sets of types,
 * or to compare two types for "exact" equality.
 * Note that this function does *not* expand type variables, or type
 * aliases.
 * But if ty_compare ty1 ty2 = 0, then the types must not be distinguishable
 * by any typing rules.
 *)
let rec ty__compare : type a. ?normalize_lists:bool -> a ty_ -> a ty_ -> int =
 fun ?(normalize_lists = false) ty_1 ty_2 ->
  let rec ty__compare : type a. a ty_ -> a ty_ -> int =
   fun ty_1 ty_2 ->
    match (ty_1, ty_2) with
    (* Only in Declared Phase *)
    | (Tthis, Tthis) -> 0
    | (Tapply (id1, tyl1), Tapply (id2, tyl2)) -> begin
      match String.compare (snd id1) (snd id2) with
      | 0 -> tyl_compare ~sort:normalize_lists ~normalize_lists tyl1 tyl2
      | n -> n
    end
    | (Trefinement (ty1, r1), Trefinement (ty2, r2)) -> begin
      match ty_compare ty1 ty2 with
      | 0 -> class_refinement_compare r1 r2
      | n -> n
    end
    | (Tmixed, Tmixed) -> 0
    | (Tlike ty1, Tlike ty2) -> ty_compare ty1 ty2
    | ((Tthis | Tapply _ | Tmixed | Tlike _), _)
    | (_, (Tthis | Tapply _ | Tmixed | Tlike _)) ->
      ty_con_ordinal_ ty_1 - ty_con_ordinal_ ty_2
    (* Both or in Localized Phase *)
    | (Tprim ty1, Tprim ty2) -> Aast_defs.compare_tprim ty1 ty2
    | (Toption ty, Toption ty2) -> ty_compare ty ty2
    | (Tvec_or_dict (tk, tv), Tvec_or_dict (tk2, tv2)) -> begin
      match ty_compare tk tk2 with
      | 0 -> ty_compare tv tv2
      | n -> n
    end
    | (Tfun fty, Tfun fty2) -> tfun_compare fty fty2
    | (Tunion tyl1, Tunion tyl2)
    | (Tintersection tyl1, Tintersection tyl2)
    | (Ttuple tyl1, Ttuple tyl2) ->
      tyl_compare ~sort:normalize_lists ~normalize_lists tyl1 tyl2
    | (Tgeneric (n1, args1), Tgeneric (n2, args2)) -> begin
      match String.compare n1 n2 with
      | 0 -> tyl_compare ~sort:false ~normalize_lists args1 args2
      | n -> n
    end
    | (Tnewtype (id, tyl, cstr1), Tnewtype (id2, tyl2, cstr2)) -> begin
      match String.compare id id2 with
      | 0 ->
        (match tyl_compare ~sort:false tyl tyl2 with
        | 0 -> ty_compare cstr1 cstr2
        | n -> n)
      | n -> n
    end
    | (Tdependent (d1, cstr1), Tdependent (d2, cstr2)) -> begin
      match compare_dependent_type d1 d2 with
      | 0 -> ty_compare cstr1 cstr2
      | n -> n
    end
    (* An instance of a class or interface, ty list are the arguments *)
    | (Tclass (id, exact, tyl), Tclass (id2, exact2, tyl2)) -> begin
      match String.compare (snd id) (snd id2) with
      | 0 -> begin
        match tyl_compare ~sort:false tyl tyl2 with
        | 0 -> exact_compare exact exact2
        | n -> n
      end
      | n -> n
    end
    | ( Tshape (shape_origin1, shape_kind1, fields1),
        Tshape (shape_origin2, shape_kind2, fields2) ) ->
      if same_type_origin shape_origin1 shape_origin2 then
        0
      else begin
        match compare_shape_kind shape_kind1 shape_kind2 with
        | 0 ->
          List.compare
            (fun (k1, v1) (k2, v2) ->
              match TShapeField.compare k1 k2 with
              | 0 -> shape_field_type_compare v1 v2
              | n -> n)
            (TShapeMap.elements fields1)
            (TShapeMap.elements fields2)
        | n -> n
      end
    | (Tvar v1, Tvar v2) -> compare v1 v2
    | (Tunapplied_alias n1, Tunapplied_alias n2) -> String.compare n1 n2
    | (Taccess (ty1, id1), Taccess (ty2, id2)) -> begin
      match ty_compare ty1 ty2 with
      | 0 -> String.compare (snd id1) (snd id2)
      | n -> n
    end
    | (Tneg neg1, Tneg neg2) -> compare_neg_type neg1 neg2
    | (Tnonnull, Tnonnull) -> 0
    | (Tdynamic, Tdynamic) -> 0
    | ( ( Tprim _ | Toption _ | Tvec_or_dict _ | Tfun _ | Tintersection _
        | Tunion _ | Ttuple _ | Tgeneric _ | Tnewtype _ | Tdependent _
        | Tclass _ | Tshape _ | Tvar _ | Tunapplied_alias _ | Tnonnull
        | Tdynamic | Taccess _ | Tany _ | Tneg _ | Trefinement _ ),
        _ ) ->
      ty_con_ordinal_ ty_1 - ty_con_ordinal_ ty_2
  and shape_field_type_compare :
      type a. a shape_field_type -> a shape_field_type -> int =
   fun sft1 sft2 ->
    let { sft_ty = ty1; sft_optional = optional1 } = sft1 in
    let { sft_ty = ty2; sft_optional = optional2 } = sft2 in
    match ty_compare ty1 ty2 with
    | 0 -> Bool.compare optional1 optional2
    | n -> n
  and user_attribute_compare ua1 ua2 =
    let { ua_name = name1; ua_classname_params = params1 } = ua1 in
    let { ua_name = name2; ua_classname_params = params2 } = ua2 in
    match String.compare (snd name1) (snd name2) with
    | 0 -> List.compare String.compare params1 params2
    | n -> n
  and user_attributes_compare ual1 ual2 =
    List.compare user_attribute_compare ual1 ual2
  and tparam_compare : type a. a ty tparam -> a ty tparam -> int =
   fun tp1 tp2 ->
    let {
      (* Type parameters on functions are always marked invariant *)
      tp_variance = _;
      tp_name = name1;
      tp_tparams = tparams1;
      tp_constraints = constraints1;
      tp_reified = reified1;
      tp_user_attributes = user_attributes1;
    } =
      tp1
    in
    let {
      tp_variance = _;
      tp_name = name2;
      tp_tparams = tparams2;
      tp_constraints = constraints2;
      tp_reified = reified2;
      tp_user_attributes = user_attributes2;
    } =
      tp2
    in
    match String.compare (snd name1) (snd name2) with
    | 0 -> begin
      match tparams_compare tparams1 tparams2 with
      | 0 -> begin
        match constraints_compare constraints1 constraints2 with
        | 0 -> begin
          match user_attributes_compare user_attributes1 user_attributes2 with
          | 0 -> Aast_defs.compare_reify_kind reified1 reified2
          | n -> n
        end
        | n -> n
      end
      | n -> n
    end
    | n -> n
  and tparams_compare : type a. a ty tparam list -> a ty tparam list -> int =
   (fun tpl1 tpl2 -> List.compare tparam_compare tpl1 tpl2)
  and constraints_compare :
      type a.
      (Ast_defs.constraint_kind * a ty) list ->
      (Ast_defs.constraint_kind * a ty) list ->
      int =
   (fun cl1 cl2 -> List.compare constraint_compare cl1 cl2)
  and constraint_compare :
      type a.
      Ast_defs.constraint_kind * a ty -> Ast_defs.constraint_kind * a ty -> int
      =
   fun (ck1, ty1) (ck2, ty2) ->
    match Ast_defs.compare_constraint_kind ck1 ck2 with
    | 0 -> ty_compare ty1 ty2
    | n -> n
  and where_constraint_compare :
      type a b.
      a ty * Ast_defs.constraint_kind * b ty ->
      a ty * Ast_defs.constraint_kind * b ty ->
      int =
   fun (ty1a, ck1, ty1b) (ty2a, ck2, ty2b) ->
    match Ast_defs.compare_constraint_kind ck1 ck2 with
    | 0 -> begin
      match ty_compare ty1a ty2a with
      | 0 -> ty_compare ty1b ty2b
      | n -> n
    end
    | n -> n
  and where_constraints_compare :
      type a b.
      (a ty * Ast_defs.constraint_kind * b ty) list ->
      (a ty * Ast_defs.constraint_kind * b ty) list ->
      int =
   (fun cl1 cl2 -> List.compare where_constraint_compare cl1 cl2)
  (* We match every field rather than using field selection syntax. This guards against future additions to function type elements *)
  and tfun_compare : type a. a ty fun_type -> a ty fun_type -> int =
   fun fty1 fty2 ->
    let {
      ft_ret = ret1;
      ft_params = params1;
      ft_flags = flags1;
      ft_implicit_params = implicit_params1;
      ft_ifc_decl = ifc_decl1;
      ft_tparams = tparams1;
      ft_where_constraints = where_constraints1;
    } =
      fty1
    in
    let {
      ft_ret = ret2;
      ft_params = params2;
      ft_flags = flags2;
      ft_implicit_params = implicit_params2;
      ft_ifc_decl = ifc_decl2;
      ft_tparams = tparams2;
      ft_where_constraints = where_constraints2;
    } =
      fty2
    in
    match possibly_enforced_ty_compare ret1 ret2 with
    | 0 -> begin
      match ft_params_compare params1 params2 with
      | 0 -> begin
        match tparams_compare tparams1 tparams2 with
        | 0 -> begin
          match
            where_constraints_compare where_constraints1 where_constraints2
          with
          | 0 -> begin
            match Int.compare flags1 flags2 with
            | 0 ->
              let { capability = capability1 } = implicit_params1 in
              let { capability = capability2 } = implicit_params2 in
              begin
                match capability_compare capability1 capability2 with
                | 0 -> compare_ifc_fun_decl ifc_decl1 ifc_decl2
                | n -> n
              end
            | n -> n
          end
          | n -> n
        end
        | n -> n
      end
      | n -> n
    end
    | n -> n
  and capability_compare : type a. a ty capability -> a ty capability -> int =
   fun cap1 cap2 ->
    match (cap1, cap2) with
    | (CapDefaults _, CapDefaults _) -> 0
    | (CapDefaults _, CapTy _) -> -1
    | (CapTy _, CapDefaults _) -> 1
    | (CapTy ty1, CapTy ty2) -> ty_compare ty1 ty2
  and ty_compare : type a. a ty -> a ty -> int =
   (fun ty1 ty2 -> ty__compare (get_node ty1) (get_node ty2))
  in
  ty__compare ty_1 ty_2

and ty_compare : type a. ?normalize_lists:bool -> a ty -> a ty -> int =
 fun ?(normalize_lists = false) ty1 ty2 ->
  ty__compare ~normalize_lists (get_node ty1) (get_node ty2)

and tyl_compare :
    type a. sort:bool -> ?normalize_lists:bool -> a ty list -> a ty list -> int
    =
 fun ~sort ?(normalize_lists = false) tyl1 tyl2 ->
  let (tyl1, tyl2) =
    if sort then
      (List.sort ~compare:ty_compare tyl1, List.sort ~compare:ty_compare tyl2)
    else
      (tyl1, tyl2)
  in
  List.compare (ty_compare ~normalize_lists) tyl1 tyl2

and possibly_enforced_ty_compare :
    type a.
    ?normalize_lists:bool ->
    a ty possibly_enforced_ty ->
    a ty possibly_enforced_ty ->
    int =
 fun ?(normalize_lists = false) ety1 ety2 ->
  match ty_compare ~normalize_lists ety1.et_type ety2.et_type with
  | 0 -> compare_enforcement ety1.et_enforced ety2.et_enforced
  | n -> n

and ft_param_compare :
    type a. ?normalize_lists:bool -> a ty fun_param -> a ty fun_param -> int =
 fun ?(normalize_lists = false) param1 param2 ->
  match
    possibly_enforced_ty_compare ~normalize_lists param1.fp_type param2.fp_type
  with
  | 0 -> Int.compare param1.fp_flags param2.fp_flags
  | n -> n

and ft_params_compare :
    type a.
    ?normalize_lists:bool -> a ty fun_param list -> a ty fun_param list -> int =
 fun ?(normalize_lists = false) params1 params2 ->
  List.compare (ft_param_compare ~normalize_lists) params1 params2

and refined_const_compare : type a. a refined_const -> a refined_const -> int =
 fun a b ->
  (* Note: `rc_is_ctx` is not used for typing inference, so we can safely ignore it *)
  match (a.rc_bound, b.rc_bound) with
  | (TRexact _, TRloose _) -> -1
  | (TRloose _, TRexact _) -> 1
  | (TRloose b1, TRloose b2) -> begin
    match tyl_compare ~sort:true b1.tr_lower b2.tr_lower with
    | 0 -> tyl_compare ~sort:true b1.tr_upper b2.tr_upper
    | n -> n
  end
  | (TRexact ty1, TRexact ty2) -> ty_compare ty1 ty2

and class_refinement_compare :
    type a. a class_refinement -> a class_refinement -> int =
 fun { cr_consts = rcs1 } { cr_consts = rcs2 } ->
  SMap.compare refined_const_compare rcs1 rcs2

and exact_compare e1 e2 =
  match (e1, e2) with
  | (Exact, Exact) -> 0
  | (Nonexact _, Exact) -> 1
  | (Exact, Nonexact _) -> -1
  | (Nonexact r1, Nonexact r2) -> class_refinement_compare r1 r2

(* Dedicated functions with more easily discoverable names *)
let compare_locl_ty : ?normalize_lists:bool -> locl_ty -> locl_ty -> int =
  ty_compare

let compare_decl_ty : ?normalize_lists:bool -> decl_ty -> decl_ty -> int =
  ty_compare

let tyl_equal tyl1 tyl2 = Int.equal 0 @@ tyl_compare ~sort:false tyl1 tyl2

let compare_exact = exact_compare

let equal_exact e1 e2 = Int.equal 0 (compare_exact e1 e2)

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
  let {
    hm_name = (_, m1);
    hm_type = ty1;
    hm_class_id = cid1;
    hm_explicit_targs = targs1;
  } =
    hm1
  in
  let {
    hm_name = (_, m2);
    hm_type = ty2;
    hm_class_id = cid2;
    hm_explicit_targs = targs2;
  } =
    hm2
  in
  let targ_compare (_, (_, hint1)) (_, (_, hint2)) =
    Aast_defs.compare_hint_ hint1 hint2
  in
  match String.compare m1 m2 with
  | 0 ->
    (match ty_compare ty1 ty2 with
    | 0 ->
      (match class_id_compare cid1 cid2 with
      | 0 -> Option.compare (List.compare targ_compare) targs1 targs2
      | comp -> comp)
    | comp -> comp)
  | comp -> comp

let can_index_compare ~normalize_lists ci1 ci2 =
  match ty_compare ~normalize_lists ci1.ci_key ci2.ci_key with
  | 0 ->
    (match ty_compare ~normalize_lists ci1.ci_val ci2.ci_val with
    | 0 -> Option.compare compare_tshape_field_name ci1.ci_shape ci2.ci_shape
    | comp -> comp)
  | comp -> comp

let can_traverse_compare ~normalize_lists ct1 ct2 =
  match Option.compare (ty_compare ~normalize_lists) ct1.ct_key ct2.ct_key with
  | 0 ->
    (match ty_compare ~normalize_lists ct1.ct_val ct2.ct_val with
    | 0 -> Bool.compare ct1.ct_is_await ct2.ct_is_await
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
  | Tcan_index _ -> 4
  | Tcan_traverse _ -> 5
  | Thas_type_member _ -> 6

let rec constraint_ty_compare ?(normalize_lists = false) ty1 ty2 =
  let (_, ty1) = deref_constraint_type ty1 in
  let (_, ty2) = deref_constraint_type ty2 in
  match (ty1, ty2) with
  | (Thas_member hm1, Thas_member hm2) ->
    has_member_compare ~normalize_lists hm1 hm2
  | (Thas_type_member htm1, Thas_type_member htm2) ->
    let { htm_id = id1; htm_lower = lower1; htm_upper = upper1 } = htm1
    and { htm_id = id2; htm_lower = lower2; htm_upper = upper2 } = htm2 in
    (match String.compare id1 id2 with
    | 0 ->
      (match ty_compare lower1 lower2 with
      | 0 -> ty_compare upper1 upper2
      | comp -> comp)
    | comp -> comp)
  | (Tcan_index ci1, Tcan_index ci2) ->
    can_index_compare ~normalize_lists ci1 ci2
  | (Tcan_traverse ct1, Tcan_traverse ct2) ->
    can_traverse_compare ~normalize_lists ct1 ct2
  | (Tdestructure d1, Tdestructure d2) ->
    destructure_compare ~normalize_lists d1 d2
  | (TCunion (lty1, cty1), TCunion (lty2, cty2))
  | (TCintersection (lty1, cty1), TCintersection (lty2, cty2)) ->
    let comp1 = ty_compare ~normalize_lists lty1 lty2 in
    if not @@ Int.equal comp1 0 then
      comp1
    else
      constraint_ty_compare ~normalize_lists cty1 cty2
  | ( _,
      ( Thas_member _ | Tcan_index _ | Tcan_traverse _ | Tdestructure _
      | TCunion _ | TCintersection _ | Thas_type_member _ ) ) ->
    constraint_ty_con_ordinal ty2 - constraint_ty_con_ordinal ty1

let constraint_ty_equal ?(normalize_lists = false) ty1 ty2 =
  Int.equal (constraint_ty_compare ~normalize_lists ty1 ty2) 0

let ty_equal ?(normalize_lists = false) ty1 ty2 =
  phys_equal (get_node ty1) (get_node ty2)
  || Int.equal 0 (ty_compare ~normalize_lists ty1 ty2)

let equal_internal_type ty1 ty2 =
  match (ty1, ty2) with
  | (LoclType ty1, LoclType ty2) -> ty_equal ~normalize_lists:true ty1 ty2
  | (ConstraintType ty1, ConstraintType ty2) ->
    constraint_ty_equal ~normalize_lists:true ty1 ty2
  | (_, (LoclType _ | ConstraintType _)) -> false

let equal_locl_ty : locl_ty -> locl_ty -> bool =
 (fun ty1 ty2 -> ty_equal ty1 ty2)

let equal_locl_ty_ : locl_ty_ -> locl_ty_ -> bool =
 (fun ty_1 ty_2 -> Int.equal 0 (ty__compare ty_1 ty_2))

let is_type_no_return : locl_ty_ -> bool = equal_locl_ty_ (Tprim Aast.Tnoreturn)

let equal_decl_ty_ : decl_ty_ -> decl_ty_ -> bool =
 (fun ty1 ty2 -> Int.equal 0 (ty__compare ty1 ty2))

let equal_decl_ty ty1 ty2 = equal_decl_ty_ (get_node ty1) (get_node ty2)

let equal_shape_field_type sft1 sft2 =
  equal_decl_ty sft1.sft_ty sft2.sft_ty
  && Bool.equal sft1.sft_optional sft2.sft_optional

let non_public_ifc ifc =
  match ifc with
  | FDPolicied (Some "PUBLIC") -> false
  | _ -> true

let equal_decl_tyl tyl1 tyl2 = List.equal equal_decl_ty tyl1 tyl2

let equal_decl_possibly_enforced_ty ety1 ety2 =
  equal_decl_ty ety1.et_type ety2.et_type
  && equal_enforcement ety1.et_enforced ety2.et_enforced

let equal_decl_fun_param param1 param2 =
  equal_decl_possibly_enforced_ty param1.fp_type param2.fp_type
  && Int.equal param1.fp_flags param2.fp_flags

let equal_decl_ft_params params1 params2 =
  List.equal equal_decl_fun_param params1 params2

let equal_decl_ft_implicit_params :
    decl_ty fun_implicit_params -> decl_ty fun_implicit_params -> bool =
 fun { capability = cap1 } { capability = cap2 } ->
  (* TODO(coeffects): could rework this so that implicit defaults and explicit
   * [defaults] are considered equal *)
  match (cap1, cap2) with
  | (CapDefaults p1, CapDefaults p2) -> Pos_or_decl.equal p1 p2
  | (CapTy c1, CapTy c2) -> equal_decl_ty c1 c2
  | (CapDefaults _, CapTy _)
  | (CapTy _, CapDefaults _) ->
    false

let equal_decl_fun_type fty1 fty2 =
  equal_decl_possibly_enforced_ty fty1.ft_ret fty2.ft_ret
  && equal_decl_ft_params fty1.ft_params fty2.ft_params
  && equal_decl_ft_implicit_params
       fty1.ft_implicit_params
       fty2.ft_implicit_params
  && Int.equal fty1.ft_flags fty2.ft_flags

let equal_abstract_typeconst at1 at2 =
  Option.equal equal_decl_ty at1.atc_as_constraint at2.atc_as_constraint
  && Option.equal
       equal_decl_ty
       at1.atc_super_constraint
       at2.atc_super_constraint
  && Option.equal equal_decl_ty at1.atc_default at2.atc_default

let equal_concrete_typeconst ct1 ct2 = equal_decl_ty ct1.tc_type ct2.tc_type

let equal_typeconst t1 t2 =
  match (t1, t2) with
  | (TCAbstract at1, TCAbstract at2) -> equal_abstract_typeconst at1 at2
  | (TCConcrete ct1, TCConcrete ct2) -> equal_concrete_typeconst ct1 ct2
  | _ -> false

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
  && equal_pos_id tp1.tp_name tp2.tp_name
  && List.equal
       (Tuple.T2.equal ~eq1:Ast_defs.equal_constraint_kind ~eq2:equal_decl_ty)
       tp1.tp_constraints
       tp2.tp_constraints
  && Aast.equal_reify_kind tp1.tp_reified tp2.tp_reified
  && List.equal
       equal_user_attribute
       tp1.tp_user_attributes
       tp2.tp_user_attributes

let equal_typedef_type tt1 tt2 =
  Pos_or_decl.equal tt1.td_pos tt2.td_pos
  && Aast.equal_typedef_visibility tt1.td_vis tt2.td_vis
  && List.equal equal_decl_tparam tt1.td_tparams tt2.td_tparams
  && Option.equal equal_decl_ty tt1.td_as_constraint tt2.td_as_constraint
  && Option.equal equal_decl_ty tt1.td_super_constraint tt2.td_super_constraint
  && equal_decl_ty tt1.td_type tt2.td_type

let equal_fun_elt fe1 fe2 =
  Option.equal String.equal fe1.fe_deprecated fe2.fe_deprecated
  && equal_decl_ty fe1.fe_type fe2.fe_type
  && Pos_or_decl.equal fe1.fe_pos fe2.fe_pos

let equal_const_decl cd1 cd2 =
  Pos_or_decl.equal cd1.cd_pos cd2.cd_pos
  && equal_decl_ty cd1.cd_type cd2.cd_type

let get_ce_abstract ce = ClassElt.is_abstract ce.ce_flags

let get_ce_final ce = ClassElt.is_final ce.ce_flags

let get_ce_superfluous_override ce =
  ClassElt.has_superfluous_override ce.ce_flags

let get_ce_lsb ce = ClassElt.has_lsb ce.ce_flags

let get_ce_synthesized ce = ClassElt.is_synthesized ce.ce_flags

let get_ce_const ce = ClassElt.is_const ce.ce_flags

let get_ce_lateinit ce = ClassElt.has_lateinit ce.ce_flags

let get_ce_readonly_prop ce = ClassElt.is_readonly_prop ce.ce_flags

let get_ce_dynamicallycallable ce = ClassElt.is_dynamicallycallable ce.ce_flags

let get_ce_support_dynamic_type ce = ClassElt.supports_dynamic_type ce.ce_flags

let get_ce_xhp_attr ce = Typing_defs_flags.ClassElt.get_xhp_attr ce.ce_flags

let get_ce_safe_global_variable ce =
  ClassElt.is_safe_global_variable ce.ce_flags

let make_ce_flags = Typing_defs_flags.ClassElt.make

(** Return true if the element is private and not marked with the __LSB
  attribute. Private elements are not inherited by child classes and are
  namespaced to the containing class--if B extends A, then A may define a
  method A::foo and B may define a method B::foo, and they both will exist in
  the hierarchy and be callable at runtime (which method is invoked depends on
  the caller).

  The __LSB attribute can be applied to properties only. LSB properties are
  (effectively) implicitly cloned into every subclass. This means that in the
  typechecker, we want to avoid filtering them out in subclasses, so we treat
  them as non-private here. *)
let class_elt_is_private_not_lsb (elt : class_elt) : bool =
  match elt.ce_visibility with
  | Vprivate _ -> not (get_ce_lsb elt)
  | Vprotected _
  | Vpublic
  | Vinternal _ ->
    false

let class_elt_is_private_or_protected_not_lsb elt =
  match elt.ce_visibility with
  | Vprivate _
  | Vprotected _
    when get_ce_lsb elt ->
    false
  | Vprivate _
  | Vprotected _ ->
    true
  | Vpublic
  | Vinternal _ ->
    false

(** Tunapplied_alias is a locl phase constructor that always stands for a higher-kinded type.
  We use this function in cases where Tunapplied_alias appears in a context where we expect
  a fully applied type, rather than a type constructor. Seeing Tunapplied_alias in such a context
  always indicates a kinding error, which means that during localization, we should have
  created Terr rather than Tunapplied_alias. Hence, this is an *internal* error, because
  something went wrong during localization. Kind mismatches in code are reported to users
  elsewhere. *)
let error_Tunapplied_alias_in_illegal_context () =
  failwith "Found Tunapplied_alias in a context where it must not occur"

let is_typeconst_type_abstract tc =
  match tc.ttc_kind with
  | TCConcrete _ -> false
  | TCAbstract _ -> true

module Attributes = struct
  let mem x xs =
    List.exists xs ~f:(fun { ua_name; _ } -> String.equal x (snd ua_name))

  let find x xs =
    List.find xs ~f:(fun { ua_name; _ } -> String.equal x (snd ua_name))
end
