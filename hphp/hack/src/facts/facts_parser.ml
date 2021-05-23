(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Facts

let mangle_xhp_mode = ref true

let flags_abstract = 1

let flags_final = 2

let extract_as_json_string
    ~(php5_compat_mode : bool)
    ~(hhvm_compat_mode : bool)
    ~(disable_nontoplevel_declarations : bool)
    ~(disable_legacy_soft_typehints : bool)
    ~(allow_new_attribute_syntax : bool)
    ~(disable_legacy_attribute_syntax : bool)
    ~(enable_xhp_class_modifier : bool)
    ~(disable_xhp_element_mangling : bool)
    ~(disallow_hash_comments : bool)
    ~(filename : Relative_path.t)
    ~(text : string) =
  (* return empty string if file has syntax errors *)
  let bool2int b =
    if b then
      1
    else
      0
  in
  ignore @@ disable_nontoplevel_declarations;
  ignore @@ disable_legacy_soft_typehints;
  ignore @@ disable_legacy_attribute_syntax;
  Rust_facts_ffi.extract_as_json_ffi
    ( (bool2int php5_compat_mode lsl 0)
    lor (bool2int hhvm_compat_mode lsl 1)
    lor (bool2int allow_new_attribute_syntax lsl 2)
    lor (bool2int enable_xhp_class_modifier lsl 3)
    lor (bool2int disable_xhp_element_mangling lsl 4)
    lor (bool2int disallow_hash_comments lsl 5) )
    filename
    text
    !mangle_xhp_mode
  |> Option.map ~f:(fun unnormalized ->
         (* make it compact (same line breaks and whitespace) via Hh_json *)
         (* to avoid differences in output (because many tests rely on it!) *)
         unnormalized |> Hh_json.json_of_string |> Hh_json.json_to_multiline)

let from_text
    ~(php5_compat_mode : bool)
    ~(hhvm_compat_mode : bool)
    ~(disable_nontoplevel_declarations : bool)
    ~(disable_legacy_soft_typehints : bool)
    ~(allow_new_attribute_syntax : bool)
    ~(disable_legacy_attribute_syntax : bool)
    ~(enable_xhp_class_modifier : bool)
    ~(disable_xhp_element_mangling : bool)
    ~(disallow_hash_comments : bool)
    ~(filename : Relative_path.t)
    ~(text : string) =
  Option.bind
    ( extract_as_json_string
        ~php5_compat_mode
        ~hhvm_compat_mode
        ~disable_nontoplevel_declarations
        ~disable_legacy_soft_typehints
        ~allow_new_attribute_syntax
        ~disable_legacy_attribute_syntax
        ~enable_xhp_class_modifier
        ~disable_xhp_element_mangling
        ~disallow_hash_comments
        ~filename
        ~text
    |> Option.map ~f:Hh_json.json_of_string )
    facts_from_json
