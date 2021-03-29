(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let make_for_decl : type a. Pos.t * a -> Decl_reference.t -> Pos_or_decl.t * a =
 (fun (p, x) decl -> (Pos_or_decl.make_decl_pos p decl, x))

let make_for_decl_of_option :
    type a. Pos.t * a -> Decl_reference.t option -> Pos_or_decl.t * a =
 (fun (p, x) decl -> (Pos_or_decl.make_decl_pos_of_option p decl, x))

let unsafe_to_raw_positioned : type a. Pos_or_decl.t * a -> Pos.t * a =
 (fun (p, x) -> (Pos_or_decl.unsafe_to_raw_pos p, x))

let of_raw_positioned (p, x) = (Pos_or_decl.of_raw_pos p, x)
