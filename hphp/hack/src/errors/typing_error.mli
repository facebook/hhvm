(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Error_code = Error_codes.Typing

module Eval_result : sig
  (* The type representing the result of evaluating a `Typing_error.t` to
     some atomic result type `'a` whilst retaining information about
     intersections of errors *)
  type 'a t

  val to_string : 'a t -> ('a -> string) -> string

  (* Given some predicate telling us whether an atomic error is suppressed,
     filter the result to ensure that we don't report intersections of errors
     where at least one error would not be reported (meaning that none should
     be reported).

     If there is some component of the  result which is suppressed, we will
     get at most one atomic result back. If not, we may get back arbitrary
     intersections of errors.

     examples

     1) suppress_intersection
         (Intersect
           [ Intersect
               [ Intersect
                  [ Single 2
                  ; Single 3
                  ]
                ; Single 1
                ]
            ; Single 4
            ]
          ) ~is_suppressed:(fun x -> x = 2)
          === Single 2

      2) suppress_intersection (Single 2) ~is_suppressed:(fun x -> x = 2)
         === Single 2


      3) suppress_intersection
          ( Intersect
             [ Single 1
             ; Intersect
                [ Single 3
                ; Intersect
                   [ Single 4
                   ; Single 5
                   ]
                ]
             ]
          )  ~is_suppressed:(fun x -> x = 2)
          === Intersect
                [ Single 1
                ; Intersect
                   [ Single 3
                   ; Intersect
                      [ Single 4
                      ; Single 5
                      ]
                   ]
                ]
  *)
  val suppress_intersection : 'a t -> is_suppressed:('a -> bool) -> 'a t

  val iter : 'a t -> f:('a -> unit) -> unit
end

module Error : sig
  type t

  val show : t -> string

  val pp : Format.formatter -> t -> unit
end

