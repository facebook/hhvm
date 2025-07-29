(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Reason = Typing_reason

type pos_id = Reason.pos_id [@@deriving eq, hash, ord, show]

type ce_visibility =
  | Vpublic
  | Vprivate of string
  | Vprotected of string
  (* When we construct `Vinternal`, we are guaranteed to be inside a module *)
  | Vinternal of string
  | Vprotected_internal of {
      class_id: string;
      module_: string;
    }
[@@deriving eq, ord, show]

type cross_package_decl = string option
[@@deriving eq, hash, ord, show { with_path = false }]

(* All the possible types, reason is a trace of why a type
   was inferred in a certain way.

   Types exists in two phases. Phase one is 'decl', meaning it is a type that
   was declared in user code. Phase two is 'locl', meaning it is a type that is
   inferred via local inference.
*)
(* create private types to represent the different type phases *)
type decl_phase = Reason.decl_phase [@@deriving eq, hash, show]

type locl_phase = Reason.locl_phase [@@deriving eq, hash, show]

type val_kind =
  | Lval
  | LvalSubexpr
  | Other
[@@deriving eq]

type type_origin =
  | Missing_origin
  | From_alias of
      string * (Pos_or_decl.t option[@hash.ignore] [@ignore] [@opaque])
[@@deriving eq, hash, ord, show]

type pos_string = (Pos_or_decl.t[@hash.ignore]) * string
[@@deriving eq, hash, ord, show]

(* Trick the Rust generators to use a BStr, the same way it does for Ast_defs.shape_field_name. *)
type t_byte_string = string [@@deriving eq, hash, ord, show]

type pos_byte_string = (Pos_or_decl.t[@hash.ignore]) * t_byte_string
[@@deriving eq, hash, ord, show]

type tshape_field_name =
  | TSFregex_group of pos_string
  | TSFlit_str of pos_byte_string
  | TSFclass_const of pos_id * pos_string
[@@deriving eq, hash, ord, show]

module TShapeField = struct
  type t = tshape_field_name [@@deriving hash, show { with_path = false }]

  let pos : t -> Pos_or_decl.t = function
    | TSFregex_group (p, _)
    | TSFlit_str (p, _) ->
      p
    | TSFclass_const ((cls_pos, _), (mem_pos, _)) ->
      Pos_or_decl.btw cls_pos mem_pos

  let name = function
    | TSFregex_group (_, s)
    | TSFlit_str (_, s) ->
      s
    | TSFclass_const ((_, s1), (_, s2)) -> s1 ^ "::" ^ s2

  let of_ast : (Pos.t -> Pos_or_decl.t) -> Ast_defs.shape_field_name -> t =
   fun convert_pos -> function
    | Ast_defs.SFlit_str (p, s) -> TSFlit_str (convert_pos p, s)
    | Ast_defs.SFclassname (p, cls) ->
      TSFclass_const ((convert_pos p, cls), (convert_pos p, "class"))
    | Ast_defs.SFclass_const ((pcls, cls), (pconst, const)) ->
      TSFclass_const ((convert_pos pcls, cls), (convert_pos pconst, const))

  (* This ignores positions!
   * We include span information in tshape_field_name to improve error
   * messages, but we don't want it being used in the comparison, so
   * we have to write our own compare. *)
  let compare x y =
    match (x, y) with
    | (TSFregex_group (_, s1), TSFregex_group (_, s2)) -> String.compare s1 s2
    | (TSFlit_str (_, s1), TSFlit_str (_, s2)) -> String.compare s1 s2
    | (TSFclass_const ((_, s1), (_, s1')), TSFclass_const ((_, s2), (_, s2')))
      ->
      Core.Tuple.T2.compare
        ~cmp1:String.compare
        ~cmp2:String.compare
        (s1, s1')
        (s2, s2')
    | (TSFregex_group _, _) -> -1
    | (TSFlit_str _, TSFregex_group _) -> 1
    | (TSFlit_str _, _) -> -1
    | (TSFclass_const _, _) -> 1

  (* This also ignores positions for the same reasons. *)
  let equal x y = Core.Int.equal 0 (compare x y)
end

module TShapeMap = struct
  include WrappedMap.Make (TShapeField)

  let map_and_rekey m f1 f2 =
    fold (fun k v acc -> add (f1 k) (f2 v) acc) m empty

  let pp
      (pp_val : Format.formatter -> 'a -> unit)
      (fmt : Format.formatter)
      (map : 'a t) : unit =
    make_pp pp_tshape_field_name pp_val fmt map

  let hash_fold_t x = make_hash_fold_t TShapeField.hash_fold_t x
end

module TShapeSet = Stdlib.Set.Make (TShapeField)

type param_mode =
  | FPnormal
  | FPinout
[@@deriving eq, show]

type xhp_attr = Xhp_attribute.t [@@deriving eq, show]

(** Denotes the categories of requirements we apply to constructor overrides.
 *
 * In the default case, we use Inconsistent. If a class has <<__ConsistentConstruct>>,
 * or if it inherits a class that has <<__ConsistentConstruct>>, we use inherited.
 * If we have a new final class that doesn't extend from <<__ConsistentConstruct>>,
 * then we use Final. Only classes that are Inconsistent or Final can have reified
 * generics. *)
type consistent_kind =
  | Inconsistent
  | ConsistentConstruct
  | FinalClass
[@@deriving eq, show]

type dependent_type =
  (* A reference to some expression. For example:
   *
   *  $x->foo()
   *
   *  The expression $x would have a reference ident
   *  The expression $x->foo() would have a different one
   *)
  | DTexpr of Expression_id.t
[@@deriving eq, hash, ord, show]

type user_attribute_param =
  | Classname of string
  | EnumClassLabel of string
  | String of t_byte_string
  | Int of string
[@@deriving eq, hash, show]

let user_attribute_param_to_string = function
  | Classname s
  | EnumClassLabel s
  | String s
  | Int s ->
    s

type user_attribute = {
  ua_name: pos_id;
  ua_params: user_attribute_param list;
  ua_raw_val: string option;
}
[@@deriving eq, hash, show]

type 'ty tparam = {
  tp_variance: Ast_defs.variance;
  tp_name: pos_id;
  tp_constraints: (Ast_defs.constraint_kind * 'ty) list;
  tp_reified: Ast_defs.reify_kind;
  tp_user_attributes: user_attribute list;
}
[@@deriving eq, hash, show, map]

type 'ty where_constraint = 'ty * Ast_defs.constraint_kind * 'ty
[@@deriving eq, hash, show, map]

type enforcement =
  | Unenforced
  | Enforced
[@@deriving eq, hash, show, ord]

type 'ty capability =
  | CapDefaults of (Pos_or_decl.t[@hash.ignore])
  | CapTy of 'ty
[@@deriving eq, hash, show { with_path = false }, map]

(** Companion to fun_params type, intended to consolidate checking of
 * implicit params for functions. *)
type 'ty fun_implicit_params = { capability: 'ty capability }
[@@deriving eq, hash, show { with_path = false }, map]

type 'ty fun_param = {
  fp_pos: Pos_or_decl.t; [@hash.ignore] [@equal (fun _ _ -> true)]
  fp_name: string option;
  fp_type: 'ty;
  fp_flags: Typing_defs_flags.FunParam.t;
  fp_def_value: string option;
}
[@@deriving eq, hash, show { with_path = false }, map]

type 'ty fun_params = 'ty fun_param list
[@@deriving eq, hash, show { with_path = false }, map]

(** The type of a function AND a method. *)
type 'ty fun_type = {
  ft_tparams: 'ty tparam list;
  ft_where_constraints: 'ty where_constraint list;
  ft_params: 'ty fun_params;
  ft_implicit_params: 'ty fun_implicit_params;
  ft_ret: 'ty;  (** Carries through the sync/async information from the aast *)
  ft_flags: Typing_defs_flags.Fun.t;
  ft_cross_package: cross_package_decl;
  ft_instantiated: bool;
}
[@@deriving eq, hash, show { with_path = false }, map]

(* This is to avoid a compile error with ppx_hash "Unbound value _hash_fold_phase". *)
let _hash_fold_phase hsv _ = hsv

type 'phase ty = ('phase Reason.t_[@transform.opaque]) * 'phase ty_

and type_tag_generic =
  | Filled of locl_phase ty
  | Wildcard of int

and type_tag =
  | BoolTag
  | IntTag
  | StringTag
  | ArraykeyTag
  | FloatTag
  | NumTag
  | ResourceTag
  | NullTag
  | ClassTag of Ast_defs.id_ * type_tag_generic list

and shape_field_predicate = {
  (* T196048813 *)
  (* sfp_optional: bool; *)
  sfp_predicate: type_predicate;
}

and shape_predicate = {
  (* T196048813 *)
  (* sp_allows_unknown_fields: bool; *)
  sp_fields: shape_field_predicate TShapeMap.t;
}

(* TODO optional and variadic components T201398626 T201398652 *)
and tuple_predicate = { tp_required: type_predicate list }

and type_predicate_ =
  | IsTag of type_tag
  | IsTupleOf of tuple_predicate
  | IsShapeOf of shape_predicate

and type_predicate =
  (Reason.t[@hash.ignore] [@transform.opaque]) * type_predicate_

(** A shape may specify whether or not fields are required. For example, consider
 * this typedef:
 *
 * ```
 * type ShapeWithOptionalField = shape(?'a' => ?int);
 * ```
 *
 * With this definition, the field 'a' may be unprovided in a shape. In this
 * case, the field 'a' would have sf_optional set to true.
 *)
and 'phase shape_field_type = {
  sft_optional: bool;
  sft_ty: 'phase ty;
}

