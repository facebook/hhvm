(* Reads JSON RPC messages from stdin in the background and timestamps them as
   they come in. *)

type t

type client_message = {
  (* The timestamp field is added when this message is read. It's not part of
     the JSON RPC spec. *)
  timestamp : float;

  method_ : string;
  id : Hh_json.json option;
  params : Hh_json.json option;
  is_request : bool;
}

type result =
  | Message of client_message
  | Error (* A recoverable error occurred (such as a parse error). *)
  | Exit (* A fatal error occurred, or the client has hung up. *)

(* Under the hood, this uses the Daemon module, so you must be sure to have
called `Daemon.entry_point` before trying to make a queue. *)
val make : unit -> t

(* Get the file descriptor that can be watched for a message coming in. *)
val get_read_fd : t -> Unix.file_descr

(* Whether we have a buffered message. *)
val has_message : t -> bool

(* Blocks until there is a message available, or an error occurs. *)
val get_message : t -> result
