(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = int option [@@deriving show]

external drop_tree_positioned : t -> unit = "drop_tree_positioned"

let leaked_pointer = ref None

let free_leaked_pointer ?(warn = true) () =
  match !leaked_pointer with
  | Some pointer ->
    leaked_pointer := None;
    if warn then Hh_logger.log "Warning: freeing leaked Rust pointer";
    drop_tree_positioned pointer
  | None -> ()

let register_leaked_pointer pointer =
  begin
    match !leaked_pointer with
    | Some pointer ->
      leaked_pointer := None;
      Hh_logger.log
        "Warning: leaking second Rust pointer before using first one";
      drop_tree_positioned pointer
    | None -> ()
  end;
  leaked_pointer := Some pointer

let unregister_leaked_pointer pointer =
  begin
    match !leaked_pointer with
    | Some p when p != pointer ->
      leaked_pointer := None;
      Hh_logger.log "Warning: unregistering pointer that was not leaked before";
      drop_tree_positioned pointer
    | _ -> ()
  end;
  leaked_pointer := None
