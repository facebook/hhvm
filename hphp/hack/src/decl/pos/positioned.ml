(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let make_for_decl : type a. Pos.t * a -> Decl_reference.t -> Pos_or_decl.t * a =
 (fun (p, x) decl -> (Pos_or_decl.make_decl_pos p decl, x))