and _ ty_ =
  (*========== Following Types Exist Only in the Declared Phase ==========*)
  | Tthis : decl_phase ty_  (** The late static bound type of a class *)
  | Tapply : (pos_id[@transform.opaque]) * decl_phase ty list -> decl_phase ty_
      (** Either an object type or a type alias, ty list are the arguments *)
  | Trefinement : decl_phase ty * decl_phase class_refinement -> decl_phase ty_
      (** 'With' refinements of the form `_ with { type T as int; type TC = C; }`. *)
  | Tmixed : decl_phase ty_
      (** "Any" is the type of a variable with a missing annotation, and "mixed" is
       * the type of a variable annotated as "mixed". THESE TWO ARE VERY DIFFERENT!
       * Any unifies with anything, i.e., it is both a supertype and subtype of any
       * other type. You can do literally anything to it; it's the "trust me" type.
       * Mixed, on the other hand, is only a supertype of everything. You need to do
       * a case analysis to figure out what it is (i.e., its elimination form).
       *
       * Here's an example to demonstrate:
       *
       * ```
       * function f($x): int {
       *   return $x + 1;
       * }
       * ```
       *
       * In that example, $x has type Tany. This unifies with anything, so adding
       * one to it is allowed, and returning that as int is allowed.
       *
       * In contrast, if $x were annotated as mixed, adding one to that would be
       * a type error -- mixed is not a subtype of int, and you must be a subtype
       * of int to take part in addition. (The converse is true though -- int is a
       * subtype of mixed.) A case analysis would need to be done on $x, via
       * is_int or similar.
       *
       * mixed exists only in the decl_phase phase because it is desugared into ?nonnull
       * during the localization phase.
       *)
  | Twildcard : decl_phase ty_
      (** Various intepretations, depending on context.
        *   inferred type e.g. (vec<_> $x) ==> $x[0]
        *   placeholder in refinement e.g. $x as Vector<_>
        *   placeholder for higher-kinded formal type parameter e.g. foo<T1<_>>(T1<int> $_)
        *)
  | Tlike : decl_phase ty -> decl_phase ty_
  (*========== Following Types Exist in Both Phases ==========*)
  | Tany : (TanySentinel.t[@transform.opaque]) -> 'phase ty_
  | Tnonnull : 'phase ty_
  | Tdynamic : 'phase ty_
      (** A dynamic type is a special type which sometimes behaves as if it were a
       * top type; roughly speaking, where a specific value of a particular type is
       * expected and that type is dynamic, anything can be given. We call this
       * behaviour "coercion", in that the types "coerce" to dynamic. In other ways it
       * behaves like a bottom type; it can be used in any sort of binary expression
       * or even have object methods called from it. However, it is in fact neither.
       *
       * it captures dynamicism within function scope.
       * See tests in typecheck/dynamic/ for more examples.
       *)
  | Toption : 'phase ty -> 'phase ty_
      (** Nullable, called "option" in the ML parlance. *)
  | Tprim : (Ast_defs.tprim[@transform.opaque]) -> 'phase ty_
      (** All the primitive types: int, string, void, etc. *)
  | Tfun : 'phase ty fun_type -> 'phase ty_
      (** A wrapper around fun_type, which contains the full type information for a
       * function, method, lambda, etc. *)
  | Ttuple : 'phase tuple_type -> 'phase ty_
      (** A wrapper around tuple_type, which contains information about tuple elements *)
  | Tshape : 'phase shape_type -> 'phase ty_
  | Tgeneric : string -> 'phase ty_
      (** The type of a generic parameter. The constraints on a generic parameter
       * are accessed through the lenv.tpenv component of the environment, which
       * is set up when checking the body of a function or method. See uses of
       * Typing_phase.add_generic_parameters_and_constraints.
       *)
  | Tunion : 'phase ty list -> 'phase ty_ [@transform.explicit]
      (** Union type.
       * The values that are members of this type are the union of the values
       * that are members of the components of the union.
       * Some examples (writing | for binary union)
       *   Tunion []  is the "nothing" type, with no values
       *   Tunion [int;float] is the same as num
       *   Tunion [null;t] is the same as Toption t
       *)
  | Tintersection : 'phase ty list -> 'phase ty_
  | Tvec_or_dict : 'phase ty * 'phase ty -> 'phase ty_
      (** Tvec_or_dict (ty1, ty2) => "vec_or_dict<ty1, ty2>" *)
  | Taccess : 'phase taccess_type -> 'phase ty_
      (** Name of class, name of type const, remaining names of type consts *)
  | Tclass_ptr : 'phase ty -> 'phase ty_
      (** A type of a class pointer, class<T>. To be compatible with classname<T>,
        * it takes an arbitrary type. In the future, it should only take a string
        * that is a class name, and be named Tclass. The current Tclass would be
        * renamed to Tinstance, where a Tinstance is an instantiation of a Tclass *)
  (*========== Below Are Types That Cannot Be Declared In User Code ==========*)
  | Tvar : (Tvid.t[@transform.opaque]) -> locl_phase ty_
  | Tnewtype : string * locl_phase ty list * locl_phase ty -> locl_phase ty_
      (** The type of an opaque type or enum. Outside their defining files or
        when they represent enums, they are "opaque", which means that they
        only unify with themselves. Within a file, uses of newtypes are
        expanded to their definitions (unless the newtype is an enum).

        However, it is possible to have a constraint that allows us to relax
        opaqueness. For example:

          newtype MyType as int = ...

        or

          enum MyType: int as int { ... }

        Outside of the file where the type was defined, this translates to:

          Tnewtype ((pos, "MyType"), [], Tprim Tint)

        which means that MyType is abstract, but is a subtype of int as well.
        When the constraint is omitted, the third parameter is set to mixed.

        The second parameter is the list of type arguments to the type.
       *)
  | Tdependent :
      (dependent_type[@transform.opaque]) * locl_phase ty
      -> locl_phase ty_  (** see dependent_type *)
  | Tclass :
      (pos_id[@transform.opaque]) * exact * locl_phase ty list
      -> locl_phase ty_
      (** An instance of a class or interface, ty list are the arguments
       * If exact=Exact, then this represents instances of *exactly* this class
       * If exact=Nonexact, this also includes subclasses
       * TODO(T199606542) rename this to Tinstance *)
  | Tneg : (type_predicate[@transform.opaque]) -> locl_phase ty_
      (** The negation of the [type_predicate] *)
  | Tlabel : string -> locl_phase ty_
      (** The type of the label expression #ID *)

and 'phase taccess_type = 'phase ty * (pos_id[@transform.opaque])

and exact =
  | Exact
  | Nonexact of locl_phase class_refinement

(** Class refinements are for type annotations like

      Box with {type T = string}
  *)
and 'phase class_refinement = { cr_consts: 'phase refined_const SMap.t }

and 'phase refined_const = {
  rc_bound: 'phase refined_const_bound;
  rc_is_ctx: bool;
}

and 'phase refined_const_bound =
  | TRexact : 'phase ty -> 'phase refined_const_bound
      (** for `=` constraints *)
  | TRloose : 'phase refined_const_bounds -> 'phase refined_const_bound
      (** for `as` or `super` constraints *)

and 'phase refined_const_bounds = {
  tr_lower: 'phase ty list;
  tr_upper: 'phase ty list;
}

(** Whether all fields of this shape are known, types of each of the
  * known arms.
  *)
and 'phase shape_type = {
  s_origin: type_origin; [@transform.opaque]
  s_unknown_value: 'phase ty;
  s_fields: 'phase shape_field_type TShapeMap.t;
}

(**
  Required and extra components of a tuple. Extra components
  are either optional + variadic, or a type splat.
  Exmaple 1:
    (string,bool,optional float,optional bool,int...)
  has require components string, bool, optional components float, bool
  and variadic component int.
  Example 2:
    (string,float,...T)
  has required components string, float, and splat component T.
*)
and 'phase tuple_type = {
  t_required: 'phase ty list;
  t_extra: 'phase tuple_extra;
}

and 'phase tuple_extra =
  | Textra of {
      t_optional: 'phase ty list;
      t_variadic: 'phase ty;
    }
  | Tsplat of 'phase ty
[@@deriving hash, transform]

type decl_ty = decl_phase ty [@@deriving hash]

type locl_ty = locl_phase ty [@@deriving hash]

let nonexact = Nonexact { cr_consts = SMap.empty }

let is_nonexact = function
  | Nonexact _ -> true
  | Exact -> false

let refined_const_kind_str : type a. a refined_const -> string =
 fun { rc_is_ctx; _ } ->
  if rc_is_ctx then
    "ctx"
  else
    "type"

module Flags = struct
  open Typing_defs_flags

  let get_ft_return_disposable ft = Fun.return_disposable ft.ft_flags

  let get_ft_returns_readonly ft = Fun.returns_readonly ft.ft_flags

  let get_ft_readonly_this ft = Fun.readonly_this ft.ft_flags

  let get_ft_async ft = Fun.async ft.ft_flags

  let get_ft_generator ft = Fun.generator ft.ft_flags

  let get_ft_support_dynamic_type ft = Fun.support_dynamic_type ft.ft_flags

  (* This flag is set true only if the exact method has the memoized attribute. *)
  let get_ft_is_memoized ft = Fun.is_memoized ft.ft_flags

  let set_ft_is_function_pointer ft is_fp =
    { ft with ft_flags = Fun.set_is_function_pointer is_fp ft.ft_flags }

  let get_ft_is_function_pointer ft = Fun.is_function_pointer ft.ft_flags

  let set_ft_readonly_this ft readonly_this =
    { ft with ft_flags = Fun.set_readonly_this readonly_this ft.ft_flags }

  let set_ft_returns_readonly ft readonly_return =
    { ft with ft_flags = Fun.set_returns_readonly readonly_return ft.ft_flags }

  let set_ft_support_dynamic_type ft readonly_return =
    {
      ft with
      ft_flags = Fun.set_support_dynamic_type readonly_return ft.ft_flags;
    }

  let get_ft_variadic ft = Fun.variadic ft.ft_flags

  let get_ft_fun_kind ft = Fun.fun_kind ft.ft_flags

  let get_fp_readonly fp = FunParam.readonly fp.fp_flags

  let get_fp_splat fp = FunParam.splat fp.fp_flags

  let make_fp_flags
      ~mode
      ~accept_disposable
      ~is_optional
      ~readonly
      ~ignore_readonly_error
      ~splat =
    let inout =
      match mode with
      | FPinout -> true
      | FPnormal -> false
    in
    FunParam.make
      ~inout
      ~accept_disposable
      ~is_optional
      ~readonly
      ~ignore_readonly_error
      ~splat

  let get_fp_accept_disposable fp = FunParam.accept_disposable fp.fp_flags

  let get_fp_is_optional fp = FunParam.is_optional fp.fp_flags

  let get_fp_ignore_readonly_error fp =
    FunParam.ignore_readonly_error fp.fp_flags

  let get_fp_mode fp =
    if FunParam.inout fp.fp_flags then
      FPinout
    else
      FPnormal