module Primary : sig
  module Shape : sig
    type t =
      | Invalid_shape_field_name of {
          pos: Pos.t;
          is_empty: bool;
        }
      | Invalid_shape_field_literal of {
          pos: Pos.t;
          witness_pos: Pos.t;
        }
      | Invalid_shape_field_const of {
          pos: Pos.t;
          witness_pos: Pos.t;
        }
      | Invalid_shape_field_type of {
          pos: Pos.t;
          ty_pos: Pos_or_decl.t;
          ty_name: string Lazy.t;
          trail: Pos_or_decl.t list;
        }
      | Shape_field_class_mismatch of {
          pos: Pos.t;
          class_name: string;
          witness_pos: Pos.t;
          witness_class_name: string;
        }
      | Shape_field_type_mismatch of {
          pos: Pos.t;
          ty_name: string Lazy.t;
          witness_pos: Pos.t;
          witness_ty_name: string Lazy.t;
        }
      | Invalid_shape_remove_key of Pos.t
      | Shapes_key_exists_always_true of {
          pos: Pos.t;
          field_name: string;
          decl_pos: Pos_or_decl.t;
        }
      | Shapes_key_exists_always_false of {
          pos: Pos.t;
          field_name: string;
          decl_pos: Pos_or_decl.t;
          reason:
            [ `Nothing of Pos_or_decl.t Message.t list Lazy.t | `Undefined ];
        }
      | Shapes_method_access_with_non_existent_field of {
          pos: Pos.t;
          field_name: string;
          decl_pos: Pos_or_decl.t;
          method_name: string;
          reason:
            [ `Nothing of Pos_or_decl.t Message.t list Lazy.t | `Undefined ];
        }
      | Shapes_access_with_non_existent_field of {
          pos: Pos.t;
          field_name: string;
          decl_pos: Pos_or_decl.t;
          reason:
            [ `Nothing of Pos_or_decl.t Message.t list Lazy.t | `Undefined ];
        }
  end

  module Enum : sig
    type t =
      | Enum_switch_redundant of {
          pos: Pos.t;
          first_pos: Pos.t;
          const_name: string;
        }
      | Enum_switch_nonexhaustive of {
          pos: Pos.t;
          kind: string;
          decl_pos: Pos_or_decl.t;
          missing: string list;
        }
      | Enum_switch_redundant_default of {
          pos: Pos.t;
          kind: string;
          decl_pos: Pos_or_decl.t;
        }
      | Enum_switch_not_const of Pos.t
      | Enum_switch_wrong_class of {
          pos: Pos.t;
          kind: string;
          expected: string;
          actual: string;
        }
      | Enum_type_bad of {
          pos: Pos.t;
          ty_name: string Lazy.t;
          is_enum_class: bool;
          trail: Pos_or_decl.t list;
        }
      | Enum_constant_type_bad of {
          pos: Pos.t;
          ty_pos: Pos_or_decl.t;
          ty_name: string Lazy.t;
          trail: Pos_or_decl.t list;
        }
      | Enum_type_typedef_nonnull of Pos.t
      | Enum_class_label_unknown of {
          pos: Pos.t;
          label_name: string;
          enum_name: string;
          decl_pos: Pos_or_decl.t;
          most_similar: (string * Pos_or_decl.t) option;
          ty_pos: Pos_or_decl.t option;
        }
      | Enum_class_label_as_expr of Pos.t
      | Enum_class_label_member_mismatch of {
          pos: Pos.t;
          label: string;
          expected_ty_msg_opt: Pos_or_decl.t Message.t list Lazy.t option;
        }
      | Incompatible_enum_inclusion_base of {
          pos: Pos.t;
          classish_name: string;
          src_classish_name: string;
        }
      | Incompatible_enum_inclusion_constraint of {
          pos: Pos.t;
          classish_name: string;
          src_classish_name: string;
        }
      | Enum_inclusion_not_enum of {
          pos: Pos.t;
          classish_name: string;
          src_classish_name: string;
        }
      | Enum_classes_reserved_syntax of Pos.t
      | Enum_supertyping_reserved_syntax of Pos.t
  end

  module Expr_tree : sig
    type t =
      | Expression_tree_non_public_member of {
          pos: Pos.t;
          decl_pos: Pos_or_decl.t;
        }
      | Reified_static_method_in_expr_tree of Pos.t
      | This_var_in_expr_tree of Pos.t
      | Experimental_expression_trees of Pos.t
      | Expression_tree_unsupported_operator of {
          pos: Pos.t;
          member_name: string;
          class_name: string;
        }
  end

  module Readonly : sig
    type t =
      | Readonly_modified of {
          pos: Pos.t;
          reason_opt: Pos_or_decl.t Message.t Lazy.t option;
        }
      | Readonly_mismatch of {
          pos: Pos.t;
          what: [ `arg_readonly | `arg_mut | `collection_mod | `prop_assign ];
          pos_sub: Pos_or_decl.t;
          pos_super: Pos_or_decl.t;
        }
      | Readonly_invalid_as_mut of Pos.t
      | Readonly_exception of Pos.t
      | Explicit_readonly_cast of {
          pos: Pos.t;
          decl_pos: Pos_or_decl.t;
          kind: [ `fn_call | `property | `static_property ];
        }
      | Readonly_method_call of {
          pos: Pos.t;
          decl_pos: Pos_or_decl.t;
        }
      | Readonly_closure_call of {
          pos: Pos.t;
          decl_pos: Pos_or_decl.t;
          suggestion: string;
        }
  end

  module Ifc : sig
    type t =
      | Illegal_information_flow of {
          pos: Pos.t;
          secondaries: Pos_or_decl.t list;
          source_poss: Pos_or_decl.t list;
          source: string;
          sink_poss: Pos_or_decl.t list;
          sink: string;
        }
      | Context_implicit_policy_leakage of {
          pos: Pos.t;
          secondaries: Pos_or_decl.t list;
          source_poss: Pos_or_decl.t list;
          source: string;
          sink_poss: Pos_or_decl.t list;
          sink: string;
        }
      | Unknown_information_flow of {
          pos: Pos.t;
          what: string;
        }
      | Ifc_internal_error of {
          pos: Pos.t;
          msg: string;
        }
  end

  module Coeffect : sig
    type t =
      | Call_coeffect of {
          pos: Pos.t;
          available_pos: Pos_or_decl.t;
          available_incl_unsafe: string Lazy.t;
          required_pos: Pos_or_decl.t;
          required: string Lazy.t;
        }
      | Op_coeffect_error of {
          pos: Pos.t;
          op_name: string;
          locally_available: string Lazy.t;
          available_pos: Pos_or_decl.t;
          err_code: Error_code.t;
          required: string Lazy.t;
          suggestion: Pos_or_decl.t Message.t list Lazy.t option;
        }
  end

  module Wellformedness : sig
    type t =
      | Missing_return of {
          pos: Pos.t;
          hint_pos: Pos_or_decl.t option;
          is_async: bool;
        }
      | Void_usage of {
          pos: Pos.t;
          reason: Pos_or_decl.t Message.t list Lazy.t;
        }
      | Noreturn_usage of {
          pos: Pos.t;
          reason: Pos_or_decl.t Message.t list Lazy.t;
        }
      | Returns_with_and_without_value of {
          pos: Pos.t;
          with_value_pos: Pos.t;
          without_value_pos_opt: Pos.t option;
        }
      | Missing_assign of Pos.t
      | Non_void_annotation_on_return_void_function of {
          is_async: bool;
          pos: Pos.t;
          hint_pos: Pos.t option;
        }
      | Tuple_syntax of Pos.t
  end

  module Modules : sig
    type t =
      | Module_hint of {
          pos: Pos.t;
          decl_pos: Pos_or_decl.t;
        }
      | Module_mismatch of {
          pos: Pos.t;
          current_module_opt: string option;
          decl_pos: Pos_or_decl.t;
          target_module: string;
        }
      | Module_unsafe_trait_access of {
          access_pos: Pos.t;
          trait_pos: Pos_or_decl.t;
        }
      | Module_missing_import of {
          pos: Pos.t;
          decl_pos: Pos_or_decl.t;
          module_pos: Pos_or_decl.t;
          current_module: string;
          target_module_opt: string option;
        }
      | Module_missing_export of {
          pos: Pos.t;
          decl_pos: Pos_or_decl.t;
          module_pos: Pos_or_decl.t;
          current_module_opt: string option;
          target_module: string;
        }
  end

  module Xhp : sig
    type t =
      | Xhp_required of {
          pos: Pos.t;
          why_xhp: string;
          ty_reason_msg: Pos_or_decl.t Message.t list Lazy.t;
        }
      | Illegal_xhp_child of {
          pos: Pos.t;
          ty_reason_msg: Pos_or_decl.t Message.t list Lazy.t;
        }
      | Missing_xhp_required_attr of {
          pos: Pos.t;
          attr: string;
          ty_reason_msg: Pos_or_decl.t Message.t list Lazy.t;
        }
  end

  (** Specific error information readily transformable into a user error *)
  type t =
    (* == Factorised errors ================================================= *)
    | Coeffect of Coeffect.t
    | Enum of Enum.t
    | Expr_tree of Expr_tree.t
    | Ifc of Ifc.t
    | Modules of Modules.t
    | Readonly of Readonly.t
    | Shape of Shape.t
    | Wellformedness of Wellformedness.t
    | Xhp of Xhp.t
    (* == Primary only ====================================================== *)
    | Exception_occurred of {
        pos: Pos.t;
        exn: Exception.t;
      }
    (* The intention is to introduce invariant violations with `report_to_user`
       set to `false` initially. Then we observe and confirm that the invariant is
       not repeatedly violated. Only then, we set it to `true` in a subsequent
       release. This should prevent us from blocking users unexpectedly while
       gradually introducing signal for unexpected compiler states. *)
    | Invariant_violation of {
        pos: Pos.t;
        telemetry: Telemetry.t;
        desc: string;
        report_to_user: bool;
      }
    | Internal_error of {
        pos: Pos.t;
        msg: string;
      }
    | Typechecker_timeout of {
        pos: Pos.t;
        fn_name: string;
        seconds: int;
      }
    | Unresolved_tyvar of Pos.t
    | Unify_error of {
        pos: Pos.t;
        msg_opt: string option;
        reasons_opt: Pos_or_decl.t Message.t list Lazy.t option;
      }
    | Generic_unify of {
        pos: Pos.t;
        msg: string;
      }
    | Using_error of {
        pos: Pos.t;
        has_await: bool;
      }
    | Bad_enum_decl of Pos.t
    | Bad_conditional_support_dynamic of {
        pos: Pos.t;
        child: string;
        parent: string;
        ty_name: string Lazy.t;
        self_ty_name: string Lazy.t;
      }
    | Bad_decl_override of {
        pos: Pos.t;
        name: string;
        parent_name: string;
      }
    | Bad_class_refinement of { pos: Pos.t }
    | Explain_where_constraint of {
        pos: Pos.t;
        in_class: bool;
        decl_pos: Pos_or_decl.t;
      }
    | Explain_constraint of Pos.t
    | Rigid_tvar_escape of {
        pos: Pos.t;
        what: string;
      }
    | Invalid_type_hint of Pos.t
    | Unsatisfied_req of {
        pos: Pos.t;
        trait_pos: Pos_or_decl.t;
        req_pos: Pos_or_decl.t;
        req_name: string;
      }
    | Unsatisfied_req_class of {
        pos: Pos.t;
        trait_pos: Pos_or_decl.t;
        req_pos: Pos_or_decl.t;
        req_name: string;
      }
    | Req_class_not_final of {
        pos: Pos.t;
        trait_pos: Pos_or_decl.t;
        req_pos: Pos_or_decl.t;
      }
    | Incompatible_reqs of {
        pos: Pos.t;
        req_name: string;
        req_class_pos: Pos_or_decl.t;
        req_extends_pos: Pos_or_decl.t;
      }
    | Trait_not_used of {
        pos: Pos.t;
        trait_name: string;
        req_class_pos: Pos_or_decl.t;
        class_pos: Pos_or_decl.t;
        class_name: string;
      }
    | Invalid_echo_argument of Pos.t
    | Index_type_mismatch of {
        pos: Pos.t;
        is_covariant_container: bool;
        msg_opt: string option;
        reasons_opt: Pos_or_decl.t Message.t list Lazy.t option;
      }
    | Member_not_found of {
        pos: Pos.t;
        kind: [ `method_ | `property ];
        class_name: string;
        class_pos: Pos_or_decl.t;
        member_name: string;
        hint: ([ `instance | `static ] * Pos_or_decl.t * string) option Lazy.t;
        reason: Pos_or_decl.t Message.t list Lazy.t;
      }
    | Construct_not_instance_method of Pos.t
    | Ambiguous_inheritance of {
        pos: Pos.t;
        class_name: string;
        origin: string;
      }
    | Expected_tparam of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
        n: int;
      }
    | Typeconst_concrete_concrete_override of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
      }
    | Constant_multiple_concrete_conflict of {
        pos: Pos.t;
        name: string;
        definitions: (Pos_or_decl.t * string option) list;
      }
    | Invalid_memoized_param of {
        pos: Pos.t;
        ty_name: string Lazy.t;
        reason: Pos_or_decl.t Message.t list Lazy.t;
      }
    | Invalid_arraykey of {
        pos: Pos.t;
        ctxt: [ `read | `write ];
        container_pos: Pos_or_decl.t;
        container_ty_name: string Lazy.t;
        key_pos: Pos_or_decl.t;
        key_ty_name: string Lazy.t;
      }
    | Invalid_keyset_value of {
        pos: Pos.t;
        container_pos: Pos_or_decl.t;
        container_ty_name: string Lazy.t;
        value_pos: Pos_or_decl.t;
        value_ty_name: string Lazy.t;
      }
    | Invalid_set_value of {
        pos: Pos.t;
        container_pos: Pos_or_decl.t;
        container_ty_name: string Lazy.t;
        value_pos: Pos_or_decl.t;
        value_ty_name: string Lazy.t;
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
    | HKT_wildcard of Pos.t
    | HKT_implicit_argument of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
        param_name: string;
      }
    | Invalid_substring of {
        pos: Pos.t;
        ty_name: string Lazy.t;
      }
    | Unset_nonidx_in_strict of {
        pos: Pos.t;
        reason: Pos_or_decl.t Message.t list Lazy.t;
      }
    | Nullable_cast of {
        pos: Pos.t;
        ty_pos: Pos_or_decl.t;
        ty_name: string Lazy.t;
      }
    | Hh_expect of {
        pos: Pos.t;
        equivalent: bool;
      }
    | Null_member of {
        pos: Pos.t;
        ctxt: [ `read | `write ];
        kind: [ `method_ | `property ];
        member_name: string;
        reason: Pos_or_decl.t Message.t list Lazy.t;
      }
    | Nullsafe_property_write_context of Pos.t
    | Uninstantiable_class of {
        pos: Pos.t;
        class_name: string;
        reason_ty_opt: (Pos.t * string Lazy.t) option;
        decl_pos: Pos_or_decl.t;
      }
    | Abstract_const_usage of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
        name: string;
      }
    | Member_not_implemented of {
        pos: Pos.t;
        member_name: string;
        decl_pos: Pos_or_decl.t;
        quickfixes: Quickfix.t list;
      }
    | Attribute_too_many_arguments of {
        pos: Pos.t;
        name: string;
        expected: int;
      }
    | Attribute_too_few_arguments of {
        pos: Pos.t;
        name: string;
        expected: int;
      }
    | Attribute_not_exact_number_of_args of {
        pos: Pos.t;
        name: string;
        actual: int;
        expected: int;
      }
    | Kind_mismatch of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
        tparam_name: string;
        expected_kind: string;
        actual_kind: string;
      }
    | Trait_parent_construct_inconsistent of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
      }
    | Top_member of {
        pos: Pos.t;
        ctxt: [ `read | `write ];
        ty_name: string Lazy.t;
        decl_pos: Pos_or_decl.t;
        kind: [ `method_ | `property ];
        name: string;
        is_nullable: bool;
        ty_reasons: (Pos_or_decl.t * string) list Lazy.t;
      }
    | Unresolved_tyvar_projection of {
        pos: Pos.t;
        proj_pos: Pos_or_decl.t;
        tconst_name: string;
      }
    | Cyclic_class_constant of {
        pos: Pos.t;
        class_name: string;
        const_name: string;
      }
    | Inout_annotation_missing of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
      }
    | Inout_annotation_unexpected of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
        param_is_variadic: bool;
        qfx_pos: Pos.t;
      }
    | Inout_argument_bad_type of {
        pos: Pos.t;
        reasons: Pos_or_decl.t Message.t list Lazy.t;
      }
    | Invalid_meth_caller_calling_convention of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
        convention: string;
      }
    | Invalid_meth_caller_readonly_return of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
      }
    | Invalid_new_disposable of Pos.t
    | Invalid_return_disposable of Pos.t
    | Invalid_disposable_hint of {
        pos: Pos.t;
        class_name: string;
      }
    | Invalid_disposable_return_hint of {
        pos: Pos.t;
        class_name: string;
      }
    | Ambiguous_lambda of {
        pos: Pos.t;
        uses: Pos_or_decl.t Message.t list Lazy.t;
      }
    | Wrong_extend_kind of {
        pos: Pos.t;
        kind: Ast_defs.classish_kind;
        name: string;
        parent_pos: Pos_or_decl.t;
        parent_kind: Ast_defs.classish_kind;
        parent_name: string;
      }
    | Cyclic_class_def of {
        pos: Pos.t;
        stack: SSet.t;
      }
    | Trait_reuse_with_final_method of {
        pos: Pos.t;
        trait_name: string;
        parent_cls_name: string Lazy.t;
        trace: Pos_or_decl.t Message.t list Lazy.t;
      }
    | Trait_reuse_inside_class of {
        pos: Pos.t;
        class_name: string;
        trait_name: string;
        occurrences: Pos_or_decl.t list;
      }
    | Invalid_is_as_expression_hint of {
        pos: Pos.t;
        op: [ `is | `as_ ];
        reasons: Pos_or_decl.t Message.t list Lazy.t;
      }
    | Invalid_enforceable_type of {
        pos: Pos.t;
        ty_info: Pos_or_decl.t Message.t list Lazy.t;
        tp_pos: Pos_or_decl.t;
        tp_name: string;
        kind: [ `constant | `param ];
      }
    | Reifiable_attr of {
        pos: Pos.t;
        ty_info: Pos_or_decl.t Message.t list Lazy.t;
        attr_pos: Pos_or_decl.t;
        kind: [ `ty | `cnstr | `super_cnstr ];
      }
    | Invalid_newable_type_argument of {
        pos: Pos.t;
        tp_pos: Pos_or_decl.t;
        tp_name: string;
      }
    | Invalid_newable_typaram_constraints of {
        pos: Pos.t;
        tp_name: string;
        constraints: string list;
      }
    | Override_per_trait of {
        pos: Pos.t;
        class_name: string;
        meth_name: string;
        trait_name: string;
        meth_pos: Pos_or_decl.t;
      }
    | Generic_at_runtime of {
        pos: Pos.t;
        prefix: string;
      }
    | Generics_not_allowed of Pos.t
    | Trivial_strict_eq of {
        pos: Pos.t;
        result: bool;
        left: Pos_or_decl.t Message.t list Lazy.t;
        right: Pos_or_decl.t Message.t list Lazy.t;
        left_trail: Pos_or_decl.t list;
        right_trail: Pos_or_decl.t list;
      }
    | Trivial_strict_not_nullable_compare_null of {
        pos: Pos.t;
        result: bool;
        ty_reason_msg: Pos_or_decl.t Message.t list Lazy.t;
      }
    | Eq_incompatible_types of {
        pos: Pos.t;
        left: Pos_or_decl.t Message.t list Lazy.t;
        right: Pos_or_decl.t Message.t list Lazy.t;
      }
    | Comparison_invalid_types of {
        pos: Pos.t;
        left: Pos_or_decl.t Message.t list Lazy.t;
        right: Pos_or_decl.t Message.t list Lazy.t;
      }
    | Strict_eq_value_incompatible_types of {
        pos: Pos.t;
        left: Pos_or_decl.t Message.t list Lazy.t;
        right: Pos_or_decl.t Message.t list Lazy.t;
      }
    | Attribute_param_type of {
        pos: Pos.t;
        x: string;
      }
    | Deprecated_use of {
        pos: Pos.t;
        decl_pos_opt: Pos_or_decl.t option;
        msg: string;
      }
    | Cannot_declare_constant of {
        pos: Pos.t;
        class_pos: Pos.t;
        class_name: string;
      }
    | Local_variable_modified_and_used of {
        pos: Pos.t;
        pos_useds: Pos.t list;
      }
    | Local_variable_modified_twice of {
        pos: Pos.t;
        pos_modifieds: Pos.t list;
      }
    | Assign_during_case of Pos.t
    | Invalid_classname of Pos.t
    | Illegal_type_structure of {
        pos: Pos.t;
        msg: string;
      }
    | Illegal_typeconst_direct_access of Pos.t
    | Wrong_expression_kind_attribute of {
        pos: Pos.t;
        attr_name: string;
        expr_kind: string;
        attr_class_pos: Pos_or_decl.t;
        attr_class_name: string;
        intf_name: string;
      }
    | Wrong_expression_kind_builtin_attribute of {
        pos: Pos.t;
        attr_name: string;
        expr_kind: string;
      }
    | Ambiguous_object_access of {
        pos: Pos.t;
        name: string;
        self_pos: Pos_or_decl.t;
        vis: string;
        subclass_pos: Pos_or_decl.t;
        class_self: string;
        class_subclass: string;
      }
    | Lateinit_with_default of Pos.t
    | Unserializable_type of {
        pos: Pos.t;
        message: string;
      }
    | Invalid_arraykey_constraint of {
        pos: Pos.t;
        ty_name: string Lazy.t;
      }
    | Redundant_covariant of {
        pos: Pos.t;
        msg: string;
        suggest: string;
      }
    | Meth_caller_trait of {
        pos: Pos.t;
        trait_name: string;
      }
    | Duplicate_interface of {
        pos: Pos.t;
        name: string;
        others: Pos_or_decl.t list;
      }
    | Tparam_non_shadowing_reuse of {
        pos: Pos.t;
        tparam_name: string;
      }
    | Reified_function_reference of Pos.t
    | Class_meth_abstract_call of {
        pos: Pos.t;
        class_name: string;
        meth_name: string;
        decl_pos: Pos_or_decl.t;
      }
    | Reinheriting_classish_const of {
        pos: Pos.t;
        classish_name: string;
        src_pos: Pos.t;
        src_classish_name: string;
        existing_const_origin: string;
        const_name: string;
      }
    | Redeclaring_classish_const of {
        pos: Pos.t;
        classish_name: string;
        redeclaration_pos: Pos.t;
        existing_const_origin: string;
        const_name: string;
      }
    | Abstract_function_pointer of {
        pos: Pos.t;
        class_name: string;
        meth_name: string;
        decl_pos: Pos_or_decl.t;
      }
    | Unnecessary_attribute of {
        pos: Pos.t;
        attr: string;
        reason: Pos.t Message.t Lazy.t;
        suggestion: string option;
      }
    | Inherited_class_member_with_different_case of {
        pos: Pos.t;
        member_type: string;
        name: string;
        name_prev: string;
        child_class: string;
        prev_class: string;
        prev_class_pos: Pos_or_decl.t;
      }
    | Multiple_inherited_class_member_with_different_case of {
        pos: Pos.t;
        child_class_name: string;
        member_type: string;
        class1_name: string;
        class1_pos: Pos_or_decl.t;
        name1: string;
        class2_name: string;
        class2_pos: Pos_or_decl.t;
        name2: string;
      }
    | Parent_support_dynamic_type of {
        pos: Pos.t;
        child_name: string;
        child_kind: Ast_defs.classish_kind;
        parent_name: string;
        parent_kind: Ast_defs.classish_kind;
        child_support_dyn: bool;
      }
    | Property_is_not_enforceable of {
        pos: Pos.t;
        prop_name: string;
        class_name: string;
        prop_pos: Pos_or_decl.t;
        prop_type: string;
      }
    | Property_is_not_dynamic of {
        pos: Pos.t;
        prop_name: string;
        class_name: string;
        prop_pos: Pos_or_decl.t;
        prop_type: string;
      }
    | Private_property_is_not_enforceable of {
        pos: Pos.t;
        prop_name: string;
        class_name: string;
        prop_pos: Pos_or_decl.t;
        prop_type: string;
      }
    | Private_property_is_not_dynamic of {
        pos: Pos.t;
        prop_name: string;
        class_name: string;
        prop_pos: Pos_or_decl.t;
        prop_type: string;
      }
    | Immutable_local of Pos.t
    | Nonsense_member_selection of {
        pos: Pos.t;
        kind: string;
      }
    | Consider_meth_caller of {
        pos: Pos.t;
        class_name: string;
        meth_name: string;
      }
    | Method_import_via_diamond of {
        pos: Pos.t;
        class_name: string;
        method_pos: Pos_or_decl.t;
        method_name: string;
        trace1: Pos_or_decl.t Message.t list Lazy.t;
        trace2: Pos_or_decl.t Message.t list Lazy.t;
      }
    | Property_import_via_diamond of {
        generic: bool;
        pos: Pos.t;
        class_name: string;
        property_pos: Pos_or_decl.t;
        property_name: string;
        trace1: Pos_or_decl.t Message.t list Lazy.t;
        trace2: Pos_or_decl.t Message.t list Lazy.t;
      }
    | Unification_cycle of {
        pos: Pos.t;
        ty_name: string Lazy.t;
      }
    | Method_variance of Pos.t
    | Explain_tconst_where_constraint of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
        msgs: Pos_or_decl.t Message.t list Lazy.t;
      }
    | Format_string of {
        pos: Pos.t;
        snippet: string;
        fmt_string: string;
        class_pos: Pos_or_decl.t;
        fn_name: string;
        class_suggest: string;
      }
    | Expected_literal_format_string of Pos.t
    | Re_prefixed_non_string of {
        pos: Pos.t;
        reason: [ `non_string | `embedded_expr ];
      }
    | Bad_regex_pattern of {
        pos: Pos.t;
        reason:
          [ `missing_delim
          | `empty_patt
          | `invalid_option
          | `bad_patt of string
          ];
      }
    | Generic_array_strict of Pos.t
    | Option_return_only_typehint of {
        pos: Pos.t;
        kind: [ `void | `noreturn ];
      }
    | Redeclaring_missing_method of {
        pos: Pos.t;
        trait_method: string;
      }
    | Expecting_type_hint of Pos.t
    | Expecting_type_hint_variadic of Pos.t
    | Expecting_return_type_hint of Pos.t
    | Duplicate_using_var of Pos.t
    | Illegal_disposable of {
        pos: Pos.t;
        verb: [ `assigned ];
      }
    | Escaping_disposable of Pos.t
    | Escaping_disposable_param of Pos.t
    | Escaping_this of Pos.t
    | Must_extend_disposable of Pos.t
    | Field_kinds of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
      }
    | Unbound_name of {
        pos: Pos.t;
        name: string;
        class_exists: bool;
      }
    | Previous_default of Pos.t
    | Return_in_void of {
        pos: Pos.t;
        decl_pos: Pos.t;
      }
    | This_var_outside_class of Pos.t
    | Unbound_global of Pos.t
    | Private_inst_meth of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
      }
    | Protected_inst_meth of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
      }
    | Private_meth_caller of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
      }
    | Protected_meth_caller of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
      }
    | Private_class_meth of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
      }
    | Protected_class_meth of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
      }
    | Array_cast of Pos.t
    | String_cast of {
        pos: Pos.t;
        ty_name: string Lazy.t;
      }
    | Static_outside_class of Pos.t
    | Self_outside_class of Pos.t
    | New_inconsistent_construct of {
        pos: Pos.t;
        class_pos: Pos_or_decl.t;
        class_name: string;
        kind: [ `static | `classname ];
      }
    | Undefined_parent of Pos.t
    | Parent_outside_class of Pos.t
    | Parent_abstract_call of {
        pos: Pos.t;
        meth_name: string;
        decl_pos: Pos_or_decl.t;
      }
    | Self_abstract_call of {
        pos: Pos.t;
        self_pos: Pos.t;
        meth_name: string;
        decl_pos: Pos_or_decl.t;
      }
    | Classname_abstract_call of {
        pos: Pos.t;
        meth_name: string;
        class_name: string;
        decl_pos: Pos_or_decl.t;
      }
    | Static_synthetic_method of {
        pos: Pos.t;
        meth_name: string;
        class_name: string;
        decl_pos: Pos_or_decl.t;
      }
    | Isset_in_strict of Pos.t
    | Isset_inout_arg of Pos.t
    | Unpacking_disallowed_builtin_function of {
        pos: Pos.t;
        fn_name: string;
      }
    | Array_get_arity of {
        pos: Pos.t;
        name: string;
        decl_pos: Pos_or_decl.t;
      }
    | Undefined_field of {
        pos: Pos.t;
        name: string;
        decl_pos: Pos_or_decl.t;
      }
    | Array_access of {
        pos: Pos.t;
        ctxt: [ `read | `write ];
        ty_name: string Lazy.t;
        decl_pos: Pos_or_decl.t;
      }
    | Keyset_set of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
      }
    | Array_append of {
        pos: Pos.t;
        ty_name: string Lazy.t;
        decl_pos: Pos_or_decl.t;
      }
    | Const_mutation of {
        pos: Pos.t;
        ty_name: string Lazy.t;
        decl_pos: Pos_or_decl.t;
      }
    | Expected_class of {
        pos: Pos.t;
        suffix: string Lazy.t option;
      }
    | Unknown_type of {
        pos: Pos.t;
        expected: string;
        reason: Pos_or_decl.t Message.t list Lazy.t;
      }
    | Parent_in_trait of Pos.t
    | Parent_undefined of Pos.t
    | Constructor_no_args of Pos.t
    | Visibility of {
        pos: Pos.t;
        msg: string;
        decl_pos: Pos_or_decl.t;
        reason_msg: string;
      }
    | Bad_call of {
        pos: Pos.t;
        ty_name: string Lazy.t;
      }
    | Extend_final of {
        pos: Pos.t;
        name: string;
        decl_pos: Pos_or_decl.t;
      }
    | Extend_sealed of {
        pos: Pos.t;
        parent_pos: Pos_or_decl.t;
        parent_name: string;
        parent_kind: [ `intf | `trait | `class_ | `enum | `enum_class ];
        verb: [ `extend | `implement | `use ];
      }
    | Sealed_not_subtype of {
        pos: Pos.t;
        name: string;
        child_kind: Ast_defs.classish_kind;
        child_pos: Pos_or_decl.t;
        child_name: string;
      }
    | Trait_prop_const_class of {
        pos: Pos.t;
        name: string;
      }
    | Read_before_write of {
        pos: Pos.t;
        member_name: string;
      }
    | Implement_abstract of {
        pos: Pos.t;
        is_final: bool;
        decl_pos: Pos_or_decl.t;
        trace: Pos_or_decl.t Message.t list Lazy.t;
        name: string;
        kind: [ `meth | `prop | `const | `ty_const ];
        quickfixes: Quickfix.t list;
      }
    | Abstract_member_in_concrete_class of {
        pos: Pos.t;
        class_name_pos: Pos.t;
        is_final: bool;
        member_kind: [ `method_ | `property | `constant | `type_constant ];
        member_name: string;
      }
    | Generic_static of {
        pos: Pos.t;
        typaram_name: string;
      }
    | Ellipsis_strict_mode of {
        pos: Pos.t;
        require: [ `Param_name | `Type_and_param_name ];
      }
    | Object_string of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
      }
    | Object_string_deprecated of Pos.t
    | Cyclic_typedef of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
      }
    | Require_args_reify of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
      }
    | Require_generic_explicit of {
        pos: Pos.t;
        param_name: string;
        decl_pos: Pos_or_decl.t;
      }
    | Invalid_reified_arg of {
        pos: Pos.t;
        param_name: string;
        decl_pos: Pos_or_decl.t;
        arg_info: Pos_or_decl.t Message.t list Lazy.t;
      }
    | Invalid_reified_arg_reifiable of {
        pos: Pos.t;
        param_name: string;
        decl_pos: Pos_or_decl.t;
        ty_pos: Pos_or_decl.t;
        ty_msg: string Lazy.t;
      }
    | New_class_reified of {
        pos: Pos.t;
        class_kind: string;
        suggested_class_name: string option;
      }
    | Class_get_reified of Pos.t
    | Static_meth_with_class_reified_generic of {
        pos: Pos.t;
        generic_pos: Pos.t;
      }
    | Consistent_construct_reified of Pos.t
    | Bad_fn_ptr_construction of Pos.t
    | Reified_generics_not_allowed of Pos.t
    | New_without_newable of {
        pos: Pos.t;
        name: string;
      }
    | Discarded_awaitable of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
      }
    | Static_redeclared_as_dynamic of {
        pos: Pos.t;
        static_pos: Pos_or_decl.t;
        member_name: string;
        elt: [ `meth | `prop ];
      }
    | Dynamic_redeclared_as_static of {
        pos: Pos.t;
        dyn_pos: Pos_or_decl.t;
        member_name: string;
        elt: [ `meth | `prop ];
      }
    | Unknown_object_member of {
        pos: Pos.t;
        member_name: string;
        elt: [ `meth | `prop ];
        reason: Pos_or_decl.t Message.t list Lazy.t;
      }
    | Non_class_member of {
        pos: Pos.t;
        member_name: string;
        elt: [ `meth | `prop ];
        ty_name: string Lazy.t;
        decl_pos: Pos_or_decl.t;
      }
    | Null_container of {
        pos: Pos.t;
        null_witness: Pos_or_decl.t Message.t list Lazy.t;
      }
    | Option_mixed of Pos.t
    | Option_null of Pos.t
    | Declared_covariant of {
        pos: Pos.t;
        param_pos: Pos.t;
        msgs: Pos.t Message.t list Lazy.t;
      }
    | Declared_contravariant of {
        pos: Pos.t;
        param_pos: Pos.t;
        msgs: Pos.t Message.t list Lazy.t;
      }
    | Static_prop_type_generic_param of {
        pos: Pos.t;
        var_ty_pos: Pos_or_decl.t;
        class_pos: Pos_or_decl.t;
      }
    | Contravariant_this of {
        pos: Pos.t;
        class_name: string;
        typaram_name: string;
      }
    | Cyclic_typeconst of {
        pos: Pos.t;
        tyconst_names: string list;
      }
    | Array_get_with_optional_field of {
        recv_pos: Pos.t;
        field_pos: Pos.t;
        field_name: string;
        decl_pos: Pos_or_decl.t;
      }
    | Mutating_const_property of Pos.t
    | Self_const_parent_not of Pos.t
    | Unexpected_ty_in_tast of {
        pos: Pos.t;
        expected_ty: string Lazy.t;
        actual_ty: string Lazy.t;
      }
    | Call_lvalue of Pos.t
    | Unsafe_cast_await of Pos.t
    (* == Primary and secondary =============================================== *)
    | Smember_not_found of {
        pos: Pos.t;
        kind:
          [ `class_constant
          | `class_typeconst
          | `class_variable
          | `static_method
          ];
        class_name: string;
        class_pos: Pos_or_decl.t;
        member_name: string;
        hint: ([ `instance | `static ] * Pos_or_decl.t * string) option;
        quickfixes: Quickfix.t list;
      }
    | Type_arity_mismatch of {
        pos: Pos.t;
        actual: int;
        decl_pos: Pos_or_decl.t;
        expected: int;
      }
    | Typing_too_many_args of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
        actual: int;
        expected: int;
      }
    | Typing_too_few_args of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
        actual: int;
        expected: int;
      }
    | Non_object_member of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
        ty_name: string Lazy.t;
        ctxt: [ `read | `write ];
        member_name: string;
        kind: [ `class_typeconst | `method_ | `property ];
      }

  val to_user_error : t -> (Pos.t, Pos_or_decl.t) User_error.t option
