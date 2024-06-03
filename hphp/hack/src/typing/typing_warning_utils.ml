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

  val code : Codes.t

  val reasons : t -> Pos_or_decl.t Message.t list

  val quickfixes : t -> Pos.t Quickfix.t list
end

module type Lint = sig
  include Warning

  val lint_code : t -> int

  val lint_quickfix : t -> Typing_warning.quickfix option
end

module SketchyEquality = struct
  type t = Typing_warning.SketchyEquality.t

  let code = Codes.SketchyEquality

  let claim { Typing_warning.SketchyEquality.result; _ } =
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
        Typing_warning.SketchyEquality.result = _;
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

module IsAsAlways = struct
  type t = Typing_warning.IsAsAlways.t

  let code = Codes.IsAsAlways

  let claim { Typing_warning.IsAsAlways.kind; lhs_ty; rhs_ty } =
    Printf.sprintf
      (match kind with
      | Typing_warning.IsAsAlways.Is_is_always_true ->
        "This `is` check is always `true`. The expression on the left has type %s which is a subtype of %s."
      | Typing_warning.IsAsAlways.Is_is_always_false ->
        "This `is` check is always `false`. The expression on the left has type %s which shares no values with %s."
      | Typing_warning.IsAsAlways.As_always_succeeds _ ->
        "This `as` assertion will always succeed and hence is redundant. The expression on the left has a type %s which is a subtype of %s."
      | Typing_warning.IsAsAlways.As_always_fails ->
        "This `as` assertion will always fail and lead to an exception at runtime. The expression on the left has type %s which shares no values with %s.")
      (Markdown_lite.md_codify lhs_ty)
      (Markdown_lite.md_codify rhs_ty)

  let reasons _ = []

  let lint_code { Typing_warning.IsAsAlways.kind; _ } =
    match kind with
    | Typing_warning.IsAsAlways.Is_is_always_true ->
      Lints_codes.Codes.is_always_true
    | Typing_warning.IsAsAlways.Is_is_always_false ->
      Lints_codes.Codes.is_always_false
    | Typing_warning.IsAsAlways.As_always_succeeds _ ->
      Lints_codes.Codes.as_always_succeeds
    | Typing_warning.IsAsAlways.As_always_fails ->
      Lints_codes.Codes.as_always_fails

  let lint_quickfix { Typing_warning.IsAsAlways.kind; _ } =
    match kind with
    | Typing_warning.IsAsAlways.Is_is_always_true
    | Typing_warning.IsAsAlways.Is_is_always_false
    | Typing_warning.IsAsAlways.As_always_fails ->
      None
    | Typing_warning.IsAsAlways.As_always_succeeds quickfix -> Some quickfix

  let quickfixes _ = []
end

module SketchyNullCheck = struct
  type t = Typing_warning.SketchyNullCheck.t

  let code = Codes.SketchyNullCheck

  let lint_code _ = Lints_codes.Codes.sketchy_null_check

  let claim { Typing_warning.SketchyNullCheck.name; kind } =
    let name = Option.value name ~default:"$x" in
    "This is a sketchy null check.\nIt detects nulls, but it will also detect many other falsy values, including `false`, `0`, `0.0`, `\"\"`, `\"0\"`, empty Containers, and more.\nIf you want to test for them, please consider doing so explicitly.\nIf you only meant to test for `null`, "
    ^
    match kind with
    | Typing_warning.SketchyNullCheck.Coalesce ->
      Printf.sprintf
        "use `%s ?? $default` instead of `%s ?: $default`"
        name
        name
    | Typing_warning.SketchyNullCheck.Eq ->
      Printf.sprintf "use `%s is null` instead" name
    | Typing_warning.SketchyNullCheck.Neq ->
      Printf.sprintf "use `%s is nonnull` instead" name

  let reasons _ = []

  let quickfixes _ = []

  let lint_quickfix _ = None
end

module NonDisjointCheck = struct
  type t = Typing_warning.NonDisjointCheck.t

  let code = Codes.NonDisjointCheck

  let lint_code _ = Lints_codes.Codes.invalid_disjointness_check

  let claim { Typing_warning.NonDisjointCheck.name; ty1; ty2; dynamic } =
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

let module_of (type a x) (kind : (x, a) Typing_warning.kind) :
    (module Warning with type t = x) =
  match kind with
  | Typing_warning.Sketchy_equality -> (module SketchyEquality)
  | Typing_warning.Is_as_always -> (module IsAsAlways)
  | Typing_warning.Sketchy_null_check -> (module SketchyNullCheck)
  | Typing_warning.Non_disjoint_check -> (module NonDisjointCheck)

let module_of_migrated
    (type x) (kind : (x, Typing_warning.migrated) Typing_warning.kind) :
    (module Lint with type t = x) =
  match kind with
  | Typing_warning.Is_as_always -> (module IsAsAlways)
  | Typing_warning.Sketchy_null_check -> (module SketchyNullCheck)
  | Typing_warning.Non_disjoint_check -> (module NonDisjointCheck)

let code_is_enabled tcopt code =
  match TypecheckerOptions.hack_warnings tcopt with
  | GlobalOptions.All -> true
  | GlobalOptions.ASome codes ->
    List.mem codes (Codes.to_enum code) ~equal:Int.equal

let add tcopt (type a x) ((pos, kind, warning) : (x, a) Typing_warning.t) : unit
    =
  let (module M) = module_of kind in
  if code_is_enabled tcopt M.code then
    Errors.add_error
      {
        User_error.severity;
        code = Codes.to_enum M.code;
        claim = (pos, M.claim warning);
        reasons = M.reasons warning;
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
      Lints_core.Lint_warning
      pos
      (M.claim warning)

let add env warning = add (Typing_env.get_tcopt env) warning

let code (type a x) (kind : (x, a) Typing_warning.kind) : Error_codes.Warning.t
    =
  let (module M) = module_of kind in
  M.code
