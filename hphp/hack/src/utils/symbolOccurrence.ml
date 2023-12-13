(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type override_info = {
  class_name: string;
  method_name: string;
  is_static: bool;
}
[@@deriving ord, eq, show]

type class_id_type =
  | ClassId
  | Other
[@@deriving ord, eq, show]

type receiver_class =
  | ClassName of string
  | UnknownClass (* invoked dynamically *)
[@@deriving ord, eq, show]

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
[@@deriving ord, eq, show]

type built_in_type_hint =
  | BIprimitive of Aast_defs.tprim
  | BImixed
  | BIdynamic
  | BInothing
  | BInonnull
  | BIshape
  (* TODO: support self and static too.*)
  | BIthis
  | BIoption
[@@deriving ord, eq, show]

type receiver =
  | FunctionReceiver of string
  | MethodReceiver of {
      cls_name: string;
      meth_name: string;
      is_static: bool;
    }
[@@deriving ord, eq, show]

type kind =
  | Class of class_id_type
  | BuiltInType of built_in_type_hint
  | Function
  | Method of receiver_class * string
  | LocalVar
  | TypeVar
  | Property of receiver_class * string
  | XhpLiteralAttr of string * string
  | ClassConst of receiver_class * string
  | Typeconst of string * string
  | GConst
  (* For __Override occurrences, we track the associated method and class. *)
  | Attribute of override_info option
  (* enum class name, label name *)
  | EnumClassLabel of string * string
  | Keyword of keyword_with_hover_docs
  | PureFunctionContext
  (* The nth argument of receiver, used for looking up the parameter
     name at the declaration site. Zero-indexed.

     This is purely for IDE hover information, and is only used when
     we can easily find a concrete receiver (e.g. no complex generics). *)
  | BestEffortArgument of receiver * int
  | HhFixme
  | Module
[@@deriving ord, eq, show]

type 'a t = {
  name: string;
  type_: kind;
  is_declaration: bool;
  (* Span of the symbol itself *)
  pos: 'a Pos.pos;
}
[@@deriving ord, show]

let to_absolute x = { x with pos = Pos.to_absolute x.pos }

let kind_to_string = function
  | Class _ -> "class"
  | BuiltInType _ -> "built_in_type"
  | Method _ -> "method"
  | Function -> "function"
  | LocalVar -> "local"
  | Property _ -> "property"
  | XhpLiteralAttr _ -> "xhp_literal_attribute"
  | ClassConst _ -> "member_const"
  | Typeconst _ -> "typeconst"
  | GConst -> "global_const"
  | TypeVar -> "generic_type_var"
  | Attribute _ -> "attribute"
  | EnumClassLabel _ -> "enum_class_label"
  | Keyword _ -> "keyword"
  | PureFunctionContext -> "context_braces"
  | BestEffortArgument _ -> "argument"
  | HhFixme -> "hh_fixme"
  | Module -> "module"

let enclosing_class occurrence =
  match occurrence.type_ with
  | Method (ClassName c, _)
  | Property (ClassName c, _)
  | XhpLiteralAttr (c, _)
  | ClassConst (ClassName c, _)
  | Typeconst (c, _)
  | EnumClassLabel (c, _) ->
    Some c
  | _ -> None

let get_class_name occurrence =
  match enclosing_class occurrence with
  | Some _ as res -> res
  | None ->
    (match occurrence.type_ with
    | Class _
    | Attribute _ ->
      Some occurrence.name
    | _ -> None)

let is_constructor occurrence =
  match occurrence.type_ with
  | Method (_, name) when name = Naming_special_names.Members.__construct ->
    true
  | _ -> false

let is_class occurrence =
  match occurrence.type_ with
  | Class _ -> true
  | _ -> false

let is_xhp_literal_attr occurrence =
  match occurrence.type_ with
  | XhpLiteralAttr _ -> true
  | _ -> false

let built_in_type_hover (bt : built_in_type_hint) : string =
  match bt with
  | BIprimitive prim ->
    (match prim with
    | Aast_defs.Tnull -> "The value `null`. The opposite of `nonnull`."
    | Aast_defs.Tvoid ->
      "A `void` return type indicates a function that never returns a value. `void` functions usually have side effects."
    | Aast_defs.Tint -> "A 64-bit integer."
    | Aast_defs.Tbool -> "A boolean value, `true` or `false`."
    | Aast_defs.Tfloat -> "A 64-bit floating-point number."
    | Aast_defs.Tstring -> "A sequence of characters."
    | Aast_defs.Tresource ->
      "An external resource, such as a file handle or database connection."
    | Aast_defs.Tnum -> "An `int` or a `float`."
    | Aast_defs.Tarraykey ->
      "An `int` or a `string`. `arraykey` is a common key type for `dict`s."
    | Aast_defs.Tnoreturn ->
      "A `noreturn` function always errors or loops forever.")
  | BImixed ->
    "Any value at all. It's usually better to use a more specific type, or a generic."
  | BIdynamic ->
    "Any value at all. Unlike `mixed`, the type checker allows any operation on a `dynamic` value, even if e.g. a method doesn't actually exist.\n\n"
    ^ "All operations on a `dynamic` value return another `dynamic` value. Prefer more specific types so the type checker can help you.\n\n"
    ^ "To convert a `generic` value to something specific, use `$foo as SomeSpecificType`. This will check the type at runtime and the "
    ^ "type checker will verify types after this point."
  | BInothing ->
    "The `nothing` type has no values, it's the empty type.\n\nA function returning `nothing` either loops forever or throws an exception. A `vec<nothing>` is always empty."
  | BInonnull -> "Any value except `null`."
  | BIshape ->
    "A shape is a key-value data structure where the keys are known."
    ^ " Shapes are value types, just like `dict` and `vec`.\n\n"
    ^ " A closed shape, such as `shape('x' => int)`, has a fixed number of keys. "
    ^ " An open shape, such as `shape('x' => int, ...)`, may have additional keys."
  | BIthis ->
    "`this` represents the current class.\n\n"
    ^ "`this` refers to the type of the current instance at runtime. In a child class, `this` refers to the child class, even if the method is defined in the parent."
  | BIoption -> "The type `?Foo` allows either `Foo` or `null`."
