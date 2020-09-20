(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type comments = (Pos.t * Prim_defs.comment) list

type t = {
  file_mode: FileInfo.mode option;
  comments: comments;
  ast: Nast.program;
  content: string;
}
