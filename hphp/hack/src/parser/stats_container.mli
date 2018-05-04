(* mutable stats container
keeps track of the count and the sum for
string-keyed values *)

type t [@@deriving show]

exception NoSuchEntry of string

(* create a fresh container *)
val new_container : unit -> t

(* write a "nicely" formatted table to the given channel *)
val write_out : out:out_channel -> t -> unit

(* wrap a nullary function with timing info *)
(* for stats, use the global instance if no stats instance is given *)
val wrap_nullary_fn_timing : ?stats:t -> key:string -> f:(unit -> 'a) -> 'a

(* set the global stats instance *)
val set_instance : t option -> unit

(* get the global stats instance *)
val get_instance : unit -> t option
