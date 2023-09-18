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
[@@deriving eq, ord, show]

(* Represents <<__Policied()>> or <<__InferFlows>> attribute *)
type ifc_fun_decl =
  | FDPolicied of string option
  | FDInferFlows
[@@deriving eq, hash, ord]

type cross_package_decl = string option [@@deriving eq, hash, ord]

(* The default policy is the public one. PUBLIC is a keyword, so no need to prevent class collisions *)
let default_ifc_fun_decl = FDPolicied (Some "PUBLIC")

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

type fun_tparams_kind =
  | FTKtparams
      (** If ft_tparams is empty, the containing fun_type is a concrete function type.
      Otherwise, it is a generic function and ft_tparams specifies its type parameters. *)
  | FTKinstantiated_targs
      (** The containing fun_type is a concrete function type which is an
      instantiation of a generic function with at least one reified type
      parameter. This means that the function requires explicit type arguments
      at every invocation, and ft_tparams specifies the type arguments with
      which the generic function was instantiated, as well as whether each
      explicit type argument must be reified. *)
[@@deriving eq]

type type_origin =
  | Missing_origin
  | From_alias of string
[@@deriving eq, hash, ord, show]

type pos_string = Pos_or_decl.t * string [@@deriving eq, hash, ord, show]

(* Trick the Rust generators to use a BStr, the same way it does for Ast_defs.shape_field_name. *)
type t_byte_string = string [@@deriving eq, hash, ord, show]

type pos_byte_string = Pos_or_decl.t * t_byte_string
[@@deriving eq, hash, ord, show]

type tshape_field_name =
  | TSFlit_int of pos_string
  | TSFlit_str of pos_byte_string
  | TSFclass_const of pos_id * pos_string
[@@deriving eq, hash, ord, show]

