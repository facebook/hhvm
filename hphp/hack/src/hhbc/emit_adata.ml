(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module Acc = Mutable_accumulator
module SU = Hhbc_string_utils
module TV = Typed_value
module TVMap = Typed_value.TVMap
open Hhbc_ast

let hack_arr_dv_arrs () =
  Hhbc_options.hack_arr_dv_arrs !Hhbc_options.compiler_options

let arrprov_enabled () =
  Hhbc_options.array_provenance !Hhbc_options.compiler_options

let adata_array_prefix = "a"

let adata_varray_prefix = "y"

let adata_vec_prefix = "v"

let adata_dict_prefix = "D"

let adata_darray_prefix = "Y"

let adata_keyset_prefix = "k"

let pos_to_prov_tag loc =
  if arrprov_enabled () then
    match loc with
    | Some loc' ->
      let (line, _, _) = Pos.info_pos loc' in
      let filename =
        match Relative_path.to_absolute @@ Pos.filename loc' with
        | "" -> "(unknown hackc filename)"
        | x -> x
      in
      Printf.sprintf
        "p:i:%d;s:%d:%s;"
        line
        (String.length filename)
        (SU.quote_string_with_escape filename)
    | None -> ""
  else
    ""

let adata_mapped_argument_to_buffer b col_type loc pairs f =
  let num = List.length pairs in
  let prov_str = pos_to_prov_tag loc in
  Printf.sprintf "%s:%d:{%s" col_type num prov_str |> Acc.add b;
  List.iter pairs ~f:(f b);
  Acc.add b "}"

let rec adata_to_buffer b argument =
  match argument with
  | TV.Uninit -> Acc.add b "uninit"
  | TV.Null -> Acc.add b "N;"
  | TV.Float f -> Printf.sprintf "d:%s;" (SU.Float.to_string f) |> Acc.add b
  | TV.String s ->
    Printf.sprintf "s:%d:%s;" (String.length s) (SU.quote_string_with_escape s)
    |> Acc.add b
  (* TODO: The False case seems to sometimes be b:0 and sometimes i:0.  Why? *)
  | TV.Bool false -> Acc.add b "b:0;"
  | TV.Bool true -> Acc.add b "b:1;"
  | TV.Int i -> Acc.add b @@ "i:" ^ Int64.to_string i ^ ";"
  | TV.HhasAdata data -> Acc.add b (String.escaped data)
  | TV.Array pairs ->
    adata_dict_collection_argument_to_buffer b adata_array_prefix None pairs
  | TV.VArray (values, loc) ->
    adata_collection_argument_to_buffer b adata_varray_prefix loc values
  | TV.Vec (values, loc) ->
    adata_collection_argument_to_buffer b adata_vec_prefix loc values
  | TV.Dict (pairs, loc) ->
    adata_dict_collection_argument_to_buffer b adata_dict_prefix loc pairs
  | TV.DArray (pairs, loc) ->
    adata_dict_collection_argument_to_buffer b adata_darray_prefix loc pairs
  | TV.Keyset values ->
    adata_collection_argument_to_buffer b adata_keyset_prefix None values

and adata_dict_collection_argument_to_buffer b col_type loc pairs =
  adata_mapped_argument_to_buffer b col_type loc pairs (fun b (v1, v2) ->
      adata_to_buffer b v1;
      adata_to_buffer b v2)

and adata_collection_argument_to_buffer b col_type loc fields =
  adata_mapped_argument_to_buffer b col_type loc fields adata_to_buffer

let attribute_to_string a =
  let name = Hhas_attribute.name a in
  let args = Hhas_attribute.arguments a in
  let b = Acc.create () in
  Printf.sprintf
    "\"%s\"(\"\"\"%s:%n:{"
    name
    adata_varray_prefix
    (List.length args)
  |> Acc.add b;
  List.iter args ~f:(adata_to_buffer b);
  Acc.add b "}\"\"\")";
  String.concat ~sep:"" (Acc.segments b)

let attributes_to_strings al =
  let al =
    List.sort
      (fun a1 a2 ->
        String.compare (Hhas_attribute.name a1) (Hhas_attribute.name a2))
      al
  in
  (* Adjust for underscore coming before alphabet *)
  let (with_underscores, no_underscores) =
    List.partition_map
      ~f:(fun x ->
        let has_underscore =
          String_utils.string_starts_with (Hhas_attribute.name x) "__"
        in
        let str = attribute_to_string x in
        if has_underscore then
          `Fst str
        else
          `Snd str)
      al
  in
  List.append with_underscores no_underscores

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
  (* If we're logging for array provenance, we musn't merge identical adatas *)
  if arrprov_enabled () then
    next_adata_id tv
  else
    match TVMap.find_opt tv !array_identifier_map with
    | None ->
      let id = next_adata_id tv in
      array_identifier_map := TVMap.add tv id !array_identifier_map;
      id
    | Some id -> id

let reset () =
  array_identifier_counter := 0;
  adata := [];
  array_identifier_map := TVMap.empty

let rewrite_typed_value tv =
  ILitConst
    (match tv with
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
      begin
        match String.sub d 0 1 with
        | s when s = adata_array_prefix -> Array identifier
        | s when s = adata_varray_prefix && hack_arr_dv_arrs () ->
          Vec identifier
        | s when s = adata_varray_prefix -> Array identifier
        | s when s = adata_darray_prefix && hack_arr_dv_arrs () ->
          Dict identifier
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
    | TV.Dict _ -> Dict (get_array_identifier tv))

let rewrite_tv instruction =
  match instruction with
  | ILitConst (TypedValue tv) -> rewrite_typed_value tv
  | _ -> instruction

let rewrite_typed_values instrseq =
  Instruction_sequence.InstrSeq.map instrseq rewrite_tv

let get_adata () = List.rev !adata