end

module Secondary : sig
  (** Specific error information which needs to be given a primary position from
       the AST being typed to be transformable into a user error.
       This can be done via applying a [Reason_callback.t] using [apply_reasons].
  *)
  type t =
    | Of_error of Error.t
    (* == Primary and secondary =============================================== *)
    | Smember_not_found of {
        pos: Pos_or_decl.t;
        kind:
          [ `class_constant
          | `class_typeconst
          | `class_variable
          | `static_method
          ];
        class_name: string;
        class_pos: Pos_or_decl.t;
        member_name: string;
        closest_member_name: string option;
        hint: ([ `instance | `static ] * Pos_or_decl.t * string) option;
        quickfixes: Quickfix.t list;
      }
    | Type_arity_mismatch of {
        pos: Pos_or_decl.t;
        actual: int;
        decl_pos: Pos_or_decl.t;
        expected: int;
      }
    | Typing_too_many_args of {
        pos: Pos_or_decl.t;
        decl_pos: Pos_or_decl.t;
        actual: int;
        expected: int;
      }
    | Typing_too_few_args of {
        pos: Pos_or_decl.t;
        decl_pos: Pos_or_decl.t;
        actual: int;
        expected: int;
      }
    | Non_object_member of {
        pos: Pos_or_decl.t;
        decl_pos: Pos_or_decl.t;
        ty_name: string Lazy.t;
        ctxt: [ `read | `write ];
        member_name: string;
        kind: [ `class_typeconst | `method_ | `property ];
      }
    | Rigid_tvar_escape of {
        pos: Pos_or_decl.t;
        name: string;
      }
    (* == Secondary only ====================================================== *)
    | Violated_constraint of {
        cstrs: (Pos_or_decl.t * Pos_or_decl.t Message.t) list;
        reasons: Pos_or_decl.t Message.t list Lazy.t;
      }
    | Concrete_const_interface_override of {
        pos: Pos_or_decl.t;
        parent_pos: Pos_or_decl.t;
        parent_origin: string;
        name: string;
      }
    | Interface_or_trait_const_multiple_defs of {
        pos: Pos_or_decl.t;
        origin: string;
        name: string;
        parent_pos: Pos_or_decl.t;
        parent_origin: string;
      }
    | Interface_typeconst_multiple_defs of {
        pos: Pos_or_decl.t;
        origin: string;
        is_abstract: bool;
        name: string;
        parent_pos: Pos_or_decl.t;
        parent_origin: string;
      }
    | Visibility_extends of {
        pos: Pos_or_decl.t;
        vis: string;
        parent_pos: Pos_or_decl.t;
        parent_vis: string;
      }
    | Visibility_override_internal of {
        pos: Pos_or_decl.t;
        module_name: string option;
        parent_pos: Pos_or_decl.t;
        parent_module: string;
      }
    | Abstract_tconst_not_allowed of {
        pos: Pos_or_decl.t;
        decl_pos: Pos_or_decl.t;
        tconst_name: string;
      }
    | Missing_constructor of Pos_or_decl.t
    | Missing_field of {
        pos: Pos_or_decl.t;
        name: string;
        decl_pos: Pos_or_decl.t;
      }
    | Shape_fields_unknown of {
        pos: Pos_or_decl.t;
        decl_pos: Pos_or_decl.t;
      }
    | Accept_disposable_invariant of {
        pos: Pos_or_decl.t;
        decl_pos: Pos_or_decl.t;
      }
    | Ifc_external_contravariant of {
        pos_sub: Pos_or_decl.t;
        pos_super: Pos_or_decl.t;
      }
    | Invalid_destructure of {
        pos: Pos_or_decl.t;
        decl_pos: Pos_or_decl.t;
        ty_name: string Lazy.t;
      }
    | Unpack_array_required_argument of {
        pos: Pos_or_decl.t;
        decl_pos: Pos_or_decl.t;
      }
    | Unpack_array_variadic_argument of {
        pos: Pos_or_decl.t;
        decl_pos: Pos_or_decl.t;
      }
    | Fun_too_many_args of {
        pos: Pos_or_decl.t;
        decl_pos: Pos_or_decl.t;
        actual: int;
        expected: int;
      }
    | Fun_too_few_args of {
        pos: Pos_or_decl.t;
        decl_pos: Pos_or_decl.t;
        actual: int;
        expected: int;
      }
    | Fun_unexpected_nonvariadic of {
        pos: Pos_or_decl.t;
        decl_pos: Pos_or_decl.t;
      }
    | Fun_variadicity_hh_vs_php56 of {
        pos: Pos_or_decl.t;
        decl_pos: Pos_or_decl.t;
      }
    | Required_field_is_optional of {
        pos: Pos_or_decl.t;
        decl_pos: Pos_or_decl.t;
        def_pos: Pos_or_decl.t;
        name: string;
      }
    | Return_disposable_mismatch of {
        pos_sub: Pos_or_decl.t;
        is_marked_return_disposable: bool;
        pos_super: Pos_or_decl.t;
      }
    | Ifc_policy_mismatch of {
        pos: Pos_or_decl.t;
        policy: string;
        pos_super: Pos_or_decl.t;
        policy_super: string;
      }
    | Overriding_prop_const_mismatch of {
        pos: Pos_or_decl.t;
        is_const: bool;
        parent_pos: Pos_or_decl.t;
        parent_is_const: bool;
      }
    | Override_final of {
        pos: Pos_or_decl.t;
        parent_pos: Pos_or_decl.t;
      }
    | Override_async of {
        pos: Pos_or_decl.t;
        parent_pos: Pos_or_decl.t;
      }
    | Override_lsb of {
        pos: Pos_or_decl.t;
        member_name: string;
        parent_pos: Pos_or_decl.t;
      }
    | Multiple_concrete_defs of {
        pos: Pos_or_decl.t;
        name: string;
        origin: string;
        class_name: string;
        parent_pos: Pos_or_decl.t;
        parent_origin: string;
      }
    | Cyclic_enum_constraint of Pos_or_decl.t
    | Inoutness_mismatch of {
        pos: Pos_or_decl.t;
        decl_pos: Pos_or_decl.t;
      }
    | Decl_override_missing_hint of Pos_or_decl.t
    | Bad_lateinit_override of {
        pos: Pos_or_decl.t;
        parent_pos: Pos_or_decl.t;
        parent_is_lateinit: bool;
      }
    | Bad_method_override of {
        pos: Pos_or_decl.t;
        member_name: string;
      }
    | Bad_prop_override of {
        pos: Pos_or_decl.t;
        member_name: string;
      }
    | Bad_xhp_attr_required_override of {
        pos: Pos_or_decl.t;
        tag: string;
        parent_pos: Pos_or_decl.t;
        parent_tag: string;
      }
    | Coeffect_subtyping of {
        pos: Pos_or_decl.t;
        cap: string Lazy.t;
        pos_expected: Pos_or_decl.t;
        cap_expected: string Lazy.t;
      }
    | Override_method_support_dynamic_type of {
        pos: Pos_or_decl.t;
        method_name: string;
        parent_pos: Pos_or_decl.t;
        parent_origin: string;
      }
    | Readonly_mismatch of {
        pos: Pos_or_decl.t;
        kind: [ `fn | `fn_return | `param ];
        reason_sub: Pos_or_decl.t Message.t list Lazy.t;
        reason_super: Pos_or_decl.t Message.t list Lazy.t;
      }
    | Not_sub_dynamic of {
        pos: Pos_or_decl.t;
        ty_name: string Lazy.t;
        dynamic_part: Pos_or_decl.t Message.t list Lazy.t;
      }
    | Subtyping_error of Pos_or_decl.t Message.t list Lazy.t
    | Method_not_dynamically_callable of {
        pos: Pos_or_decl.t;
        parent_pos: Pos_or_decl.t;
      }
    | This_final of {
        pos_sub: Pos_or_decl.t;
        pos_super: Pos_or_decl.t;
        class_name: string;
      }
    | Typeconst_concrete_concrete_override of {
        pos: Pos_or_decl.t;
        parent_pos: Pos_or_decl.t;
      }
    | Abstract_concrete_override of {
        pos: Pos_or_decl.t;
        parent_pos: Pos_or_decl.t;
        kind: [ `constant | `method_ | `property | `typeconst ];
      }
    | Should_not_be_override of {
        pos: Pos_or_decl.t;
        class_id: string;
        id: string;
      }
    | Override_no_default_typeconst of {
        pos: Pos_or_decl.t;
        parent_pos: Pos_or_decl.t;
      }
    | Unsupported_class_refinement of Pos_or_decl.t
    | Missing_type_constant of {
        pos: Pos_or_decl.t;
        class_id: string;
        type_id: string;
      }
