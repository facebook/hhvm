(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module SN = Naming_special_names

module Env = struct
  let typed_open_shapes Naming_phase_env.{ typed_open_shapes; _ } =
    typed_open_shapes
end

(* `mixed...` is accepted regardless of the flag because it is semantically
   equivalent to plain `...` (the implicit unknown-fields type). Detect bare
   `Hmixed`, `Happly` forms produced before/around `Happly`-hint elaboration,
   and `supportdyn(mixed)` (introduced by implicit-sdt elaboration). *)
let rec is_mixed_hint (_, hint_) =
  match hint_ with
  | Aast.Hmixed -> true
  | Aast.Happly ((_, name), []) ->
    String.equal name SN.Typehints.mixed
    || String.equal name ("\\" ^ SN.Typehints.mixed)
    || String.equal name ("\\HH\\" ^ SN.Typehints.mixed)
  | Aast.Happly ((_, name), [inner])
    when String.equal name SN.Classes.cSupportDyn ->
    is_mixed_hint inner
  | _ -> false

let on_hint on_error hint ~ctx =
  let err_opt =
    if Env.typed_open_shapes ctx then
      None
    else
      match hint with
      | (_, Aast.Hshape { Aast.nsi_unknown_fields_type = Some h; _ })
        when not (is_mixed_hint h) ->
        let (pos, _) = h in
        Some
          (Naming_phase_error.naming
          @@ Naming_error.Typed_open_shape_disallowed pos)
      | _ -> None
  in
  Option.iter ~f:on_error err_opt;
  (ctx, Ok hint)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.{ id with on_ty_hint = Some (on_hint on_error) }
