(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val only : string -> string -> unit
(** e.g. Args.parse options (only "foo"), to show that the only anonymous
     argument allowed is "foo" (an anonymous argument is one without -), and that
     "foo" may only be provided at most once.
     Specifically: only, given a string, returns a function which may only be invoked once, and only with that string *)

val root : string ref -> string * Arg.spec * string
(** e.g. Args.parse [root my_ref] anon, to show that "--root" argument can be used.
    If present, and if it fails to specify a directory containing .hhconfig,
    then Args.parse will throw an exception. *)

val prototype_lock_file : string -> string
(** Minor helper function for the '--root' argument, to get the filename of the
    corresponding /tmp/hh_server/[root].prototype.lock file.
    e.g. "Lock.grab (prototype_lock_file root)"
    This is a file used by prototype to guarantee only a single instance of
    prototype is running per project roop.
    The way it works: when 'hh_mapreduce prototype' starts up, it attempts to
    grab an exclusive unix readwrite lock on that file. It never releases it,
    except that the OS automatically releases the lock when the process
    terminates. If the attempt to grab fails, then prototype knows that another
    instance is still alive, and so gives up. *)

val prototype_sock_file : string -> string
(** Minor helper function for the '--root' argument, to get the filename of the
    corresponding /tmp/hh_server/[root].prototype.sock file.
    e.g. "Unix.ADDR_UNIX (Args.prototype_sock_file root)"
    This socket is used by an orchestrator to recruit additional workers from the
    prototype and then communicate with those workers.
    The way it works: once 'hh_mapreduce prototype' has ensured that it's a
    singleton thanks to prototype_lock_file, then it creates this socket
    file and waits for connections.
    Any future orchestrator who wants to recruit a worker will use Protype.rpc_request_new_worker,
    which opens a connection to the second, and sends an initial handshake message
    saying what kind of worker it wants (in JSON). In response the prototype
    will fork a worker, and the orchestrator can communicate with the worker
    over the socket fd. *)
