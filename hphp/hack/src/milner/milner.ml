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

let alias_type_prefix = "ALIAS_TYPE"

let alias_type_regexp = Pcre.regexp @@ alias_type_prefix ^ "#([0-9]+)"

let intersection_type_prefix = "INTERSECTION_TYPE"

let intersection_type_regexp =
  Pcre.regexp @@ intersection_type_prefix ^ "#([0-9]+)"

let subtype_prefix = "SUBTYPE"

let subtype_regexp = Pcre.regexp @@ subtype_prefix ^ "#([0-9]+)"

let expr_prefix = "expr"

let expr_regexp = Pcre.regexp @@ expr_prefix ^ "#([0-9]+)"

let hierarchy_prefixes =
  [
    "CLASS_TYPE";
    "ANCESTOR_TYPE";
    "construct";
    "read";
    "write";
    "identity";
    "hierarchy";
    "dispatch";
    "DISPATCH";
    "trait_read";
    "interface_read";
    "interface_write";
  ]
  |> List.sort ~compare:(fun left right ->
         Int.compare (String.length right) (String.length left))

let generic_prefixes =
  [
    "GENERIC_TAGGED_WRITER";
    "GENERIC_TAGGED_CLASS";
    "GENERIC_SUBTYPE";
    "generic_subexpr";
    "GENERIC_READER";
    "GENERIC_WRITER";
    "GENERIC_FAMILY";
    "GENERIC_CLASS";
    "GENERIC_WIDE";
    "GENERIC_KEY";
    "generic_key";
  ]

