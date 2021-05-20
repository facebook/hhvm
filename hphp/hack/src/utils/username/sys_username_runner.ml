let () =
  match Sys_username.get_logged_in_username () with
  | Some name -> Printf.printf "%s\n" name
  | None -> failwith "Can't get logged in username"
