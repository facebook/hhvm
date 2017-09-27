module Types = struct
  exception Malformed_result
  type hg_rev = string
  type svn_rev = string
end

module type S = sig
  include module type of Types

  val get_old_version_of_files :
    rev:svn_rev ->
    files:string list ->
    out:string ->
    repo:string ->
    unit Future.t

  val current_working_copy_hg_rev : string ->
    (** bool indicates if there are working copy changes. *)
    (hg_rev * bool) Future.t

  (** Get the SVN base revision of the current working copy in the given
   * repo dir. *)
  val current_working_copy_base_rev : string ->
    svn_rev Future.t

  val get_closest_svn_ancestor : hg_rev -> string -> svn_rev Future.t
  val files_changed_since_svn_rev :
    svn_rev ->
    (** repository path. *)
    string ->
    (string list) Future.t

  (** hg update to the base svn revision. *)
  val update_to_base_rev : svn_rev -> string -> unit Future.t

  module Mocking : sig
    val current_working_copy_hg_rev_returns : (hg_rev * bool) Future.t -> unit
    val current_working_copy_base_rev_returns : svn_rev Future.t -> unit
    val closest_svn_ancestor_bind_value : hg_rev -> svn_rev Future.t -> unit
    val files_changed_since_svn_rev_returns : (string list) Future.t -> unit
  end
end
