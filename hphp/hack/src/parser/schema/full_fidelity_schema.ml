(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* If you make changes to the schema that cause it to serialize / deserialize
   differently, please update this version number *)
let full_fidelity_schema_version_number = "2024-01-31-0000"

(* TODO: Consider basing the version number on an auto-generated
   hash of a file rather than relying on people remembering to update it. *)
(* TODO: It may be worthwhile to investigate how Thrift describes data types
   and use that standard. *)

include Operator_schema_definition
include Token_schema_definition
include Schema_definition

let schema_map =
  let add map ({ kind_name; _ } as schema_node) =
    SMap.add kind_name schema_node map
  in
  List.fold_left add SMap.empty
  @@ {
       kind_name = "Token";
       type_name = "Token.t";
       func_name = "token";
       description = "token";
       prefix = "token";
       aggregates = [Expression; Name];
       fields = [];
     }
     :: {
          kind_name = "error";
          type_name = "error";
          func_name = "error";
          description = "error";
          prefix = "error";
          aggregates = [];
          fields = [];
        }
     :: schema

type trivia_node = {
  trivia_kind: string;
  trivia_text: string;
}

type transformation = {
  pattern: string;
  func: schema_node -> string;
}

type token_transformation = {
  token_pattern: string;
  token_func: token_node list -> string;
}

type trivia_transformation = {
  trivia_pattern: string;
  trivia_func: trivia_node list -> string;
}

type aggregate_transformation = {
  aggregate_pattern: string;
  aggregate_func: aggregate_type -> string;
}

type operator_transformation = {
  operator_pattern: string;
  operator_func: operator_node list -> string;
}

type template_file = {
  filename: string;
  template: string;
  transformations: transformation list;
  token_transformations: token_transformation list;
  token_no_text_transformations: token_transformation list;
  token_variable_text_transformations: token_transformation list;
  token_given_text_transformations: token_transformation list;
  trivia_transformations: trivia_transformation list;
  aggregate_transformations: aggregate_transformation list;
  operator_transformations: operator_transformation list;
}

let make_template_file
    ?(transformations = [])
    ?(token_transformations = [])
    ?(token_no_text_transformations = [])
    ?(token_given_text_transformations = [])
    ?(token_variable_text_transformations = [])
    ?(trivia_transformations = [])
    ?(aggregate_transformations = [])
    ?(operator_transformations = [])
    ~filename
    ~template
    () =
  {
    filename;
    template;
    transformations;
    token_transformations;
    token_no_text_transformations;
    token_given_text_transformations;
    token_variable_text_transformations;
    trivia_transformations;
    aggregate_transformations;
    operator_transformations;
  }

module Language_flags = struct
  let php_and_hack = "php_and_hack"

  let hack_only = "hack_only"

  let is_hack_only : string -> bool = function
    | "php_and_hack" -> false
    | "hack_only" -> true
    | f -> failwith ("Unknown language flag " ^ f ^ " for token.")
end

module LF = Language_flags

module Optional_flags = struct
  let xhp = "xhp"

  (* See documentation of token_node.allowed_as_identifier. *)
  let allowed_as_identifier = "allowed_as_identifier"

  let is_recognized : string -> bool = function
    | "xhp"
    | "allowed_as_identifier" ->
      true
    | _ -> false

  let is_xhp : string list -> bool = (fun flags -> List.mem xhp flags)

  let is_allowed_as_identifier : string list -> bool = function
    | flags -> List.mem allowed_as_identifier flags
end

module OF = Optional_flags

let trivia_node_from_list l =
  match l with
  | [trivia_kind; trivia_text] -> { trivia_kind; trivia_text }
  | _ -> failwith "bad trivia schema"

let trivia_kinds =
  List.map
    trivia_node_from_list
    [
      ["WhiteSpace"; "whitespace"];
      ["EndOfLine"; "end_of_line"];
      ["DelimitedComment"; "delimited_comment"];
      ["SingleLineComment"; "single_line_comment"];
      ["FixMe"; "fix_me"];
      ["IgnoreError"; "ignore_error"];
      ["FallThrough"; "fall_through"];
      ["ExtraTokenError"; "extra_token_error"];
    ]

let escape_token_text t =
  (* add one extra backslash because
     it is removed by Str.replace_first downstream *)
  if t = "\\" then
    "\\\\\\"
  else
    t

let map_and_concat_separated separator f items =
  String.concat separator (List.map f items)

let map_and_concat f items = map_and_concat_separated "" f items

let filter_map_concat p f items = map_and_concat f (List.filter p items)

let transform_schema f = map_and_concat f schema

let transform_aggregate f = map_and_concat f generated_aggregate_types

let replace pattern new_text source =
  Str.replace_first (Str.regexp pattern) new_text source

let generate_string template =
  let syntax_folder s x = replace x.pattern (transform_schema x.func) s in
  let tokens_folder token_list s x =
    replace x.token_pattern (x.token_func token_list) s
  in
  let trivia_folder trivia_list s x =
    replace x.trivia_pattern (x.trivia_func trivia_list) s
  in
  let aggregate_folder s x =
    replace x.aggregate_pattern (transform_aggregate x.aggregate_func) s
  in
  let operator_folder operators s x =
    replace x.operator_pattern (x.operator_func operators) s
  in
  let result =
    List.fold_left syntax_folder template.template template.transformations
  in
  let result =
    List.fold_left (tokens_folder tokens) result template.token_transformations
  in
  let result =
    List.fold_left
      (tokens_folder no_text_tokens)
      result
      template.token_no_text_transformations
  in
  let result =
    List.fold_left
      (tokens_folder given_text_tokens)
      result
      template.token_given_text_transformations
  in
  let result =
    List.fold_left
      (tokens_folder variable_text_tokens)
      result
      template.token_variable_text_transformations
  in
  let result =
    List.fold_left
      (trivia_folder trivia_kinds)
      result
      template.trivia_transformations
  in
  let result =
    List.fold_left
      (operator_folder operators)
      result
      template.operator_transformations
  in
  let result =
    List.fold_left aggregate_folder result template.aggregate_transformations
  in
  result

