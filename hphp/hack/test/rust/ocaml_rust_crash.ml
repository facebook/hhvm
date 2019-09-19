external rust_crash : unit -> unit = "rust_crash"

external rust_no_crash : unit -> unit = "rust_no_crash"

let parse_args () =
  let crash = ref false in
  let options = [("--crash", Arg.Set crash, "")] in
  Arg.parse options (fun _ -> ()) "";
  !crash

(*
Crash:
buck run @mode/dbg hphp/hack/test/rust:ocaml_rust_crash -- --crash
No crash:
buck run @mode/dbg hphp/hack/test/rust:ocaml_rust_crash
OR
buck run @mode/dev hphp/hack/test/rust:ocaml_rust_crash -- --crash
*)
let () =
  let crash = parse_args () in
  if crash then
    rust_crash ()
  else
    rust_no_crash ();
  print_endline "OK"
