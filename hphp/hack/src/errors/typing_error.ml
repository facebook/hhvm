(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Error_code = Error_codes.Typing

type on_error =
  ?code:int ->
  ?quickfixes:Quickfix.t list ->
  Pos_or_decl.t Message.t list ->
  unit

type error =
  Error_code.t
  * Pos.t Message.t
  * Pos_or_decl.t Message.t list
  * Quickfix.t list

module Common = struct
  let reasons_of_trail trail =
    List.map trail ~f:(fun pos -> (pos, "Typedef definition comes from here"))

  let typing_too_many_args pos decl_pos actual expected =
    let claim =
      ( pos,
        Printf.sprintf
          "Too many arguments (expected %d but got %d)"
          expected
          actual )
    and reasons = [(decl_pos, "Definition is here")] in
    (Error_code.TypingTooManyArgs, claim, reasons)

  let typing_too_few_args pos decl_pos actual expected =
    let claim =
      ( pos,
        Printf.sprintf
          "Too few arguments (required %d but got %d)"
          expected
          actual )
    and reasons = [(decl_pos, "Definition is here")] in
    (Error_code.TypingTooFewArgs, claim, reasons)

  let snot_found_suggestion orig similar kind =
    match similar with
    | (`instance, pos, v) ->
      begin
        match kind with
        | `static_method ->
          Render.suggestion_message ~modifier:"instance method " orig v pos
        | `class_constant ->
          Render.suggestion_message ~modifier:"instance property " orig v pos
        | `class_variable
        | `class_typeconst ->
          Render.suggestion_message orig v pos
      end
    | (`static, pos, v) -> Render.suggestion_message orig v pos

  let smember_not_found pos kind member_name class_name class_pos hint =
    let msg =
      Printf.sprintf
        "No %s %s in %s"
        (Render.string_of_class_member_kind kind)
        (Markdown_lite.md_codify member_name)
        (Markdown_lite.md_codify @@ Render.strip_ns class_name)
    in
    let default =
      [
        ( class_pos,
          "Declaration of "
          ^ (Markdown_lite.md_codify @@ Render.strip_ns class_name)
          ^ " is here" );
      ]
    in
    let reasons =
      Option.value_map hint ~default ~f:(fun similar ->
          snot_found_suggestion member_name similar kind :: default)
    in

    (Error_code.SmemberNotFound, (pos, msg), reasons)

  let non_object_member pos ctxt ty_name member_name kind decl_pos =
    let code =
      match ctxt with
      | `read -> Error_code.NonObjectMemberRead
      | `write -> Error_code.NonObjectMemberWrite
    in
    let msg_start =
      Printf.sprintf
        "You are trying to access the %s %s but this is %s"
        (Render.string_of_class_member_kind kind)
        (Markdown_lite.md_codify member_name)
        ty_name
    in
    let msg =
      if String.equal ty_name "a shape" then
        msg_start ^ ". Did you mean `$foo['" ^ member_name ^ "']` instead?"
      else
        msg_start
    in
    let claim = (pos, msg) and reasons = [(decl_pos, "Definition is here")] in
    (code, claim, reasons)

  let badpos_message =
    Printf.sprintf
      "Incomplete position information! Your type error is in this file, but we could only find related positions in another file. %s"
      Error_message_sentinel.please_file_a_bug_message

  let badpos_message_2 =
    Printf.sprintf
      "Incomplete position information! We couldn't find the exact line of your type error in this definition. %s"
      Error_message_sentinel.please_file_a_bug_message

  let wrap_error_in_different_file ~current_file ~current_span reasons =
    let message =
      List.map reasons ~f:(fun (pos, msg) ->
          Pos.print_verbose_relative (Pos_or_decl.unsafe_to_raw_pos pos)
          ^ ": "
          ^ msg)
    in
    let stack =
      Exception.get_current_callstack_string 99 |> Exception.clean_stack
    in
    HackEventLogger.type_check_primary_position_bug
      ~current_file
      ~message
      ~stack;
    let claim =
      if Pos.equal current_span Pos.none then
        (Pos.make_from current_file, badpos_message)
      else
        (current_span, badpos_message_2)
    in
    (claim, reasons)

  let eval_assert ctx current_span = function
    | (code, ((pos, msg) :: rest as reasons)) ->
      let (claim, reasons) =
        match
          Pos_or_decl.fill_in_filename_if_in_current_decl
            ~current_decl_and_file:ctx
            pos
        with
        | Some pos -> ((pos, msg), rest)
        | _ ->
          wrap_error_in_different_file
            ~current_file:ctx.Pos_or_decl.file
            ~current_span
            reasons
      in
      Some (code, claim, reasons, [])
    | _ -> None
end

module Primary = struct
  module Shape = struct
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
          reason: [ `Nothing of Pos_or_decl.t Message.t list | `Undefined ];
        }
      | Shapes_method_access_with_non_existent_field of {
          pos: Pos.t;
          field_name: string;
          decl_pos: Pos_or_decl.t;
          method_name: string;
          reason: [ `Nothing of Pos_or_decl.t Message.t list | `Undefined ];
        }
      | Shapes_access_with_non_existent_field of {
          pos: Pos.t;
          field_name: string;
          decl_pos: Pos_or_decl.t;
          reason: [ `Nothing of Pos_or_decl.t Message.t list | `Undefined ];
        }

    let invalid_shape_field_type pos ty_pos ty_name trail =
      let reasons =
        (ty_pos, "Not " ^ Lazy.force ty_name) :: Common.reasons_of_trail trail
      and claim = (pos, "A shape field name must be an `int` or `string`") in
      (Error_code.InvalidShapeFieldType, claim, reasons, [])

    let invalid_shape_field_name pos =
      let msg =
        "Was expecting a constant string, class constant, or int (for shape access)"
      in
      (Error_code.InvalidShapeFieldName, (pos, msg), [], [])

    let invalid_shape_field_name_empty pos =
      let msg = "A shape field name cannot be an empty string" in

      (Error_code.InvalidShapeFieldNameEmpty, (pos, msg), [], [])

    let invalid_shape_field_literal pos witness_pos =
      let claim = (pos, "Shape uses literal string as field name")
      and reason =
        [(Pos_or_decl.of_raw_pos witness_pos, "But expected a class constant")]
      in
      (Error_code.InvalidShapeFieldLiteral, claim, reason, [])

    let invalid_shape_field_const pos witness_pos =
      let claim = (pos, "Shape uses class constant as field name")
      and reason =
        [(Pos_or_decl.of_raw_pos witness_pos, "But expected a literal string")]
      in
      (Error_code.InvalidShapeFieldConst, claim, reason, [])

    let shape_field_class_mismatch pos class_name witness_pos witness_class_name
        =
      let claim =
        ( pos,
          "Shape field name is class constant from "
          ^ Markdown_lite.md_codify class_name )
      and reason =
        [
          ( Pos_or_decl.of_raw_pos witness_pos,
            "But expected constant from "
            ^ Markdown_lite.md_codify witness_class_name );
        ]
      in
      (Error_code.ShapeFieldClassMismatch, claim, reason, [])

    let shape_field_type_mismatch pos ty_name witness_pos witness_ty_name =
      let claim =
        (pos, "Shape field name is " ^ Lazy.force ty_name ^ " class constant")
      and reason =
        [
          ( Pos_or_decl.of_raw_pos witness_pos,
            "But expected " ^ Lazy.force witness_ty_name );
        ]
      in
      (Error_code.ShapeFieldTypeMismatch, claim, reason, [])

    let invalid_shape_remove_key pos =
      let msg = "You can only unset fields of **local** variables" in
      (Error_code.InvalidShapeRemoveKey, (pos, msg), [], [])

    let shapes_key_exists_always_true pos field_name decl_pos =
      let claim = (pos, "This `Shapes::keyExists()` check is always true")
      and reason =
        [
          ( decl_pos,
            "The field "
            ^ Markdown_lite.md_codify field_name
            ^ " exists because of this definition" );
        ]
      in
      (Error_code.ShapesKeyExistsAlwaysTrue, claim, reason, [])

    let shape_field_non_existence_reason pos name = function
      | `Undefined ->
        [
          ( pos,
            "The field "
            ^ Markdown_lite.md_codify name
            ^ " is not defined in this shape" );
        ]
      | `Nothing reason ->
        ( pos,
          "The type of the field "
          ^ Markdown_lite.md_codify name
          ^ " in this shape doesn't allow any values" )
        :: reason

    let shapes_key_exists_always_false pos field_name decl_pos reason =
      let claim = (pos, "This `Shapes::keyExists()` check is always false")
      and reason =
        shape_field_non_existence_reason decl_pos field_name reason
      in
      (Error_code.ShapesKeyExistsAlwaysFalse, claim, reason, [])

    let shapes_method_access_with_non_existent_field
        pos field_name method_name decl_pos reason =
      let claim =
        ( pos,
          "You are calling "
          ^ Markdown_lite.md_codify ("Shapes::" ^ method_name ^ "()")
          ^ " on a field known to not exist" )
      and reason =
        shape_field_non_existence_reason decl_pos field_name reason
      in
      (Error_code.ShapesMethodAccessWithNonExistentField, claim, reason, [])

    let shapes_access_with_non_existent_field pos field_name decl_pos reason =
      let claim = (pos, "You are accessing a field known to not exist")
      and reason =
        shape_field_non_existence_reason decl_pos field_name reason
      in
      (Error_code.ShapeAccessWithNonExistentField, claim, reason, [])

    let to_error = function
      | Invalid_shape_field_type { pos; ty_pos; ty_name; trail } ->
        invalid_shape_field_type pos ty_pos ty_name trail
      | Invalid_shape_field_name { pos; is_empty = false } ->
        invalid_shape_field_name pos
      | Invalid_shape_field_name { pos; is_empty = true } ->
        invalid_shape_field_name_empty pos
      | Invalid_shape_field_literal { pos; witness_pos } ->
        invalid_shape_field_literal pos witness_pos
      | Invalid_shape_field_const { pos; witness_pos } ->
        invalid_shape_field_const pos witness_pos
      | Shape_field_class_mismatch
          { pos; class_name; witness_pos; witness_class_name } ->
        shape_field_class_mismatch pos class_name witness_pos witness_class_name
      | Shape_field_type_mismatch { pos; ty_name; witness_pos; witness_ty_name }
        ->
        shape_field_type_mismatch pos ty_name witness_pos witness_ty_name
      | Invalid_shape_remove_key pos -> invalid_shape_remove_key pos
      | Shapes_key_exists_always_true { pos; field_name; decl_pos } ->
        shapes_key_exists_always_true pos field_name decl_pos
      | Shapes_key_exists_always_false { pos; field_name; decl_pos; reason } ->
        shapes_key_exists_always_false pos field_name decl_pos reason
      | Shapes_method_access_with_non_existent_field
          { pos; field_name; method_name; decl_pos; reason } ->
        shapes_method_access_with_non_existent_field
          pos
          field_name
          method_name
          decl_pos
          reason
      | Shapes_access_with_non_existent_field
          { pos; field_name; decl_pos; reason } ->
        shapes_access_with_non_existent_field pos field_name decl_pos reason
  end

  module Enum = struct
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
          class_name: string;
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

    let enum_class_label_member_mismatch pos label expected_ty_msg_opt =
      let claim = (pos, "Enum class label/member mismatch")
      and reasons =
        match expected_ty_msg_opt with
        | Some expected_ty_msg ->
          Lazy.force expected_ty_msg
          @ [
              ( Pos_or_decl.of_raw_pos pos,
                Format.sprintf "But got an enum class label: `#%s`" label );
            ]
        | None ->
          [
            ( Pos_or_decl.of_raw_pos pos,
              Format.sprintf "Unexpected enum class label: `#%s`" label );
          ]
      in
      (Error_code.UnifyError, claim, reasons, [])

    let enum_type_bad pos is_enum_class ty_name trail =
      let ty = Markdown_lite.md_codify @@ Lazy.force ty_name in
      let msg =
        if is_enum_class then
          "Invalid base type for an enum class: "
        else
          "Enums must be `int` or `string` or `arraykey`, not "
      in
      let claim = (pos, msg ^ ty) and reasons = Common.reasons_of_trail trail in
      (Error_code.EnumTypeBad, claim, reasons, [])

    let enum_constant_type_bad pos ty_pos ty_name trail =
      let claim = (pos, "Enum constants must be an `int` or `string`")
      and reasons =
        (ty_pos, "Not " ^ Markdown_lite.md_codify @@ Lazy.force ty_name)
        :: Common.reasons_of_trail trail
      in
      (Error_code.EnumConstantTypeBad, claim, reasons, [])

    let enum_type_typedef_nonnull pos =
      let claim =
        (pos, "Can't use `typedef` that resolves to nonnull in enum")
      in
      (Error_code.EnumTypeTypedefNonnull, claim, [], [])

    let enum_switch_redundant pos first_pos const_name =
      let claim = (pos, "Redundant `case` statement")
      and reason =
        [
          ( Pos_or_decl.of_raw_pos first_pos,
            Markdown_lite.md_codify const_name ^ " already handled here" );
        ]
      in
      (Error_code.EnumSwitchRedundant, claim, reason, [])

    let enum_switch_nonexhaustive pos kind decl_pos missing =
      let claim =
        ( pos,
          "`switch` statement nonexhaustive; the following cases are missing: "
          ^ (List.map ~f:Markdown_lite.md_codify missing
            |> String.concat ~sep:", ") )
      and reason = [(decl_pos, kind ^ " declared here")] in
      (Error_code.EnumSwitchNonexhaustive, claim, reason, [])

    let enum_switch_redundant_default pos kind decl_pos =
      let claim =
        ( pos,
          "All cases already covered; a redundant `default` case prevents "
          ^ "detecting future errors. If your goal is to guard against "
          ^ "invalid values for this type, do an `is` check before the switch."
        )
      and reason = [(decl_pos, kind ^ " declared here")] in
      (Error_code.EnumSwitchRedundantDefault, claim, reason, [])

    let enum_switch_not_const pos =
      let claim = (pos, "Case in `switch` on enum is not an enum constant") in
      (Error_code.EnumSwitchNotConst, claim, [], [])

    let enum_switch_wrong_class pos kind expected actual =
      let claim =
        ( pos,
          "Switching on "
          ^ kind
          ^ Markdown_lite.md_codify expected
          ^ " but using constant from "
          ^ Markdown_lite.md_codify actual )
      in
      (Error_code.EnumSwitchWrongClass, claim, [], [])

    let enum_class_label_unknown pos label_name class_name =
      let class_name = Render.strip_ns class_name in
      let claim =
        (pos, "Unknown constant " ^ label_name ^ " in " ^ class_name)
      in
      (Error_code.EnumClassLabelUnknown, claim, [], [])

    let enum_class_label_as_expr pos =
      let claim =
        ( pos,
          "Not enough type information to infer the type of this enum class label."
        )
      in
      (Error_code.EnumClassLabelAsExpression, claim, [], [])

    let incompatible_enum_inclusion_base pos classish_name src_classish_name =
      let claim =
        ( pos,
          "Enum "
          ^ Render.strip_ns classish_name
          ^ " includes enum "
          ^ Render.strip_ns src_classish_name
          ^ " but their base types are incompatible" )
      in
      (Error_code.IncompatibleEnumInclusion, claim, [], [])

    let incompatible_enum_inclusion_constraint
        pos classish_name src_classish_name =
      let claim =
        ( pos,
          "Enum "
          ^ Render.strip_ns classish_name
          ^ " includes enum "
          ^ Render.strip_ns src_classish_name
          ^ " but their constraints are incompatible" )
      in
      (Error_code.IncompatibleEnumInclusion, claim, [], [])

    let enum_inclusion_not_enum pos classish_name src_classish_name =
      let claim =
        ( pos,
          "Enum "
          ^ Render.strip_ns classish_name
          ^ " includes "
          ^ Render.strip_ns src_classish_name
          ^ " which is not an enum" )
      in
      (Error_code.IncompatibleEnumInclusion, claim, [], [])

    let enum_classes_reserved_syntax pos =
      ( Error_code.EnumClassesReservedSyntax,
        ( pos,
          "This syntax is reserved for the Enum Classes feature.\n"
          ^ "Enable it with the enable_enum_classes option in .hhconfig" ),
        [],
        [] )

    let enum_supertyping_reserved_syntax pos =
      ( Error_code.EnumSupertypingReservedSyntax,
        ( pos,
          "This Enum uses syntax reserved for the Enum Supertyping feature.\n"
          ^ "Enable it with the enable_enum_supertyping option in .hhconfig" ),
        [],
        [] )

    let to_error = function
      | Enum_type_bad { pos; is_enum_class; ty_name; trail } ->
        enum_type_bad pos is_enum_class ty_name trail
      | Enum_constant_type_bad { pos; ty_pos; ty_name; trail } ->
        enum_constant_type_bad pos ty_pos ty_name trail
      | Enum_type_typedef_nonnull pos -> enum_type_typedef_nonnull pos
      | Enum_switch_redundant { pos; first_pos; const_name } ->
        enum_switch_redundant pos first_pos const_name
      | Enum_switch_nonexhaustive { pos; kind; decl_pos; missing } ->
        enum_switch_nonexhaustive pos kind decl_pos missing
      | Enum_switch_redundant_default { pos; kind; decl_pos } ->
        enum_switch_redundant_default pos kind decl_pos
      | Enum_switch_not_const pos -> enum_switch_not_const pos
      | Enum_switch_wrong_class { pos; kind; expected; actual } ->
        enum_switch_wrong_class pos kind expected actual
      | Enum_class_label_unknown { pos; label_name; class_name } ->
        enum_class_label_unknown pos label_name class_name
      | Enum_class_label_as_expr pos -> enum_class_label_as_expr pos
      | Enum_class_label_member_mismatch { pos; label; expected_ty_msg_opt } ->
        enum_class_label_member_mismatch pos label expected_ty_msg_opt
      | Incompatible_enum_inclusion_base
          { pos; classish_name; src_classish_name } ->
        incompatible_enum_inclusion_base pos classish_name src_classish_name
      | Incompatible_enum_inclusion_constraint
          { pos; classish_name; src_classish_name } ->
        incompatible_enum_inclusion_constraint
          pos
          classish_name
          src_classish_name
      | Enum_inclusion_not_enum { pos; classish_name; src_classish_name } ->
        enum_inclusion_not_enum pos classish_name src_classish_name
      | Enum_classes_reserved_syntax pos -> enum_classes_reserved_syntax pos
      | Enum_supertyping_reserved_syntax pos ->
        enum_supertyping_reserved_syntax pos
  end

  module Expr_tree = struct
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

    let expression_tree_non_public_member pos decl_pos =
      let claim =
        (pos, "Cannot access non-public members within expression trees.")
      and reason = [(decl_pos, "Member defined here")] in
      (Error_code.ExpressionTreeNonPublicProperty, claim, reason, [])

    let reified_static_method_in_expr_tree pos =
      let claim =
        ( pos,
          "Static method calls on reified generics are not permitted in Expression Trees."
        )
      in
      (Error_code.ReifiedStaticMethodInExprTree, claim, [], [])

    let this_var_in_expr_tree pos =
      let claim = (pos, "`$this` is not bound inside expression trees") in
      (Error_code.ThisVarOutsideClass, claim, [], [])

    let experimental_expression_trees pos =
      let claim =
        ( pos,
          "This type is not permitted as an expression tree visitor. It is not included in "
          ^ "`allowed_expression_tree_visitors` in `.hhconfig`, and this file does not "
          ^ "contain `<<file:__EnableUnstableFeatures('expression_trees')>>`."
        )
      in
      (Error_code.ExperimentalExpressionTrees, claim, [], [])

    let expression_tree_unsupported_operator pos member_name class_name =
      let msg =
        match member_name with
        | "__bool" ->
          (* If the user writes `if ($not_bool)`, provide a more specific
             error rather than a generic missing method error for
             `$not_bool->__bool()`. *)
          Printf.sprintf
            "`%s` cannot be used as a boolean (it has no instance method named `__bool`)"
            class_name
        | _ ->
          (* If the user writes `$not_int + ...`, provide a more specific
             error rather than a generic missing method error for
             `$not_int->__plus(...)`. *)
          Printf.sprintf
            "`%s` does not support this operator (it has no instance method named `%s`)"
            class_name
            member_name
      in
      (Error_code.MemberNotFound, (pos, msg), [], [])

    let to_error = function
      | Expression_tree_non_public_member { pos; decl_pos } ->
        expression_tree_non_public_member pos decl_pos
      | Reified_static_method_in_expr_tree pos ->
        reified_static_method_in_expr_tree pos
      | This_var_in_expr_tree pos -> this_var_in_expr_tree pos
      | Experimental_expression_trees pos -> experimental_expression_trees pos
      | Expression_tree_unsupported_operator { pos; member_name; class_name } ->
        expression_tree_unsupported_operator pos member_name class_name
  end

  module Readonly = struct
    type t =
      | Readonly_modified of {
          pos: Pos.t;
          reason_opt: Pos_or_decl.t Message.t option;
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

    let readonly_modified pos reason_opt =
      let claim =
        (pos, "This value is readonly, its properties cannot be modified")
      and reason = Option.value_map reason_opt ~default:[] ~f:List.return in
      (Error_code.ReadonlyValueModified, claim, reason, [])

    let readonly_mismatch pos what pos_sub pos_super =
      let (msg, msg_sub, msg_super) =
        match what with
        | `prop_assign ->
          ( "property assignment",
            "readonly",
            "But it's being assigned to a mutable property" )
        | `collection_mod ->
          ("collection modification", "readonly", "But this value is mutable")
        | `arg_readonly ->
          ( "argument",
            "readonly",
            "It is incompatible with this parameter, which is mutable" )
        | `arg_mut ->
          ( "argument",
            "mutable",
            "It is incompatible with this parameter, which is readonly" )
      in
      let claim = (pos, Format.sprintf "Invalid %s" msg)
      and reason =
        [
          (pos_sub, Format.sprintf "This expression is %s" msg_sub);
          (pos_super, msg_super);
        ]
      in
      (Error_code.ReadonlyMismatch, claim, reason, [])

    let readonly_invalid_as_mut pos =
      let claim =
        ( pos,
          "Only value types and arrays can be converted to mutable. This value can never be a primitive."
        )
      in
      (Error_code.ReadonlyInvalidAsMut, claim, [], [])

    let readonly_exception pos =
      let claim =
        ( pos,
          "This exception is readonly; throwing readonly exceptions is not currently supported."
        )
      in
      (Error_code.ReadonlyException, claim, [], [])

    let explicit_readonly_cast pos decl_pos kind =
      let (start_line, start_column) = Pos.line_column pos in
      (* Create a zero-width position at the start of the offending
         expression, so we can insert text without overwriting anything. *)
      let qf_pos =
        pos |> Pos.set_line_end start_line |> Pos.set_col_end start_column
      in
      let quickfixes =
        [Quickfix.make ~title:"Insert `readonly`" ~new_text:"readonly " qf_pos]
      in
      let kind_str =
        match kind with
        | `fn_call -> "function call"
        | `property -> "property"
        | `static_property -> "static property"
      in
      let claim =
        ( pos,
          "This "
          ^ kind_str
          ^ " returns a readonly value. It must be explicitly wrapped in a readonly expression."
        )
      and reason = [(decl_pos, "The " ^ kind_str ^ " is defined here.")] in
      (Error_code.ExplicitReadonlyCast, claim, reason, quickfixes)

    let readonly_method_call pos decl_pos =
      let claim =
        ( pos,
          "This expression is readonly, so it can only call readonly methods" )
      and reason = [(decl_pos, "This method is not readonly")] in
      (Error_code.ReadonlyMethodCall, claim, reason, [])

    let readonly_closure_call pos decl_pos suggestion =
      let claim =
        ( pos,
          "This function is readonly, so it must be marked readonly at declaration time to be called."
        )
      and reason = [(decl_pos, "Did you mean to " ^ suggestion ^ "?")] in
      (Error_code.ReadonlyClosureCall, claim, reason, [])

    let to_error = function
      | Readonly_modified { pos; reason_opt } ->
        readonly_modified pos reason_opt
      | Readonly_mismatch { pos; what; pos_sub; pos_super } ->
        readonly_mismatch pos what pos_sub pos_super
      | Readonly_invalid_as_mut pos -> readonly_invalid_as_mut pos
      | Readonly_exception pos -> readonly_exception pos
      | Explicit_readonly_cast { pos; decl_pos; kind } ->
        explicit_readonly_cast pos decl_pos kind
      | Readonly_method_call { pos; decl_pos } ->
        readonly_method_call pos decl_pos
      | Readonly_closure_call { pos; decl_pos; suggestion } ->
        readonly_closure_call pos decl_pos suggestion
  end

  module Ifc = struct
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

    let illegal_information_flow
        pos secondaries source_poss source sink_poss sink =
      let explain poss node printer reasons =
        let msg = printer node in
        List.map ~f:(fun pos -> (pos, msg)) poss @ reasons
      in
      let source = Markdown_lite.md_codify source in
      let sink = Markdown_lite.md_codify sink in
      let sprintf_main = sprintf "Data with policy %s appears in context %s." in
      let claim = (pos, sprintf_main source sink) in
      let reasons =
        let sprintf = Printf.sprintf in
        let sprintf_source =
          sprintf "This may be the data source with policy %s"
        in
        let sprintf_sink = sprintf "This may be the data sink with policy %s" in
        let other_occurrences =
          let f p =
            (p, "Another program point contributing to the illegal flow")
          in
          List.map ~f secondaries
        in
        []
        |> explain source_poss source sprintf_source
        |> explain sink_poss sink sprintf_sink
        |> List.append other_occurrences
        |> List.rev
      in
      (Error_code.IllegalInformationFlow, claim, reasons, [])

    let ifc_internal_error pos msg =
      let claim =
        ( pos,
          "IFC Internal Error: "
          ^ msg
          ^ ". If you see this error and aren't expecting it, please `hh rage` and let the Hack team know."
        )
      in
      (Error_code.IFCInternalError, claim, [], [])

    let unknown_information_flow pos what =
      let claim =
        ( pos,
          "Unable to analyze information flow for "
          ^ what
          ^ ". This might be unsafe." )
      in
      (Error_code.UnknownInformationFlow, claim, [], [])

    let context_implicit_policy_leakage
        pos secondaries source_poss source sink_poss sink =
      let program_point p =
        (p, "Another program point contributing to the leakage")
      in
      let explain_source p = (p, "Leakage source") in
      let explain_sink p = (p, "Leakage sink") in
      let claim =
        ( pos,
          Printf.sprintf
            "Context-implicit policy leaks into %s via %s."
            (Markdown_lite.md_codify sink)
            (Markdown_lite.md_codify source) )
      in
      let reasons =
        List.map ~f:program_point secondaries
        @ List.map ~f:explain_source source_poss
        @ List.map ~f:explain_sink sink_poss
      in
      (Error_code.ContextImplicitPolicyLeakage, claim, reasons, [])

    let to_error = function
      | Illegal_information_flow
          { pos; secondaries; source_poss; source; sink_poss; sink } ->
        illegal_information_flow
          pos
          secondaries
          source_poss
          source
          sink_poss
          sink
      | Ifc_internal_error { pos; msg } -> ifc_internal_error pos msg
      | Unknown_information_flow { pos; what } ->
        unknown_information_flow pos what
      | Context_implicit_policy_leakage
          { pos; secondaries; source_poss; source; sink_poss; sink } ->
        context_implicit_policy_leakage
          pos
          secondaries
          source_poss
          source
          sink_poss
          sink
  end

  module Record = struct
    type t =
      | Unexpected_record_field_name of {
          pos: Pos.t;
          field_name: string;
          record_name: string;
          decl_pos: Pos_or_decl.t;
        }
      | Missing_record_field_name of {
          pos: Pos.t;
          field_name: string;
          record_name: string;
          decl_pos: Pos_or_decl.t;
        }
      | Type_not_record of {
          pos: Pos.t;
          ty_name: string;
        }
      | New_abstract_record of {
          pos: Pos.t;
          name: string;
        }

    let unexpected_record_field_name pos field_name record_name decl_pos =
      let claim =
        ( pos,
          Printf.sprintf
            "Record %s has no field %s"
            (Render.strip_ns record_name |> Markdown_lite.md_codify)
            (Markdown_lite.md_codify field_name) )
      and reasons = [(decl_pos, "Definition is here")] in
      (Error_code.RecordUnknownField, claim, reasons, [])

    let missing_record_field_name pos field_name record_name decl_pos =
      let claim =
        ( pos,
          Printf.sprintf
            "Mising required field %s in %s"
            (Markdown_lite.md_codify field_name)
            (Render.strip_ns record_name |> Markdown_lite.md_codify) )
      and reasons = [(decl_pos, "Field definition is here")] in
      (Error_code.RecordMissingRequiredField, claim, reasons, [])

    let type_not_record pos ty_name =
      let claim =
        ( pos,
          Printf.sprintf
            "Expected a record type, but got %s."
            (Render.strip_ns ty_name |> Markdown_lite.md_codify) )
      in
      (Error_code.NotARecord, claim, [], [])

    let new_abstract_record pos name =
      let name = Render.strip_ns name in
      let msg =
        Printf.sprintf
          "Cannot create instance of abstract record %s"
          (Markdown_lite.md_codify name)
      in
      (Error_code.NewAbstractRecord, (pos, msg), [], [])

    let to_error = function
      | Unexpected_record_field_name { pos; field_name; record_name; decl_pos }
        ->
        unexpected_record_field_name pos field_name record_name decl_pos
      | Missing_record_field_name { pos; field_name; record_name; decl_pos } ->
        missing_record_field_name pos field_name record_name decl_pos
      | Type_not_record { pos; ty_name } -> type_not_record pos ty_name
      | New_abstract_record { pos; name } -> new_abstract_record pos name
  end

  module Coeffect = struct
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
          suggestion: Pos_or_decl.t Message.t list option;
        }

    let call_coeffect
        pos available_pos available_incl_unsafe required_pos required =
      let reasons =
        [
          ( available_pos,
            "From this declaration, the context of this function body provides "
            ^ Lazy.force available_incl_unsafe );
          ( required_pos,
            "But the function being called requires " ^ Lazy.force required );
        ]
      and claim =
        ( pos,
          "This call is not allowed because its coeffects are incompatible with the context"
        )
      in
      (Error_code.CallCoeffects, claim, reasons, [])

    let op_coeffect_error
        pos op_name required available_pos locally_available suggestion err_code
        =
      let reasons =
        ( available_pos,
          "The local (enclosing) context provides "
          ^ Lazy.force locally_available )
        :: Option.value ~default:[] suggestion
      and claim =
        ( pos,
          op_name
          ^ " requires "
          ^ Lazy.force required
          ^ ", which is not provided by the context." )
      in
      (err_code, claim, reasons, [])

    let to_error = function
      | Op_coeffect_error
          {
            pos;
            op_name;
            required;
            available_pos;
            locally_available;
            suggestion;
            err_code;
          } ->
        op_coeffect_error
          pos
          op_name
          required
          available_pos
          locally_available
          suggestion
          err_code
      | Call_coeffect
          { pos; available_pos; available_incl_unsafe; required_pos; required }
        ->
        call_coeffect
          pos
          available_pos
          available_incl_unsafe
          required_pos
          required
  end

  module Wellformedness = struct
    type t =
      | Missing_return of Pos.t
      | Dollardollar_lvalue of Pos.t
      | Void_usage of {
          pos: Pos.t;
          reason: Pos_or_decl.t Message.t list;
        }
      | Noreturn_usage of {
          pos: Pos.t;
          reason: Pos_or_decl.t Message.t list;
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

    let missing_return pos =
      let claim = (pos, "Invalid return type") in
      (Error_code.MissingReturnInNonVoidFunction, claim, [], [])

    let dollardollar_lvalue pos =
      let claim =
        (pos, "Cannot assign a value to the special pipe variable `$$`")
      in
      (Error_code.DollardollarLvalue, claim, [], [])

    let void_usage pos reason =
      let msg = "You are using the return value of a `void` function" in
      (Error_code.VoidUsage, (pos, msg), reason, [])

    let noreturn_usage pos reason =
      let msg = "You are using the return value of a `noreturn` function" in
      (Error_code.NoreturnUsage, (pos, msg), reason, [])

    let returns_with_and_without_value pos with_value_pos without_value_pos_opt
        =
      let claim =
        (pos, "This function can exit with and without returning a value")
      and reason =
        (Pos_or_decl.of_raw_pos with_value_pos, "Returning a value here.")
        :: Option.value_map
             without_value_pos_opt
             ~default:
               [
                 ( Pos_or_decl.of_raw_pos pos,
                   "This function does not always return a value" );
               ]
             ~f:(fun p ->
               [(Pos_or_decl.of_raw_pos p, "Returning without a value here")])
      in
      (Error_code.ReturnsWithAndWithoutValue, claim, reason, [])

    let missing_assign pos =
      (Error_code.MissingAssign, (pos, "Please assign a value"), [], [])

    let non_void_annotation_on_return_void_function is_async pos hint_pos =
      let (async_indicator, return_type) =
        if is_async then
          ("Async f", "Awaitable<void>")
        else
          ("F", "void")
      in
      let message =
        Printf.sprintf
          "%sunctions that do not return a value must have a type of %s"
          async_indicator
          return_type
      in
      let quickfix =
        match hint_pos with
        | None -> []
        | Some hint_pos ->
          [
            Quickfix.make
              ~title:("Change to " ^ Markdown_lite.md_codify return_type)
              ~new_text:return_type
              hint_pos;
          ]
      in

      (Error_code.NonVoidAnnotationOnReturnVoidFun, (pos, message), [], quickfix)

    let tuple_syntax p =
      ( Error_code.TupleSyntax,
        (p, "Did you want a *tuple*? Try `(X,Y)`, not `tuple<X,Y>`"),
        [],
        [] )

    let to_error = function
      | Missing_return pos -> missing_return pos
      | Dollardollar_lvalue pos -> dollardollar_lvalue pos
      | Void_usage { pos; reason } -> void_usage pos reason
      | Noreturn_usage { pos; reason } -> noreturn_usage pos reason
      | Returns_with_and_without_value
          { pos; with_value_pos; without_value_pos_opt } ->
        returns_with_and_without_value pos with_value_pos without_value_pos_opt
      | Missing_assign pos -> missing_assign pos
      | Non_void_annotation_on_return_void_function { is_async; pos; hint_pos }
        ->
        non_void_annotation_on_return_void_function is_async pos hint_pos
      | Tuple_syntax pos -> tuple_syntax pos
  end

  module Modules = struct
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

    let module_hint pos decl_pos =
      let claim = (pos, "You cannot use this type in a public declaration.")
      and reason = [(decl_pos, "It is declared as `internal` here")] in
      (Error_code.ModuleHintError, claim, reason, [])

    let module_mismatch pos current_module_opt decl_pos target_module =
      let claim =
        ( pos,
          Printf.sprintf
            "Cannot access an internal element from module `%s` %s"
            target_module
            (match current_module_opt with
            | Some m -> Printf.sprintf "in module `%s`" m
            | None -> "outside of a module") )
      and reason =
        [(decl_pos, Printf.sprintf "This is from module `%s`" target_module)]
      in
      (Error_code.ModuleError, claim, reason, [])

    let to_error = function
      | Module_hint { pos; decl_pos } -> module_hint pos decl_pos
      | Module_mismatch { pos; current_module_opt; decl_pos; target_module } ->
        module_mismatch pos current_module_opt decl_pos target_module
  end

  module Xhp = struct
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

    let xhp_required pos why_xhp ty_reason_msg =
      let msg = "An XHP instance was expected" in
      ( Error_code.XhpRequired,
        (pos, msg),
        (Pos_or_decl.of_raw_pos pos, why_xhp) :: ty_reason_msg,
        [] )

    let illegal_xhp_child pos ty_reason_msg =
      let msg = "XHP children must be compatible with XHPChild" in
      (Error_code.IllegalXhpChild, (pos, msg), ty_reason_msg, [])

    let missing_xhp_required_attr pos attr ty_reason_msg =
      let msg =
        "Required attribute " ^ Markdown_lite.md_codify attr ^ " is missing."
      in
      (Error_code.MissingXhpRequiredAttr, (pos, msg), ty_reason_msg, [])

    let to_error = function
      | Xhp_required { pos; why_xhp; ty_reason_msg } ->
        xhp_required pos why_xhp @@ Lazy.force ty_reason_msg
      | Illegal_xhp_child { pos; ty_reason_msg } ->
        illegal_xhp_child pos @@ Lazy.force ty_reason_msg
      | Missing_xhp_required_attr { pos; attr; ty_reason_msg } ->
        missing_xhp_required_attr pos attr @@ Lazy.force ty_reason_msg
  end

  type t =
    (* == Factorised errors ================================================= *)
    | Coeffect of Coeffect.t
    | Enum of Enum.t
    | Expr_tree of Expr_tree.t
    | Ifc of Ifc.t
    | Modules of Modules.t
    | Readonly of Readonly.t
    | Record of Record.t
    | Shape of Shape.t
    | Wellformedness of Wellformedness.t
    | Xhp of Xhp.t
    (* == Primary only ====================================================== *)
    | Exception_occurred of {
        pos: Pos.t;
        exn: Exception.t;
      }
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
        parent_pos: Pos_or_decl.t;
        parent_name: string;
      }
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
        reason: Pos_or_decl.t Message.t list;
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
    | Invalid_substring of {
        pos: Pos.t;
        ty_name: string Lazy.t;
      }
    | Unset_nonidx_in_strict of {
        pos: Pos.t;
        reason: Pos_or_decl.t Message.t list;
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
        reason: Pos_or_decl.t Message.t list;
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
        parent_pos: Pos_or_decl.t;
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
        uses: Pos_or_decl.t Message.t list;
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
    | Cyclic_record_def of {
        pos: Pos.t;
        names: string list;
      }
    | Trait_reuse_with_final_method of {
        pos: Pos.t;
        trait_name: string;
        parent_cls_name: string;
        trace: Pos_or_decl.t Message.t list;
      }
    | Trait_reuse of {
        pos: Pos.t;
        class_name: string;
        trait_name: string;
        parent_pos: Pos_or_decl.t;
        parent_name: string;
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
        reasons: Pos_or_decl.t Message.t list;
      }
    | Invalid_enforceable_type of {
        pos: Pos.t;
        ty_info: Pos_or_decl.t Message.t list;
        tp_pos: Pos_or_decl.t;
        tp_name: string;
        kind: [ `constant | `param ];
      }
    | Reifiable_attr of {
        pos: Pos.t;
        ty_info: Pos_or_decl.t Message.t list;
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
        kind: [ `enum | `record ];
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
    | Illegal_type_structure of Pos.t
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
        reason: Pos.t Message.t;
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
        trace1: Pos_or_decl.t Message.t list;
        trace2: Pos_or_decl.t Message.t list;
      }
    | Generic_property_import_via_diamond of {
        pos: Pos.t;
        class_name: string;
        property_pos: Pos_or_decl.t;
        property_name: string;
        trace1: Pos_or_decl.t Message.t list;
        trace2: Pos_or_decl.t Message.t list;
      }
    | Unification_cycle of {
        pos: Pos.t;
        ty_name: string Lazy.t;
      }
    | Method_variance of Pos.t
    | Explain_tconst_where_constraint of {
        pos: Pos.t;
        decl_pos: Pos_or_decl.t;
        msgs: Pos_or_decl.t Message.t list;
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
        reason: Pos_or_decl.t Message.t list;
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
    | Extend_non_abstract_record of {
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
        name: string;
        kind: [ `meth | `prop | `const | `ty_const ];
        quickfixes: Quickfix.t list;
      }
    | Generic_static of {
        pos: Pos.t;
        typaram_name: string;
      }
    | Ellipsis_strict_mode of {
        pos: Pos.t;
        require: [ `Param_name | `Type_and_param_name ];
      }
    | Untyped_lambda_strict_mode of Pos.t
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
        arg_info: Pos_or_decl.t Message.t list;
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
        reason: Pos_or_decl.t Message.t list;
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
        null_witness: Pos_or_decl.t Message.t list;
      }
    | Option_mixed of Pos.t
    | Option_null of Pos.t
    | Declared_covariant of {
        pos: Pos.t;
        param_pos: Pos.t;
        msgs: Pos.t Message.t list;
      }
    | Declared_contravariant of {
        pos: Pos.t;
        param_pos: Pos.t;
        msgs: Pos.t Message.t list;
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
        pos: Pos.t;
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

  (* == User error helpers ================================================== *)

  let unify_error pos msg_opt reasons_opt =
    let claim = (pos, Option.value ~default:"Typing error" msg_opt)
    and reasons = Option.value_map ~default:[] ~f:Lazy.force reasons_opt in
    (Error_code.UnifyError, claim, reasons, [])

  let generic_unify pos msg =
    let claim = (pos, msg) in
    (Error_code.GenericUnify, claim, [], [])

  let internal_error pos msg =
    (Error_code.InternalError, (pos, "Internal error: " ^ msg), [], [])

  let typechecker_timeout pos fn_name seconds =
    let claim =
      ( pos,
        Printf.sprintf
          "Type checker timed out after %d seconds whilst checking function %s"
          seconds
          fn_name )
    in
    (Error_code.TypecheckerTimeout, claim, [], [])

  let unresolved_tyvar pos =
    let claim =
      (pos, "The type of this expression contains an unresolved type variable")
    in
    (Error_code.UnresolvedTypeVariable, claim, [], [])

  let using_error pos has_await =
    let (note, cls) =
      if has_await then
        (" with await", Naming_special_names.Classes.cIAsyncDisposable)
      else
        ("", Naming_special_names.Classes.cIDisposable)
    in
    let claim =
      ( pos,
        Printf.sprintf
          "This expression is used in a `using` clause%s so it must have type `%s`"
          note
          cls )
    in
    (Error_code.UnifyError, claim, [], [])

  let bad_enum_decl pos =
    ( Error_code.BadEnumExtends,
      (pos, "This enum declaration is invalid."),
      [],
      [] )

  let bad_conditional_support_dynamic pos child parent ty_name self_ty_name =
    let statement =
      Lazy.force ty_name
      ^ " is subtype of dynamic implies "
      ^ Lazy.force self_ty_name
      ^ " is subtype of dynamic"
    in
    ( Error_code.BadConditionalSupportDynamic,
      ( pos,
        "Class "
        ^ Render.strip_ns child
        ^ " must support dynamic at least as often as "
        ^ Render.strip_ns parent
        ^ ":\n"
        ^ statement ),
      [],
      [] )

  let bad_decl_override pos name parent_pos parent_name =
    ( Error_code.BadDeclOverride,
      ( pos,
        "Class "
        ^ (Render.strip_ns name |> Markdown_lite.md_codify)
        ^ " does not correctly implement all required members " ),
      [
        ( parent_pos,
          "Some members are incompatible with those declared in type "
          ^ (Render.strip_ns parent_name |> Markdown_lite.md_codify) );
      ],
      [] )

  let explain_where_constraint pos decl_pos in_class =
    ( Error_code.TypeConstraintViolation,
      (pos, "A `where` type constraint is violated here"),
      [
        ( decl_pos,
          Printf.sprintf "This is the %s with `where` type constraints"
          @@
          if in_class then
            "class"
          else
            "method" );
      ],
      [] )

  let explain_constraint pos =
    ( Error_code.TypeConstraintViolation,
      (pos, "Some type arguments violate their constraints"),
      [],
      [] )

  let rigid_tvar_escape pos what =
    ( Error_code.RigidTVarEscape,
      (pos, "Rigid type variable escapes its " ^ what),
      [],
      [] )

  let invalid_type_hint pos =
    (Error_code.InvalidTypeHint, (pos, "Invalid type hint"), [], [])

  let unsatisfied_req pos trait_pos req_name req_pos =
    let reasons =
      let r =
        ( trait_pos,
          "This requires to extend or implement " ^ Render.strip_ns req_name )
      in
      if Pos_or_decl.equal trait_pos req_pos then
        [r]
      else
        [r; (req_pos, "Required here")]
    and claim =
      ( pos,
        "This class does not satisfy all the requirements of its traits or interfaces."
      )
    in
    (Error_code.UnsatisfiedReq, claim, reasons, [])

  let invalid_echo_argument pos =
    let claim =
      ( pos,
        "Invalid "
        ^ Markdown_lite.md_codify "echo"
        ^ "/"
        ^ Markdown_lite.md_codify "print"
        ^ " argument" )
    in
    (Error_code.InvalidEchoArgument, claim, [], [])

  let index_type_mismatch pos is_covariant_container msg_opt reasons_opt =
    let code =
      if is_covariant_container then
        Error_code.CovariantIndexTypeMismatch
      else
        Error_code.IndexTypeMismatch
    and claim = (pos, Option.value ~default:"Invalid index expression" msg_opt)
    and reasons = Option.value_map reasons_opt ~default:[] ~f:Lazy.force in
    (code, claim, reasons, [])

  let member_not_found pos kind member_name class_name class_pos hint reason =
    let kind_str =
      match kind with
      | `method_ -> "instance method"
      | `property -> "property"
    in
    let msg =
      Printf.sprintf
        "No %s %s in %s"
        kind_str
        (Markdown_lite.md_codify member_name)
        (Markdown_lite.md_codify @@ Render.strip_ns class_name)
    in
    let default =
      reason
      @ [
          ( class_pos,
            "Declaration of "
            ^ (Markdown_lite.md_codify @@ Render.strip_ns class_name)
            ^ " is here" );
        ]
    in
    let reasons =
      Option.value_map hint ~default ~f:(function
          | (`instance, pos, v) ->
            Render.suggestion_message member_name v pos :: default
          | (`static, pos, v) ->
            let modifier =
              match kind with
              | `method_ -> "static method "
              | `property -> "static property "
            in
            Render.suggestion_message member_name ~modifier v pos :: default)
    in
    let quickfixes =
      Option.value_map hint ~default:[] ~f:(fun (_, _, new_text) ->
          [Quickfix.make ~title:("Change to ->" ^ new_text) ~new_text pos])
    in
    (Error_code.MemberNotFound, (pos, msg), reasons, quickfixes)

  let construct_not_instance_method pos =
    let claim =
      ( pos,
        "`__construct` is not an instance method and shouldn't be invoked directly"
      )
    in
    (Error_code.ConstructNotInstanceMethod, claim, [], [])

  let ambiguous_inheritance pos origin class_name =
    let claim =
      ( pos,
        "This declaration was inherited from an object of type "
        ^ Markdown_lite.md_codify origin
        ^ ". Redeclare this member in "
        ^ Markdown_lite.md_codify class_name
        ^ " with a compatible signature." )
    in
    (Error_code.UnifyError, claim, [], [])

  let expected_tparam pos n decl_pos =
    let claim =
      ( pos,
        "Expected "
        ^
        match n with
        | 0 -> "no type parameters"
        | 1 -> "exactly one type parameter"
        | n -> string_of_int n ^ " type parameters" )
    and reasons = [(decl_pos, "Definition is here")] in
    (Error_code.ExpectedTparam, claim, reasons, [])

  let typeconst_concrete_concrete_override pos decl_pos =
    let reasons = [(decl_pos, "Previously defined here")]
    and claim = (pos, "Cannot re-declare this type constant") in
    (Error_code.TypeconstConcreteConcreteOverride, claim, reasons, [])

  let invalid_memoized_param pos reason =
    let claim =
      ( pos,
        "Parameters to memoized function must be null, bool, int, float, string, an object deriving IMemoizeParam, or a Container thereof. See also http://docs.hhvm.com/hack/attributes/special#__memoize"
      )
    in
    (Error_code.InvalidMemoizedParam, claim, Lazy.force reason, [])

  let invalid_arraykey
      pos container_pos container_ty_name key_pos key_ty_name ctxt =
    let reasons =
      [
        (container_pos, "This container is " ^ container_ty_name);
        ( key_pos,
          String.capitalize key_ty_name
          ^ " cannot be used as a key for "
          ^ container_ty_name );
      ]
    and claim = (pos, "This value is not a valid key type for this container")
    and code =
      Error_code.(
        match ctxt with
        | `read -> InvalidArrayKeyRead
        | `write -> InvalidArrayKeyWrite)
    in
    (code, claim, reasons, [])

  let invalid_keyset_value
      pos container_pos container_ty_name value_pos value_ty_name =
    let reasons =
      [
        (container_pos, "This container is " ^ container_ty_name);
        (value_pos, String.capitalize value_ty_name ^ " is not an arraykey");
      ]
    and claim = (pos, "Keyset values must be arraykeys") in
    (Error_code.InvalidKeysetValue, claim, reasons, [])

  let invalid_set_value
      pos container_pos container_ty_name value_pos value_ty_name =
    let reasons =
      [
        (container_pos, "This container is " ^ container_ty_name);
        (value_pos, String.capitalize value_ty_name ^ " is not an arraykey");
      ]
    and claim = (pos, "Set values must be arraykeys") in
    (Error_code.InvalidKeysetValue, claim, reasons, [])

  let hkt_alias_with_implicit_constraints
      pos
      typedef_name
      typedef_pos
      used_class_in_def_pos
      used_class_in_def_name
      used_class_tparam_name
      typedef_tparam_name =
    let reasons =
      [
        ( typedef_pos,
          "The definition of " ^ Render.strip_ns typedef_name ^ " is here." );
        ( used_class_in_def_pos,
          "The definition of "
          ^ Render.strip_ns typedef_name
          ^ " relies on "
          ^ Render.strip_ns used_class_in_def_name
          ^ " and the constraints that "
          ^ Render.strip_ns used_class_in_def_name
          ^ " imposes on its type parameter "
          ^ Render.strip_ns used_class_tparam_name
          ^ " then become implicit constraints on the type parameter "
          ^ typedef_tparam_name
          ^ " of "
          ^ Render.strip_ns typedef_name
          ^ "." );
      ]
    and claim =
      ( pos,
        Format.sprintf
          "The type %s implicitly imposes constraints on its type parameters. Therefore, it cannot be used as a higher-kinded type at this time."
        @@ Render.strip_ns typedef_name )
    in
    (Error_code.HigherKindedTypesUnsupportedFeature, claim, reasons, [])

  let invalid_substring pos ty_name =
    let claim =
      ( pos,
        "Expected an object convertible to string but got " ^ Lazy.force ty_name
      )
    in
    (Error_code.InvalidSubString, claim, [], [])

  let nullable_cast pos ty_pos ty_name =
    let reasons =
      [(ty_pos, "This is " ^ Markdown_lite.md_codify (Lazy.force ty_name))]
    and claim = (pos, "Casting from a nullable type is forbidden") in
    (Error_code.NullableCast, claim, reasons, [])

  let hh_expect pos equivalent =
    let (msg, error_code) =
      if equivalent then
        ( "hh_expect_equivalent type mismatch",
          Error_code.HHExpectEquivalentFailure )
      else
        ("hh_expect type mismatch", Error_code.HHExpectFailure)
    in

    (error_code, (pos, msg), [], [])

  let null_member pos ctxt kind member_name reason =
    let msg =
      Printf.sprintf
        "You are trying to access the %s %s but this object can be null."
        (match kind with
        | `method_ -> "method"
        | `property -> "property")
        (Markdown_lite.md_codify member_name)
    in
    let error_code =
      match ctxt with
      | `read -> Error_code.NullMemberRead
      | `write -> Error_code.NullMemberWrite
    in
    (error_code, (pos, msg), reason, [])

  let typing_too_many_args pos decl_pos actual expected =
    let (code, claim, reasons) =
      Common.typing_too_many_args pos decl_pos actual expected
    in
    (code, claim, reasons, [])

  let typing_too_few_args pos decl_pos actual expected =
    let (code, claim, reasons) =
      Common.typing_too_few_args pos decl_pos actual expected
    in
    (code, claim, reasons, [])

  let non_object_member pos ctxt ty_name member_name kind decl_pos =
    let (code, claim, reasons) =
      Common.non_object_member
        pos
        ctxt
        (Lazy.force ty_name)
        member_name
        kind
        decl_pos
    in
    (code, claim, reasons, [])

  let nullsafe_property_write_context pos =
    let msg =
      "`?->` syntax not supported here, this function effectively does a write"
    in
    (Error_code.NullsafePropertyWriteContext, (pos, msg), [], [])

  let uninstantiable_class pos class_name reason_ty_opt decl_pos =
    let name = Render.strip_ns class_name in
    let default =
      ( (pos, Markdown_lite.md_codify name ^ " is uninstantiable"),
        [(decl_pos, "Declaration is here")] )
    in
    let (claim, reasons) =
      Option.value_map reason_ty_opt ~default ~f:(fun (reason_pos, ty_name) ->
          let msg =
            "This would be "
            ^ Lazy.force ty_name
            ^ " which must be instantiable"
          in
          let reasons =
            Message.map ~f:Pos_or_decl.of_raw_pos (fst default) :: snd default
          in
          let claim = (reason_pos, msg) in
          (claim, reasons))
    in
    (Error_code.UninstantiableClass, claim, reasons, [])

  let abstract_const_usage pos name decl_pos =
    let name = Render.strip_ns name in
    let msg =
      "Cannot reference abstract constant "
      ^ Markdown_lite.md_codify name
      ^ " directly"
    and reason = [(decl_pos, "Declaration is here")] in
    (Error_code.AbstractConstUsage, (pos, msg), reason, [])

  let type_arity_mismatch pos decl_pos actual expected =
    let msg =
      Printf.sprintf
        "Wrong number of type arguments (expected %d, got %d)"
        expected
        actual
    and reasons = [(decl_pos, "Definition is here")] in
    (Error_code.TypeArityMismatch, (pos, msg), reasons, [])

  let member_not_implemented pos parent_pos member_name decl_pos quickfixes =
    let claim =
      ( pos,
        "This type doesn't implement the method "
        ^ Markdown_lite.md_codify member_name )
    and reasons =
      [
        (parent_pos, "Which is required by this interface");
        (decl_pos, "As defined here");
      ]
    in
    (Error_code.MemberNotImplemented, claim, reasons, quickfixes)

  let attribute_too_many_arguments pos name expected =
    let msg =
      "The attribute "
      ^ Markdown_lite.md_codify name
      ^ " expects at most "
      ^ Render.pluralize_arguments expected
    in
    (Error_code.AttributeTooManyArguments, (pos, msg), [], [])

  let attribute_too_few_arguments pos name expected =
    let msg =
      "The attribute "
      ^ Markdown_lite.md_codify name
      ^ " expects at least "
      ^ Render.pluralize_arguments expected
    in
    (Error_code.AttributeTooFewArguments, (pos, msg), [], [])

  let attribute_not_exact_number_of_args pos name actual expected =
    let code =
      if actual > expected then
        Error_code.AttributeTooManyArguments
      else
        Error_code.AttributeTooFewArguments
    and msg =
      "The attribute "
      ^ Markdown_lite.md_codify name
      ^ " expects "
      ^
      match expected with
      | 0 -> "no arguments"
      | 1 -> "exactly 1 argument"
      | _ -> "exactly " ^ string_of_int expected ^ " arguments"
    in
    (code, (pos, msg), [], [])

  let kind_mismatch pos decl_pos tparam_name expected_kind actual_kind =
    let msg =
      "This is "
      ^ actual_kind
      ^ ", but "
      ^ expected_kind
      ^ " was expected here."
    and reason =
      [
        ( decl_pos,
          "We are expecting "
          ^ expected_kind
          ^ " due to the definition of "
          ^ tparam_name
          ^ " here." );
      ]
    in
    (Error_code.KindMismatch, (pos, msg), reason, [])

  let trait_parent_construct_inconsistent pos decl_pos =
    let msg =
      "This use of `parent::__construct` requires that the parent class be marked <<__ConsistentConstruct>>"
    and reason = [(decl_pos, "Parent definition is here")] in
    (Error_code.TraitParentConstructInconsistent, (pos, msg), reason, [])

  let top_member pos ctxt ty_name decl_pos kind name is_nullable =
    let kind_str =
      match kind with
      | `method_ -> "method"
      | `property -> "property"
    in
    let claim =
      ( pos,
        Printf.sprintf
          "You are trying to access the %s %s but this is %s. Use a **specific** class or interface name."
          kind_str
          (Markdown_lite.md_codify name)
          (Lazy.force ty_name) )
    and reason = [(decl_pos, "Definition is here")]
    and code =
      Error_code.(
        match ctxt with
        | `read when is_nullable -> NullMemberRead
        | `write when is_nullable -> NullMemberWrite
        | `read -> NonObjectMemberRead
        | `write -> NonObjectMemberWrite)
    in
    (code, claim, reason, [])

  let unresolved_tyvar_projection pos proj_pos tconst_name =
    let claim =
      ( pos,
        "Can't access a type constant "
        ^ tconst_name
        ^ " from an unresolved type" )
    and reason =
      [
        (proj_pos, "Access happens here");
        ( Pos_or_decl.of_raw_pos pos,
          "Disambiguate the types using explicit type annotations here." );
      ]
    in
    (Error_code.UnresolvedTypeVariableProjection, claim, reason, [])

  let cyclic_class_constant pos class_name const_name =
    let claim =
      ( pos,
        "Cannot declare self-referencing constant "
        ^ const_name
        ^ " in "
        ^ Render.strip_ns class_name )
    in
    (Error_code.CyclicClassConstant, claim, [], [])

  let inout_annotation_missing pos1 pos2 =
    let msg1 = (pos1, "This argument should be annotated with `inout`") in
    let msg2 = (pos2, "Because this is an `inout` parameter") in
    let (_, start_column) = Pos.line_column pos1 in
    let pos = Pos.set_col_end start_column pos1 in

    ( Error_code.InoutAnnotationMissing,
      msg1,
      [msg2],
      [Quickfix.make ~title:"Insert `inout` annotation" ~new_text:"inout " pos]
    )

  let inout_annotation_unexpected pos1 pos2 pos2_is_variadic pos3 =
    let msg1 = (pos1, "Unexpected `inout` annotation for argument") in
    let msg2 =
      ( pos2,
        if pos2_is_variadic then
          "A variadic parameter can never be `inout`"
        else
          "This is a normal parameter (does not have `inout`)" )
    in
    ( Error_code.InoutAnnotationUnexpected,
      msg1,
      [msg2],
      [Quickfix.make ~title:"Remove `inout` annotation" ~new_text:"" pos3] )

  let inout_argument_bad_type pos reasons =
    let claim =
      ( pos,
        "Expected argument marked `inout` to be contained in a local or "
        ^ "a value-typed container (e.g. vec, dict, keyset, array). "
        ^ "To use `inout` here, assign to/from a temporary local variable." )
    in
    (Error_code.InoutArgumentBadType, claim, Lazy.force reasons, [])

  let invalid_meth_caller_calling_convention pos decl_pos convention =
    let claim =
      ( pos,
        "`meth_caller` does not support methods with the "
        ^ convention
        ^ " calling convention" )
    and reason =
      [
        ( decl_pos,
          "This is why I think this method uses the `inout` calling convention"
        );
      ]
    in
    (Error_code.InvalidMethCallerCallingConvention, claim, reason, [])

  let invalid_new_disposable pos =
    let claim =
      ( pos,
        "Disposable objects may only be created in a `using` statement or `return` from function marked `<<__ReturnDisposable>>`"
      )
    in
    (Error_code.InvalidNewDisposable, claim, [], [])

  let invalid_return_disposable pos =
    let claim =
      ( pos,
        "Return expression must be new disposable in function marked `<<__ReturnDisposable>>`"
      )
    in
    (Error_code.InvalidReturnDisposable, claim, [], [])

  let invalid_disposable_hint pos class_name =
    let claim =
      ( pos,
        "Parameter with type "
        ^ Markdown_lite.md_codify class_name
        ^ " must not implement `IDisposable` or `IAsyncDisposable`. "
        ^ "Please use `<<__AcceptDisposable>>` attribute or create disposable object with `using` statement instead."
      )
    in
    (Error_code.InvalidDisposableHint, claim, [], [])

  let invalid_disposable_return_hint pos class_name =
    let claim =
      ( pos,
        "Return type "
        ^ Markdown_lite.md_codify class_name
        ^ " must not implement `IDisposable` or `IAsyncDisposable`. Please add `<<__ReturnDisposable>>` attribute."
      )
    in
    (Error_code.InvalidDisposableReturnHint, claim, [], [])

  let ambiguous_lambda pos uses =
    let claim =
      ( pos,
        "Lambda has parameter types that could not be determined at definition site."
      )
    and reason =
      ( Pos_or_decl.of_raw_pos pos,
        Printf.sprintf
          "%d distinct use types were determined: please add type hints to lambda parameters."
          (List.length uses) )
      :: List.map uses ~f:(fun (pos, ty) ->
             (pos, "This use has type " ^ Markdown_lite.md_codify ty))
    in
    (Error_code.AmbiguousLambda, claim, reason, [])

  let smember_not_found pos kind member_name class_name class_pos hint =
    let (code, claim, reasons) =
      Common.smember_not_found pos kind member_name class_name class_pos hint
    in
    let quickfixes =
      Option.value_map hint ~default:[] ~f:(fun (_, _, new_text) ->
          Quickfix.[make ~title:("Change to ::" ^ new_text) ~new_text pos])
    in
    (code, claim, reasons, quickfixes)

  let wrong_extend_kind pos kind name parent_pos parent_kind parent_name =
    let parent_kind_str = Ast_defs.string_of_classish_kind parent_kind in
    let parent_name = Render.strip_ns parent_name in
    let child_name = Render.strip_ns name in
    let use_msg =
      Printf.sprintf
        " Did you mean to add `use %s;` within the body of %s?"
        parent_name
        (Markdown_lite.md_codify child_name)
    in
    let child_msg =
      match kind with
      | Ast_defs.Cclass _ ->
        let extends_msg = "Classes can only extend other classes." in
        let suggestion =
          if Ast_defs.is_c_interface parent_kind then
            " Did you mean `implements " ^ parent_name ^ "`?"
          else if Ast_defs.is_c_trait parent_kind then
            use_msg
          else
            ""
        in
        extends_msg ^ suggestion
      | Ast_defs.Cinterface ->
        let extends_msg = "Interfaces can only extend other interfaces." in
        let suggestion =
          if Ast_defs.is_c_trait parent_kind then
            use_msg
          else
            ""
        in
        extends_msg ^ suggestion
      | Ast_defs.Cenum_class _ ->
        "Enum classes can only extend other enum classes."
      | Ast_defs.Cenum ->
        (* This case should never happen, as the type checker will have already caught
           it with EnumTypeBad. But just in case, report this error here too. *)
        "Enums can only extend int, string, or arraykey."
      | Ast_defs.Ctrait ->
        (* This case should never happen, as the parser will have caught it before
            we get here. *)
        "A trait cannot use `extends`. This is a parser error."
    in
    let msg1 = (pos, child_msg) in
    let msg2 = (parent_pos, "This is " ^ parent_kind_str ^ ".") in
    (Error_code.WrongExtendKind, msg1, [msg2], [])

  let cyclic_class_def pos stack =
    let stack =
      SSet.fold
        (fun x y -> (Render.strip_ns x |> Markdown_lite.md_codify) ^ " " ^ y)
        stack
        ""
    in
    ( Error_code.CyclicClassDef,
      (pos, "Cyclic class definition : " ^ stack),
      [],
      [] )

  let cyclic_record_def pos names =
    let names =
      List.map ~f:(fun n -> Render.strip_ns n |> Markdown_lite.md_codify) names
    in
    ( Error_code.CyclicRecordDef,
      ( pos,
        Printf.sprintf
          "Record inheritance cycle: %s"
          (String.concat ~sep:" " names) ),
      [],
      [] )

  let trait_reuse_with_final_method use_pos trait_name parent_cls_name trace =
    let msg =
      Printf.sprintf
        "Traits with final methods cannot be reused, and `%s` is already used by `%s`."
        (Render.strip_ns trait_name)
        (Render.strip_ns parent_cls_name)
    in
    (Error_code.TraitReuse, (use_pos, msg), trace, [])

  let trait_reuse pos c_name trait_name parent_pos parent_name =
    let c_name = Render.strip_ns c_name |> Markdown_lite.md_codify in
    let trait = Render.strip_ns trait_name |> Markdown_lite.md_codify in
    let err =
      "Class " ^ c_name ^ " reuses trait " ^ trait ^ " in its hierarchy"
    in
    let err' =
      "It is already used through "
      ^ (Render.strip_ns parent_name |> Markdown_lite.md_codify)
    in
    (Error_code.TraitReuse, (pos, err), [(parent_pos, err')], [])

  let trait_reuse_inside_class c_pos c_name trait occurrences =
    let c_name = Render.strip_ns c_name |> Markdown_lite.md_codify in
    let trait = Render.strip_ns trait |> Markdown_lite.md_codify in
    let err = "Class " ^ c_name ^ " uses trait " ^ trait ^ " multiple times" in
    ( Error_code.TraitReuseInsideClass,
      (c_pos, err),
      List.map ~f:(fun p -> (p, "used here")) occurrences,
      [] )

  let invalid_is_as_expression_hint hint_pos op reasons =
    let op =
      match op with
      | `is -> "is"
      | `as_ -> "as"
    in
    ( Error_code.InvalidIsAsExpressionHint,
      (hint_pos, "Invalid " ^ Markdown_lite.md_codify op ^ " expression hint"),
      List.map reasons ~f:(fun (ty_pos, ty_str) ->
          ( ty_pos,
            "The "
            ^ Markdown_lite.md_codify op
            ^ " operator cannot be used with "
            ^ ty_str )),
      [] )

  let invalid_enforceable_type targ_pos ty_info kind tp_pos tp_name =
    let kind_str =
      match kind with
      | `constant -> "constant"
      | `param -> "parameter"
    in
    let (ty_pos, ty_str) = List.hd_exn ty_info in
    ( Error_code.InvalidEnforceableTypeArgument,
      (targ_pos, "Invalid type"),
      [
        ( tp_pos,
          "Type "
          ^ kind_str
          ^ " "
          ^ Markdown_lite.md_codify tp_name
          ^ " was declared `__Enforceable` here" );
        (ty_pos, "This type is not enforceable because it has " ^ ty_str);
      ],
      [] )

  let reifiable_attr attr_pos kind decl_pos ty_info =
    let decl_kind =
      match kind with
      | `ty -> "type"
      | `cnstr -> "constraint"
      | `super_cnstr -> "super_constraint"
    in
    let (ty_pos, ty_msg) = List.hd_exn ty_info in
    ( Error_code.DisallowPHPArraysAttr,
      (decl_pos, "Invalid " ^ decl_kind),
      [
        (attr_pos, "This type constant has the `__Reifiable` attribute");
        (ty_pos, "It cannot contain " ^ ty_msg);
      ],
      [] )

  let invalid_newable_type_argument pos tp_pos tp_name =
    ( Error_code.InvalidNewableTypeArgument,
      ( pos,
        "A newable type argument must be a concrete class or a newable type parameter."
      ),
      [
        ( tp_pos,
          "Type parameter "
          ^ Markdown_lite.md_codify tp_name
          ^ " was declared `__Newable` here" );
      ],
      [] )

  let invalid_newable_type_param_constraints
      (tparam_pos, tparam_name) constraint_list =
    let partial =
      if List.is_empty constraint_list then
        "No constraints"
      else
        "The constraints "
        ^ String.concat ~sep:", " (List.map ~f:Render.strip_ns constraint_list)
    in
    let msg =
      "The type parameter "
      ^ Markdown_lite.md_codify tparam_name
      ^ " has the `<<__Newable>>` attribute. "
      ^ "Newable type parameters must be constrained with `as`, and exactly one of those constraints must be a valid newable class. "
      ^ "The class must either be final, or it must have the `<<__ConsistentConstruct>>` attribute or extend a class that has it. "
      ^ partial
      ^ " are valid newable classes"
    in
    (Error_code.InvalidNewableTypeParamConstraints, (tparam_pos, msg), [], [])

  let override_per_trait class_name meth_name trait_name m_pos =
    let (c_pos, c_name) = class_name in
    let err_msg =
      Printf.sprintf
        "`%s::%s` is marked `__Override` but `%s` does not define or inherit a `%s` method."
        (Render.strip_ns trait_name)
        meth_name
        (Render.strip_ns c_name)
        meth_name
    in
    ( Error_code.OverridePerTrait,
      (c_pos, err_msg),
      [
        ( m_pos,
          "Declaration of " ^ Markdown_lite.md_codify meth_name ^ " is here" );
      ],
      [] )

  let generic_at_runtime p prefix =
    ( Error_code.ErasedGenericAtRuntime,
      ( p,
        prefix
        ^ " generics can only be used in type hints because they do not exist at runtime."
      ),
      [],
      [] )

  let generics_not_allowed p =
    ( Error_code.GenericsNotAllowed,
      (p, "Generics are not allowed in this position."),
      [],
      [] )

  let typedef_trail_entry pos = (pos, "Typedef definition comes from here")

  let trivial_strict_eq p b left right left_trail right_trail =
    let b =
      Markdown_lite.md_codify
        (if b then
          "true"
        else
          "false")
    in
    let msg = sprintf "This expression is always %s" b in
    let left_trail = List.map left_trail ~f:typedef_trail_entry in
    let right_trail = List.map right_trail ~f:typedef_trail_entry in
    ( Error_code.TrivialStrictEq,
      (p, msg),
      left @ left_trail @ right @ right_trail,
      [] )

  let trivial_strict_not_nullable_compare_null p result type_reason =
    let b =
      Markdown_lite.md_codify
        (if result then
          "true"
        else
          "false")
    in
    let msg = sprintf "This expression is always %s" b in
    (Error_code.NotNullableCompareNullTrivial, (p, msg), type_reason, [])

  let eq_incompatible_types p left right =
    let msg = "This equality test has incompatible types" in
    (Error_code.EqIncompatibleTypes, (p, msg), left @ right, [])

  let comparison_invalid_types p left right =
    let msg =
      "This comparison has invalid types.  Only comparisons in which both arguments are strings, nums, DateTime, or DateTimeImmutable are allowed"
    in
    (Error_code.ComparisonInvalidTypes, (p, msg), left @ right, [])

  let strict_eq_value_incompatible_types p left right =
    let msg =
      "The arguments to this value equality test are not the same types or are not the allowed types (int, bool, float, string, vec, keyset, dict). The behavior for this test is changing and will soon either be universally false or throw an exception."
    in
    (Error_code.StrictEqValueIncompatibleTypes, (p, msg), left @ right, [])

  let attribute_param_type pos x =
    ( Error_code.AttributeParamType,
      (pos, "This attribute parameter should be " ^ x),
      [],
      [] )

  let deprecated_use pos ?(pos_def = None) msg =
    let def_message =
      match pos_def with
      | Some pos_def -> [(pos_def, "Definition is here")]
      | None -> []
    in
    (Error_code.DeprecatedUse, (pos, msg), def_message, [])

  let cannot_declare_constant kind pos (class_pos, class_name) =
    let kind_str =
      match kind with
      | `enum -> "an enum"
      | `record -> "a record"
    in
    ( Error_code.CannotDeclareConstant,
      (pos, "Cannot declare a constant in " ^ kind_str),
      [
        ( Pos_or_decl.of_raw_pos class_pos,
          (Render.strip_ns class_name |> Markdown_lite.md_codify)
          ^ " was defined as "
          ^ kind_str
          ^ " here" );
      ],
      [] )

  let local_variable_modified_and_used pos_modified pos_used_l =
    let used_msg p = (Pos_or_decl.of_raw_pos p, "And accessed here") in
    ( Error_code.LocalVariableModifedAndUsed,
      ( pos_modified,
        "Unsequenced modification and access to local variable. Modified here"
      ),
      List.map pos_used_l ~f:used_msg,
      [] )

  let local_variable_modified_twice pos_modified pos_modified_l =
    let modified_msg p = (Pos_or_decl.of_raw_pos p, "And also modified here") in
    ( Error_code.LocalVariableModifedTwice,
      ( pos_modified,
        "Unsequenced modifications to local variable. Modified here" ),
      List.map pos_modified_l ~f:modified_msg,
      [] )

  let assign_during_case p =
    ( Error_code.AssignDuringCase,
      (p, "Don't assign to variables inside of case labels"),
      [],
      [] )

  let invalid_classname p =
    (Error_code.InvalidClassname, (p, "Not a valid class name"), [], [])

  let illegal_type_structure pos =
    let errmsg = "second argument is not a string" in
    let msg =
      "The two arguments to `type_structure()` must be:"
      ^ "\n - first: `ValidClassname::class` or an object of that class"
      ^ "\n - second: a single-quoted string literal containing the name"
      ^ " of a type constant of that class"
      ^ "\n"
      ^ errmsg
    in
    (Error_code.IllegalTypeStructure, (pos, msg), [], [])

  let illegal_typeconst_direct_access pos =
    let msg =
      "Type constants cannot be directly accessed. "
      ^ "Use `type_structure(ValidClassname::class, 'TypeConstName')` instead"
    in
    (Error_code.IllegalTypeStructure, (pos, msg), [], [])

  let wrong_expression_kind_attribute
      expr_kind pos attr attr_class_pos attr_class_name intf_name =
    let msg1 =
      Printf.sprintf
        "The %s attribute cannot be used on %s."
        (Render.strip_ns attr |> Markdown_lite.md_codify)
        expr_kind
    in
    let msg2 =
      Printf.sprintf
        "The attribute's class is defined here. To be available for use on %s, the %s class must implement %s."
        expr_kind
        (Render.strip_ns attr_class_name |> Markdown_lite.md_codify)
        (Render.strip_ns intf_name |> Markdown_lite.md_codify)
    in
    ( Error_code.WrongExpressionKindAttribute,
      (pos, msg1),
      [(attr_class_pos, msg2)],
      [] )

  let wrong_expression_kind_builtin_attribute expr_kind pos attr =
    let msg1 =
      Printf.sprintf
        "The %s attribute cannot be used on %s."
        (Render.strip_ns attr |> Markdown_lite.md_codify)
        expr_kind
    in
    (Error_code.WrongExpressionKindAttribute, (pos, msg1), [], [])

  let ambiguous_object_access
      pos name self_pos vis subclass_pos class_self class_subclass =
    let class_self = Render.strip_ns class_self in
    let class_subclass = Render.strip_ns class_subclass in
    ( Error_code.AmbiguousObjectAccess,
      ( pos,
        "This object access to "
        ^ Markdown_lite.md_codify name
        ^ " is ambiguous" ),
      [
        ( self_pos,
          "You will access the private instance declared in "
          ^ Markdown_lite.md_codify class_self );
        ( subclass_pos,
          "Instead of the "
          ^ vis
          ^ " instance declared in "
          ^ Markdown_lite.md_codify class_subclass );
      ],
      [] )

  let lateinit_with_default pos =
    ( Error_code.LateInitWithDefault,
      (pos, "A late-initialized property cannot have a default value"),
      [],
      [] )

  let unserializable_type pos message =
    ( Error_code.UnserializableType,
      ( pos,
        "Unserializable type (could not be converted to JSON and back again): "
        ^ message ),
      [],
      [] )

  let invalid_arraykey_constraint pos t =
    ( Error_code.InvalidArrayKeyConstraint,
      ( pos,
        "This type is "
        ^ t
        ^ ", which cannot be used as an arraykey (string | int)" ),
      [],
      [] )

  let redundant_covariant pos msg suggest =
    ( Error_code.RedundantGeneric,
      ( pos,
        "This generic parameter is redundant because it only appears in a covariant (output) position"
        ^ msg
        ^ ". Consider replacing uses of generic parameter with "
        ^ Markdown_lite.md_codify suggest
        ^ " or specifying `<<__Explicit>>` on the generic parameter" ),
      [],
      [] )

  let meth_caller_trait pos trait_name =
    ( Error_code.MethCallerTrait,
      ( pos,
        (Render.strip_ns trait_name |> Markdown_lite.md_codify)
        ^ " is a trait which cannot be used with `meth_caller`. Use a class instead."
      ),
      [],
      [] )

  let duplicate_interface pos name others =
    ( Error_code.DuplicateInterface,
      ( pos,
        Printf.sprintf
          "Interface %s is used more than once in this declaration."
          (Render.strip_ns name |> Markdown_lite.md_codify) ),
      List.map others ~f:(fun pos -> (pos, "Here is another occurrence")),
      [] )

  let tparam_non_shadowing_reuse pos var_name =
    ( Error_code.TypeParameterNameAlreadyUsedNonShadow,
      ( pos,
        "The name "
        ^ Markdown_lite.md_codify var_name
        ^ " was already used for another generic parameter. Please use a different name to avoid confusion."
      ),
      [],
      [] )

  let reified_function_reference call_pos =
    ( Error_code.ReifiedFunctionReference,
      ( call_pos,
        "Invalid function reference. This function requires reified generics. Prefer using a lambda instead."
      ),
      [],
      [] )

  let class_meth_abstract_call cname meth_name call_pos decl_pos =
    let cname = Render.strip_ns cname in
    ( Error_code.ClassMethAbstractCall,
      ( call_pos,
        "Cannot create a class_meth of "
        ^ cname
        ^ "::"
        ^ meth_name
        ^ "; it is abstract." ),
      [(decl_pos, "Declaration is here")],
      [] )

  let reinheriting_classish_const
      dest_classish_pos
      dest_classish_name
      src_classish_pos
      src_classish_name
      existing_const_origin
      const_name =
    ( Error_code.RedeclaringClassishConstant,
      ( src_classish_pos,
        Render.strip_ns dest_classish_name
        ^ " cannot re-inherit constant "
        ^ const_name
        ^ " from "
        ^ Render.strip_ns src_classish_name ),
      [
        ( Pos_or_decl.of_raw_pos dest_classish_pos,
          "because it already inherited it via "
          ^ Render.strip_ns existing_const_origin );
      ],
      [] )

  let redeclaring_classish_const
      classish_pos
      classish_name
      redeclaration_pos
      existing_const_origin
      const_name =
    ( Error_code.RedeclaringClassishConstant,
      ( redeclaration_pos,
        Render.strip_ns classish_name
        ^ " cannot re-declare constant "
        ^ const_name ),
      [
        ( Pos_or_decl.of_raw_pos classish_pos,
          "because it already inherited it via "
          ^ Render.strip_ns existing_const_origin );
      ],
      [] )

  let abstract_function_pointer cname meth_name call_pos decl_pos =
    let cname = Render.strip_ns cname in
    ( Error_code.AbstractFunctionPointer,
      ( call_pos,
        "Cannot create a function pointer to "
        ^ Markdown_lite.md_codify (cname ^ "::" ^ meth_name)
        ^ "; it is abstract" ),
      [(decl_pos, "Declaration is here")],
      [] )

  let unnecessary_attribute pos ~attr ~reason ~suggestion =
    let attr = Render.strip_ns attr in
    let (reason_pos, reason_msg) = reason in
    let suggestion =
      match suggestion with
      | None -> "Try deleting this attribute"
      | Some s -> s
    in
    ( Error_code.UnnecessaryAttribute,
      (pos, sprintf "The attribute `%s` is unnecessary" attr),
      [
        ( Pos_or_decl.of_raw_pos reason_pos,
          "It is unnecessary because " ^ reason_msg );
        (Pos_or_decl.of_raw_pos pos, suggestion);
      ],
      [] )

  let inherited_class_member_with_different_case
      member_type name name_prev p child_class prev_class prev_class_pos =
    let name = Render.strip_ns name in
    let name_prev = Render.strip_ns name_prev in
    let child_class = Render.strip_ns child_class in
    let prev_class = Render.strip_ns prev_class in
    let claim =
      ( p,
        child_class
        ^ " inherits a "
        ^ member_type
        ^ " named "
        ^ Markdown_lite.md_codify name_prev
        ^ " whose name differs from this one ("
        ^ Markdown_lite.md_codify name
        ^ ") only by case." )
    in
    let reasons =
      [
        ( prev_class_pos,
          "It was inherited from "
          ^ prev_class
          ^ " as "
          ^ (Render.highlight_differences name name_prev
            |> Markdown_lite.md_codify)
          ^ ". If you meant to override it, please use the same casing as the inherited "
          ^ member_type
          ^ "."
          ^ " Otherwise, please choose a different name for the new "
          ^ member_type );
      ]
    in
    (Error_code.InheritedMethodCaseDiffers, claim, reasons, [])

  let multiple_inherited_class_member_with_different_case
      ~member_type ~name1 ~name2 ~class1 ~class2 ~child_class ~child_p ~p1 ~p2 =
    let name1 = Render.strip_ns name1 in
    let name2 = Render.strip_ns name2 in
    let class1 = Render.strip_ns class1 in
    let class2 = Render.strip_ns class2 in
    let child_class = Render.strip_ns child_class in
    let claim =
      ( child_p,
        Markdown_lite.md_codify child_class
        ^ " inherited two versions of the "
        ^ member_type
        ^ " "
        ^ Markdown_lite.md_codify name1
        ^ " whose names differ only by case." )
    in
    let reasons =
      [
        ( p1,
          "It inherited "
          ^ Markdown_lite.md_codify name1
          ^ " from "
          ^ class1
          ^ " here." );
        ( p2,
          "And "
          ^ Markdown_lite.md_codify name2
          ^ " from "
          ^ class2
          ^ " here. Please rename these "
          ^ member_type
          ^ "s to the same casing." );
      ]
    in
    (Error_code.InheritedMethodCaseDiffers, claim, reasons, [])

  let classish_kind_to_string = function
    | Ast_defs.Cclass _ -> "class "
    | Ast_defs.Ctrait -> "trait "
    | Ast_defs.Cinterface -> "interface "
    | Ast_defs.Cenum_class _ -> "enum class "
    | Ast_defs.Cenum -> "enum "

  let parent_support_dynamic_type
      pos
      (child_name, child_kind)
      (parent_name, parent_kind)
      child_support_dynamic_type =
    let kinds_to_use child_kind parent_kind =
      match (child_kind, parent_kind) with
      | (_, Ast_defs.Cclass _) -> "extends "
      | (_, Ast_defs.Ctrait) -> "uses "
      | (Ast_defs.Cinterface, Ast_defs.Cinterface) -> "extends "
      | (_, Ast_defs.Cinterface) -> "implements "
      | (_, Ast_defs.Cenum_class _)
      | (_, Ast_defs.Cenum) ->
        ""
    in
    let child_name = Markdown_lite.md_codify (Render.strip_ns child_name) in
    let child_kind_s = classish_kind_to_string child_kind in
    let parent_name = Markdown_lite.md_codify (Render.strip_ns parent_name) in
    let parent_kind_s = classish_kind_to_string parent_kind in
    ( Error_code.ImplementsDynamic,
      ( pos,
        String.capitalize child_kind_s
        ^ child_name
        ^ (if child_support_dynamic_type then
            " cannot "
          else
            " must ")
        ^ "declare <<__SupportDynamicType>> because it "
        ^ kinds_to_use child_kind parent_kind
        ^ parent_kind_s
        ^ parent_name
        ^ " which does"
        ^
        if child_support_dynamic_type then
          " not"
        else
          "" ),
      [],
      [] )

  let property_is_not_enforceable pos prop_name class_name (prop_pos, prop_type)
      =
    let class_name = Markdown_lite.md_codify (Render.strip_ns class_name) in
    let prop_name = Markdown_lite.md_codify prop_name in
    let prop_type = Markdown_lite.md_codify prop_type in
    ( Error_code.ImplementsDynamic,
      ( pos,
        "Class "
        ^ class_name
        ^ " cannot support dynamic because property "
        ^ prop_name
        ^ " does not have an enforceable type" ),
      [(prop_pos, "Property " ^ prop_name ^ " has type " ^ prop_type)],
      [] )

  let property_is_not_dynamic pos prop_name class_name (prop_pos, prop_type) =
    let class_name = Markdown_lite.md_codify (Render.strip_ns class_name) in
    let prop_name = Markdown_lite.md_codify prop_name in
    let prop_type = Markdown_lite.md_codify prop_type in
    ( Error_code.ImplementsDynamic,
      ( pos,
        "Class "
        ^ class_name
        ^ " cannot support dynamic because property "
        ^ prop_name
        ^ " cannot be assigned to dynamic" ),
      [(prop_pos, "Property " ^ prop_name ^ " has type " ^ prop_type)],
      [] )

  let private_property_is_not_enforceable
      pos prop_name class_name (prop_pos, prop_type) =
    let class_name = Markdown_lite.md_codify (Render.strip_ns class_name) in
    let prop_name = Markdown_lite.md_codify prop_name in
    let prop_type = Markdown_lite.md_codify prop_type in
    ( Error_code.PrivateDynamicWrite,
      ( pos,
        "Cannot write to property "
        ^ prop_name
        ^ " through dynamic type because private property in "
        ^ class_name
        ^ " does not have an enforceable type" ),
      [(prop_pos, "Property " ^ prop_name ^ " has type " ^ prop_type)],
      [] )

  let private_property_is_not_dynamic
      pos prop_name class_name (prop_pos, prop_type) =
    let class_name = Markdown_lite.md_codify (Render.strip_ns class_name) in
    let prop_name = Markdown_lite.md_codify prop_name in
    let prop_type = Markdown_lite.md_codify prop_type in
    ( Error_code.PrivateDynamicRead,
      ( pos,
        "Cannot read from property "
        ^ prop_name
        ^ " through dynamic type because private property in "
        ^ class_name
        ^ " cannot be assigned to dynamic" ),
      [(prop_pos, "Property " ^ prop_name ^ " has type " ^ prop_type)],
      [] )

  let immutable_local pos =
    ( Error_code.ImmutableLocal,
      ( pos,
        (* TODO: generalize this error message in the future for arbitrary immutable locals *)
        "This variable cannot be reassigned because it is used for a dependent context"
      ),
      [],
      [] )

  let nonsense_member_selection pos kind =
    ( Error_code.NonsenseMemberSelection,
      ( pos,
        "Dynamic member access requires a local variable, not `" ^ kind ^ "`."
      ),
      [],
      [] )

  let consider_meth_caller pos class_name meth_name =
    ( Error_code.ConsiderMethCaller,
      ( pos,
        "Function pointer syntax requires a static method. "
        ^ "Use `meth_caller("
        ^ Render.strip_ns class_name
        ^ "::class, '"
        ^ meth_name
        ^ "')` to create a function pointer to the instance method" ),
      [],
      [] )

  let method_import_via_diamond
      pos class_name method_pos method_name trace1 trace2 =
    let class_name = Markdown_lite.md_codify (Render.strip_ns class_name) in
    let method_name = Markdown_lite.md_codify (Render.strip_ns method_name) in
    let msg1 =
      ( pos,
        "Class "
        ^ class_name
        ^ " inherits trait method "
        ^ method_name
        ^ " via multiple traits.  Remove the multiple paths or override the method"
      )
    in
    let msg2 = (method_pos, "Trait method is defined here") in
    (Error_code.DiamondTraitMethod, msg1, (msg2 :: trace1) @ trace2, [])

  let generic_property_import_via_diamond
      pos class_name property_pos property_name trace1 trace2 =
    let class_name = Markdown_lite.md_codify (Render.strip_ns class_name) in
    let property_name =
      Markdown_lite.md_codify (Render.strip_ns property_name)
    in
    let msg1 =
      ( pos,
        "Class "
        ^ class_name
        ^ " inherits generic trait property "
        ^ property_name
        ^ " via multiple traits.  Remove the multiple paths" )
    in
    let msg2 = (property_pos, "Trait property is defined here") in
    (Error_code.DiamondTraitProperty, msg1, (msg2 :: trace1) @ trace2, [])

  let unification_cycle pos ty =
    ( Error_code.UnificationCycle,
      ( pos,
        "Type circularity: in order to type-check this expression it "
        ^ "is necessary for a type [rec] to be equal to type "
        ^ Markdown_lite.md_codify ty ),
      [],
      [] )

  let method_variance pos =
    ( Error_code.MethodVariance,
      ( pos,
        "Covariance or contravariance is not allowed in type parameters of methods or functions."
      ),
      [],
      [] )

  let explain_tconst_where_constraint use_pos definition_pos msgl =
    let inst_msg = "A `where` type constraint is violated here" in

    ( Error_code.TypeConstraintViolation,
      (use_pos, inst_msg),
      [
        ( definition_pos,
          "This method's `where` constraints contain a generic type access" );
      ]
      @ msgl,
      [] )

  let format_string pos snippet s class_pos fname class_suggest =
    ( Error_code.FormatString,
      ( pos,
        "Invalid format string "
        ^ Markdown_lite.md_codify snippet
        ^ " in "
        ^ Markdown_lite.md_codify ("\"" ^ s ^ "\"") ),
      [
        ( class_pos,
          "You can add a new format specifier by adding "
          ^ Markdown_lite.md_codify (fname ^ "()")
          ^ " to "
          ^ Markdown_lite.md_codify class_suggest );
      ],
      [] )

  let expected_literal_format_string pos =
    ( Error_code.ExpectedLiteralFormatString,
      (pos, "This argument must be a literal format string"),
      [],
      [] )

  let re_prefixed_non_string pos reason =
    let non_strings =
      match reason with
      | `embedded_expr -> "Strings with embedded expressions"
      | `non_string -> "Non-strings"
    in
    ( Error_code.RePrefixedNonString,
      (pos, non_strings ^ " are not allowed to be to be `re`-prefixed"),
      [],
      [] )

  let bad_regex_pattern pos reason =
    let s =
      match reason with
      | `bad_patt s -> s
      | `empty_patt -> "This pattern is empty"
      | `missing_delim -> "Missing delimiter(s)"
      | `invalid_option -> "Invalid global option(s)"
    in
    (Error_code.BadRegexPattern, (pos, "Bad regex pattern; " ^ s ^ "."), [], [])

  let generic_array_strict p =
    ( Error_code.GenericArrayStrict,
      (p, "You cannot have an array without generics in strict mode"),
      [],
      [] )

  let option_return_only_typehint p kind =
    let (typehint, reason) =
      match kind with
      | `void -> ("?void", "only return implicitly")
      | `noreturn -> ("?noreturn", "never return")
    in
    ( Error_code.OptionReturnOnlyTypehint,
      ( p,
        Markdown_lite.md_codify typehint
        ^ " is a nonsensical typehint; a function cannot both "
        ^ reason
        ^ " and return null." ),
      [],
      [] )

  let redeclaring_missing_method p trait_method =
    ( Error_code.RedeclaringMissingMethod,
      ( p,
        "Attempting to redeclare a trait method "
        ^ Markdown_lite.md_codify trait_method
        ^ " which was never inherited. "
        ^ "You might be trying to redeclare a non-static method as `static` or vice-versa."
      ),
      [],
      [] )

  let expecting_type_hint p =
    (Error_code.ExpectingTypeHint, (p, "Was expecting a type hint"), [], [])

  let expecting_type_hint_variadic p =
    ( Error_code.ExpectingTypeHintVariadic,
      (p, "Was expecting a type hint on this variadic parameter"),
      [],
      [] )

  let expecting_return_type_hint p =
    ( Error_code.ExpectingReturnTypeHint,
      (p, "Was expecting a return type hint"),
      [],
      [] )

  let duplicate_using_var pos =
    ( Error_code.DuplicateUsingVar,
      (pos, "Local variable already used in `using` statement"),
      [],
      [] )

  let illegal_disposable pos verb =
    let verb =
      match verb with
      | `assigned -> "assigned"
    in
    ( Error_code.IllegalDisposable,
      ( pos,
        "Disposable objects must only be " ^ verb ^ " in a `using` statement" ),
      [],
      [] )

  let escaping_disposable pos =
    ( Error_code.EscapingDisposable,
      ( pos,
        "Variable from `using` clause may only be used as receiver in method invocation "
        ^ "or passed to function with `<<__AcceptDisposable>>` parameter attribute"
      ),
      [],
      [] )

  let escaping_disposable_parameter pos =
    ( Error_code.EscapingDisposableParameter,
      ( pos,
        "Parameter with `<<__AcceptDisposable>>` attribute may only be used as receiver in method invocation "
        ^ "or passed to another function with `<<__AcceptDisposable>>` parameter attribute"
      ),
      [],
      [] )

  let escaping_this pos =
    ( Error_code.EscapingThis,
      ( pos,
        "`$this` implementing `IDisposable` or `IAsyncDisposable` may only be used as receiver in method invocation "
        ^ "or passed to another function with `<<__AcceptDisposable>>` parameter attribute"
      ),
      [],
      [] )

  let must_extend_disposable pos =
    ( Error_code.MustExtendDisposable,
      ( pos,
        "A disposable type may not extend a class or use a trait that is not disposable"
      ),
      [],
      [] )

  let field_kinds pos1 pos2 =
    ( Error_code.FieldKinds,
      (pos1, "You cannot use this kind of field (value)"),
      [(pos2, "Mixed with this kind of field (key => value)")],
      [] )

  let unbound_name_typing pos name =
    ( Error_code.UnboundNameTyping,
      ( pos,
        "Unbound name (typing): "
        ^ Markdown_lite.md_codify (Render.strip_ns name) ),
      [],
      [] )

  let previous_default p =
    ( Error_code.PreviousDefault,
      ( p,
        "A previous parameter has a default value.\n"
        ^ "Remove all the default values for the preceding parameters,\n"
        ^ "or add a default value to this one." ),
      [],
      [] )

  let return_in_void pos1 pos2 =
    ( Error_code.ReturnInVoid,
      (pos1, "You cannot return a value"),
      [(Pos_or_decl.of_raw_pos pos2, "This is a `void` function")],
      [] )

  let this_var_outside_class p =
    ( Error_code.ThisVarOutsideClass,
      (p, "Can't use `$this` outside of a class"),
      [],
      [] )

  let unbound_global cst_pos =
    ( Error_code.UnboundGlobal,
      (cst_pos, "Unbound global constant (Typing)"),
      [],
      [] )

  let private_inst_meth use_pos def_pos =
    ( Error_code.PrivateInstMeth,
      ( use_pos,
        "You cannot use this method with `inst_meth` (whether you are in the same class or not)."
      ),
      [(def_pos, "It is declared as `private` here")],
      [] )

  let protected_inst_meth use_pos def_pos =
    ( Error_code.ProtectedInstMeth,
      ( use_pos,
        "You cannot use this method with `inst_meth` (whether you are in the same class hierarchy or not)."
      ),
      [(def_pos, "It is declared as `protected` here")],
      [] )

  let private_meth_caller use_pos def_pos =
    ( Error_code.PrivateMethCaller,
      ( use_pos,
        "You cannot access this method with `meth_caller` (even from the same class hierarchy)"
      ),
      [(def_pos, "It is declared as `private` here")],
      [] )

  let protected_meth_caller use_pos def_pos =
    ( Error_code.ProtectedMethCaller,
      ( use_pos,
        "You cannot access this method with `meth_caller` (even from the same class hierarchy)"
      ),
      [(def_pos, "It is declared as `protected` here")],
      [] )

  let private_class_meth use_pos def_pos =
    ( Error_code.PrivateClassMeth,
      ( use_pos,
        "You cannot use this method with `class_meth` (whether you are in the same class or not)."
      ),
      [(def_pos, "It is declared as `private` here")],
      [] )

  let protected_class_meth use_pos def_pos =
    ( Error_code.ProtectedClassMeth,
      ( use_pos,
        "You cannot use this method with `class_meth` (whether you are in the same class hierarchy or not)."
      ),
      [(def_pos, "It is declared as `protected` here")],
      [] )

  let array_cast pos =
    ( Error_code.ArrayCast,
      ( pos,
        "(array) cast forbidden; arrays with unspecified key and value types are not allowed"
      ),
      [],
      [] )

  let string_cast pos ty =
    ( Error_code.StringCast,
      ( pos,
        Printf.sprintf
          "Cannot cast a value of type %s to string. Only primitives may be used in a `(string)` cast."
          (Markdown_lite.md_codify ty) ),
      [],
      [] )

  let static_outside_class pos =
    ( Error_code.StaticOutsideClass,
      (pos, "`static` is undefined outside of a class"),
      [],
      [] )

  let self_outside_class pos =
    ( Error_code.SelfOutsideClass,
      (pos, "`self` is undefined outside of a class"),
      [],
      [] )

  let new_inconsistent_construct new_pos (cpos, cname) kind =
    let name = Render.strip_ns cname in
    let preamble =
      match kind with
      | `static ->
        "Can't use `new static()` for " ^ Markdown_lite.md_codify name
      | `classname ->
        "Can't use `new` on "
        ^ Markdown_lite.md_codify ("classname<" ^ name ^ ">")
    in
    ( Error_code.NewStaticInconsistent,
      ( new_pos,
        preamble
        ^ "; `__construct` arguments are not guaranteed to be consistent in child classes"
      ),
      [
        ( cpos,
          "This declaration is neither `final` nor uses the `<<__ConsistentConstruct>>` attribute"
        );
      ],
      [] )

  let undefined_parent pos =
    (Error_code.UndefinedParent, (pos, "The parent class is undefined"), [], [])

  let parent_outside_class pos =
    ( Error_code.ParentOutsideClass,
      (pos, "`parent` is undefined outside of a class"),
      [],
      [] )

  let parent_abstract_call call_pos meth_name decl_pos =
    ( Error_code.AbstractCall,
      ( call_pos,
        "Cannot call "
        ^ Markdown_lite.md_codify ("parent::" ^ meth_name ^ "()")
        ^ "; it is abstract" ),
      [(decl_pos, "Declaration is here")],
      [] )

  let self_abstract_call call_pos meth_name self_pos decl_pos =
    let quickfixes =
      [
        Quickfix.make
          ~title:
            ("Change to " ^ Markdown_lite.md_codify ("static::" ^ meth_name))
          ~new_text:"static"
          self_pos;
      ]
    in
    ( Error_code.AbstractCall,
      ( call_pos,
        "Cannot call "
        ^ Markdown_lite.md_codify ("self::" ^ meth_name ^ "()")
        ^ "; it is abstract. Did you mean "
        ^ Markdown_lite.md_codify ("static::" ^ meth_name ^ "()")
        ^ "?" ),
      [(decl_pos, "Declaration is here")],
      quickfixes )

  let classname_abstract_call call_pos meth_name cname decl_pos =
    let cname = Render.strip_ns cname in
    ( Error_code.AbstractCall,
      ( call_pos,
        "Cannot call "
        ^ Markdown_lite.md_codify (cname ^ "::" ^ meth_name ^ "()")
        ^ "; it is abstract" ),
      [(decl_pos, "Declaration is here")],
      [] )

  let static_synthetic_method call_pos meth_name cname decl_pos =
    let cname = Render.strip_ns cname in
    ( Error_code.StaticSyntheticMethod,
      ( call_pos,
        "Cannot call "
        ^ Markdown_lite.md_codify (cname ^ "::" ^ meth_name ^ "()")
        ^ "; "
        ^ Markdown_lite.md_codify meth_name
        ^ " is not defined in "
        ^ Markdown_lite.md_codify cname ),
      [(decl_pos, "Declaration is here")],
      [] )

  let isset_in_strict pos =
    ( Error_code.IssetEmptyInStrict,
      ( pos,
        "`isset` tends to hide errors due to variable typos and so is limited to dynamic checks in "
        ^ "`strict` mode" ),
      [],
      [] )

  let isset_inout_arg pos =
    ( Error_code.InoutInPseudofunction,
      (pos, "`isset` does not allow arguments to be passed by `inout`"),
      [],
      [] )

  let unset_nonidx_in_strict pos msgs =
    ( Error_code.UnsetNonidxInStrict,
      ( pos,
        "In `strict` mode, `unset` is banned except on dynamic, "
        ^ "darray, keyset, or dict indexing" ),
      msgs,
      [] )

  let unpacking_disallowed_builtin_function pos name =
    let name = Render.strip_ns name in
    ( Error_code.UnpackingDisallowed,
      (pos, "Arg unpacking is disallowed for " ^ Markdown_lite.md_codify name),
      [],
      [] )

  let array_get_arity pos1 name pos2 =
    ( Error_code.ArrayGetArity,
      ( pos1,
        "You cannot use this "
        ^ (Render.strip_ns name |> Markdown_lite.md_codify) ),
      [(pos2, "It is missing its type parameters")],
      [] )

  let undefined_field use_pos name shape_type_pos =
    ( Error_code.UndefinedField,
      (use_pos, "The field " ^ Markdown_lite.md_codify name ^ " is undefined"),
      [(shape_type_pos, "Definition is here")],
      [] )

  let array_access code pos1 pos2 ty =
    ( code,
      (pos1, "This is not an object of type `KeyedContainer`, this is " ^ ty),
      (if not Pos_or_decl.(equal pos2 none) then
        [(pos2, "Definition is here")]
      else
        []),
      [] )

  let array_access_read = array_access Error_code.ArrayAccessRead
  let array_access_write = array_access Error_code.ArrayAccessWrite

  let keyset_set pos1 pos2 =
    ( Error_code.KeysetSet,
      (pos1, "Elements in a keyset cannot be assigned, use append instead."),
      (if not Pos_or_decl.(equal pos2 none) then
        [(pos2, "Definition is here")]
      else
        []),
      [] )

  let array_append pos1 pos2 ty =
    ( Error_code.ArrayAppend,
      (pos1, ty ^ " does not allow array append"),
      (if not Pos_or_decl.(equal pos2 none) then
        [(pos2, "Definition is here")]
      else
        []),
      [] )

  let const_mutation pos1 pos2 ty =
    ( Error_code.ConstMutation,
      (pos1, "You cannot mutate this"),
      (if not Pos_or_decl.(equal pos2 none) then
        [(pos2, "This is " ^ ty)]
      else
        []),
      [] )

  let expected_class pos suffix =
    (Error_code.ExpectedClass, (pos, "Was expecting a class" ^ suffix), [], [])

  let unknown_type pos description r =
    let msg = "Was expecting " ^ description ^ " but type is unknown" in
    (Error_code.UnknownType, (pos, msg), r, [])

  let parent_in_trait pos =
    ( Error_code.ParentInTrait,
      ( pos,
        "You can only use `parent::` in traits that specify `require extends SomeClass`"
      ),
      [],
      [] )

  let parent_undefined pos =
    (Error_code.ParentUndefined, (pos, "parent is undefined"), [], [])

  let constructor_no_args pos =
    ( Error_code.ConstructorNoArgs,
      (pos, "This constructor expects no argument"),
      [],
      [] )

  let visibility p msg1 p_vis msg2 =
    (Error_code.Visibility, (p, msg1), [(p_vis, msg2)], [])

  let bad_call pos ty =
    ( Error_code.BadCall,
      (pos, "This call is invalid, this is not a function, it is " ^ ty),
      [],
      [] )

  let extend_final extend_pos decl_pos name =
    let name = Render.strip_ns name in
    ( Error_code.ExtendFinal,
      ( extend_pos,
        "You cannot extend final class " ^ Markdown_lite.md_codify name ),
      [(decl_pos, "Declaration is here")],
      [] )

  let extend_non_abstract_record extend_pos name decl_pos =
    let name = Render.strip_ns name in
    let msg =
      Printf.sprintf
        "Cannot extend record %s because it isn't abstract"
        (Markdown_lite.md_codify name)
    in
    ( Error_code.ExtendFinal,
      (extend_pos, msg),
      [(decl_pos, "Declaration is here")],
      [] )

  let extend_sealed child_pos parent_pos parent_name parent_kind verb =
    let parent_kind =
      match parent_kind with
      | `intf -> "interface"
      | `trait -> "trait"
      | `class_ -> "class"
      | `enum -> "enum"
      | `enum_class -> "enum class"
    and verb =
      match verb with
      | `extend -> "extend"
      | `implement -> "implement"
      | `use -> "use"
    and name = Render.strip_ns parent_name in
    ( Error_code.ExtendSealed,
      ( child_pos,
        "You cannot "
        ^ verb
        ^ " sealed "
        ^ parent_kind
        ^ " "
        ^ Markdown_lite.md_codify name ),
      [(parent_pos, "Declaration is here")],
      [] )

  let sealed_not_subtype parent_pos parent_name child_name child_kind child_pos
      =
    let parent_name = Render.strip_ns parent_name
    and child_name = Render.strip_ns child_name
    and (child_kind, verb) =
      match child_kind with
      | Ast_defs.Cclass _ -> ("Class", "extend")
      | Ast_defs.Cinterface -> ("Interface", "implement")
      | Ast_defs.Ctrait -> ("Trait", "use")
      | Ast_defs.Cenum -> ("Enum", "use")
      | Ast_defs.Cenum_class _ -> ("Enum Class", "extend")
    in

    ( Error_code.SealedNotSubtype,
      ( parent_pos,
        child_kind
        ^ " "
        ^ Markdown_lite.md_codify child_name
        ^ " in sealed whitelist for "
        ^ Markdown_lite.md_codify parent_name
        ^ ", but does not "
        ^ verb
        ^ " "
        ^ Markdown_lite.md_codify parent_name ),
      [(child_pos, "Definition is here")],
      [] )

  let trait_prop_const_class pos x =
    ( Error_code.TraitPropConstClass,
      ( pos,
        "Trait declaration of non-const property "
        ^ Markdown_lite.md_codify x
        ^ " is incompatible with a const class" ),
      [],
      [] )

  let read_before_write (pos, v) =
    ( Error_code.ReadBeforeWrite,
      ( pos,
        Utils.sl
          [
            "Read access to ";
            Markdown_lite.md_codify ("$this->" ^ v);
            " before initialization";
          ] ),
      [],
      [] )

  let implement_abstract pos1 is_final pos2 x kind qfxs =
    let kind =
      match kind with
      | `meth -> "method"
      | `prop -> "property"
      | `const -> "constant"
      | `ty_const -> "type constant"
    in
    let name = "abstract " ^ kind ^ " " ^ Markdown_lite.md_codify x in
    let msg1 =
      if is_final then
        "This class was declared as `final`. It must provide an implementation for the "
        ^ name
      else
        "This class must be declared `abstract`, or provide an implementation for the "
        ^ name
    in
    ( Error_code.ImplementAbstract,
      (pos1, msg1),
      [(pos2, "Declaration is here")],
      qfxs )

  let generic_static pos x =
    ( Error_code.GenericStatic,
      ( pos,
        "This static variable cannot use the type parameter "
        ^ Markdown_lite.md_codify x
        ^ "." ),
      [],
      [] )

  let ellipsis_strict_mode pos require =
    let msg =
      match require with
      | `Param_name ->
        "Variadic function arguments require a name in strict mode, e.g. `...$args`."
      | `Type_and_param_name ->
        "Variadic function arguments require a name and type in strict mode, e.g. `int ...$args`."
    in
    (Error_code.EllipsisStrictMode, (pos, msg), [], [])

  let untyped_lambda_strict_mode pos =
    let msg =
      "Cannot determine types of lambda parameters in strict mode. Please add type hints on parameters."
    in
    (Error_code.UntypedLambdaStrictMode, (pos, msg), [], [])

  let object_string pos1 pos2 =
    ( Error_code.ObjectString,
      (pos1, "You cannot use this object as a string"),
      [(pos2, "This object doesn't implement `__toString`")],
      [] )

  let object_string_deprecated pos =
    ( Error_code.ObjectString,
      ( pos,
        "You cannot use this object as a string\nImplicit conversions of Stringish objects to string are deprecated."
      ),
      [],
      [] )

  let cyclic_typedef def_pos use_pos =
    ( Error_code.CyclicTypedef,
      (def_pos, "Cyclic type definition"),
      [(use_pos, "Cyclic use is here")],
      [] )

  let require_args_reify arg_pos def_pos =
    ( Error_code.RequireArgsReify,
      ( arg_pos,
        "All type arguments must be specified because a type parameter is reified"
      ),
      [(def_pos, "Definition is here")],
      [] )

  let require_generic_explicit arg_pos def_pos def_name =
    ( Error_code.RequireGenericExplicit,
      ( arg_pos,
        "Generic type parameter "
        ^ Markdown_lite.md_codify def_name
        ^ " must be specified explicitly" ),
      [(def_pos, "Definition is here")],
      [] )

  let invalid_reified_argument hint_pos def_name def_pos arg_info =
    let (arg_pos, arg_kind) = List.hd_exn arg_info in
    ( Error_code.InvalidReifiedArgument,
      (hint_pos, "Invalid reified hint"),
      [
        ( arg_pos,
          "This is "
          ^ arg_kind
          ^ ", it cannot be used as a reified type argument" );
        (def_pos, Markdown_lite.md_codify def_name ^ " is reified");
      ],
      [] )

  let invalid_reified_argument_reifiable arg_pos def_name def_pos ty_pos ty_msg
      =
    ( Error_code.InvalidReifiedArgument,
      (arg_pos, "PHP arrays cannot be used as a reified type argument"),
      [
        (ty_pos, String.capitalize ty_msg);
        (def_pos, Markdown_lite.md_codify def_name ^ " is reified");
      ],
      [] )

  let new_class_reified pos class_type suggested_class =
    let suggestion =
      match suggested_class with
      | Some s ->
        let s = Render.strip_ns s in
        sprintf ". Try `new %s` instead." s
      | None -> ""
    in
    ( Error_code.NewClassReified,
      ( pos,
        sprintf
          "Cannot call `new %s` because the current class has reified generics%s"
          class_type
          suggestion ),
      [],
      [] )

  let class_get_reified pos =
    ( Error_code.ClassGetReified,
      (pos, "Cannot access static properties on reified generics"),
      [],
      [] )

  let static_meth_with_class_reified_generic meth_pos generic_pos =
    ( Error_code.StaticMethWithClassReifiedGeneric,
      ( meth_pos,
        "Static methods cannot use generics reified at the class level. Try reifying them at the static method itself."
      ),
      [
        ( Pos_or_decl.of_raw_pos generic_pos,
          "Class-level reified generic used here." );
      ],
      [] )

  let consistent_construct_reified pos =
    ( Error_code.ConsistentConstructReified,
      ( pos,
        "This class or one of its ancestors is annotated with `<<__ConsistentConstruct>>`. It cannot have reified generics."
      ),
      [],
      [] )

  let bad_function_pointer_construction pos =
    ( Error_code.BadFunctionPointerConstruction,
      (pos, "Function pointers must be explicitly named"),
      [],
      [] )

  let reified_generics_not_allowed pos =
    ( Error_code.InvalidReifiedFunctionPointer,
      ( pos,
        "Creating function pointers with reified generics is not currently allowed"
      ),
      [],
      [] )

  let new_without_newable pos name =
    ( Error_code.NewWithoutNewable,
      ( pos,
        Markdown_lite.md_codify name
        ^ " cannot be used with `new` because it does not have the `<<__Newable>>` attribute"
      ),
      [],
      [] )

  let discarded_awaitable pos1 pos2 =
    ( Error_code.DiscardedAwaitable,
      ( pos1,
        "This expression is of type `Awaitable`, but it's "
        ^ "either being discarded or used in a dangerous way before "
        ^ "being awaited" ),
      [(pos2, "This is why I think it is `Awaitable`")],
      [] )

  let elt_type_to_string = function
    | `meth -> "method"
    | `prop -> "property"

  let static_redeclared_as_dynamic
      dyn_position static_position member_name elt_type =
    let dollar =
      match elt_type with
      | `prop -> "$"
      | _ -> ""
    in
    let elt_type = elt_type_to_string elt_type in
    let msg_dynamic =
      "The "
      ^ elt_type
      ^ " "
      ^ Markdown_lite.md_codify (dollar ^ member_name)
      ^ " is declared here as non-static"
    in
    let msg_static =
      "But it conflicts with an inherited static declaration here"
    in
    ( Error_code.StaticDynamic,
      (dyn_position, msg_dynamic),
      [(static_position, msg_static)],
      [] )

  let dynamic_redeclared_as_static
      static_position dyn_position member_name elt_type =
    let dollar =
      match elt_type with
      | `prop -> "$"
      | _ -> ""
    in
    let elt_type = elt_type_to_string elt_type in
    let msg_static =
      "The "
      ^ elt_type
      ^ " "
      ^ Markdown_lite.md_codify (dollar ^ member_name)
      ^ " is declared here as static"
    in
    let msg_dynamic =
      "But it conflicts with an inherited non-static declaration here"
    in
    ( Error_code.StaticDynamic,
      (static_position, msg_static),
      [(dyn_position, msg_dynamic)],
      [] )

  let unknown_object_member pos s elt r =
    let elt = elt_type_to_string elt in
    let msg =
      Printf.sprintf
        "You are trying to access the %s %s on a value whose class is unknown."
        elt
        (Markdown_lite.md_codify s)
    in
    (Error_code.UnknownObjectMember, (pos, msg), r, [])

  let non_class_member pos1 s elt ty pos2 =
    let elt = elt_type_to_string elt in
    let msg =
      Printf.sprintf
        "You are trying to access the static %s %s but this is %s"
        elt
        (Markdown_lite.md_codify s)
        ty
    in
    (Error_code.NonClassMember, (pos1, msg), [(pos2, "Definition is here")], [])

  let null_container p null_witness =
    ( Error_code.NullContainer,
      ( p,
        "You are trying to access an element of this container"
        ^ " but the container could be `null`. " ),
      null_witness,
      [] )

  let option_mixed pos =
    ( Error_code.OptionMixed,
      (pos, "`?mixed` is a redundant typehint - just use `mixed`"),
      [],
      [] )

  let option_null pos =
    ( Error_code.OptionNull,
      (pos, "`?null` is a redundant typehint - just use `null`"),
      [],
      [] )

  let declared_covariant pos1 pos2 emsg =
    ( Error_code.DeclaredCovariant,
      (pos2, "Illegal usage of a covariant type parameter"),
      [
        ( Pos_or_decl.of_raw_pos pos1,
          "This is where the parameter was declared as covariant `+`" );
      ]
      @ List.map emsg ~f:(Message.map ~f:Pos_or_decl.of_raw_pos),
      [] )

  let declared_contravariant pos1 pos2 emsg =
    ( Error_code.DeclaredContravariant,
      (pos2, "Illegal usage of a contravariant type parameter"),
      [
        ( Pos_or_decl.of_raw_pos pos1,
          "This is where the parameter was declared as contravariant `-`" );
      ]
      @ List.map emsg ~f:(Message.map ~f:Pos_or_decl.of_raw_pos),
      [] )

  let static_property_type_generic_param generic_pos class_pos var_type_pos =
    ( Error_code.ClassVarTypeGenericParam,
      ( generic_pos,
        "A generic parameter cannot be used in the type of a static property" ),
      [
        ( var_type_pos,
          "This is where the type of the static property was declared" );
        (class_pos, "This is the class containing the static property");
      ],
      [] )

  let contravariant_this pos class_name tp =
    ( Error_code.ContravariantThis,
      ( pos,
        "The `this` type cannot be used in this "
        ^ "contravariant position because its enclosing class "
        ^ Markdown_lite.md_codify class_name
        ^ " "
        ^ "is final and has a variant type parameter "
        ^ Markdown_lite.md_codify tp ),
      [],
      [] )

  let cyclic_typeconst pos sl =
    let sl =
      List.map sl ~f:(fun s -> Render.strip_ns s |> Markdown_lite.md_codify)
    in
    ( Error_code.CyclicTypeconst,
      (pos, "Cyclic type constant:\n  " ^ String.concat ~sep:" -> " sl),
      [],
      [] )

  let array_get_with_optional_field pos1 name pos2 =
    ( Error_code.ArrayGetWithOptionalField,
      ( pos1,
        Printf.sprintf
          "The field %s may not be present in this shape. Use `Shapes::idx()` instead."
          (Markdown_lite.md_codify name) ),
      [(pos2, "This is where the field was declared as optional.")],
      [] )

  let mutating_const_property pos =
    ( Error_code.AssigningToConst,
      (pos, "Cannot mutate a `__Const` property"),
      [],
      [] )

  let self_const_parent_not pos =
    ( Error_code.SelfConstParentNot,
      (pos, "A `__Const` class may only extend other `__Const` classes"),
      [],
      [] )

  let unexpected_ty_in_tast pos ~actual_ty ~expected_ty =
    ( Error_code.UnexpectedTy,
      ( pos,
        "Unexpected type in TAST: expected "
        ^ Markdown_lite.md_codify expected_ty
        ^ ", got "
        ^ Markdown_lite.md_codify actual_ty ),
      [],
      [] )

  let call_lvalue pos =
    ( Error_code.CallLvalue,
      ( pos,
        "Array updates cannot be applied to function results. Use a local variable instead."
      ),
      [],
      [] )

  let unsafe_cast_await pos =
    ( Error_code.UnsafeCastAwait,
      (pos, "UNSAFE_CAST cannot be used as the operand of an await operation"),
      [],
      [] )

  let internal_compiler_error_msg =
    Printf.sprintf
      "Encountered an internal compiler error while typechecking this. %s %s"
      Error_message_sentinel.remediation_message
      Error_message_sentinel.please_file_a_bug_message

  let invariant_violation pos =
    (Error_code.InvariantViolated, (pos, internal_compiler_error_msg), [], [])

  let exception_occurred pos =
    (Error_code.ExceptionOccurred, (pos, internal_compiler_error_msg), [], [])

  let to_error_ = function
    | Coeffect err -> Coeffect.to_error err
    | Enum err -> Enum.to_error err
    | Expr_tree err -> Expr_tree.to_error err
    | Ifc err -> Ifc.to_error err
    | Modules err -> Modules.to_error err
    | Readonly err -> Readonly.to_error err
    | Record err -> Record.to_error err
    | Shape err -> Shape.to_error err
    | Wellformedness err -> Wellformedness.to_error err
    | Xhp err -> Xhp.to_error err
    | Unify_error { pos; msg_opt; reasons_opt } ->
      unify_error pos msg_opt reasons_opt
    | Generic_unify { pos; msg } -> generic_unify pos msg
    | Exception_occurred { pos; _ } -> exception_occurred pos
    | Invariant_violation { pos; _ } -> invariant_violation pos
    | Internal_error { pos; msg } -> internal_error pos msg
    | Typechecker_timeout { pos; fn_name; seconds } ->
      typechecker_timeout pos fn_name seconds
    | Unresolved_tyvar pos -> unresolved_tyvar pos
    | Using_error { pos; has_await } -> using_error pos has_await
    | Bad_enum_decl pos -> bad_enum_decl pos
    | Bad_conditional_support_dynamic
        { pos; child; parent; ty_name; self_ty_name } ->
      bad_conditional_support_dynamic pos child parent ty_name self_ty_name
    | Bad_decl_override { pos; name; parent_pos; parent_name } ->
      bad_decl_override pos name parent_pos parent_name
    | Explain_where_constraint { pos; decl_pos; in_class } ->
      explain_where_constraint pos decl_pos in_class
    | Explain_constraint pos -> explain_constraint pos
    | Rigid_tvar_escape { pos; what } -> rigid_tvar_escape pos what
    | Invalid_type_hint pos -> invalid_type_hint pos
    | Unsatisfied_req { pos; trait_pos; req_name; req_pos } ->
      unsatisfied_req pos trait_pos req_name req_pos
    | Invalid_echo_argument pos -> invalid_echo_argument pos
    | Index_type_mismatch { pos; is_covariant_container; msg_opt; reasons_opt }
      ->
      index_type_mismatch pos is_covariant_container msg_opt reasons_opt
    | Member_not_found
        { pos; kind; member_name; class_name; class_pos; hint; reason } ->
      member_not_found
        pos
        kind
        member_name
        class_name
        class_pos
        (Lazy.force hint)
        reason
    | Construct_not_instance_method pos -> construct_not_instance_method pos
    | Ambiguous_inheritance { pos; origin; class_name } ->
      ambiguous_inheritance pos origin class_name
    | Expected_tparam { pos; n; decl_pos } -> expected_tparam pos n decl_pos
    | Typeconst_concrete_concrete_override { pos; decl_pos } ->
      typeconst_concrete_concrete_override pos decl_pos
    | Invalid_memoized_param { pos; reason; _ } ->
      invalid_memoized_param pos reason
    | Invalid_arraykey
        { pos; container_pos; container_ty_name; key_pos; key_ty_name; ctxt } ->
      invalid_arraykey
        pos
        container_pos
        (Lazy.force container_ty_name)
        key_pos
        (Lazy.force key_ty_name)
        ctxt
    | Invalid_keyset_value
        { pos; container_pos; container_ty_name; value_pos; value_ty_name } ->
      invalid_keyset_value
        pos
        container_pos
        (Lazy.force container_ty_name)
        value_pos
        (Lazy.force value_ty_name)
    | Invalid_set_value
        { pos; container_pos; container_ty_name; value_pos; value_ty_name } ->
      invalid_set_value
        pos
        container_pos
        (Lazy.force container_ty_name)
        value_pos
        (Lazy.force value_ty_name)
    | HKT_alias_with_implicit_constraints
        {
          pos;
          typedef_name;
          typedef_pos;
          used_class_in_def_pos;
          used_class_in_def_name;
          used_class_tparam_name;
          typedef_tparam_name;
          _;
        } ->
      hkt_alias_with_implicit_constraints
        pos
        typedef_name
        typedef_pos
        used_class_in_def_pos
        used_class_in_def_name
        used_class_tparam_name
        typedef_tparam_name
    | Object_string_deprecated pos -> object_string_deprecated pos
    | Invalid_substring { pos; ty_name } -> invalid_substring pos ty_name
    | Unset_nonidx_in_strict { pos; reason } ->
      unset_nonidx_in_strict pos reason
    | Nullable_cast { pos; ty_pos; ty_name } -> nullable_cast pos ty_pos ty_name
    | Hh_expect { pos; equivalent } -> hh_expect pos equivalent
    | Null_member { pos; ctxt; kind; member_name; reason } ->
      null_member pos ctxt kind member_name reason
    | Typing_too_many_args { pos; decl_pos; actual; expected } ->
      typing_too_many_args pos decl_pos actual expected
    | Typing_too_few_args { pos; decl_pos; actual; expected } ->
      typing_too_few_args pos decl_pos actual expected
    | Non_object_member { pos; ctxt; ty_name; member_name; kind; decl_pos } ->
      non_object_member pos ctxt ty_name member_name kind decl_pos
    | Nullsafe_property_write_context pos -> nullsafe_property_write_context pos
    | Uninstantiable_class { pos; class_name; reason_ty_opt; decl_pos } ->
      uninstantiable_class pos class_name reason_ty_opt decl_pos
    | Abstract_const_usage { pos; name; decl_pos } ->
      abstract_const_usage pos name decl_pos
    | Type_arity_mismatch { pos; decl_pos; actual; expected } ->
      type_arity_mismatch pos decl_pos actual expected
    | Member_not_implemented
        { pos; parent_pos; member_name; decl_pos; quickfixes } ->
      member_not_implemented pos parent_pos member_name decl_pos quickfixes
    | Attribute_too_many_arguments { pos; name; expected } ->
      attribute_too_many_arguments pos name expected
    | Attribute_too_few_arguments { pos; name; expected } ->
      attribute_too_few_arguments pos name expected
    | Attribute_not_exact_number_of_args { pos; name; actual; expected } ->
      attribute_not_exact_number_of_args pos name actual expected
    | Kind_mismatch { pos; decl_pos; tparam_name; expected_kind; actual_kind }
      ->
      kind_mismatch pos decl_pos tparam_name expected_kind actual_kind
    | Trait_parent_construct_inconsistent { pos; decl_pos } ->
      trait_parent_construct_inconsistent pos decl_pos
    | Top_member { pos; ctxt; ty_name; decl_pos; kind; name; is_nullable } ->
      top_member pos ctxt ty_name decl_pos kind name is_nullable
    | Unresolved_tyvar_projection { pos; proj_pos; tconst_name } ->
      unresolved_tyvar_projection pos proj_pos tconst_name
    | Cyclic_class_constant { pos; class_name; const_name } ->
      cyclic_class_constant pos class_name const_name
    | Inout_annotation_missing { pos; decl_pos } ->
      inout_annotation_missing pos decl_pos
    | Inout_annotation_unexpected { pos; decl_pos; param_is_variadic; qfx_pos }
      ->
      inout_annotation_unexpected pos decl_pos param_is_variadic qfx_pos
    | Inout_argument_bad_type { pos; reasons } ->
      inout_argument_bad_type pos reasons
    | Invalid_meth_caller_calling_convention { pos; decl_pos; convention } ->
      invalid_meth_caller_calling_convention pos decl_pos convention
    | Invalid_new_disposable pos -> invalid_new_disposable pos
    | Invalid_return_disposable pos -> invalid_return_disposable pos
    | Invalid_disposable_hint { pos; class_name } ->
      invalid_disposable_hint pos class_name
    | Invalid_disposable_return_hint { pos; class_name } ->
      invalid_disposable_return_hint pos class_name
    | Ambiguous_lambda { pos; uses } -> ambiguous_lambda pos uses
    | Wrong_extend_kind
        { pos; kind; name; parent_pos; parent_kind; parent_name } ->
      wrong_extend_kind pos kind name parent_pos parent_kind parent_name
    | Smember_not_found { pos; kind; member_name; class_name; class_pos; hint }
      ->
      smember_not_found pos kind member_name class_name class_pos hint
    | Cyclic_class_def { pos; stack } -> cyclic_class_def pos stack
    | Cyclic_record_def { pos; names } -> cyclic_record_def pos names
    | Trait_reuse_with_final_method { pos; trait_name; parent_cls_name; trace }
      ->
      trait_reuse_with_final_method pos trait_name parent_cls_name trace
    | Trait_reuse { pos; class_name; trait_name; parent_pos; parent_name } ->
      trait_reuse pos class_name trait_name parent_pos parent_name
    | Trait_reuse_inside_class { pos; class_name; trait_name; occurrences } ->
      trait_reuse_inside_class pos class_name trait_name occurrences
    | Invalid_is_as_expression_hint { pos; op; reasons } ->
      invalid_is_as_expression_hint pos op reasons
    | Invalid_enforceable_type { pos; ty_info; tp_pos; tp_name; kind } ->
      invalid_enforceable_type pos ty_info kind tp_pos tp_name
    | Reifiable_attr { pos; ty_info; attr_pos; kind } ->
      reifiable_attr attr_pos kind pos ty_info
    | Invalid_newable_type_argument { pos; tp_pos; tp_name } ->
      invalid_newable_type_argument pos tp_pos tp_name
    | Invalid_newable_typaram_constraints { pos; tp_name; constraints } ->
      invalid_newable_type_param_constraints (pos, tp_name) constraints
    | Override_per_trait { pos; class_name; meth_name; trait_name; meth_pos } ->
      override_per_trait (pos, class_name) meth_name trait_name meth_pos
    | Generic_at_runtime { pos; prefix } -> generic_at_runtime pos prefix
    | Generics_not_allowed pos -> generics_not_allowed pos
    | Trivial_strict_eq { pos; result; left; right; left_trail; right_trail } ->
      trivial_strict_eq
        pos
        result
        (Lazy.force left)
        (Lazy.force right)
        left_trail
        right_trail
    | Trivial_strict_not_nullable_compare_null { pos; result; ty_reason_msg } ->
      trivial_strict_not_nullable_compare_null pos result
      @@ Lazy.force ty_reason_msg
    | Eq_incompatible_types { pos; left; right } ->
      eq_incompatible_types pos (Lazy.force left) (Lazy.force right)
    | Comparison_invalid_types { pos; left; right } ->
      comparison_invalid_types pos (Lazy.force left) (Lazy.force right)
    | Strict_eq_value_incompatible_types { pos; left; right } ->
      strict_eq_value_incompatible_types
        pos
        (Lazy.force left)
        (Lazy.force right)
    | Attribute_param_type { pos; x } -> attribute_param_type pos x
    | Deprecated_use { pos; decl_pos_opt; msg } ->
      deprecated_use pos ~pos_def:decl_pos_opt msg
    | Cannot_declare_constant { pos; kind; class_pos; class_name } ->
      cannot_declare_constant kind pos (class_pos, class_name)
    | Local_variable_modified_and_used { pos; pos_useds } ->
      local_variable_modified_and_used pos pos_useds
    | Local_variable_modified_twice { pos; pos_modifieds } ->
      local_variable_modified_twice pos pos_modifieds
    | Assign_during_case pos -> assign_during_case pos
    | Invalid_classname pos -> invalid_classname pos
    | Illegal_type_structure pos -> illegal_type_structure pos
    | Illegal_typeconst_direct_access pos -> illegal_typeconst_direct_access pos
    | Wrong_expression_kind_attribute
        {
          pos;
          attr_name;
          expr_kind;
          attr_class_pos;
          attr_class_name;
          intf_name;
        } ->
      wrong_expression_kind_attribute
        expr_kind
        pos
        attr_name
        attr_class_pos
        attr_class_name
        intf_name
    | Wrong_expression_kind_builtin_attribute { pos; attr_name; expr_kind } ->
      wrong_expression_kind_builtin_attribute expr_kind pos attr_name
    | Ambiguous_object_access
        { pos; name; self_pos; vis; subclass_pos; class_self; class_subclass }
      ->
      ambiguous_object_access
        pos
        name
        self_pos
        vis
        subclass_pos
        class_self
        class_subclass
    | Lateinit_with_default pos -> lateinit_with_default pos
    | Unserializable_type { pos; message } -> unserializable_type pos message
    | Invalid_arraykey_constraint { pos; ty_name } ->
      invalid_arraykey_constraint pos @@ Lazy.force ty_name
    | Redundant_covariant { pos; msg; suggest } ->
      redundant_covariant pos msg suggest
    | Meth_caller_trait { pos; trait_name } -> meth_caller_trait pos trait_name
    | Duplicate_interface { pos; name; others } ->
      duplicate_interface pos name others
    | Tparam_non_shadowing_reuse { pos; tparam_name } ->
      tparam_non_shadowing_reuse pos tparam_name
    | Reified_function_reference pos -> reified_function_reference pos
    | Class_meth_abstract_call { pos; class_name; meth_name; decl_pos } ->
      class_meth_abstract_call class_name meth_name pos decl_pos
    | Reinheriting_classish_const
        {
          pos;
          classish_name;
          src_pos;
          src_classish_name;
          existing_const_origin;
          const_name;
        } ->
      reinheriting_classish_const
        pos
        classish_name
        src_pos
        src_classish_name
        existing_const_origin
        const_name
    | Redeclaring_classish_const
        {
          pos;
          classish_name;
          redeclaration_pos;
          existing_const_origin;
          const_name;
        } ->
      redeclaring_classish_const
        pos
        classish_name
        redeclaration_pos
        existing_const_origin
        const_name
    | Abstract_function_pointer { pos; class_name; meth_name; decl_pos } ->
      abstract_function_pointer class_name meth_name pos decl_pos
    | Unnecessary_attribute { pos; attr; reason; suggestion } ->
      unnecessary_attribute pos ~attr ~reason ~suggestion
    | Inherited_class_member_with_different_case
        {
          pos;
          member_type;
          name;
          name_prev;
          child_class;
          prev_class;
          prev_class_pos;
        } ->
      inherited_class_member_with_different_case
        member_type
        name
        name_prev
        pos
        child_class
        prev_class
        prev_class_pos
    | Multiple_inherited_class_member_with_different_case
        {
          pos;
          child_class_name;
          member_type;
          class1_name;
          class1_pos;
          name1;
          class2_name;
          class2_pos;
          name2;
        } ->
      multiple_inherited_class_member_with_different_case
        ~member_type
        ~name1
        ~name2
        ~class1:class1_name
        ~class2:class2_name
        ~child_class:child_class_name
        ~child_p:pos
        ~p1:class1_pos
        ~p2:class2_pos
    | Parent_support_dynamic_type
        {
          pos;
          child_name;
          child_kind;
          parent_name;
          parent_kind;
          child_support_dyn;
        } ->
      parent_support_dynamic_type
        pos
        (child_name, child_kind)
        (parent_name, parent_kind)
        child_support_dyn
    | Property_is_not_enforceable
        { pos; prop_name; class_name; prop_pos; prop_type } ->
      property_is_not_enforceable pos prop_name class_name (prop_pos, prop_type)
    | Property_is_not_dynamic
        { pos; prop_name; class_name; prop_pos; prop_type } ->
      property_is_not_dynamic pos prop_name class_name (prop_pos, prop_type)
    | Private_property_is_not_enforceable
        { pos; prop_name; class_name; prop_pos; prop_type } ->
      private_property_is_not_enforceable
        pos
        prop_name
        class_name
        (prop_pos, prop_type)
    | Private_property_is_not_dynamic
        { pos; prop_name; class_name; prop_pos; prop_type } ->
      private_property_is_not_dynamic
        pos
        prop_name
        class_name
        (prop_pos, prop_type)
    | Immutable_local pos -> immutable_local pos
    | Nonsense_member_selection { pos; kind } ->
      nonsense_member_selection pos kind
    | Consider_meth_caller { pos; class_name; meth_name } ->
      consider_meth_caller pos class_name meth_name
    | Method_import_via_diamond
        { pos; class_name; method_pos; method_name; trace1; trace2 } ->
      method_import_via_diamond
        pos
        class_name
        method_pos
        method_name
        trace1
        trace2
    | Generic_property_import_via_diamond
        { pos; class_name; property_pos; property_name; trace1; trace2 } ->
      generic_property_import_via_diamond
        pos
        class_name
        property_pos
        property_name
        trace1
        trace2
    | Unification_cycle { pos; ty_name } ->
      unification_cycle pos @@ Lazy.force ty_name
    | Method_variance pos -> method_variance pos
    | Explain_tconst_where_constraint { pos; decl_pos; msgs } ->
      explain_tconst_where_constraint pos decl_pos msgs
    | Format_string
        { pos; snippet; fmt_string; class_pos; fn_name; class_suggest } ->
      format_string pos snippet fmt_string class_pos fn_name class_suggest
    | Expected_literal_format_string pos -> expected_literal_format_string pos
    | Re_prefixed_non_string { pos; reason } ->
      re_prefixed_non_string pos reason
    | Bad_regex_pattern { pos; reason } -> bad_regex_pattern pos reason
    | Generic_array_strict pos -> generic_array_strict pos
    | Option_return_only_typehint { pos; kind } ->
      option_return_only_typehint pos kind
    | Redeclaring_missing_method { pos; trait_method } ->
      redeclaring_missing_method pos trait_method
    | Expecting_type_hint pos -> expecting_type_hint pos
    | Expecting_type_hint_variadic pos -> expecting_type_hint_variadic pos
    | Expecting_return_type_hint pos -> expecting_return_type_hint pos
    | Duplicate_using_var pos -> duplicate_using_var pos
    | Illegal_disposable { pos; verb } -> illegal_disposable pos verb
    | Escaping_disposable pos -> escaping_disposable pos
    | Escaping_disposable_param pos -> escaping_disposable_parameter pos
    | Escaping_this pos -> escaping_this pos
    | Must_extend_disposable pos -> must_extend_disposable pos
    | Field_kinds { pos; decl_pos } -> field_kinds pos decl_pos
    | Unbound_name { pos; name } -> unbound_name_typing pos name
    | Previous_default pos -> previous_default pos
    | Return_in_void { pos; decl_pos } -> return_in_void pos decl_pos
    | This_var_outside_class pos -> this_var_outside_class pos
    | Unbound_global pos -> unbound_global pos
    | Private_inst_meth { pos; decl_pos } -> private_inst_meth pos decl_pos
    | Protected_inst_meth { pos; decl_pos } -> protected_inst_meth pos decl_pos
    | Private_meth_caller { pos; decl_pos } -> private_meth_caller pos decl_pos
    | Protected_meth_caller { pos; decl_pos } ->
      protected_meth_caller pos decl_pos
    | Private_class_meth { pos; decl_pos } -> private_class_meth pos decl_pos
    | Protected_class_meth { pos; decl_pos } ->
      protected_class_meth pos decl_pos
    | Array_cast pos -> array_cast pos
    | String_cast { pos; ty_name } -> string_cast pos @@ Lazy.force ty_name
    | Static_outside_class pos -> static_outside_class pos
    | Self_outside_class pos -> self_outside_class pos
    | New_inconsistent_construct { pos; class_pos; class_name; kind } ->
      new_inconsistent_construct pos (class_pos, class_name) kind
    | Undefined_parent pos -> undefined_parent pos
    | Parent_outside_class pos -> parent_outside_class pos
    | Parent_abstract_call { pos; meth_name; decl_pos } ->
      parent_abstract_call pos meth_name decl_pos
    | Self_abstract_call { pos; self_pos; meth_name; decl_pos } ->
      self_abstract_call pos meth_name self_pos decl_pos
    | Classname_abstract_call { pos; meth_name; class_name; decl_pos } ->
      classname_abstract_call pos meth_name class_name decl_pos
    | Static_synthetic_method { pos; meth_name; class_name; decl_pos } ->
      static_synthetic_method pos meth_name class_name decl_pos
    | Isset_in_strict pos -> isset_in_strict pos
    | Isset_inout_arg pos -> isset_inout_arg pos
    | Unpacking_disallowed_builtin_function { pos; fn_name } ->
      unpacking_disallowed_builtin_function pos fn_name
    | Array_get_arity { pos; name; decl_pos } ->
      array_get_arity pos name decl_pos
    | Undefined_field { pos; name; decl_pos } ->
      undefined_field pos name decl_pos
    | Array_access { pos; ctxt = `read; ty_name; decl_pos } ->
      array_access_read pos decl_pos @@ Lazy.force ty_name
    | Array_access { pos; ctxt = `write; ty_name; decl_pos } ->
      array_access_write pos decl_pos @@ Lazy.force ty_name
    | Keyset_set { pos; decl_pos } -> keyset_set pos decl_pos
    | Array_append { pos; ty_name; decl_pos } ->
      array_append pos decl_pos @@ Lazy.force ty_name
    | Const_mutation { pos; ty_name; decl_pos } ->
      const_mutation pos decl_pos @@ Lazy.force ty_name
    | Expected_class { pos; suffix } ->
      expected_class pos Option.(value_map ~default:"" ~f:Lazy.force suffix)
    | Unknown_type { pos; expected; reason } -> unknown_type pos expected reason
    | Parent_in_trait pos -> parent_in_trait pos
    | Parent_undefined pos -> parent_undefined pos
    | Constructor_no_args pos -> constructor_no_args pos
    | Visibility { pos; msg; decl_pos; reason_msg } ->
      visibility pos msg decl_pos reason_msg
    | Bad_call { pos; ty_name } -> bad_call pos @@ Lazy.force ty_name
    | Extend_final { pos; name; decl_pos } -> extend_final pos decl_pos name
    | Extend_non_abstract_record { pos; name; decl_pos } ->
      extend_non_abstract_record pos name decl_pos
    | Extend_sealed { pos; parent_pos; parent_name; parent_kind; verb } ->
      extend_sealed pos parent_pos parent_name parent_kind verb
    | Sealed_not_subtype { pos; name; child_kind; child_pos; child_name } ->
      sealed_not_subtype pos name child_name child_kind child_pos
    | Trait_prop_const_class { pos; name } -> trait_prop_const_class pos name
    | Read_before_write { pos; member_name } ->
      read_before_write (pos, member_name)
    | Implement_abstract { pos; is_final; decl_pos; name; kind; quickfixes } ->
      implement_abstract pos is_final decl_pos name kind quickfixes
    | Generic_static { pos; typaram_name } -> generic_static pos typaram_name
    | Ellipsis_strict_mode { pos; require } -> ellipsis_strict_mode pos require
    | Untyped_lambda_strict_mode pos -> untyped_lambda_strict_mode pos
    | Object_string { pos; decl_pos } -> object_string pos decl_pos
    | Cyclic_typedef { pos; decl_pos } -> cyclic_typedef pos decl_pos
    | Require_args_reify { pos; decl_pos } -> require_args_reify pos decl_pos
    | Require_generic_explicit { pos; param_name; decl_pos } ->
      require_generic_explicit pos decl_pos param_name
    | Invalid_reified_arg { pos; param_name; decl_pos; arg_info } ->
      invalid_reified_argument pos param_name decl_pos arg_info
    | Invalid_reified_arg_reifiable
        { pos; param_name : string; decl_pos; ty_pos; ty_msg } ->
      invalid_reified_argument_reifiable pos param_name decl_pos ty_pos
      @@ Lazy.force ty_msg
    | New_class_reified { pos; class_kind; suggested_class_name } ->
      new_class_reified pos class_kind suggested_class_name
    | Class_get_reified pos -> class_get_reified pos
    | Static_meth_with_class_reified_generic { pos; generic_pos } ->
      static_meth_with_class_reified_generic pos generic_pos
    | Consistent_construct_reified pos -> consistent_construct_reified pos
    | Bad_fn_ptr_construction pos -> bad_function_pointer_construction pos
    | Reified_generics_not_allowed pos -> reified_generics_not_allowed pos
    | New_without_newable { pos; name } -> new_without_newable pos name
    | Discarded_awaitable { pos; decl_pos } -> discarded_awaitable pos decl_pos
    | Static_redeclared_as_dynamic { pos; static_pos; member_name; elt } ->
      static_redeclared_as_dynamic pos static_pos member_name elt
    | Dynamic_redeclared_as_static { pos; dyn_pos; member_name; elt } ->
      dynamic_redeclared_as_static pos dyn_pos member_name elt
    | Unknown_object_member { pos; member_name; elt; reason } ->
      unknown_object_member pos member_name elt reason
    | Non_class_member { pos; member_name; elt; ty_name; decl_pos } ->
      non_class_member pos member_name elt (Lazy.force ty_name) decl_pos
    | Null_container { pos; null_witness } -> null_container pos null_witness
    | Option_mixed pos -> option_mixed pos
    | Option_null pos -> option_null pos
    | Declared_covariant { pos; param_pos; msgs } ->
      declared_covariant param_pos pos msgs
    | Declared_contravariant { pos; param_pos; msgs } ->
      declared_contravariant pos param_pos msgs
    | Static_prop_type_generic_param { pos; var_ty_pos; class_pos } ->
      static_property_type_generic_param pos class_pos var_ty_pos
    | Contravariant_this { pos; class_name; typaram_name } ->
      contravariant_this pos class_name typaram_name
    | Cyclic_typeconst { pos; tyconst_names } ->
      cyclic_typeconst pos tyconst_names
    | Array_get_with_optional_field { pos; field_name; decl_pos } ->
      array_get_with_optional_field pos field_name decl_pos
    | Mutating_const_property pos -> mutating_const_property pos
    | Self_const_parent_not pos -> self_const_parent_not pos
    | Unexpected_ty_in_tast { pos; expected_ty; actual_ty } ->
      unexpected_ty_in_tast
        pos
        ~expected_ty:(Lazy.force expected_ty)
        ~actual_ty:(Lazy.force actual_ty)
    | Call_lvalue pos -> call_lvalue pos
    | Unsafe_cast_await pos -> unsafe_cast_await pos

  let to_error = function
    | Invariant_violation { report_to_user = false; _ } -> None
    | err -> Some (to_error_ err)

  let code err =
    let (code, _, _, _) = to_error_ err in
    code

  let to_user_error t =
    Option.map ~f:(fun (code, claim, reasons, quickfixes) ->
        User_error.make (Error_code.to_enum code) claim reasons ~quickfixes)
    @@ to_error t
end

module rec Error : sig
  type t

  val iter :
    t -> on_prim:(Primary.t -> unit) -> on_snd:(Secondary.t -> unit) -> unit

  val eval : t -> current_span:Pos.t -> error option

  val to_user_error :
    t -> current_span:Pos.t -> (Pos.t, Pos_or_decl.t) User_error.t option

  val primary : Primary.t -> t
  val coeffect : Primary.Coeffect.t -> t
  val enum : Primary.Enum.t -> t
  val expr_tree : Primary.Expr_tree.t -> t
  val ifc : Primary.Ifc.t -> t
  val modules : Primary.Modules.t -> t
  val readonly : Primary.Readonly.t -> t
  val record : Primary.Record.t -> t
  val shape : Primary.Shape.t -> t
  val wellformedness : Primary.Wellformedness.t -> t
  val xhp : Primary.Xhp.t -> t
  val apply_reasons : Secondary.t -> on_error:Reasons_callback.t -> t
  val apply : t -> on_error:Callback.t -> t
  val assert_in_current_decl : Secondary.t -> ctx:Pos_or_decl.ctx -> t
end = struct
  type t =
    | Primary of Primary.t
    | Apply of Callback.t * t
    | Apply_reasons of Reasons_callback.t * Secondary.t
    | Assert_in_current_decl of Secondary.t * Pos_or_decl.ctx

  let iter t ~on_prim ~on_snd =
    let rec aux = function
      | Primary prim -> on_prim prim
      | Apply (cb, t) ->
        aux t;
        Callback.iter cb ~on_prim
      | Apply_reasons (cb, snd_err) ->
        Secondary.iter snd_err ~on_prim ~on_snd;
        Reasons_callback.iter cb ~on_prim ~on_snd
      | Assert_in_current_decl (snd_err, _ctx) ->
        Secondary.iter snd_err ~on_prim ~on_snd
    in
    aux t

  (* -- Evaluation ------------------------------------------------------------ *)

  let eval t ~current_span =
    let rec aux ~k = function
      | Primary base -> k @@ Primary.to_error base
      | Apply_reasons (cb, err) ->
        k
        @@ Option.bind
             (Secondary.eval err ~current_span)
             ~f:(fun (code, reasons) ->
               Reasons_callback.apply_help cb ~code ~reasons ~current_span)
      | Apply (cb, err) ->
        aux err ~k:(function
            | Some (code, claim, reasons, quickfixes) ->
              k @@ Callback.apply cb ~code ~claim ~reasons ~quickfixes
            | _ -> k None)
      | Assert_in_current_decl (snd_err, ctx) ->
        Option.bind ~f:(Common.eval_assert ctx current_span)
        @@ Secondary.eval snd_err ~current_span
    in

    aux ~k:Fn.id t

  let make_error (code, claim, reasons, quickfixes) =
    User_error.make (Error_code.to_enum code) claim reasons ~quickfixes

  let to_user_error t ~current_span =
    Option.map ~f:make_error @@ eval t ~current_span

  (* -- Constructors ---------------------------------------------------------- *)
  let primary prim_err = Primary prim_err
  let coeffect err = primary @@ Primary.Coeffect err
  let enum err = primary @@ Primary.Enum err
  let expr_tree err = primary @@ Primary.Expr_tree err
  let ifc err = primary @@ Primary.Ifc err
  let modules err = primary @@ Primary.Modules err
  let readonly err = primary @@ Primary.Readonly err
  let record err = primary @@ Primary.Record err
  let shape err = primary @@ Primary.Shape err
  let wellformedness err = primary @@ Primary.Wellformedness err
  let xhp err = primary @@ Primary.Xhp err
  let apply_reasons t ~on_error = Apply_reasons (on_error, t)
  let apply t ~on_error = Apply (on_error, t)
  let assert_in_current_decl snd ~ctx = Assert_in_current_decl (snd, ctx)
end

and Secondary : sig
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
        hint: ([ `instance | `static ] * Pos_or_decl.t * string) option;
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
        reasons: Pos_or_decl.t Message.t list;
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
        reason_sub: Pos_or_decl.t Message.t list;
        reason_super: Pos_or_decl.t Message.t list;
      }
    | Not_sub_dynamic of {
        pos: Pos_or_decl.t;
        ty_name: string Lazy.t;
        dynamic_part: Pos_or_decl.t Message.t list;
      }
    | Subtyping_error of Pos_or_decl.t Message.t list
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

  val iter :
    t -> on_prim:(Primary.t -> unit) -> on_snd:(Secondary.t -> unit) -> unit

  val eval :
    t ->
    current_span:Pos.t ->
    (Error_code.t * Pos_or_decl.t Message.t list) option
end = struct
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
        hint: ([ `instance | `static ] * Pos_or_decl.t * string) option;
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
        reasons: Pos_or_decl.t Message.t list;
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
        reason_sub: Pos_or_decl.t Message.t list;
        reason_super: Pos_or_decl.t Message.t list;
      }
    | Not_sub_dynamic of {
        pos: Pos_or_decl.t;
        ty_name: string Lazy.t;
        dynamic_part: Pos_or_decl.t Message.t list;
      }
    | Subtyping_error of Pos_or_decl.t Message.t list
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

  let iter t ~on_prim ~on_snd =
    match t with
    | Of_error err -> Error.iter ~on_prim ~on_snd err
    | snd_err -> on_snd snd_err

  (* -- Evaluation ---------------------------------------------------------- *)

  let fun_too_many_args pos decl_pos actual expected =
    let reasons =
      [
        ( pos,
          Printf.sprintf
            "Too many mandatory arguments (expected %d but got %d)"
            expected
            actual );
        (decl_pos, "Because of this definition");
      ]
    in
    (Error_code.FunTooManyArgs, reasons)

  let fun_too_few_args pos decl_pos actual expected =
    let reasons =
      [
        ( pos,
          Printf.sprintf
            "Too few arguments (required %d but got %d)"
            expected
            actual );
        (decl_pos, "Because of this definition");
      ]
    in
    (Error_code.FunTooFewArgs, reasons)

  let fun_unexpected_nonvariadic pos decl_pos =
    let reasons =
      [
        (pos, "Should have a variadic argument");
        (decl_pos, "Because of this definition");
      ]
    in
    (Error_code.FunUnexpectedNonvariadic, reasons)

  let fun_variadicity_hh_vs_php56 pos decl_pos =
    let reasons =
      [
        (pos, "Variadic arguments: `...`-style is not a subtype of `...$args`");
        (decl_pos, "Because of this definition");
      ]
    in
    (Error_code.FunVariadicityHhVsPhp56, reasons)

  let type_arity_mismatch pos actual decl_pos expected =
    let reasons =
      [
        (pos, "This type has " ^ string_of_int actual ^ " arguments");
        (decl_pos, "This one has " ^ string_of_int expected);
      ]
    in
    (Error_code.TypeArityMismatch, reasons)

  let violated_constraint cstrs reasons =
    let pos_msg = "This type constraint is violated" in
    let f (p_cstr, (p_tparam, tparam)) =
      [
        ( p_tparam,
          Printf.sprintf
            "%s is a constrained type parameter"
            (Markdown_lite.md_codify tparam) );
        (p_cstr, pos_msg);
      ]
    in
    let msgs = List.concat_map ~f cstrs in
    (Error_code.TypeConstraintViolation, msgs @ reasons)

  let concrete_const_interface_override pos parent_pos name parent_origin =
    let reasons =
      [
        ( parent_pos,
          "You could make "
          ^ Markdown_lite.md_codify name
          ^ " abstract in "
          ^ (Markdown_lite.md_codify @@ Render.strip_ns parent_origin)
          ^ "." );
      ]
    and claim =
      ( pos,
        "Non-abstract constants defined in an interface cannot be overridden when implementing or extending that interface."
      )
    in
    (Error_code.ConcreteConstInterfaceOverride, claim :: reasons)

  let interface_or_trait_const_multiple_defs
      pos origin parent_pos parent_origin name =
    let parent_origin = Render.strip_ns parent_origin
    and child_origin = Render.strip_ns origin in
    let reasons =
      [
        ( pos,
          "Non-abstract constants defined in an interface or trait cannot conflict with other inherited constants."
        );
        ( parent_pos,
          Markdown_lite.md_codify name
          ^ " inherited from "
          ^ Markdown_lite.md_codify parent_origin );
        ( pos,
          "conflicts with constant "
          ^ Markdown_lite.md_codify name
          ^ " inherited from "
          ^ Markdown_lite.md_codify child_origin
          ^ "." );
      ]
    in
    (Error_code.ConcreteConstInterfaceOverride, reasons)

  let interface_typeconst_multiple_defs
      pos parent_pos name origin parent_origin is_abstract =
    let parent_origin = Render.strip_ns parent_origin
    and child_origin = Render.strip_ns origin
    and child_pos = pos
    and child =
      if is_abstract then
        "abstract type constant with default value"
      else
        "concrete type constant"
    in
    let reasons =
      [
        ( pos,
          "Concrete and abstract type constants with default values in an interface cannot conflict with inherited"
          ^ " concrete or abstract type constants with default values." );
        ( parent_pos,
          Markdown_lite.md_codify name
          ^ " inherited from "
          ^ Markdown_lite.md_codify parent_origin );
        ( child_pos,
          "conflicts with "
          ^ Markdown_lite.md_codify child
          ^ " "
          ^ Markdown_lite.md_codify name
          ^ " inherited from "
          ^ Markdown_lite.md_codify child_origin
          ^ "." );
      ]
    in
    (Error_code.ConcreteConstInterfaceOverride, reasons)

  let missing_field pos name decl_pos =
    let reasons =
      [
        (pos, "The field " ^ Markdown_lite.md_codify name ^ " is missing");
        (decl_pos, "The field " ^ Markdown_lite.md_codify name ^ " is defined");
      ]
    in
    (Error_code.MissingField, reasons)

  let shape_fields_unknown pos decl_pos =
    let reasons =
      [
        ( pos,
          "This shape type allows unknown fields, and so it may contain fields other than those explicitly declared in its declaration."
        );
        ( decl_pos,
          "It is incompatible with a shape that does not allow unknown fields."
        );
      ]
    in
    (Error_code.ShapeFieldsUnknown, reasons)

  let abstract_tconst_not_allowed pos decl_pos tconst_name =
    let reasons =
      [
        (pos, "An abstract type constant is not allowed in this position.");
        ( decl_pos,
          Printf.sprintf
            "%s is abstract here."
            (Markdown_lite.md_codify tconst_name) );
      ]
    in
    (Error_code.AbstractTconstNotAllowed, reasons)

  let invalid_destructure pos decl_pos ty_name =
    let reasons =
      [
        ( pos,
          "This expression cannot be destructured with a `list(...)` expression"
        );
        (decl_pos, "This is " ^ Markdown_lite.md_codify @@ Lazy.force ty_name);
      ]
    in
    (Error_code.InvalidDestructure, reasons)

  let unpack_array_required_argument pos decl_pos =
    let reasons =
      [
        ( pos,
          "An array cannot be unpacked into the required arguments of a function"
        );
        (decl_pos, "Definition is here");
      ]
    in
    (Error_code.SplatArrayRequired, reasons)

  let unpack_array_variadic_argument pos decl_pos =
    let reasons =
      [
        ( pos,
          "A function that receives an unpacked array as an argument must have a variadic parameter to accept the elements of the array"
        );
        (decl_pos, "Definition is here");
      ]
    in
    (Error_code.SplatArrayRequired, reasons)

  let overriding_prop_const_mismatch pos is_const parent_pos =
    let (msg, reason_msg) =
      if is_const then
        ("This property is `__Const`", "This property is not `__Const`")
      else
        ("This property is not `__Const`", "This property is `__Const`")
    in
    let reasons = [(pos, msg); (parent_pos, reason_msg)] in
    (Error_code.OverridingPropConstMismatch, reasons)

  let visibility_extends pos vis parent_pos parent_vis =
    let reasons =
      [
        (pos, "This member visibility is: " ^ Markdown_lite.md_codify vis);
        (parent_pos, Markdown_lite.md_codify parent_vis ^ " was expected");
      ]
    in
    (Error_code.VisibilityExtends, reasons)

  let visibility_override_internal pos module_name parent_module parent_pos =
    let msg =
      match module_name with
      | None ->
        Printf.sprintf
          "Cannot override this member outside module `%s`"
          parent_module
      | Some m -> Printf.sprintf "Cannot override this member in module `%s`" m
    in
    let reasons =
      [
        (pos, msg);
        ( parent_pos,
          Printf.sprintf "This member is internal to module `%s`" parent_module
        );
      ]
    in
    (Error_code.ModuleError, reasons)

  let missing_constructor pos =
    let reasons = [(pos, "The constructor is not implemented")] in
    (Error_code.MissingConstructor, reasons)

  let accept_disposable_invariant pos decl_pos =
    let reasons =
      [
        (pos, "This parameter is marked `<<__AcceptDisposable>>`");
        (decl_pos, "This parameter is not marked `<<__AcceptDisposable>>`");
      ]
    in
    (Error_code.AcceptDisposableInvariant, reasons)

  let ifc_external_contravariant pos_sub pos_super =
    let reasons =
      [
        ( pos_super,
          "Parameters with `<<__External>>` must be overridden by other parameters with <<__External>>. This parameter is marked `<<__External>>`"
        );
        (pos_sub, "But this parameter is not marked `<<__External>>`");
      ]
    in
    (Error_code.IFCExternalContravariant, reasons)

  let required_field_is_optional pos name decl_pos =
    let reasons =
      [
        (pos, "The field " ^ Markdown_lite.md_codify name ^ " is **optional**");
        ( decl_pos,
          "The field "
          ^ Markdown_lite.md_codify name
          ^ " is defined as **required**" );
      ]
    in
    (Error_code.RequiredFieldIsOptional, reasons)

  let return_disposable_mismatch pos_sub is_marked_return_disposable pos_super =
    let (msg, reason_msg) =
      if is_marked_return_disposable then
        ( "This is marked `<<__ReturnDisposable>>`.",
          "This is not marked `<<__ReturnDisposable>>`." )
      else
        ( "This is not marked `<<__ReturnDisposable>>`.",
          "This is marked `<<__ReturnDisposable>>`." )
    in
    let reasons = [(pos_super, msg); (pos_sub, reason_msg)] in
    (Error_code.ReturnDisposableMismatch, reasons)

  let ifc_policy_mismatch pos policy pos_super policy_super =
    let reasons =
      [
        ( pos,
          "IFC policies must be invariant with respect to inheritance. This method is policied with "
          ^ policy );
        ( pos_super,
          "This is incompatible with its inherited policy, which is "
          ^ policy_super );
      ]
    in
    (Error_code.IFCPolicyMismatch, reasons)

  let override_final pos parent_pos =
    let reasons =
      [
        (pos, "You cannot override this method");
        (parent_pos, "It was declared as final");
      ]
    in
    (Error_code.OverrideFinal, reasons)

  let override_lsb pos member_name parent_pos =
    let reasons =
      [
        ( pos,
          "Member "
          ^ Markdown_lite.md_codify member_name
          ^ " may not override `__LSB` member of parent" );
        (parent_pos, "This is being overridden");
      ]
    in
    (Error_code.OverrideLSB, reasons)

  let multiple_concrete_defs pos origin name parent_pos parent_origin class_name
      =
    let child_origin = Markdown_lite.md_codify @@ Render.strip_ns origin
    and parent_origin = Markdown_lite.md_codify @@ Render.strip_ns parent_origin
    and class_ = Markdown_lite.md_codify @@ Render.strip_ns class_name
    and name = Markdown_lite.md_codify name in
    let reasons =
      [
        ( pos,
          child_origin
          ^ " and "
          ^ parent_origin
          ^ " both declare ambiguous implementations of "
          ^ name
          ^ "." );
        (pos, child_origin ^ "'s definition is here.");
        (parent_pos, parent_origin ^ "'s definition is here.");
        ( pos,
          "Redeclare "
          ^ name
          ^ " in "
          ^ class_
          ^ " with a compatible signature." );
      ]
    in
    (Error_code.MultipleConcreteDefs, reasons)

  let cyclic_enum_constraint pos =
    let reasons = [(pos, "Cyclic enum constraint")] in
    (Error_code.CyclicEnumConstraint, reasons)

  let inoutness_mismatch pos decl_pos =
    let reasons =
      [
        (pos, "This is an `inout` parameter");
        (decl_pos, "It is incompatible with a normal parameter");
      ]
    in
    (Error_code.InoutnessMismatch, reasons)

  let decl_override_missing_hint pos =
    let reasons =
      [
        ( pos,
          "When redeclaring class members, both declarations must have a typehint"
        );
      ]
    in
    (Error_code.DeclOverrideMissingHint, reasons)

  let bad_lateinit_override pos parent_pos parent_is_lateinit =
    let verb =
      if parent_is_lateinit then
        "is"
      else
        "is not"
    in
    let reasons =
      [
        (pos, "Redeclared properties must be consistently declared `__LateInit`");
        (parent_pos, "The property " ^ verb ^ " declared `__LateInit` here");
      ]
    in

    (Error_code.BadLateInitOverride, reasons)

  let bad_xhp_attr_required_override pos parent_pos parent_tag tag =
    let reasons =
      [
        (pos, "Redeclared attribute must not be less strict");
        ( parent_pos,
          "The attribute is " ^ parent_tag ^ ", which is stricter than " ^ tag
        );
      ]
    in
    (Error_code.BadXhpAttrRequiredOverride, reasons)

  let coeffect_subtyping pos cap pos_expected cap_expected =
    let reasons =
      [
        ( pos_expected,
          "Expected a function that requires " ^ Lazy.force cap_expected );
        (pos, "But got a function that requires " ^ Lazy.force cap);
      ]
    in
    (Error_code.SubtypeCoeffects, reasons)

  let not_sub_dynamic pos ty_name dynamic_part =
    let reasons =
      [
        ( pos,
          "Type "
          ^ (Markdown_lite.md_codify @@ Lazy.force ty_name)
          ^ " is not a subtype of `dynamic` under dynamic-aware subtyping" );
      ]
    in
    (Error_code.UnifyError, dynamic_part @ reasons)

  let override_method_support_dynamic_type
      pos method_name parent_origin parent_pos =
    let reasons =
      [
        ( pos,
          "Method "
          ^ method_name
          ^ " must be declared <<__SupportDynamicType>> because it overrides method in class "
          ^ Render.strip_ns parent_origin
          ^ " which does." );
        (parent_pos, "Overridden method is defined here.");
      ]
    in
    (Error_code.ImplementsDynamic, reasons)

  let readonly_mismatch pos kind reason_sub reason_super =
    let reasons =
      ( pos,
        match kind with
        | `fn -> "Function readonly mismatch"
        | `fn_return -> "Function readonly return mismatch"
        | `param -> "Mismatched parameter readonlyness" )
      :: reason_sub
      @ reason_super
    in
    (Error_code.ReadonlyMismatch, reasons)

  let typing_too_many_args pos decl_pos actual expected =
    let (code, claim, reasons) =
      Common.typing_too_many_args pos decl_pos actual expected
    in
    (code, claim :: reasons)

  let typing_too_few_args pos decl_pos actual expected =
    let (code, claim, reasons) =
      Common.typing_too_few_args pos decl_pos actual expected
    in
    (code, claim :: reasons)

  let non_object_member pos ctxt ty_name member_name kind decl_pos =
    let (code, claim, reasons) =
      Common.non_object_member
        pos
        ctxt
        (Lazy.force ty_name)
        member_name
        kind
        decl_pos
    in
    (code, claim :: reasons)

  let rigid_tvar_escape pos name =
    ( Error_code.RigidTVarEscape,
      [(pos, "Rigid type variable " ^ name ^ " is escaping")] )

  let smember_not_found pos kind member_name class_name class_pos hint =
    let (code, claim, reasons) =
      Common.smember_not_found pos kind member_name class_name class_pos hint
    in
    (code, claim :: reasons)

  let bad_method_override pos member_name =
    let reasons =
      [
        ( pos,
          "The method "
          ^ (Render.strip_ns member_name |> Markdown_lite.md_codify)
          ^ " is not compatible with the overridden method" );
      ]
    in
    (Error_code.BadMethodOverride, reasons)

  let bad_prop_override pos member_name =
    let reasons =
      [
        ( pos,
          "The property "
          ^ (Render.strip_ns member_name |> Markdown_lite.md_codify)
          ^ " has the wrong type" );
      ]
    in
    (Error_code.BadMethodOverride, reasons)

  let subtyping_error reasons = (Error_code.UnifyError, reasons)

  let method_not_dynamically_callable pos parent_pos =
    let reasons =
      [
        (parent_pos, "This method is `__DynamicallyCallable`.");
        (pos, "This method is **not**.");
      ]
    in
    (Error_code.BadMethodOverride, reasons)

  let this_final pos_sub pos_super class_name =
    let n = Render.strip_ns class_name |> Markdown_lite.md_codify in
    let message1 = "Since " ^ n ^ " is not final" in
    let message2 = "this might not be a " ^ n in
    (Error_code.ThisFinal, [(pos_super, message1); (pos_sub, message2)])

  let typeconst_concrete_concrete_override pos parent_pos =
    ( Error_code.TypeconstConcreteConcreteOverride,
      [
        (pos, "Cannot re-declare this type constant");
        (parent_pos, "Previously defined here");
      ] )

  let abstract_concrete_override pos parent_pos kind =
    let kind_str =
      match kind with
      | `method_ -> "method"
      | `typeconst -> "type constant"
      | `constant -> "constant"
      | `property -> "property"
    in
    ( Error_code.AbstractConcreteOverride,
      [
        (pos, "Cannot re-declare this " ^ kind_str ^ " as abstract");
        (parent_pos, "Previously defined here");
      ] )

  let should_not_be_override pos class_id id =
    ( Error_code.ShouldNotBeOverride,
      [
        ( pos,
          Printf.sprintf
            "%s has no parent class with a method %s to override"
            (Render.strip_ns class_id |> Markdown_lite.md_codify)
            (Markdown_lite.md_codify id) );
      ] )

  let override_no_default_typeconst pos parent_pos =
    ( Error_code.OverrideNoDefaultTypeconst,
      [
        (pos, "This abstract type constant does not have a default type");
        ( parent_pos,
          "It cannot override an abstract type constant that has a default type"
        );
      ] )

  let eval t ~current_span =
    match t with
    | Of_error err ->
      Option.map ~f:(fun (code, claim, reasons, _) ->
          (code, Message.map ~f:Pos_or_decl.of_raw_pos claim :: reasons))
      @@ Error.eval err ~current_span
    | Fun_too_many_args { pos; decl_pos; actual; expected } ->
      Some (fun_too_many_args pos decl_pos actual expected)
    | Fun_too_few_args { pos; decl_pos; actual; expected } ->
      Some (fun_too_few_args pos decl_pos actual expected)
    | Fun_unexpected_nonvariadic { pos; decl_pos } ->
      Some (fun_unexpected_nonvariadic pos decl_pos)
    | Fun_variadicity_hh_vs_php56 { pos; decl_pos } ->
      Some (fun_variadicity_hh_vs_php56 pos decl_pos)
    | Type_arity_mismatch { pos; actual; decl_pos; expected } ->
      Some (type_arity_mismatch pos actual decl_pos expected)
    | Violated_constraint { cstrs; reasons; _ } ->
      Some (violated_constraint cstrs reasons)
    | Concrete_const_interface_override { pos; parent_pos; name; parent_origin }
      ->
      Some (concrete_const_interface_override pos parent_pos name parent_origin)
    | Interface_or_trait_const_multiple_defs
        { pos; origin; parent_pos; parent_origin; name } ->
      Some
        (interface_or_trait_const_multiple_defs
           pos
           origin
           parent_pos
           parent_origin
           name)
    | Interface_typeconst_multiple_defs
        { pos; parent_pos; name; origin; parent_origin; is_abstract } ->
      Some
        (interface_typeconst_multiple_defs
           pos
           parent_pos
           name
           origin
           parent_origin
           is_abstract)
    | Missing_field { pos; name; decl_pos } ->
      Some (missing_field pos name decl_pos)
    | Shape_fields_unknown { pos; decl_pos } ->
      Some (shape_fields_unknown pos decl_pos)
    | Abstract_tconst_not_allowed { pos; decl_pos; tconst_name } ->
      Some (abstract_tconst_not_allowed pos decl_pos tconst_name)
    | Invalid_destructure { pos; decl_pos; ty_name } ->
      Some (invalid_destructure pos decl_pos ty_name)
    | Unpack_array_required_argument { pos; decl_pos } ->
      Some (unpack_array_required_argument pos decl_pos)
    | Unpack_array_variadic_argument { pos; decl_pos } ->
      Some (unpack_array_variadic_argument pos decl_pos)
    | Overriding_prop_const_mismatch { pos; is_const; parent_pos; _ } ->
      Some (overriding_prop_const_mismatch pos is_const parent_pos)
    | Visibility_extends { pos; vis; parent_pos; parent_vis } ->
      Some (visibility_extends pos vis parent_pos parent_vis)
    | Visibility_override_internal
        { pos; module_name; parent_module; parent_pos } ->
      Some
        (visibility_override_internal pos module_name parent_module parent_pos)
    | Missing_constructor pos -> Some (missing_constructor pos)
    | Accept_disposable_invariant { pos; decl_pos } ->
      Some (accept_disposable_invariant pos decl_pos)
    | Ifc_external_contravariant { pos_sub; pos_super } ->
      Some (ifc_external_contravariant pos_sub pos_super)
    | Required_field_is_optional { pos; name; decl_pos } ->
      Some (required_field_is_optional pos name decl_pos)
    | Return_disposable_mismatch
        { pos_sub; is_marked_return_disposable; pos_super } ->
      Some
        (return_disposable_mismatch
           pos_sub
           is_marked_return_disposable
           pos_super)
    | Ifc_policy_mismatch { pos; policy; pos_super; policy_super } ->
      Some (ifc_policy_mismatch pos policy pos_super policy_super)
    | Override_final { pos; parent_pos } -> Some (override_final pos parent_pos)
    | Override_lsb { pos; member_name; parent_pos } ->
      Some (override_lsb pos member_name parent_pos)
    | Multiple_concrete_defs
        { pos; origin; name; parent_pos; parent_origin; class_name } ->
      Some
        (multiple_concrete_defs
           pos
           origin
           name
           parent_pos
           parent_origin
           class_name)
    | Cyclic_enum_constraint pos -> Some (cyclic_enum_constraint pos)
    | Inoutness_mismatch { pos; decl_pos } ->
      Some (inoutness_mismatch pos decl_pos)
    | Decl_override_missing_hint pos -> Some (decl_override_missing_hint pos)
    | Bad_lateinit_override { pos; parent_pos; parent_is_lateinit } ->
      Some (bad_lateinit_override pos parent_pos parent_is_lateinit)
    | Bad_xhp_attr_required_override { pos; parent_pos; parent_tag; tag } ->
      Some (bad_xhp_attr_required_override pos parent_pos parent_tag tag)
    | Coeffect_subtyping { pos; cap; pos_expected; cap_expected } ->
      Some (coeffect_subtyping pos cap pos_expected cap_expected)
    | Not_sub_dynamic { pos; ty_name; dynamic_part } ->
      Some (not_sub_dynamic pos ty_name dynamic_part)
    | Override_method_support_dynamic_type
        { pos; method_name; parent_origin; parent_pos } ->
      Some
        (override_method_support_dynamic_type
           pos
           method_name
           parent_origin
           parent_pos)
    | Readonly_mismatch { pos; kind; reason_sub; reason_super } ->
      Some (readonly_mismatch pos kind reason_sub reason_super)
    | Typing_too_many_args { pos; decl_pos; actual; expected } ->
      Some (typing_too_many_args pos decl_pos actual expected)
    | Typing_too_few_args { pos; decl_pos; actual; expected } ->
      Some (typing_too_few_args pos decl_pos actual expected)
    | Non_object_member { pos; ctxt; ty_name; member_name; kind; decl_pos } ->
      Some (non_object_member pos ctxt ty_name member_name kind decl_pos)
    | Rigid_tvar_escape { pos; name } -> Some (rigid_tvar_escape pos name)
    | Smember_not_found { pos; kind; member_name; class_name; class_pos; hint }
      ->
      Some (smember_not_found pos kind member_name class_name class_pos hint)
    | Bad_method_override { pos; member_name } ->
      Some (bad_method_override pos member_name)
    | Bad_prop_override { pos; member_name } ->
      Some (bad_prop_override pos member_name)
    | Subtyping_error reasons -> Some (subtyping_error reasons)
    | Method_not_dynamically_callable { pos; parent_pos } ->
      Some (method_not_dynamically_callable pos parent_pos)
    | This_final { pos_sub; pos_super; class_name } ->
      Some (this_final pos_sub pos_super class_name)
    | Typeconst_concrete_concrete_override { pos; parent_pos } ->
      Some (typeconst_concrete_concrete_override pos parent_pos)
    | Abstract_concrete_override { pos; parent_pos; kind } ->
      Some (abstract_concrete_override pos parent_pos kind)
    | Should_not_be_override { pos; class_id; id } ->
      Some (should_not_be_override pos class_id id)
    | Override_no_default_typeconst { pos; parent_pos } ->
      Some (override_no_default_typeconst pos parent_pos)
