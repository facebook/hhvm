module Types = struct
  exception NotADirectory of string

  exception No_such_file_or_directory of string

  exception Rename_target_already_exists of string

  exception Rename_target_dir_not_empty of string
end

module type S = sig
  include module type of Types

  val is_real_disk : bool

  val cat : string -> string

  val write_file : file:string -> contents:string -> unit

  val file_exists : string -> bool

  (* Delete the given path - if it is a directory, delete recurisvely. *)
  val rm_dir_tree : string -> unit

  val is_directory : string -> bool

  val getcwd : unit -> string

  val chdir : string -> unit

  (* Changes the permissions on a path, which could be a file or a directory.
      The integer value is the permissions mode. If `recursive` is true, then
      applies the permissions mode to the directory and all of its contents,
      recursively.
      See the Unix documentation for the related `chmod` command. *)
  val chmod : recursive:bool -> string -> int -> unit

  val mkdir : string -> int -> unit

  (* Equivalent to `mkdir -p` - creates subdirectories recursively if they
     don't exist. *)
  val mkdir_p : string -> unit

  (* Return the paths of all the regular files present in the given directory
      and its subdirectories. *)
  val readpath : string -> string list

  (* Return the names of all files present in the given directory. *)
  val readdir : string -> string array

  (* Rename from old path to new path. *)
  val rename : string -> string -> unit

  (* Size in bytes of a file or a directory and its contents, recursively *)
  val treesize : string -> int

  (* Modified time of a given file *)
  val filemtime : string -> float
end
