(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast

let mem x xs =
  List.exists xs ~f:(fun { ua_name; _ } -> String.equal x (snd ua_name))

let mem2 x1 x2 xs =
  List.exists xs ~f:(fun { ua_name = (_, n); _ } ->
      String.equal x1 n || String.equal x2 n)

let find x xs =
  List.find xs ~f:(fun { ua_name; _ } -> String.equal x (snd ua_name))

let find2 x1 x2 xs =
  List.find xs ~f:(fun { ua_name = (_, n); _ } ->
      String.equal x1 n || String.equal x2 n)

let mem_pos x xs =
  let attr = find x xs in
  match attr with
  | Some { ua_name = (pos, _); _ } -> Some pos
  | None -> None
