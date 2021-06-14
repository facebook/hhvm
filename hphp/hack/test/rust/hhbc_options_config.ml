(*
 * Copyright (c) 2020, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let usage =
  Printf.sprintf
    {|USAGE:
Parses a series of JSONs, each terminated by a NUL character (\x00):

  { "obj_or_emptystrs": [{...}, "", ...],
    "cli_keyval_args": ["key1=val1", "key2=val2", ...]] }

then parses each into the string lists that are inputs to functions
  Hhbc_options.apply_config_overrides_statelessly
and
  Hhbc_options.from_configs_rust
and finally verifies that the Rust implementation (the latter) behaves
equivalently -- i.e., produces the same Hhbc_options.t -- as the
OCaml implementation (the former) of the config-combining logic.

(The typical usage is to run the Facebook-specific script `hhbc_options_log`
and pipe its output to this program, or redirect it from a temporary file.)

The program reports failures on stderr (and only exits with 0 if no failures).
It prints commands for diagnostic & debugging on stdout so that they can be
easily redirected to `bash` and inspected interectively.  E.g., for crashes
it allows inspection of pretty-printed inputs using jq & less,
while for differences it shows

Command-line options:|}

type opts = {
  interactive: bool ref;
  soft: bool ref;
  unordered_aliased_namespaces: bool ref;
}

let opts =
  {
    interactive = ref false;
    soft = ref false;
    unordered_aliased_namespaces = ref false;
  }

let () =
  Arg.parse
    [
      ( "--interactive",
        Arg.Set opts.interactive,
        "add `| less` to each command printed on stdout" );
      ( "--soft",
        Arg.Set opts.soft,
        "compare results of to_string (ignore representation of options)" );
      ( "--unordered-aliased-namespaces",
        Arg.Set opts.unordered_aliased_namespaces,
        "ignore JSON array element order for hhvm.aliased_namespaces" );
    ]
    (fun _ -> ())
    usage

exception UnexpectedConfigJsonsLayout

exception InvalidConfigElem

let check_cnt = ref 0

let tmp_prefix =
  String.concat
    ~sep:Filename.dir_sep
    [Sys_utils.temp_dir_name; "hhbc_options_configs."]

let check_configs (pair_json_str : string) : bool =
  let open Hh_json in
  match json_of_string pair_json_str with
  | JSON_Object
      [(json_key, JSON_Array json_vals); (keyval_key, JSON_Array keyval_vals)]
    when String.(
           equal json_key "obj_or_emptystrs"
           && equal keyval_key "cli_keyval_args") ->
    let jsons : string list =
      List.filter_map
        ~f:(function
          | JSON_Object _ as o -> Some (json_to_string o)
          | JSON_String s when String.is_empty s -> None
          | _ -> raise InvalidConfigElem)
        json_vals
    in
    let args : string list =
      List.map
        ~f:(function
          | JSON_String s -> s
          | _ -> raise InvalidConfigElem)
        keyval_vals
    in
    check_cnt := !check_cnt + 1;
    Printf.eprintf
      "CHECK #%d: |%s|=%d |%s|=%d\n"
      !check_cnt
      json_key
      (List.length jsons)
      keyval_key
      (List.length args);
    let (caml, fail) =
      Hhbc_options.apply_config_overrides_statelessly args jsons
    in
    if fail then begin
      Printf.eprintf "FAILED #%d\n" !check_cnt;
      let interact_suffix =
        if !(opts.interactive) then
          " | less"
        else
          ""
      in
      let caml =
        if !(opts.unordered_aliased_namespaces) then
          Hhbc_options.
            {
              caml with
              option_aliased_namespaces =
                canonical_aliased_namespaces caml.option_aliased_namespaces;
            }
        else
          caml
      in
      let caml_outfile = Printf.sprintf "%s%d.ocaml" tmp_prefix !check_cnt in
      let rust_outfile = Printf.sprintf "%s%d.rust" tmp_prefix !check_cnt in
      let caml_str = Hhbc_options.to_string caml in
      Sys_utils.write_file ~file:caml_outfile caml_str;
      let fail =
        try
          let rust_str =
            Hhbc_options.(to_string @@ from_configs_rust ~args ~jsons)
          in
          Sys_utils.write_file ~file:rust_outfile rust_str;
          if !(opts.soft) && String.equal rust_str caml_str then
            false
          else begin
            Printf.printf
              "diff '%s' '%s'%s\n"
              caml_outfile
              rust_outfile
              interact_suffix;
            true
          end
        with _ ->
          Out_channel.flush stderr;
          Printf.eprintf "RUST CRASHED #%d\n" !check_cnt;
          (* Record the inputs for easy inspection with less *)
          let file = Printf.sprintf "%s%d.configs" tmp_prefix !check_cnt in
          Sys_utils.write_file ~file pair_json_str;
          Printf.printf "jq . '%s'%s\n" file interact_suffix;
          true
      in
      Out_channel.flush stdout;
      not fail
    end else
      true
  | _ -> raise UnexpectedConfigJsonsLayout

let () =
  let input = Sys_utils.cat "/dev/stdin" in
  let input = String.sub input ~pos:0 ~len:(String.length input - 1) in
  let pairs = String.split_on_chars ~on:['\x00'] input in
  let total_cnt = List.length pairs in
  let pass_cnt =
    pairs
    |> List.fold_left ~init:0 ~f:(fun cnt s ->
           cnt
           +
           if check_configs s then
             1
           else
             0)
  in
  Printf.eprintf "PASSED %d/%d\n" pass_cnt total_cnt;
  exit
    ( if pass_cnt = total_cnt then
      0
    else
      1 )
