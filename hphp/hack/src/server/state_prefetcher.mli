(** A prefetcher downloads saved states for us. *)

type t
type svn_rev = int
val dummy : t

(**
 * Create a Prefetcher from the path to a script.
 * Script will be invoked by shelling out to subprocess.
 * First argument is the SVN revision to prefetch for.
 *)
val of_script_opt : Path.t option -> t

(**
 * Run the prefetcher asynchronously within a spawned daemon.
 *
 * Note: you can safely drop the returned process handle. It is a
 * separate process that is responsible for its own logging. The
 * prefetcher will complete (or fail) asynchronously.
 *)
val run : svn_rev -> t -> Process_types.t
