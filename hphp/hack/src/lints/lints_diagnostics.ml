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

let quickfix { Typing_warning.can_be_captured; original_pos; replacement_pos } =
  let path = Pos.filename (Pos.to_absolute original_pos) in
  let lines = Diagnostics.read_lines path in
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
       "Method %s in trait %s is overridden in class %s and trait %s has `require class %s`. This method is never used."
       (Utils.strip_ns method_name |> Markdown_lite.md_codify)
       (Utils.strip_ns trait_name |> Markdown_lite.md_codify)
       (Utils.strip_ns class_name |> Markdown_lite.md_codify)
       (Utils.strip_ns trait_name |> Markdown_lite.md_codify)
       (Utils.strip_ns class_name))

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
         {
           Typing_warning.can_be_captured;
           original_pos = cast_pos;
           replacement_pos = expr_pos;
         })
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
    Lint_advice

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
  Lints.add ~autofix Codes.loose_unsafe_cast_lower_bound Lint_advice p msg

let loose_unsafe_cast_upper_bound p =
  Lints.add
    Codes.loose_unsafe_cast_upper_bound
    Lint_advice
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
  let lines = Diagnostics.read_lines path in
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
