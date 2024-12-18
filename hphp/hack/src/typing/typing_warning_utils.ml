(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core
module Codes = Error_codes.Warning

let severity = User_error.Warning

module type Warning = sig
  type t

  val claim : t -> string

  val code : t -> Codes.t

  val codes : Codes.t list

  val reasons : t -> Pos_or_decl.t Message.t list

  val quickfixes : t -> Pos.t Quickfix.t list
end

module type Lint = sig
  include Warning

  val lint_code : t -> int

  val lint_severity : t -> Lints_core.severity

  val lint_quickfix : t -> Typing_warning.quickfix option
end

module Sketchy_equality = struct
  type t = Typing_warning.Sketchy_equality.t

  let code = Codes.SketchyEquality

  let codes = [code]

  let code _ = code

  let claim { Typing_warning.Sketchy_equality.result; _ } =
    let b =
      Markdown_lite.md_codify
        (if result then
          "true"
        else
          "false")
    in
    Printf.sprintf "This expression is likely always %s" b

  let reasons
      {
        Typing_warning.Sketchy_equality.result = _;
        left = (lazy left);
        right = (lazy right);
        left_trail;
        right_trail;
      } =
    let typedef_trail_entry pos = (pos, "Typedef definition comes from here") in
    let left_trail = List.map left_trail ~f:typedef_trail_entry in
    let right_trail = List.map right_trail ~f:typedef_trail_entry in
    left @ left_trail @ right @ right_trail

  let quickfixes _ = []
end

module Is_as_always = struct
  type t = Typing_warning.Is_as_always.t

  let code { Typing_warning.Is_as_always.kind; _ } =
    match kind with
    | Typing_warning.Is_as_always.Is_is_always_true -> Codes.IsIsAlwaysTrue
    | Typing_warning.Is_as_always.Is_is_always_false -> Codes.IsIsAlwaysFalse
    | Typing_warning.Is_as_always.As_always_succeeds _ -> Codes.AsAlwaysSucceeds
    | Typing_warning.Is_as_always.As_always_fails -> Codes.AsAlwaysFails

  let codes =
    [
      Codes.IsIsAlwaysTrue;
      Codes.IsIsAlwaysFalse;
      Codes.AsAlwaysSucceeds;
      Codes.AsAlwaysFails;
    ]

  let lint_severity _ = Lints_core.Lint_warning

  let claim { Typing_warning.Is_as_always.kind; lhs_ty; rhs_ty } =
    Printf.sprintf
      (match kind with
      | Typing_warning.Is_as_always.Is_is_always_true ->
        "This `is` check is always `true`. The expression on the left has type %s which is a subtype of %s."
      | Typing_warning.Is_as_always.Is_is_always_false ->
        "This `is` check is always `false`. The expression on the left has type %s which shares no values with %s."
      | Typing_warning.Is_as_always.As_always_succeeds _ ->
        "This `as` assertion will always succeed and hence is redundant. The expression on the left has a type %s which is a subtype of %s."
      | Typing_warning.Is_as_always.As_always_fails ->
        "This `as` assertion will always fail and lead to an exception at runtime. The expression on the left has type %s which shares no values with %s.")
      (Markdown_lite.md_codify lhs_ty)
      (Markdown_lite.md_codify rhs_ty)

  let reasons _ = []

  let lint_code { Typing_warning.Is_as_always.kind; _ } =
    match kind with
    | Typing_warning.Is_as_always.Is_is_always_true ->
      Lints_codes.Codes.is_always_true
    | Typing_warning.Is_as_always.Is_is_always_false ->
      Lints_codes.Codes.is_always_false
    | Typing_warning.Is_as_always.As_always_succeeds _ ->
      Lints_codes.Codes.as_always_succeeds
    | Typing_warning.Is_as_always.As_always_fails ->
      Lints_codes.Codes.as_always_fails

  let lint_quickfix { Typing_warning.Is_as_always.kind; _ } =
    match kind with
    | Typing_warning.Is_as_always.Is_is_always_true
    | Typing_warning.Is_as_always.Is_is_always_false
    | Typing_warning.Is_as_always.As_always_fails ->
      None
    | Typing_warning.Is_as_always.As_always_succeeds quickfix -> Some quickfix

  let quickfixes _ = []
end

module Sketchy_null_check = struct
  type t = Typing_warning.Sketchy_null_check.t

  let code = Codes.SketchyNullCheck

  let codes = [code]

  let code _ = code

  let lint_code _ = Lints_codes.Codes.sketchy_null_check

  let lint_severity _ = Lints_core.Lint_warning

  let claim { Typing_warning.Sketchy_null_check.name; kind; ty } =
    let name = Option.value name ~default:"$x" in

    "This is a sketchy null check on an expression of type "
    ^ ty
    ^ ".\n"
    ^ "It detects nulls, but it will also detect many other falsy values, including `false`, `0`, `0.0`, `\"\"`, `\"0\"`, empty Containers, and more.\n"
    ^ "If you want to test for them, please consider doing so explicitly.\n"
    ^ "If you only meant to test for `null`, "
    ^
    match kind with
    | Typing_warning.Sketchy_null_check.Coalesce ->
      Printf.sprintf
        "use `%s ?? $default` instead of `%s ?: $default`"
        name
        name
    | Typing_warning.Sketchy_null_check.Eq ->
      Printf.sprintf "use `%s is null` instead" name
    | Typing_warning.Sketchy_null_check.Neq ->
      Printf.sprintf "use `%s is nonnull` instead" name

  let reasons _ = []

  let quickfixes _ = []

  let lint_quickfix _ = None
end

module Non_disjoint_check = struct
  type t = Typing_warning.Non_disjoint_check.t

  let code = Codes.NonDisjointCheck

  let codes = [code]

  let code _ = code

  let lint_code _ = Lints_codes.Codes.invalid_disjointness_check

  let lint_severity _ = Lints_core.Lint_warning

  let claim { Typing_warning.Non_disjoint_check.name; ty1; ty2; dynamic } =
    Printf.sprintf
      "%s to '%s' will always return the same value, because type %s is disjoint from type %s."
      (if dynamic then
        "Non-dynamic calls"
      else
        "This call")
      name
      ty1
      ty2

  let reasons _ = []

  let quickfixes _ = []

  let lint_quickfix _ = None
end

module Cast_non_primitive = struct
  type t = Typing_warning.Cast_non_primitive.t

  let code = Codes.CastNonPrimitive

  let codes = [code]

  let code _ = code

  let lint_code _ = Lints_codes.Codes.cast_non_primitive

  let lint_severity _ = Lints_core.Lint_error

  let claim { Typing_warning.Cast_non_primitive.cast_hint; ty } =
    Printf.sprintf
      ("Casting %s to %s: casting a non-primitive type to a primitive rarely yields a useful value. "
      ^^ "Did you mean to extract a value from this object before casting it, or to do a null-check?"
      )
      ty
      cast_hint

  let reasons _ = []

  let quickfixes _ = []

  let lint_quickfix _ = None
end

module Truthiness_test = struct
  open Typing_warning.Truthiness_test

  type nonrec t = t

  let code = Codes.TruthinessTest

  let codes = [code]

  let code _ = code

  let lint_code { kind; _ } =
    match kind with
    | Invalid _ -> Lints_codes.Codes.invalid_truthiness_test
    | Sketchy _ -> Lints_codes.Codes.sketchy_truthiness_test

  let lint_severity _ = Lints_core.Lint_warning

  let claim { kind; ty; expr; not } =
    match kind with
    | Invalid { truthy } ->
      Printf.sprintf
        "Invalid condition: a value of type %s will always be %s"
        (Markdown_lite.md_codify ty)
        (if truthy then
          "truthy"
        else
          "falsy")
    | Sketchy sketchy ->
      Printf.sprintf
        "%sketchy condition: testing the %s of %s may not behave as expected.\n%s"
        (match expr with
        | None -> "S"
        | Some e ->
          Printf.sprintf
            "`%s%s` is a s"
            (if not then
              "!"
            else
              "")
            e)
        (if not then
          "falsiness"
        else
          "truthiness")
        ty
        (match sketchy with
        | String ->
          Printf.sprintf
            "The values `\"\"` and `\"0\"` are both considered falsy. To check for %semptiness, use `%sStr\\is_empty%s`."
            (if not then
              ""
            else
              "non-")
            (if not then
              ""
            else
              "!")
            (match expr with
            | None -> ""
            | Some e -> Printf.sprintf "(%s)" e)
        | Arraykey ->
          "The values `0`, `\"\"`, and `\"0\"` are all considered falsy. Test for them explicitly."
        | Stringish ->
          "The values `\"\"` and `\"0\"` are both considered falsy, but objects will be truthy even if their `__toString` returns `\"\"` or `\"0\"`.\n"
          ^ "To check for emptiness, convert to a string and use `Str\\is_empty`."
        | Xhp_child ->
          "The values `\"\"` and `\"0\"` are both considered falsy, but objects (including XHP elements) will be truthy even if their `__toString` returns `\"\"` or `\"0\"`."
        | Traversable ->
          "A value of this type may be truthy even when empty.\n"
          ^ "Hack collections and arrays are falsy when empty, but user-defined Traversables will always be truthy, even when empty.\n"
          ^ "If you would like to only allow containers which are falsy when empty, use the `Container` or `KeyedContainer` interfaces.")

  let reasons _ = []

  let quickfixes _ = []

  let lint_quickfix _ = None
end

module Equality_check = struct
  open Typing_warning.Equality_check

  type t = Typing_warning.Equality_check.t

  let code = Codes.EqualityCheck

  let codes = [code]

  let code _ = code

  let lint_code { kind; _ } =
    match kind with
    | Equality _ -> Lints_codes.Codes.non_equatable_comparison
    | Contains
    | Contains_key ->
      Lints_codes.Codes.invalid_contains_check

  let lint_severity _ = Lints_core.Lint_warning

  let claim { kind; ty1; ty2 } =
    match kind with
    | Equality b ->
      Printf.sprintf
        "Invalid comparison: This expression will always return %s.\nA value of type %s can never be equal to a value of type %s"
        (string_of_bool b |> Markdown_lite.md_codify)
        (Markdown_lite.md_codify ty1)
        (Markdown_lite.md_codify ty2)
    | Contains ->
      Printf.sprintf
        "Invalid `C\\contains` check: This call will always return `false`.\nA `Traversable<%s>` cannot contain a value of type %s"
        ty1
        (Markdown_lite.md_codify ty2)
    | Contains_key ->
      Printf.sprintf
        "Invalid `C\\contains_key` check: This call will always return `false`.\nA `KeyedTraversable<%s, ...>` cannot contain a key of type %s"
        ty1
        (Markdown_lite.md_codify ty2)

  let reasons _ = []

  let quickfixes _ = []

  let lint_quickfix _ = None
end

module Duplicated_properties = struct
  open Typing_warning.Duplicate_properties

  type t = Typing_warning.Duplicate_properties.t

  let code = Codes.DuplicateProperties

  let codes = [code]

  let code _ = code

  let lint_code { initialized_with_constant; _ } =
    if initialized_with_constant then
      Lints_codes.Codes.duplicate_property_enum_init
    else
      Lints_codes.Codes.duplicate_property

  let lint_severity { initialized_with_constant; _ } =
    if initialized_with_constant then
      Lints_core.Lint_error
    else
      Lints_core.Lint_warning

  let rec prettify_class_list names =
    match names with
    | [] -> ""
    | [c] -> c
    | [c1; c2] -> c1 ^ " and " ^ c2
    | h :: t -> h ^ ", " ^ prettify_class_list t

  let claim { initialized_with_constant; class_name; prop_name; class_names } =
    if initialized_with_constant then
      "Property "
      ^ (Utils.strip_ns prop_name |> Markdown_lite.md_codify)
      ^ ", defined in "
      ^ prettify_class_list (List.map ~f:Utils.strip_ns class_names)
      ^ ", is inherited multiple times by class "
      ^ (Utils.strip_ns class_name |> Markdown_lite.md_codify)
      ^ " and one of its instances is initialised with a class or enum constant"
    else
      "Duplicated property "
      ^ (Utils.strip_ns prop_name |> Markdown_lite.md_codify)
      ^ " in "
      ^ (Utils.strip_ns class_name |> Markdown_lite.md_codify)
      ^ " (defined in "
      ^ prettify_class_list
          (List.map
             ~f:(fun n -> Utils.strip_ns n |> Markdown_lite.md_codify)
             class_names)
      ^ "): all instances will be aliased at runtime"

  let reasons _ = []

  let quickfixes _ = []

  let lint_quickfix _ = None
end

module Class_pointer_to_string = struct
  type t = Typing_warning.Class_pointer_to_string.t

  let code = Codes.ClassPointerToString

  let codes = [code]

  let code _ = code

  let claim { Typing_warning.Class_pointer_to_string.ty; _ } =
    "Using "
    ^ ty
    ^ " in this position will trigger an implicit runtime conversion to string. You may use "
    ^ Markdown_lite.md_codify "HH\\class_to_classname"
    ^ " to get the class name as a string."

  let reasons _ = []

  let quickfixes _ = []
end

let module_of (type a x) (kind : (x, a) Typing_warning.kind) :
    (module Warning with type t = x) =
  match kind with
  | Typing_warning.Sketchy_equality -> (module Sketchy_equality)
  | Typing_warning.Is_as_always -> (module Is_as_always)
  | Typing_warning.Sketchy_null_check -> (module Sketchy_null_check)
  | Typing_warning.Non_disjoint_check -> (module Non_disjoint_check)
  | Typing_warning.Cast_non_primitive -> (module Cast_non_primitive)
  | Typing_warning.Truthiness_test -> (module Truthiness_test)
  | Typing_warning.Equality_check -> (module Equality_check)
  | Typing_warning.Duplicate_properties -> (module Duplicated_properties)
  | Typing_warning.Class_pointer_to_string -> (module Class_pointer_to_string)

let module_of_migrated
    (type x) (kind : (x, Typing_warning.migrated) Typing_warning.kind) :
    (module Lint with type t = x) =
  match kind with
  | Typing_warning.Is_as_always -> (module Is_as_always)
  | Typing_warning.Sketchy_null_check -> (module Sketchy_null_check)
  | Typing_warning.Non_disjoint_check -> (module Non_disjoint_check)
  | Typing_warning.Cast_non_primitive -> (module Cast_non_primitive)
  | Typing_warning.Truthiness_test -> (module Truthiness_test)
  | Typing_warning.Equality_check -> (module Equality_check)
  | Typing_warning.Duplicate_properties -> (module Duplicated_properties)

let code_is_enabled tcopt code =
  match TypecheckerOptions.hack_warnings tcopt with
  | GlobalOptions.NNone -> false
  | GlobalOptions.All_except disabled_codes ->
    not (List.mem disabled_codes (Codes.to_enum code) ~equal:Int.equal)

let add tcopt (type a x) ((pos, kind, warning) : (x, a) Typing_warning.t) : unit
    =
  let (module M) = module_of kind in
  if code_is_enabled tcopt (M.code warning) then
    Errors.add_error
      {
        User_error.severity;
        code = Codes.to_enum (M.code warning);
        claim = (pos, M.claim warning);
        reasons = M.reasons warning;
        explanation = Explanation.empty;
        quickfixes = M.quickfixes warning;
        flags = User_error_flags.empty;
        custom_msgs = [];
        is_fixmed = false;
      }

let add_for_migration
    tcopt
    (type x)
    ~(as_lint : Tast.check_status option option)
    (warning : (x, Typing_warning.migrated) Typing_warning.t) : unit =
  match as_lint with
  | None -> add tcopt warning
  | Some check_status ->
    let (pos, kind, warning) = warning in
    let (module M) = module_of_migrated kind in
    Lints_core.add
      ~autofix:(M.lint_quickfix warning |> Option.map ~f:Lints_errors.quickfix)
      (M.lint_code warning)
      ~check_status
      (M.lint_severity warning)
      pos
      (M.claim warning)

let add env warning = add (Typing_env.get_tcopt env) warning

let codes (type a x) (kind : (x, a) Typing_warning.kind) :
    Error_codes.Warning.t list =
  let (module M) = module_of kind in
  M.codes
