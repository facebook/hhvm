(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

module B = Buffer
module A = Ast
module SS = String_sequence
module SU = Hhbc_string_utils
module TV = Typed_value

let rec adata_argument_to_string argument =
  match argument with
  | TV.Uninit -> SS.str "uninit"
  | TV.Null -> SS.str "N;"
  | TV.Float f -> SS.str @@ Printf.sprintf "d:%.14f;" f
  | TV.String s -> SS.str @@
    Printf.sprintf "s:%d:%s;" (String.length s) (SU.quote_string_with_escape s)
  (* TODO: The False case seems to sometimes be b:0 and sometimes i:0.  Why? *)
  | TV.Bool false -> SS.str "b:0;"
  | TV.Bool true -> SS.str "b:1;"
  | TV.Int i -> SS.str @@ "i:" ^ (Int64.to_string i) ^ ";"
  | TV.Array pairs -> adata_dict_collection_argument_to_string "a" pairs
  | TV.Vec values -> adata_collection_argument_to_string "v" values
  | TV.Dict pairs -> adata_dict_collection_argument_to_string "D" pairs
  | TV.Keyset values -> adata_collection_argument_to_string "k" values

and adata_dict_collection_argument_to_string col_type pairs =
  let num = List.length pairs in
  let fields = List.concat (Core.List.map pairs (fun (v1, v2) -> [v1;v2])) in
  let fields_str = adata_arguments_to_string fields in
  SS.gather [
    SS.str @@ Printf.sprintf "%s:%d:{" col_type num;
    fields_str;
    SS.str "}"
  ]

and adata_collection_argument_to_string col_type fields =
  let fields_str = adata_arguments_to_string fields in
  let num = List.length fields in
  SS.gather [
    SS.str @@ Printf.sprintf "%s:%d:{" col_type num;
    fields_str;
    SS.str "}"
  ]

and adata_arguments_to_string arguments =
  arguments
    |> Core.List.map ~f:adata_argument_to_string
    |> SS.gather


let adata_to_string_helper ~has_keys ~if_class_attribute name args =
  let count = List.length args in
  let count =
    if not has_keys then count
    else
      (if count mod 2 = 0 then count / 2
      else failwith
        "adata string with keys should have even amount of arguments")
  in
  let arguments = adata_arguments_to_string args in
  let adata_str = format_of_string @@
    if if_class_attribute
    then "\"%s\"(\"\"\"a:%n:{"
    else "\"\"\"%s:%n:{"
  in
  let adata_begin = Printf.sprintf adata_str name count in
  let adata_end =
    if if_class_attribute
    then "}\"\"\")"
    else "}\"\"\""
  in
  SS.gather [
    SS.str adata_begin;
    arguments;
    SS.str adata_end;
  ]

let attribute_to_string a =
  let name = Hhas_attribute.name a in
  let args = Hhas_attribute.arguments a in
  SS.seq_to_string @@
  adata_to_string_helper ~has_keys:true ~if_class_attribute:true name args
