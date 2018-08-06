(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
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
  Process.exec "hg" ~env:["HGPLAIN=1"] args

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
      let max_svn primary p2 = match primary, p2 with
        | Error x, _ -> Error x
        | _, Error y -> Error y
        | Ok x, Ok y -> Ok (max x y)
      in
      Future.merge primary_mergebase p2_mergebase max_svn


  (**
    * List the revisions between start and finish revisions, inclusive.
    *
    * hg log --rev start::finish -T {node}\n
    *)
  let revs_between_revs_inclusive ~start ~finish repo =
    let rev_range = Printf.sprintf "%s::%s" (rev_string start) (rev_string finish) in
    let process = exec_hg [
      "log";
      "--rev";
      rev_range;
      "-T";
      "{node}\n";
      "--cwd";
      repo;
    ] in
    Future.make process Sys_utils.split_lines

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

  (** Similar to above, except instead of listing files to get us to
   * the repo's current state, it gets us to the given "finish" revision.
   *
   * i.e. If we start at "start" revision, what files need be changed to get us
   * to "finish" revision.
   *
   * hg status -n --rev "start::end-start"
   *
   * Note, this is different from: hg status -n --rev "start::end"
   *
   * by subtracting out start from the rev range. Itmust be subtracted out
   * because we don't care about the files inside the start revision itself
   * (only what files were changed since then)
   *)
  let files_changed_since_rev_to_rev ~start ~finish repo =
    if String.equal (rev_string start) (rev_string finish) then
      (** Optimization: start and finish are syntactically the same.
       * They may still be the same revision but just written out
       * differently - this will be caught below.
       * *)
      Future.of_value []
    else
      let rev_list = revs_between_revs_inclusive ~start ~finish repo in
      let rev_range = Printf.sprintf "%s::%s-%s"
        (rev_string start) (rev_string finish) (rev_string start) in
      let process = exec_hg [
        "status";
        "-n";
        "--rev";
        rev_range;
        "--cwd";
        repo;
      ] in
      let file_list = Future.make process Sys_utils.split_lines in
      let merge_result file_list rev_list = match file_list, rev_list with
        | Ok file_list, _ ->
          (** We were able t retrieve the list of files. Just return them. *)
          Ok file_list
        | Error e, Ok rev_list -> begin
          (**
           * Getting list of files errored. This could happen if "start" and "end"
           * refer to the same rev (but appear different, since there are different
           * ways to describe revs, such as the hash itself vs. something
           * like ".~10"). If they do refer to the same rev, then the
           * number of revs between them will be exactly 1 (the rev itself).
           * So we return an empty list of files.
           *)
          if (List.length rev_list) = 1 then
            Ok []
          else
            (** "start" and "end" rev are not the same, but get the list of
             * files errored. So just return that error. *)
            Error e
        end
        | Error e, _ ->
          Error e
      in
      Future.merge file_list rev_list merge_result

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

      let reset_current_working_copy_base_rev_returns _ =
        raise Cannot_set_when_mocks_disabled

      let closest_svn_ancestor_bind_value _ _ =
        raise Cannot_set_when_mocks_disabled

      let files_changed_since_rev_returns _ =
        raise Cannot_set_when_mocks_disabled

      let files_changed_since_rev_to_rev_returns ~start:_ ~finish:_ _ =
        raise Cannot_set_when_mocks_disabled

      let reset_files_changed_since_rev_to_rev_returns _ =
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
    let files_changed_since_rev_to_rev = Hashtbl.create 10

    let current_working_copy_hg_rev_returns v =
      current_working_copy_hg_rev := v

    let current_working_copy_base_rev_returns v =
      current_working_copy_base_rev := v

    let reset_current_working_copy_base_rev_returns () =
      current_working_copy_base_rev := Future.of_value 0

    let closest_svn_ancestor_bind_value hg_rev svn_rev =
      Hashtbl.replace closest_svn_ancestor hg_rev svn_rev

    let files_changed_since_rev_returns v =
      files_changed_since_rev := v
    let files_changed_since_rev_to_rev_returns ~start ~finish v =
      Hashtbl.replace files_changed_since_rev_to_rev (start, finish) v
    let reset_files_changed_since_rev_to_rev_returns () =
      Hashtbl.reset files_changed_since_rev_to_rev
  end

  let current_working_copy_hg_rev _ = !Mocking.current_working_copy_hg_rev
  let current_working_copy_base_rev _ = !Mocking.current_working_copy_base_rev
  let get_closest_svn_ancestor hg_rev _ =
    Hashtbl.find Mocking.closest_svn_ancestor hg_rev
  let files_changed_since_rev _ _ = !Mocking.files_changed_since_rev
  let files_changed_since_rev_to_rev ~start ~finish _ =
    Hashtbl.find Mocking.files_changed_since_rev_to_rev (start, finish)
  let update_to_rev _ _ = Future.of_value ()
  let get_old_version_of_files ~rev:_ ~files:_ ~out:_ ~repo:_ = Future.of_value ()


end;;

include (val (if Injector_config.use_test_stubbing
  then (module Hg_mock : Hg_sig.S)
  else (module Hg_actual : Hg_sig.S)
))
