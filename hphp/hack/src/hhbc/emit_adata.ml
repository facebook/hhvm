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
module TVMap = Typed_value.TVMap
open Hhbc_ast
open Core

let rec adata_to_string_seq argument =
  match argument with
  | TV.Uninit -> SS.str "uninit"
  | TV.Null -> SS.str "N;"
  | TV.Float f -> SS.str @@ Printf.sprintf "d:%s;" (SU.Float.to_string f)
  | TV.String s -> SS.str @@
    Printf.sprintf "s:%d:%s;" (String.length s) (SU.quote_string_with_escape s)
  (* TODO: The False case seems to sometimes be b:0 and sometimes i:0.  Why? *)
  | TV.Bool false -> SS.str "b:0;"
  | TV.Bool true -> SS.str "b:1;"
  | TV.Int i -> SS.str @@ "i:" ^ (Int64.to_string i) ^ ";"
  | TV.Array pairs -> adata_dict_collection_argument_to_string_seq "a" pairs
  | TV.Vec values -> adata_collection_argument_to_string_seq "v" values
  | TV.Dict pairs -> adata_dict_collection_argument_to_string_seq "D" pairs
  | TV.Keyset values -> adata_collection_argument_to_string_seq "k" values

and adata_dict_collection_argument_to_string_seq col_type pairs =
  let num = List.length pairs in
  let fields = List.concat (Core.List.map pairs (fun (v1, v2) -> [v1;v2])) in
  let fields_str = adata_arguments_to_string_seq fields in
  SS.gather [
    SS.str @@ Printf.sprintf "%s:%d:{" col_type num;
    fields_str;
    SS.str "}"
  ]

and adata_collection_argument_to_string_seq col_type fields =
  let fields_str = adata_arguments_to_string_seq fields in
  let num = List.length fields in
  SS.gather [
    SS.str @@ Printf.sprintf "%s:%d:{" col_type num;
    fields_str;
    SS.str "}"
  ]

and adata_arguments_to_string_seq arguments =
  arguments
    |> Core.List.map ~f:adata_to_string_seq
    |> SS.gather

let attribute_to_string a =
  let name = Hhas_attribute.name a in
  let args = Hhas_attribute.arguments a in
  let arguments = adata_arguments_to_string_seq args in
  let adata_begin =
    Printf.sprintf "\"%s\"(\"\"\"a:%n:{" name (List.length args / 2) in
  let adata_end = "}\"\"\")" in
  SS.seq_to_string @@ SS.gather [
    SS.str adata_begin;
    arguments;
    SS.str adata_end;
  ]

let attributes_to_strings al =
  let al = List.sort
    (fun a1 a2 -> String.compare (Hhas_attribute.name a1)
      (Hhas_attribute.name a2)) al in
  (* Adjust for underscore coming before alphabet *)
  let with_underscores, no_underscores =
    Core.List.partition_tf
      ~f:(fun x -> String_utils.string_starts_with (Hhas_attribute.name x) "__")
      al
  in
  Core.List.map (with_underscores @ no_underscores) attribute_to_string


(* Array identifier map. Maintain list as well, in generated order *)
let array_identifier_counter = ref 0
let adata = ref []
let next_adata_id tv =
  let c = !array_identifier_counter in
  array_identifier_counter := c + 1;
  let id = "A_" ^ string_of_int c in
  adata := Hhas_adata.make id tv :: !adata;
  id

let array_identifier_map = ref TVMap.empty
let get_array_identifier tv =
  match TVMap.get tv !array_identifier_map with
  | None ->
    let id = next_adata_id tv in
    array_identifier_map := TVMap.add tv id !array_identifier_map;
    id
  | Some id ->
    id

let rewrite_typed_value tv =
  ILitConst (
  match tv with
  | TV.Uninit -> failwith "rewrite_typed_value: uninit"
  | TV.Null -> Null
  | TV.Bool false -> False
  | TV.Bool true -> True
  | TV.Int i -> Int i
  | TV.Float f -> Double (SU.Float.to_string f)
  | TV.String s -> String s
  | TV.Array _ -> Array (get_array_identifier tv)
  | TV.Vec _ -> Vec (get_array_identifier tv)
  | TV.Keyset _ -> Keyset (get_array_identifier tv)
  | TV.Dict _ -> Dict (get_array_identifier tv)
  )

let get_adata () = List.rev !adata