end

include Flags

module Pp = struct
  let rec pp_ty : type a. Format.formatter -> a ty -> unit =
   fun fmt t ->
    let (a0, a1) = t in
    Format.fprintf fmt "(@[";
    Reason.pp_t_ fmt a0;
    Format.fprintf fmt ",@ ";
    pp_ty_ fmt a1;
    Format.fprintf fmt "@])"

  and pp_ty_ : type a. Format.formatter -> a ty_ -> unit =
   fun fmt ty ->
    match ty with
    | Tany _ -> Format.pp_print_string fmt "Tany"
    | Tthis -> Format.pp_print_string fmt "Tthis"
    | Tmixed -> Format.pp_print_string fmt "Tmixed"
    | Twildcard -> Format.pp_print_string fmt "Twildcard"
    | Tdynamic -> Format.pp_print_string fmt "Tdynamic"
    | Tnonnull -> Format.pp_print_string fmt "Tnonnull"
    | Tapply (a0, a1) ->
      Format.fprintf fmt "(@[<2>Tapply (@,";
      let () = pp_pos_id fmt a0 in
      Format.fprintf fmt ",@ ";
      pp_list pp_ty fmt a1;
      Format.fprintf fmt "@,))@]"
    | Trefinement (a0, a1) ->
      Format.fprintf fmt "(@[<2>Trefinement (@,";
      pp_ty fmt a0;
      Format.fprintf fmt ",@ ";
      pp_class_refinement fmt a1;
      Format.fprintf fmt "@,))@]"
    | Tgeneric a0 ->
      Format.fprintf fmt "(@[<2>Tgeneric@ ";
      Format.fprintf fmt "%S" a0;
      Format.fprintf fmt "@])"
    | Taccess a0 ->
      Format.fprintf fmt "(@[<2>Taccess@ ";
      pp_taccess_type fmt a0;
      Format.fprintf fmt "@])"
    | Tvec_or_dict (a0, a1) ->
      Format.fprintf fmt "(@[<2>Tvec_or_dict@ ";
      pp_ty fmt a0;
      Format.fprintf fmt ",@ ";
      pp_ty fmt a1;
      Format.fprintf fmt "@])"
    | Toption a0 ->
      Format.fprintf fmt "(@[<2>Toption@ ";
      pp_ty fmt a0;
      Format.fprintf fmt "@])"
    | Tlike a0 ->
      Format.fprintf fmt "(@[<2>Tlike@ ";
      pp_ty fmt a0;
      Format.fprintf fmt "@])"
    | Tprim a0 ->
      Format.fprintf fmt "(@[<2>Tprim@ ";
      Aast.pp_tprim fmt a0;
      Format.fprintf fmt "@])"
    | Tfun a0 ->
      Format.fprintf fmt "(@[<2>Tfun@ ";
      pp_fun_type pp_ty fmt a0;
      Format.fprintf fmt "@])"
    | Ttuple a0 ->
      Format.fprintf fmt "(@[<2>Ttuple@ ";
      pp_tuple_type fmt a0;
      Format.fprintf fmt "@])"
    | Tshape a0 ->
      Format.fprintf fmt "(@[<2>Tshape@ ";
      pp_shape_type fmt a0;
      Format.fprintf fmt "@])"
    | Tvar a0 ->
      Format.fprintf fmt "(@[<2>Tvar@ ";
      Tvid.pp fmt a0;
      Format.fprintf fmt "@])"
    | Tnewtype (a0, a1, a2) ->
      Format.fprintf fmt "(@[<2>Tnewtype (@,";
      Format.fprintf fmt "%S" a0;
      Format.fprintf fmt ",@ ";
      pp_list pp_ty fmt a1;
      Format.fprintf fmt ",@ ";
      pp_ty fmt a2;
      Format.fprintf fmt "@,))@]"
    | Tdependent (a0, a1) ->
      Format.fprintf fmt "(@[<2>Tdependent@ ";
      pp_dependent_type fmt a0;
      Format.fprintf fmt ",@ ";
      pp_ty fmt a1;
      Format.fprintf fmt "@])"
    | Tunion tyl ->
      Format.fprintf fmt "(@[<2>Tunion@ ";
      pp_list pp_ty fmt tyl;
      Format.fprintf fmt "@])"
    | Tintersection tyl ->
      Format.fprintf fmt "(@[<2>Tintersection@ ";
      pp_list pp_ty fmt tyl;
      Format.fprintf fmt "@])"
    | Tclass (a0, a2, a1) ->
      Format.fprintf fmt "(@[<2>Tclass (@,";
      pp_pos_id fmt a0;
      Format.fprintf fmt ",@ ";
      pp_exact fmt a2;
      Format.fprintf fmt ",@ ";
      pp_list pp_ty fmt a1;
      Format.fprintf fmt "@,))@]"
    | Tneg a0 ->
      Format.fprintf fmt "(@[<2>Tneg@ ";
      pp_type_predicate fmt a0;
      Format.fprintf fmt "@])"
    | Tlabel a0 ->
      Format.fprintf fmt "(@[<2>Tlabel@ ";
      Format.fprintf fmt "%S" a0;
      Format.fprintf fmt "@])"
    | Tclass_ptr a0 ->
      Format.fprintf fmt "(@[<2>Tclass_ptr@ ";
      pp_ty fmt a0;
      Format.fprintf fmt "@])"

  and pp_list :
      type a.
      (Format.formatter -> a -> unit) -> Format.formatter -> a list -> unit =
   fun pp_elem fmt l ->
    Format.fprintf fmt "@[<2>[";
    ignore
      (List.fold_left
         ~f:(fun sep x ->
           if sep then Format.fprintf fmt ";@ ";
           pp_elem fmt x;
           true)
         ~init:false
         l);
    Format.fprintf fmt "@,]@]"

  and pp_refined_const : type a. Format.formatter -> a refined_const -> unit =
   fun fmt { rc_bound; rc_is_ctx } ->
    Format.fprintf fmt "@[<2>{";
    Format.fprintf fmt "rc_bound = ";
    (match rc_bound with
    | TRexact exact ->
      Format.fprintf fmt "TRexact ";
      pp_ty fmt exact
    | TRloose { tr_lower = lower; tr_upper = upper } ->
      Format.fprintf fmt "TRloose @[<2>{";
      Format.pp_print_string fmt "tr_lower = ";
      pp_list pp_ty fmt lower;
      Format.fprintf fmt ";@ ";
      Format.pp_print_string fmt "tr_upper = ";
      pp_list pp_ty fmt upper);
    Format.fprintf fmt ";@ ";
    Format.fprintf fmt "rc_is_ctx = %B" rc_is_ctx;
    Format.fprintf fmt "}@]"

  and pp_class_refinement :
      type a. Format.formatter -> a class_refinement -> unit =
   fun fmt { cr_consts } ->
    Format.fprintf fmt "@[<2>{";
    Format.fprintf fmt "cr_consts = ";
    SMap.pp pp_refined_const fmt cr_consts;
    Format.fprintf fmt ";@ ";
    Format.fprintf fmt "}@]"

  and pp_exact fmt (e : exact) =
    match e with
    | Exact -> Format.pp_print_string fmt "Exact"
    | Nonexact cr ->
      Format.pp_print_string fmt "Nonexact ";
      pp_class_refinement fmt cr

  and pp_taccess_type : type a. Format.formatter -> a taccess_type -> unit =
   fun fmt (a0, a1) ->
    Format.fprintf fmt "(@[";
    pp_ty fmt a0;
    Format.fprintf fmt ",@ ";
    Format.fprintf fmt "@[<2>[";
    pp_pos_id fmt a1;
    Format.fprintf fmt "@,]@]";
    Format.fprintf fmt "@])"

  and pp_shape_field_type :
      type a. Format.formatter -> a shape_field_type -> unit =
   fun fmt { sft_optional; sft_ty } ->
    Format.fprintf fmt "@[<2>{ ";
    Format.fprintf fmt "@[%s =@ " "sft_optional";
    Format.fprintf fmt "%B" sft_optional;
    Format.fprintf fmt "@]";
    Format.fprintf fmt ";@ ";
    Format.fprintf fmt "@[%s =@ " "sft_ty";
    pp_ty fmt sft_ty;
    Format.fprintf fmt "@]";
    Format.fprintf fmt "@ }@]"

  and pp_shape_type : type a. Format.formatter -> a shape_type -> unit =
   fun fmt { s_origin; s_fields; s_unknown_value } ->
    Format.fprintf fmt "@[<2>{ ";

    Format.fprintf fmt "@[%s =@ " "s_origin";
    pp_type_origin fmt s_origin;
    Format.fprintf fmt "@]";
    Format.fprintf fmt ";@ ";

    Format.fprintf fmt "@[%s =@ " "s_fields";
    TShapeMap.pp pp_shape_field_type fmt s_fields;
    Format.fprintf fmt "@]";
    Format.fprintf fmt ";@ ";

    Format.fprintf fmt "@[%s =@ " "s_unknown_value";
    pp_ty fmt s_unknown_value;
    Format.fprintf fmt "@]";

    Format.fprintf fmt "@ }@]"

  and pp_tuple_extra : type a. Format.formatter -> a tuple_extra -> unit =
   fun fmt extra ->
    match extra with
    | Textra { t_optional; t_variadic } ->
      Format.fprintf fmt "@[%s =@ " "t_optional";
      pp_list pp_ty fmt t_optional;
      Format.fprintf fmt "@]";
      Format.fprintf fmt ";@ ";

      Format.fprintf fmt "@[%s =@ " "t_variadic";
      pp_ty fmt t_variadic;
      Format.fprintf fmt "@]"
    | Tsplat t_splat ->
      Format.fprintf fmt "@[%s =@ " "t_splat";
      pp_ty fmt t_splat;
      Format.fprintf fmt "@]"

  and pp_tuple_type : type a. Format.formatter -> a tuple_type -> unit =
   fun fmt { t_required; t_extra } ->
    Format.fprintf fmt "@[<2>{ ";

    Format.fprintf fmt "@[%s =@ " "t_required";
    pp_list pp_ty fmt t_required;
    Format.fprintf fmt "@]";
    Format.fprintf fmt ";@ ";

    pp_tuple_extra fmt t_extra;

    Format.fprintf fmt "@ }@]"

  and pp_type_predicate_ fmt predicate_ =
    match predicate_ with
    | IsTag tag ->
      Format.fprintf fmt "(@[<2>IsTag@ ";
      pp_type_tag fmt tag;
      Format.fprintf fmt "@])"
    | IsTupleOf tuple_predicate ->
      Format.fprintf fmt "(@[<2>IsTupleOf@ ";
      pp_tuple_predicate fmt tuple_predicate;
      Format.fprintf fmt "@])"
    | IsShapeOf shape_predicate ->
      Format.fprintf fmt "(@[<2>IsShapeOf@ ";
      pp_shape_predicate fmt shape_predicate;
      Format.fprintf fmt "@])"

  and pp_tuple_predicate fmt { tp_required } =
    Format.fprintf fmt "@[<2>{ ";
    Format.fprintf fmt "@[%s =@ " "tp_required";
    pp_list pp_type_predicate fmt tp_required;
    Format.fprintf fmt "@]";
    Format.fprintf fmt "@ }@]"

  and pp_shape_predicate fmt { sp_fields } =
    Format.fprintf fmt "@[<2>{ ";
    Format.fprintf fmt "@[%s =@ " "sp_fields";
    TShapeMap.pp pp_shape_field_predicate fmt sp_fields;
    Format.fprintf fmt "@]";
    Format.fprintf fmt "@ }@]"

  and pp_shape_field_predicate fmt { sfp_predicate } =
    Format.fprintf fmt "@[<2>{ ";
    Format.fprintf fmt "@[%s =@ " "sfp_predicate";
    pp_type_predicate fmt sfp_predicate;
    Format.fprintf fmt "@]";
    Format.fprintf fmt "@ }@]"

  and pp_type_tag_generic fmt generic =
    match generic with
    | Filled ty ->
      Format.fprintf fmt "(@[<2>Filled@ ";
      pp_ty fmt ty;
      Format.fprintf fmt "@])"
    | Wildcard id ->
      Format.fprintf fmt "(@[<2>Wildcard@ ";
      Format.pp_print_int fmt id;
      Format.fprintf fmt "@])"

  and pp_type_tag fmt tag =
    match tag with
    | BoolTag -> Format.pp_print_string fmt "BoolTag"
    | IntTag -> Format.pp_print_string fmt "IntTag"
    | StringTag -> Format.pp_print_string fmt "StringTag"
    | ArraykeyTag -> Format.pp_print_string fmt "ArraykeyTag"
    | FloatTag -> Format.pp_print_string fmt "FloatTag"
    | NumTag -> Format.pp_print_string fmt "NumTag"
    | ResourceTag -> Format.pp_print_string fmt "ResourceTag"
    | NullTag -> Format.pp_print_string fmt "NullTag"
    | ClassTag (id, args) ->
      Format.fprintf fmt "(@[<2>ClassTag (@,";
      Format.pp_print_string fmt id;
      Format.fprintf fmt ",@ ";
      pp_list pp_type_tag_generic fmt args;
      Format.fprintf fmt "@,))@]"

  and pp_type_predicate fmt predicate = pp_type_predicate_ fmt (snd predicate)

  let pp_decl_ty : Format.formatter -> decl_ty -> unit =
   (fun fmt ty -> pp_ty fmt ty)

  let pp_locl_ty : Format.formatter -> locl_ty -> unit =
   (fun fmt ty -> pp_ty fmt ty)

  let show_ty : type a. a ty -> string = (fun x -> Format.asprintf "%a" pp_ty x)

  let show_decl_ty x = show_ty x

  let show_locl_ty x = show_ty x

  let show_type_predicate_ predicate_ =
    Format.asprintf "%a" pp_type_predicate_ predicate_
