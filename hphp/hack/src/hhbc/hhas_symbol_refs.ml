(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)


(* Data structure for keeping track of symbols (and includes) we encounter in
   the course of emitting bytecode for an AST. We split them into these four
   categories for the sake of HHVM, which has lookup function corresponding to
   each.
*)
type t =
{ includes: SSet.t
; constants: SSet.t
; functions: SSet.t
; classes: SSet.t
}
