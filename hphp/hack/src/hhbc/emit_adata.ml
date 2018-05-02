(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

module B = Buffer
module SU = Hhbc_string_utils
module TV = Typed_value
module TVMap = Typed_value.TVMap
open Hhbc_ast
open Hh_core

let hack_arr_dv_arrs () =
  Hhbc_options.hack_arr_dv_arrs !Hhbc_options.compiler_options

let adata_array_prefix = "a"

let adata_varray_prefix = "y"

let adata_vec_prefix = "v"

let adata_dict_prefix = "D"

let adata_darray_prefix = "Y"

let adata_keyset_prefix = "k"

let adata_mapped_argument_to_buffer b col_type pairs f =
  let num = List.length pairs in
  Printf.bprintf b "%s:%d:{" col_type num;
  Core_list.iter pairs ~f:(f b);
  B.add_string b "}"

let rec adata_to_buffer b argument =
  match argument with
  | TV.Uninit -> B.add_string b "uninit"
  | TV.Null -> B.add_string b "N;"
  | TV.Float f -> Printf.bprintf b "d:%s;" (SU.Float.to_string f)
  | TV.String s ->
    Printf.bprintf b "s:%d:%s;" (String.length s) (SU.quote_string_with_escape s)
  (* TODO: The False case seems to sometimes be b:0 and sometimes i:0.  Why? *)
  | TV.Bool false -> B.add_string b "b:0;"
  | TV.Bool true -> B.add_string b "b:1;"
  | TV.Int i -> B.add_string b @@ "i:" ^ (Int64.to_string i) ^ ";"
  | TV.HhasAdata data -> B.add_string b data
  | TV.Array pairs ->
      adata_dict_collection_argument_to_buffer b adata_array_prefix pairs
  | TV.VArray values ->
      adata_collection_argument_to_buffer b adata_varray_prefix values
  | TV.Vec values ->
      adata_collection_argument_to_buffer b adata_vec_prefix values
  | TV.Dict pairs ->
      adata_dict_collection_argument_to_buffer b adata_dict_prefix pairs
  | TV.DArray pairs ->
      adata_dict_collection_argument_to_buffer b adata_darray_prefix pairs
  | TV.Keyset values ->
      adata_collection_argument_to_buffer b adata_keyset_prefix values

and adata_dict_collection_argument_to_buffer b col_type pairs =
  adata_mapped_argument_to_buffer b col_type pairs begin fun b (v1, v2) ->
    adata_to_buffer b v1;
    adata_to_buffer b v2;
  end

and adata_collection_argument_to_buffer b col_type fields =
  adata_mapped_argument_to_buffer b col_type fields adata_to_buffer

let attribute_to_string a =
  let name = Hhas_attribute.name a in
  let args = Hhas_attribute.arguments a in
  let b = B.create 16 in
  Printf.bprintf b "\"%s\"(\"\"\"a:%n:{" name (List.length args / 2);
  List.iter args ~f:(adata_to_buffer b);
  B.add_string b "}\"\"\")";
  B.contents b

let attributes_to_strings al =
  let al = List.sort
    (fun a1 a2 -> String.compare (Hhas_attribute.name a1)
      (Hhas_attribute.name a2)) al in
  (* Adjust for underscore coming before alphabet *)
  let with_underscores, no_underscores =
    Hh_core.List.partition_map
      ~f:(fun x ->
        let has_underscore = String_utils.string_starts_with (Hhas_attribute.name x) "__" in
        let str = attribute_to_string x in
        if has_underscore then `Fst str else `Snd str)
      al
  in
  Core_list.append with_underscores no_underscores

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

let reset () =
  array_identifier_counter := 0;
  adata := [];
  array_identifier_map := TVMap.empty

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
  | TV.HhasAdata "" -> failwith "HhasAdata may not be empty"
  | TV.HhasAdata d ->
    let identifier = get_array_identifier tv in
    begin match String.sub d 0 1 with
    | s when s = adata_array_prefix -> Array identifier
    | s when s = adata_varray_prefix && hack_arr_dv_arrs () -> Vec identifier
    | s when s = adata_varray_prefix -> Array identifier
    | s when s = adata_darray_prefix && hack_arr_dv_arrs () -> Dict identifier
    | s when s = adata_darray_prefix -> Array identifier
    | s when s = adata_vec_prefix -> Vec identifier
    | s when s = adata_keyset_prefix -> Keyset identifier
    | s when s = adata_dict_prefix -> Dict identifier
    | _ -> failwith ("Unknown HhasAdata data: " ^ d)
    end
  | TV.Array _ -> Array (get_array_identifier tv)
  | TV.VArray _ when hack_arr_dv_arrs () -> Vec (get_array_identifier tv)
  | TV.VArray _ -> Array (get_array_identifier tv)
  | TV.DArray _ when hack_arr_dv_arrs () -> Dict (get_array_identifier tv)
  | TV.DArray _ -> Array (get_array_identifier tv)
  | TV.Vec _ -> Vec (get_array_identifier tv)
  | TV.Keyset _ -> Keyset (get_array_identifier tv)
  | TV.Dict _ -> Dict (get_array_identifier tv)
  )

let get_adata () = List.rev !adata
