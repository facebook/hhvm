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

module Locals = struct

  let strip_dollar s = String_utils.lstrip s "$"

end

(* XHP name mangling *)
module Xhp = struct

  let mangle s =
    if String.length s = 0 || s.[0] <> ':' then s else
      "xhp_" ^
        String_utils.lstrip s ":" |>
        Str.global_replace (Str.regexp ":") "__" |>
        Str.global_replace (Str.regexp "-") "_"

  let unmangle s =
    if String_utils.string_starts_with s "xhp_"
    then
      String_utils.lstrip s "xhp_" |>
      Str.global_replace (Str.regexp "__") ":" |>
      Str.global_replace (Str.regexp "_") "-"
    else s

end
