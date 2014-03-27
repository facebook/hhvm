(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
open Common

(*****************************************************************************)
(* Purpose *)
(*****************************************************************************)

(*****************************************************************************)
(* Flags *)
(*****************************************************************************)

let thrift = ref false

let split_vars = ref false

let upgrade = ref false

(* action mode *)
let action = ref ""

let apply_patch = ref false

let force = ref false

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

(*****************************************************************************)
(* Main action *)
(*****************************************************************************)

let main_action files_or_dirs =
  let files_or_dirs = files_or_dirs +> List.map Common.realpath in
  let www = Www.find_www_root_from_absolute_path (List.hd files_or_dirs) in
  let fullxs = Lib_parsing_php.find_source_files_of_dir_or_files files_or_dirs in
  List.iter (fun file ->
    let modified = 
      if !split_vars
      then begin
        (* split_vars doesn't need to run inside with_trying_modif since it
         * should always produce valid PHP/Hack, assuming the input file was
         * itself valid PHP/Hack. No semantics are changed -- it's purely to
         * allow further processing to insert typehints. *)
        Common.opt (Common.write_file ~file) (Hackificator.split_vars file);
        true
      end else Hackificator.with_trying_modif
        ~undo_when_error:(not !force)
        ~www file (
        if !thrift then [Hackificator.hackify_thrift]
        else [
          Hackificator.hackify ~upgrade:!upgrade "<?hh // strict";
          Hackificator.hackify ~upgrade:!upgrade "<?hh";
          Hackificator.hackify ~upgrade:!upgrade "<?hh // decl"
        ]
      )
    in
    if !thrift && modified (* || contain generated magic comment *)
    then Resign.sign file
  ) fullxs

(*****************************************************************************)
(* Extra actions *)
(*****************************************************************************)

(* Regression testing *)
let test dir =
  let suite = Unit_hackificator.unittest dir in
  OUnit.run_test_tt suite +> ignore;
  ()


let extra_actions () = [
  "-test", " <testdir>",
  Common.mk_action_1_arg (fun dir ->
    let dir = Common.realpath dir in
    test dir
  );

  "-hackify", " <file>",
  Common.mk_action_1_arg (fun file ->
    let file = Common.realpath file in
    let res = Hackificator.hackify_thrift file in
    match res with
    | None -> failwith "no transfo"
    | Some s ->
      let tmp = Common.new_temp_file "hackify" "php" in
      Common.write_file ~file:tmp s;
      let diff = Common2.unix_diff file tmp in
      diff +> List.iter pr2
  );
  "-hackify_class_of_typed_interface",  " <file interface> <file class>",
  Common.mk_action_2_arg (fun ifile cfile ->
    let s = Hackificator.hackify_class_of_typed_interface ifile cfile in
      let tmp = Common.new_temp_file "hackify" "php" in
      Common.write_file ~file:tmp s;
      let diff = Common2.unix_diff cfile tmp in
      diff +> List.iter pr2;
      if !apply_patch
      then Common.write_file ~file:cfile s;
  );
    
]

(*****************************************************************************)
(* The options *)
(*****************************************************************************)

let all_actions () = 
  extra_actions () @
 []

let options () = [
  "-thrift", Arg.Set thrift,
  " experimental transformation for thrift files";
  "-split_vars", Arg.Set split_vars,
  " split class variable declarations (even in existing hh files)";
  "-upgrade", Arg.Set upgrade,
  " take existing Hack files from decl => partial => strict if possible";
  "-apply_patch", Arg.Set apply_patch,
  " do the modification";
  "-force", Arg.Set force,
  " transform file even if error";
  "-debug", Arg.Unit (fun () ->
    (* Unparse_php.debug := true; *)
    ()
  ), " ";
  ] @
  Common.options_of_actions action (all_actions()) @
  Common2.cmdline_flags_devel ()

(*****************************************************************************)
(* Main entry point *)
(*****************************************************************************)

let main () = 
  Gc.set {(Gc.get ()) with Gc.stack_limit = 200 * 1024 * 1024};

  let usage_msg = 
    "Usage: " ^ Common2.basename Sys.argv.(0) ^ 
      " [options] <dir or file> " ^ "\n" ^ "Options are:"
  in
  (* does side effect on many global flags *)
  let args = Common.parse_options (options()) usage_msg Sys.argv in

  (* must be done after Arg.parse, because Common.profile is set by it *)
  Common.profile_code "Main total" (fun () -> 

    (match args with
   
    (* --------------------------------------------------------- *)
    (* actions, useful to debug subpart *)
    (* --------------------------------------------------------- *)
    | xs when List.mem !action (Common.action_list (all_actions())) -> 
        Common.do_action !action xs (all_actions())

    | _ when not (Common.null_string !action) -> 
        failwith ("unrecognized action or wrong params: " ^ !action)

    (* --------------------------------------------------------- *)
    (* main entry *)
    (* --------------------------------------------------------- *)
    | x::xs -> 
        main_action (x::xs)

    (* --------------------------------------------------------- *)
    (* empty entry *)
    (* --------------------------------------------------------- *)
    | _ -> 
        Common.usage usage_msg (options()); 
        failwith "too few arguments"
    )
  )

(*****************************************************************************)
let _ =
  Common.main_boilerplate (fun () -> 
      main ();
  )
