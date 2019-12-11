(*
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

module Store =
  SharedMem.WithCache (SharedMem.Immediate) (UnitKey)
    (struct
      type t = TypecheckerOptions.t

      let prefix = Prefix.make ()

      let description = "Global_naming_options"
    end)

let local_memory_tcopt : TypecheckerOptions.t option ref = ref None

let get () : TypecheckerOptions.t =
  let tcopt_opt =
    match Provider_backend.get () with
    | Provider_backend.Shared_memory
    | Provider_backend.Local_memory _ ->
      Store.get ()
    | Provider_backend.Decl_service _ -> !local_memory_tcopt
  in
  match tcopt_opt with
  | Some tcopt -> tcopt
  | None ->
    Hh_logger.fatal "%s"
    @@ "Global_naming_options: Attempt to read global naming options "
    ^ "before they were set";
    failwith "Global_naming_options: global naming options not set"

let set (tcopt : TypecheckerOptions.t) : unit =
  let was_already_set =
    match Provider_backend.get () with
    | Provider_backend.Shared_memory
    | Provider_backend.Local_memory _ ->
      let was_already_set = Store.get () <> None in
      Store.add () tcopt;
      was_already_set
    | Provider_backend.Decl_service _ ->
      let was_already_set = !local_memory_tcopt <> None in
      local_memory_tcopt := Some tcopt;
      was_already_set
  in
  if was_already_set then (
    (* We end up invoking ServerInit.init after setting the global naming
       options in the Unit_build tests, so we only log here instead of throwing
       an exception. *)
    Hh_logger.log "WARNING: Attempt to change global naming options";
    Hh_logger.log "%s"
    @@ "This is not permitted--the global naming options cannot change "
    ^ "during the lifetime of the server.";
    Hh_logger.log "%s" @@ "The new settings have been ignored.";
    Hh_logger.log
      "%s"
      (Printexc.raw_backtrace_to_string (Printexc.get_callstack 100))
  )