end

include Pp

(* Constructor and deconstructor functions for types and constraint types.
 * Abstracting these lets us change the implementation, e.g. hash cons
 *)
let mk p = p

let deref p = p

let get_reason (r, _) = r

let get_node (_, n) = n

let ty_con_ordinal_ : type a. a ty_ -> int = function
  (* only decl constructors *)
  | Tthis -> 100
  | Tapply _ -> 101
  | Tmixed -> 102
  | Tlike _ -> 103
  | Trefinement _ -> 104
  | Twildcard -> 105
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
  | Tclass_ptr _ -> 26
  (* only locl constructors *)
  | Tnewtype _ -> 201
  | Tdependent _ -> 202
  | Tclass _ -> 204
  | Tneg _ -> 205
  | Tlabel _ -> 206

let same_type_origin (orig1 : type_origin) orig2 =
  match orig1 with
  | Missing_origin -> false
  | _ -> equal_type_origin orig1 orig2

let type_predicate__con_ordinal type_predicate_ =
  match type_predicate_ with
  | IsTag _ -> 0
  | IsTupleOf _ -> 1
  | IsShapeOf _ -> 2

let type_tag_con_ordinal type_tag =
  match type_tag with
  | BoolTag -> 0
  | IntTag -> 1
  | StringTag -> 2
  | ArraykeyTag -> 3
  | FloatTag -> 4
  | NumTag -> 5
  | ResourceTag -> 6
  | NullTag -> 7
  | ClassTag _ -> 8

