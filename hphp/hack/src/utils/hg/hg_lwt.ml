(* The Mercurial commit hash. *)
type rev = string

let rev_to_string (rev: rev): string =
  rev

(** Currently just the hash is stored in the revision, but we might want to
store additional information in the future. For example, we might also want to
store the SVN revision, if applicable. *)
let get_hash_from_rev (rev: rev): string =
  rev

let exec_hg
    (args: string array)
    : (Lwt_utils.Process_success.t, Lwt_utils.Process_failure.t) Lwt_result.t =
  let%lwt result = Lwt_utils.exec_checked "hg" args
    (* Disable user aliases or configs. *)
    ~env:[| "HGPLAIN=1" |]
  in
  Lwt.return result

let get_latest_ancestor_public_commit
    ~(repo: Path.t)
    : (rev, string) Lwt_result.t =
  let repo = Path.to_string repo in
  let%lwt result = exec_hg [|
    "log";
    "--cwd"; repo;
    (* TODO: verify that this produces something sensible during a merge
    conflict. *)
    "-r"; Printf.sprintf "ancestor(last(public()), .)";
    "-T"; "{node}";
  |] in
  match result with
  | Error process_failure ->
    Lwt.return (Error (Lwt_utils.Process_failure.to_string process_failure))
  | Ok { Lwt_utils.Process_success.command_line; stdout; _ } ->
    let result = String.trim stdout in
    if String.length result < 1 then
      Lwt.return_error (Printf.sprintf "Empty result from: %s" command_line)
    else
      (* Whether or not there are local changes; we don't use this information for
      now. *)
      let result =
        if result.[(String.length result) - 1] = '+' then
          (String.sub result 0 ((String.length result) - 1))
        else
          result
      in
      Lwt.return_ok result
