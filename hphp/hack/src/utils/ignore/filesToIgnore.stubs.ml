module J = Hh_json_helpers.AdhocJsonHelpers

let paths_to_ignore = ref []

let get_paths_to_ignore () = !paths_to_ignore

let set_paths_to_ignore x = paths_to_ignore := x

let ignore_path regexp = paths_to_ignore := regexp :: !paths_to_ignore

let should_ignore path =
  List.exists (fun p -> Str.string_match p path 0) !paths_to_ignore

let watchman_monitor_expression_terms =
  [
    J.pred "not"
    @@ [
         J.pred "anyof"
         @@ [
              J.strlist ["name"; ".hg"];
              J.strlist ["dirname"; ".hg"];
              J.strlist ["name"; ".git"];
              J.strlist ["dirname"; ".git"];
              J.strlist ["name"; ".svn"];
              J.strlist ["dirname"; ".svn"];
            ];
       ];
  ]

let hg_dirname = J.strlist ["dirname"; ".hg"]

let git_dirname = J.strlist ["dirname"; ".git"]

let svn_dirname = J.strlist ["dirname"; ".svn"]

let watchman_server_expression_terms =
  [
    J.strlist ["type"; "f"];
    J.pred "anyof"
    @@ [
         J.strlist ["name"; ".hhconfig"];
         J.pred "anyof"
         @@ [
              J.strlist ["suffix"; "php"];
              J.strlist ["suffix"; "phpt"];
              J.strlist ["suffix"; "hack"];
              J.strlist ["suffix"; "hackpartial"];
              J.strlist ["suffix"; "hck"];
              J.strlist ["suffix"; "hh"];
              J.strlist ["suffix"; "hhi"];
              J.strlist ["suffix"; "xhp"];
            ];
       ];
    J.pred "not" @@ [J.pred "anyof" @@ [hg_dirname; git_dirname; svn_dirname]];
  ]

let watchman_watcher_expression_terms =
  [J.strlist ["type"; "f"]; J.strlist ["name"; "updatestate"]]
