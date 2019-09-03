(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Pretty_printing_library_sig

(* Functor to create the actual module containing defined functionality *)
module Make (C : DocCompare) : Library
