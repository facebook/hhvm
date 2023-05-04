(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* These are types separated from [searchUtils.ml] to be oxidized (converted to Rust). *)

(* Flattened enum that contains one element for each type of symbol *)
type si_kind =
  | SI_Class
  | SI_Interface
  | SI_Enum
  | SI_Trait
  | SI_Unknown
  | SI_Mixed
  | SI_Function
  | SI_Typedef
  | SI_GlobalConstant
  | SI_XHP
  | SI_Namespace
  | SI_ClassMethod
  | SI_Literal
  | SI_ClassConstant
  | SI_Property
  | SI_LocalVariable
  | SI_Keyword
  | SI_Constructor
[@@deriving eq, show]

(* An [si_fullitem] without the filepath, used mainly for FFI. *)
type si_addendum = {
  (* This is expected not to contain the leading namespace backslash! See
     [Utils.strip_ns]. *)
  sia_name: string;
  sia_kind: si_kind;
  sia_is_abstract: bool;
  sia_is_final: bool;
}
[@@deriving show]
