(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

let quote_string s = "\"" ^ Php_escaping.escape s ^ "\""
let quote_string_with_escape s = "\\\"" ^ Php_escaping.escape s ^ "\\\""

let prefix_namespace n s = n ^ "\\" ^ s
let strip_global_ns s =
  if String.length s > 0 || s.[0] = '\\'
  then String_utils.lstrip s "\\"
  else s
let strip_ns s =
  (* strip zero or more chars followed by a backslash *)
  Str.replace_first (Str.regexp {|.*\\|}) "" s

module Float = struct
  let to_string f = Printf.sprintf "%0.17g" f
end

module Locals = struct

  let strip_dollar s = String_utils.lstrip s "$"

end

module Closures = struct
  (* Closure classes have names of the form
   *   Closure$ scope ix ; num
   * where
   *   scope  ::=
   *     <function-name>
   *   | <class-name> :: <method-name>
   *   |
   *   ix ::=
   *     # <digits>
   *)
  let unmangle_closure s =
    if String_utils.string_starts_with s "Closure$"
    then
      let suffix = String_utils.lstrip s "Closure$" in
      match String_utils.split ';' suffix with
      | [prefix; _count] ->
        begin match String_utils.split '#' prefix with
        | [prefix; _] -> Some prefix
        | _ -> Some prefix
        end
      | _ -> None
    else None

  let mangle_closure scope ix count =
    "Closure$"
    ^ scope
    ^ (if ix = 1 then "" else "#" ^ string_of_int ix)
    ^ ";" ^ string_of_int count

  let split_scope_name s = Str.split (Str.regexp "::") s
  let join_method class_name method_name = class_name ^ "::" ^ method_name
end

(* XHP name mangling *)
module Xhp = struct

  let clean s =
    if String.length s = 0 || s.[0] <> ':'
    then s else String_utils.lstrip s ":"

  (* Mangle an unqualified ID *)
  let mangle_id s =
    if String.length s = 0 || s.[0] <> ':' then s else
      "xhp_" ^
        String_utils.lstrip s ":" |>
        Str.global_replace (Str.regexp ":") "__" |>
        Str.global_replace (Str.regexp "-") "_"

  (* Mangle a possibly-qualified ID *)
  let mangle s =
    match List.rev (Str.split_delim (Str.regexp "\\") s) with
    | [] -> ""
    | id::rest ->
      String.concat "\\" (List.rev (mangle_id id :: rest))

  let unmangle_id s =
    if String_utils.string_starts_with s "xhp_"
    then
      ":" ^ String_utils.lstrip s "xhp_" |>
      Str.global_replace (Str.regexp "__") ":" |>
      Str.global_replace (Str.regexp "_") "-"
    else s

  let unmangle s =
    match List.rev (Str.split_delim (Str.regexp "\\") s) with
    | [] -> ""
    | id::rest ->
      String.concat "\\" (List.rev (unmangle_id id :: rest))

end
