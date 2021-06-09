exception Out_of_retries

let rec mkdtemp ~(dir : Path.t) ~(skip_mocking : bool) ~(retries : int) : Path.t
    =
  if retries < 0 then
    raise Out_of_retries
  else
    let name = Random_id.(short_string_with_alphabet alphanumeric_alphabet) in
    let tmp_dir = Path.concat dir name in
    try
      let () = Sys_utils.mkdir_p (Path.to_string tmp_dir) ~skip_mocking in
      tmp_dir
    with Unix.Unix_error _ -> mkdtemp ~dir ~skip_mocking ~retries:(retries - 1)

let mkdtemp_with_dir (dir : Path.t) =
  mkdtemp ~dir ~skip_mocking:false ~retries:30

let mkdtemp ~skip_mocking =
  mkdtemp ~dir:(Path.make Sys_utils.temp_dir_name) ~skip_mocking ~retries:30

let with_tempdir ~skip_mocking g =
  let dir = mkdtemp ~skip_mocking in
  let f () = g dir in
  Utils.try_finally ~f ~finally:(fun () ->
      Sys_utils.rm_dir_tree (Path.to_string dir) ~skip_mocking)

let with_real_tempdir g =
  Random.self_init ();
  with_tempdir ~skip_mocking:true g

let with_tempdir g = with_tempdir ~skip_mocking:false g
