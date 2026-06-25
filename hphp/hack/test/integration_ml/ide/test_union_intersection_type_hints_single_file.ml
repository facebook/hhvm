(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Regression test for a bug on the single-file / IDE typecheck path
   ([Typing_toplevel.nast_to_tast], used by [hh --single] and IDE diagnostics).

   That path used to skip applying a file's
   [<<file: __EnableUnstableFeatures(...)>>] attribute to the typechecker
   options, so a definition that legitimately opted into an unstable feature
   gated by [__GatedByFeatureFlag] spuriously produced GatedByFeatureFlag
   (Typing[4509]) under [hh --single] / IDE, even though the full check (which
   applies the attribute via [Typing_toplevel.set_tcopt_unstable_features]) was
   clean.

   This drives the single-file path via [Test.Client.open_file] and asserts no
   such spurious error. The test config leaves [experimental_features] empty, so
   feature gating depends solely on the per-file attribute being honored --
   exactly the code path under test.

   The feature name here ('union_intersection_type_hints') is just a label for
   the [__GatedByFeatureFlag] gate; the test does not use union/intersection
   type-hint syntax. Any registered, non-released unstable feature works -- we
   pick a durable one. *)

module Test = Integration_test_base

let gated_def_name = "gated_def.php"

(* Stands in for a gated declaration that lives elsewhere (e.g. a newtype in an
   hhi guarded by [__GatedByFeatureFlag]). *)
let gated_def_contents =
  "<?hh
<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

<<__GatedByFeatureFlag('union_intersection_type_hints')>>
newtype MyGated<T> = T;
"

let use_name = "use_gated.php"

(* A file that enables the unstable feature at file scope and references the
   gated type. *)
let use_contents =
  "<?hh
<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

type UseGated = MyGated<int>;
"

let test () =
  (* union_intersection_type_hints is an Unstable feature, so the parser only
     accepts <<file: __EnableUnstableFeatures(...)>> for it when
     [allow_unstable_features] is set. We leave [experimental_features] empty so
     the feature is not OngoingRelease, keeping the typing-phase gate
     ([is_unstable_feature_enabled]) dependent solely on the per-file attribute
     being applied -- exactly the code path under test. *)
  let po = ParserOptions.{ default with allow_unstable_features = true } in
  let global_opts : GlobalOptions.t =
    GlobalOptions.set ~po GlobalOptions.default
  in
  let custom_config = ServerConfig.default_config in
  let custom_config = ServerConfig.set_tc_options custom_config global_opts in
  let custom_config = ServerConfig.set_parser_options custom_config po in
  Test.Client.with_env ~custom_config:(Some custom_config) @@ fun env ->
  let env =
    Test.Client.setup_disk
      env
      [(gated_def_name, gated_def_contents); (use_name, use_contents)]
  in
  (* Opening the using file drives [Typing_toplevel.nast_to_tast]. Because the
     file enables the feature, referencing the gated newtype must NOT raise
     GatedByFeatureFlag (Typing[4509]). *)
  let (env, diagnostics) = Test.Client.open_file env use_name in
  Test.Client.assert_no_diagnostics diagnostics;
  ignore env;
  ()
