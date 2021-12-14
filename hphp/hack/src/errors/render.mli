(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
val strip_ns : string -> string

val verb_to_string : [< `extend | `implement | `use ] -> string

val vis_to_string : [< `internal | `private_ | `protected | `public ] -> string

val highlight_differences : string -> string -> string

val suggestion_message :
  ?modifier:string -> string -> string -> 'a -> 'a * string

val string_of_class_member_kind :
  [< `class_constant
  | `class_typeconst
  | `class_variable
  | `method_
  | `property
  | `static_method
  ] ->
  string

val pluralize_arguments : int -> string
