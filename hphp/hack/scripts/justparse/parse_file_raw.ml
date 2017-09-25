module SourceText = Full_fidelity_source_text
module S = Full_fidelity_syntax_tree
module E = Full_fidelity_syntax_error

(* Take the arguments, which are absolute paths, parse the corresponding files.
 * If there are errors, print them. If there are no errors, print "no errors!".
 *
 * This script is just here to support timing the parser from a cold start over
 * a collection of files.*)

let process_file path =
  let rpath = Relative_path.create Relative_path.Dummy path in
  let st = SourceText.from_file rpath in
  let tree = S.make st in
  Printf.printf "file: %s\n" path;
  let errors = S.all_errors tree in
  let () = match errors with
  | [] -> Printf.printf "no errors!"
  | _  -> List.iter (fun x -> Printf.printf "error:%s\n" (E.message x)) (S.all_errors tree) in
  Printf.printf "\n"

let main () =
  for i = 1 to (-1 + (Array.length Sys.argv)); do
    process_file Sys.argv.(i)
  done

let () = main ()
