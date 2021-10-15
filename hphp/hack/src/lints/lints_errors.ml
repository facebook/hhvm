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

let non_equatable_due_to_opaque_types p ty1 ty2 enums =
  Lints.add Codes.non_equatable_comparison Lint_warning p
  @@ Printf.sprintf
       "Invalid comparison:\nA value of type %s should not be compared to a value of type %s%s"
       (Markdown_lite.md_codify ty1)
       (Markdown_lite.md_codify ty2)
       begin
         match enums with
         | [] -> ""
         | enums ->
           Printf.sprintf
             " because the enum type%s %s %s opaque.\nUse functions like `%s::coerce` and `%s::assert` to convert values to the appropriate type before comparing."
             (if List.length enums = 1 then
               ""
             else
               "s")
             (match enums with
             | [a; b] ->
               Printf.sprintf
                 "%s and %s"
                 (Markdown_lite.md_codify a)
                 (Markdown_lite.md_codify b)
             | _ -> String.concat ~sep:", " enums)
             (if List.length enums = 1 then
               "is"
             else
               "are")
             (List.hd_exn enums)
             (List.hd_exn enums)
       end

let is_always_true p lhs_class rhs_class =
  let lhs_class = Markdown_lite.md_codify lhs_class in
  let rhs_class = Markdown_lite.md_codify rhs_class in
  Lints.add
    Codes.is_always_true
    Lint_warning
    p
    (Printf.sprintf
       "This `is` check is always `true`. The expression on the left is an instance of %s. It is always an instance of %s because %s derives from %s."
       lhs_class
       rhs_class
       lhs_class
       rhs_class)

let is_always_false p lhs_class rhs_class =
  let lhs_class = Markdown_lite.md_codify lhs_class in
  let rhs_class = Markdown_lite.md_codify rhs_class in
  Lints.add
    Codes.is_always_false
    Lint_warning
    p
    (Printf.sprintf
       "This `is` check is always `false`. The expression on the left is an instance of %s. It can never be an instance of %s because both %s and %s are classes and neither %s derives from %s nor %s derives from %s."
       lhs_class
       rhs_class
       lhs_class
       rhs_class
       rhs_class
       lhs_class
       lhs_class
       rhs_class)

let as_always_succeeds p lhs_class rhs_class =
  let lhs_class = Markdown_lite.md_codify lhs_class in
  let rhs_class = Markdown_lite.md_codify rhs_class in
  Lints.add
    Codes.as_always_succeeds
    Lint_warning
    p
    (Printf.sprintf
       "This `as` assertion will always succeed and hence is redundant. The expression on the left is an instance of %s. It is always an instance of %s because %s derives from %s."
       lhs_class
       rhs_class
       lhs_class
       rhs_class)

let as_always_fails p lhs_class rhs_class =
  let lhs_class = Markdown_lite.md_codify lhs_class in
  let rhs_class = Markdown_lite.md_codify rhs_class in
  Lints.add
    Codes.as_always_fails
    Lint_warning
    p
    (Printf.sprintf
       "This `as` assertion will always fail. The expression on the left is an instance of %s. It can never be an instance of %s because both %s and %s are classes and neither %s derives from %s nor %s derives from %s."
       lhs_class
       rhs_class
       lhs_class
       rhs_class
       rhs_class
       lhs_class
       lhs_class
       rhs_class)

let as_invalid_type pos var hint =
  Lints.add
    Codes.as_invalid_type
    Lint_error
    pos
    (Printf.sprintf
       "A value of type %s will always throw an exception when refining to %s"
       (Markdown_lite.md_codify var)
       (Markdown_lite.md_codify hint))

let class_overrides_all_trait_methods pos class_name trait_name =
  Lints.add
    Codes.class_overrides_all_trait_methods
    Lint_warning
    pos
    (Printf.sprintf
       "Unused trait: %s is overriding all the methods in %s"
       (Utils.strip_ns class_name |> Markdown_lite.md_codify)
       (Utils.strip_ns trait_name |> Markdown_lite.md_codify))

