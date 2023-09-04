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

   Then B inherits the concrete method A::f even though trait methods normally
   overwrite parent methods in folded methods tables--we use the fact that A::f
   is concrete and T::f is not to choose which member to inherit in B. Since
   changes to the abstractness of A::f or T::f can affect what appears in B's
   member collection, we categorize these changes with Changed_inheritance.

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

let constructor_change_to_json : constructor_change -> Hh_json.json =
  Hh_json.opt_ (fun x -> Hh_json.string_ @@ show_member_change x)

module MembersChangeCategory = struct
  type t = {
    some_added: bool;
    some_removed: bool;
    some_changed_inheritance: bool;
    some_modified: bool;
    some_private_change: bool;
  }

  let no_change =
    {
      some_added = false;
      some_removed = false;
      some_changed_inheritance = false;
      some_modified = false;
      some_private_change = false;
    }

  let of_member_change_map (changes : member_change SMap.t) : t option =
    if SMap.is_empty changes then
      None
    else
      Some
        (SMap.fold
           (fun _ change acc ->
             match change with
             | Added -> { acc with some_added = true }
             | Removed -> { acc with some_removed = true }
             | Changed_inheritance ->
               { acc with some_changed_inheritance = true }
             | Modified -> { acc with some_modified = true }
             | Private_change -> { acc with some_private_change = true })
           changes
           no_change)

  let to_json
      {
        some_added;
        some_removed;
        some_changed_inheritance;
        some_modified;
        some_private_change;
      } =
    Hh_json.JSON_Object
      [
        ("some_added", Hh_json.bool_ some_added);
        ("some_removed", Hh_json.bool_ some_removed);
        ("some_changed_inheritance", Hh_json.bool_ some_changed_inheritance);
        ("some_modified", Hh_json.bool_ some_modified);
        ("some_private_change", Hh_json.bool_ some_private_change);
      ]
end

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

module MemberDiffCategory = struct
  type t = {
    consts_category: MembersChangeCategory.t option;
    typeconsts_category: MembersChangeCategory.t option;
    props_category: MembersChangeCategory.t option;
    sprops_category: MembersChangeCategory.t option;
    methods_category: MembersChangeCategory.t option;
    smethods_category: MembersChangeCategory.t option;
    constructor_category: constructor_change;
  }

  let of_member_diff
      { consts; typeconsts; props; sprops; methods; smethods; constructor } =
    {
      consts_category = MembersChangeCategory.of_member_change_map consts;
      typeconsts_category =
        MembersChangeCategory.of_member_change_map typeconsts;
      props_category = MembersChangeCategory.of_member_change_map props;
      sprops_category = MembersChangeCategory.of_member_change_map sprops;
      methods_category = MembersChangeCategory.of_member_change_map methods;
      smethods_category = MembersChangeCategory.of_member_change_map smethods;
      constructor_category = constructor;
    }

  let to_json
      {
        consts_category;
        typeconsts_category;
        props_category;
        sprops_category;
        methods_category;
        smethods_category;
        constructor_category;
      } =
    Hh_json.JSON_Object
      [
        ("consts", Hh_json.opt_ MembersChangeCategory.to_json consts_category);
        ( "typeconsts",
          Hh_json.opt_ MembersChangeCategory.to_json typeconsts_category );
        ("props", Hh_json.opt_ MembersChangeCategory.to_json props_category);
        ("sprops", Hh_json.opt_ MembersChangeCategory.to_json sprops_category);
        ("methods", Hh_json.opt_ MembersChangeCategory.to_json methods_category);
        ( "smethods",
          Hh_json.opt_ MembersChangeCategory.to_json smethods_category );
        ("constructor", constructor_change_to_json constructor_category);
      ]
end

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

  let to_json change_to_json = function
    | Added -> Hh_json.string_ "Added"
    | Removed -> Hh_json.string_ "Removed"
    | Modified change ->
      Hh_json.JSON_Object [("Modified", change_to_json change)]

  let map ~f = function
    | Added -> Added
    | Removed -> Removed
    | Modified x -> Modified (f x)
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

let classish_kind_to_json kind =
  Hh_json.string_ @@ Ast_defs.show_classish_kind kind

