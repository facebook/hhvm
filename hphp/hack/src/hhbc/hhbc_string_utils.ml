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

let cmp ?(case_sensitive=true) ?(ignore_ns=false) s1 s2 =
  let s1, s2 =
    if case_sensitive then s1, s2 else
    String.lowercase_ascii s1, String.lowercase_ascii s2
  in
  let s1, s2 =
    if not ignore_ns then s1, s2 else
    strip_ns s1, strip_ns s2
  in
  s1 = s2


module Types = struct
  let fix_casing s = match String.lowercase_ascii s with
    | "vector" -> "Vector"
    | "immvector" -> "ImmVector"
    | "set" -> "Set"
    | "immset" -> "ImmSet"
    | "map" -> "Map"
    | "immmap" -> "ImmMap"
    | "pair" -> "Pair"
    | _ -> s
end

(* Integers are represented as strings *)
module Integer = struct
  (* Dont accidentally convert 0 to 0o *)
  let to_decimal s = Int64.to_string @@ Int64.of_string @@
    if String.length s > 1 && s.[0] = '0' then
    match s.[1] with
    (* Binary *)
    | 'b' | 'B'
    (* Hex *)
    | 'x' | 'X' -> s
    (* Octal *)
    | _ -> "0o" ^ String_utils.lstrip s "0"
    else s
end

module Float = struct
  let to_string f =
    match Printf.sprintf "%0.17g" f with
    | "nan" -> "NAN"
    | "inf" -> "INF"
    | s -> s

  (* Unfortunately the g flag does not provide enough of a match with hhvm,
   * hence we go for manual manipulation *)
  let with_scientific_notation f =
    if String.contains f 'E' || String.contains f 'e'
    then Printf.sprintf "%0.1E" (float_of_string f)
    else f
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
      match Str.split_delim (Str.regexp ";") suffix with
      | [prefix; _count] ->
        begin match Str.split_delim (Str.regexp "#") prefix with
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

  let is_xhp s = String.length s <> 0 && s.[0] = ':'

  let strip_colon s = String_utils.lstrip s ":"

  let clean s = if not (is_xhp s) then s else strip_colon s

  (* Mangle an unqualified ID *)
  let mangle_id s =
    if not (is_xhp s) then s else
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
