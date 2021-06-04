(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* This `.mli` file was generated automatically. It may include extra
definitions that should not actually be exposed to the caller. If you notice
that this interface file is a poor interface, please take a few minutes to
clean it up manually, and then delete this comment once the interface is in
shape. *)

type override_info = {
  class_name: string;
  method_name: string;
  is_static: bool;
}
[@@deriving eq]

type class_id_type =
  | ClassId
  | Other
[@@deriving ord, eq]

type kind =
  | Class of class_id_type
  | Record
  | Function
  | Method of string * string
  | LocalVar
  | Property of string * string
  (*
    XhpLiteralAttr is only used for attributes in XHP literals.
    i.e.
        <foo:bar my-attribute={} />
    For all other cases, Property is used.
    i.e.
        $x->:my-attribute
        or attributes in class definitions
   *)
  | XhpLiteralAttr of string * string
  | ClassConst of string * string
  | Typeconst of string * string
  | GConst
  | Attribute of override_info option
  | EnumClassLabel of string * string
[@@deriving eq]

type 'a t = {
  name: string;
  type_: kind;
  is_declaration: bool;
  (* Span of the symbol itself *)
  pos: 'a Pos.pos;
}
[@@deriving ord]

val to_absolute : Relative_path.t t -> string t

val kind_to_string : kind -> string

val enclosing_class : 'a t -> string option

val get_class_name : 'a t -> string option

val is_constructor : 'a t -> bool

val is_class : 'a t -> bool

val is_xhp_literal_attr : 'a t -> bool
