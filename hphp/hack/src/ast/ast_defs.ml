(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* We could open Hh_prelude here, but then we get errors about using ==,
   which some visitor below uses. *)
open Base.Export

(*****************************************************************************)
(* The Abstract Syntax Tree *)
(*****************************************************************************)

type pos = (Pos.t[@visitors.opaque]) [@@transform.opaque]

and id_ = string

and id = pos * id_

and pstring = pos * string

and byte_string = string

and positioned_byte_string = pos * byte_string

and shape_field_name =
  | SFlit_int of pstring
  | SFlit_str of positioned_byte_string
  | SFclass_const of id * pstring
[@@transform.opaque]

and variance =
  | Covariant
  | Contravariant
  | Invariant
[@@transform.opaque]

and constraint_kind =
  | Constraint_as
  | Constraint_eq
  | Constraint_super
[@@transform.opaque]

and abstraction =
  | Concrete
  | Abstract
[@@transform.opaque]

and classish_kind =
  | Cclass of abstraction  (** Kind for `class` and `abstract class` *)
  | Cinterface  (** Kind for `interface` *)
  | Ctrait  (** Kind for `trait` *)
  | Cenum  (** Kind for `enum` *)
  | Cenum_class of abstraction
      (** Kind for `enum class` and `abstract enum class`.
      See https://docs.hhvm.com/hack/built-in-types/enum-class
  *)
[@@transform.opaque]

and param_kind =
  | Pinout of pos
      (**
       * Contains the position for an entire `inout` annotated expression, e.g.:
       *
       *   foo(inout $bar);
       *       ^^^^^^^^^^
       *)
  | Pnormal
[@@transform.opaque]

and readonly_kind = Readonly [@@transform.opaque]

and og_null_flavor =
  | OG_nullthrows
  | OG_nullsafe
[@@transform.opaque]

and prop_or_method =
  | Is_prop
  | Is_method
[@@transform.opaque]

and fun_kind =
  | FSync
  | FAsync
  | FGenerator
  | FAsyncGenerator
[@@transform.opaque]

and bop =
  | Plus  (** Addition: x + y *)
  | Minus  (** Subtraction: x - y  *)
  | Star  (** Multiplication: x * y *)
  | Slash  (** Division: x / y *)
  | Eqeq  (** Value/coercing equality: x == y *)
  | Eqeqeq  (** Same-type-and-value equality: x === y *)
  | Starstar  (** Exponent: x ** y *)
  | Diff  (** Value inquality: x != y *)
  | Diff2  (** Not-same-type-and-value-equality: x !== y *)
  | Ampamp  (** Logical AND: x && y *)
  | Barbar  (** Logical OR: x || y *)
  | Lt  (** Less than: x < y *)
  | Lte  (** Less than or equal to: x <= y *)
  | Gt  (** Greater than: x > y *)
  | Gte  (** Greater than or equal to: x >= y *)
  | Dot  (** String concatenation: x . y *)
  | Amp  (** Bitwise AND: x & y *)
  | Bar  (** Bitwise OR: x | y *)
  | Ltlt  (** Bitwise left shift: x << y *)
  | Gtgt  (** Bitwise right shift: x >> y *)
  | Percent  (** Modulo: x % y *)
  | Xor  (** Bitwise XOR: x ^ y *)
  | Cmp  (** Spaceship operator: x <=> y *)
  | QuestionQuestion  (** Coalesce: x ?? y *)
  | Eq of bop option  (** =, +=, -=, ... *)
[@@transform.opaque]

and uop =
  | Utild  (** Bitwise negation: ~x *)
  | Unot  (** Logical not: !b *)
  | Uplus  (** Unary plus: +x *)
  | Uminus  (** Unary minus: -x *)
  | Uincr  (** Unary increment: ++i  *)
  | Udecr  (** Unary decrement: --i *)
  | Upincr  (** Unary postfix increment: i++ *)
  | Updecr  (** Unary postfix decrement: i-- *)
  | Usilence  (** Error control/Silence (ignore) expections: @e *)
[@@transform.opaque]

and visibility =
  | Private [@visitors.name "visibility_Private"]
  | Public [@visitors.name "visibility_Public"]
  | Protected [@visitors.name "visibility_Protected"]
  | Internal [@visitors.name "visibility_Internal"]
[@@transform.opaque]

(** Literal values that can occur in XHP enum properties.
 *
 * class :my-xhp-class {
 *   attribute enum {'big', 'small'} my-prop;
 * }
 *)
and xhp_enum_value =
  | XEV_Int of int
  | XEV_String of string
[@@transform.opaque]

(** Hack's primitive types (as the typechecker understands them).
 *
 * Used in the AST of typehints (Aast_defs.Hprim) and in the representation of
 * types (Typing_defs.Tprim).
 *)
and tprim =
  | Tnull
  | Tvoid
  | Tint
  | Tbool
  | Tfloat
  | Tstring
  | Tresource
  | Tnum
  | Tarraykey
  | Tnoreturn
