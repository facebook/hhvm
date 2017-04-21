(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(*****************************************************************************)
(* Module used to type DynamicYield
 * Each class that uses the DynamicYield trait or which extends a class that
 * uses the DynamicYield trait implicitly defines a few methods. If it
 * explicitly defines a yieldFoo method, then it implicitly also defines genFoo
 * (unless this method is explicitly defined).
 * It does this with __call().
 *)
(*****************************************************************************)
module Reason = Typing_reason
module SN     = Naming_special_names

let is_dynamic_yield name = (name = SN.FB.cDynamicYield)

let clean_dynamic_yield methods =
  SMap.filter begin
    fun name _ -> name <> Naming_special_names.Members.__call
  end methods
