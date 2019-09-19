let paths_to_ignore = ref []

let get_paths_to_ignore () = !paths_to_ignore

let set_paths_to_ignore x = paths_to_ignore := x

let ignore_path path = paths_to_ignore := Str.regexp path :: !paths_to_ignore

let should_ignore path =
  List.exists (fun p -> Str.string_match p path 0) !paths_to_ignore
