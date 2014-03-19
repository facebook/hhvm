(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
open Format

type env = {
  root: Path.path option;
  user: string option;
}

let get_running_servers env = 
  let locks = Lock.find_all_locks "lock" in
  (* Filter out the servers no longer running *)
  List.fold_left begin fun acc (user, root) ->
    try 
      let use = match env.user with
        | None -> true 
        | Some u -> u = user in
      let use = use && match env.root with
        | None -> true 
        | Some r -> Path.equal r root in
      if use 
      then begin
        ignore (ClientUtils.connect ~user:(Some user) root);
        (user, root)::acc
      end
      else acc
    with 
    | ClientExceptions.Server_cant_connect
    | ClientExceptions.Server_missing -> acc
    | e -> (user, root)::acc
  end [] locks

let main env =
  let all_servers = get_running_servers env in
  if List.length all_servers = 0 
  then begin
    Printf.printf "No running servers found!\n%!";
    exit 0
  end else
    open_tbox ();
    set_tab ();
    printf "USER";
    print_break 12 0;
    set_tab ();
    printf "WWW_ROOT";
    List.iter (fun (user, root) ->
      print_tab ();
      printf "%s" user;
      print_tbreak 0 16;
      printf "%s" (Path.string_of_path root);
    ) all_servers;
    print_newline();
    exit 0
