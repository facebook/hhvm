module C = Configurator.V1

let () =
  C.main ~name:"build_options" (fun (_ : C.t) ->
      let sysconfdir = Sys.getenv "CMAKE_INSTALL_FULL_SYSCONFDIR"
      and bindir = Sys.getenv "CMAKE_INSTALL_FULL_BINDIR" in
      C.Flags.write_lines
        "generated_buildOptions.ml"
        [
          "let system_config_path = \"" ^ sysconfdir ^ "\"";
          "let default_hackfmt_path = \"" ^ bindir ^ "/hackfmt\"";
        ])
