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

module Is_as_always = struct
  type kind =
    | Is_is_always_true
    | Is_is_always_false
    | As_always_succeeds of quickfix
    | As_always_fails of { is_nullable: bool }

  type t = {
    kind: kind;
    lhs_ty: string;
    rhs_ty: string;
  }
end

module Null_coalesce_always = struct
  type kind =
    | Null_coalesce_always_null
    | Null_coalesce_never_null

  type t = {
    kind: kind;
    lhs_pos: Pos_or_decl.t;
    lhs_ty: string;
  }
end

module Sketchy_null_check = struct
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

module Non_disjoint_check = struct
  type t = {
    name: string;
    ty1: string;
    ty2: string;
    dynamic: bool;
  }
end

module Sketchy_equality = struct
  type t = {
    result: bool;
    left: Pos_or_decl.t Message.t list Lazy.t;
    right: Pos_or_decl.t Message.t list Lazy.t;
    left_trail: Pos_or_decl.t list;
    right_trail: Pos_or_decl.t list;
  }
end

module Cast_non_primitive = struct
  type t = {
    cast_hint: string;
    ty: string;
  }
end

module Truthiness_test = struct
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

module Equality_check = struct
  type kind =
    | Equality of bool
    | Switch
    | Contains
    | Contains_key

  type t = {
    kind: kind;
    ty1: string;
    ty2: string;
  }
end

module Duplicate_properties = struct
  type t = {
    class_name: string;
    prop_name: string;
    class_names: string list;
    initialized_with_constant: bool;
  }
end

module Class_pointer_to_string = struct
  type t = {
    pos: Pos.t;
    ty: string;
  }
end

module No_disjoint_union_check = struct
  type t = {
    disjuncts: (Pos_or_decl.t * string) list Lazy.t;
    tparam_pos: Pos_or_decl.t;
  }
end

module Switch_redundancy = struct
  type t =
    | SwitchHasRedundancy of {
        positions: Pos.t list Lazy.t;
        redundancy_size: int;
      }  (** Primary position on the switch *)
    | DuplicatedCase of {
        first_occurrence: Pos.t;
        case: string Lazy.t;
      }  (** Primary position is on the duplicate occurrence of the case *)
    | RedundantDefault  (** Primary position on the default *)
end

module Static_call_on_trait = struct
  type t = {
    meth_name: string;
    trait_name: string;
    trait_pos: Pos_or_decl.t;
  }
end

module Static_property_override = struct
  type t = {
    prop_name: string;
    child_prop_pos: Pos_or_decl.t;
  }
end

module String_to_class_pointer = struct
  type t = {
    cls_name: string;
    ty_pos: Pos_or_decl.t;
  }
end

module Call_needs_concrete = struct
  type t = {
    call_pos: Pos.t;
    class_name: string;
    meth_name: string;
    decl_pos: Pos_or_decl.t;
    via: [ `Id | `Self | `Parent | `Static ];
  }
end

module Abstract_access_via_static = struct
  type t = {
    access_pos: Pos.t;
    class_name: string;
    member_name: string;
    decl_pos: Pos_or_decl.t;
    containing_method_pos: Pos.t option;
  }
end

module Uninstantiable_class_via_static = struct
  type t = {
    usage_pos: Pos.t;
    class_name: string;
    decl_pos: Pos_or_decl.t;
  }
end

module Needs_concrete_override = struct
  type t = {
    pos: Pos_or_decl.t;
    parent_pos: Pos_or_decl.t;
    method_name_for_method_defined_outside_class: string option;
        (** `Some m` iff `m` is a trait method of a trait that is used by the class.
     * In such cases, the location for the warning will be a class name, so we need
     * to preserve the method name to give a meaningful error message.
     * Example:   `class Child extends Parent { use Tr; }`
     *                   ~~~~~
     * where trait `Tr` defines the method with the incorrect override of `Parent::m`
     *)
  }
end

module Expect_bool_for_condition = struct
  type t = { ty: string }
end

module Redundant_nullsafe_operation = struct
  type kind =
    | Redundant_nullsafe_member_select
    | Nullsafe_member_select_on_null
    | Redundant_nullsafe_pipe
    | Nullsafe_pipe_on_null

  type t = {
    kind: kind;
    ty: string;
  }
end

module Unbound_name_warning = struct
  type t = {
    name: string;
    kind_str: string;
  }
end

type (_, _) kind =
  | Sketchy_equality : (Sketchy_equality.t, warn) kind
  | Is_as_always : (Is_as_always.t, migrated) kind
  | Sketchy_null_check : (Sketchy_null_check.t, migrated) kind
  | Non_disjoint_check : (Non_disjoint_check.t, migrated) kind
  | Cast_non_primitive : (Cast_non_primitive.t, migrated) kind
  | Truthiness_test : (Truthiness_test.t, migrated) kind
  | Equality_check : (Equality_check.t, migrated) kind
  | Duplicate_properties : (Duplicate_properties.t, migrated) kind
  | Class_pointer_to_string : (Class_pointer_to_string.t, warn) kind
  | No_disjoint_union_check : (No_disjoint_union_check.t, warn) kind
  | Switch_redundancy : (Switch_redundancy.t, warn) kind
  | Static_call_on_trait : (Static_call_on_trait.t, warn) kind
  | Static_property_override : (Static_property_override.t, warn) kind
  | String_to_class_pointer : (String_to_class_pointer.t, warn) kind
  | Null_coalesce_always : (Null_coalesce_always.t, warn) kind
  | Call_needs_concrete : (Call_needs_concrete.t, warn) kind
  | Abstract_access_via_static : (Abstract_access_via_static.t, warn) kind
  | Uninstantiable_class_via_static
      : (Uninstantiable_class_via_static.t, warn) kind
  | Needs_concrete_override : (Needs_concrete_override.t, warn) kind
  | Expect_bool_for_condition : (Expect_bool_for_condition.t, warn) kind
  | Redundant_nullsafe_operation : (Redundant_nullsafe_operation.t, warn) kind
  | Unbound_name_warning : (Unbound_name_warning.t, warn) kind

type ('x, 'a) t = Pos.t * ('x, 'a) kind * 'x
