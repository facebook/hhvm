(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(** Tools for shelling out to Mercurial. *)

module Hg_actual = struct

  include Hg_sig.Types

  let rev_string rev = match rev with
    | Hg_rev hash -> hash
    | Svn_rev rev -> Printf.sprintf "r%d" rev

let exec_hg args =
  (** Disable user aliases or configs. *)
  Process.exec "env" ("HGPLAIN=1" :: "hg" :: args)

(** Given a list of files and their revisions, saves the files to the output
 * directory. For example,
 * get_old_version_of_files ~rev:"X" ~files:["file1.php"]
 * ~out:"/tmp/hh_server/%s" ~repo:"~/www"
 * runs the command
 *
 * hg cat -r X file1.php -o "/tmp/hh_server/%s" --cwd ~/www
 *
 * which saves the version of file1.php at revision X in directory
 * /tmp/hh_server/file1.php
 *)
let get_old_version_of_files ~rev ~files ~out ~repo =
  let process = exec_hg ([
    "cat";
    "-r";
    (rev_string rev);
    ] @ files @ [
    "-o";
    out;
    "--cwd";
    repo;
  ])
  in
  Future.make process ignore

(** Returns the closest SVN ancestor in master to the given rev.
 *
 * hg log -r 'ancestor(master,rev)' -T '{svnrev}\n'
 *)
let get_closest_svn_ancestor rev repo =
  let process = exec_hg [
    "log";
    "-r";
    Printf.sprintf "ancestor(master,%s)" rev;
    "-T";
    "{svnrev}\n";
    "--cwd";
    repo;
  ]
  in
  Future.make process (fun s -> int_of_string (String.trim s))

  (** Get the hg revision hash of the current working copy in the repo dir.
   *
   * hg id -i --cwd <repo> *)
  let current_working_copy_hg_rev repo =
    let process = exec_hg ["id"; "--debug"; "-i"; "--cwd"; repo; ] in
    Future.make process @@ fun result ->
      let result = String.trim result in
      if String.length result < 1 then
        raise Malformed_result
      else
        if result.[(String.length result) - 1] = '+' then
          (String.sub result 0 ((String.length result) - 1)), true
        else
          result, false

  (** hg log -r 'p2()' -T '{node}' *)
  let get_p2_node repo =
    let process = exec_hg [
      "log";
      "-r";
      "p2()";
      "-T";
      "{node}";
      "--cwd";
      repo;
    ]
    in
    let future = Future.make process String.trim in
    match Future.get future with
    | Ok "" -> None
    | Ok s -> Some s
    | Error _ -> None

  (**
   * Returns the SVN base revision. If the current node is a normal
   * commit, this is simply the closest_svn_ancestor.
   *
   * If the current node is a merge commit (for example during a merge-conflict
   * state), then it computes the two merge bases with master (one for each
   * parent) and uses the greater of the two.
   * *)
  let current_working_copy_base_rev repo =
    let primary_mergebase = get_closest_svn_ancestor "." repo in
    (** Ok, since (get_closest_svn_ancestor p2) depends on getting p2, we
     * actually block on getting p2 first. *)
    match get_p2_node repo with
    | None -> primary_mergebase
    | Some p2 ->
      let p2_mergebase = get_closest_svn_ancestor p2 repo in
      let max_svn primary p2 = max primary p2 in
      Future.merge primary_mergebase p2_mergebase max_svn

  (** Returns the files changed since the given svn_rev
   *
   * hg status -n --rev r<svn_rev> --cwd <repo> *)
  let files_changed_since_rev rev repo =
    let process = exec_hg [
      "status";
      "-n";
      "--rev";
      rev_string rev;
      "--cwd";
      repo;
    ] in
    Future.make process Sys_utils.split_lines

  (** hg update --rev r<svn_rev> --cwd <repo> *)
  let update_to_rev rev repo =
    let process = exec_hg [
      "update";
      "--rev";
      rev_string rev;
      "--cwd";
      repo;
    ] in
    Future.make process ignore

    module Mocking = struct
      exception Cannot_set_when_mocks_disabled

      let current_working_copy_hg_rev_returns _ =
        raise Cannot_set_when_mocks_disabled

      let current_working_copy_base_rev_returns _ =
        raise Cannot_set_when_mocks_disabled

      let closest_svn_ancestor_bind_value _ _ =
        raise Cannot_set_when_mocks_disabled

      let files_changed_since_rev_returns _ =
        raise Cannot_set_when_mocks_disabled
    end

end;;

module Hg_mock = struct

  include Hg_sig.Types

  module Mocking = struct
    let current_working_copy_hg_rev = ref @@ Future.of_value ("", false)
    let current_working_copy_base_rev = ref @@ Future.of_value 0
    let closest_svn_ancestor = Hashtbl.create 10
    let files_changed_since_rev = ref @@ Future.of_value []

    let current_working_copy_hg_rev_returns v =
      current_working_copy_hg_rev := v

    let current_working_copy_base_rev_returns v =
      current_working_copy_base_rev := v

    let closest_svn_ancestor_bind_value hg_rev svn_rev =
      Hashtbl.replace closest_svn_ancestor hg_rev svn_rev

    let files_changed_since_rev_returns v =
      files_changed_since_rev := v
  end

  let current_working_copy_hg_rev _ = !Mocking.current_working_copy_hg_rev
  let current_working_copy_base_rev _ = !Mocking.current_working_copy_base_rev
  let get_closest_svn_ancestor hg_rev _ =
    Hashtbl.find Mocking.closest_svn_ancestor hg_rev
  let files_changed_since_rev _ _ = !Mocking.files_changed_since_rev
  let update_to_rev _ _ = Future.of_value ()
  let get_old_version_of_files ~rev:_ ~files:_ ~out:_ ~repo:_ = Future.of_value ()


end;;

include (val (if Injector_config.use_test_stubbing
  then (module Hg_mock : Hg_sig.S)
  else (module Hg_actual : Hg_sig.S)
))