let chain_compare comp cont =
  match comp with
  | 0 -> cont ()
  | n -> n

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
      chain_compare
        (String.compare (snd id1) (snd id2))
        (fun _ -> tyl_compare ~sort:normalize_lists ~normalize_lists tyl1 tyl2)
    end
    | (Trefinement (ty1, r1), Trefinement (ty2, r2)) -> begin
      chain_compare (ty_compare ty1 ty2) (fun _ ->
          class_refinement_compare r1 r2)
    end
    | (Tmixed, Tmixed) -> 0
    | (Twildcard, Twildcard) -> 0
    | (Tlike ty1, Tlike ty2) -> ty_compare ty1 ty2
    | ((Tthis | Tapply _ | Tmixed | Twildcard | Tlike _), _)
    | (_, (Tthis | Tapply _ | Tmixed | Twildcard | Tlike _)) ->
      ty_con_ordinal_ ty_1 - ty_con_ordinal_ ty_2
    (* Both or in Localized Phase *)
    | (Tprim ty1, Tprim ty2) -> Aast_defs.compare_tprim ty1 ty2
    | (Toption ty, Toption ty2) -> ty_compare ty ty2
    | (Tvec_or_dict (tk, tv), Tvec_or_dict (tk2, tv2)) -> begin
      chain_compare (ty_compare tk tk2) (fun _ -> ty_compare tv tv2)
    end
    | (Tfun fty, Tfun fty2) -> tfun_compare fty fty2
    | (Tunion tyl1, Tunion tyl2)
    | (Tintersection tyl1, Tintersection tyl2) ->
      tyl_compare ~sort:normalize_lists ~normalize_lists tyl1 tyl2
    | (Ttuple t1, Ttuple t2) -> tuple_type_compare t1 t2
    | (Tgeneric n1, Tgeneric n2) -> String.compare n1 n2
    | (Tnewtype (id, tyl, cstr1), Tnewtype (id2, tyl2, cstr2)) -> begin
      chain_compare (String.compare id id2) (fun _ ->
          chain_compare (tyl_compare ~sort:false tyl tyl2) (fun _ ->
              ty_compare cstr1 cstr2))
    end
    | (Tdependent (d1, cstr1), Tdependent (d2, cstr2)) -> begin
      chain_compare (compare_dependent_type d1 d2) (fun _ ->
          ty_compare cstr1 cstr2)
    end
    (* An instance of a class or interface, ty list are the arguments *)
    | (Tclass (id, exact, tyl), Tclass (id2, exact2, tyl2)) -> begin
      chain_compare
        (String.compare (snd id) (snd id2))
        (fun _ ->
          chain_compare (tyl_compare ~sort:false tyl tyl2) (fun _ ->
              exact_compare exact exact2))
    end
    | (Tshape s1, Tshape s2) -> shape_type_compare s1 s2
    | (Tvar v1, Tvar v2) -> Tvid.compare v1 v2
    | (Taccess (ty1, id1), Taccess (ty2, id2)) -> begin
      chain_compare (ty_compare ty1 ty2) (fun _ ->
          String.compare (snd id1) (snd id2))
    end
    | (Tneg neg1, Tneg neg2) -> compare_type_predicate neg1 neg2
    | (Tnonnull, Tnonnull) -> 0
    | (Tdynamic, Tdynamic) -> 0
    | (Tlabel name1, Tlabel name2) -> String.compare name1 name2
    | (Tclass_ptr ty1, Tclass_ptr ty2) -> ty_compare ty1 ty2
    | ( ( Tprim _ | Toption _ | Tvec_or_dict _ | Tfun _ | Tintersection _
        | Tunion _ | Ttuple _ | Tgeneric _ | Tnewtype _ | Tdependent _
        | Tclass _ | Tshape _ | Tvar _ | Tnonnull | Tdynamic | Taccess _
        | Tany _ | Tneg _ | Trefinement _ | Tlabel _ | Tclass_ptr _ ),
        _ ) ->
      ty_con_ordinal_ ty_1 - ty_con_ordinal_ ty_2
  and shape_field_type_compare :
      type a. a shape_field_type -> a shape_field_type -> int =
   fun sft1 sft2 ->
    let { sft_ty = ty1; sft_optional = optional1 } = sft1 in
    let { sft_ty = ty2; sft_optional = optional2 } = sft2 in
    chain_compare (ty_compare ty1 ty2) (fun _ ->
        Bool.compare optional1 optional2)
  and shape_type_compare : type a. a shape_type -> a shape_type -> int =
   fun s1 s2 ->
    let {
      s_origin = shape_origin1;
      s_unknown_value = unknown_fields_type1;
      s_fields = fields1;
    } =
      s1
    in
    let {
      s_origin = shape_origin2;
      s_unknown_value = unknown_fields_type2;
      s_fields = fields2;
    } =
      s2
    in
    if same_type_origin shape_origin1 shape_origin2 then
      0
    else begin
      chain_compare
        (ty_compare unknown_fields_type1 unknown_fields_type2)
        (fun _ ->
          List.compare
            (fun (k1, v1) (k2, v2) ->
              chain_compare (TShapeField.compare k1 k2) (fun _ ->
                  shape_field_type_compare v1 v2))
            (TShapeMap.elements fields1)
            (TShapeMap.elements fields2))
    end
  and tuple_extra_compare : type a. a tuple_extra -> a tuple_extra -> int =
   fun t1 t2 ->
    match (t1, t2) with
    | (Textra _, Tsplat _) -> -1
    | (Tsplat _, Textra _) -> 1
    | (Tsplat t_splat1, Tsplat t_splat2) -> ty_compare t_splat1 t_splat2
    | ( Textra { t_optional = t_optional1; t_variadic = t_variadic1 },
        Textra { t_optional = t_optional2; t_variadic = t_variadic2 } ) ->
      chain_compare (ty_compare t_variadic1 t_variadic2) (fun _ ->
          List.compare ty_compare t_optional1 t_optional2)
  and tuple_type_compare : type a. a tuple_type -> a tuple_type -> int =
   fun t1 t2 ->
    let { t_required = t_required1; t_extra = t_extra1 } = t1 in
    let { t_required = t_required2; t_extra = t_extra2 } = t2 in
    chain_compare (List.compare ty_compare t_required1 t_required2) (fun _ ->
        tuple_extra_compare t_extra1 t_extra2)
  and user_attribute_param_compare p1 p2 =
    let dest_user_attribute_param p =
      match p with
      | Classname s -> (0, s)
      | EnumClassLabel s -> (1, s)
      | String s -> (2, s)
      | Int i -> (3, i)
    in
    let (id1, s1) = dest_user_attribute_param p1 in
    let (id2, s2) = dest_user_attribute_param p2 in
    chain_compare (Int.compare id1 id2) (fun _ -> String.compare s1 s2)
  and user_attribute_compare ua1 ua2 =
    let { ua_name = name1; ua_params = params1; _ } = ua1 in
    let { ua_name = name2; ua_params = params2; _ } = ua2 in
    chain_compare
      (String.compare (snd name1) (snd name2))
      (fun _ -> List.compare user_attribute_param_compare params1 params2)
  and user_attributes_compare ual1 ual2 =
    List.compare user_attribute_compare ual1 ual2
  and tparam_compare : type a. a ty tparam -> a ty tparam -> int =
   fun tp1 tp2 ->
    let {
      (* Type parameters on functions are always marked invariant *)
      tp_variance = _;
      tp_name = name1;
      tp_constraints = constraints1;
      tp_reified = reified1;
      tp_user_attributes = user_attributes1;
    } =
      tp1
    in
    let {
      tp_variance = _;
      tp_name = name2;
      tp_constraints = constraints2;
      tp_reified = reified2;
      tp_user_attributes = user_attributes2;
    } =
      tp2
    in
    chain_compare
      (String.compare (snd name1) (snd name2))
      (fun _ ->
        chain_compare (constraints_compare constraints1 constraints2) (fun _ ->
            chain_compare
              (user_attributes_compare user_attributes1 user_attributes2)
              (fun _ -> Aast_defs.compare_reify_kind reified1 reified2)))
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
    chain_compare (Ast_defs.compare_constraint_kind ck1 ck2) (fun _ ->
        ty_compare ty1 ty2)
  and where_constraint_compare :
      type a b.
      a ty * Ast_defs.constraint_kind * b ty ->
      a ty * Ast_defs.constraint_kind * b ty ->
      int =
   fun (ty1a, ck1, ty1b) (ty2a, ck2, ty2b) ->
    chain_compare (Ast_defs.compare_constraint_kind ck1 ck2) (fun _ ->
        chain_compare (ty_compare ty1a ty2a) (fun _ -> ty_compare ty1b ty2b))
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
      ft_tparams = tparams1;
      ft_where_constraints = where_constraints1;
      ft_cross_package = cross_package1;
      ft_instantiated = inst1;
    } =
      fty1
    in
    let {
      ft_ret = ret2;
      ft_params = params2;
      ft_flags = flags2;
      ft_implicit_params = implicit_params2;
      ft_tparams = tparams2;
      ft_where_constraints = where_constraints2;
      ft_cross_package = cross_package2;
      ft_instantiated = inst2;
    } =
      fty2
    in
    chain_compare (ty_compare ret1 ret2) (fun _ ->
        chain_compare (ft_params_compare params1 params2) (fun _ ->
            chain_compare (tparams_compare tparams1 tparams2) (fun _ ->
                chain_compare
                  (where_constraints_compare
                     where_constraints1
                     where_constraints2)
                  (fun _ ->
                    chain_compare
                      (Typing_defs_flags.Fun.compare flags1 flags2)
                      (fun _ ->
                        let { capability = capability1 } = implicit_params1 in
                        let { capability = capability2 } = implicit_params2 in
                        chain_compare
                          (capability_compare capability1 capability2)
                          (fun _ ->
                            chain_compare
                              (compare_cross_package_decl
                                 cross_package1
                                 cross_package2)
                              (fun _ -> Bool.compare inst1 inst2)))))))
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

and compare_type_predicate (_, p1) (_, p2) =
  match (p1, p2) with
  | (IsTag tag1, IsTag tag2) -> compare_type_tag tag1 tag2
  | (IsTupleOf tp1, IsTupleOf tp2) -> compare_tuple_predicate tp1 tp2
  | (IsShapeOf sp1, IsShapeOf sp2) -> compare_shape_predicate sp1 sp2
  | _ -> type_predicate__con_ordinal p1 - type_predicate__con_ordinal p2

and compare_type_tag_generic g1 g2 =
  match (g1, g2) with
  | (Wildcard id1, Wildcard id2) -> Int.compare id1 id2
  | (Filled ty1, Filled ty2) -> ty_compare ty1 ty2
  | (Wildcard _, Filled _) -> 1
  | (Filled _, Wildcard _) -> -1

and compare_type_tag tag1 tag2 =
  match (tag1, tag2) with
  | (BoolTag, BoolTag)
  | (IntTag, IntTag)
  | (StringTag, StringTag)
  | (ArraykeyTag, ArraykeyTag)
  | (FloatTag, FloatTag)
  | (NumTag, NumTag)
  | (ResourceTag, ResourceTag)
  | (NullTag, NullTag) ->
    0
  | (ClassTag (id1, args1), ClassTag (id2, args2)) ->
    chain_compare (String.compare id1 id2) (fun _ ->
        List.compare compare_type_tag_generic args1 args2)
  | _ -> type_tag_con_ordinal tag1 - type_tag_con_ordinal tag2

and compare_tuple_predicate tp1 tp2 =
  List.compare compare_type_predicate tp1.tp_required tp2.tp_required

and compare_shape_predicate sp1 sp2 =
  TShapeMap.compare compare_shape_field_predicate sp1.sp_fields sp2.sp_fields

and compare_shape_field_predicate { sfp_predicate = p1 } { sfp_predicate = p2 }
    =
  compare_type_predicate p1 p2

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

and ft_param_compare :
    type a. ?normalize_lists:bool -> a ty fun_param -> a ty fun_param -> int =
 fun ?(normalize_lists = false) param1 param2 ->
  chain_compare
    (ty_compare ~normalize_lists param1.fp_type param2.fp_type)
    (fun _ ->
      Typing_defs_flags.FunParam.compare param1.fp_flags param2.fp_flags)

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
  | (TRloose b1, TRloose b2) ->
    chain_compare (tyl_compare ~sort:true b1.tr_lower b2.tr_lower) (fun _ ->
        tyl_compare ~sort:true b1.tr_upper b2.tr_upper)
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

let exact_equal e1 e2 = Int.equal 0 (exact_compare e1 e2)

type decl_ty_ = decl_phase ty_

let equal_decl_ty_ : decl_ty_ -> decl_ty_ -> bool =
 (fun ty1 ty2 -> Int.equal 0 (ty__compare ty1 ty2))

let equal_decl_ty ty1 ty2 = equal_decl_ty_ (get_node ty1) (get_node ty2)

let equal_shape_field_type (sft1 : decl_phase shape_field_type) sft2 =
  let { sft_ty; sft_optional } = sft1 in
  equal_decl_ty sft_ty sft2.sft_ty && Bool.equal sft_optional sft2.sft_optional

type locl_ty_ = locl_phase ty_

type decl_tparam = decl_ty tparam [@@deriving eq, show]

type locl_tparam = locl_ty tparam

type decl_where_constraint = decl_ty where_constraint [@@deriving eq, show]

type locl_where_constraint = locl_ty where_constraint

type decl_fun_type = decl_ty fun_type [@@deriving eq]

