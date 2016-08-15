open Integration_test_base_types

val setup_server: unit -> ServerEnv.env
val run_loop_once:
  ServerEnv.env -> loop_inputs -> (ServerEnv.env * loop_outputs)

val fail: string -> unit
val assertEqual: string -> string -> unit
