(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type warn = private Warn_tag

type migrated = private Migrated_tag

type quickfix = {
  can_be_captured: bool;
  original_pos: Pos.t;
  replacement_pos: Pos.t;
}

module IsAsAlways = struct
  type kind =
    | Is_is_always_true
    | Is_is_always_false
    | As_always_succeeds of quickfix
    | As_always_fails

  type t = {
    kind: kind;
    lhs_ty: string;
    rhs_ty: string;
  }
end

module SketchyNullCheck = struct
  type kind =
    | Coalesce
    | Eq
    | Neq

  type t = {
    name: string option;
    kind: kind;
    ty: string;
  }
end

module NonDisjointCheck = struct
  type t = {
    name: string;
    ty1: string;
    ty2: string;
    dynamic: bool;
  }
end

module SketchyEquality = struct
  type t = {
    result: bool;
    left: Pos_or_decl.t Message.t list Lazy.t;
    right: Pos_or_decl.t Message.t list Lazy.t;
    left_trail: Pos_or_decl.t list;
    right_trail: Pos_or_decl.t list;
  }
end

module CastNonPrimitive = struct
  type t = {
    cast_hint: string;
    ty: string;
  }
end

module TruthinessTest = struct
  type sketchy_kind =
    | String
    | Arraykey
    | Stringish
    | Xhp_child
    | Traversable

  type kind =
    | Invalid of { truthy: bool }
    | Sketchy of sketchy_kind

  type t = {
    kind: kind;
    ty: string;
    expr: string option;
    not: bool;
  }
end

module EqualityCheck = struct
  type kind =
    | Equality of bool
    | Contains
    | Contains_key

  type t = {
    kind: kind;
    ty1: string;
    ty2: string;
  }
end

module DuplicateProperties = struct
  type t = {
    class_name: string;
    prop_name: string;
    class_names: string list;
    initialized_with_constant: bool;
  }
end

type (_, _) kind =
  | Sketchy_equality : (SketchyEquality.t, warn) kind
  | Is_as_always : (IsAsAlways.t, migrated) kind
  | Sketchy_null_check : (SketchyNullCheck.t, migrated) kind
  | Non_disjoint_check : (NonDisjointCheck.t, migrated) kind
  | Cast_non_primitive : (CastNonPrimitive.t, migrated) kind
  | Truthiness_test : (TruthinessTest.t, migrated) kind
  | Equality_check : (EqualityCheck.t, migrated) kind
  | Duplicate_properties : (DuplicateProperties.t, migrated) kind

type ('x, 'a) t = Pos.t * ('x, 'a) kind * 'x
