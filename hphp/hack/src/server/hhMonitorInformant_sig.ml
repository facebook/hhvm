(**
 * Put common type definitions outside of the module type signature so it can
 * be included in both the .mli and .ml.
 *)
module Types = struct
  type options = {
    root : Path.t;
    allow_subscriptions : bool;
    (** Disable the informant - use the dummy instead. *)
    use_dummy : bool;
  }
  type init_env = options
end

module type S = sig
  type t
  include module type of Types
  include Informant_sig.S with type t := t and type init_env := init_env
end

(** A prefetcher downloads saved states for us. *)
module type Prefetcher_S = sig
  type t
  type svn_rev = int
  val dummy : t
  (**
   * Create a Prefetcher from the path to a script.
   * Script will be invoked by shelling out to subprocess.
   * First argument is the SVN revision to prefetch for.
   *)
  val of_script : Path.t -> t

  (**
   * Run the prefetcher asynchronously within a spawned daemon.
   *
   * Note: you can safely drop the returned process handle. It is a
   * separate process that is responsible for its own logging. The
   * prefetcher will complete (or fail) asynchronously.
   *)
  val run : svn_rev -> t -> Process_types.t

end
