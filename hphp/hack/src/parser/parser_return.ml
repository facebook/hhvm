(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type t = {
    file_mode  : FileInfo.mode option; (* None if PHP *)
    is_hh_file : bool;
    comments   : (Pos.t * Prim_defs.comment) list;
    ast        : Ast.program;
    content   : string;
  }
