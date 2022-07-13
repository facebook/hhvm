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
[@@deriving eq, show { with_path = false }]

(** Order member_change values by corresponding fanout size. *)
let ord_member_change = function
  | Private_change -> 0
  | Modified -> 1
  | Changed_inheritance -> 2
  | Added -> 3
  | Removed -> 4

(** Compare member_change values by corresponding fanout size. *)
let compare_member_change x y = ord_member_change x - ord_member_change y

(* Returns true if the member changed in a way that affects what a descendant
   might inherit. This is trivially true for Added/Removed. The
   Changed_inheritance case captures situations where member metadata is used to
   select which member is used when a member of the same name is inherited from
   multiple parents.

   For example, if we have:

   class A { public function f(): void {} }
   trait T { public abstract function f(): void; }
   class B extends A { use T; }

   Then B inherits the concrete method A::f even though the trait T occurs first
   in B's linearization--we use the fact that A::f is concrete and T::f is not
   to choose which member to inherit in B. Since changes to the abstractness of
   A::f or T::f can affect what appears in B's member collection, we categorize
   these changes with Changed_inheritance.

   We must handle changes which affect descendants in this way differently from
   changes which do not because of how we record member dependencies in the
   dependency graph (see Shallow_class_fanout for this handling).

   Other changes to members are categorized with Modified. In these cases, we
   need to recheck uses of the member, but this is straightforward to do using
   the dependency graph.

   Adding or removing a private member can be handled in the same way as
   Modified. We need to recheck uses of the member (if any exist), but the
   presence of a private member does not affect descendants (descendants are not
   prevented from defining a member of the same name, since each class gets its
   own "namespace" for private members in the runtime).

   Changes in positions in members are not considered a change at all by this
   module, since we re-typecheck all files with errors for every global check
   (so errors with positions referring to moved members will be recomputed). *)
let method_or_property_change_affects_descendants member_change =
  match member_change with
  | Added
  | Removed
  | Changed_inheritance ->
    true
  | Modified
  | Private_change ->
    false

type constructor_change = member_change option [@@deriving eq, ord]

type member_diff = {
  consts: member_change SMap.t;
  typeconsts: member_change SMap.t;
  props: member_change SMap.t;
  sprops: member_change SMap.t;
  methods: member_change SMap.t;
  smethods: member_change SMap.t;
  constructor: constructor_change;
}
[@@deriving eq]

let empty_member_diff =
  {
    consts = SMap.empty;
    typeconsts = SMap.empty;
    props = SMap.empty;
    sprops = SMap.empty;
    smethods = SMap.empty;
    methods = SMap.empty;
    constructor = None;
  }

let is_empty_member_diff member_diff = member_diff = empty_member_diff

(* This is written explicitly instead of derived so that we can omit empty
   fields of the member_diff record (to make logs easier to read). *)
let pp_member_diff fmt member_diff =
  Format.fprintf fmt "@[<2>{";

  let sep = ref false in
  let pp_smap_field name data =
    if not (SMap.is_empty data) then (
      if !sep then
        Format.fprintf fmt ";@ "
      else (
        Format.fprintf fmt " ";
        sep := true
      );
      Format.fprintf fmt "@[%s =@ " name;
      SMap.pp pp_member_change fmt data;
      Format.fprintf fmt "@]"
    )
  in
  let { consts; typeconsts; props; sprops; methods; smethods; constructor } =
    member_diff
  in
  pp_smap_field "consts" consts;
  pp_smap_field "typeconsts" typeconsts;
  pp_smap_field "props" props;
  pp_smap_field "sprops" sprops;
  pp_smap_field "methods" methods;
  pp_smap_field "smethods" smethods;

  begin
    match constructor with
    | None -> ()
    | Some member_change ->
      if !sep then
        Format.fprintf fmt ";@ "
      else
        sep := true;
      Format.fprintf fmt "@[%s =@ " "constructor";
      pp_member_change fmt member_change;
      Format.fprintf fmt "@]"
  end;

  if !sep then Format.fprintf fmt "@ ";

  Format.fprintf fmt "}@]"

let show_member_diff member_diff =
  Format.asprintf "%a" pp_member_diff member_diff

(** The maximum of two constructor changes is the change which has the largest fanout. *)
let max_constructor_change left right =
  if compare_constructor_change left right >= 0 then
    left
  else
    right

module ValueChange = struct
  type 'change t =
    | Added
    | Removed
    | Modified of 'change
  [@@deriving eq, show { with_path = false }]
end

module NamedItemsListChange = struct
  type 'change t = {
    per_name_changes: 'change ValueChange.t SMap.t;
    order_change: bool;
  }
  [@@deriving eq, show { with_path = false }]
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
[@@deriving eq, show { with_path = false }]

type kind_change = {
  old_kind: Ast_defs.classish_kind;
  new_kind: Ast_defs.classish_kind;
}
[@@deriving eq, show { with_path = false }]

type bool_change =
  | Became
  | No_more
[@@deriving eq, show { with_path = false }]

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
[@@deriving eq, show { with_path = false }]

type minor_change = {
  mro_positions_changed: bool;
  member_diff: member_diff;
}
[@@deriving eq, show { with_path = false }]

module MajorChange = struct
  type t =
    | Unknown
    | Added
    | Removed
    | Modified of class_shell_change
  [@@deriving eq, show { with_path = false }]
end

type t =
  | Unchanged
  | Major_change of MajorChange.t
  | Minor_change of minor_change
[@@deriving eq, show { with_path = false }]
