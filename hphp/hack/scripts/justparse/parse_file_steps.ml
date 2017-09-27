module SourceText = Full_fidelity_source_text
module Syn = Full_fidelity_syntax_tree

(* Take the arguments, which are absolute paths, parse the corresponding files.
 *
 * construct various kinds of intermediate formats and report how long it took to
 * produce them.
 *)

let get_ts = Unix.gettimeofday

let[@warning "-26"] process_file path =
  let start = get_ts () in
  let rpath = Relative_path.create Relative_path.Dummy path in
  let st = SourceText.from_file rpath in
  let after_file_read = get_ts () in
  let tree = Syn.make st in
  let after_make_tree = get_ts () in
  let full_ast = begin
    Full_fidelity_ast.from_text
      ~elaborate_namespaces:true
      ~include_line_comments:true
      ~keep_errors:false (* TODO: we get a segmentation fault if keep_errors is true *)
      ~ignore_pos:false
      ~quick:false
      ~suppress_output:false
      ~lower_coroutines:false
      ~parser_options:ParserOptions.default (* TODO: be explicit here *)
      (* file *)         rpath
      (* source_text *)  st
  end in
  let after_make_full_tree = get_ts () in
  Printf.printf "%f\n" (after_make_full_tree -. after_file_read)

let main () =
  for i = 1 to (-1 + (Array.length Sys.argv)); do
    process_file Sys.argv.(i)
  done

let () = main ()