let dependent_prefixes =
  [
    "DEPENDENT_READ_BOUND";
    "DEPENDENT_REFINED";
    "DEPENDENT_CLASS";
    "DEPENDENT_BOUND";
    "DEPENDENT_BASE";
    "DEPENDENT_ITEM";
    "DEPENDENT_READ";
  ]

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
  let alias_types = init_table template alias_type_regexp in
  let intersection_types = init_table template intersection_type_regexp in
  let hierarchies = Hashtbl.create (module Int) in
  List.iter hierarchy_prefixes ~f:(fun prefix ->
      let table = init_table template (Pcre.regexp (prefix ^ "#([0-9]+)")) in
      Hashtbl.iter_keys table ~f:(fun key ->
          Hashtbl.set hierarchies ~key ~data:()));
  let generics = Hashtbl.create (module Int) in
  List.iter generic_prefixes ~f:(fun prefix ->
      let table = init_table template (Pcre.regexp (prefix ^ "#([0-9]+)")) in
      Hashtbl.iter_keys table ~f:(fun key -> Hashtbl.set generics ~key ~data:()));
  let dependents = Hashtbl.create (module Int) in
  List.iter dependent_prefixes ~f:(fun prefix ->
      let table = init_table template (Pcre.regexp (prefix ^ "#([0-9]+)")) in
      Hashtbl.iter_keys table ~f:(fun key ->
          Hashtbl.set dependents ~key ~data:()));
  let renv_for key =
    if Hashtbl.mem alias_types key then
      Gen.ReadOnlyEnvironment.for_alias renv
    else
      renv
  in
  let mk_type ~key ~data:() = Gen.Type.mk (renv_for key) env in
  (* Farm the type placeholders from the template and randomly generate types *)
  let ty_table = init_table template type_regexp in
  let expr_table = init_table template expr_regexp in
  let another_table = init_table template (Pcre.regexp "another#([0-9]+)") in
  let subty_table = init_table template subtype_regexp in
  List.iter
    [
      expr_table;
      another_table;
      subty_table;
      alias_types;
      hierarchies;
      generics;
      dependents;
    ]
    ~f:(fun table ->
      Hashtbl.iter_keys table ~f:(fun key -> Hashtbl.set ty_table ~key ~data:()));
  let ty_table =
    Hashtbl.fold
      ty_table
      ~init:(Hashtbl.create (module Int))
      ~f:(fun ~key ~data table ->
        let rec generate () =
          let (env, ty) = mk_type ~key ~data in
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

  let hierarchy_table =
    Hashtbl.mapi hierarchies ~f:(fun ~key ~data:() ->
        let (env, ty) = Hashtbl.find_exn ty_table key in
        let (env, bindings) =
          Gen.Type.hierarchy_bindings (renv_for key) env ty
        in
        Hashtbl.set ty_table ~key ~data:(env, ty);
        bindings)
  in

  let generic_table =
    Hashtbl.mapi generics ~f:(fun ~key ~data:() ->
        let (env, value) = Hashtbl.find_exn ty_table key in
        let (env, witness) =
          Gen.Type.mk_generic_witness (renv_for key) env ~value
        in
        Hashtbl.set ty_table ~key ~data:(env, value);
        let open Gen.Type in
        [
          ("GENERIC_CLASS", show witness.generic_class);
          ("GENERIC_WIDE", show witness.generic_wide);
          ("GENERIC_READER", show witness.generic_reader);
          ("GENERIC_WRITER", show witness.generic_writer);
          ("GENERIC_KEY", show witness.generic_key);
          ("GENERIC_SUBTYPE", show witness.generic_narrow);
          ("GENERIC_TAGGED_CLASS", show witness.generic_tagged_class);
          ("GENERIC_TAGGED_WRITER", show witness.generic_tagged_writer);
          ("GENERIC_FAMILY", witness.generic_family);
          ("generic_key", inhabitant_of (renv_for key) env witness.generic_key);
          ( "generic_subexpr",
            inhabitant_of (renv_for key) env witness.generic_narrow );
        ])
  in

  let dependent_table =
    Hashtbl.mapi dependents ~f:(fun ~key ~data:() ->
        let (env, value) = Hashtbl.find_exn ty_table key in
        let (env, witness) =
          Gen.Type.mk_dependent_witness (renv_for key) env ~value
        in
        Hashtbl.set ty_table ~key ~data:(env, value);
        let open Gen.Type in
        [
          ("DEPENDENT_CLASS", witness.dependent_class);
          ("DEPENDENT_BASE", witness.dependent_base);
          ("DEPENDENT_ITEM", show witness.dependent_item);
          ("DEPENDENT_BOUND", show witness.dependent_bound);
          ("DEPENDENT_READ", witness.dependent_read);
          ("DEPENDENT_READ_BOUND", witness.dependent_read_bound);
          ( "DEPENDENT_REFINED",
            witness.dependent_base ^ " with { type Item = " ^ show value ^ " }"
          );
        ])
  in

  let gen_subty_from_ty_table ~key ~data:_ =
    let (env, ty) = Hashtbl.find_exn ty_table key in
    Gen.Type.subtype_of (renv_for key) env ty
  in
  let subty_table = Hashtbl.mapi subty_table ~f:gen_subty_from_ty_table in

  let gen_expr_from_ty_table ~key ~data:_ =
    let (env, ty) = Hashtbl.find_exn ty_table key in
    Gen.Type.inhabitant_of (renv_for key) env ty
  in
  let expr_table = Hashtbl.mapi expr_table ~f:gen_expr_from_ty_table in
  let another_table = Hashtbl.mapi another_table ~f:gen_expr_from_ty_table in

  let defs =
    let get_defs (_, (env, _)) = Gen.Environment.definitions env in
    let ty_defs = Hashtbl.to_alist ty_table |> List.map ~f:get_defs in
    List.concat ty_defs
  in
  let ty_table = Hashtbl.map ty_table ~f:(fun (_, ty) -> ty) in

  ( defs,
    ty_table,
    subty_table,
    expr_table,
    another_table,
    hierarchy_table,
    generic_table,
    dependent_table )

(* Add generated types and expressions back in the template *)
let fill_in_template
    ty_table
    subty_table
    expr_table
    another_table
    hierarchy_table
    generic_table
    dependent_table
    template =
  let fill_table table ~prefix contents =
    let replace ~key ~data contents =
      String.substr_replace_all
        ~pattern:(placeholder prefix key)
        ~with_:data
        contents
    in
    Hashtbl.to_alist table
    |> List.sort ~compare:(fun (left, _) (right, _) -> Int.compare right left)
    |> List.fold ~init:contents ~f:(fun contents (key, data) ->
           replace ~key ~data contents)
  in

  let ty_str_table = Hashtbl.map ty_table ~f:Gen.Type.show in
  let subty_str_table = Hashtbl.map subty_table ~f:Gen.Type.show in
  let template =
    List.fold hierarchy_prefixes ~init:template ~f:(fun contents prefix ->
        let table =
          Hashtbl.map hierarchy_table ~f:(fun bindings ->
              List.Assoc.find_exn bindings ~equal:String.equal prefix)
        in
        fill_table table ~prefix contents)
  in
  let template =
    List.fold generic_prefixes ~init:template ~f:(fun contents prefix ->
        let table =
          Hashtbl.map generic_table ~f:(fun bindings ->
              List.Assoc.find_exn bindings ~equal:String.equal prefix)
        in
        fill_table table ~prefix contents)
  in
  let template =
    List.fold dependent_prefixes ~init:template ~f:(fun contents prefix ->
        let table =
          Hashtbl.map dependent_table ~f:(fun bindings ->
              List.Assoc.find_exn bindings ~equal:String.equal prefix)
        in
        fill_table table ~prefix contents)
  in
  (* Replace longer prefixes before TYPE, which is their common suffix. *)
  template
  |> fill_table subty_str_table ~prefix:subtype_prefix
  |> fill_table ty_str_table ~prefix:alias_type_prefix
  |> fill_table ty_str_table ~prefix:intersection_type_prefix
  |> fill_table ty_str_table ~prefix:type_prefix
  |> fill_table expr_table ~prefix:expr_prefix
  |> fill_table another_table ~prefix:"another"

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
  let ( defs,
        ty_table,
        subty_table,
        expr_table,
        another_table,
        hierarchy_table,
        generic_table,
        dependent_table ) =
    generate_tables ~verbose ~debug_pattern template
  in
  let output =
    fill_in_template
      ty_table
      subty_table
      expr_table
      another_table
      hierarchy_table
      generic_table
      dependent_table
      template
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
