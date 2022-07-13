(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type member_change =
  | Added
  | Removed
  | Changed_inheritance (* Modified in a way that affects inheritance *)
  | Modified (* Modified in a way that does not affect inheritance *)
  | Private_change (* Added/removed a private member *)
[@@deriving eq, show]

type constructor_change = member_change option [@@deriving eq]

type member_diff = {
  consts: member_change SMap.t;
  typeconsts: member_change SMap.t;
  props: member_change SMap.t;
  sprops: member_change SMap.t;
  methods: member_change SMap.t;
  smethods: member_change SMap.t;
  constructor: constructor_change;
}
[@@deriving eq, show]

module ValueChange : sig
  type 'change t =
    | Added
    | Removed
    | Modified of 'change
  [@@deriving eq, show]
end

module NamedItemsListChange : sig
  type 'change t = {
    per_name_changes: 'change ValueChange.t SMap.t;
    order_change: bool;
  }
  [@@deriving eq, show]
end

type parent_changes = {
  extends_changes: unit NamedItemsListChange.t NamedItemsListChange.t option;
  implements_changes: unit NamedItemsListChange.t NamedItemsListChange.t option;
  req_extends_changes:
    unit NamedItemsListChange.t NamedItemsListChange.t option;
  req_implements_changes:
    unit NamedItemsListChange.t NamedItemsListChange.t option;
  req_class_changes: unit NamedItemsListChange.t NamedItemsListChange.t option;
  uses_changes: unit NamedItemsListChange.t NamedItemsListChange.t option;
  xhp_attr_changes: unit NamedItemsListChange.t NamedItemsListChange.t option;
}

type kind_change = {
  old_kind: Ast_defs.classish_kind;
  new_kind: Ast_defs.classish_kind;
}
[@@deriving eq, show]

type bool_change =
  | Became
  | No_more
[@@deriving eq, show]

type class_shell_change = {
  parent_changes: parent_changes option;
  type_parameters_change: unit NamedItemsListChange.t option;
  kind_change: kind_change option;
  final_change: bool_change option;
  abstract_change: bool_change option;
  is_xhp_change: bool_change option;
  internal_change: bool_change option;
  has_xhp_keyword_change: bool_change option;
  support_dynamic_type_change: bool_change option;
  module_change: unit ValueChange.t option;
  xhp_enum_values_change: bool;
  user_attributes_changes: unit NamedItemsListChange.t option;
  enum_type_change: unit ValueChange.t option;
}
[@@deriving eq, show]

module MajorChange : sig
  type t =
    | Unknown
    | Added
    | Removed
    | Modified of class_shell_change
  [@@deriving eq, show]
end

type minor_change = {
  mro_positions_changed: bool;
  member_diff: member_diff;
}
[@@deriving eq, show]

type t =
  | Unchanged
  | Major_change of MajorChange.t
  | Minor_change of minor_change
[@@deriving eq, show]

val empty_member_diff : member_diff

val is_empty_member_diff : member_diff -> bool

(** Wether a change on a method or property affects the descendants.
  This is the case for added and removed members, but also if e.g. abstractness or visibility
  has changed. *)
val method_or_property_change_affects_descendants : member_change -> bool

(** The maximum of two constructor changes is the change which has the largest fanout. *)
val max_constructor_change :
  constructor_change -> constructor_change -> constructor_change
