(*
 * Copyright (c) Facebook, Inc. and its affiliates.
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
      type t = ParserOptions.t

      let prefix = Prefix.make ()

      let description = "Parser_options_provider"
    end)

let local_memory_popt : ParserOptions.t option ref = ref None

let get_opt () : ParserOptions.t option =
  match Provider_backend.get () with
  | Provider_backend.Shared_memory -> Store.get ()
  | Provider_backend.Local_memory _
  | Provider_backend.Decl_service _ ->
    !local_memory_popt

let get () : ParserOptions.t =
  match get_opt () with
  | Some popt -> popt
  | None ->
    Hh_logger.fatal "%s"
    @@ "Parser_options_provider: Attempt to read global parser options "
    ^ "before they were set";
    failwith "Parser_options_provider: global parser options not set"

let set (popt : ParserOptions.t) : unit =
  match get_opt () with
  | None ->
    begin
      match Provider_backend.get () with
      | Provider_backend.Shared_memory -> Store.add () popt
      | Provider_backend.Local_memory _
      | Provider_backend.Decl_service _ ->
        local_memory_popt := Some popt
    end
  | Some _ ->
    (* We end up invoking ServerInit.init after setting the global parser
       options in the Unit_build tests, so we only log here instead of throwing
       an exception. *)
    Hh_logger.log "WARNING: Attempt to change global parser options";
    Hh_logger.log "%s"
    @@ "This is not permitted--the global parser options cannot change "
    ^ "during the lifetime of the server.";
    Hh_logger.log "%s" @@ "The new settings have been ignored.";
    Hh_logger.log
      "%s"
      (Printexc.raw_backtrace_to_string (Printexc.get_callstack 100))
