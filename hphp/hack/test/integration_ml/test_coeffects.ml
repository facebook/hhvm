(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Hh_prelude

let source =
  "<?hh // strict

namespace HH\\Contexts {
  type a = mixed;
  namespace Unsafe {
    type a = mixed;
  }
}

namespace {
  function foo()[a]: void {}
  //             ^ 11:18
}
"

let go_identify expected ~ctx ~entry ~line ~column =
  let actual =
    ServerIdentifyFunction.go_quarantined_absolute ~ctx ~entry ~line ~column
    |> Nuclide_rpc_message_printer.identify_symbol_response_to_json
  in
  Asserter.Hh_json_json_asserter.assert_equals
    (expected |> Hh_json.json_of_string)
    actual
    (Printf.sprintf "ServerIdentifyFunction at line %d, column %d" line column);
  ()

let go_hover expected ~ctx ~entry ~line ~column =
  let hovers_to_string h =
    List.map h ~f:HoverService.string_of_result |> String.concat ~sep:"; "
  in
  let actual = Ide_hover.go_quarantined ~ctx ~entry ~line ~column in
  Asserter.String_asserter.assert_equals
    (expected |> hovers_to_string)
    (actual |> hovers_to_string)
    (Printf.sprintf "Ide_hover at line %d, column %d" line column);
  ()

let pos_at (line1, column1) (line2, column2) =
  Some
    (Pos.make_from_lnum_bol_offset
       ~pos_file:Relative_path.default
       ~pos_start:(line1, 0, column1 - 1)
       ~pos_end:(line2, 0, column2))

let identify_tests =
  [
    ( (11, 18),
      go_identify
        {| [{
      "name":"\\HH\\Contexts\\a",
      "result_type":"class",
      "pos":{"filename":"/source.php","line":11,"char_start":18,"char_end":18},
      "definition_pos":{"filename":"/source.php","line":4,"char_start":8,"char_end":8},
      "definition_span":{"filename":"/source.php","line_start":4,"char_start":8,"line_end":4,"char_end":16},
      "definition_id":"type_id::HH\\Contexts\\a"}] |}
    );
    ( (11, 18),
      go_hover
        [
          {
            HoverService.snippet = "Contexts\\a";
            addendum = [];
            pos = pos_at (11, 18) (11, 18);
          };
        ] );
  ]

let test () =
  let root = "/" in
  let hhconfig_filename = Filename.concat root ".hhconfig" in
  let files = [("source.php", source)] in

  Relative_path.set_path_prefix Relative_path.Root (Path.make root);
  TestDisk.set hhconfig_filename "";
  let options = ServerArgs.default_options ~root in
  let (custom_config, _) = ServerConfig.load ~silent:false options in
  let env =
    Integration_test_base.setup_server
      ()
      ~custom_config
      ~hhi_files:(Hhi.get_raw_hhi_contents () |> Array.to_list)
  in
  let env = Integration_test_base.setup_disk env files in
  Integration_test_base.assert_no_errors env;

  let path = Relative_path.from_root ~suffix:"source.php" in
  let (ctx, entry) =
    Provider_context.add_entry_if_missing
      ~ctx:(Provider_utils.ctx_from_server_env env)
      ~path
  in

  List.iter identify_tests ~f:(fun ((line, column), go) ->
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          go ~ctx ~entry ~line ~column);
      ());
  (* Ide_hover.go_quarantined ~ctx ~entry ~line ~column *)
  ()
