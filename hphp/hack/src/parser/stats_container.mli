(* mutable stats container
keeps track of the count and the sum for
string-keyed values *)

type t

exception NoSuchEntry of string

(* create a fresh container *)
val new_container : unit -> t

(* record a single float observation *)
val record_float : t -> string -> float -> unit

(* record a single int observation *)
val record_int : t -> string -> int -> unit

(* get the count for a key *)
val get_count_exn : t -> string -> float

(* get the sum for a key *)
val get_sum_exn : t -> string -> float

(* get the arithmetic mean for a key *)
val get_mean_exn : t -> string -> float

(* write a "nicely" formatted table to the given channel *)
val write_out : out:out_channel -> t -> unit

(* wrap a unary function with timing info *)
(* for stats, use the global instance if no stats instance is given *)
val wrap_unary_fn_timing : ?stats:t -> key:string -> f:('a -> 'b) -> ('a -> 'b)

(* wrap a nullary function with timing info *)
(* for stats, use the global instance if no stats instance is given *)
val wrap_nullary_fn_timing : ?stats:t -> key:string -> f:(unit -> 'a) -> 'a

(* set the global stats instance *)
val set_instance : t option -> unit

(* get the global stats instance *)
val get_instance : unit -> t option