module TShapeField = struct
  type t = tshape_field_name [@@deriving hash]

  let pos : t -> Pos_or_decl.t = function
    | TSFlit_int (p, _)
    | TSFlit_str (p, _) ->
      p
    | TSFclass_const ((cls_pos, _), (mem_pos, _)) ->
      Pos_or_decl.btw cls_pos mem_pos

  let name = function
    | TSFlit_int (_, s)
    | TSFlit_str (_, s) ->
      s
    | TSFclass_const ((_, s1), (_, s2)) -> s1 ^ "::" ^ s2

  let of_ast : (Pos.t -> Pos_or_decl.t) -> Ast_defs.shape_field_name -> t =
   fun convert_pos -> function
    | Ast_defs.SFlit_int (p, s) -> TSFlit_int (convert_pos p, s)
    | Ast_defs.SFlit_str (p, s) -> TSFlit_str (convert_pos p, s)
    | Ast_defs.SFclass_const ((pcls, cls), (pconst, const)) ->
      TSFclass_const ((convert_pos pcls, cls), (convert_pos pconst, const))

  (* We include span information in shape_field_name to improve error
   * messages, but we don't want it being used in the comparison, so
   * we have to write our own compare. *)
  let compare x y =
    match (x, y) with
    | (TSFlit_int (_, s1), TSFlit_int (_, s2)) -> String.compare s1 s2
    | (TSFlit_str (_, s1), TSFlit_str (_, s2)) -> String.compare s1 s2
    | (TSFclass_const ((_, s1), (_, s1')), TSFclass_const ((_, s2), (_, s2')))
      ->
      Core.Tuple.T2.compare
        ~cmp1:String.compare
        ~cmp2:String.compare
        (s1, s1')
        (s2, s2')
    | (TSFlit_int _, _) -> -1
    | (TSFlit_str _, TSFlit_int _) -> 1
    | (TSFlit_str _, _) -> -1
    | (TSFclass_const _, _) -> 1

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

module TShapeSet = Caml.Set.Make (TShapeField)

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
  | DTexpr of Ident_provider.Ident.t
[@@deriving eq, hash, ord, show]

type user_attribute_param =
  | Classname of string
  | EnumClassLabel of string
  | String of t_byte_string
  | Int of string
[@@deriving eq, hash, show]

type user_attribute = {
  ua_name: pos_id;
  ua_params: user_attribute_param list;
}
[@@deriving eq, hash, show]

type 'ty tparam = {
  tp_variance: Ast_defs.variance;
  tp_name: pos_id;
  tp_tparams: 'ty tparam list;
  tp_constraints: (Ast_defs.constraint_kind * 'ty) list;
  tp_reified: Ast_defs.reify_kind;
  tp_user_attributes: user_attribute list;
}
[@@deriving eq, hash, show]

type 'ty where_constraint = 'ty * Ast_defs.constraint_kind * 'ty
[@@deriving eq, hash, show]

type enforcement =
  | Unenforced
  | Enforced
[@@deriving eq, hash, show, ord]

(* This is to avoid a compile error with ppx_hash "Unbound value _hash_fold_phase". *)
let _hash_fold_phase hsv _ = hsv

type 'phase ty = 'phase Reason.t_ * 'phase ty_

and decl_ty = decl_phase ty

and locl_ty = locl_phase ty

and neg_type =
  | Neg_prim of Ast_defs.tprim
  | Neg_class of pos_id

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
  | Tapply : pos_id * decl_ty list -> decl_phase ty_
      (** Either an object type or a type alias, ty list are the arguments *)
  | Trefinement : decl_ty * decl_phase class_refinement -> decl_phase ty_
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
  | Tlike : decl_ty -> decl_phase ty_
  (*========== Following Types Exist in Both Phases ==========*)
  | Tany : TanySentinel.t -> 'phase ty_
  | Tnonnull
  | Tdynamic
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
  | Tprim : Ast_defs.tprim -> 'phase ty_
      (** All the primitive types: int, string, void, etc. *)
  | Tfun : 'phase ty fun_type -> 'phase ty_
      (** A wrapper around fun_type, which contains the full type information for a
       * function, method, lambda, etc. *)
  | Ttuple : 'phase ty list -> 'phase ty_
      (** Tuple, with ordered list of the types of the elements of the tuple. *)
  | Tshape : 'phase shape_type -> 'phase ty_
  | Tgeneric : string * 'phase ty list -> 'phase ty_
      (** The type of a generic parameter. The constraints on a generic parameter
       * are accessed through the lenv.tpenv component of the environment, which
       * is set up when checking the body of a function or method. See uses of
       * Typing_phase.add_generic_parameters_and_constraints. The list denotes
       * type arguments.
       *)
  | Tunion : 'phase ty list -> 'phase ty_
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
  | Tnewtype : string * 'phase ty list * 'phase ty -> 'phase ty_
      (** The type of an opaque type or enum. Outside their defining files or
       * when they represent enums, they are "opaque", which means that they
       * only unify with themselves. Within a file, uses of newtypes are
       * expanded to their definitions (unless the newtype is an enum).
       *
       * However, it is possible to have a constraint that allows us to relax
       * opaqueness. For example:
       *
       *   newtype MyType as int = ...
       *
       * or
       *
       *   enum MyType: int as int { ... }
       *
       * Outside of the file where the type was defined, this translates to:
       *
       *   Tnewtype ((pos, "MyType"), [], Tprim Tint)
       *
       * which means that MyType is abstract, but is a subtype of int as well.
       * When the constraint is omitted, the third parameter is set to mixed.
       *
       * The second parameter is the list of type arguments to the type.
       *)
  (*========== Below Are Types That Cannot Be Declared In User Code ==========*)
  | Tvar : Ident.t -> locl_phase ty_
  | Tunapplied_alias : string -> locl_phase ty_
      (** This represents a type alias that lacks necessary type arguments. Given
           type Foo<T1,T2> = ...
         Tunappliedalias "Foo" stands for usages of plain Foo, without supplying
         further type arguments. In particular, Tunappliedalias always stands for
         a higher-kinded type. It is never used for an alias like
           type Foo2 = ...
         that simply doesn't require type arguments. *)
  | Tdependent : dependent_type * locl_ty -> locl_phase ty_
      (** see dependent_type *)
  | Tclass : pos_id * exact * locl_ty list -> locl_phase ty_
      (** An instance of a class or interface, ty list are the arguments
       * If exact=Exact, then this represents instances of *exactly* this class
       * If exact=Nonexact, this also includes subclasses
       *)
  | Tneg : neg_type -> locl_phase ty_
      (** The negation of the type in neg_type *)

and 'phase taccess_type = 'phase ty * pos_id

and exact =
  | Exact
  | Nonexact of locl_phase class_refinement

and 'phase class_refinement = { cr_consts: 'phase refined_const SMap.t }

and 'phase refined_const = {
  rc_bound: 'phase refined_const_bound;
  rc_is_ctx: bool;
}

and 'phase refined_const_bound =
  | TRexact : 'phase ty -> 'phase refined_const_bound
  | TRloose : 'phase refined_const_bounds -> 'phase refined_const_bound

and 'phase refined_const_bounds = {
  tr_lower: 'phase ty list;
  tr_upper: 'phase ty list;
}

