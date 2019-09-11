module WEW = WatchmanEventWatcher
module Config = WatchmanEventWatcherConfig

let () = Random.self_init ()

module Args = struct
  type t = {
    root: Path.t;
    daemonize: bool;
    get_sockname: bool;
  }

  let usage =
    Printf.sprintf "Usage: %s [--daemonize] [REPO DIRECTORY]\n" Sys.argv.(0)

  let parse () =
    let root = ref None in
    let daemonize = ref false in
    let get_sockname = ref false in
    let options =
      [
        ("--daemonize", Arg.Set daemonize, "spawn watcher daemon");
        ("--get-sockname", Arg.Set get_sockname, "print socket and exit");
      ]
    in
    let () = Arg.parse options (fun s -> root := Some s) usage in
    match !root with
    | None ->
      Printf.eprintf "%s" usage;
      exit 1
    | Some root ->
      {
        root = Path.make root;
        daemonize = !daemonize;
        get_sockname = !get_sockname;
      }
end

let () =
  Daemon.check_entry_point ();
  let args = Args.parse () in
  if args.Args.get_sockname then
    Printf.printf "%s%!" (Config.socket_file args.Args.root)
  else if args.Args.daemonize then
    WEW.spawn_daemon args.Args.root
  else
    WEW.main args.Args.root
