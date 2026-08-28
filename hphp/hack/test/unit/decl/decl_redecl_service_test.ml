(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

open Asserter
module Dep = Typing_deps.Dep
module DepSet = Typing_deps.DepSet

let deps_mode = Typing_deps_mode.InMemoryMode None

let module_dep = Dep.make (Dep.Module "test.module")

let consumer_dep = Dep.make (Dep.Fun "module_consumer")

let sentinel_dep = Dep.make (Dep.Fun "existing_fanout")

let errors_sentinel_dep = Dep.make (Dep.Fun "existing_error_fanout")

let make_stale_worker_fanout () : Fanout.t =
  {
    Fanout.changed = DepSet.singleton module_dep;
    to_recheck = DepSet.singleton sentinel_dep;
    to_recheck_if_errors = DepSet.singleton errors_sentinel_dep;
  }

let assert_membership
    (set : DepSet.t) (dep : Dep.t) ~(expected : bool) (message : string) : unit
    =
  Bool_asserter.assert_equals expected (DepSet.mem set dep) message

let with_master_dependency (f : unit -> 'a) : 'a =
  Typing_deps.replace deps_mode;
  let previous_reads_allowed =
    Typing_deps.allow_dependency_table_reads deps_mode true
  in
  Base.Exn.protect
    ~f:(fun () ->
      Typing_deps.add_idep
        deps_mode
        (Dep.Fun "module_consumer")
        (Dep.Module "test.module");
      Typing_deps.flush_deps deps_mode;
      f ())
    ~finally:(fun () ->
      let (_ : bool) =
        Typing_deps.allow_dependency_table_reads
          deps_mode
          previous_reads_allowed
      in
      Typing_deps.replace deps_mode)

let test_reconciles_master_dependencies () : bool =
  with_master_dependency @@ fun () ->
  let fanout = make_stale_worker_fanout () in
  let result =
    Decl_redecl_service.For_test.reconcile_parallel_redecl_fanout
      ~during_init:false
      ~deps_mode
      fanout
  in
  assert_membership
    result.Fanout.to_recheck
    consumer_dep
    ~expected:true
    "master-only dependent should be added";
  assert_membership
    result.Fanout.to_recheck
    module_dep
    ~expected:false
    "changed dependency should not be added to to_recheck";
  assert_membership
    result.Fanout.to_recheck
    sentinel_dep
    ~expected:true
    "existing worker fanout should be preserved";
  Int_asserter.assert_equals
    2
    (DepSet.cardinal result.Fanout.to_recheck)
    "only the consumer and existing worker fanout should be rechecked";
  assert_membership
    result.Fanout.changed
    module_dep
    ~expected:true
    "changed set should be preserved";
  Int_asserter.assert_equals
    1
    (DepSet.cardinal result.Fanout.changed)
    "changed set cardinality should be preserved";
  assert_membership
    result.Fanout.to_recheck_if_errors
    errors_sentinel_dep
    ~expected:true
    "error-only fanout should be preserved";
  Int_asserter.assert_equals
    1
    (DepSet.cardinal result.Fanout.to_recheck_if_errors)
    "error-only fanout cardinality should be preserved";
  true

let test_does_not_reconcile_during_init () : bool =
  with_master_dependency @@ fun () ->
  let fanout = make_stale_worker_fanout () in
  let result =
    Decl_redecl_service.For_test.reconcile_parallel_redecl_fanout
      ~during_init:true
      ~deps_mode
      fanout
  in
  assert_membership
    result.Fanout.to_recheck
    consumer_dep
    ~expected:false
    "guarded reconciliation should not add the master-only dependent";
  assert_membership
    result.Fanout.to_recheck
    sentinel_dep
    ~expected:true
    "guarded reconciliation should preserve worker fanout";
  Int_asserter.assert_equals
    1
    (DepSet.cardinal result.Fanout.to_recheck)
    "initialization should leave to_recheck unchanged";
  true

let () =
  Unit_test.run_all
    [
      ( "test_reconciles_master_dependencies",
        test_reconciles_master_dependencies );
      ( "test_does_not_reconcile_during_init",
        test_does_not_reconcile_during_init );
    ]
