(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@warning "-33"]

open Hh_prelude

[@@@warning "+33"]

(** Does preorder traversal of typed AST and serializes the
 * topmost types as JSONs, one per line.
 *)
let tast_to_json tast =
  (object
     inherit [_] Aast.iter as super

     method! on_'ex _en _ty =
       Printf.printf "<Ty";
       super#on_'ex _en _ty;
       Printf.printf ">\n"
  end)
    #on_program
    ()
    tast
