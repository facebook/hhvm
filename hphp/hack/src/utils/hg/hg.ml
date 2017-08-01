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

(** Returns the closest SVN ancestor in master to the given hg rev.
 *
 * hg log -r 'ancestor(master,hg_rev)' -T '{svnrev}\n'
 *
 * Note: hg_rev only works in short form from `hg id -i` (see below)
 * and not the full hash form from `hg --debug id -i`
 *)
let get_closest_svn_ancestor hg_rev repo =
  let process = Process.exec "hg" [
    "log";
    "-r";
    Printf.sprintf "ancestor(master,%s)" hg_rev;
    "-T";
    "{svnrev}\n";
    "--cwd";
    repo;
  ]
  in
  Future.make process String.trim

  (** Get the hg revision hash of the current working copy in the repo dir.
   *
   * hg id -i --cwd <repo> *)
  let current_working_copy_hg_rev repo =
    let process = Process.exec "hg" ["id"; "-i"; "--cwd"; repo; ] in
    Future.make process @@ fun result ->
      let result = String.trim result in
      if String.length result < 1 then
        raise Malformed_result
      else
        if result.[(String.length result) - 1] = '+' then
          (String.sub result 0 ((String.length result) - 1)), true
        else
          result, false

  (** hg log -r 'ancestor(master,.)' -T '{svnrev}\n' *)
  let current_working_copy_base_rev repo =
    let process = Process.exec "hg" [
      "log";
      "-r";
      "ancestor(master,.)";
      "-T";
      "{svnrev}\n";
      "--cwd";
      repo;
    ] in
    Future.make process String.trim

  (** Returns the files changed between the hg_rev and the ancestor
   * SVN revision.
   *
   * hg status --rev r<svn_rev> --rev <hg_rev> --cwd <repo> *)
  let files_changed_since_svn_rev hg_rev svn_rev repo =
    let process = Process.exec "hg" [
      "status";
      "--rev";
      Printf.sprintf "r%s" svn_rev;
      "--rev";
      hg_rev;
      "--cwd";
      repo;
    ] in
    Future.make process String.trim

  (** hg update --rev r<svn_rev> --cwd <repo> *)
  let update_to_base_rev svn_rev repo =
    let process = Process.exec "hg" [
      "update";
      "--rev";
      Printf.sprintf "r%s" svn_rev;
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

      let files_changed_since_svn_rev_returns _ =
        raise Cannot_set_when_mocks_disabled
    end

end;;

module Hg_mock = struct

  include Hg_sig.Types

  module Mocking = struct
    let current_working_copy_hg_rev = ref @@ Future.of_value ("", false)
    let current_working_copy_base_rev = ref @@ Future.of_value ""
    let closest_svn_ancestor = Hashtbl.create 10
    let files_changed_since_svn_rev = ref @@ Future.of_value ""

    let current_working_copy_hg_rev_returns v =
      current_working_copy_hg_rev := v

    let current_working_copy_base_rev_returns v =
      current_working_copy_base_rev := v

    let closest_svn_ancestor_bind_value hg_rev svn_rev =
      Hashtbl.replace closest_svn_ancestor hg_rev svn_rev

    let files_changed_since_svn_rev_returns v =
      files_changed_since_svn_rev := v
  end

  let current_working_copy_hg_rev _ = !Mocking.current_working_copy_hg_rev
  let current_working_copy_base_rev _ = !Mocking.current_working_copy_base_rev
  let get_closest_svn_ancestor hg_rev _ =
    Hashtbl.find Mocking.closest_svn_ancestor hg_rev
  let files_changed_since_svn_rev _ _ _ = !Mocking.files_changed_since_svn_rev
  let update_to_base_rev _ _ = Future.of_value ()

end;;

include (val (if Injector_config.use_test_stubbing
  then (module Hg_mock : Hg_sig.S)
  else (module Hg_actual : Hg_sig.S)
))
