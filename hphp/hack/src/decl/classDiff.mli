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
  | Private_change_not_in_trait (* Added/removed a private member *)
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
        (** The order has changed if there exists x and y
            such that x comes before y in the list pre-change,
            but y comes before x in the list post-change,
            i.e. if there exists two elements which are in both lists
            and have been swapped. *)
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

module KindChange : sig
  type t = { new_kind: Ast_defs.classish_kind } [@@deriving eq, show]
end

module BoolChange : sig
  type t =
    | Became
    | No_more
  [@@deriving eq, show]
end

module ValueDiff : sig
  type 'value t = {
    old_value: 'value;
    new_value: 'value;
  }
  [@@deriving eq, show]
end

type enum_type_change = {
  base_change: Typing_defs.decl_ty ValueDiff.t option;
  constraint_change: Typing_defs.decl_ty ValueDiff.t ValueChange.t option;
  includes_change: unit NamedItemsListChange.t NamedItemsListChange.t option;
}
[@@deriving eq, show]

type class_shell_change = {
  old_classish_kind: Ast_defs.classish_kind;
      (** The new classish kind will be in kind_change if it has changed. *)
  parent_changes: parent_changes option;
  type_parameters_change: unit NamedItemsListChange.t option;
  kind_change: KindChange.t option;
  final_change: BoolChange.t option;
  abstract_change: BoolChange.t option;
  is_xhp_change: BoolChange.t option;
  internal_change: BoolChange.t option;
  has_xhp_keyword_change: BoolChange.t option;
  support_dynamic_type_change: BoolChange.t option;
  module_change: unit ValueChange.t option;
  xhp_enum_values_change: bool;
  user_attributes_changes:
    unit NamedItemsListChange.t NamedItemsListChange.t option;
  enum_type_change: enum_type_change ValueChange.t option;
}
[@@deriving eq, show]

type unknown_kind =
  | Old_decl_not_found
  | New_decl_not_found
  | Neither_found
[@@deriving eq, show]

module MajorChange : sig
  type t =
    | Unknown of unknown_kind
    | Added
    | Removed
    | Modified of class_shell_change * member_diff
  [@@deriving eq, show]
end

type t =
  | Major_change of MajorChange.t
  | Minor_change of member_diff
[@@deriving eq, show]

val pretty : name:string -> t -> string

val empty_member_diff : member_diff

val is_empty_member_diff : member_diff -> bool

(** Wether a change on a method or property affects the descendants.
  This is the case for added and removed members, but also if e.g. abstractness or visibility
  has changed. *)
val method_or_property_change_affects_descendants : member_change -> bool

(** The maximum of two constructor changes is the change which has the largest fanout. *)
val max_constructor_change :
  constructor_change -> constructor_change -> constructor_change

val to_category_json : t -> Yojson.Safe.t
