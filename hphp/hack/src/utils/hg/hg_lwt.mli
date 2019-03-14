type rev

val rev_to_string: rev -> string
(** Get a human-readable representation of the revision identifier. *)

val get_hash_from_rev: rev -> string
(** Get the Mercurial commit hash for this revision identifier.

It's recommended to wait on calling this function until the last possible moment
in your program, so that you don't mix up a regular string with the revision
identifier. *)

val get_latest_ancestor_public_commit: repo:Path.t -> (rev, string) Lwt_result.t