let format_ocaml src path : string =
  (* Write the string to a temporary file. *)
  let tmp_filename = Filename.temp_file "" (Filename.basename path) in

  let file = Core.Out_channel.create tmp_filename in
  Printf.fprintf file "%s" src;
  Core.Out_channel.close file;

  let ocamlformat_path =
    Option.value
      (Stdlib.Sys.getenv_opt "OCAMLFORMAT_PATH")
      ~default:"../tools/third-party/ocamlformat/ocamlformat"
  in

  (* Run ocamlformat on the file. *)
  let cmd =
    Printf.sprintf "%s -i --name=%s %s" ocamlformat_path path tmp_filename
  in
  ignore (Sys.command cmd);

  (* Read the formatted file, then delete it. *)
  let res =
    Core.In_channel.with_file tmp_filename ~f:(fun channel ->
        Core.In_channel.input_all channel)
  in
  Sys.remove tmp_filename;
  res

let generate_formatted_string (template : template_file) : string =
  let open Core in
  let s = generate_string template in
  let has_suffix s = String.is_suffix template.filename ~suffix:s in
  if has_suffix ".ml" || has_suffix ".mli" then
    format_ocaml s template.filename
  else
    s

let generate_file (template : template_file) : unit =
  let open Core in
  let filename = template.filename in
  let file = Out_channel.create filename in
  let s = generate_formatted_string template in
  Printf.fprintf file "%s" s;
  Out_channel.close file

module GenerateFFJSONSchema = struct
  let to_json_trivia { trivia_kind; trivia_text } =
    Printf.sprintf
      "    { \"trivia_kind_name\" : \"%s\",
      \"trivia_type_name\" : \"%s\" }"
      trivia_kind
      trivia_text

  let to_json_given_text x =
    Printf.sprintf
      "    { \"token_kind\" : \"%s\",
      \"token_text\" : \"%s\" },
"
      x.token_kind
      (escape_token_text x.token_text)

  let to_json_variable_text x =
    Printf.sprintf
      "    { \"token_kind\" : \"%s\",
      \"token_text\" : null },
"
      x.token_kind

  let to_json_ast_nodes x =
    let mapper (f, _) = Printf.sprintf "{ \"field_name\" : \"%s\" }" f in
    let fields = String.concat ",\n        " (List.map mapper x.fields) in
    Printf.sprintf
      "    { \"kind_name\" : \"%s\",
      \"type_name\" : \"%s\",
      \"description\" : \"%s\",
      \"prefix\" : \"%s\",
      \"fields\" : [
        %s
      ] },
"
      x.kind_name
      x.type_name
      x.description
      x.prefix
      fields

  let full_fidelity_json_schema_template =
    "{ \"description\" :
  \"@"
    ^ "generated JSON schema of the Hack Full Fidelity Parser AST\",
  \"version\" : \""
    ^ full_fidelity_schema_version_number
    ^ "\",
  \"trivia\" : [
TRIVIA_KINDS ],
  \"tokens\" : [
GIVEN_TEXT_TOKENS
VARIABLE_TEXT_TOKENS
    { \"token_kind\" : \"EndOfFile\",
      \"token_text\" : null } ],
  \"AST\" : [
AST_NODES
    { \"kind_name\" : \"Token\",
      \"type_name\" : \"token\",
      \"description\" : \"token\",
      \"prefix\" : \"\",
      \"fields\" : [
        { \"field_name\" : \"leading\" },
        { \"field_name\" : \"trailing\" } ] },
    { \"kind_name\" : \"Missing\",
      \"type_name\" : \"missing\",
      \"description\" : \"missing\",
      \"prefix\" : \"\",
      \"fields\" : [ ] },
    { \"kind_name\" : \"SyntaxList\",
      \"type_name\" : \"syntax_list\",
      \"description\" : \"syntax_list\",
      \"prefix\" : \"\",
      \"fields\" : [ ] } ] }"

  let full_fidelity_json_schema =
    make_template_file
      ~transformations:[{ pattern = "AST_NODES"; func = to_json_ast_nodes }]
      ~token_given_text_transformations:
        [
          {
            token_pattern = "GIVEN_TEXT_TOKENS";
            token_func = map_and_concat to_json_given_text;
          };
        ]
      ~token_variable_text_transformations:
        [
          {
            token_pattern = "VARIABLE_TEXT_TOKENS";
            token_func = map_and_concat to_json_variable_text;
          };
        ]
      ~trivia_transformations:
        [
          {
            trivia_pattern = "TRIVIA_KINDS";
            trivia_func = map_and_concat_separated ",\n" to_json_trivia;
          };
        ]
      ~template:full_fidelity_json_schema_template
      ~filename:"hphp/hack/src/parser/js/full_fidelity_schema.json"
      ()
end

let schema_as_json () =
  generate_string GenerateFFJSONSchema.full_fidelity_json_schema