(** Whether all fields of this shape are known, types of each of the
  * known arms.
  *)
and 'phase shape_type = {
  s_origin: type_origin;
  s_unknown_value: 'phase ty;
  s_fields: 'phase shape_field_type TShapeMap.t;
}

and 'ty capability =
  | CapDefaults of Pos_or_decl.t
  | CapTy of 'ty

(** Companion to fun_params type, intended to consolidate checking of
 * implicit params for functions. *)
and 'ty fun_implicit_params = { capability: 'ty capability }

(** The type of a function AND a method. *)
and 'ty fun_type = {
  ft_tparams: 'ty tparam list;
  ft_where_constraints: 'ty where_constraint list;
  ft_params: 'ty fun_params;
  ft_implicit_params: 'ty fun_implicit_params;
  ft_ret: 'ty possibly_enforced_ty;
      (** Carries through the sync/async information from the aast *)
  ft_flags: Typing_defs_flags.fun_type_flags;
  ft_ifc_decl: ifc_fun_decl;
  ft_cross_package: cross_package_decl;
}

and 'ty possibly_enforced_ty = {
  et_enforced: enforcement;
      (** True if consumer of this type enforces it at runtime *)
  et_type: 'ty;
}

and 'ty fun_param = {
  fp_pos: Pos_or_decl.t;
  fp_name: string option;
  fp_type: 'ty possibly_enforced_ty;
  fp_flags: Typing_defs_flags.fun_param_flags;
}

