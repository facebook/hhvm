(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
    saved_root : Path.t;
    saved_hhi : Path.t;
    saved_tmp : Path.t;
    trace : bool;
    fuzzy : bool;
    profile_log : bool;
    fixme_codes : ISet.t;
    paths_to_ignore : Str.regexp list;
    no_load : bool;
    logging_init : unit -> unit;
  }

let save ~logging_init = {
    saved_root = Path.make (Relative_path.(path_of_prefix Root));
    saved_hhi = Path.make (Relative_path.(path_of_prefix Hhi));
    saved_tmp = Path.make (Relative_path.(path_of_prefix Tmp));
    trace = !Typing_deps.trace;
    fuzzy = SymbolIndex.fuzzy_search_enabled ();
    profile_log = !Utils.profile;
    fixme_codes = !Errors.ignored_fixme_codes;
    paths_to_ignore = FilesToIgnore.get_paths_to_ignore ();
    no_load = ServerLoadFlag.get_no_load ();
    logging_init;
  }

let restore state =
  Relative_path.(set_path_prefix Root state.saved_root);
  Relative_path.(set_path_prefix Hhi state.saved_hhi);
  Relative_path.(set_path_prefix Tmp state.saved_tmp);
  Typing_deps.trace := state.trace;
  SymbolIndex.set_fuzzy_search_enabled state.fuzzy;
  Utils.profile := state.profile_log;
  Errors.ignored_fixme_codes := state.fixme_codes;
  FilesToIgnore.set_paths_to_ignore state.paths_to_ignore;
  ServerLoadFlag.set_no_load state.no_load;
  Errors.set_allow_errors_in_default_path false;
  state.logging_init ()

let to_string state =
  let saved_root = Path.to_string state.saved_root in
  let saved_hhi = Path.to_string state.saved_hhi in
  let saved_tmp = Path.to_string state.saved_tmp in
  let trace = if state.trace then "true" else "false" in
  let fuzzy = if state.fuzzy then "true" else "false" in
  let profile_log = if state.profile_log then "true" else "false" in
  let fixme_codes = ISet.to_string state.fixme_codes in
  (* OCaml regexps cannot be re-serialized to strings *)
  let paths_to_ignore = "(...)" in
  [
    ("saved_root", saved_root);
    ("saved_hhi", saved_hhi);
    ("saved_tmp", saved_tmp);
    ("trace", trace);
    ("fuzzy", fuzzy);
    ("profile_log", profile_log);
    ("fixme_codes", fixme_codes);
    ("paths_to_ignore", paths_to_ignore);
  ]
    |> List.map (fun (x, y) -> Printf.sprintf "%s : %s" x y)
    |> String.concat ", "
    |> Printf.sprintf "{%s}"
