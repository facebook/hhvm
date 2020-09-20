(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let intentional_raise b =
  let _ = Printf.sprintf "%s" "no inlining" in
  if b then failwith "intentional raise";
  let _ = Printf.sprintf "%s" "no inlining" in
  ()
