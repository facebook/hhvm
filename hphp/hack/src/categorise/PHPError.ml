(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open ScubaQuery

module FF = Full_fidelity_syntax_tree

exception Malformed_stack_trace_line of string
exception Malformed_result_row of string
exception Column_not_found of string
exception File_not_found of string

type call_pattern =
  | Not_defined of string
  | Top_level
  | Strict_call_strict
  | Nonstrict_call_nonstrict
  | Strict_call_nonstrict
  | Nonstrict_call_strict

type line_offset  = int
type fix_me_type  = string
type soft_type    = string
type lib_function = string
type parent_class = string

type stack_trace_line = {
  filepath      : string;
  line_number   : int;
  function_name : string;
}

type stack = string list
type formatted_stack = stack_trace_line list

(* The filename in here should be absolute *)
type php_error = {
  time         : int;
  error_file   : string;
  error_line   : int;
  error_string : string;
  error_type   : int;
  stack        : formatted_stack;
}

type analysis = {
  php_error      : php_error;
  fix_me         : bool * line_offset * fix_me_type;
  soft_type_hint : bool * line_offset * soft_type;
  php_lib_call   : bool * line_offset * lib_function;
  override       : bool * line_offset * parent_class;
  call_pattern   : call_pattern;
}

(* type bucket_results = {
  total : int;
  fix_me_count : int;
  soft_type_hint_count : int;
  php_lib_call_count : int;
  override_count : int;
  top_level_count : int;
  strict_call_strict_count : int;
  nonstrict_call_nonstrict_count : int;
  strict_call_nonstrict_count : int;
  nonstrict_call_strict_count : int;
} *)

type bucket_results = int list


(*******************************)
(******** Find WWW root ********)
(*******************************)

let www_root = ref ""

let rec guess_root start recursion_limit : Path.t option =
  if start = Path.parent start then None (* Reach fs root, nothing to do. *)
  else if Wwwroot.is_www_directory start then Some start
  else if recursion_limit <= 0 then None
  else let dir = Path.concat start @@
    Array.fold_left
      (fun acc x -> if Pcre.pmatch ~rex:(Pcre.regexp "www") x then x else acc)
      ""
      (Sys.readdir (Path.to_string start)) in
    if Wwwroot.is_www_directory dir (* find www even if we're in fbcode *)
    then Some dir
    else guess_root (Path.parent start) (recursion_limit - 1)

let get_root () =
  let start_path = Path.make "." in
  let root = match guess_root start_path 50 with
    | None -> start_path
    | Some r -> r in
  Wwwroot.assert_www_directory root;
  (* Relative_path.(set_path_prefix Root root); *)
  root

let set_www_root () =
  www_root := (Path.to_string (get_root ())) ^ "/"


 (**********************************)
 (******** Helper functions ********)
 (**********************************)

let stl_to_string { filepath = p; line_number = n; function_name = f; } =
  Printf.sprintf "file: %s , line: %d , func: %s" p n f

(* returns index of first column matching the name *)
let get_column_index name columns =
  match Core.List.findi columns (fun _ x -> x = name) with
    | None        -> raise (Column_not_found name)
    | Some (i, _) -> i

let string_of_table_entry table_entry = match table_entry with
  | NormEntry str -> str
  | NormVectorEntry strs -> "[" ^ (String.concat ", " strs) ^ "]"

let legible = Str.replace_first (Str.regexp_string !www_root) ""

let empty_php_error = {
  time         = 0;
  error_file   = "";
  error_line   = 0;
  error_string = "";
  error_type   = 0;
  stack        = [];
}

let empty_analysis = {
  php_error      = empty_php_error;
  fix_me         = (false, 0, "");
  soft_type_hint = (false, 0, "");
  php_lib_call   = (false, 0, "");
  override       = (false, 0, "");
  call_pattern   = Not_defined "";
}

(* let empty_bucket_results = {
  total = 0;
  fix_me_count = 0;
  soft_type_hint_count = 0;
  php_lib_call_count = 0;
  override_count = 0;
  top_level_count = 0;
  strict_call_strict_count = 0;
  nonstrict_call_nonstrict_count = 0;
  strict_call_nonstrict_count = 0;
  nonstrict_call_strict_count = 0;
} *)

let bucket_column_titles =
  [ "bucket"
  ; "total"
  ; "fix_mes"
  ; "soft_type_hints"
  ; "php_lib_calls"
  ; "overrides"
  ; "top_level_call"
  ; "strict_call_strict"
  ; "nonstrict_call_nonstrict"
  ; "strict_call_nonstrict"
  ; "nonstrict_call_strict" ]

let empty_bucket_results =
  [0; 0; 0; 0; 0; 0; 0; 0; 0; 0;]


 (********************************************)
 (****** Query scuba and format results ******)
 (********************************************)

let new_php_error_query () =
  new_query "php_errors" |>
  add_columns ["time"; "error_file"; "error_line"; "error_string"; "error_type"; "stack"]

let extract_stack index row : stack =
  match List.nth row index with
    | NormVectorEntry s -> s
    | _ -> raise (Malformed_result_row "stack column does not contain vectors")

let split_stack (stack : stack) : formatted_stack =
  let format_stack_line l =
    let split_stack_line = Pcre.split ~pat:("@") ~max:(3) l in
    match split_stack_line with
      | [ filepath; line_number; function_name; ] ->
        let line_number =
          try int_of_string line_number with
          | Failure s when s = "int_of_string" ->
            raise (Malformed_stack_trace_line l) in
        let filepath = Filename.concat !www_root filepath in
        { filepath; line_number; function_name; }
      | _ -> raise (Malformed_stack_trace_line l) in
  List.map format_stack_line stack

let format_stack_trace row index =
  let stack = extract_stack index row in
  split_stack stack

let format_results column_names rows : php_error list =
  let time_index  = get_column_index "time" column_names in
  let file_index  = get_column_index "error_file" column_names in
  let line_index  = get_column_index "error_line" column_names in
  let str_index   = get_column_index "error_string" column_names in
  let type_index  = get_column_index "error_type" column_names in
  let stack_index = get_column_index "stack" column_names in
  let format_result r =
    let error_file = string_of_table_entry (List.nth r file_index) in
    let error_file = Filename.concat !www_root error_file in
    let error_string = string_of_table_entry (List.nth r str_index) in
    let stack = format_stack_trace r stack_index in
    try
      let time = int_of_string (string_of_table_entry (List.nth r time_index)) in
      let error_line = int_of_string (string_of_table_entry (List.nth r line_index)) in
      let error_type = int_of_string (string_of_table_entry (List.nth r type_index)) in
      { time; error_file; error_line; error_string; error_type; stack; }
    with
      | _ -> let str = String.concat " ; " (List.map string_of_table_entry r) in
             raise (Malformed_result_row str) in
  List.map format_result rows


 (*************************************)
 (***** File parsing and analysis *****)
 (*************************************)

let parse_file path =
  let content =
    try Sys_utils.cat path
    with
      _ -> let e = Printf.sprintf "File \"%s\" does not exist" (legible path) in
           raise (File_not_found e) in
  let source_text = Full_fidelity_source_text.make content in
  FF.make source_text

let get_mode file =
  let tree = parse_file file in
  let mode = FF.mode tree in
  if mode = "" then "non-strict" else mode

let print_mode ?prefix:(prefix="") file =
  try
    let mode = get_mode file in
    Printf.printf "%sFile %s written in mode %s\n" prefix (legible file) mode
  with
    | File_not_found s -> Printf.printf "%sError: %s\n" prefix s

let list_stack_trace_by_mode ({ error_string; stack; _ } : php_error) =
  let print_stack_line_mode =
    fun { filepath; _ } -> print_mode ~prefix:("  ") filepath in
  Printf.printf "ERROR STRING: \"%s\"\n" error_string;
  print_endline "  CALL TRACE:";
  List.iter print_stack_line_mode stack

let analyse_fix_me _ = (false, 0, "")            (** TODO **)
let analyse_soft_type_hint _ = (false, 0, "")    (** TODO **)
let analyse_php_lib_call _ = (false, 0, "")      (** TODO **)
let analyse_override _ = (false, 0, "")          (** TODO **)

let analyse_call_pattern { stack; _ } = match stack with
  | [] -> Not_defined "empty stack"
  | { filepath = error_file; _ } :: { filepath = called_from; _ } :: _ ->
    (match get_mode error_file, get_mode called_from with
      | "strict"    , "strict"     -> Strict_call_strict
      | "strict"    , "non-strict" -> Nonstrict_call_strict
      | "non-strict", "strict"     -> Strict_call_nonstrict
      | "non-strict", "non-strict" -> Nonstrict_call_nonstrict
      | ef, cf -> Not_defined (Printf.sprintf "%s called from %s" ef cf)
      | exception File_not_found s -> Not_defined s)
  | _ -> Top_level

let analyse_error (php_error : php_error) : analysis =
  let fix_me = analyse_fix_me php_error in
  let soft_type_hint = analyse_soft_type_hint php_error in
  let php_lib_call = analyse_php_lib_call php_error in
  let override = analyse_override php_error in
  let call_pattern = analyse_call_pattern php_error in
  { php_error; fix_me; soft_type_hint; php_lib_call; override; call_pattern }


 (*************************************)
 (********** Error bucketing **********)
 (*************************************)

let get_error_templates = function
  | None -> []
  | Some filepath ->
    let handle = open_in filepath in
    let rec read_templates templates =
      try
        let templates = input_line handle :: templates in
        read_templates templates
      with
        | End_of_file -> close_in handle; List.rev templates in
    read_templates []

let get_fix_me_incr { fix_me = (check,_,_); _ } =
  if check then 1 else 0
let get_soft_type_hint_incr { soft_type_hint = (check,_,_); _ } =
  if check then 1 else 0
let get_php_lib_call_incr { php_lib_call = (check,_,_); _ } =
  if check then 1 else 0
let get_override_incr { override = (check,_,_); _ } =
  if check then 1 else 0
let get_call_pattern_incr { call_pattern; _ } =
  match call_pattern with
    | Not_defined _            -> [0; 0; 0; 0; 0]
    | Top_level                -> [1; 0; 0; 0; 0]
    | Strict_call_strict       -> [0; 1; 0; 0; 0]
    | Nonstrict_call_nonstrict -> [0; 0; 1; 0; 0]
    | Strict_call_nonstrict    -> [0; 0; 0; 1; 0]
    | Nonstrict_call_strict    -> [0; 0; 0; 0; 1]

let augment_results results analysis =
  let incr_funcs =
    [ get_fix_me_incr
    ; get_soft_type_hint_incr
    ; get_php_lib_call_incr
    ; get_override_incr ] in
  let incrs = List.map (fun f -> f analysis) incr_funcs in
  let incrs = 1 :: incrs @ (get_call_pattern_incr analysis) in
  List.map2 (+) incrs results

let matches_template str template =
  Str.string_match (Str.regexp template) str 0

(* To do the bucketing, we first build a list of buckets
   (template, results pairs). We then iterate over the list of analysed
   errors, finding the first bucket whose template matches the error string and
   augmenting the totals in that bucket (the templates should be exclusive,
   so no need to iterate over the whole list in every case). *)
let rec bucket_error buckets (analysis : analysis) =
  let { php_error = { error_string; _ }; _ } = analysis in
  match buckets with
    | (t, results) :: buckets ->
      begin
        if matches_template error_string t
        then (t, augment_results results analysis) :: buckets
        else (t, results) :: bucket_error buckets analysis
      end
    | [] -> [(error_string, augment_results empty_bucket_results analysis)]

let bucket_by_error analysed_errors templates =
  let buckets = List.map (fun t -> t, empty_bucket_results) templates in
  List.fold_left bucket_error buckets analysed_errors

let format_data bucketed =
  let buffer = Buffer.create 512 in
  let string_of_bucket (template, results) =
    String.concat "," @@ template :: List.map string_of_int results in
  Buffer.add_string buffer @@ (String.concat "," bucket_column_titles  ^ "\n");
  List.iter (fun b -> Buffer.add_string buffer @@ (string_of_bucket b) ^ "\n") bucketed;
  buffer

let analyse_and_bucket ?stats_out templs days errors =
    let now = int_of_float (Unix.time ()) in
    let day_length = 60*60*24 in
    let cns, rows =
      new_php_error_query ()
      |> add_filter (normfilter "error_string" Contains
        [ "must be of type"
        ; "must implement interface"
        ; "must be an instance of" ])
      |> add_time_range (now - days * day_length, now)
      |> add_order_by ("time", Desc)
      |> add_limit errors
      |> query_scuba in
    let php_errors = format_results cns rows in
    let analysed_errors = List.map analyse_error php_errors in
    let err_templates = get_error_templates templs in
    let bucketed = bucket_by_error analysed_errors err_templates in
    let buffer = format_data bucketed in
    let handle = Option.value_map ~default:stdout ~f:open_out stats_out in
    Buffer.output_buffer handle buffer;
    close_out handle
