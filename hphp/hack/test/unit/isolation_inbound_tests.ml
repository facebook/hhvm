(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(* The dependency graph is empty in this fixture: [Common_setup] decls the files
   but never type checks them, and typing deps are recorded while checking. So
   every closure here is the file alone, and these cases pin the memo and the
   bound rather than reachability — except for the [referrers] cases at the
   bottom, which record the one edge they need. What the walk reaches over a
   real graph is covered by the integration test in the diff that grows
   clusters. *)
let setup () =
  Common_setup.setup ~sqlite:false Global_options.default ~xhp_as:`Namespaces

let test_closure_of_an_unreferenced_file_is_just_itself () =
  Server_isolation_inbound.reset ();
  let { Common_setup.ctx; foo_path; naming_table; _ } = setup () in
  match Server_isolation_inbound.get ctx naming_table ~max:100 foo_path with
  | None -> false
  | Some closure ->
    (* Pins that the file itself is in its own closure, which is what makes the
       set safe to absorb whole. Says nothing about reachability: with no graph
       every closure is a singleton. *)
    Relative_path.Set.equal closure (Relative_path.Set.singleton foo_path)

let test_second_request_is_memoised () =
  Server_isolation_inbound.reset ();
  let { Common_setup.ctx; bar_path; naming_table; _ } = setup () in
  let first = Server_isolation_inbound.get ctx naming_table ~max:100 bar_path in
  let after_one = Server_isolation_inbound.closures_computed () in
  let second =
    Server_isolation_inbound.get ctx naming_table ~max:100 bar_path
  in
  let after_two = Server_isolation_inbound.closures_computed () in
  (* The count is the assertion. A closure is a global fact, so the second
     request must be served rather than recomputed however many clusters ask. *)
  Int.equal after_one 1
  && Int.equal after_two 1
  && Option.equal Relative_path.Set.equal first second

let test_refusals_are_memoised_too () =
  Server_isolation_inbound.reset ();
  let { Common_setup.ctx; bar_path; naming_table; _ } = setup () in
  let first = Server_isolation_inbound.get ctx naming_table ~max:0 bar_path in
  let second = Server_isolation_inbound.get ctx naming_table ~max:0 bar_path in
  (* A refusal costs as much as an answer, and widely-used files are asked about
     repeatedly, so [None] has to be cached rather than recomputed. *)
  Option.is_none first
  && Option.is_none second
  && Int.equal (Server_isolation_inbound.closures_computed ()) 1

let test_reset_drops_the_memo () =
  Server_isolation_inbound.reset ();
  let { Common_setup.ctx; bar_path; naming_table; _ } = setup () in
  let (_ : Relative_path.Set.t option) =
    Server_isolation_inbound.get ctx naming_table ~max:100 bar_path
  in
  Server_isolation_inbound.reset ();
  let cleared = Int.equal (Server_isolation_inbound.closures_computed ()) 0 in
  let (_ : Relative_path.Set.t option) =
    Server_isolation_inbound.get ctx naming_table ~max:100 bar_path
  in
  (* Recomputing is what shows the entry went: had it survived, this request
     would have been served and the count would still be 0. *)
  cleared && Int.equal (Server_isolation_inbound.closures_computed ()) 1

let test_referrers_answers_for_the_whole_candidate_at_once () =
  Server_isolation_inbound.reset ();
  let { Common_setup.ctx; foo_path; bar_path; naming_table; _ } = setup () in
  let candidate = Relative_path.Set.of_list [foo_path; bar_path] in
  let (_ : Relative_path.Set.t) =
    Server_isolation_inbound.referrers ctx naming_table candidate
  in
  (* The count is the assertion, and it is the point of the walk: answering for
     two files must not cost two closures. [referrers] does not touch the
     per-file memo at all, so nothing is recorded against it. *)
  Int.equal (Server_isolation_inbound.closures_computed ()) 0

(* The fixture's graph is empty because nothing type checks it, so the one edge
   a reachability case needs is recorded by hand. [Foo::foo] really does take a
   [Bar], so this records the dependency the fixture already describes rather
   than inventing one.

   Restored afterwards: the graph is process-wide, and a case that left an edge
   behind would decide the next one. *)
let with_foo_depending_on_bar ctx f =
  let deps_mode = Provider_context.get_deps_mode ctx in
  let previously_allowed =
    Typing_deps.allow_dependency_table_reads deps_mode true
  in
  Base.Exn.protect
    ~f:(fun () ->
      Typing_deps.add_idep
        deps_mode
        (Typing_deps.Dep.Type "\\Foo")
        (Typing_deps.Dep.Type "\\Bar");
      Typing_deps.flush_deps deps_mode;
      f ())
    ~finally:(fun () ->
      let (_ : bool) =
        Typing_deps.allow_dependency_table_reads deps_mode previously_allowed
      in
      Typing_deps.replace deps_mode)

let test_referrers_names_the_file_that_references_in () =
  Server_isolation_inbound.reset ();
  let { Common_setup.ctx; foo_path; bar_path; naming_table; _ } = setup () in
  with_foo_depending_on_bar ctx @@ fun () ->
  let candidate = Relative_path.Set.singleton bar_path in
  let found = Server_isolation_inbound.referrers ctx naming_table candidate in
  Relative_path.Set.equal found (Relative_path.Set.singleton foo_path)

let test_referrers_of_a_candidate_holding_both_ends_is_empty () =
  Server_isolation_inbound.reset ();
  let { Common_setup.ctx; foo_path; bar_path; naming_table; _ } = setup () in
  with_foo_depending_on_bar ctx @@ fun () ->
  let candidate = Relative_path.Set.of_list [foo_path; bar_path] in
  (* Empty is the move's green light, so it has to mean "nothing references
     in" and not "nothing was looked at". The previous case shares the graph
     and finds the edge, which is what rules that out. *)
  Relative_path.Set.is_empty
    (Server_isolation_inbound.referrers ctx naming_table candidate)

let tests =
  [
    ( "test_closure_of_an_unreferenced_file_is_just_itself",
      test_closure_of_an_unreferenced_file_is_just_itself );
    ("test_second_request_is_memoised", test_second_request_is_memoised);
    ("test_refusals_are_memoised_too", test_refusals_are_memoised_too);
    ("test_reset_drops_the_memo", test_reset_drops_the_memo);
    ( "test_referrers_answers_for_the_whole_candidate_at_once",
      test_referrers_answers_for_the_whole_candidate_at_once );
    ( "test_referrers_names_the_file_that_references_in",
      test_referrers_names_the_file_that_references_in );
    ( "test_referrers_of_a_candidate_holding_both_ends_is_empty",
      test_referrers_of_a_candidate_holding_both_ends_is_empty );
  ]

let () =
  let config =
    Shared_mem.
      {
        global_size = 1024;
        heap_size = 1024 * 1024 * 8;
        hash_table_pow = 14;
        shm_dirs = [];
        shm_use_sharded_hashtbl = false;
        shm_cache_size = -1;
        shm_min_avail = 0;
        log_level = 0;
        sample_rate = 0.0;
        compression = 0;
      }
  in
  Event_logger.init_fake ();
  let (_ : Shared_mem.handle) = Shared_mem.init config ~num_workers:0 in
  tests
  |> List.map ~f:(fun (name, do_) ->
         ( name,
           fun () ->
             Utils.with_context
               ~enter:Provider_backend.set_shared_memory_backend
               ~exit:(fun () -> ())
               ~do_ ))
  |> Unit_test.run_all