end

module Callback : sig
  (** A mechanism to apply transformations to primary errors *)
  type t

  (* -- Constructors -------------------------------------------------------- *)

  (** Ignore any arguments and always return the given base error *)
  val always : Primary.t -> t

  (** Perform an aribrary side-effects when the error is evaluated *)
  val with_side_effect : t -> eff:(unit -> unit) -> t
    [@@ocaml.deprecated
      "This function will be removed. Please avoid adding side effects to error callbacks."]

  (** Provide default code, claim, reasons and quickfixes from the supplied
      `Primary.t` error *)
  val of_primary_error : Primary.t -> t

  (** Provide a default error code *)
  val with_code : code:Error_code.t -> t

  (** Ignore any incoming error code argument and use the existing one *)
  val retain_code : t -> t

  (** Fix the claim of the callback to `new_claim` and add the received claim
      argument to the list of reasons *)
  val with_claim_as_reason : t -> new_claim:Primary.t -> t

  (* -- Specific callbacks -------------------------------------------------- *)

  val unify_error : t

  val index_type_mismatch : t

  val covariant_index_type_mismatch : t

  val expected_stringlike : t

  val constant_does_not_match_enum_type : t

  val enum_underlying_type_must_be_arraykey : t

  val enum_constraint_must_be_arraykey : t

  val enum_subtype_must_have_compatible_constraint : t

  val parameter_default_value_wrong_type : t

  val newtype_alias_must_satisfy_constraint : t

  val missing_return : t

  val inout_return_type_mismatch : t

  val class_constant_value_does_not_match_hint : t

  val class_property_initializer_type_does_not_match_hint : t

  val xhp_attribute_does_not_match_hint : t

  val strict_str_concat_type_mismatch : t

  val strict_str_interp_type_mismatch : t

  val bitwise_math_invalid_argument : t

  val inc_dec_invalid_argument : t

  val math_invalid_argument : t

  val using_error : Pos.t -> has_await:bool -> t
