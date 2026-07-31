exception Out_of_retries

let rec mkdtemp_in ~parent ~skip_mocking ~retries =
  if retries < 0 then
    raise Out_of_retries
  else
    let name = Random_id.short_string () in
    let tmp_dir = Path.concat parent name in
    try
      let () = Sys_utils.mkdir_p (Path.to_string tmp_dir) ~skip_mocking in
      tmp_dir
    with
    | Unix.Unix_error _ ->
      mkdtemp_in ~parent ~skip_mocking ~retries:(retries - 1)

let mkdtemp ~skip_mocking =
  mkdtemp_in
    ~parent:(Path.make Sys_utils.temp_dir_name)
    ~skip_mocking
    ~retries:30

let with_tempdir ?parent ~skip_mocking g =
  let parent =
    Option.value parent ~default:(Path.make Sys_utils.temp_dir_name)
  in
  let dir = mkdtemp_in ~parent ~skip_mocking ~retries:30 in
  let f () = g dir in
  let%lwt result =
    Lwt_utils.try_finally ~f ~finally:(fun () ->
        Sys_utils.rm_dir_tree (Path.to_string dir) ~skip_mocking;
        Lwt.return_unit)
  in
  Lwt.return result

let with_real_tempdir g =
  Random.self_init ();
  with_tempdir ~skip_mocking:true g

let with_tempdir ?parent g = with_tempdir ?parent ~skip_mocking:false g