[@@transform.opaque]

and typedef_visibility =
  | Transparent
  | Opaque
  | OpaqueModule
  | CaseType
[@@transform.opaque]

and reify_kind =
  | Erased
  | SoftReified
  | Reified
[@@transform.opaque]
[@@deriving
  show { with_path = false },
    eq,
    hash,
    ord,
    transform ~restart:(`Disallow `Encode_as_result),
    visitors
      {
        variety = "iter";
        nude = true;
        visit_prefix = "on_";
        ancestors = ["Visitors_runtime.iter_base"];
      },
    visitors
      {
        variety = "endo";
        nude = true;
        visit_prefix = "on_";
        ancestors = ["Visitors_runtime.endo_base"];
      },
    visitors
      {
        variety = "reduce";
        nude = true;
        visit_prefix = "on_";
        ancestors = ["Visitors_runtime.reduce_base"];
      },
    visitors
      {
        variety = "map";
        nude = true;
        visit_prefix = "on_";
        ancestors = ["Visitors_runtime.map_base"];
      }]

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let get_pos : type a. Pos.t * a -> Pos.t = (fun (p, _) -> p)

let get_id : id -> id_ = (fun (_p, id) -> id)

let is_concrete = function
  | Concrete -> true
  | Abstract -> false

let is_abstract = function
  | Abstract -> true
  | Concrete -> false

let is_c_class = function
  | Cclass _ -> true
  | Cenum
  | Cenum_class _
  | Ctrait
  | Cinterface ->
    false

let is_c_normal = function
  | Cclass c -> is_concrete c
  | Cenum
  | Cenum_class _
  | Ctrait
  | Cinterface ->
    false

let is_c_concrete = function
  | Cclass k
  | Cenum_class k ->
    is_concrete k
  | Cinterface
  | Ctrait
  | Cenum ->
    false

let is_c_abstract = function
  | Cclass c -> is_abstract c
  | Cinterface
  | Cenum_class _
  | Ctrait
  | Cenum ->
    false

let is_classish_abstract = function
  | Cenum_class c
  | Cclass c ->
    is_abstract c
  | Cinterface
  | Ctrait
  | Cenum ->
    false

let is_c_enum = function
  | Cenum -> true
  | Cenum_class _
  | Cclass _
  | Ctrait
  | Cinterface ->
    false

let is_c_enum_class = function
  | Cenum_class _ -> true
  | Cenum
  | Cclass _
  | Ctrait
  | Cinterface ->
    false

let is_c_interface = function
  | Cinterface -> true
  | Cclass _
  | Cenum_class _
  | Ctrait
  | Cenum ->
    false

let is_c_trait = function
  | Ctrait -> true
  | Cinterface
  | Cenum_class _
  | Cclass _
  | Cenum ->
    false

let is_f_async = function
  | FAsync -> true
  | _ -> false

let is_f_async_or_generator = function
  | FAsync
  | FAsyncGenerator ->
    true
  | _ -> false

let string_of_classish_kind kind =
  match kind with
  | Cclass c ->
    (match c with
    | Abstract -> "an abstract class"
    | Concrete -> "a class")
  | Cinterface -> "an interface"
  | Ctrait -> "a trait"
  | Cenum -> "an enum"
  | Cenum_class c ->
    (match c with
    | Abstract -> "an abstract enum class"
    | Concrete -> "an enum class")

let swap_variance = function
  | Covariant -> Contravariant
  | Contravariant -> Covariant
  | Invariant -> Invariant

module ShapeField = struct
  type t = shape_field_name

  (* We include span information in shape_field_name to improve error
   * messages, but we don't want it being used in the comparison, so
   * we have to write our own compare. *)
  let compare x y =
    match (x, y) with
    | (SFlit_int (_, s1), SFlit_int (_, s2)) -> String.compare s1 s2
    | (SFlit_str (_, s1), SFlit_str (_, s2)) -> String.compare s1 s2
    | (SFclass_const ((_, s1), (_, s1')), SFclass_const ((_, s2), (_, s2'))) ->
      Core.Tuple.T2.compare
        ~cmp1:String.compare
        ~cmp2:String.compare
        (s1, s1')
        (s2, s2')
    | (SFlit_int _, _) -> -1
    | (SFlit_str _, SFlit_int _) -> 1
    | (SFlit_str _, _) -> -1
    | (SFclass_const _, _) -> 1

  let equal x y = Core.Int.equal 0 (compare x y)
end

module ShapeMap = struct
  include WrappedMap.Make (ShapeField)

  let map_and_rekey m f1 f2 =
    fold (fun k v acc -> add (f1 k) (f2 v) acc) m empty

  let pp
      (pp_val : Format.formatter -> 'a -> unit)
      (fmt : Format.formatter)
      (map : 'a t) : unit =
    make_pp pp_shape_field_name pp_val fmt map
end

module ShapeSet = Stdlib.Set.Make (ShapeField)