end

module Reasons_callback : sig
  (** A mechanism to apply transformations to secondary errors *)
  type t

  (** Evaluate the `Reasons_callback.t` to a `(Pos.t,Pos_or_decl.t) User_error.t`
      for use in error reporting.

      The optional code, claim, reasons, and quickfixes are the 'starting' values
      of the user error and may be ignored or override the defaults specified in
      the callback.

      We don't require any of these since a reasons callback has both a code
      and claim by construction and we can use the empty list as the default for
      both reasons and quickfixes.
  *)
  val apply :
    ?code:Error_code.t ->
    ?claim:Pos.t Message.t Lazy.t ->
    ?reasons:Pos_or_decl.t Message.t list Lazy.t ->
    ?quickfixes:Quickfix.t list ->
    t ->
    current_span:Pos.t ->
    (Pos.t, Pos_or_decl.t) User_error.t Eval_result.t
  (* -- Constructors -------------------------------------------------------- *)

  (** Construct a `Reasons_callback.t` from a side-effecting function. This is
      included to support legacy code only and will be removed once that code is
      migrated *)
  val from_on_error :
    (?code:int ->
    ?quickfixes:Quickfix.t list ->
    Pos_or_decl.t Message.t list ->
    unit) ->
    t
    [@@ocaml.deprecated
      "This function will be removed. Please use the provided combinators for constructing error callbacks."]

  (** Create a constant `Reasons_callback.t` which defaults to the value of the
      supplied `Error.t` when applied. Each component of that error may be
      subsequently overridden if a value for that component is supplied during
      evaluation *)
  val of_error : Error.t -> t

  (** Create a `Reasons_callback.t` which defaults to the value of the supplied
      `Primary.t` error when applied; this is equivalent
       to `of_error @@ primary err`
  *)
  val of_primary_error : Primary.t -> t

  (** Replace the current default claim with the claim from the supplied
      `Pos.t Message.t` *)
  val with_claim : Callback.t -> claim:Pos.t Message.t Lazy.t -> t

  (** Replace the current default error code with the supplied one *)
  val with_code : t -> code:Error_code.t -> t

  (** Replace the current list of reasons with those supplied. This is typically
       used in combination with `retain_reasons` to fix the `reasons` of the
      `User_error.t` that is obtained when the callback is applied
  *)
  val with_reasons : t -> reasons:Pos_or_decl.t Message.t list Lazy.t -> t

  (** Add the `reason` to the start of current list of reasons *)
  val prepend_reason : t -> reason:Pos_or_decl.t Message.t Lazy.t -> t

  (** Add the `reason` to the end of current list of reasons *)
  val append_reason : t -> reason:Pos_or_decl.t Message.t Lazy.t -> t

  (** When applied, append the supplied reasons to the current list of reasons *)
  val append_incoming_reasons : t -> t

  (** When applied, prepend the supplied reasons to the current list of reasons *)
  val prepend_incoming_reasons : t -> t

  (** Ignore the incoming error code argument when evaluating to a `User_error.t`.

      This is equivalent to the following function, given some callback `on_error`:
      `(fun ?code:_ ?quickfixes reasons -> on_error ?quickfixes reasons)`
  *)
  val retain_code : t -> t

  (** Ignore the incoming reasons argument when evaluating to a `User_error.t`.

      This is equivalent to the following function, given some callback `on_error`:
      `(fun ?code ?quickfixes _ -> on_error ?code ?quickfixes)`
  *)
  val retain_reasons : t -> t

  (** Ignore the incoming quickfixes component when evaluating to a `User_error.t`.

      This is equivalent to the following function, given some callback `on_error`:
      `(fun ?code ?quickfixes:_ reasons -> on_error ?code reasons)`
  *)
  val retain_quickfixes : t -> t

  (** Create a constant `Reasons_callback.t` which always evaluates to the
      supplied `Error.t` when applied

      This is equivalent to the following function, for some `error` of type
      `Error.t`:
      `(fun ?code:_ ?quickfixes:_ _ -> error)`

      This is also the same as:
      `retain_code @@ retain_reasons @@ reatain_quickfixes @@ of_error error`
  *)
  val always : Error.t -> t

  (** Applying the `Reasons_callback.t` `(prepend_on_apply err snd_err1) snd_err2`
       is the same as applying `err` the secondary error created by prepending
       `snd_err1` to `snd_err2`.

       The `Secondary.t` error created by the prepending operation has the same
       code of `snd_err1` and the reasons from `snd_err1` prepended to those of
       `snd_err2`.
  *)
  val prepend_on_apply : t -> Secondary.t -> t

  (* Applying the `Reasons_callback.t` `(assert_in_current_decl code ctx) err`
     will evaluate the `Secondary.t` `err` then use the head of the list of
     reasons as claim in the resulting error, given the position for that
     reason is in the current decl. *)
  val assert_in_current_decl : Error_code.t -> ctx:Pos_or_decl.ctx -> t

  (* -- Specific callbacks -------------------------------------------------- *)

  val unify_error_at : Pos.t -> t

  val bad_enum_decl : Pos.t -> t

  val bad_conditional_support_dynamic :
    Pos.t ->
    child:string ->
    parent:string ->
    ty_name:string Lazy.t ->
    self_ty_name:string Lazy.t ->
    t

  val bad_decl_override :
    name:string -> parent_pos:Pos.t -> parent_name:string -> t

  val bad_class_refinement : Pos.t -> t

  val explain_where_constraint :
    Pos.t -> in_class:bool -> decl_pos:Pos_or_decl.t -> t

  val explain_constraint : Pos.t -> t

  val rigid_tvar_escape_at : Pos.t -> string -> t

  val invalid_type_hint : Pos.t -> t

  val type_constant_mismatch : t -> t

  val class_constant_type_mismatch : t -> t

  val unsatisfied_req_callback :
    class_pos:Pos.t ->
    trait_pos:Pos_or_decl.t ->
    req_pos:Pos_or_decl.t ->
    string ->
    t

  val invalid_echo_argument_at : Pos.t -> t

  val index_type_mismatch_at : Pos.t -> t

  val unify_error_assert_primary_pos_in_current_decl : Pos_or_decl.ctx -> t

  val invalid_type_hint_assert_primary_pos_in_current_decl :
    Pos_or_decl.ctx -> t