and 'ty fun_params = 'ty fun_param list [@@deriving hash]

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

  let get_ft_return_disposable ft =
    is_set ft.ft_flags ft_flags_return_disposable

  let get_ft_returns_readonly ft = is_set ft.ft_flags ft_flags_returns_readonly

  let get_ft_readonly_this ft = is_set ft.ft_flags ft_flags_readonly_this

  let get_ft_async ft = is_set ft.ft_flags ft_flags_async

  let get_ft_generator ft = is_set ft.ft_flags ft_flags_generator

  let get_ft_support_dynamic_type ft =
    is_set ft.ft_flags ft_flags_support_dynamic_type

  (* This flag is set true only if the exact method has the memoized attribute. *)
  let get_ft_is_memoized ft = is_set ft.ft_flags ft_flags_is_memoized

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

  let set_ft_is_function_pointer ft is_fp =
    {
      ft with
      ft_flags = set_bit ft_flags_is_function_pointer is_fp ft.ft_flags;
    }

  let set_ft_readonly_this ft readonly_this =
    {
      ft with
      ft_flags = set_bit ft_flags_readonly_this readonly_this ft.ft_flags;
    }

  let set_ft_returns_readonly ft readonly_return =
    {
      ft with
      ft_flags = set_bit ft_flags_returns_readonly readonly_return ft.ft_flags;
    }

  let get_ft_is_function_pointer ft =
    is_set ft.ft_flags ft_flags_is_function_pointer

  let get_ft_variadic ft = is_set ft.ft_flags ft_flags_variadic

  let get_ft_fun_kind ft =
    match (get_ft_async ft, get_ft_generator ft) with
    | (false, false) -> Ast_defs.FSync
    | (true, false) -> Ast_defs.FAsync
    | (false, true) -> Ast_defs.FGenerator
    | (true, true) -> Ast_defs.FAsyncGenerator

  let get_fp_ifc_external fp = is_set fp.fp_flags fp_flags_ifc_external

  let get_fp_ifc_can_call fp = is_set fp.fp_flags fp_flags_ifc_can_call

  let get_fp_readonly fp = is_set fp.fp_flags fp_flags_readonly

  let fun_kind_to_flags kind =
    match kind with
    | Ast_defs.FSync -> 0
    | Ast_defs.FAsync -> ft_flags_async
    | Ast_defs.FGenerator -> ft_flags_generator
    | Ast_defs.FAsyncGenerator -> Int.bit_or ft_flags_async ft_flags_generator

  let make_ft_flags
      kind
      ~return_disposable
      ~returns_readonly
      ~readonly_this
      ~support_dynamic_type
      ~is_memoized
      ~variadic =
    let flags = fun_kind_to_flags kind in
    let flags = set_bit ft_flags_return_disposable return_disposable flags in
    let flags = set_bit ft_flags_returns_readonly returns_readonly flags in
    let flags = set_bit ft_flags_readonly_this readonly_this flags in
    let flags =
      set_bit ft_flags_support_dynamic_type support_dynamic_type flags
    in
    let flags = set_bit ft_flags_is_memoized is_memoized flags in
    let flags = set_bit ft_flags_variadic variadic flags in
    flags

  let mode_to_flags mode =
    match mode with
    | FPnormal -> 0x0
    | FPinout -> fp_flags_inout

  let make_fp_flags
      ~mode
      ~accept_disposable
      ~has_default
      ~ifc_external
      ~ifc_can_call
      ~readonly =
    let flags = mode_to_flags mode in
    let flags = set_bit fp_flags_accept_disposable accept_disposable flags in
    let flags = set_bit fp_flags_has_default has_default flags in
    let flags = set_bit fp_flags_ifc_external ifc_external flags in
    let flags = set_bit fp_flags_ifc_can_call ifc_can_call flags in
    let flags = set_bit fp_flags_readonly readonly flags in
    flags

  let get_fp_accept_disposable fp =
    is_set fp.fp_flags fp_flags_accept_disposable

  let get_fp_has_default fp = is_set fp.fp_flags fp_flags_has_default

  let get_fp_mode fp =
    if is_set fp.fp_flags fp_flags_inout then
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
    Reason.pp fmt a0;
    Format.fprintf fmt ",@ ";
    pp_ty_ fmt a1;
    Format.fprintf fmt "@])"

  and pp_neg_type : Format.formatter -> neg_type -> unit =
   fun fmt neg_ty ->
    match neg_ty with
    | Neg_prim tp ->
      Format.fprintf fmt "(@[<2>Neg_prim@ ";
      Aast.pp_tprim fmt tp;
      Format.fprintf fmt "@])"
    | Neg_class c ->
      Format.fprintf fmt "(@[<2>Neg_class ";
      pp_pos_id fmt c;
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
    | Tgeneric (a0, a1) ->
      Format.fprintf fmt "(@[<2>Tgeneric (@,";
      Format.fprintf fmt "%S" a0;
      Format.fprintf fmt ",@ ";
      pp_list pp_ty fmt a1;
      Format.fprintf fmt "@,)@])"
    | Tunapplied_alias a0 ->
      Format.fprintf fmt "(@[<2>Tunappliedalias@ ";
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
      pp_fun_type fmt a0;
      Format.fprintf fmt "@])"
    | Ttuple a0 ->
      Format.fprintf fmt "(@[<2>Ttuple@ ";
      pp_list pp_ty fmt a0;
      Format.fprintf fmt "@])"
    | Tshape a0 ->
      Format.fprintf fmt "(@[<2>Tshape@ ";
      pp_shape_type fmt a0;
      Format.fprintf fmt "@])"
    | Tvar a0 ->
      Format.fprintf fmt "(@[<2>Tvar@ ";
      Ident.pp fmt a0;
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
      pp_neg_type fmt a0;
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

  and pp_exact fmt e =
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

  and pp_possibly_enforced_ty :
      type a. Format.formatter -> a ty possibly_enforced_ty -> unit =
   fun fmt x ->
    Format.fprintf fmt "@[<2>{ ";

    Format.fprintf fmt "@[%s =@ " "et_enforced";
    Format.fprintf fmt "%a" pp_enforcement x.et_enforced;
    Format.fprintf fmt "@]";
    Format.fprintf fmt ";@ ";

    Format.fprintf fmt "@[%s =@ " "et_type";
    pp_ty fmt x.et_type;
    Format.fprintf fmt "@]";
    Format.fprintf fmt "@ }@]"

  and pp_capability : type a. Format.formatter -> a ty capability -> unit =
   fun fmt -> function
    | CapTy ty ->
      Format.pp_print_string fmt "(CapTy ";
      pp_ty fmt ty;
      Format.pp_print_string fmt ")"
    | CapDefaults pos ->
      Format.pp_print_string fmt "(CapDefaults ";
      Pos_or_decl.pp fmt pos;
      Format.pp_print_string fmt ")"

  and pp_fun_implicit_params :
      type a. Format.formatter -> a ty fun_implicit_params -> unit =
   fun fmt x ->
    Format.fprintf fmt "@[<2>{ ";

    Format.fprintf fmt "@[%s =@ " "capability";
    pp_capability fmt x.capability;
    Format.fprintf fmt "@]";

    Format.fprintf fmt "@ }@]"

  and pp_shape_field_type :
      type a. Format.formatter -> a shape_field_type -> unit =
   fun fmt x ->
    Format.fprintf fmt "@[<2>{ ";
    Format.fprintf fmt "@[%s =@ " "sft_optional";
    Format.fprintf fmt "%B" x.sft_optional;
    Format.fprintf fmt "@]";
    Format.fprintf fmt ";@ ";
    Format.fprintf fmt "@[%s =@ " "sft_ty";
    pp_ty fmt x.sft_ty;
    Format.fprintf fmt "@]";
    Format.fprintf fmt "@ }@]"

  and pp_ifc_fun_decl : Format.formatter -> ifc_fun_decl -> unit =
   fun fmt r ->
    match r with
    | FDInferFlows -> Format.pp_print_string fmt "FDInferFlows"
    | FDPolicied None -> Format.pp_print_string fmt "FDPolicied {}"
    | FDPolicied (Some s) ->
      Format.pp_print_string fmt "FDPolicied {";
      Format.pp_print_string fmt s;
      Format.pp_print_string fmt "}"

  and pp_cross_package_decl : Format.formatter -> cross_package_decl -> unit =
   fun fmt r ->
    match r with
    | Some s ->
      Format.pp_print_string fmt "CrossPackage(";
      Format.pp_print_string fmt s;
      Format.pp_print_string fmt ")"
    | None -> Format.pp_print_string fmt "None"

  and pp_shape_type : type a. Format.formatter -> a shape_type -> unit =
   fun fmt x ->
    Format.fprintf fmt "@[<2>{ ";

    Format.fprintf fmt "@[%s =@ " "s_origin";
    pp_type_origin fmt x.s_origin;
    Format.fprintf fmt "@]";
    Format.fprintf fmt ";@ ";

    Format.fprintf fmt "@[%s =@ " "s_fields";
    TShapeMap.pp pp_shape_field_type fmt x.s_fields;
    Format.fprintf fmt "@]";
    Format.fprintf fmt ";@ ";

    Format.fprintf fmt "@[%s =@ " "s_unknown_value";
    pp_ty fmt x.s_unknown_value;
    Format.fprintf fmt "@]";

    Format.fprintf fmt "@ }@]"

  and pp_fun_type : type a. Format.formatter -> a ty fun_type -> unit =
   fun fmt x ->
    Format.fprintf fmt "@[<2>{ ";

    Format.fprintf fmt "@[%s =@ " "ft_tparams";
    pp_list pp_tparam_ fmt x.ft_tparams;
    Format.fprintf fmt "@]";
    Format.fprintf fmt ";@ ";

    Format.fprintf fmt "@[%s =@ " "ft_where_constraints";
    pp_list pp_where_constraint_ fmt x.ft_where_constraints;
    Format.fprintf fmt "@]";
    Format.fprintf fmt ";@ ";

    Format.fprintf fmt "@[%s =@ " "ft_params";
    pp_list pp_fun_param fmt x.ft_params;
    Format.fprintf fmt "@]";
    Format.fprintf fmt ";@ ";

    Format.fprintf fmt "@[%s =@ " "ft_implicit_params";
    pp_fun_implicit_params fmt x.ft_implicit_params;
    Format.fprintf fmt "@]";
    Format.fprintf fmt ";@ ";

    Format.fprintf fmt "@[%s =@ " "ft_ret";
    pp_possibly_enforced_ty fmt x.ft_ret;
    Format.fprintf fmt "@]";
    Format.fprintf fmt ";@ ";

    let pp_ft_flags fmt ft =
      Format.fprintf fmt "@[<2>(%s@ " "make_ft_flags";

      Format.fprintf fmt "@[";
      Format.fprintf fmt "%s" (Ast_defs.show_fun_kind (get_ft_fun_kind ft));
      Format.fprintf fmt "@]";
      Format.fprintf fmt "@ ";

      Format.fprintf fmt "@[~%s:" "return_disposable";
      Format.fprintf fmt "%B" (get_ft_return_disposable ft);
      Format.fprintf fmt "@]";
      Format.fprintf fmt "@ ";

      Format.fprintf fmt "@[~%s:" "returns_readonly";
      Format.fprintf fmt "%B" (get_ft_returns_readonly ft);
      Format.fprintf fmt "@]";
      Format.fprintf fmt "@ ";

      Format.fprintf fmt "@[~%s:" "support_dynamic_type";
      Format.fprintf fmt "%B" (get_ft_support_dynamic_type ft);
      Format.fprintf fmt "@]";
      Format.fprintf fmt "@ ";

      Format.fprintf fmt "@[~%s:" "readonly_this";
      Format.fprintf fmt "%B" (get_ft_readonly_this ft);
      Format.fprintf fmt "@]";
      Format.fprintf fmt "@ ";

      Format.fprintf fmt "@[~%s:" "is_memoized";
      Format.fprintf fmt "%B" (get_ft_is_memoized ft);
      Format.fprintf fmt "@]";
      Format.fprintf fmt "@ ";

      Format.fprintf fmt "@[~%s:" "variadic";
      Format.fprintf fmt "%B" (get_ft_variadic ft);
      Format.fprintf fmt "@]";

      Format.fprintf fmt ")@]"
    in

    Format.fprintf fmt "@[%s =@ " "ft_flags";
    pp_ft_flags fmt x;
    Format.fprintf fmt "@]";
    Format.fprintf fmt ";@ ";

    Format.fprintf fmt "@[%s =@ " "ft_ifc_decl";
    pp_ifc_fun_decl fmt x.ft_ifc_decl;
    Format.fprintf fmt "@]";
    Format.fprintf fmt ";@ ";

    Format.fprintf fmt "@[%s =@ " "ft_cross_package";
    pp_cross_package_decl fmt x.ft_cross_package;
    Format.fprintf fmt "@]";

    Format.fprintf fmt "@ }@]"

  and pp_where_constraint_ :
      type a. Format.formatter -> a ty where_constraint -> unit =
   (fun fmt whcstr -> pp_where_constraint pp_ty fmt whcstr)

  and pp_fun_param : type a. Format.formatter -> a ty fun_param -> unit =
    let pp_fp_flags fmt fp =
      Format.fprintf fmt "@[<2>(%s@ " "make_fp_flags";

      Format.fprintf fmt "@[~%s:" "accept_disposable";
      Format.fprintf fmt "%B" (get_fp_accept_disposable fp);
      Format.fprintf fmt "@]";
      Format.fprintf fmt "@ ";

      Format.fprintf fmt "@[~%s:" "has_default";
      Format.fprintf fmt "%B" (get_fp_has_default fp);
      Format.fprintf fmt "@]";
      Format.fprintf fmt "@ ";

      Format.fprintf fmt "@[~%s:" "mode";
      Format.fprintf fmt "%s" (show_param_mode (get_fp_mode fp));
      Format.fprintf fmt "@]";
      Format.fprintf fmt "@ ";

      Format.fprintf fmt "@[~%s:" "ifc_external";
      Format.fprintf fmt "%B" (get_fp_ifc_external fp);
      Format.fprintf fmt "@]";
      Format.fprintf fmt "@ ";

      Format.fprintf fmt "@[~%s:" "ifc_can_call";
      Format.fprintf fmt "%B" (get_fp_ifc_can_call fp);
      Format.fprintf fmt "@]";
      Format.fprintf fmt "@ ";

      Format.fprintf fmt "@[~%s:" "readonly";
      Format.fprintf fmt "%B" (get_fp_readonly fp);
      Format.fprintf fmt "@]";

      Format.fprintf fmt ")@]"
    in

    fun fmt x ->
      Format.fprintf fmt "@[<2>{ ";

      Format.fprintf fmt "@[%s =@ " "fp_pos";
      Pos_or_decl.pp fmt x.fp_pos;
      Format.fprintf fmt "@]";
      Format.fprintf fmt ";@ ";

      Format.fprintf fmt "@[%s =@ " "fp_name";
      (match x.fp_name with
      | None -> Format.pp_print_string fmt "None"
      | Some x ->
        Format.pp_print_string fmt "(Some ";
        Format.fprintf fmt "%S" x;
        Format.pp_print_string fmt ")");
      Format.fprintf fmt "@]";
      Format.fprintf fmt ";@ ";

      Format.fprintf fmt "@[%s =@ " "fp_type";
      pp_possibly_enforced_ty fmt x.fp_type;
      Format.fprintf fmt "@]";
      Format.fprintf fmt ";@ ";

      Format.fprintf fmt "@[%s =@ " "fp_flags";
      pp_fp_flags fmt x;
      Format.fprintf fmt "@]";
      Format.fprintf fmt ";@ ";
      Format.fprintf fmt "@ }@]"

  and pp_tparam_ : type a. Format.formatter -> a ty tparam -> unit =
   (fun fmt tparam -> pp_tparam pp_ty fmt tparam)

  let pp_possibly_enforced_ty :
      type a.
      (Format.formatter -> a ty -> unit) ->
      Format.formatter ->
      a ty possibly_enforced_ty ->
      unit =
   (fun _ fmt x -> pp_possibly_enforced_ty fmt x)

  let pp_decl_ty : Format.formatter -> decl_ty -> unit =
   (fun fmt ty -> pp_ty fmt ty)

  let pp_locl_ty : Format.formatter -> locl_ty -> unit =
   (fun fmt ty -> pp_ty fmt ty)

  let show_ty : type a. a ty -> string = (fun x -> Format.asprintf "%a" pp_ty x)

  let show_decl_ty x = show_ty x

  let show_locl_ty x = show_ty x