module KindChange = struct
  type t = { new_kind: Ast_defs.classish_kind }
  [@@deriving eq, show { with_path = false }]

  let to_json { new_kind } =
    Hh_json.JSON_Object [("new_kind", classish_kind_to_json new_kind)]
end

module BoolChange = struct
  type t =
    | Became
    | No_more
  [@@deriving eq, show { with_path = false }]

  let to_json x = Hh_json.string_ @@ show x
end

module ValueDiff = struct
  type 'value t = {
    old_value: 'value;
    new_value: 'value;
  }
  [@@deriving eq, show { with_path = false }]
end

type enum_type_change = {
  base_change: Typing_defs.decl_ty ValueDiff.t option;
  constraint_change: Typing_defs.decl_ty ValueDiff.t ValueChange.t option;
  includes_change: unit NamedItemsListChange.t NamedItemsListChange.t option;
}
[@@deriving eq, show { with_path = false }]

type class_shell_change = {
  classish_kind: Ast_defs.classish_kind;
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
  user_attributes_changes: unit NamedItemsListChange.t option;
  enum_type_change: enum_type_change ValueChange.t option;
}
[@@deriving eq, show { with_path = false }]

module MajorChange = struct
  type t =
    | Unknown
    | Added
    | Removed
    | Modified of class_shell_change * member_diff
  [@@deriving eq, show { with_path = false }]
end

type t =
  | Unchanged
  | Major_change of MajorChange.t
  | Minor_change of member_diff
[@@deriving eq, show { with_path = false }]

let has_changed = function
  | Unchanged -> false
  | Major_change _
  | Minor_change _ ->
    true

let pretty ~(name : string) (diff : t) : string =
  let buf = Buffer.create 512 in
  let fmt = Format.formatter_of_buffer buf in
  Format.pp_set_margin fmt 120;
  Format.fprintf
    fmt
    "%s  @[<2>%s:@ %a@]@?"
    (String.make 35 ' ') (* indentation hack (width of log timestamp) *)
    (Utils.strip_ns name)
    pp
    diff;
  let diffstr = Buffer.contents buf in
  (* indentation hack *)
  Printf.sprintf "  %s" (Caml.String.trim diffstr)

