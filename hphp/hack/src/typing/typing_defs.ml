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
  include Stdlib.Set.Make (CCR)

  type t_as_list = CCR.t list [@@deriving show]

  let pp fmt set = pp_t_as_list fmt (elements set)

  let show set = show_t_as_list (elements set)
end

type const_decl = {
  cd_pos: Pos_or_decl.t;
  cd_type: decl_ty;
  cd_value: string option;
}
[@@deriving show, eq]

type class_elt = {
  ce_visibility: ce_visibility;
  ce_type: decl_ty Lazy.t;
  ce_origin: string;  (** identifies the class from which this elt originates *)
  ce_deprecated: string option;
  ce_pos: Pos_or_decl.t Lazy.t;  (** pos of the type of the elt *)
  ce_flags: Typing_defs_flags.ClassElt.t;
  ce_sort_text: string option;
}
[@@deriving show]

type fun_elt = {
  fe_deprecated: string option;
  fe_module: Ast_defs.id option;
  fe_internal: bool;  (** Top-level functions have limited visibilities *)
  fe_type: decl_ty;
  fe_pos: Pos_or_decl.t;
  (* TODO: ideally these should be packed flags *)
  fe_php_std_lib: bool;
  fe_support_dynamic_type: bool;
  fe_no_auto_dynamic: bool;
  fe_no_auto_likes: bool;
}
[@@deriving show, eq]

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
type requirement = Pos_or_decl.t * decl_ty [@@deriving show]

type abstract_typeconst = {
  atc_as_constraint: decl_ty option;
  atc_super_constraint: decl_ty option;
  atc_default: decl_ty option;
}
[@@deriving eq, show]

type concrete_typeconst = { tc_type: decl_ty } [@@deriving eq, show]

type partially_abstract_typeconst = {
  patc_constraint: decl_ty;
  patc_type: decl_ty;
}
[@@deriving show]

type typeconst =
  | TCAbstract of abstract_typeconst
  | TCConcrete of concrete_typeconst
[@@deriving eq, show]

type typeconst_type = {
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
[@@deriving show]

type enum_type = {
  te_base: decl_ty;
  te_constraint: decl_ty option;
  te_includes: decl_ty list;
}
[@@deriving eq, show]

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
[@@deriving eq, show]

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

  (** Returns true if there was an attempt to add a cycle to the expansion list. *)
  val cyclic_expansion : t -> bool
end = struct
  type t = {
    report_cycle: (Pos.t * string) option;
        (** If we are expanding the RHS of a type definition, [report_cycle] contains
            the position and id of the LHS. This way, if the RHS expands at some point
            to the LHS id, we are able to report a cycle. *)
    expansions: (Pos_or_decl.t * string) list;
        (** A list of the type defs and type access we have expanded thus far. Used
            to prevent entering into a cycle when expanding these types. *)
    cyclic_expansion: bool ref;
  }

  let empty_w_cycle_report ~report_cycle =
    { report_cycle; expansions = []; cyclic_expansion = ref false }

  let empty = empty_w_cycle_report ~report_cycle:None

  let add ({ expansions; _ } as exps) exp =
    { exps with expansions = exp :: expansions }

  let has_expanded { report_cycle; expansions; _ } x =
    match report_cycle with
    | Some (p, x') when String.equal x x' -> Some (Some p)
    | Some _
    | None ->
      List.find_map expansions ~f:(function
          | (_, x') when String.equal x x' -> Some None
          | _ -> None)

  let add_and_check_cycles exps (p, id) =
    let has_cycle = has_expanded exps id in
    if Option.is_some has_cycle then exps.cyclic_expansion := true;
    let exps = add exps (p, id) in
    (exps, has_cycle)

  let as_list { report_cycle; expansions; _ } =
    (report_cycle
    |> Option.map ~f:(Tuple2.map_fst ~f:Pos_or_decl.of_raw_pos)
    |> Option.to_list)
    @ List.rev expansions

  let ids exps = as_list exps |> List.map ~f:snd

  let positions exps = as_list exps |> List.map ~f:fst

  let cyclic_expansion exps = !(exps.cyclic_expansion)
end

(** How should we treat the wildcard character _ when localizing?
 *  1. Generate a fresh type variable, e.g. in type argument to constructor or function,
 *     or in a lambda parameter or return type.
 *       Example: foo<shape('a' => _)>($myshape);
 *       Example: ($v : vec<_>) ==> $v[0]
 *  2. As a placeholder in a formal higher-kinded type parameter
 *       Example: function test<T1, T2<_>>() // T2 is HK and takes a type to a type
 *  3. Generate a fresh generic (aka Skolem variable), e.g. in `is` or `as` test
 *       Example: if ($x is Vector<_>) { // $x has type Vector<T#1> }
 *  4. Reject, when in a type argument to a generic parameter marked <<__Explicit>>
 *       Example: makeVec<_>(3)  where function makeVec<<<__Explicit>> T>(T $_): void
 *  5. Reject, because the type must be explicit.
 *)
type wildcard_action =
  | Wildcard_fresh_tyvar
  | Wildcard_fresh_generic
  | Wildcard_higher_kinded_placeholder
  | Wildcard_require_explicit of decl_tparam
  | Wildcard_illegal

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
  wildcard_action: wildcard_action;
}

let empty_expand_env =
  {
    type_expansions = Type_expansions.empty;
    expand_visible_newtype = true;
    substs = SMap.empty;
    this_ty =
      mk (Reason.none, Tgeneric (Naming_special_names.Typehints.this, []));
    on_error = None;
    wildcard_action = Wildcard_fresh_tyvar;
  }

let empty_expand_env_with_on_error on_error =
  { empty_expand_env with on_error = Some on_error }

let add_type_expansion_check_cycles env exp =
  let (type_expansions, has_cycle) =
    Type_expansions.add_and_check_cycles env.type_expansions exp
  in
  let env = { env with type_expansions } in
  (env, has_cycle)

let cyclic_expansion env = Type_expansions.cyclic_expansion env.type_expansions

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
  | Tvar v' when Tvid.equal v v' -> true
  | _ -> false

let is_generic t =
  match get_node t with
  | Tgeneric _ -> true
  | _ -> false

let is_dynamic t =
  match get_node t with
  | Tdynamic -> true
  | _ -> false

let is_nothing t =
  match get_node t with
  | Tunion [] -> true
  | _ -> false

let is_wildcard t =
  match get_node t with
  | Twildcard -> true
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
    | DTexpr i -> Expression_id.display i

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
  | Tshape { s_origin = _; s_unknown_value = unknown_field_type; s_fields = sm }
    ->
    TShapeMap.for_all (fun _ { sft_ty; _ } -> is_denotable sft_ty) sm
    && unknown_field_type_is_denotable unknown_field_type
  | Tfun { ft_params; ft_ret; _ } ->
    is_denotable ft_ret
    && List.for_all ft_params ~f:(fun { fp_type; _ } -> is_denotable fp_type)
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

and unknown_field_type_is_denotable ty =
  match get_node ty with
  | Tunion [] -> true
  | Tintersection [] -> true
  | Toption ty -> begin
    match get_node ty with
    | Tnonnull -> true
    | _ -> false
  end
  | _ -> false

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

let is_type_no_return : locl_ty_ -> bool = equal_locl_ty_ (Tprim Aast.Tnoreturn)

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
