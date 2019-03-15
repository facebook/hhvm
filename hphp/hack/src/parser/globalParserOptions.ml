(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module UnitKey = struct
  type t = unit
  let compare () () = 0
  let to_string () = "()"
end

module Store = SharedMem.WithCache (SharedMem.Immediate) (UnitKey) (struct
  type t = ParserOptions.t
  let prefix = Prefix.make ()
  let description = "GlobalParserOptions"
end)

let get () =
  match Store.get () with
  | Some popt -> popt
  | None ->
    Hh_logger.fatal "%s" @@
      "GlobalParserOptions: Attempt to read global parser options " ^
      "before they were set";
    failwith "GlobalParserOptions: global parser options not set"

let set popt =
  match Store.get () with
  | None -> Store.add () popt
  | Some _ ->
    (* We end up invoking ServerInit.init after setting the global parser
       options in the Unit_build tests, so we only log here instead of throwing
       an exception. *)
    Hh_logger.log
      "WARNING: Attempt to change global parser options";
    Hh_logger.log "%s" @@
      "This is not permitted--the global parser options cannot change " ^
      "during the lifetime of the server.";
    Hh_logger.log "%s" @@
      "The new settings have been ignored.";
    Hh_logger.log "%s"
      (Printexc.raw_backtrace_to_string (Printexc.get_callstack 100))
