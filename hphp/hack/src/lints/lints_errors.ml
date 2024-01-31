(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Codes = Lints_codes.Codes
open Lints_core
module Lints = Lints_core

let quickfix ~can_be_captured ~original_pos ~replacement_pos =
  let path = Pos.filename (Pos.to_absolute original_pos) in
  let lines = Errors.read_lines path in
  let content = String.concat ~sep:"\n" lines in
  let modified = Pos.get_text_from_pos ~content replacement_pos in
  let modified =
    if can_be_captured then
      "(" ^ modified ^ ")"
    else
      modified
  in
  (modified, original_pos)

let clone_use p =
  Lints.add
    Codes.clone_use
    Lint_advice
    p
    ("Objects created with `clone` will have references to shared "
    ^ "deep objects. Prefer to implement your own explicit copy "
    ^ "method to ensure the semantics you want. See "
    ^ "http://php.net/manual/en/language.oop5.cloning.php")

let deprecated p msg = Lints.add Codes.deprecated Lint_warning p msg

let include_use p msg = Lints.add Codes.include_use Lint_error p msg

let if_literal p msg = Lints.add Codes.if_literal Lint_warning p msg

let await_in_loop p =
  Lints.add
    Codes.await_in_loop
    Lint_warning
    p
    ("Do not use `await` in a loop. It almost always incurs "
    ^ "non-obvious serial fetching that is easy to miss. "
    ^ "See https://fburl.com/awaitinloop for more information.")

let loop_variable_shadows_local_variable p1 id p2 =
  Lints.add
    Codes.loop_variable_shadows_local_variable
    Lint_warning
    p1
    (Printf.sprintf
       "Loop variable %s shadows a local variable defined or last assigned here:\n%s"
       (Local_id.get_name id |> Markdown_lite.md_codify)
       (Pos.string (Pos.to_relative_string p2)))

let bad_virtualized_method p =
  Lints.add
    Codes.bool_method_return_hint
    Lint_warning
    p
    "`__bool` methods should return `bool`. They show that an expression tree type can be used in a boolean expression."

let non_equatable_comparison p ret ty1 ty2 =
  Lints.add Codes.non_equatable_comparison Lint_warning p
  @@ Printf.sprintf
       "Invalid comparison: This expression will always return %s.\nA value of type %s can never be equal to a value of type %s"
       (string_of_bool ret |> Markdown_lite.md_codify)
       (Markdown_lite.md_codify ty1)
       (Markdown_lite.md_codify ty2)

let invalid_contains_check p trv_val_ty val_ty =
  Lints.add Codes.invalid_contains_check Lint_warning p
  @@ Printf.sprintf
       "Invalid `C\\contains` check: This call will always return `false`.\nA `Traversable<%s>` cannot contain a value of type %s"
       trv_val_ty
       (Markdown_lite.md_codify val_ty)

let invalid_contains_key_check p trv_key_ty key_ty =
  Lints.add Codes.invalid_contains_check Lint_warning p
  @@ Printf.sprintf
       "Invalid `C\\contains_key` check: This call will always return `false`.\nA `KeyedTraversable<%s, ...>` cannot contain a key of type %s"
       trv_key_ty
       (Markdown_lite.md_codify key_ty)

let is_always_true ~check_status p lhs_ty rhs_ty =
  let lhs_ty = Markdown_lite.md_codify lhs_ty in
  let rhs_ty = Markdown_lite.md_codify rhs_ty in
  Lints.add
    Codes.is_always_true
    ~check_status:(Some check_status)
    Lint_warning
    p
    (Printf.sprintf
       "This `is` check is always `true`. The expression on the left has type %s which is a subtype of %s."
       lhs_ty
       rhs_ty)

let is_always_false ~check_status p lhs_ty rhs_ty =
  let lhs_ty = Markdown_lite.md_codify lhs_ty in
  let rhs_ty = Markdown_lite.md_codify rhs_ty in
  Lints.add
    Codes.is_always_false
    ~check_status:(Some check_status)
    Lint_warning
    p
    (Printf.sprintf
       "This `is` check is always `false`. The expression on the left has type %s which shares no values with %s."
       lhs_ty
       rhs_ty)

let as_always_succeeds
    ~check_status ~can_be_captured ~as_pos ~child_expr_pos lhs_ty rhs_ty =
  let lhs_ty = Markdown_lite.md_codify lhs_ty in
  let rhs_ty = Markdown_lite.md_codify rhs_ty in
  let autofix =
    Some
      (quickfix
         ~can_be_captured
         ~original_pos:as_pos
         ~replacement_pos:child_expr_pos)
  in
  Lints.add
    ~autofix
    ~check_status:(Some check_status)
    Codes.as_always_succeeds
    Lint_warning
    as_pos
    (Printf.sprintf
       "This `as` assertion will always succeed and hence is redundant. The expression on the left has a type %s which is a subtype of %s."
       lhs_ty
       rhs_ty)

let as_always_fails ~check_status p lhs_ty rhs_ty =
  let lhs_ty = Markdown_lite.md_codify lhs_ty in
  let rhs_ty = Markdown_lite.md_codify rhs_ty in
  Lints.add
    Codes.as_always_fails
    ~check_status:(Some check_status)
    Lint_warning
    p
    (Printf.sprintf
       "This `as` assertion will always fail and lead to an exception at runtime. The expression on the left has type %s which shares no values with %s."
       lhs_ty
       rhs_ty)

let class_overrides_all_trait_methods pos class_name trait_name =
  Lints.add
    Codes.class_overrides_all_trait_methods
    Lint_warning
    pos
    (Printf.sprintf
       "Unused trait: %s is overriding all the methods in %s"
       (Utils.strip_ns class_name |> Markdown_lite.md_codify)
       (Utils.strip_ns trait_name |> Markdown_lite.md_codify))

let trait_requires_class_that_overrides_method
    pos class_name trait_name method_name =
  Lints.add
    Codes.unreachable_method_in_trait
    Lint_warning
    pos
    (Printf.sprintf
       "Method %s in trait %s is overriden in class %s and trait %s has `require class %s`. This method is never used."
       (Utils.strip_ns method_name |> Markdown_lite.md_codify)
       (Utils.strip_ns trait_name |> Markdown_lite.md_codify)
       (Utils.strip_ns class_name |> Markdown_lite.md_codify)
       (Utils.strip_ns trait_name |> Markdown_lite.md_codify)
       (Utils.strip_ns class_name |> Markdown_lite.md_codify))

let invalid_disjointness_check p name ty1 ty2 =
  Lints.add Codes.invalid_disjointness_check Lint_warning p
  @@ Printf.sprintf
       "This call to '%s' will always return the same value, because type %s is disjoint from type %s."
       name
       ty1
       ty2

let invalid_disjointness_check_dynamic p name ty1 ty2 =
  Lints.add Codes.invalid_disjointness_check Lint_warning p
  @@ Printf.sprintf
       "Non-dynamic calls to '%s' will always return the same value, because type %s is disjoint from type %s."
       name
       ty1
       ty2

let invalid_switch_case_value_type
    (case_value_p : Ast_defs.pos) case_value_ty scrutinee_ty =
  Lints.add Codes.invalid_switch_case_value_type Lint_warning case_value_p
  @@ Printf.sprintf
       "Switch statements use `===` equality. Comparing values of type %s with %s may not give the desired result."
       (Markdown_lite.md_codify @@ Lazy.force case_value_ty)
       (Markdown_lite.md_codify @@ Lazy.force scrutinee_ty)

let missing_override_attribute
    ~meth_pos ~first_attr_pos ~name_pos ~class_name ~method_name =
  let msg =
    Printf.sprintf
      "Method %s is also defined on %s, but this method is missing `<<__Override>>`."
      (Markdown_lite.md_codify method_name)
      (Utils.strip_ns class_name |> Markdown_lite.md_codify)
  in
  let autofix =
    match first_attr_pos with
    | Some first_attr_pos ->
      (* The method already has <<Foo, Bar>> attributes, add __Override. *)
      let fix_pos = Pos.shrink_to_start first_attr_pos in
      Some ("__Override, ", fix_pos)
    | None ->
      (* The method has no attributes. *)
      let fix_pos = Pos.shrink_to_start meth_pos in
      Some ("<<__Override>>\n  ", fix_pos)
  in

  Lints.add ~autofix Codes.missing_override_attribute Lint_error name_pos @@ msg

let sketchy_null_check pos name kind =
  let name = Option.value name ~default:"$x" in
  Lints.add Codes.sketchy_null_check Lint_warning pos
  @@ "This is a sketchy null check.\nIt detects nulls, but it will also detect many other falsy values, including `false`, `0`, `0.0`, `\"\"`, `\"0\"`, empty Containers, and more.\nIf you want to test for them, please consider doing so explicitly.\nIf you only meant to test for `null`, "
  ^
  match kind with
  | `Coalesce ->
    Printf.sprintf "use `%s ?? $default` instead of `%s ?: $default`" name name
  | `Eq -> Printf.sprintf "use `%s is null` instead" name
  | `Neq -> Printf.sprintf "use `%s is nonnull` instead" name

let invalid_truthiness_test pos ty =
  Lints.add Codes.invalid_truthiness_test Lint_warning pos
  @@ Printf.sprintf
       "Invalid condition: a value of type %s will always be truthy"
       (Markdown_lite.md_codify ty)

let invalid_truthiness_test_falsy pos ty =
  Lints.add Codes.invalid_truthiness_test Lint_warning pos
  @@ Printf.sprintf
       "Invalid condition: a value of type %s will always be falsy"
       (Markdown_lite.md_codify ty)

let sketchy_truthiness_test pos ty truthiness =
  Lints.add Codes.sketchy_truthiness_test Lint_warning pos
  @@
  match truthiness with
  | `String ->
    Printf.sprintf
      "Sketchy condition: testing the truthiness of %s may not behave as expected.\nThe values `\"\"` and `\"0\"` are both considered falsy. To check for emptiness, use `Str\\is_empty`."
      ty
  | `Arraykey ->
    Printf.sprintf
      "Sketchy condition: testing the truthiness of %s may not behave as expected.\nThe values `0`, `\"\"`, and `\"0\"` are all considered falsy. Test for them explicitly."
      ty
  | `Stringish ->
    Printf.sprintf
      "Sketchy condition: testing the truthiness of a %s may not behave as expected.\nThe values `\"\"` and `\"0\"` are both considered falsy, but objects will be truthy even if their `__toString` returns `\"\"` or `\"0\"`.\nTo check for emptiness, convert to a string and use `Str\\is_empty`."
      ty
  | `XHPChild ->
    Printf.sprintf
      "Sketchy condition: testing the truthiness of an %s may not behave as expected.\nThe values `\"\"` and `\"0\"` are both considered falsy, but objects (including XHP elements) will be truthy even if their `__toString` returns `\"\"` or `\"0\"`."
      ty
  | `Traversable ->
    (* We have a truthiness test on a value with an interface type which is a
         subtype of Traversable, but not a subtype of Container.
         Since the runtime value may be a falsy-when-empty Container or an
         always-truthy Iterable/Generator, we forbid the test. *)
    Printf.sprintf
      "Sketchy condition: a value of type %s may be truthy even when empty.\nHack collections and arrays are falsy when empty, but user-defined Traversables will always be truthy, even when empty.\nIf you would like to only allow containers which are falsy when empty, use the `Container` or `KeyedContainer` interfaces."
      ty

let redundant_covariant pos name msg suggest =
  Lints.add Codes.redundant_generic Lint_warning pos
  @@ "The generic parameter "
  ^ Markdown_lite.md_codify name
  ^ " is redundant because it only appears in a covariant (output) position"
  ^ msg
  ^ ". Consider replacing uses of generic parameter with "
  ^ Markdown_lite.md_codify suggest
  ^ " or specifying `<<__Explicit>>` on the generic parameter"

let redundant_contravariant pos name msg suggest =
  Lints.add Codes.redundant_generic Lint_warning pos
  @@ "The generic parameter "
  ^ Markdown_lite.md_codify name
  ^ " is redundant because it only appears in a contravariant (input) position"
  ^ msg
  ^ ". Consider replacing uses of generic parameter with "
  ^ Markdown_lite.md_codify suggest
  ^ " or specifying `<<__Explicit>>` on the generic parameter"

let redundant_generic pos name =
  Lints.add Codes.redundant_generic Lint_warning pos
  @@ Printf.sprintf
       "The generic parameter %s is unused."
       (Markdown_lite.md_codify name)

let inferred_variance pos name description syntax =
  Lints.add Codes.inferred_variance Lint_advice pos
  @@ "The generic parameter "
  ^ Markdown_lite.md_codify name
  ^ " could be marked "
  ^ description
  ^ ". Consider prefixing it with "
  ^ syntax

let nullsafe_not_needed pos =
  Lints.add Codes.nullsafe_not_needed Lint_advice pos
  @@ "You are using the "
  ^ Markdown_lite.md_codify "?->"
  ^ " operator but this object cannot be null."
  ^ " You can use the "
  ^ Markdown_lite.md_codify "->"
  ^ " operator instead."

let invalid_attribute_value
    pos (attr_name : string) (valid_values : string list) =
  let valid_values = List.map valid_values ~f:Markdown_lite.md_codify in
  Lints.add
    Codes.bad_xhp_enum_attribute_value
    Lint_error
    pos
    (Printf.sprintf
       "Invalid value for %s, expected one of %s."
       (Markdown_lite.md_codify attr_name)
       (String.concat ~sep:", " valid_values))

let parse_error code pos msg = Lints.add code Lint_error pos msg

let rec prettify_class_list names =
  match names with
  | [] -> ""
  | [c] -> c
  | [c1; c2] -> c1 ^ " and " ^ c2
  | h :: t -> h ^ ", " ^ prettify_class_list t

let duplicate_property_class_constant_init
    pos ~class_name ~prop_name ~class_names =
  Lints.add
    Codes.duplicate_property_enum_init
    Lint_error
    pos
    ("Property "
    ^ (Utils.strip_ns prop_name |> Markdown_lite.md_codify)
    ^ ", defined in "
    ^ prettify_class_list (List.map ~f:Utils.strip_ns class_names)
    ^ ", is inherited multiple times by class "
    ^ (Utils.strip_ns class_name |> Markdown_lite.md_codify)
    ^ " and one of its instances is initialised with a class or enum constant")

let duplicate_property pos ~class_name ~prop_name ~class_names =
  Lints.add
    Codes.duplicate_property
    Lint_warning
    pos
    ("Duplicated property "
    ^ (Utils.strip_ns prop_name |> Markdown_lite.md_codify)
    ^ " in "
    ^ (Utils.strip_ns class_name |> Markdown_lite.md_codify)
    ^ " (defined in "
    ^ prettify_class_list
        (List.map
           ~f:(fun n -> Utils.strip_ns n |> Markdown_lite.md_codify)
           class_names)
    ^ "): all instances will be aliased at runtime")

let redundant_cast_common
    ~can_be_captured
    ?(check_status = None)
    cast_type
    cast
    cast_pos
    expr_pos
    code
    severity =
  let msg =
    "This use of `"
    ^ cast
    ^ "` is redundant since the type of the expression is a subtype of the type being cast to."
    ^ " Please consider removing this cast."
  in
  let msg =
    match cast_type with
    | `UNSAFE_CAST -> msg
    | `CAST ->
      msg
      ^ " This cast is runtime significant and types might occasionally lie."
      ^ " Please be prudent when acting on this lint."
  in
  let autofix =
    Some
      (quickfix
         ~can_be_captured
         ~original_pos:cast_pos
         ~replacement_pos:expr_pos)
  in
  Lints.add ~check_status ~autofix code severity cast_pos msg

let redundant_unsafe_cast ~can_be_captured hole_pos expr_pos =
  let cast = "HH\\FIXME\\UNSAFE_CAST" in
  let code = Codes.redundant_unsafe_cast in
  redundant_cast_common
    ~can_be_captured
    `UNSAFE_CAST
    cast
    hole_pos
    expr_pos
    code
    Lint_error

let redundant_cast ~can_be_captured ~check_status cast cast_pos expr_pos =
  let code = Codes.redundant_cast in
  redundant_cast_common
    ~check_status:(Some check_status)
    ~can_be_captured
    `CAST
    cast
    cast_pos
    expr_pos
    code
    Lint_advice

let loose_unsafe_cast_lower_bound p ty_str_opt =
  let msg =
    "The input type to `HH\\FIXME\\UNSAFE_CAST` should be as specific as possible."
  in
  let (msg, autofix) =
    match ty_str_opt with
    | Some ty_str ->
      ( msg ^ " Consider using " ^ Markdown_lite.md_codify ty_str ^ " instead.",
        Some (ty_str, p) )
    | None -> (msg, None)
  in
  Lints.add ~autofix Codes.loose_unsafe_cast_lower_bound Lint_error p msg

let loose_unsafe_cast_upper_bound p =
  Lints.add
    Codes.loose_unsafe_cast_upper_bound
    Lint_error
    p
    "HH\\FIXME\\UNSAFE_CAST output type annotation is too loose, please use a more specific type."

let switch_nonexhaustive p =
  Lints.add
    Codes.switch_nonexhaustive
    Lint_warning
    p
    ("This switch statement is not exhaustive."
    ^ " The expression it scrutinises has a type with infinitely many values and the statement does not have a default case."
    ^ " If none of the cases match, an exception will be thrown."
    ^ " Consider adding a default case.")

let calling_pointless_boolean p_lint p_quickfix txt =
  Lints.add
    ~autofix:(Some ("", p_quickfix))
    Codes.pointless_booleans_expression
    Lint_warning
    p_lint
    txt

let comparing_booleans p_expr p_var name value =
  let msg =
    if value then
      "Consider changing this statement to " ^ "(" ^ name ^ ") instead"
    else
      "Consider changing this statement to " ^ "(!" ^ name ^ ") instead"
  in
  let path = Pos.filename (Pos.to_absolute p_var) in
  let lines = Errors.read_lines path in
  let src = String.concat ~sep:"\n" lines in
  let replacement =
    if value then
      Pos.get_text_from_pos ~content:src p_var
    else
      String.make 1 '!' ^ Pos.get_text_from_pos ~content:src p_var
  in
  Lints.add
    ~autofix:(Some (replacement, p_expr))
    Codes.comparing_booleans
    Lint_advice
    p_expr
    msg

let unconditional_recursion p =
  Lints.add
    Codes.unconditional_recursion
    Lint_error
    p
    ("This is a recursive function with no base case to terminate. This will result in a Stack Overflow error."
    ^ " Consider adding a base case.")

let branches_return_same_value p =
  Lints.add
    Codes.branch_return_same_value
    Lint_warning
    p
    "There are multiple return statements, but they all return the same value."

let internal_classname p =
  Lints.add
    Codes.internal_classname
    Lint_warning
    p
    ("This is a classname of an `internal` class. Internal classnames are dangerous because they are effectively raw strings. "
    ^ "Please avoid them, or make sure that they are never used outside of the module."
    )

let async_lambda pos =
  Lints.add
    Codes.async_lambda
    Lint_advice
    pos
    "Use `async ... ==> await ...` for lambdas that directly return `Awaitable`s, so stack traces include the lambda position."

let awaitable_awaitable pos =
  Lints.add
    Codes.awaitable_awaitable
    Lint_warning
    pos
    ("This lambda returns an Awaitable of Awaitable."
    ^ " You probably want to use await inside this async lambda,"
    ^ " so stack traces include the lambda position."
    ^ " If this is intentional, please annotate the return type.")

let cast_non_primitive pos =
  Lints.add
    Codes.cast_non_primitive
    Lint_error
    pos
    ("Casting a non-primitive to a primitive rarely yields a "
    ^ "useful value. Did you mean to extract a value from this object "
    ^ "before casting it, or to do a null-check?")
