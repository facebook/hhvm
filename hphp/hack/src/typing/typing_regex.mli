(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

exception Missing_delimiter

val type_pattern : Nast.expr -> Typing_defs.Reason.t * Typing_defs.locl Typing_defs.ty_
