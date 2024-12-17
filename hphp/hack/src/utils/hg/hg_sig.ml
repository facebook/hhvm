module type Types_S = sig
  exception Malformed_result

  module Rev : sig
    type t [@@deriving eq, show, yojson]

    val to_string : t -> string

    val of_string : string -> t

    val of_string_check_empty : string -> t option
  end

  type global_rev = int [@@deriving eq, show, yojson]

  type rev =
    | Hg_rev of Rev.t
    | Global_rev of global_rev
  [@@deriving eq, show]

  module Rev_comparator : sig
    type t = rev

    val to_string : t -> string

    val is_equal : t -> t -> bool
  end
end

module Types = struct
  open Ppx_yojson_conv_lib.Yojson_conv.Primitives

  exception Malformed_result

  module Rev = struct
    type t = string [@@deriving show, yojson]

    let equal rev1 rev2 =
      String.starts_with rev1 ~prefix:rev2
      || String.starts_with rev2 ~prefix:rev1

    let to_string x = x

    let of_string x = x

    let of_string_check_empty = function
      | "" -> None
      | x -> Some x
  end

  (** This is a monotonically increasing revision number. *)
  type global_rev = int [@@deriving eq, show, yojson]

  type rev =
    | Hg_rev of Rev.t
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
  include Types_S

  val get_old_version_of_files :
    rev:rev -> files:string list -> out:string -> repo:string -> unit Future.t

  val get_hg_revision_time : rev -> string -> int Future.t

  val current_mergebase_hg_rev : string -> Rev.t Future.t

  (** [current_working_copy_hg_rev repo] gets the hg revision hash of the
      current working copy in the repo dir.
      The boolean returned indicates if there are working copy changes.

  Similar to
        hg id -i --cwd <repo> *)
  val current_working_copy_hg_rev : string -> (Rev.t * bool) Future.t

  (** Get the global base revision of the current working copy in the given
   * repo dir. *)
  val current_working_copy_base_rev : string -> global_rev Future.t

  (** The globalrev of a given revision. If that revision is not public,
  the globalrev of `parents(roots(draft() & ::<rev>))`, and if there are merge conflicts,
  the globalrev of the mergebase. *)
  val get_closest_global_ancestor : Rev.t -> string -> global_rev Future.t

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

  val hg_root : unit -> string Future.t

  module Mocking : sig
    val current_working_copy_hg_rev_returns : (Rev.t * bool) Future.t -> unit

    val current_working_copy_base_rev_returns : global_rev Future.t -> unit

    val get_hg_revision_time : Rev.t -> string -> int Future.t

    val reset_current_working_copy_base_rev_returns : unit -> unit

    val closest_global_ancestor_bind_value :
      Rev.t -> global_rev Future.t -> unit

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
