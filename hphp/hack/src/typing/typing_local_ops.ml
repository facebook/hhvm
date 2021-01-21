(*
 * Copyright (c) 2020, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs_core
open Typing_env_types
module Env = Typing_env

let check_local_capability (mk_required : env -> env * locl_ty) callback env =
  (* gate the check behavior on coeffects TC option *)
  if TypecheckerOptions.local_coeffects (Env.get_tcopt env) then
    let available = Env.get_local env Typing_coeffects.local_capability_id in
    let (env, required) = mk_required env in
    Typing_subtype.sub_type_or_fail env available required (fun () ->
        callback available required)
  else
    env

let enforce_local_capability
    ?((* Run-time enforced ops must have the default as it's unfixmeable *)
    err_code = Error_codes.Typing.err_code Error_codes.Typing.OpCoeffects)
    (mk_required : env -> env * locl_ty)
    (op : string)
    (op_pos : Pos.t)
    env =
  check_local_capability
    mk_required
    (fun available required ->
      Errors.op_coeffect_error
        ~locally_available:(Typing_print.coeffects env available)
        ~available_pos:(Typing_defs.get_pos available)
        ~required:(Typing_print.coeffects env required)
        ~err_code
        op
        op_pos)
    env

module Capabilities = struct
  module Reason = Typing_reason
  include SN.Capabilities

  let mk special_name env =
    let r = Reason.none in
    Typing_make_type.apply r (Reason.to_pos r, special_name) []
    |> Typing_phase.localize_with_self env
end

let enforce_static_property_access =
  enforce_local_capability
    Capabilities.(mk accessStaticVariable)
    "Static property access"

let enforce_io =
  enforce_local_capability Capabilities.(mk io) "`echo` or `print` builtin"

let enforce_rx_is_enabled =
  enforce_local_capability Capabilities.(mk rx) ("`" ^ SN.Rx.is_enabled ^ "`")

let enforce_awaitable_immediately_awaited =
  enforce_local_capability
    Capabilities.(mk accessStaticVariable)
    "Not immediately `await`ing `Awaitable`-typed values"
