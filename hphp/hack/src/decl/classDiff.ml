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
let change_affects_descendants member_change =
  match member_change with
  | Added
  | Removed
  | Changed_inheritance ->
    true
  | Modified
  | Private_change ->
    false

type member_diff = {
  consts: member_change SMap.t;
  typeconsts: member_change SMap.t;
  props: member_change SMap.t;
  sprops: member_change SMap.t;
  methods: member_change SMap.t;
  smethods: member_change SMap.t;
  constructor: member_change option;
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

type minor_change = {
  mro_positions_changed: bool;
  member_diff: member_diff;
}
[@@deriving eq, show { with_path = false }]

type t =
  | Unchanged
  | Major_change
  | Minor_change of minor_change
[@@deriving eq, show { with_path = false }]
