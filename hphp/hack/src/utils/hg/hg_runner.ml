(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * Prints out the current HG revision, closest SVN ancestor, and the files
 * changed between that HG revision and the SVN ancestor.
 *
 * This tool is particularly useful to manually test the Hg module.
 *)

module Args = struct
  type t = { root: string }

  let usage = Printf.sprintf "Usage: %s [REPO DIRECTORY]\n" Sys.argv.(0)

  let parse () =
    let root = ref None in
    let () = Arg.parse [] (fun s -> root := Some s) usage in
    match !root with
    | None ->
      Printf.eprintf "%s" usage;
      exit 1
    | Some root -> { root }

  let root args = args.root
end

let () =
  Sys_utils.set_signal Sys.sigint (Sys.Signal_handle (fun _ -> exit 0));
  let args = Args.parse () in
  let current_hg_rev = Hg.current_working_copy_hg_rev @@ Args.root args in
  let (current_hg_rev, _) = Future.get_exn current_hg_rev in
  Printf.eprintf "Current HG rev: %s\n" (Hg.Rev.to_string current_hg_rev);
  let ancestor =
    Hg.get_closest_global_ancestor current_hg_rev @@ Args.root args
  in
  let ancestor = Future.get_exn ancestor in
  Printf.eprintf "SVN ancestor: %d\n" ancestor;
  let changes =
    Hg.files_changed_since_rev (Hg.Global_rev ancestor) @@ Args.root args
  in
  let changes = Future.get_exn changes in
  let changes = String.concat "\n" changes in
  Printf.eprintf "Changes: %s\n" changes;
  let changes_between_current_and_ancestor =
    Hg.files_changed_since_rev_to_rev
      ~start:(Hg.Global_rev ancestor)
      ~finish:(Hg.Hg_rev current_hg_rev)
      (Args.root args)
    |> Future.get_exn ~timeout:30
    |> String.concat ","
  in
  Printf.eprintf
    "Changes between global and hg rev: %s\n"
    changes_between_current_and_ancestor
