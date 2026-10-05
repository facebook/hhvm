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
   bound rather than reachability. What the walk actually reaches is covered by
   the integration test in the diff that grows clusters. *)
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

let tests =
  [
    ( "test_closure_of_an_unreferenced_file_is_just_itself",
      test_closure_of_an_unreferenced_file_is_just_itself );
    ("test_second_request_is_memoised", test_second_request_is_memoised);
    ("test_refusals_are_memoised_too", test_refusals_are_memoised_too);
    ("test_reset_drops_the_memo", test_reset_drops_the_memo);
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
