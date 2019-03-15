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
  type t = TypecheckerOptions.t
  let prefix = Prefix.make ()
  let description = "GlobalNamingOptions"
end)

let get () =
  match Store.get () with
  | Some tcopt -> tcopt
  | None ->
    Hh_logger.fatal "%s" @@
      "GlobalNamingOptions: Attempt to read global naming options " ^
      "before they were set";
    failwith "GlobalNamingOptions: global naming options not set"

let set tcopt =
  match Store.get () with
  | None -> Store.add () tcopt
  | Some _ ->
    (* We end up invoking ServerInit.init after setting the global naming
       options in the Unit_build tests, so we only log here instead of throwing
       an exception. *)
    Hh_logger.log
      "WARNING: Attempt to change global naming options";
    Hh_logger.log "%s" @@
      "This is not permitted--the global naming options cannot change " ^
      "during the lifetime of the server.";
    Hh_logger.log "%s" @@
      "The new settings have been ignored.";
    Hh_logger.log "%s"
      (Printexc.raw_backtrace_to_string (Printexc.get_callstack 100))
