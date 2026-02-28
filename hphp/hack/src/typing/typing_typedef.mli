(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** This is a helper for [Typing_toplevel.typedef_def]. Call that instead. *)
val typedef_def : Provider_context.t -> Nast.typedef -> Tast.typedef
