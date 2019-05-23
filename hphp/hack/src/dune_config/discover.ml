(** This is a dune configurator:
https://jbuilder.readthedocs.io/en/latest/configurator.html *)

module C = Configurator.V1

let () =
  C.main ~name:"hphpdir" (fun (_c : C.t) ->
  let split = (fun s -> if s = "" then [] else String.split_on_char ' ' s) in
  let rpath = match Sys.getenv "FB_LD_OPTS" with
    | t -> let flags = split t in
      List.fold_left (fun acc x -> "-ccopt" :: x :: acc ) [] flags
    | exception Not_found -> [] in
  C.Flags.write_sexp "ld-opts.sexp" rpath)
