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
    | As_always_fails

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

type ('x, 'a) t = Pos.t * ('x, 'a) kind * 'x
