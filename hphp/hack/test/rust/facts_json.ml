(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
let () =
  let enable_xhp = ref false in
  let force_hh = ref false in
  let file_path = ref "" in
  let options = [
    "--enable-xhp", Arg.Set enable_xhp, "";
    "--force-hh", Arg.Set force_hh, "";
    "--file-path", Arg.String (fun s -> file_path := s), ""
  ] in
  Arg.parse options (fun _ -> ()) "";

  let text = Sys_utils.cat !file_path in

  let json =

    match Facts_parser.extract_as_json
      ~php5_compat_mode:true
      ~hhvm_compat_mode:true
      ~force_hh:!force_hh
      ~enable_xhp:!enable_xhp
      ~filename:(!file_path |> Relative_path.create Relative_path.Dummy)
      ~text
    with
    | Some(hh_json) -> Hh_json.json_to_string hh_json
    | None -> "{}"
  in
  Printf.printf "%s\n" json;
  ()
