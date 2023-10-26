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
[@@deriving eq, show { with_path = false }]

type si_addendum = {
  sia_name: string;
      (** This is expected not to contain the leading namespace backslash! See [Utils.strip_ns]. *)
  sia_kind: si_kind;
  sia_is_abstract: bool;
  sia_is_final: bool;
}
[@@deriving show]

(** This is used as a filter on top-level symbol searches, for both autocomplete and symbol-search. *)
type autocomplete_type =
  | Acid
      (** satisfies [valid_for_acid], e.g. in autocomplete contexts like `|` at the start of a statement *)
  | Acclassish
      (** satisfies [valid_for_acclassish], currently only used for `nameof` *)
  | Acnew
      (** satisfies [valid_for_acnew] AND isn't an abstract class, e.g. in autocomplete contexts like `$x = new |` *)
  | Actype
      (** satisfies [valid_for_actype], e.g. in autocomplete contexts like `Foo<|` *)
  | Actrait_only
      (** is [SI_Trait], e.g. in autocomplete contexts like `uses |` *)
  | Ac_workspace_symbol  (** isn't [SI_Namespace]; repo-wide symbol search *)
[@@deriving eq, show { with_path = false }]

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

type si_complete =
  | Complete
  | Incomplete
[@@deriving eq]

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

(** Acclassish represents entities that parse as ClassishDeclaration *)
let valid_for_acclassish = function
  | SI_Class
  | SI_Interface
  | SI_Enum
  | SI_Trait
  | SI_XHP ->
    true
  | _ -> false

(** ACNEW represents instantiation of an object. (Caller should also verify that it's not abstract.) *)
let valid_for_acnew (kind : si_kind) : bool =
  match kind with
  | SI_Class
  | SI_XHP ->
    true
  | _ -> false

module Find_refs = struct
  type member =
    | Method of string
    | Property of string
    | Class_const of string
    | Typeconst of string
  [@@deriving show]

  type action =
    | Class of string
        (** When user does find-all-refs on Foo::f or self::f or parent::f, it produces `Class "Foo"`
        and passes this to serverFindRefs. As expected, it will find all references to the class Foo,
        including self:: and parent:: references.
        Class is used for all typelikes -- typedefs, enums, enum classes, classes, interfaces, traits *)
    | ExplicitClass of string
        (** When user does rename of Foo to Bar, then serverRename produces `ExplicitClass "Foo"`
        and passes this to serverFindRefs. This will only find the explicit references to the class Foo,
        not including self:: and parent::, since they shouldn't be renamed. *)
    | Member of string * member
    | Function of string
    | GConst of string
    | LocalVar of {
        filename: Relative_path.t;
        file_content: string;
        line: int;
        char: int;
      }
  [@@deriving show]
end
