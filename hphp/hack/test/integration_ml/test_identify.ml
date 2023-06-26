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

let source_with_errors =
  "<?hh // strict
interface I<T> {}
function test(I<NotDefined::Foo> $_): void {}
"

let test () =
  let root = "/" in
  let hhconfig_filename = Filename.concat root ".hhconfig" in
  let hhconfig_contents = "" in
  let files = [("source_with_errors.php", source_with_errors)] in

  Relative_path.set_path_prefix Relative_path.Root (Path.make root);
  TestDisk.set hhconfig_filename hhconfig_contents;
  let options = ServerArgs.default_options ~root in
  let (custom_config, _) = ServerConfig.load ~silent:false options in
  let env =
    Integration_test_base.setup_server
      ()
      ~custom_config
      ~hhi_files:(Hhi.get_raw_hhi_contents () |> Array.to_list)
  in
  let env = Integration_test_base.setup_disk env files in

  let path = Relative_path.from_root ~suffix:"source_with_errors.php" in
  let (ctx, entry) =
    Provider_context.add_entry_if_missing
      ~ctx:(Provider_utils.ctx_from_server_env env)
      ~path
  in
  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_quarantined ~ctx ~entry
  in
  let symbols =
    IdentifySymbolService.all_symbols
      ctx
      tast.Tast_with_dynamic.under_normal_assumptions
  in
  Asserter.Int_asserter.assert_equals
    7
    (List.length symbols)
    "symbol count for error file";
  ()
