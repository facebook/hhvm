(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)
module SN = Naming_special_names

let quote_string s = "\"" ^ Php_escaping.escape s ^ "\""
let quote_string_with_escape ?(f = Php_escaping.escape_char) s =
  "\\\"" ^ Php_escaping.escape ~f s ^ "\\\""
let single_quote_string_with_escape ?(f = Php_escaping.escape_char) s =
  "'" ^ Php_escaping.escape ~f s ^ "'"
let triple_quote_string s = "\"\"\"" ^ Php_escaping.escape s ^ "\"\"\""

let prefix_namespace n s = n ^ "\\" ^ s
let strip_global_ns s =
  if String.length s > 0 || s.[0] = '\\'
  then String_utils.lstrip s "\\"
  else s
let strip_ns =
  let rx = Str.regexp {|.*\\|} in
  (* strip zero or more chars followed by a backslash *)
  fun s -> Str.replace_first rx "" s
let has_ns =
  let rx = Str.regexp {|.+\\.+|} in
  fun s -> Str.string_match rx s 0
let strip_type_list =
  let rx = Str.regexp {|<.*>|} in
  fun s -> Str.global_replace rx "" s

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

let is_self s =
  String.lowercase_ascii s = SN.Classes.cSelf

let is_parent s =
  String.lowercase_ascii s = SN.Classes.cParent

let is_static s =
  String.lowercase_ascii s = SN.Classes.cStatic

let is_class s =
  String.lowercase_ascii s = SN.Members.mClass

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

  (* In order for this to be true, every char has to be a number as well as
   * if the first digit is a zero, then there cannot be more digits
   * negative zero is dealt specially as it is not casted to zero
   *
   * E.g.:
   * -1 -> true
   * -0 -> false (special case)
   * 08 -> false (octal)
   * 0b1 -> false (binary)
   *
   **)
  let is_decimal_int s = if s = "-0" then false else
    let s = String_utils.lstrip s "-" in
    String_utils.fold_left s
      ~acc:true
      ~f:(fun acc i -> String_utils.is_decimal_digit i && acc)
    && (String.length s = 1 || (s.[0] <> '0'))
end

module Float = struct
  let to_string f =
    match Printf.sprintf "%0.17g" f with
    | "nan" -> "NAN"
    | "inf" -> "INF"
    | "-inf" -> "-INF"
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

module Classes = struct

  let mangle_class prefix scope ix =
    prefix ^ "$"
    ^ scope
    ^ (if ix = 1 then "" else "#" ^ string_of_int ix)

  (* Anonymous classes have names of the form
   *   class@anonymous$ scope ix ; num
   * where
   *   scope  ::=
   *     <function-name>
   *   | <class-name> :: <method-name>
   *   |
   *   ix ::=
   *     # <digits>
   *)
  let mangle_anonymous_class scope ix =
    mangle_class "class@anonymous" scope ix

  let is_anonymous_class_name n =
    String_utils.string_starts_with n "class@anonymous"
end

module Closures = struct

  let is_closure_name s =
    String_utils.string_starts_with s "Closure$"
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
  let unmangle_closure =
    let rx = Str.regexp "#" in
    fun s ->
      if is_closure_name s
      then
        let suffix = String_utils.lstrip s "Closure$" in
        match Str.split_delim rx suffix with
        | [prefix; _] -> Some prefix
        | _ -> Some suffix
      else None

  let mangle_closure scope ix =
    Classes.mangle_class "Closure" scope ix
end

(* XHP name mangling *)
module Xhp = struct
  let is_xhp s = String.length s <> 0 && s.[0] = ':'

  let strip_colon s = String_utils.lstrip s ":"

  let clean s = if not (is_xhp s) then s else strip_colon s

  let ignore_id s =
    Classes.is_anonymous_class_name s || Closures.is_closure_name s

  (* Mangle an unqualified ID *)
  let mangle_id_worker =
    let rx_colon = Str.regexp ":" in
    let rx_dash = Str.regexp "-" in
    fun s ->
      if ignore_id s then s
      else
      let need_prefix = is_xhp s in
      let s = if need_prefix then (strip_colon s) else s in
      let s =
        s
        |> Str.global_replace rx_colon "__"
        |> Str.global_replace rx_dash "_" in
      if need_prefix then "xhp_" ^ s else s

  let mangle_id s =
    if ignore_id s then s else mangle_id_worker s

  (* Mangle a possibly-qualified ID *)
  let mangle =
    let rx = Str.regexp "\\" in
    fun s ->
      if ignore_id s then s
      else
      match List.rev (Str.split_delim rx s) with
      | [] -> ""
      | id::rest ->
        String.concat "\\" (List.rev (mangle_id_worker id :: rest))

  let unmangle_id_worker =
    let rx_dunder = Str.regexp "__" in
    let rx_under = Str.regexp "_" in
    fun s ->
      let has_prefix = String_utils.string_starts_with s "xhp_" in
      let s = if has_prefix then String_utils.lstrip s "xhp_" else s in
      let s =
        s
        |> Str.global_replace rx_dunder ":"
        |>Str.global_replace rx_under "-" in
      if has_prefix then ":" ^ s else s

  let unmangle =
    let rx = Str.regexp "\\" in
    fun s ->
      if ignore_id s then s
      else
      match List.rev (Str.split_delim rx s) with
      | [] -> ""
      | id::rest ->
        String.concat "\\" (List.rev (unmangle_id_worker id :: rest))

end