let invalid_null_check p ret ty =
  Lints.add Codes.invalid_null_check Lint_warning p
  @@ Printf.sprintf
       "Invalid null check: This expression will always return %s.\nA value of type %s can never be null."
       (string_of_bool ret |> Markdown_lite.md_codify)
       (Markdown_lite.md_codify ty)

let redundant_nonnull_assertion p ty =
  Lints.add Codes.redundant_nonnull_assertion Lint_warning p
  @@ Printf.sprintf
       "This `as` assertion will always succeed and hence is redundant. A value of type %s can never be null."
       (Markdown_lite.md_codify ty)

let invalid_disjointness_check p name ty1 ty2 =
  Lints.add Codes.invalid_disjointness_check Lint_warning p
  @@ Printf.sprintf
       "This call to '%s' will always return the same value, because type %s is disjoint from type %s."
       name
       ty1
       ty2

let invalid_switch_case_value_type
    (case_value_p : Ast_defs.pos) case_value_ty scrutinee_ty =
  Lints.add Codes.invalid_switch_case_value_type Lint_warning case_value_p
  @@ Printf.sprintf
       "Switch statements use `==` equality, so comparing values of type %s with %s may not give the desired result."
       (Markdown_lite.md_codify case_value_ty)
       (Markdown_lite.md_codify scrutinee_ty)

let missing_override_attribute p ~class_name ~method_name =
  let msg =
    Printf.sprintf
      "Method %s is also defined on %s, but this method is missing `<<__Override>>`."
      (Markdown_lite.md_codify method_name)
      (Utils.strip_ns class_name |> Markdown_lite.md_codify)
  in
  Lints.add Codes.missing_override_attribute Lint_error p @@ msg

let missing_via_label_attribute p ~class_name ~method_name =
  let msg =
    Printf.sprintf
      "Method %s is also defined on %s, but its first parameter is missing
      `<<%s>>`."
      (Markdown_lite.md_codify method_name)
      (Utils.strip_ns class_name |> Markdown_lite.md_codify)
      Naming_special_names.UserAttributes.uaViaLabel
  in
  Lints.add Codes.missing_via_label_attribute Lint_error p @@ msg

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

let redundant_covariant pos msg suggest =
  Lints.add Codes.redundant_generic Lint_warning pos
  @@ "This generic parameter is redundant because it only appears in a covariant (output) position"
  ^ msg
  ^ ". Consider replacing uses of generic parameter with "
  ^ Markdown_lite.md_codify suggest
  ^ " or specifying `<<__Explicit>>` on the generic parameter"

let redundant_contravariant pos msg suggest =
  Lints.add Codes.redundant_generic Lint_warning pos
  @@ "This generic parameter is redundant because it only appears in a contravariant (input) position"
  ^ msg
  ^ ". Consider replacing uses of generic parameter with "
  ^ Markdown_lite.md_codify suggest
  ^ " or specifying `<<__Explicit>>` on the generic parameter"

let redundant_generic pos =
  Lints.add Codes.redundant_generic Lint_warning pos
  @@ "This generic parameter is unused."

let inferred_variance pos description syntax =
  Lints.add Codes.inferred_variance Lint_advice pos
  @@ "This generic parameter could be marked "
  ^ description
  ^ ". Consider prefixing the generic parameter with "
  ^ syntax

let nullsafe_not_needed pos =
  Lints.add Codes.nullsafe_not_needed Lint_advice pos
  @@ "You are using the "
  ^ Markdown_lite.md_codify "?->"
  ^ " operator but this object cannot be null."
  ^ " You can use the "
  ^ Markdown_lite.md_codify "->"
  ^ " operator instead."

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

let loose_unsafe_cast_lower_bound p ty_str_opt =
  let msg =
    "HH_FIXME input type annotation is too loose, please use a more specific type."
  in
  let msg =
    match ty_str_opt with
    | Some ty_str ->
      msg
      ^ " The typechecker infers "
      ^ Markdown_lite.md_codify ty_str
      ^ " as the most specific type."
    | None -> msg
  in
  Lints.add Codes.loose_unsafe_cast_lower_bound Lint_error p msg

let loose_unsafe_cast_upper_bound p =
  Lints.add
    Codes.loose_unsafe_cast_upper_bound
    Lint_error
    p
    "HH_FIXME output type annotation is too loose, please use a more specific type."
