(*
 * Copyright (c) 2021, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs
module Env = Tast_env
module Cls = Decl_provider.Class

let get_name dty =
  match get_node dty with
  | Tapply ((_, name), []) -> Some name
  | _ -> None

(* Check that the base type of an enum or (enum class) is not an alias
 * to the enum being defined:
 *
 * enum Foo : Foo {}
 * enum Bar0 : Bar1 {}
 * enum Bar1 : Bar0 {}
 *
 * Such code would make HHVM fatal.
 *
 * Note that we have a similar check for enum/class constants themselves in
 * Cyclic_class_constant but it doesn't take into account the empty enums.
 *)
let find_cycle env class_name =
  (* Note w.r.t. Cyclic_class_constant:
   * Since `self` is not allowed (parsing error) in this position, we just
   * keep track of the hints we see.
   *)
  let rec spot_target seen current =
    let open Option in
    let enum_def = Env.get_enum env current in
    let enum_info = enum_def |> Decl_entry.to_option >>= Cls.enum_type in
    let te_base = enum_info >>= fun info -> get_name info.te_base in
    match te_base with
    | None -> None
    | Some base ->
      if SSet.mem base seen then
        Some seen
      else
        spot_target (SSet.add base seen) base
  in
  spot_target SSet.empty class_name

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_class_ env c =
      let (pos, c_name) = c.c_name in
      match find_cycle env c_name with
      | Some stack ->
        Typing_error_utils.add_typing_error
          ~env:(Env.tast_env_as_typing_env env)
          Typing_error.(primary @@ Primary.Cyclic_class_def { pos; stack })
      | None -> ()
  end
