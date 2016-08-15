type connection_type =
  | Persistent
  | Non_persistent

type connection_response =
  | Persistent_client_connected
  | Persistent_client_alredy_exists

type 'a command =
  | Rpc of 'a ServerRpc.t
  | Stream of streamed
  | Debug

and streamed =
  | SHOW of string
  | LIST_FILES
  | LIST_MODES
  | BUILD of ServerBuild.build_opts

(** Timeout on reading the command from the client - client probably frozen. *)
exception Read_command_timeout