end

type t = Error.t

val show : t -> string

val pp : Format.formatter -> t -> unit

(** Iterate over an error calling `on_prim` and `on_snd` when each `Primary.t`
     and `Secondary.t` error is encountered, respectively. *)
val iter :
  t -> on_prim:(Primary.t -> unit) -> on_snd:(Secondary.t -> unit) -> unit

(** Evaluate an error to a `User_error.t` for error reporting; we return an
    option to model ignoring errors *)
val to_user_error :
  t -> current_span:Pos.t -> (Pos.t, Pos_or_decl.t) User_error.t Eval_result.t

(* -- Constructors -------------------------------------------------------- *)

(** Lift a `Primary.t` error to a `Typing_error.t` *)
val primary : Primary.t -> t

(** Lift a `Primary.Coeffect.t` error to a `Typing_error.t` *)
val coeffect : Primary.Coeffect.t -> t

(** Lift a `Primary.Enum.t` error to a `Typing_error.t` *)
val enum : Primary.Enum.t -> t

(** Lift a `Primary.Expr_tree.t` error to a `Typing_error.t` *)
val expr_tree : Primary.Expr_tree.t -> t

(** Lift a `Primary.Coeffect.t` error to a `Typing_error.t` *)
val ifc : Primary.Ifc.t -> t

