(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let matches rex source =
  try Pcre.exec_all ~rex source |> Array.to_list with
  | Stdlib.Not_found -> []

let header = Pcre.regexp "\\A<\\?hh\\b[ \\t]*\\r?\\n"

let file_attributes =
  Pcre.regexp "(?m)^[ \\t]*<<[ \\t]*file:[^>]*>>[ \\t]*\\r?\\n?"

let entrypoint_attribute = Pcre.regexp "<<[ \\t]*__EntryPoint[ \\t]*>>"

let entrypoint =
  Pcre.regexp
    "(?m)^[ \\t]*<<[ \\t]*__EntryPoint[ \\t]*>>\\s*function\\s+([A-Za-z_][A-Za-z0-9_]*)\\s*\\(\\s*\\)\\s*:\\s*void\\b"

let append_definitions definitions source =
  source ^ "\n// Auxiliary definitions\n" ^ definitions ^ "\n"

let render ~definitions source =
  let definitions = String.concat ~sep:"\n" definitions in
  let ordinary () = append_definitions definitions source in
  if
    Pcre.pmatch ~rex:(Pcre.regexp "(?m)^////[ \\t]") source
    || not (Pcre.pmatch ~rex:header source)
  then
    ordinary ()
  else
    let source =
      if
        (String.is_substring source ~substring:"MilnerDsl"
        || String.is_substring definitions ~substring:"MilnerDsl")
        && not (String.is_substring source ~substring:"expression_trees")
      then
        Pcre.substitute
          ~rex:header
          ~subst:(fun _ ->
            "<?hh\n<<file: __EnableUnstableFeatures('expression_trees')>>\n")
          source
      else
        source
    in
    let ordinary () = append_definitions definitions source in
    match matches entrypoint source with
    | [entry]
      when Random.int 8 = 0
           && (not (Pcre.pmatch ~rex:(Pcre.regexp "\\bnewtype\\b") definitions))
           && (not (Pcre.pmatch ~rex:(Pcre.regexp "\\bnewtype\\b") source))
           && not
                (Pcre.pmatch
                   ~rex:(Pcre.regexp "(?m)^\\s*(?:module|namespace)\\b")
                   source) ->
      let entry_name = Pcre.get_substring entry 1 in
      let rec fresh_wrapper suffix =
        let name = "milner_generated_entrypoint" ^ suffix in
        if
          String.is_substring source ~substring:name
          || String.is_substring definitions ~substring:name
        then
          fresh_wrapper (suffix ^ "_")
        else
          name
      in
      let attributes =
        matches file_attributes source
        |> List.map ~f:(fun attribute -> Pcre.get_substring attribute 0)
        |> String.concat ~sep:""
      in
      let member_header =
        "<?hh\n" ^ attributes ^ "\nmodule milner_generated;\n"
      in
      let body =
        source
        |> Pcre.substitute ~rex:header ~subst:(fun _ -> "")
        |> Pcre.substitute ~rex:file_attributes ~subst:(fun _ -> "")
        |> Pcre.substitute ~rex:entrypoint_attribute ~subst:(fun _ -> "")
      in
      let definitions =
        Pcre.substitute
          ~rex:(Pcre.regexp "(?m)^(?:async[ \\t]+)?function[ \\t]+")
          ~subst:(fun declaration -> "internal " ^ declaration)
          definitions
      in
      String.concat
        ~sep:"\n"
        [
          "//// modules.php\n<?hh\nnew module milner_generated {}";
          "//// definitions.php\n" ^ member_header ^ definitions;
          "//// main.php\n" ^ member_header ^ body;
          "<<__EntryPoint>>\nfunction " ^ fresh_wrapper "" ^ "(): void {";
          "  require_once __DIR__.'/modules.php';";
          "  require_once __DIR__.'/definitions.php';";
          "  " ^ entry_name ^ "();\n}\n";
        ]
    | _ -> ordinary ()