module ClassShellChangeCategory = struct
  module ListChange = struct
    type t = {
      some_added: bool;
      some_removed: bool;
      some_modified: bool;
      order_change: bool;
    }

    let no_change : t =
      {
        some_added = false;
        some_removed = false;
        some_modified = false;
        order_change = false;
      }

    let of_list_change_map
        { NamedItemsListChange.per_name_changes = changes; order_change } =
      SMap.fold
        (fun _ change acc ->
          match change with
          | ValueChange.Added -> { acc with some_added = true }
          | ValueChange.Removed -> { acc with some_removed = true }
          | ValueChange.Modified _ -> { acc with some_modified = true })
        changes
        { no_change with order_change }

    let to_json { some_added; some_removed; some_modified; order_change } =
      Hh_json.JSON_Object
        [
          ("some_added", Hh_json.bool_ some_added);
          ("some_removed", Hh_json.bool_ some_removed);
          ("some_modified", Hh_json.bool_ some_modified);
          ("order_change", Hh_json.bool_ order_change);
        ]
  end

  module ParentsChangeCategory = struct
    type t = {
      extends_changes_category: ListChange.t option;
      implements_changes_category: ListChange.t option;
      req_extends_changes_category: ListChange.t option;
      req_implements_changes_category: ListChange.t option;
      req_class_changes_category: ListChange.t option;
      uses_changes_category: ListChange.t option;
      xhp_attr_changes_category: ListChange.t option;
    }

    let of_parents_change
        {
          extends_changes;
          implements_changes;
          req_extends_changes;
          req_implements_changes;
          req_class_changes;
          uses_changes;
          xhp_attr_changes;
        } =
      {
        extends_changes_category =
          Option.map ListChange.of_list_change_map extends_changes;
        implements_changes_category =
          Option.map ListChange.of_list_change_map implements_changes;
        req_extends_changes_category =
          Option.map ListChange.of_list_change_map req_extends_changes;
        req_implements_changes_category =
          Option.map ListChange.of_list_change_map req_implements_changes;
        req_class_changes_category =
          Option.map ListChange.of_list_change_map req_class_changes;
        uses_changes_category =
          Option.map ListChange.of_list_change_map uses_changes;
        xhp_attr_changes_category =
          Option.map ListChange.of_list_change_map xhp_attr_changes;
      }

    let to_json
        {
          extends_changes_category;
          implements_changes_category;
          req_extends_changes_category;
          req_implements_changes_category;
          req_class_changes_category;
          uses_changes_category;
          xhp_attr_changes_category;
        } =
      Hh_json.JSON_Object
        [
          ( "extends_changes",
            Hh_json.opt_ ListChange.to_json extends_changes_category );
          ( "implements_changes",
            Hh_json.opt_ ListChange.to_json implements_changes_category );
          ( "req_extends_changes",
            Hh_json.opt_ ListChange.to_json req_extends_changes_category );
          ( "req_implements_changes",
            Hh_json.opt_ ListChange.to_json req_implements_changes_category );
          ( "req_class_changes",
            Hh_json.opt_ ListChange.to_json req_class_changes_category );
          ("uses_changes", Hh_json.opt_ ListChange.to_json uses_changes_category);
          ( "xhp_attr_changes",
            Hh_json.opt_ ListChange.to_json xhp_attr_changes_category );
        ]
  end

  let unit_to_json () = Hh_json.JSON_Null

  module EnumTypeChangeCategory = struct
    type t = {
      has_base_change: bool;
      constraint_change_category: unit ValueChange.t option;
      includes_change_category: ListChange.t option;
    }

    let of_enum_type_change (change : enum_type_change) : t =
      let { base_change; constraint_change; includes_change } = change in
      {
        has_base_change = Option.is_some base_change;
        constraint_change_category =
          Option.map (ValueChange.map ~f:(fun _ -> ())) constraint_change;
        includes_change_category =
          Option.map ListChange.of_list_change_map includes_change;
      }

    let to_json
        {
          has_base_change;
          constraint_change_category;
          includes_change_category;
        } =
      Hh_json.JSON_Object
        [
          ("has_base_change", Hh_json.bool_ has_base_change);
          ( "constraint_change_category",
            Hh_json.opt_
              (ValueChange.to_json unit_to_json)
              constraint_change_category );
          ( "includes_change_category",
            Hh_json.opt_ ListChange.to_json includes_change_category );
        ]
  end

  type t = {
    classish_kind: Ast_defs.classish_kind;
    parent_changes_category: ParentsChangeCategory.t option;
    type_parameters_change_category: ListChange.t option;
    kind_change_category: KindChange.t option;
    final_change_category: BoolChange.t option;
    abstract_change_category: BoolChange.t option;
    is_xhp_change_category: BoolChange.t option;
    internal_change_category: BoolChange.t option;
    has_xhp_keyword_change_category: BoolChange.t option;
    support_dynamic_type_change_category: BoolChange.t option;
    module_change_category: unit ValueChange.t option;
    xhp_enum_values_change_category: bool;
    user_attributes_changes_category: ListChange.t option;
    enum_type_change_category: EnumTypeChangeCategory.t ValueChange.t option;
  }

  let of_class_shell_change
      ({
         classish_kind;
         parent_changes;
         type_parameters_change;
         kind_change;
         final_change;
         abstract_change;
         is_xhp_change;
         internal_change;
         has_xhp_keyword_change;
         support_dynamic_type_change;
         module_change;
         xhp_enum_values_change;
         user_attributes_changes;
         enum_type_change;
       } :
        class_shell_change) =
    {
      classish_kind;
      parent_changes_category =
        Option.map ParentsChangeCategory.of_parents_change parent_changes;
      type_parameters_change_category =
        Option.map ListChange.of_list_change_map type_parameters_change;
      kind_change_category = kind_change;
      final_change_category = final_change;
      abstract_change_category = abstract_change;
      is_xhp_change_category = is_xhp_change;
      internal_change_category = internal_change;
      has_xhp_keyword_change_category = has_xhp_keyword_change;
      support_dynamic_type_change_category = support_dynamic_type_change;
      module_change_category = module_change;
      xhp_enum_values_change_category = xhp_enum_values_change;
      user_attributes_changes_category =
        Option.map ListChange.of_list_change_map user_attributes_changes;
      enum_type_change_category =
        Option.map
          (ValueChange.map ~f:EnumTypeChangeCategory.of_enum_type_change)
          enum_type_change;
    }

  let to_json
      {
        classish_kind;
        parent_changes_category;
        type_parameters_change_category;
        kind_change_category;
        final_change_category;
        abstract_change_category;
        is_xhp_change_category;
        internal_change_category;
        has_xhp_keyword_change_category;
        support_dynamic_type_change_category;
        module_change_category;
        xhp_enum_values_change_category;
        user_attributes_changes_category;
        enum_type_change_category;
      } =
    let open Hh_json in
    JSON_Object
      [
        ("classish_kind", classish_kind_to_json classish_kind);
        ( "parent_changes",
          Hh_json.opt_ ParentsChangeCategory.to_json parent_changes_category );
        ( "type_parameters_change",
          Hh_json.opt_ ListChange.to_json type_parameters_change_category );
        ("kind_change", Hh_json.opt_ KindChange.to_json kind_change_category);
        ("final_change", Hh_json.opt_ BoolChange.to_json final_change_category);
        ( "abstract_change",
          Hh_json.opt_ BoolChange.to_json abstract_change_category );
        ("is_xhp_change", Hh_json.opt_ BoolChange.to_json is_xhp_change_category);
        ( "internal_change",
          Hh_json.opt_ BoolChange.to_json internal_change_category );
        ( "has_xhp_keyword_change",
          Hh_json.opt_ BoolChange.to_json has_xhp_keyword_change_category );
        ( "support_dynamic_type_change",
          Hh_json.opt_ BoolChange.to_json support_dynamic_type_change_category
        );
        ( "module_change",
          Hh_json.opt_ (ValueChange.to_json unit_to_json) module_change_category
        );
        ("xhp_enum_values_change", Hh_json.bool_ xhp_enum_values_change_category);
        ( "user_attributes_changes",
          Hh_json.opt_ ListChange.to_json user_attributes_changes_category );
        ( "enum_type_change",
          Hh_json.opt_
            (ValueChange.to_json EnumTypeChangeCategory.to_json)
            enum_type_change_category );
      ]
