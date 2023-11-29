(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Error_code : sig
  type t
end

type visibility =
  | Vprivate
  | Vpublic
  | Vinternal
  | Vprotected

type return_only_hint =
  | Hvoid
  | Hnoreturn

type unsupported_feature =
  | Ft_where_constraints
  | Ft_constraints
  | Ft_reification
  | Ft_user_attrs
  | Ft_variance

type t =
  | Unsupported_trait_use_as of Pos.t
  | Unsupported_instead_of of Pos.t
  | Unexpected_arrow of {
      pos: Pos.t;
      cname: string;
    }
  | Missing_arrow of {
      pos: Pos.t;
      cname: string;
    }
  | Disallowed_xhp_type of {
      pos: Pos.t;
      ty_name: string;
    }
  | Name_is_reserved of {
      pos: Pos.t;
      name: string;
    }
  | Dollardollar_unused of Pos.t
  | Method_name_already_bound of {
      pos: Pos.t;
      meth_name: string;
    }
  | Error_name_already_bound of {
      pos: Pos.t;
      name: string;
      prev_pos: Pos.t;
    }
  | Unbound_name of {
      pos: Pos.t;
      name: string;
      kind: Name_context.t;
    }
  | Invalid_fun_pointer of {
      pos: Pos.t;
      name: string;
    }
  | Undefined of {
      pos: Pos.t;
      var_name: string;
      did_you_mean: (string * Pos.t) option;
    }
  | Undefined_in_expr_tree of {
      pos: Pos.t;
      var_name: string;
      dsl: string option;
      did_you_mean: (string * Pos.t) option;
    }
  | This_reserved of Pos.t
  | Start_with_T of Pos.t
  | Already_bound of {
      pos: Pos.t;
      name: string;
    }
  | Unexpected_typedef of {
      pos: Pos.t;
      decl_pos: Pos.t;
      expected_kind: Name_context.t;
    }
  | Field_name_already_bound of Pos.t
  | Primitive_top_level of Pos.t
  | Primitive_invalid_alias of {
      pos: Pos.t;
      ty_name_used: string;
      ty_name_canon: string;
    }
  | Dynamic_new_in_strict_mode of Pos.t
  | Invalid_type_access_root of {
      pos: Pos.t;
      id: string option;
    }
  | Duplicate_user_attribute of {
      pos: Pos.t;
      attr_name: string;
      prev_pos: Pos.t;
    }
  | Invalid_memoize_label of {
      pos: Pos.t;
      attr_name: string;
    }
  | Unbound_attribute_name of {
      pos: Pos.t;
      attr_name: string;
      closest_attr_name: string option;
    }
  | This_no_argument of Pos.t
  | Object_cast of Pos.t
  | This_hint_outside_class of Pos.t
  | Parent_outside_class of Pos.t
  | Self_outside_class of Pos.t
  | Static_outside_class of Pos.t
  | This_type_forbidden of {
      pos: Pos.t;
      in_extends: bool;
      in_req_extends: bool;
    }
  | Nonstatic_property_with_lsb of Pos.t
  | Lowercase_this of {
      pos: Pos.t;
      ty_name: string;
    }
  | Classname_param of Pos.t
  | Tparam_applied_to_type of {
      pos: Pos.t;
      tparam_name: string;
    }
  | Tparam_with_tparam of {
      pos: Pos.t;
      tparam_name: string;
    }
  | Shadowed_tparam of {
      pos: Pos.t;
      tparam_name: string;
      prev_pos: Pos.t;
    }
  | Missing_typehint of Pos.t
  | Expected_variable of Pos.t
  | Too_many_arguments of Pos.t
  | Too_few_arguments of Pos.t
  | Expected_collection of {
      pos: Pos.t;
      cname: string;
    }
  | Illegal_CLASS of Pos.t
  | Illegal_TRAIT of Pos.t
  | Illegal_fun of Pos.t
  | Illegal_member_variable_class of Pos.t
  | Illegal_meth_fun of Pos.t
  | Illegal_inst_meth of Pos.t
  | Illegal_meth_caller of Pos.t
  | Illegal_class_meth of Pos.t
  | Lvar_in_obj_get of {
      pos: Pos.t;
      lvar_pos: Pos.t;
      lvar_name: string;
    }
  | Class_meth_non_final_self of {
      pos: Pos.t;
      class_name: string;
    }
  | Class_meth_non_final_CLASS of {
      pos: Pos.t;
      class_name: string;
      is_trait: bool;
    }
  | Const_without_typehint of {
      pos: Pos.t;
      const_name: string;
      ty_name: string;
    }
  | Prop_without_typehint of {
      pos: Pos.t;
      prop_name: string;
      vis: visibility;
    }
  | Illegal_constant of Pos.t
  | Invalid_require_implements of Pos.t
  | Invalid_require_extends of Pos.t
  | Invalid_require_class of Pos.t
  | Did_you_mean of {
      pos: Pos.t;
      name: string;
      suggest_pos: Pos.t;
      suggest_name: string;
    }
  | Using_internal_class of {
      pos: Pos.t;
      class_name: string;
    }
  | Too_few_type_arguments of Pos.t
  | Dynamic_class_name_in_strict_mode of Pos.t
  | Xhp_optional_required_attr of {
      pos: Pos.t;
      attr_name: string;
    }
  | Xhp_required_with_default of {
      pos: Pos.t;
      attr_name: string;
    }
  | Array_typehints_disallowed of Pos.t
  | Wildcard_hint_disallowed of Pos.t
  | Wildcard_tparam_disallowed of Pos.t
  | Illegal_use_of_dynamically_callable of {
      attr_pos: Pos.t;
      meth_pos: Pos.t;
      vis: visibility;
    }
  | Parent_in_function_pointer of {
      pos: Pos.t;
      meth_name: string;
      parent_name: string option;
    }
  | Self_in_non_final_function_pointer of {
      pos: Pos.t;
      meth_name: string;
      class_name: string option;
    }
  | Invalid_wildcard_context of Pos.t
  | Return_only_typehint of {
      pos: Pos.t;
      kind: return_only_hint;
    }
  | Unexpected_type_arguments of Pos.t
  | Too_many_type_arguments of Pos.t
  | This_as_lexical_variable of Pos.t
  | HKT_unsupported_feature of {
      pos: Pos.t;
      because_nested: bool;
      var_name: string;
      feature: unsupported_feature;
    }
  | HKT_partial_application of {
      pos: Pos.t;
      count: int;
    }
  | HKT_wildcard of Pos.t
  | HKT_implicit_argument of {
      pos: Pos.t;
      decl_pos: Pos_or_decl.t;
      param_name: string;
    }
  | HKT_class_with_constraints_used of {
      pos: Pos.t;
      class_name: string;
    }
  | HKT_alias_with_implicit_constraints of {
      pos: Pos.t;
      typedef_pos: Pos_or_decl.t;
      used_class_in_def_pos: Pos_or_decl.t;
      typedef_name: string;
      typedef_tparam_name: string;
      used_class_in_def_name: string;
      used_class_tparam_name: string;
    }
  | Explicit_consistent_constructor of {
      pos: Pos.t;
      classish_kind: Ast_defs.classish_kind;
    }
  | Module_declaration_outside_allowed_files of Pos.t
  | Internal_module_level_trait of Pos.t
  | Dynamic_method_access of Pos.t
  | Deprecated_use of {
      pos: Pos.t;
      fn_name: string;
    }
  | Unnecessary_attribute of {
      pos: Pos.t;
      attr: string;
      class_pos: Pos.t;
      class_name: string;
      suggestion: string option;
    }
  | Tparam_non_shadowing_reuse of {
      pos: Pos.t;
      tparam_name: string;
    }
  | Dynamic_hint_disallowed of Pos.t
  | Illegal_typed_local of {
      join: bool;
      id_pos: Pos.t;
      id_name: string;
      def_pos: Pos.t;
    }
  | Toplevel_statement of Pos.t

val to_user_error : t -> (Pos.t, Pos_or_decl.t) User_error.t
