(*
 * These are the arguments that integration test binaries using Test_harness
 * will want to parse.
 *)

type t = {
  hh_server: Path.t;
  hh_client: Path.t;
  template_repo: Path.t;
}

let usage =
  Printf.sprintf
    "Usage: %s --hh-server <%s> --hh-client <%s> [template repo]\n"
    "hh_server_path"
    "hh_client_path"
    Sys.argv.(0)

let usage_exit () =
  Printf.eprintf "%s\n" usage;
  exit 1

let string_spec str_ref = Arg.String (fun x -> str_ref := Some x)

let requires name opt =
  match !opt with
  | None ->
    let () = Printf.eprintf "Missing required argument: %s\n" name in
    usage_exit ()
  | Some x -> x

let parse () =
  let template_repo = ref None in
  let hh_server = ref None in
  let hh_client = ref None in
  let options =
    [
      ("--hh-server", string_spec hh_server, "Path to hh_server");
      ("--hh-client", string_spec hh_client, "Path to hh_client");
    ]
  in
  let () = Arg.parse options (fun s -> template_repo := Some s) usage in
  let template_repo = requires "template repo" template_repo in
  let hh_server = requires "hh_server" hh_server in
  let hh_client = requires "hh_client" hh_client in
  {
    hh_server = Path.make hh_server;
    hh_client = Path.make hh_client;
    template_repo = Path.make template_repo;
  }
