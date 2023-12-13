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

type receiver_class =
  | ClassName of string
  | UnknownClass (* invoked dynamically *)
[@@deriving ord, eq]

type keyword_with_hover_docs =
  | Class
  | Interface
  | Trait
  | Enum
  | EnumClass
  | Type
  | Newtype
  | FinalOnClass
  | FinalOnMethod
  | AbstractOnClass
  | AbstractOnMethod
  | ExtendsOnClass
  | ExtendsOnInterface
  | ReadonlyOnMethod
  | ReadonlyOnParameter
  | ReadonlyOnReturnType
  | ReadonlyOnExpression
  | XhpAttribute
  | XhpChildren
  | ConstGlobal
  | ConstOnClass
  | ConstType
  | StaticOnMethod
  | StaticOnProperty
  | Use
  | FunctionGlobal
  | FunctionOnMethod
  | Async
  | AsyncBlock
  | Await
  | Concurrent
  | Public
  | Protected
  | Private
  | Internal
  | ModuleInModuleDeclaration
  | ModuleInModuleMembershipDeclaration
[@@deriving ord, eq]

type built_in_type_hint =
  | BIprimitive of Aast_defs.tprim
  | BImixed
  | BIdynamic
  | BInothing
  | BInonnull
  | BIshape
  | BIthis
  | BIoption
[@@deriving eq]

type receiver =
  | FunctionReceiver of string
  | MethodReceiver of {
      cls_name: string;
      meth_name: string;
      is_static: bool;
    }
[@@deriving eq]

type kind =
  | Class of class_id_type
  | BuiltInType of built_in_type_hint
  | Function
  | Method of receiver_class * string
  | LocalVar
  | TypeVar
  | Property of receiver_class * string
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
  | ClassConst of receiver_class * string
  | Typeconst of string * string
  | GConst
  | Attribute of override_info option
  | EnumClassLabel of string * string
  | Keyword of keyword_with_hover_docs
  | PureFunctionContext
  | BestEffortArgument of receiver * int
  | HhFixme
  | Module
[@@deriving eq, show]

type 'a t = {
  name: string;
  type_: kind;
  is_declaration: bool;
  (* Span of the symbol itself *)
  pos: 'a Pos.pos;
}
[@@deriving ord, show]

val to_absolute : Relative_path.t t -> string t

val kind_to_string : kind -> string

val enclosing_class : 'a t -> string option

val get_class_name : 'a t -> string option

val is_constructor : 'a t -> bool

val is_class : 'a t -> bool

val is_xhp_literal_attr : 'a t -> bool

val built_in_type_hover : built_in_type_hint -> string
