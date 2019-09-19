(** This is a dune configurator:
https://jbuilder.readthedocs.io/en/latest/configurator.html *)

module C = Configurator.V1

let () =
  C.main ~name:"hphpdir" (fun (c : C.t) ->
      let split s =
        if s = "" then
          []
        else
          String.split_on_char ' ' s
      in
      let flags =
        match Sys.getenv "FB_LD_OPTS" with
        | t ->
          let flags = split t in
          List.fold_left (fun acc x -> "-ccopt" :: x :: acc) [] flags
        | exception Not_found -> []
      in
      let flags =
        match C.ocaml_config_var_exn c "system" with
        (* ocaml builds with `-no_compact_unwind`, which breaks libunwind on
         * MacOS; we need libunwind to work for rust std::panic::catch_unwind.
         *
         * This fix is included in ocaml 4.08
         * (https://github.com/ocaml/ocaml/pull/8673) - but we're still on 4.07 *)
        | "macosx" -> flags @ ["-ccopt"; "-Wl,-keep_dwarf_unwind"]
        | _ -> flags
      in
      C.Flags.write_sexp "ld-opts.sexp" flags)
