(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core
module Codes = Error_codes.Warning

type t = {
  code: Codes.t;
  claim: Pos.t Message.t Lazy.t;
  reasons: Pos_or_decl.t Message.t list Lazy.t;
  quickfixes: Pos.t Quickfix.t list;
}

let severity = User_error.Warning

let to_user_error (warning : t) : Errors.error =
  let { code; claim; reasons; quickfixes } = warning in
  {
    User_error.severity;
    code = Codes.to_enum code;
    claim = Lazy.force claim;
    reasons = Lazy.force reasons;
    quickfixes;
    flags = User_error_flags.empty;
    custom_msgs = [];
    is_fixmed = false;
  }

let sketchy_equality pos b left right left_trail right_trail =
  let typedef_trail_entry pos = (pos, "Typedef definition comes from here") in
  let claim =
    lazy
      (let b =
         Markdown_lite.md_codify
           (if b then
             "true"
           else
             "false")
       in
       let msg = Printf.sprintf "This expression is likely always %s" b in
       (pos, msg))
  and reasons =
    Lazy.(
      left >>= fun left ->
      right >>= fun right ->
      let left_trail = List.map left_trail ~f:typedef_trail_entry in
      let right_trail = List.map right_trail ~f:typedef_trail_entry in
      return (left @ left_trail @ right @ right_trail))
  in
  { code = Codes.SketchyEquality; claim; reasons; quickfixes = [] }

module IsAsAlways = struct
  let message { Typing_warning.IsAsAlways.kind; lhs_ty; rhs_ty } =
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

  let lint_code (kind : Typing_warning.IsAsAlways.kind) =
    match kind with
    | Typing_warning.IsAsAlways.Is_is_always_true ->
      Lints_codes.Codes.is_always_true
    | Typing_warning.IsAsAlways.Is_is_always_false ->
      Lints_codes.Codes.is_always_false
    | Typing_warning.IsAsAlways.As_always_succeeds _ ->
      Lints_codes.Codes.as_always_succeeds
    | Typing_warning.IsAsAlways.As_always_fails ->
      Lints_codes.Codes.as_always_fails

  let quickfix (kind : Typing_warning.IsAsAlways.kind) =
    match kind with
    | Typing_warning.IsAsAlways.Is_is_always_true
    | Typing_warning.IsAsAlways.Is_is_always_false
    | Typing_warning.IsAsAlways.As_always_fails ->
      None
    | Typing_warning.IsAsAlways.As_always_succeeds quickfix -> Some quickfix

  let warn pos x =
    {
      code = Codes.IsAsAlways;
      claim = lazy (pos, message x);
      reasons = lazy [];
      quickfixes = (* TODO @catg migrate quickfixes too. *) [];
    }
end

module Lint = struct
  let code (warning : Typing_warning.migrated Typing_warning.t_) : int =
    match warning with
    | Typing_warning.Is_as_always { Typing_warning.IsAsAlways.kind; _ } ->
      IsAsAlways.lint_code kind

  let message (warning : Typing_warning.migrated Typing_warning.t_) : string =
    match warning with
    | Typing_warning.Is_as_always x -> IsAsAlways.message x

  let quickfix (warning : Typing_warning.migrated Typing_warning.t_) =
    (match warning with
    | Typing_warning.Is_as_always { Typing_warning.IsAsAlways.kind; _ } ->
      IsAsAlways.quickfix kind)
    |> Option.map ~f:Lints_errors.quickfix
end

let to_error (type a) ((pos, warning) : a Typing_warning.t) : t =
  match warning with
  | Typing_warning.Sketchy_equality
      { result; left; right; left_trail; right_trail } ->
    sketchy_equality pos result left right left_trail right_trail
  | Typing_warning.Is_as_always x -> IsAsAlways.warn pos x

let code_is_enabled tcopt code =
  match TypecheckerOptions.hack_warnings tcopt with
  | GlobalOptions.All -> true
  | GlobalOptions.ASome codes ->
    List.mem codes (Codes.to_enum code) ~equal:Int.equal

let add tcopt (type a) (warning : a Typing_warning.t) : unit =
  let t = to_error warning in
  if code_is_enabled tcopt t.code then Errors.add_error @@ to_user_error t

let add_for_migration
    tcopt
    ~(as_lint : Tast.check_status option option)
    (warning : Typing_warning.migrated Typing_warning.t) : unit =
  match as_lint with
  | None -> add tcopt warning
  | Some check_status ->
    let (pos, warning) = warning in
    Lints_core.add
      ~autofix:(Lint.quickfix warning)
      (Lint.code warning)
      ~check_status
      Lints_core.Lint_warning
      pos
      (Lint.message warning)

let add env warning = add (Typing_env.get_tcopt env) warning