type locl_fun_type = locl_ty fun_type

type decl_fun_param = decl_ty fun_param [@@deriving eq]

type locl_fun_param = locl_ty fun_param

type decl_fun_params = decl_ty fun_params [@@deriving eq]

type locl_fun_params = locl_ty fun_params

type decl_class_refinement = decl_phase class_refinement

type locl_class_refinement = locl_phase class_refinement

type decl_refined_const = decl_phase refined_const

type locl_refined_const = locl_phase refined_const

type destructure_kind =
  | ListDestructure
  | SplatUnpack
[@@deriving eq, ord, show]

type destructure = {
  d_required: locl_ty list;
      (** This represents the standard parameters of a function or the fields in a list
       * destructuring assignment. Example:
       *
       * function take(bool $b, float $f = 3.14, arraykey ...$aks): void {}
       * function f((bool, float, int, string) $tup): void {
       *   take(...$tup);
       * }
       *
       * corresponds to the subtyping assertion
       *
       * (bool, float, int, string) <: splat([#1], [opt#2], ...#3)
       *)
  d_optional: locl_ty list;
      (** Represents the optional parameters in a function, only used for splats *)
  d_variadic: locl_ty option;
      (** Represents a function's variadic parameter, also only used for splats *)
  d_kind: destructure_kind;
      (** list() destructuring allows for partial matches on lists, even when the operation
       * might throw i.e. list($a) = vec[]; *)
}
[@@deriving show]

type has_member = {
  hm_name: Nast.sid;
  hm_type: locl_ty;
  hm_class_id: Nast.class_id_; [@opaque]
      (** This is required to check ambiguous object access, where sometimes
  HHVM would access the private member of a parent class instead of the
  one from the current class. *)
  hm_explicit_targs: Nast.targ list option; [@opaque]
      (* - For a "has-property" constraint, this is `None`
       * - For a "has-method" constraint, this is `Some targs`, where targs
       *   is the list of explicit type arguments provided to the method call.
       *   Note that this list can be empty (i.e. `Some []`) in the case of a
       *   method not taking type arguments, or when we leave them implicit
       *
       * We need to know if this is a "has-property" or "has-method" to pass
       * the correct `is_method` parameter to `Typing_object_get.obj_get`.
       *)
}
[@@deriving show]

type can_index = {
  ci_key: locl_ty;
  ci_val: locl_ty;
  ci_index_expr: Nast.expr;
  ci_lhs_of_null_coalesce: bool;
  ci_expr_pos: Pos.t;
  ci_array_pos: Pos.t;
  ci_index_pos: Pos.t;
}
[@@deriving show]

type can_traverse = {
  ct_key: locl_ty option;
  ct_val: locl_ty;
  ct_is_await: bool;
  ct_reason: Reason.t;
}
[@@deriving show]

type has_type_member = {
  htm_id: string;
  htm_lower: locl_ty;
  htm_upper: locl_ty;
}
[@@deriving show]

type constraint_type_ =
  | Thas_member of has_member
  | Thas_type_member of has_type_member
  | Thas_const of {
      name: string;
      ty: locl_ty;
    }
      (** Check if the given type has a class constant that is compatible with [ty] *)
  | Tcan_index of can_index
  | Tcan_traverse of can_traverse
  | Tdestructure of destructure
      (** The type of container destructuring via list() or splat `...` *)
  | Ttype_switch of {
      predicate: type_predicate;
      ty_true: locl_ty;
      ty_false: locl_ty;
    }

and constraint_type = Reason.t * constraint_type_ [@@deriving show]

type internal_type =
  | LoclType of locl_ty
  | ConstraintType of constraint_type
[@@deriving show]

let map_reason (r, ty) ~(f : _ Reason.t_ -> _ Reason.t_) = (f r, ty)

let map_ty : type ph. ph ty -> f:(ph ty_ -> ph ty_) -> ph ty =
 (fun (r, ty) ~(f : _ ty_ -> _ ty_) -> (r, f ty))

let with_reason ty r = map_reason ty ~f:(fun _r -> r)

let get_pos t = Reason.to_pos (get_reason t)

let mk_constraint_type p = p

let deref_constraint_type p = p

let get_reason_i : internal_type -> Reason.t = function
  | LoclType lty -> get_reason lty
  | ConstraintType (r, _) -> r

(** Hack keyword for this visibility *)
let string_of_visibility : ce_visibility -> string = function
  | Vpublic -> "public"
  | Vprivate _ -> "private"
  | Vprotected _ -> "protected"
  | Vinternal _ -> "internal"
  | Vprotected_internal _ -> "protected internal"

(* Dedicated functions with more easily discoverable names *)
let compare_locl_ty : ?normalize_lists:bool -> locl_ty -> locl_ty -> int =
  ty_compare

let compare_decl_ty : ?normalize_lists:bool -> decl_ty -> decl_ty -> int =
  ty_compare

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
  chain_compare (String.compare m1 m2) (fun _ ->
      chain_compare (ty_compare ty1 ty2) (fun _ ->
          chain_compare (class_id_compare cid1 cid2) (fun _ ->
              Option.compare (List.compare targ_compare) targs1 targs2)))

let can_index_compare ~normalize_lists ci1 ci2 =
  (* not comparing the ast because it is decided by expr_pos *)
  chain_compare (ty_compare ~normalize_lists ci1.ci_key ci2.ci_key) (fun _ ->
      chain_compare
        (ty_compare ~normalize_lists ci1.ci_val ci2.ci_val)
        (fun _ ->
          chain_compare
            (Bool.compare
               ci1.ci_lhs_of_null_coalesce
               ci2.ci_lhs_of_null_coalesce)
            (fun _ ->
              chain_compare
                (Aast.compare_pos ci1.ci_expr_pos ci2.ci_expr_pos)
                (fun _ ->
                  chain_compare
                    (Aast.compare_pos ci1.ci_array_pos ci2.ci_array_pos)
                    (fun _ ->
                      Aast.compare_pos ci1.ci_index_pos ci2.ci_index_pos)))))

let can_traverse_compare ~normalize_lists ct1 ct2 =
  chain_compare
    (Option.compare (ty_compare ~normalize_lists) ct1.ct_key ct2.ct_key)
    (fun _ ->
      chain_compare
        (ty_compare ~normalize_lists ct1.ct_val ct2.ct_val)
        (fun _ -> Bool.compare ct1.ct_is_await ct2.ct_is_await))

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
  chain_compare (tyl_compare ~normalize_lists ~sort:false tyl1 tyl2) (fun _ ->
      chain_compare
        (tyl_compare ~normalize_lists ~sort:false tyl_opt1 tyl_opt2)
        (fun _ ->
          chain_compare (Option.compare ty_compare ty_opt1 ty_opt2) (fun _ ->
              compare_destructure_kind e1 e2)))

let constraint_ty_con_ordinal cty =
  match cty with
  | Thas_member _ -> 0
  | Tdestructure _ -> 1
  | Tcan_index _ -> 2
  | Tcan_traverse _ -> 3
  | Thas_type_member _ -> 4
  | Ttype_switch _ -> 5
  | Thas_const _ -> 6

let constraint_ty_compare ?(normalize_lists = false) ty1 ty2 =
  let (_, ty1) = deref_constraint_type ty1 in
  let (_, ty2) = deref_constraint_type ty2 in
  match (ty1, ty2) with
  | (Thas_member hm1, Thas_member hm2) ->
    has_member_compare ~normalize_lists hm1 hm2
  | (Thas_type_member htm1, Thas_type_member htm2) ->
    let { htm_id = id1; htm_lower = lower1; htm_upper = upper1 } = htm1
    and { htm_id = id2; htm_lower = lower2; htm_upper = upper2 } = htm2 in
    chain_compare (String.compare id1 id2) (fun _ ->
        chain_compare (ty_compare lower1 lower2) (fun _ ->
            ty_compare upper1 upper2))
  | (Tcan_index ci1, Tcan_index ci2) ->
    can_index_compare ~normalize_lists ci1 ci2
  | (Tcan_traverse ct1, Tcan_traverse ct2) ->
    can_traverse_compare ~normalize_lists ct1 ct2
  | (Tdestructure d1, Tdestructure d2) ->
    destructure_compare ~normalize_lists d1 d2
  | ( Ttype_switch
        { predicate = predicate1; ty_true = ty_true1; ty_false = ty_false1 },
      Ttype_switch
        { predicate = predicate2; ty_true = ty_true2; ty_false = ty_false2 } )
    ->
    chain_compare (compare_type_predicate predicate1 predicate2) (fun _ ->
        chain_compare (ty_compare ~normalize_lists ty_true1 ty_true2) (fun _ ->
            ty_compare ~normalize_lists ty_false1 ty_false2))
  | ( Thas_const { name = name1; ty = ty1 },
      Thas_const { name = name2; ty = ty2 } ) ->
    chain_compare (String.compare name1 name2) (fun _ ->
        ty_compare ~normalize_lists ty1 ty2)
  | ( _,
      ( Thas_member _ | Tcan_index _ | Tcan_traverse _ | Tdestructure _
      | Thas_type_member _ | Ttype_switch _ | Thas_const _ ) ) ->
    constraint_ty_con_ordinal ty2 - constraint_ty_con_ordinal ty1

let constraint_ty_equal ?(normalize_lists = false) ty1 ty2 =
  Int.equal (constraint_ty_compare ~normalize_lists ty1 ty2) 0

let ty_equal ?(normalize_lists = false) ty1 ty2 =
  phys_equal (get_node ty1) (get_node ty2)
  || Int.equal 0 (ty_compare ~normalize_lists ty1 ty2)

let equal_locl_ty : locl_ty -> locl_ty -> bool =
 (fun ty1 ty2 -> ty_equal ty1 ty2)

let equal_locl_ty_ : locl_ty_ -> locl_ty_ -> bool =
 (fun ty_1 ty_2 -> Int.equal 0 (ty__compare ty_1 ty_2))

let equal_decl_tyl tyl1 tyl2 = List.equal equal_decl_ty tyl1 tyl2

(** Ignore position and reason info. *)
let equal_internal_type ty1 ty2 =
  match (ty1, ty2) with
  | (LoclType ty1, LoclType ty2) -> ty_equal ~normalize_lists:true ty1 ty2
  | (ConstraintType ty1, ConstraintType ty2) ->
    constraint_ty_equal ~normalize_lists:true ty1 ty2
  | (_, (LoclType _ | ConstraintType _)) -> false

let equal_type_predicate p1 p2 = Int.equal 0 (compare_type_predicate p1 p2)

module Locl_subst = struct
  type t = locl_ty SMap.t

  let rec apply_ty (ty : locl_ty) ~subst ~combine_reasons =
    match deref ty with
    | (reason_src, Tgeneric nm) ->
      (match SMap.find_opt nm subst with
      | Some ty_subst ->
        map_reason ty_subst ~f:(fun reason_dest ->
            combine_reasons ~src:reason_src ~dest:reason_dest)
      | _ -> ty)
    | (r, Toption ty) -> mk (r, Toption (apply_ty ty ~subst ~combine_reasons))
    | (r, Tclass_ptr ty) ->
      mk (r, Tclass_ptr (apply_ty ty ~subst ~combine_reasons))
    | (r, Taccess (ty, pos_id)) ->
      mk (r, Taccess (apply_ty ty ~subst ~combine_reasons, pos_id))
    | (r, Tvec_or_dict (ty_k, ty_v)) ->
      mk
        ( r,
          Tvec_or_dict
            ( apply_ty ty_k ~subst ~combine_reasons,
              apply_ty ty_v ~subst ~combine_reasons ) )
    | (r, Ttuple tuple_ty) ->
      mk (r, Ttuple (apply_tuple tuple_ty ~subst ~combine_reasons))
    | (r, Tunion tys) ->
      mk (r, Tunion (List.map tys ~f:(apply_ty ~subst ~combine_reasons)))
    | (r, Tintersection tys) ->
      mk (r, Tintersection (List.map tys ~f:(apply_ty ~subst ~combine_reasons)))
    | (r, Tfun fun_ty) -> mk (r, Tfun (apply_fun fun_ty ~subst ~combine_reasons))
    | (r, Tshape shape_ty) ->
      mk (r, Tshape (apply_shape shape_ty ~subst ~combine_reasons))
    | (r, Tnewtype (nm, ty_params, ty_bound)) ->
      mk
        ( r,
          Tnewtype
            ( nm,
              List.map ty_params ~f:(apply_ty ~subst ~combine_reasons),
              apply_ty ty_bound ~subst ~combine_reasons ) )
    | (r, Tdependent (dep_ty, ty)) ->
      mk (r, Tdependent (dep_ty, apply_ty ty ~subst ~combine_reasons))
    | (r, Tclass (id, exact, tys)) ->
      mk
        ( r,
          Tclass
            ( id,
              apply_exact exact ~subst ~combine_reasons,
              List.map tys ~f:(apply_ty ~subst ~combine_reasons) ) )
    | (_, (Tvar _ | Tany _ | Tnonnull | Tdynamic | Tprim _ | Tlabel _ | Tneg _))
      ->
      ty

  and apply_tuple { t_required; t_extra } ~subst ~combine_reasons =
    let t_required = List.map t_required ~f:(apply_ty ~subst ~combine_reasons)
    and t_extra =
      match t_extra with
      | Tsplat ty -> Tsplat (apply_ty ty ~subst ~combine_reasons)
      | Textra { t_optional; t_variadic } ->
        let t_optional =
          List.map t_optional ~f:(apply_ty ~subst ~combine_reasons)
        and t_variadic = apply_ty t_variadic ~subst ~combine_reasons in
        Textra { t_optional; t_variadic }
    in
    { t_required; t_extra }

  and apply_fun
      ({
         ft_tparams;
         ft_where_constraints;
         ft_params;
         ft_implicit_params;
         ft_ret;
         _;
       } as fun_type)
      ~subst
      ~combine_reasons =
    let ft_tparams =
      List.map ft_tparams ~f:(apply_tparam ~subst ~combine_reasons)
    and ft_params =
      List.map ft_params ~f:(apply_fun_param ~subst ~combine_reasons)
    and ft_implicit_params =
      apply_fun_implicit_params ft_implicit_params ~subst ~combine_reasons
    and ft_where_constraints =
      List.map
        ft_where_constraints
        ~f:(apply_where_constraint ~subst ~combine_reasons)
    and ft_ret = apply_ty ft_ret ~subst ~combine_reasons in
    {
      fun_type with
      ft_tparams;
      ft_where_constraints;
      ft_params;
      ft_implicit_params;
      ft_ret;
    }

  and apply_tparam ({ tp_constraints; _ } as tparam) ~subst ~combine_reasons =
    let tp_constraints =
      List.map tp_constraints ~f:(fun (cstr_kind, ty) ->
          (cstr_kind, apply_ty ty ~subst ~combine_reasons))
    in
    { tparam with tp_constraints }

  and apply_fun_param ({ fp_type; _ } as fun_param) ~subst ~combine_reasons =
    let fp_type = apply_ty fp_type ~subst ~combine_reasons in
    { fun_param with fp_type }

  and apply_fun_implicit_params ({ capability } as t) ~subst ~combine_reasons =
    match capability with
    | CapDefaults _ -> t
    | CapTy ty -> { capability = CapTy (apply_ty ty ~subst ~combine_reasons) }

  and apply_where_constraint (ty1, cstr_kind, ty2) ~subst ~combine_reasons =
    ( apply_ty ty1 ~subst ~combine_reasons,
      cstr_kind,
      apply_ty ty2 ~subst ~combine_reasons )

  and apply_shape
      { s_origin; s_unknown_value; s_fields } ~subst ~combine_reasons =
    let s_unknown_value = apply_ty s_unknown_value ~subst ~combine_reasons
    and s_fields =
      TShapeMap.map (apply_shape_field ~subst ~combine_reasons) s_fields
    in
    { s_origin; s_unknown_value; s_fields }

  and apply_shape_field { sft_optional; sft_ty } ~subst ~combine_reasons =
    let sft_ty = apply_ty sft_ty ~subst ~combine_reasons in
    { sft_optional; sft_ty }

  and apply_exact exact ~subst ~combine_reasons =
    match exact with
    | Exact -> exact
    | Nonexact { cr_consts } ->
      let cr_consts =
        SMap.map (apply_refined_const ~subst ~combine_reasons) cr_consts
      in
      Nonexact { cr_consts }

  and apply_refined_const { rc_bound; rc_is_ctx } ~subst ~combine_reasons =
    let rc_bound =
      match rc_bound with
      | TRexact ty -> TRexact (apply_ty ty ~subst ~combine_reasons)
      | TRloose { tr_lower; tr_upper } ->
        let tr_lower = List.map tr_lower ~f:(apply_ty ~subst ~combine_reasons)
        and tr_upper =
          List.map tr_upper ~f:(apply_ty ~subst ~combine_reasons)
        in
        TRloose { tr_lower; tr_upper }
    in
    { rc_bound; rc_is_ctx }

  let apply ty ~subst ~combine_reasons =
    (* Avoid a pointless traversal *)
    if SMap.is_empty subst then
      ty
    else
      apply_ty ty ~subst ~combine_reasons
end

module Find_locl = struct
  let rec find_ty (ty : locl_ty) ~p =
    if p ty then
      Some ty
    else begin
      match get_node ty with
      | Toption ty
      | Tclass_ptr ty
      | Taccess (ty, _)
      | Tdependent (_, ty) ->
        find_ty ty ~p
      | Tunion tys
      | Tintersection tys ->
        find_first_ty tys ~p
      | Tvec_or_dict (ty_k, ty_v) -> begin
        match find_ty ty_k ~p with
        | None -> find_ty ty_v ~p
        | res -> res
      end
      | Tclass (_, exact, ty_args) -> begin
        match find_exact exact ~p with
        | None -> find_first_ty ty_args ~p
        | res -> res
      end
      | Tnewtype (_, ty_args, ty_bound) -> begin
        match find_first_ty ty_args ~p with
        | None -> find_ty ty_bound ~p
        | res -> res
      end
      | Ttuple tuple_ty -> find_tuple tuple_ty ~p
      | Tfun fun_ty -> find_fun fun_ty ~p
      | Tshape shape_ty -> find_shape shape_ty ~p
      | Tgeneric _
      | Tvar _
      | Tany _
      | Tnonnull
      | Tdynamic
      | Tprim _
      | Tlabel _
      | Tneg _ ->
        None
    end

  and find_first_ty tys ~p =
    match tys with
    | [] -> None
    | ty :: _ when p ty -> Some ty
    | _ :: tys -> find_first_ty tys ~p

  and find_tuple { t_required; t_extra } ~p =
    match find_first_ty t_required ~p with
    | None -> begin
      match t_extra with
      | Tsplat ty -> find_ty ty ~p
      | Textra { t_optional; t_variadic } -> begin
        match find_first_ty t_optional ~p with
        | None -> find_ty t_variadic ~p
        | res -> res
      end
    end
    | res -> res

  and find_fun
      {
        ft_tparams;
        ft_where_constraints;
        ft_params;
        ft_implicit_params;
        ft_ret;
        _;
      }
      ~p =
    match find_first_tparam ft_tparams ~p with
    | None -> begin
      match find_first_fun_param ft_params ~p with
      | None -> begin
        match find_implicit_params ft_implicit_params ~p with
        | None -> begin
          match find_first_where_constraint ft_where_constraints ~p with
          | None -> find_ty ft_ret ~p
          | res -> res
        end
        | res -> res
      end
      | res -> res
    end
    | res -> res

  and find_first_tparam tparams ~p =
    match tparams with
    | [] -> None
    | tparam :: tparams -> begin
      match find_tparam tparam ~p with
      | None -> find_first_tparam tparams ~p
      | res -> res
    end

  and find_tparam { tp_constraints; _ } ~p =
    find_first_constraint tp_constraints ~p

  and find_first_constraint cstrs ~p =
    match cstrs with
    | [] -> None
    | (_, ty) :: _ when p ty -> Some ty
    | _ :: cstrs -> find_first_constraint cstrs ~p

  and find_first_fun_param fun_params ~p =
    match fun_params with
    | [] -> None
    | { fp_type; _ } :: _ when p fp_type -> Some fp_type
    | _ :: cstrs -> find_first_fun_param cstrs ~p

  and find_implicit_params { capability } ~p =
    match capability with
    | CapDefaults _ -> None
    | CapTy ty -> find_ty ty ~p

  and find_first_where_constraint cstrs ~p =
    match cstrs with
    | [] -> None
    | cstr :: cstrs -> begin
      match find_where_constraint cstr ~p with
      | None -> find_first_where_constraint cstrs ~p
      | res -> res
    end

  and find_where_constraint (ty1, _, ty2) ~p =
    match find_ty ty1 ~p with
    | None -> find_ty ty2 ~p
    | res -> res

  and find_shape { s_unknown_value; s_fields; _ } ~p =
    match find_ty s_unknown_value ~p with
    | None -> find_first_shape_field (TShapeMap.bindings s_fields) ~p
    | res -> res

  and find_first_shape_field fields ~p =
    match fields with
    | [] -> None
    | (_, { sft_ty; _ }) :: fields -> begin
      match find_ty sft_ty ~p with
      | None -> find_first_shape_field fields ~p
      | res -> res
    end

  and find_exact exact ~p =
    match exact with
    | Exact -> None
    | Nonexact { cr_consts } ->
      find_first_refined_const (SMap.bindings cr_consts) ~p

  and find_first_refined_const cr_consts ~p =
    match cr_consts with
    | [] -> None
    | (_, next) :: rest -> begin
      match find_refined_const next ~p with
      | None -> find_first_refined_const rest ~p
      | res -> res
    end

  and find_refined_const { rc_bound; _ } ~p =
    match rc_bound with
    | TRexact ty -> find_ty ty ~p
    | TRloose { tr_lower; tr_upper } -> begin
      match find_first_ty tr_lower ~p with
      | None -> find_first_ty tr_upper ~p
      | res -> res
    end
end

let find_locl_ty locl_ty ~p = Find_locl.find_ty locl_ty ~p

module Transform_top_down_decl = struct
  let rec transform ty ~on_ty ~on_rc_bound ~ctx =
    match on_ty ty ~ctx with
    | (_, `Stop ty) -> ty
    | (ctx, `Continue ty) -> traverse ty ~on_ty ~on_rc_bound ~ctx
    | (ctx, `Restart ty) -> transform ty ~on_ty ~on_rc_bound ~ctx

  and traverse ty ~on_ty ~on_rc_bound ~ctx =
    match deref ty with
    | ( _,
        ( Tthis | Tmixed | Twildcard | Tany _ | Tnonnull | Tdynamic | Tprim _
        | Tgeneric _ ) ) ->
      ty
    | (r, Tlike ty) -> mk (r, Tlike (transform ty ~on_ty ~on_rc_bound ~ctx))
    | (r, Toption ty) -> mk (r, Toption (transform ty ~on_ty ~on_rc_bound ~ctx))
    | (r, Tclass_ptr ty) ->
      mk (r, Tclass_ptr (transform ty ~on_ty ~on_rc_bound ~ctx))
    | (r, Taccess (ty, id)) ->
      mk (r, Taccess (transform ty ~on_ty ~on_rc_bound ~ctx, id))
    | (r, Tvec_or_dict (ty_k, ty_v)) ->
      mk
        ( r,
          Tvec_or_dict
            ( transform ty_k ~on_ty ~on_rc_bound ~ctx,
              transform ty_v ~on_ty ~on_rc_bound ~ctx ) )
    | (r, Tunion tys) ->
      mk (r, Tunion (List.map tys ~f:(transform ~on_ty ~on_rc_bound ~ctx)))
    | (r, Tintersection tys) ->
      mk
        (r, Tintersection (List.map tys ~f:(transform ~on_ty ~on_rc_bound ~ctx)))
    | (r, Tapply (id, tys)) ->
      mk (r, Tapply (id, List.map tys ~f:(transform ~on_ty ~on_rc_bound ~ctx)))
    | (r, Trefinement (ty, class_refinement)) ->
      mk
        ( r,
          Trefinement
            ( transform ty ~on_ty ~on_rc_bound ~ctx,
              traverse_class_refinement
                class_refinement
                ~on_ty
                ~on_rc_bound
                ~ctx ) )
    | (r, Tfun fun_ty) ->
      mk (r, Tfun (traverse_fun_ty fun_ty ~on_ty ~on_rc_bound ~ctx))
    | (r, Ttuple tuple_ty) ->
      mk (r, Ttuple (traverse_tuple_ty tuple_ty ~on_ty ~on_rc_bound ~ctx))
    | (r, Tshape shape_ty) ->
      mk (r, Tshape (traverse_shape_ty shape_ty ~on_ty ~on_rc_bound ~ctx))

  and traverse_class_refinement { cr_consts } ~on_ty ~on_rc_bound ~ctx =
    {
      cr_consts =
        SMap.map (traverse_refined_const ~on_ty ~on_rc_bound ~ctx) cr_consts;
    }

  and traverse_refined_const { rc_bound; rc_is_ctx } ~on_ty ~on_rc_bound ~ctx =
    let rc_bound = transform_rc_bound rc_bound ~on_ty ~on_rc_bound ~ctx in
    { rc_bound; rc_is_ctx }

  and transform_rc_bound rc_bound ~on_ty ~on_rc_bound ~ctx =
    match on_rc_bound rc_bound ~ctx with
    | (_, `Stop rc_bound) -> rc_bound
    | (ctx, `Continue rc_bound) ->
      traverse_rc_bound rc_bound ~on_ty ~on_rc_bound ~ctx
    | (ctx, `Restart rc_bound) ->
      transform_rc_bound rc_bound ~on_ty ~on_rc_bound ~ctx

  and traverse_rc_bound rc_bound ~on_ty ~on_rc_bound ~ctx =
    match rc_bound with
    | TRexact ty -> TRexact (transform ty ~on_ty ~on_rc_bound ~ctx)
    | TRloose { tr_lower; tr_upper } ->
      let tr_lower = List.map tr_lower ~f:(transform ~on_ty ~on_rc_bound ~ctx)
      and tr_upper =
        List.map tr_upper ~f:(transform ~on_ty ~on_rc_bound ~ctx)
      in
      TRloose { tr_lower; tr_upper }

  and traverse_fun_ty
      ({
         ft_tparams;
         ft_where_constraints;
         ft_params;
         ft_implicit_params;
         ft_ret;
         _;
       } as fun_ty)
      ~on_ty
      ~on_rc_bound
      ~ctx =
    let ft_tparams =
      List.map ft_tparams ~f:(fun ({ tp_constraints; _ } as tparam) ->
          let tp_constraints =
            List.map tp_constraints ~f:(fun (cstr_kind, ty) ->
                (cstr_kind, transform ty ~on_ty ~on_rc_bound ~ctx))
          in
          { tparam with tp_constraints })
    and ft_params =
      List.map ft_params ~f:(fun ({ fp_type; _ } as fun_param) ->
          let fp_type = transform fp_type ~on_ty ~on_rc_bound ~ctx in
          { fun_param with fp_type })
    and ft_implicit_params =
      let { capability } = ft_implicit_params in
      match capability with
      | CapDefaults _ -> ft_implicit_params
      | CapTy ty ->
        { capability = CapTy (transform ty ~on_ty ~on_rc_bound ~ctx) }
    and ft_where_constraints =
      List.map ft_where_constraints ~f:(fun (ty1, cstr_kind, ty2) ->
          ( transform ty1 ~on_ty ~on_rc_bound ~ctx,
            cstr_kind,
            transform ty2 ~on_ty ~on_rc_bound ~ctx ))
    and ft_ret = transform ft_ret ~on_ty ~on_rc_bound ~ctx in
    {
      fun_ty with
      ft_tparams;
      ft_where_constraints;
      ft_params;
      ft_implicit_params;
      ft_ret;
    }

  and traverse_tuple_ty { t_required; t_extra } ~on_ty ~on_rc_bound ~ctx =
    let t_required = List.map t_required ~f:(transform ~on_ty ~on_rc_bound ~ctx)
    and t_extra =
      match t_extra with
      | Tsplat ty -> Tsplat (transform ty ~on_ty ~on_rc_bound ~ctx)
      | Textra { t_optional; t_variadic } ->
        let t_optional =
          List.map t_optional ~f:(transform ~on_ty ~on_rc_bound ~ctx)
        and t_variadic = transform t_variadic ~on_ty ~on_rc_bound ~ctx in
        Textra { t_optional; t_variadic }
    in
    { t_required; t_extra }

  and traverse_shape_ty
      { s_origin; s_unknown_value; s_fields } ~on_ty ~on_rc_bound ~ctx =
    let s_unknown_value = transform s_unknown_value ~on_ty ~on_rc_bound ~ctx
    and s_fields =
      TShapeMap.map
        (fun { sft_optional; sft_ty } ->
          let sft_ty = transform sft_ty ~on_ty ~on_rc_bound ~ctx in
          { sft_optional; sft_ty })
        s_fields
    in
    { s_origin; s_unknown_value; s_fields }
end

let transform_top_down_decl_ty decl_ty ~on_ty ~on_rc_bound ~ctx =
  Transform_top_down_decl.transform decl_ty ~on_ty ~on_rc_bound ~ctx

let is_type_tag_generic_wildcard generic =
  match generic with
  | Wildcard _ -> true
  | Filled _ -> false
