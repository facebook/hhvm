(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Cmdliner
module Gen = Milner_generate

let type_prefix = "TYPE"

let subtype_prefix = "SUBTYPE"

let expr_prefix = "expr"

let placeholder_regexp prefix =
  Pcre.regexp ("(?<![A-Za-z0-9_])" ^ prefix ^ "#([0-9]+)")

let type_regexp = placeholder_regexp type_prefix

let subtype_regexp = placeholder_regexp subtype_prefix

let expr_regexp = placeholder_regexp expr_prefix

let matches template regexp =
  try Pcre.exec_all ~rex:regexp template |> Array.to_list with
  | Stdlib.Not_found -> []

let init_table contents regexp =
  let table = Hashtbl.create (module Int) in
  List.iter (matches contents regexp) ~f:(fun placeholder ->
      let key = (Pcre.get_substrings placeholder).(1) |> int_of_string in
      Hashtbl.set ~key ~data:() table);
  table

let context_types template regexp =
  let table = Hashtbl.create (module Int) in
  List.iter (matches template regexp) ~f:(fun context ->
      let contents = (Pcre.get_substrings context).(1) in
      init_table contents type_regexp
      |> Hashtbl.iter_keys ~f:(fun key -> Hashtbl.set ~key ~data:() table));
  table

let generate_tables
    ~verbose ~debug_pattern ~allow_unsafe_named_parameter_order template =
  let renv =
    Gen.ReadOnlyEnvironment.default
      ~verbose
      ~debug_pattern
      ~allow_unsafe_named_parameter_order
  in
  let alias_types =
    context_types
      template
      (Pcre.regexp
         "(?m)^\\s*(?:case\\s+type|newtype|type)\\s+[^;=]+=(\\s*[^;]+);")
  in
  let intersection_types =
    context_types
      template
      (Pcre.regexp "(?<![A-Za-z0-9_])(TYPE#[0-9]+(?:\\s*&\\s*TYPE#[0-9]+)+)")
  in
  let renv_for key =
    if Hashtbl.mem alias_types key then
      Gen.ReadOnlyEnvironment.for_alias renv
    else
      renv
  in
  let ty_table = init_table template type_regexp in
  let expr_table = init_table template expr_regexp in
  let subty_table = init_table template subtype_regexp in
  List.iter [expr_table; subty_table] ~f:(fun table ->
      Hashtbl.iter_keys table ~f:(fun key -> Hashtbl.set ty_table ~key ~data:()));
  let ty_table =
    Hashtbl.keys ty_table
    |> List.sort ~compare:Int.compare
    |> List.fold
         ~init:(Hashtbl.create (module Int))
         ~f:(fun table key ->
           let rec generate () =
             let (env, ty) =
               Gen.Type.mk (renv_for key) Gen.Environment.default
             in
             let compatible =
               (not (Hashtbl.mem intersection_types key))
               || Hashtbl.for_alli
                    table
                    ~f:(fun ~key:other ~data:(other_env, other_ty) ->
                      (not (Hashtbl.mem intersection_types other))
                      || Gen.Type.intersection_law_compatible
                           env
                           ty
                           other_env
                           other_ty)
             in
             if compatible then
               (env, ty)
             else
               generate ()
           in
           Hashtbl.set table ~key ~data:(generate ());
           table)
  in
  let subty_table =
    Hashtbl.mapi subty_table ~f:(fun ~key ~data:() ->
        let (env, ty) = Hashtbl.find_exn ty_table key in
        Gen.Type.subtype_of (renv_for key) env ty)
  in
  let expr_table =
    Hashtbl.mapi expr_table ~f:(fun ~key ~data:() ->
        let (env, ty) = Hashtbl.find_exn ty_table key in
        let (env, expression) = Gen.Type.inhabitant_of (renv_for key) env ty in
        Hashtbl.set ty_table ~key ~data:(env, ty);
        expression)
  in
  let defs =
    Hashtbl.to_alist ty_table
    |> List.sort ~compare:(fun (left, _) (right, _) -> Int.compare left right)
    |> List.concat_map ~f:(fun (_, (env, _)) -> Gen.Environment.definitions env)
    |> List.map ~f:Gen.Definition.show
  in
  let seen_defs = Hash_set.create (module String) in
  let defs =
    List.filter defs ~f:(fun definition ->
        if Hash_set.mem seen_defs definition then
          false
        else begin
          Hash_set.add seen_defs definition;
          true
        end)
  in
  let ty_table = Hashtbl.map ty_table ~f:(fun (_, ty) -> ty) in
  (defs, ty_table, subty_table, expr_table)

let fill_in_template ty_table subty_table expr_table template =
  let fill_table table ~prefix contents =
    let replace ~key ~data contents =
      let rex =
        Pcre.regexp
          ("(?<![A-Za-z0-9_])"
          ^ Pcre.quote (prefix ^ "#" ^ string_of_int key)
          ^ "(?![0-9])")
      in
      Pcre.substitute ~rex ~subst:(fun _ -> data) contents
    in
    Hashtbl.to_alist table
    |> List.sort ~compare:(fun (left, _) (right, _) -> Int.compare right left)
    |> List.fold ~init:contents ~f:(fun contents (key, data) ->
           replace ~key ~data contents)
  in
  let ty_str_table = Hashtbl.map ty_table ~f:Gen.Type.show in
  let subty_str_table = Hashtbl.map subty_table ~f:Gen.Type.show in
  template
  |> fill_table subty_str_table ~prefix:subtype_prefix
  |> fill_table ty_str_table ~prefix:type_prefix
  |> fill_table expr_table ~prefix:expr_prefix

let milner
    verbose
    debug_pattern
    allow_unsafe_named_parameter_order
    seed
    template_path
    destination_path =
  if verbose > 0 then begin
    Format.eprintf "Seed: %d\n" seed;
    Format.eprintf "Template: %s\n" template_path;
    Format.eprintf "Destination: %s\n"
    @@ Option.value ~default:"None" destination_path;
    Format.eprintf "\n";
    Out_channel.flush Out_channel.stderr
  end;
  let () = Random.init seed in
  let template = In_channel.read_all template_path in
  let (defs, ty_table, subty_table, expr_table) =
    generate_tables
      ~verbose
      ~debug_pattern
      ~allow_unsafe_named_parameter_order
      template
  in
  let output =
    fill_in_template ty_table subty_table expr_table template
    |> Milner_program.render ~definitions:defs
  in
  match destination_path with
  | Some path -> Out_channel.write_all path ~data:output
  | None -> Format.printf "%s" output

let verbose =
  let doc =
    "Print debugging information at a given verbosity level to STDERR"
  in
  Arg.(value & opt int 0 & info ["v"; "verbose"] ~docv:"LEVEL" ~doc)

let debug_pattern =
  let doc = "Fixed string used to activate debug logging" in
  Arg.(
    value & opt (some string) None & info ["debug-pattern"] ~docv:"PATTERN" ~doc)

let seed =
  let doc = "Seed for the random nubmer generator" in
  Arg.(value & opt int 0 & info ["s"; "seed"] ~docv:"SEED" ~doc)

let allow_unsafe_named_parameter_order =
  let doc =
    "Disable the T289079831 declaration-order guard for generated functions and methods"
  in
  Arg.(value & flag & info ["allow-unsafe-named-parameter-order"] ~doc)

let template =
  let doc = "Template file to generate well-typed programs from" in
  Arg.(required & pos 0 (some file) None & info [] ~docv:"TEMPLATE" ~doc)

let destination =
  let doc = "Path to place generated program" in
  Arg.(
    value & opt (some string) None & info ["d"; "destination"] ~docv:"PATH" ~doc)

let milner_t =
  Term.(
    const milner
    $ verbose
    $ debug_pattern
    $ allow_unsafe_named_parameter_order
    $ seed
    $ template
    $ destination)

let cmd =
  let doc = "a random well-typed program generator for Hack" in
  Cmd.v (Cmd.info "milner" ~doc) milner_t

let () = exit (Cmd.eval cmd)
