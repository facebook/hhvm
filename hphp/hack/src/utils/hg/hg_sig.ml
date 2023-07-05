module Types = struct
  exception Malformed_result

  type hg_rev = string [@@deriving eq, show]

  (** This is a monotonically increasing revision number. *)
  type global_rev = int [@@deriving eq, show]

  type rev =
    | Hg_rev of hg_rev
    | Global_rev of global_rev
  [@@deriving eq, show]

  module Rev_comparator = struct
    type t = rev

    let to_string v =
      match v with
      | Hg_rev s -> Printf.sprintf "Hg_rev %s" s
      | Global_rev i -> Printf.sprintf "Global_rev %d" i

    let is_equal exp actual =
      (* Avoid polymorphic equal. *)
      match (exp, actual) with
      | (Hg_rev exp, Hg_rev actual) -> exp = actual
      | (Global_rev exp, Global_rev actual) -> exp = actual
      | _ -> false
  end
end

module type S = sig
  include module type of Types

  val get_old_version_of_files :
    rev:rev -> files:string list -> out:string -> repo:string -> unit Future.t

  val get_hg_revision_time : rev -> string -> int Future.t

  val current_mergebase_hg_rev : string -> hg_rev Future.t

  (** [current_working_copy_hg_rev repo] gets the hg revision hash of the
      current working copy in the repo dir.
      The boolean returned indicates if there are working copy changes.

      Similar to
        hg id -i --cwd <repo> *)
  val current_working_copy_hg_rev : string -> (hg_rev * bool) Future.t

  (** Get the global base revision of the current working copy in the given
   * repo dir. *)
  val current_working_copy_base_rev : string -> global_rev Future.t

  val get_closest_global_ancestor : hg_rev -> string -> global_rev Future.t

  val files_changed_since_rev :
    rev -> (* repository path. *)
           string -> string list Future.t

  val files_changed_in_rev :
    rev -> (* repository path. *)
           string -> string list Future.t

  val files_changed_since_rev_to_rev :
    start:rev ->
    finish:rev ->
    (* repository path. *)
    string ->
    string list Future.t

  (** hg update to the base global revision. *)
  val update_to_rev : rev -> string -> unit Future.t

  module Mocking : sig
    val current_working_copy_hg_rev_returns : (hg_rev * bool) Future.t -> unit

    val current_working_copy_base_rev_returns : global_rev Future.t -> unit

    val get_hg_revision_time : hg_rev -> string -> int Future.t

    val reset_current_working_copy_base_rev_returns : unit -> unit

    val closest_global_ancestor_bind_value :
      hg_rev -> global_rev Future.t -> unit

    val files_changed_since_rev_returns :
      rev:rev -> string list Future.t -> unit

    val files_changed_in_rev_returns : rev:rev -> string list Future.t -> unit

    val files_changed_since_rev_to_rev_returns :
      start:rev -> finish:rev -> string list Future.t -> unit

    val reset_files_changed_since_rev_to_rev_returns : unit -> unit

    val reset_files_changed_since_rev_returns : unit -> unit

    val reset_files_changed_in_rev_returns : unit -> unit
  end
end
