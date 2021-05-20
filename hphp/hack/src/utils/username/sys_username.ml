(** When sudo is used to execute a process, effective user ID is root,
 * but user id will still be the original user. *)
let get_real_user_name () =
  let uid = Unix.getuid () in
  if uid = 1 then
    None
  else
    let pwd_entry = Unix.getpwuid uid in
    Some pwd_entry.Unix.pw_name

let get_logged_in_username () =
  let name =
    try Unix.getlogin ()
    with Unix.Unix_error (Unix.ENOENT, m, _) when String.equal m "getlogin" ->
      begin
        try
          (* Linux getlogin(3) man page suggests checking LOGNAME. *)
          Sys.getenv "LOGNAME"
        with Not_found -> Sys.getenv "SUDO_USER"
      end
  in
  try
    if String.equal name "root" then
      get_real_user_name ()
    else
      Some name
  with Not_found -> None
