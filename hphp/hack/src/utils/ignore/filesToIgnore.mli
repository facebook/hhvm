val get_paths_to_ignore : unit -> Str.regexp list

val set_paths_to_ignore : Str.regexp list -> unit

val should_ignore : string -> bool

val ignore_path : string -> unit