end

and Callback : sig
  type t

  val iter : t -> on_prim:(Primary.t -> unit) -> unit

  val apply :
    ?code:Error_code.t ->
    ?reasons:Pos_or_decl.t Message.t list ->
    ?quickfixes:Quickfix.t list ->
    t ->
    claim:Pos.t Message.t ->
    error option

  val always : Primary.t -> t

  val with_side_effect : t -> eff:(unit -> unit) -> t
    [@@ocaml.deprecated
      "This function will be removed. Please avoid adding side effects to error callbacks."]

  val of_primary_error : Primary.t -> t
  val with_code : code:Error_code.t -> t
  val retain_code : t -> t
  val with_claim_as_reason : t -> new_claim:Primary.t -> t
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
  val record_init_value_does_not_match_hint : t
  val strict_str_concat_type_mismatch : t
  val strict_str_interp_type_mismatch : t
  val bitwise_math_invalid_argument : t
  val inc_dec_invalid_argument : t
  val math_invalid_argument : t
  val using_error : Pos.t -> has_await:bool -> t
end = struct
  type t =
    | Always of Primary.t
    | With_claim_as_reason of t * Primary.t
    | With_code of Error_code.t
    | Retain_code of t
    | With_side_effect of t * (unit -> unit)

  let iter t ~on_prim =
    let rec aux = function
      | Always prim -> on_prim prim
      | With_claim_as_reason (t, prim) ->
        on_prim prim;
        aux t
      | Retain_code t
      | With_side_effect (t, _) ->
        aux t
      | With_code _ -> ()
    in
    aux t

  (* -- Evaluation ---------------------------------------------------------- *)
  type error_state = {
    code_opt: Error_code.t option;
    claim_opt: Pos.t Message.t option;
    reasons: Pos_or_decl.t Message.t list;
    quickfixes: Quickfix.t list;
  }

  let with_code code { claim_opt; reasons; quickfixes; _ } =
    Some (code, claim_opt, reasons, quickfixes)

  let rec eval t st =
    match t with
    | With_side_effect (t, eff) ->
      eff ();
      eval t st
    | Always err ->
      Option.map ~f:(fun (code, claim, reasons, quickfixes) ->
          (code, Some claim, reasons, quickfixes))
      @@ Primary.to_error err
    | With_claim_as_reason (err, claim_from) ->
      let reasons =
        Option.value_map
          ~default:st.reasons
          ~f:(fun claim ->
            Tuple2.map_fst ~f:Pos_or_decl.of_raw_pos claim :: st.reasons)
          st.claim_opt
      in
      let claim_opt =
        Option.map ~f:(fun (_, claim, _, _) -> claim)
        @@ Primary.to_error claim_from
      in
      eval err { st with claim_opt; reasons }
    | Retain_code t -> eval t { st with code_opt = None }
    | With_code default -> with_code (Option.value ~default st.code_opt) st

  let apply ?code ?(reasons = []) ?(quickfixes = []) t ~claim =
    let st = { code_opt = code; claim_opt = Some claim; reasons; quickfixes } in
    Option.map ~f:(fun (code, claim_opt, reasons, quickfixes) ->
        (code, Option.value ~default:claim claim_opt, reasons, quickfixes))
    @@ eval t st

  (* -- Constructors -------------------------------------------------------- *)

  let always err = Always err
  let with_side_effect t ~eff = (With_side_effect (t, eff) [@alert.deprecated])
  let with_code ~code = With_code code
  let of_primary_error err = with_code ~code:(Primary.code err)
  let retain_code t = Retain_code t
  let with_claim_as_reason t ~new_claim = With_claim_as_reason (t, new_claim)

  (* -- Specific errors ----------------------------------------------------- *)
  let unify_error = with_code ~code:Error_code.UnifyError
  let index_type_mismatch = with_code ~code:Error_code.IndexTypeMismatch

  let covariant_index_type_mismatch =
    with_code ~code:Error_code.CovariantIndexTypeMismatch

  let expected_stringlike = with_code ~code:Error_code.ExpectedStringlike

  let constant_does_not_match_enum_type =
    with_code ~code:Error_code.ConstantDoesNotMatchEnumType

  let enum_underlying_type_must_be_arraykey =
    with_code ~code:Error_code.EnumUnderlyingTypeMustBeArraykey

  let enum_constraint_must_be_arraykey =
    with_code ~code:Error_code.EnumConstraintMustBeArraykey

  let enum_subtype_must_have_compatible_constraint =
    with_code ~code:Error_code.EnumSubtypeMustHaveCompatibleConstraint

  let parameter_default_value_wrong_type =
    with_code ~code:Error_code.ParameterDefaultValueWrongType

  let newtype_alias_must_satisfy_constraint =
    with_code ~code:Error_code.NewtypeAliasMustSatisfyConstraint

  let missing_return = with_code ~code:Error_code.MissingReturnInNonVoidFunction

  let inout_return_type_mismatch =
    with_code ~code:Error_code.InoutReturnTypeMismatch

  let class_constant_value_does_not_match_hint =
    with_code ~code:Error_code.ClassConstantValueDoesNotMatchHint

  let class_property_initializer_type_does_not_match_hint =
    with_code ~code:Error_code.ClassPropertyInitializerTypeDoesNotMatchHint

  let xhp_attribute_does_not_match_hint =
    with_code ~code:Error_code.XhpAttributeValueDoesNotMatchHint

  let record_init_value_does_not_match_hint =
    with_code ~code:Error_code.RecordInitValueDoesNotMatchHint

  let strict_str_concat_type_mismatch =
    with_code ~code:Error_code.StrictStrConcatTypeMismatch

  let strict_str_interp_type_mismatch =
    with_code ~code:Error_code.StrictStrInterpTypeMismatch

  let bitwise_math_invalid_argument =
    with_code ~code:Error_code.BitwiseMathInvalidArgument

  let inc_dec_invalid_argument =
    with_code ~code:Error_code.IncDecInvalidArgument

  let math_invalid_argument = with_code ~code:Error_code.MathInvalidArgument

  let using_error pos ~has_await =
    let new_claim = Primary.Using_error { pos; has_await } in
    with_claim_as_reason ~new_claim @@ retain_code unify_error
