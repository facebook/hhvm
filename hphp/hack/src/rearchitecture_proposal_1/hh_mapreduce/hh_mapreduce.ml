(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** hh_mapreduce is scheduled to be deleted in 2020, once the typechecker has
    migrated to Rust. At that time it will be replaced by a collection of
    smaller wholly independent executables. Every aspect of hh_mapreduce is
    solely an artifact of ocaml compatibility.

    How it works: (1) the local decl service invokes "hh_mapreduce prototype <sharedmem>"
    and this launches a persistent singleton process, the "prototype", which
    maps the sharedmem to some base address and then listens on
    /tmp/hh_server/[root].prototype.sock unix domain socket for requests.
    (2) Someone invokes "hh_mapreduce typecheck" or some other mode. There are
    several modes. This orchestrates a "map-reduce" using multiple
    worker processes, each worker process being able to typecheck files.
    The way it recruits new worker processes is by connecting to that
    prototype.sock socket. (3) When the prototype accepts a connection on that
    socket, it forks a worker process. The worker has the shared-memory already
    mapped. It communicates over the socket with its orchestrator.

    Why is it like this? [A] While the typechecker is in ocaml, the decl blobs
    we store in sharedmem are native ocaml format blobs. Thus, the content of
    the blob that the worker maps at (say) 0x4000 will have pointers to other
    addresses near 0x4000. You can appreciate that every worker who maps sharedmem
    must therefore map it at exactly the same address. [B] The only way to guarantee
    multiple processes mapping the same address, is if they all fork after the
    sharedmem has already been mapped. [C] Therefore, the prototype that forks
    those workers must have the power to dispatch to "typecheck" and all the other
    modes of map-reduce: all modes must be in a single binary. And the single binary
    must also include the prototype server.

    In future, when we no longer store native ocaml format blobs and instead store
    decls in sharedmem using our own relative memory format, then we'll no longer
    need a "prototype", and we'll happily be able to split all the different
    kinds of hh_mapreduce into their own separate executables, and this whole
    hh_mapreduce executable will go away. *)

open Core_kernel

let () =
  (* Writing to a closed socket fd causes SIGPIPE, which normally terminates. *)
  (* We'll instead just ignore it and get back an error on the write. *)
  (* This will apply to both prototype and workers. *)
  Sys_utils.set_signal Sys.sigpipe Sys.Signal_ignore;

  let command =
    if Array.length Sys.argv < 2 then
      "<unspecified>"
    else
      Sys.argv.(1)
  in
  (* We have a global dispatch-list that (1) maps between the well-typed
     Dispatch.kind <-> strings as used by users and json; (2) says how
     to invoke the main orchestrator, and workers, based on Dispatch.kind.
     This list is needed when requesting new workers to be forked, when prototype
     dispatches new workers, and now when we pick which orchestrator to run. *)
  Dispatch.(
    register
      [
        {
          name = "prototype";
          kind = Dispatch.Prototype;
          run = Prototype.run;
          run_worker = (fun _ -> failwith "Shouldn't ask for prototype worker");
        };
        {
          name = "typecheck";
          kind = Dispatch.Typecheck;
          run = Typecheck.run;
          run_worker = Typecheck.run_worker;
        };
      ]);

  match Dispatch.find_by_name command with
  | Some info -> info.Dispatch.run ()
  | None ->
    Printf.eprintf "Unrecognized %s\n" command;
    exit 1
