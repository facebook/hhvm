let paths_to_ignore = ref []

let ignore_path path = paths_to_ignore := (Str.regexp path)::!paths_to_ignore

let should_ignore path =
  List.exists (fun p -> Str.string_match p path 0) !paths_to_ignore