end

and Reasons_callback : sig
  type t

  val iter :
    t -> on_prim:(Primary.t -> unit) -> on_snd:(Secondary.t -> unit) -> unit

  val from_on_error : on_error -> t
    [@@ocaml.deprecated
      "This function will be removed. Please use the provided combinators for constructing error callbacks."]

  val ignore_error : t
  val always : Error.t -> t
  val of_error : Error.t -> t
  val of_primary_error : Primary.t -> t
  val with_claim : Callback.t -> claim:Pos.t Message.t -> t
  val with_code : t -> code:Error_code.t -> t
  val with_reasons : t -> reasons:Pos_or_decl.t Message.t list -> t
  val prepend_reason : t -> reason:Pos_or_decl.t Message.t -> t
  val append_reason : t -> reason:Pos_or_decl.t Message.t -> t
  val append_incoming_reasons : t -> t
  val prepend_incoming_reasons : t -> t
  val retain_code : t -> t
  val retain_reasons : t -> t
  val retain_quickfixes : t -> t
  val prepend_on_apply : t -> Secondary.t -> t
  val assert_in_current_decl : Error_code.t -> ctx:Pos_or_decl.ctx -> t

  val apply_help :
    ?code:Error_code.t ->
    ?claim:Pos.t Message.t ->
    ?reasons:Pos_or_decl.t Message.t list ->
    ?quickfixes:Quickfix.t list ->
    t ->
    current_span:Pos.t ->
    error option

  val apply :
    ?code:Error_code.t ->
    ?claim:Pos.t Message.t ->
    ?reasons:Pos_or_decl.t Message.t list ->
    ?quickfixes:Quickfix.t list ->
    t ->
    current_span:Pos.t ->
    (Pos.t, Pos_or_decl.t) User_error.t option

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
    Pos.t -> name:string -> parent_pos:Pos_or_decl.t -> parent_name:string -> t

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
end = struct
  type op =
    | Append
    | Prepend

  type component =
    | Code
    | Reasons
    | Quickfixes

  type t =
    | Ignore
    | Always of Error.t
    | Of_error of Error.t
    | Of_callback of Callback.t * Pos.t Message.t
    | Retain of t * component
    | Incoming_reasons of t * op
    | With_code of t * Error_code.t
    | With_reasons of t * Pos_or_decl.t Message.t list
    | Add_reason of t * op * Pos_or_decl.t Message.t
    | From_on_error of on_error
    | Prepend_on_apply of t * Secondary.t
    | Assert_in_current_decl of Error_code.t * Pos_or_decl.ctx

  let iter t ~on_prim ~on_snd =
    let rec aux = function
      | Always err
      | Of_error err ->
        Error.iter err ~on_prim ~on_snd
      | Of_callback (cb, _) -> Callback.iter cb ~on_prim
      | Retain (t, _)
      | Incoming_reasons (t, _)
      | With_code (t, _)
      | With_reasons (t, _)
      | Add_reason (t, _, _)
      | Prepend_on_apply (t, _) ->
        aux t
      | From_on_error _
      | Ignore
      | Assert_in_current_decl _ ->
        ()
    in
    aux t

  (* -- Constructors -------------------------------------------------------- *)

  let from_on_error f = From_on_error f
  let ignore_error = Ignore
  let of_error err = Of_error err
  let of_primary_error prim_err = Of_error (Error.primary prim_err)
  let with_claim no_claim ~claim = Of_callback (no_claim, claim)
  let with_code t ~code = With_code (t, code)
  let with_reasons t ~reasons = With_reasons (t, reasons)
  let prepend_reason t ~reason = Add_reason (t, Prepend, reason)
  let append_reason t ~reason = Add_reason (t, Append, reason)
  let append_incoming_reasons t = Incoming_reasons (t, Append)
  let prepend_incoming_reasons t = Incoming_reasons (t, Prepend)
  let retain_code t = Retain (t, Code)
  let retain_reasons t = Retain (t, Reasons)
  let retain_quickfixes t = Retain (t, Quickfixes)
  let always err = Always err
  let prepend_on_apply t snd_err = Prepend_on_apply (t, snd_err)
  let assert_in_current_decl code ~ctx = Assert_in_current_decl (code, ctx)

  (* -- Evaluation ------------------------------------------------------------ *)

  module Error_state = struct
    type t = {
      code_opt: Error_code.t option;
      claim_opt: Pos.t Message.t option;
      reasons_opt: Pos_or_decl.t Message.t list option;
      quickfixes_opt: Quickfix.t list option;
    }

    let with_code t code_opt =
      { t with code_opt = Option.first_some t.code_opt code_opt }

    let prepend_secondary
        ({ claim_opt; reasons_opt; quickfixes_opt; _ } as st)
        snd_err
        ~current_span =
      match Secondary.eval snd_err ~current_span with
      | Some (code, reasons) ->
        {
          code_opt = Some code;
          claim_opt;
          reasons_opt = Some (reasons @ Option.value ~default:[] reasons_opt);
          quickfixes_opt;
        }
      | _ -> st

    (** Replace any missing values in the error state with those of the error *)
    let with_defaults
        { code_opt; claim_opt; reasons_opt; quickfixes_opt } err ~current_span =
      Option.(
        map ~f:(fun (code, claim, reasons, quickfixes) ->
            ( value code_opt ~default:code,
              value claim_opt ~default:claim,
              value reasons_opt ~default:reasons,
              value quickfixes_opt ~default:quickfixes )))
      @@ Error.eval err ~current_span
  end

  let eval_callback
      cb Error_state.{ code_opt; reasons_opt; quickfixes_opt; _ } ~claim =
    Callback.apply
      ?code:code_opt
      ?reasons:reasons_opt
      ?quickfixes:quickfixes_opt
      ~claim
      cb

  let eval t ~st ~current_span =
    let rec eval t st =
      match t with
      | From_on_error f ->
        let code = Option.map ~f:Error_code.to_enum st.Error_state.code_opt
        and quickfixes = st.Error_state.quickfixes_opt
        and reasons = Option.value ~default:[] st.Error_state.reasons_opt in
        f ?code ?quickfixes reasons;
        None
      | Ignore -> None
      | Always err -> Error.eval err ~current_span
      | Of_error err -> Error_state.with_defaults st err ~current_span
      | Of_callback (cb, claim) -> eval_callback cb st ~claim
      | Assert_in_current_decl (default, ctx) ->
        let Error_state.{ code_opt; reasons_opt; quickfixes_opt; _ } = st in
        let crs =
          Option.(value ~default code_opt, value ~default:[] reasons_opt)
        in
        let res_opt = Common.eval_assert ctx current_span crs in
        Option.map res_opt ~f:(function
            | (code, claim, reasons, []) ->
              (code, claim, reasons, Option.value ~default:[] quickfixes_opt)
            | res -> res)
      | With_code (err, code) ->
        let st = Error_state.with_code st @@ Some code in
        eval err st
      | With_reasons (err, reasons) ->
        eval err Error_state.{ st with reasons_opt = Some reasons }
      | Add_reason (err, op, reason) -> eval_reason_op op err reason st
      | Retain (t, comp) -> eval_retain t comp st
      | Incoming_reasons (err, op) ->
        Option.map ~f:(fun ((code, claim, reasons, qfxs) as err) ->
            match (st.Error_state.reasons_opt, op) with
            | (None, _)
            | (Some [], _) ->
              err
            | (Some rs, Append) -> (code, claim, reasons @ rs, qfxs)
            | (Some rs, Prepend) -> (code, claim, rs @ reasons, qfxs))
        @@ eval err Error_state.{ st with reasons_opt = None }
      | Prepend_on_apply (t, snd_err) ->
        eval t (Error_state.prepend_secondary st snd_err ~current_span)
    and eval_reason_op op err base_reason (Error_state.{ reasons_opt; _ } as st)
        =
      let reasons_opt =
        match op with
        | Append ->
          Option.(
            first_some (map reasons_opt ~f:(fun rs -> rs @ [base_reason]))
            @@ Some [base_reason])
        | Prepend ->
          Option.(
            first_some (map reasons_opt ~f:(fun rs -> base_reason :: rs))
            @@ Some [base_reason])
      in
      eval err Error_state.{ st with reasons_opt }
    and eval_retain t comp st =
      match comp with
      | Code -> eval t Error_state.{ st with code_opt = None }
      | Reasons -> eval t Error_state.{ st with reasons_opt = None }
      | Quickfixes -> eval t Error_state.{ st with quickfixes_opt = None }
    in
    eval t st

  let apply_help ?code ?claim ?reasons ?quickfixes t ~current_span =
    let claim = Option.map claim ~f:(Message.map ~f:Pos_or_decl.of_raw_pos) in
    let reasons_opt =
      match (claim, reasons) with
      | (Some claim, Some reasons) -> Some (claim :: reasons)
      | (Some claim, _) -> Some [claim]
      | _ -> reasons
    in
    eval
      t
      ~st:
        Error_state.
          {
            code_opt = code;
            claim_opt = None;
            reasons_opt;
            quickfixes_opt = quickfixes;
          }
      ~current_span

  let apply ?code ?claim ?reasons ?quickfixes t ~current_span =
    let f (code, claim, reasons, quickfixes) =
      User_error.make (Error_code.to_enum code) claim reasons ~quickfixes
    in
    Option.map ~f
    @@ apply_help ?code ?claim ?reasons ?quickfixes t ~current_span
  (* -- Specific callbacks -------------------------------------------------- *)

  let unify_error_at pos =
    of_error
    @@ Error.primary
    @@ Primary.Unify_error { pos; msg_opt = None; reasons_opt = None }

  let bad_enum_decl pos =
    retain_code
    @@ retain_quickfixes
    @@ of_error
    @@ Error.primary
    @@ Primary.Bad_enum_decl pos

  let bad_conditional_support_dynamic pos ~child ~parent ~ty_name ~self_ty_name
      =
    retain_code
    @@ retain_quickfixes
    @@ of_primary_error
    @@ Primary.Bad_conditional_support_dynamic
         { pos; child; parent; ty_name; self_ty_name }

  let bad_decl_override pos ~name ~parent_pos ~parent_name =
    append_incoming_reasons
    @@ retain_quickfixes
    @@ of_primary_error
    @@ Primary.Bad_decl_override { pos; name; parent_pos; parent_name }

  let explain_where_constraint pos ~in_class ~decl_pos =
    append_incoming_reasons
    @@ retain_code
    @@ retain_quickfixes
    @@ of_primary_error
    @@ Primary.Explain_where_constraint { pos; in_class; decl_pos }

  let explain_constraint pos =
    retain_code @@ of_primary_error @@ Primary.Explain_constraint pos

  let rigid_tvar_escape_at pos what =
    retain_quickfixes
    @@ retain_code
    @@ of_primary_error
    @@ Primary.Rigid_tvar_escape { pos; what }

  let invalid_type_hint pos =
    retain_quickfixes @@ of_primary_error @@ Primary.Invalid_type_hint pos

  let type_constant_mismatch t =
    retain_quickfixes @@ with_code ~code:Error_code.TypeConstantMismatch t

  let class_constant_type_mismatch t =
    retain_quickfixes @@ with_code ~code:Error_code.ClassConstantTypeMismatch t

  let unsatisfied_req_callback ~class_pos ~trait_pos ~req_pos req_name =
    append_incoming_reasons
    @@ retain_code
    @@ of_primary_error
    @@ Primary.Unsatisfied_req { pos = class_pos; trait_pos; req_pos; req_name }

  let invalid_echo_argument_at pos =
    of_primary_error @@ Primary.Invalid_echo_argument pos

  let index_type_mismatch_at pos =
    of_primary_error
    @@ Primary.Index_type_mismatch
         {
           pos;
           msg_opt = None;
           reasons_opt = None;
           is_covariant_container = false;
         }

  let unify_error_assert_primary_pos_in_current_decl ctx =
    assert_in_current_decl Error_code.UnifyError ~ctx

  let invalid_type_hint_assert_primary_pos_in_current_decl ctx =
    assert_in_current_decl Error_code.InvalidTypeHint ~ctx
end

include Error
