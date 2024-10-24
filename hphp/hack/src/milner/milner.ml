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

let type_regexp = Pcre.regexp @@ type_prefix ^ "#([0-9]+)"

let expr_prefix = "expr"

let expr_regexp = Pcre.regexp @@ expr_prefix ^ "#([0-9]+)"

let placeholder prefix key = prefix ^ "#" ^ string_of_int key

let init_table contents placeholder =
  let placeholders =
    try Pcre.exec_all ~rex:placeholder contents |> Array.to_list with
    | _ -> []
  in
  let table = Hashtbl.create (module Int) in
  List.iter placeholders ~f:(fun placeholder ->
      let key = (Pcre.get_substrings placeholder).(1) |> int_of_string in
      Hashtbl.set ~key ~data:() table);
  table

(* Generate types and conforming expressions for all placeholders in the
   template *)
let generate_tables ~verbose ~debug_pattern template =
  let renv = Gen.ReadOnlyEnvironment.default ~verbose ~debug_pattern in
  let env = Gen.Environment.default in
  let mk_type () = Gen.Type.mk renv env in
  (* Farm the type placeholders from the template and randomly generate types *)
  let ty_table = init_table template type_regexp in
  let ty_table = Hashtbl.map ty_table ~f:mk_type in

  (* Generate expressions that conform to the types in the type table.
     If there are expression placeholders without a corresponding type, generate
     a random type and use that to generate an expression. *)
  let expr_table = init_table template expr_regexp in
  let gen_expr_from_ty_table ~key ~data:_ =
    let (env, ty) =
      Hashtbl.find ty_table key |> Option.value_or_thunk ~default:mk_type
    in
    Gen.Type.inhabitant_of renv env ty
  in
  let expr_table = Hashtbl.mapi expr_table ~f:gen_expr_from_ty_table in

  let defs =
    let get_defs (_, (env, _)) = Gen.Environment.definitions env in
    let ty_defs = Hashtbl.to_alist ty_table |> List.map ~f:get_defs in
    List.concat ty_defs
  in
  let ty_table = Hashtbl.map ty_table ~f:(fun (_, ty) -> ty) in

  (defs, ty_table, expr_table)

(* Add generated types and expressions back in the template *)
let fill_in_template ty_table expr_table template =
  let fill_table table ~prefix contents =
    let replace ~key ~data contents =
      String.substr_replace_all
        ~pattern:(placeholder prefix key)
        ~with_:data
        contents
    in
    Hashtbl.fold table ~init:contents ~f:replace
  in

  let ty_str_table = Hashtbl.map ty_table ~f:Gen.Type.show in
  template
  |> fill_table ty_str_table ~prefix:type_prefix
  |> fill_table expr_table ~prefix:expr_prefix

let add_missing_definitions defs output =
  output
  ^ "\n"
  ^ "// Auxiliary definitions\n"
  ^ String.concat ~sep:"\n" (List.map ~f:Gen.Definition.show defs)
  ^ "\n"

let milner verbose debug_pattern seed template_path destination_path =
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
  let (defs, ty_table, expr_table) =
    generate_tables ~verbose ~debug_pattern template
  in
  let output =
    fill_in_template ty_table expr_table template
    |> add_missing_definitions defs
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

let template =
  let doc = "Template file to generate well-typed programs from" in
  Arg.(required & pos 0 (some file) None & info [] ~docv:"TEMPLATE" ~doc)

let destination =
  let doc = "Path to place generated program" in
  Arg.(
    value & opt (some string) None & info ["d"; "destination"] ~docv:"PATH" ~doc)

let milner_t =
  Term.(const milner $ verbose $ debug_pattern $ seed $ template $ destination)

let cmd =
  let doc = "a random well-typed program generator for Hack" in
  Cmd.v (Cmd.info "milner" ~doc) milner_t

let () = exit (Cmd.eval cmd)
