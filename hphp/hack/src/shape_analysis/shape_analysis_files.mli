open Hh_prelude
module T = Shape_analysis_types

val constraints_file_extension : string

(** write constraints to `constraints_dir`. *)
val write_constraints :
  constraints_dir:string -> worker:int -> T.ConstraintEntry.t -> unit

(* Flush the channel used to write constraints.
Only needed if you need to read constraints right away. *)
val flush : unit -> unit

(**
  Ephemeral sequence where each step is a list of constraint entries that came
  from the same source file.
*)
val read_entries_by_source_file :
  constraints_file:Base.string -> T.ConstraintEntry.t list Sequence.t

(**
  Ephemeral sequence where each step is a list of constraint entries that came
  from the same callable
*)
val read_entries_by_callable :
  constraints_file:Base.string -> T.ConstraintEntry.t list Sequence.t
