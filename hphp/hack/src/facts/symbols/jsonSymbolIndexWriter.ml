(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Hh_json
open IndexBuilderTypes
open SearchUtils

let record_one_jsonfile
    (path : string) (predicate : string) (filenum : int) (elems : json list) :
    string =
  (* Ensure path exists *)
  begin
    try Unix.mkdir path 0o777 with _ -> ()
  end;

  (* Open the file *)
  let filename =
    Printf.sprintf "%s/%s.%s.json" path predicate (string_of_int filenum)
  in
  (* Write all results in json format *)
  let json_export =
    JSON_Array
      [
        JSON_Object
          [("predicate", JSON_String predicate); ("facts", JSON_Array elems)];
      ]
  in
  (* Close the json file *)
  let json_str = json_to_string ~pretty:true json_export in
  let channel = Out_channel.create filename in
  Out_channel.output_string channel json_str;
  Out_channel.close channel;

  (* Here's your file *)
  filename

(* Save a list of symbols into a range of json files split into chunk_size *)
let record_in_jsonfiles
    (chunk_size : int) (path : string) (symbols : si_scan_result) : string list
    =
  (* Note that glean elements must start with high ID numbers *)
  let json_element_id = ref 500_000 in
  (* Revised schema tables *)
  let hackSymbols =
    List.map symbols.sisr_capture ~f:(fun symbol ->
        incr json_element_id;
        let (ns, _) = Utils.split_ns_from_name symbol.sif_name in
        let nsid = Caml.Hashtbl.find symbols.sisr_namespaces ns in
        let hash =
          Caml.Hashtbl.find symbols.sisr_filepaths symbol.sif_filepath
        in
        JSON_Object
          [
            ( "key",
              JSON_Object
                [
                  ( "name_lowercase",
                    JSON_String (String.lowercase symbol.sif_name) );
                  ( "valid",
                    JSON_Object
                      [
                        ("acid", JSON_Bool (valid_for_acid symbol));
                        ("acnew", JSON_Bool (valid_for_acnew symbol));
                        ("actype", JSON_Bool (valid_for_actype symbol));
                      ] );
                  ( "kind_id",
                    JSON_Number (string_of_int (kind_to_int symbol.sif_kind)) );
                  ("ns_id", JSON_Number (string_of_int nsid));
                  ("filehash_id", JSON_String (Int64.to_string hash));
                  ("is_abstract", JSON_Bool (valid_for_actype symbol));
                  ("is_final", JSON_Bool (valid_for_actype symbol));
                  ("canonical_name", JSON_String symbol.sif_name);
                ] );
            ("id", JSON_Number (string_of_int !json_element_id));
          ])
  in
  (* Save schema 1 files by chunk size *)
  let array_chunk_list = List.chunks_of hackSymbols ~length:chunk_size in
  let sym_files =
    List.mapi array_chunk_list ~f:(fun chunk_id chunk ->
        record_one_jsonfile path "hack.symbol.1" chunk_id chunk)
  in
  (* Namespace schema tables *)
  let hackNamespaces =
    Caml.Hashtbl.fold
      (fun ns id acc ->
        incr json_element_id;
        JSON_Object
          [
            ( "key",
              JSON_Object
                [
                  ("namespace_id", JSON_Number (string_of_int id));
                  ("namespace_name", JSON_String ns);
                ] );
            ("id", JSON_Number (string_of_int !json_element_id));
          ]
        :: acc)
      symbols.sisr_namespaces
      []
  in
  (* Save schema 1 files by chunk size *)
  let array_chunk_list = List.chunks_of hackNamespaces ~length:chunk_size in
  let ns_files =
    List.mapi array_chunk_list ~f:(fun chunk_id chunk ->
        record_one_jsonfile path "hack.symbolNamespace.1" chunk_id chunk)
  in
  (* Filename schema tables *)
  let hackFilepaths =
    Caml.Hashtbl.fold
      (fun path hash acc ->
        incr json_element_id;
        JSON_Object
          [
            ( "key",
              JSON_Object
                [
                  ("filename", JSON_String path);
                  ("filehash_id", JSON_String (Int64.to_string hash));
                ] );
            ("id", JSON_Number (string_of_int !json_element_id));
          ]
        :: acc)
      symbols.sisr_filepaths
      []
  in
  (* Save schema 1 files by chunk size *)
  let array_chunk_list = List.chunks_of hackFilepaths ~length:chunk_size in
  let filepath_files =
    List.mapi array_chunk_list ~f:(fun chunk_id chunk ->
        record_one_jsonfile path "hack.filename.1" chunk_id chunk)
  in
  (* Save kind information in a single file *)
  let kind_fn = Printf.sprintf "%s/hack.kind.1.0.json" path in
  let data =
    "[{\"predicate\":\"hack.kind.1\",\"facts\":["
    ^ "{\"key\":{\"id\":1,\"name\":\"Class\"},\"id\":999901},"
    ^ "{\"key\":{\"id\":2,\"name\":\"Interface\"},\"id\":999902},"
    ^ "{\"key\":{\"id\":3,\"name\":\"Enum\"},\"id\":999903},"
    ^ "{\"key\":{\"id\":4,\"name\":\"Trait\"},\"id\":999904},"
    ^ "{\"key\":{\"id\":5,\"name\":\"Unknown\"},\"id\":999905},"
    ^ "{\"key\":{\"id\":6,\"name\":\"Mixed\"},\"id\":999906},"
    ^ "{\"key\":{\"id\":7,\"name\":\"Function\"},\"id\":999907},"
    ^ "{\"key\":{\"id\":8,\"name\":\"Typedef\"},\"id\":999908},"
    ^ "{\"key\":{\"id\":9,\"name\":\"GlobalConstant\"},\"id\":999909},"
    ^ "{\"key\":{\"id\":10,\"name\":\"XHP\"},\"id\":999910}]}]"
  in
  Out_channel.write_all kind_fn ~data;

  (* Final results *)
  let results = List.append sym_files ns_files |> List.append filepath_files in
  kind_fn :: results
