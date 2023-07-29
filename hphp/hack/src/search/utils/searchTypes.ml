(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

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

type si_addendum = {
  sia_name: string;
      (** This is expected not to contain the leading namespace backslash! See [Utils.strip_ns]. *)
  sia_kind: si_kind;
  sia_is_abstract: bool;
  sia_is_final: bool;
}
[@@deriving show]

(** The context in which autocomplete is being performed *)
type autocomplete_type =
  | Acid
  | Acnew
  | Actype
  | Actrait_only
  | Ac_workspace_symbol  (** Excludes namespaces; used for symbol search *)
[@@deriving eq, show]

type si_file =
  | SI_Filehash of string  (** string represent Int64 *)
  | SI_Path of Relative_path.t

type si_item = {
  si_name: string;
  si_kind: si_kind;
  si_file: si_file;
      (** needed so that local file deletes can "tombstone" the item *)
  si_fullname: string;
}

(**
 * Convert the enum to an integer whose value will not change accidentally.
 *
 * Although [@@deriving] would be a very ocaml-ish way to solve this same
 * problem, we would run the risk that anyone who edits the code to reorder
 * list of si_kinds, perhaps by adding something new in its correct
 * alphabetic order, would cause unexpected autocomplete behavior due to
 * mismatches between the [@@deriving] ordinal values and the integers
 * stored in sqlite.
 *)
let kind_to_int (kind : si_kind) : int =
  match kind with
  | SI_Class -> 1
  | SI_Interface -> 2
  | SI_Enum -> 3
  | SI_Trait -> 4
  | SI_Unknown -> 5
  | SI_Mixed -> 6
  | SI_Function -> 7
  | SI_Typedef -> 8
  | SI_GlobalConstant -> 9
  | SI_XHP -> 10
  | SI_Namespace -> 11
  | SI_ClassMethod -> 12
  | SI_Literal -> 13
  | SI_ClassConstant -> 14
  | SI_Property -> 15
  | SI_LocalVariable -> 16
  | SI_Keyword -> 17
  | SI_Constructor -> 18

(** Convert an integer back to an enum *)
let int_to_kind (kind_num : int) : si_kind =
  match kind_num with
  | 1 -> SI_Class
  | 2 -> SI_Interface
  | 3 -> SI_Enum
  | 4 -> SI_Trait
  | 5 -> SI_Unknown
  | 6 -> SI_Mixed
  | 7 -> SI_Function
  | 8 -> SI_Typedef
  | 9 -> SI_GlobalConstant
  | 10 -> SI_XHP
  | 11 -> SI_Namespace
  | 12 -> SI_ClassMethod
  | 13 -> SI_Literal
  | 14 -> SI_ClassConstant
  | 15 -> SI_Property
  | 16 -> SI_LocalVariable
  | 17 -> SI_Keyword
  | 18 -> SI_Constructor
  | _ -> SI_Unknown

(** ACID represents a statement.  Everything other than interfaces are valid *)
let valid_for_acid (kind : si_kind) : bool =
  match kind with
  | SI_Mixed
  | SI_Unknown
  | SI_Interface ->
    false
  | _ -> true

(** ACTYPE represents a type definition that can be passed as a parameter *)
let valid_for_actype (kind : si_kind) : bool =
  match kind with
  | SI_Mixed
  | SI_Unknown
  | SI_Trait
  | SI_Function
  | SI_GlobalConstant ->
    false
  | _ -> true

(** ACNEW represents instantiation of an object. (Caller should also verify that it's not abstract.) *)
let valid_for_acnew (kind : si_kind) : bool =
  match kind with
  | SI_Class
  | SI_Typedef
  | SI_XHP ->
    true
  | _ -> false