(** Lift a `Primary.Modules.t` error to a `Typing_error.t` *)
val modules : Primary.Modules.t -> t

(** Lift a `Primary.Readonly.t` error to a `Typing_error.t` *)
val readonly : Primary.Readonly.t -> t

(** Lift a `Primary.Shape.t` error to a `Typing_error.t` *)
val shape : Primary.Shape.t -> t

(** Lift a `Primary.Wellformedness.t` error to a `Typing_error.t` *)
val wellformedness : Primary.Wellformedness.t -> t

(** Lift a `Primary.Xhp.t` error to a `Typing_error.t` *)
val xhp : Primary.Xhp.t -> t

(** Apply a the `Reasons_callback.t` to the supplied `Secondary.t` error, using
    the reasons and error code associated with that error *)
val apply_reasons : Secondary.t -> on_error:Reasons_callback.t -> t

(** Apply a the `Callback.t` to the supplied `Secondary.t` error, using
    the claim, reasons, quickfixes and error code associated with that error *)
val apply : t -> on_error:Callback.t -> t

(** Lift a `Secondary.t` error to a `Typing_error.t` by asserting that the
     position of the first reason is in the current decl and treating it as a
     claim  *)
val assert_in_current_decl : Secondary.t -> ctx:Pos_or_decl.ctx -> t

(** Report a list of errors at each type of an intersection *)
val intersect : t list -> t

(** Calling `intersect_opt xs` returns `None` if the list is empty and `intersect xs` otherwise *)
val intersect_opt : t list -> t option

(** Report a list of errors at each type of a union*)
val union : t list -> t

(** Calling `union_opt xs` returns `None` if the list is empty and `union xs` otherwise *)
val union_opt : t list -> t option

(** Report multiple errors at a single type *)
val multiple : t list -> t

(** Calling `multiple_opt xs` returns `None` if the list is empty and `multiple xs` otherwise *)
val multiple_opt : t list -> t option

(** Report two errors at a single type; `both t1 t2` is the same as
    `multiple [t1;t2]`*)
val both : t -> t -> t

(** Modify the code that will be reported when evaluated to a `User_error.t`  *)
val with_code : t -> code:Error_code.t -> t