end

include Pp

type decl_ty_ = decl_phase ty_

type locl_ty_ = locl_phase ty_

type decl_tparam = decl_ty tparam [@@deriving show]

type locl_tparam = locl_ty tparam

type decl_where_constraint = decl_ty where_constraint [@@deriving show]

type locl_where_constraint = locl_ty where_constraint

type decl_fun_type = decl_ty fun_type

type locl_fun_type = locl_ty fun_type

type decl_possibly_enforced_ty = decl_ty possibly_enforced_ty

type locl_possibly_enforced_ty = locl_ty possibly_enforced_ty [@@deriving show]

type decl_fun_param = decl_ty fun_param

type locl_fun_param = locl_ty fun_param

type decl_fun_params = decl_ty fun_params

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
  ci_shape: tshape_field_name option;
  ci_val: locl_ty;
  ci_expr_pos: Pos.t;
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
  | Tcan_index of can_index
  | Tcan_traverse of can_traverse
  | Tdestructure of destructure
      (** The type of container destructuring via list() or splat `...` *)
  | TCunion of locl_ty * constraint_type
  | TCintersection of locl_ty * constraint_type

and constraint_type = Reason.t * constraint_type_ [@@deriving show]

type internal_type =
  | LoclType of locl_ty
  | ConstraintType of constraint_type
[@@deriving show]

(* [@@deriving ord] doesn't support GADT. If we want to get
 * rid of this one, we will have to write it *)
let compare_decl_ty : decl_ty -> decl_ty -> int = Stdlib.compare

(* Constructor and deconstructor functions for types and constraint types.
 * Abstracting these lets us change the implementation, e.g. hash cons
 *)
let mk p = p

let deref p = p

let get_reason (r, _) = r

let get_node (_, n) = n

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
