(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(*****************************************************************************)
(* Constants *)
(*****************************************************************************)

type cst_kind =
  (* The constant was introduced with: define('X', ...); *)
  | Cst_define
  (* The constant was introduced with: const X = ...; *)
  | Cst_const
  [@@deriving show]

(*****************************************************************************)
(* The Abstract Syntax Tree *)
(*****************************************************************************)

type id = Pos.t * string [@@deriving show]
type pstring = Pos.t * string [@@deriving show]

type variance =
  | Covariant
  | Contravariant
  | Invariant
  [@@deriving show]

type ns_kind =
  | NSNamespace
  | NSClass
  | NSClassAndNamespace
  | NSFun
  | NSConst
  [@@deriving show]

type constraint_kind =
  | Constraint_as
  | Constraint_eq
  | Constraint_super
  [@@deriving show]

type class_kind =
  | Cabstract
  | Cnormal
  | Cinterface
  | Ctrait
  | Cenum
  [@@deriving show]

type trait_req_kind =
  | MustExtend
  | MustImplement
  [@@deriving show]

type kind =
  | Final
  | Static
  | Abstract
  | Private
  | Public
  | Protected
  [@@deriving show]

type og_null_flavor =
  | OG_nullthrows
  | OG_nullsafe
  [@@deriving show]

type fun_decl_kind =
  | FDeclAsync
  | FDeclSync
  [@@deriving show]

type fun_kind =
  | FSync
  | FAsync
  | FGenerator
  | FAsyncGenerator
  [@@deriving show]

type shape_field_name =
  | SFlit of pstring
  | SFclass_const of id * pstring
  [@@deriving show]

type bop =
| Plus
| Minus | Star | Slash | Eqeq | EQeqeq | Starstar
| Diff | Diff2 | AMpamp | BArbar | LogXor | Lt
| Lte | Gt | Gte | Dot | Amp | Bar | Ltlt
| Gtgt | Percent | Xor | Cmp
| Eq of bop option
[@@deriving show]

type uop =
| Utild
| Unot | Uplus | Uminus | Uincr
| Udecr | Upincr | Updecr
| Uref | Usplat | Usilence
[@@deriving show]

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let string_of_class_kind = function
  | Cabstract -> "an abstract class"
  | Cnormal -> "a class"
  | Cinterface -> "an interface"
  | Ctrait -> "a trait"
  | Cenum -> "an enum"

let string_of_kind = function
  | Final -> "final"
  | Static -> "static"
  | Abstract -> "abstract"
  | Private -> "private"
  | Public -> "public"
  | Protected -> "protected"

module ShapeField = struct
  type t = shape_field_name
  (* We include span information in shape_field_name to improve error
   * messages, but we don't want it being used in the comparison, so
   * we have to write our own compare. *)
  let compare x y =
    match x, y with
      | SFlit _, SFclass_const _ -> -1
      | SFclass_const _, SFlit _ -> 1
      | SFlit (_, s1), SFlit (_, s2) -> Pervasives.compare s1 s2
      | SFclass_const ((_, s1), (_, s1')),
        SFclass_const ((_, s2), (_, s2')) ->
        Pervasives.compare (s1, s1') (s2, s2')
end

module ShapeMap = struct
  include MyMap.Make (ShapeField)

  let map_and_rekey m f1 f2 =
    fold (fun k v acc -> add (f1 k) (f2 v) acc) m empty

  let pp _ fmt _ = Format.pp_print_string fmt "[ShapeMap]"
end