end

module MajorChangeCategory = struct
  type t =
    | Unknown
    | Added
    | Removed
    | Modified of ClassShellChangeCategory.t * MemberDiffCategory.t

  let of_major_change = function
    | MajorChange.Unknown -> Unknown
    | MajorChange.Added -> Added
    | MajorChange.Removed -> Removed
    | MajorChange.Modified (change, member_diff) ->
      Modified
        ( ClassShellChangeCategory.of_class_shell_change change,
          MemberDiffCategory.of_member_diff member_diff )

  let to_json = function
    | Unknown -> Hh_json.string_ "Unknown"
    | Added -> Hh_json.string_ "Added"
    | Removed -> Hh_json.string_ "Removed"
    | Modified (change, member_diff) ->
      Hh_json.JSON_Object
        [
          ( "Modified",
            Hh_json.JSON_Array
              [
                ClassShellChangeCategory.to_json change;
                MemberDiffCategory.to_json member_diff;
              ] );
        ]
end

module ChangeCategory = struct
  type t =
    | CUnchanged
    | CMajor_change of MajorChangeCategory.t
    | CMinor_change of MemberDiffCategory.t

  let of_change = function
    | Unchanged -> CUnchanged
    | Major_change change ->
      CMajor_change (MajorChangeCategory.of_major_change change)
    | Minor_change member_diff ->
      CMinor_change (MemberDiffCategory.of_member_diff member_diff)

  let to_json = function
    | CUnchanged -> Hh_json.string_ "Unchanged"
    | CMajor_change change ->
      Hh_json.JSON_Object [("Major_change", MajorChangeCategory.to_json change)]
    | CMinor_change change ->
      Hh_json.JSON_Object [("Minor_change", MemberDiffCategory.to_json change)]
end

let to_category_json x = ChangeCategory.to_json @@ ChangeCategory.of_change x
