(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include Ast_defs_visitors_ancestors

(*****************************************************************************)
(* The Abstract Syntax Tree *)
(*****************************************************************************)

type pos = (Pos.t[@visitors.opaque])

and id_ = string

and id = pos * id_

and pstring = pos * string

and byte_string = string

and positioned_byte_string = pos * byte_string

and shape_field_name =
  | SFlit_int of pstring
  | SFlit_str of positioned_byte_string
  | SFclass_const of id * pstring

and variance =
  | Covariant
  | Contravariant
  | Invariant

and constraint_kind =
  | Constraint_as
  | Constraint_eq
  | Constraint_super

and reified = bool

and abstraction =
  | Concrete
  | Abstract

and classish_kind =
  | Cclass of abstraction  (** Kind for `class` and `abstract class` *)
  | Cinterface  (** Kind for `interface` *)
  | Ctrait  (** Kind for `trait` *)
  | Cenum  (** Kind for `enum` *)
  | Cenum_class of abstraction
      (** Kind for `enum class` and `abstract enum class`.
      See https://docs.hhvm.com/hack/built-in-types/enum-class
  *)

and param_kind =
  | Pinout of pos
      (**
       * Contains the position for an entire `inout` annotated expression, e.g.:
       *
       *   foo(inout $bar);
       *       ^^^^^^^^^^
       *)
  | Pnormal

and readonly_kind = Readonly

and og_null_flavor =
  | OG_nullthrows
  | OG_nullsafe

and prop_or_method =
  | Is_prop
  | Is_method

and fun_kind =
  | FSync
  | FAsync
  | FGenerator
  | FAsyncGenerator

and bop =
  | Plus
  | Minus
  | Star
  | Slash
  | Eqeq
  | Eqeqeq
  | Starstar
  | Diff
  | Diff2
  | Ampamp
  | Barbar
  | Lt
  | Lte
  | Gt
  | Gte
  | Dot
  | Amp
  | Bar
  | Ltlt
  | Gtgt
  | Percent
  | Xor
  | Cmp
  | QuestionQuestion
  | Eq of bop option

and uop =
  | Utild
  | Unot
  | Uplus
  | Uminus
  | Uincr
  | Udecr
  | Upincr
  | Updecr
  | Usilence

and visibility =
  | Private [@visitors.name "visibility_Private"]
  | Public [@visitors.name "visibility_Public"]
  | Protected [@visitors.name "visibility_Protected"]
  | Internal [@visitors.name "visibility_Internal"]
[@@deriving
  show { with_path = false },
    eq,
    ord,
    visitors
      {
        name = "iter_defs";
        variety = "iter";
        nude = true;
        visit_prefix = "on_";
        ancestors = ["iter_defs_base"];
      },
    visitors
      {
        name = "endo_defs";
        variety = "endo";
        nude = true;
        visit_prefix = "on_";
        ancestors = ["endo_defs_base"];
      },
    visitors
      {
        name = "reduce_defs";
        variety = "reduce";
        nude = true;
        visit_prefix = "on_";
        ancestors = ["reduce_defs_base"];
      },
    visitors
      {
        name = "map_defs";
        variety = "map";
        nude = true;
        visit_prefix = "on_";
        ancestors = ["map_defs_base"];
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
      Core_kernel.Tuple.T2.compare
        ~cmp1:String.compare
        ~cmp2:String.compare
        (s1, s1')
        (s2, s2')
    | (SFlit_int _, _) -> -1
    | (SFlit_str _, SFlit_int _) -> 1
    | (SFlit_str _, _) -> -1
    | (SFclass_const _, _) -> 1

  let equal x y = Core_kernel.Int.equal 0 (compare x y)
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

module ShapeSet = Set.Make (ShapeField)

(** Literal values that can occur in XHP enum properties.
 *
 * class :my-xhp-class {
 *   attribute enum {'big', 'small'} my-prop;
 * }
 *)
type xhp_enum_value =
  | XEV_Int of int
  | XEV_String of string
[@@deriving eq, show]
