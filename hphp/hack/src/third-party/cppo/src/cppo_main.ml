open Printf

let add_extension tbl s =
  let i =
    try String.index s ':'
    with Not_found ->
      failwith "Invalid -x argument"
  in
  let id = String.sub s 0 i in
  let raw_tpl = String.sub s (i+1) (String.length s - i - 1) in
  let cmd_tpl = Cppo_command.parse raw_tpl in
  if Hashtbl.mem tbl id then
    failwith ("Multiple definitions for extension " ^ id)
  else
    Hashtbl.add tbl id cmd_tpl

let semver_re = Str.regexp "\
\\([0-9]+\\)\
\\.\\([0-9]+\\)\
\\.\\([0-9]+\\)\
\\(-\\([^+]*\\)\\)?\
\\(\\+\\(.*\\)\\)?\
\r?$"

let parse_semver s =
  if not (Str.string_match semver_re s 0) then
    None
  else
    let major = Str.matched_group 1 s in
    let minor = Str.matched_group 2 s in
    let patch = Str.matched_group 3 s in
    let prerelease = try Some (Str.matched_group 5 s) with Not_found -> None in
    let build = try Some (Str.matched_group 7 s) with Not_found -> None in
    Some (major, minor, patch, prerelease, build)

let define var s =
  [sprintf "#define %s %s\n" var s]

let opt_define var o =
  match o with
  | None -> []
  | Some s -> define var s

let parse_version_spec s =
  let error () =
    failwith (sprintf "Invalid version specification: %S" s)
  in
  let prefix, version_full =
    try
      let len = String.index s ':' in
      String.sub s 0 len, String.sub s (len+1) (String.length s - (len+1))
    with Not_found ->
      error ()
  in
  match parse_semver version_full with
  | None ->
      error ()
  | Some (major, minor, patch, opt_prerelease, opt_build) ->
      let version = sprintf "(%s, %s, %s)" major minor patch in
      let version_string = sprintf "%s.%s.%s" major minor patch in
      List.flatten [
        define (prefix ^ "_MAJOR") major;
        define (prefix ^ "_MINOR") minor;
        define (prefix ^ "_PATCH") patch;
        opt_define (prefix ^ "_PRERELEASE") opt_prerelease;
        opt_define (prefix ^ "_BUILD") opt_build;
        define (prefix ^ "_VERSION") version;
        define (prefix ^ "_VERSION_STRING") version_string;
        define (prefix ^ "_VERSION_FULL") s;
      ]

let main () =
  let extensions = Hashtbl.create 10 in
  let files = ref [] in
  let header = ref [] in
  let incdirs = ref [] in
  let out_file = ref None in
  let preserve_quotations = ref false in
  let show_exact_locations = ref false in
  let show_no_locations = ref false in
  let options = [
    "-D", Arg.String (fun s -> header := ("#define " ^ s ^ "\n") :: !header),
    "DEF
          Equivalent of interpreting '#define DEF' before processing the
          input, e.g. `cppo -D 'VERSION \"1.2.3\"'` (no equal sign)";

    "-U", Arg.String (fun s -> header := ("#undef " ^ s ^ "\n") :: !header),
    "IDENT
          Equivalent of interpreting '#undef IDENT' before processing the
          input";

    "-I", Arg.String (fun s -> incdirs := s :: !incdirs),
    "DIR
          Add directory DIR to the search path for included files";

    "-V", Arg.String (fun s -> header := parse_version_spec s @ !header),
    "VAR:MAJOR.MINOR.PATCH-OPTPRERELEASE+OPTBUILD
          Define the following variables extracted from a version string
          (following the Semantic Versioning syntax http://semver.org/):

            VAR_MAJOR           must be a non-negative int
            VAR_MINOR           must be a non-negative int
            VAR_PATCH           must be a non-negative int
            VAR_PRERELEASE      if the OPTPRERELEASE part exists
            VAR_BUILD           if the OPTBUILD part exists
            VAR_VERSION         is the tuple (MAJOR, MINOR, PATCH)
            VAR_VERSION_STRING  is the string MAJOR.MINOR.PATCH
            VAR_VERSION_FULL    is the original string

          Example: cppo -V OCAML:4.02.1
";

    "-o", Arg.String (fun s -> out_file := Some s),
    "FILE
          Output file";

    "-q", Arg.Set preserve_quotations,
    "
          Identify and preserve camlp4 quotations";

    "-s", Arg.Set show_exact_locations,
    "
          Output line directives pointing to the exact source location of
          each token, including those coming from the body of macro
          definitions.  This behavior is off by default.";

    "-n", Arg.Set show_no_locations,
    "
          Do not output any line directive other than those found in the
          input (overrides -s).";

    "-version", Arg.Unit (fun () ->
                            print_endline Cppo_version.cppo_version;
                            exit 0),
    "
          Print the version of the program and exit.";

    "-x", Arg.String (fun s -> add_extension extensions s),
    "NAME:CMD_TEMPLATE
          Define a custom preprocessor target section starting with:
            #ext \"NAME\"
          and ending with:
            #endext

          NAME must be a lowercase identifier of the form [a-z][A-Za-z0-9_]*

          CMD_TEMPLATE is a command template supporting the following
          special sequences:
            %F  file name (unescaped; beware of potential scripting attacks)
            %B  number of the first line
            %E  number of the last line
            %%  a single percent sign

          Filename, first line number and last line number are also
          available from the following environment variables:
          CPPO_FILE, CPPO_FIRST_LINE, CPPO_LAST_LINE.

          The command produced is expected to read the data lines from stdin
          and to write its output to stdout."
  ]
  in
  let msg = sprintf "\
Usage: %s [OPTIONS] [FILE1 [FILE2 ...]]
Options:" Sys.argv.(0) in
  let add_file s = files := s :: !files in
  Arg.parse options add_file msg;

  let inputs =
    let preliminaries =
      match List.rev !header with
          [] -> []
        | l ->
            let s = String.concat "" l in
            [ Sys.getcwd (),
              "<command line>",
              (fun () -> Lexing.from_string s),
              (fun () -> ()) ]
    in
    let main =
      match List.rev !files with
          [] -> [ Sys.getcwd (),
                  "<stdin>",
                  (fun () -> Lexing.from_channel stdin),
                  (fun () -> ()) ]
        | l ->
            List.map (
              fun file ->
                let ic = lazy (open_in file) in
                Filename.dirname file,
                file,
                (fun () -> Lexing.from_channel (Lazy.force ic)),
                (fun () -> close_in (Lazy.force ic))
            ) l
    in
    preliminaries @ main
  in

  let env = Cppo_eval.builtin_env in
  let buf = Buffer.create 10_000 in
  let _env =
    Cppo_eval.include_inputs
      ~extensions
      ~preserve_quotations: !preserve_quotations
      ~incdirs: (List.rev !incdirs)
      ~show_exact_locations: !show_exact_locations
      ~show_no_locations: !show_no_locations
      buf env inputs
  in
  match !out_file with
      None ->
        print_string (Buffer.contents buf);
        flush stdout
    | Some file ->
        let oc = open_out file in
        output_string oc (Buffer.contents buf);
        close_out oc

let () =
  if not !Sys.interactive then
    try
      main ()
    with
    | Cppo_types.Cppo_error msg
    | Failure msg ->
        eprintf "Error: %s\n%!" msg;
        exit 1
