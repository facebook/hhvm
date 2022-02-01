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
 * topmost types one per line, each of which contains arbitrary bytes
 * (not necessarily valid UTF-8, so be careful with reading it)
 *)
let encode_tys_as_stdout_lines tast =
  (object
     inherit [_] Aast.iter as _super

     method! on_'ex _en ty = Typing_ser_ffi.encode_ty_to_stdout ty
  end)
    #on_program
    ()
    tast
