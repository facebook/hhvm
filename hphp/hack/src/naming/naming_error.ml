(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


open Utils

let alok (pos, x) =
  error pos ("You probably forgot to bind this type parameter right?\nAdd <"^
               x^"> somewhere (after the function name definition, or after the class name)\nExamples: "^
              "function foo<T> or class A<T>")

(* This is in the case where somebody tries to write
   class A<T> { public static T $x; }
   it is not allowed for now, because of type erasure
*)
let generic_class_var (pos, _) =
  error pos "A class variable cannot be generic"

let too_many_args pos =
  error pos "Too many arguments"

let unexpected_arrow (pos, cname) =
  error pos ("Keys may not be specified for " ^ cname ^ " initialization")

let missing_arrow (pos, cname) =
  error pos ("Keys must be specified for " ^ cname ^ " initialization")

let disallowed_xhp_type (pos, name) =
  error pos (name ^ " is not a valid type. Use :xhp or XHPChild.")
