(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core
open Ide_api_types
open String_utils
open Sys_utils

module TNBody       = Typing_naming_body

module StringAnnotation = struct
  type t = string
  let pp fmt str = Format.pp_print_string fmt str
end

module TASTStringMapper = Aast_mapper.MapAnnotatedAST (Tast.AnnotationType)
  (StringAnnotation)

module StringNAST = Nast.AnnotatedAST(StringAnnotation)

module TASTTypeStripper = Aast_mapper.MapAnnotatedAST (Tast.AnnotationType)
  (Nast.PosAnnotation)


(*****************************************************************************)
(* Types, constants *)
(*****************************************************************************)

type mode =
  | Ai of Ai_options.t
  | Autocomplete
  | Ffp_autocomplete
  | Color
  | Coverage
  | Dump_symbol_info
  | Dump_inheritance
  | Errors
  | AllErrors
  | Lint
  | Suggest
  | Dump_deps
  | Identify_symbol of int * int
  | Find_local of int * int
  | Outline
  | Dump_nast
  | Dump_stripped_tast
  | Dump_tast
  | Find_refs of int * int
  | Highlight_refs of int * int
  | Decl_compare
  | Infer_return_types
  | Least_upper_bound

type options = {
  filename : string;
  mode : mode;
  no_builtins : bool;
  filter_positions: bool;
  tcopt : GlobalOptions.t;
}

(* Canonical builtins from our hhi library *)
let hhi_builtins = Hhi.get_raw_hhi_contents ()

(* All of the stuff that hh_single_type_check relies on is sadly not contained
 * in the hhi library, so we include a very small number of magic builtins *)
let magic_builtins = [|
  (
    "hh_single_type_check_magic.hhi",
    "<?hh // decl\n" ^
    "function gena();\n" ^
    "function genva();\n" ^
    "function gen_array_rec();\n" ^
    "function hh_show($val) {}\n" ^
    "function hh_show_env() {}\n" ^
    "function hh_log_level($level) {}\n"
  )
|]

(* Take the builtins (file, contents) array and create relative paths *)
let builtins = Array.fold_left begin fun acc (f, src) ->
  Relative_path.Map.add acc
    ~key:(Relative_path.create Relative_path.Dummy f)
    ~data:src
end Relative_path.Map.empty (Array.append magic_builtins hhi_builtins)

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let die str =
  let oc = stderr in
  output_string oc str;
  close_out oc;
  exit 2

let error ?(indent=false) l =
  output_string stderr (Errors.to_string ~indent (Errors.to_absolute l))

let parse_options () =
  let fn_ref = ref None in
  let usage = Printf.sprintf "Usage: %s filename\n" Sys.argv.(0) in
  let mode = ref Errors in
  let no_builtins = ref false in
  let filter_positions = ref false in
  let line = ref 0 in
  let set_mode x () =
    if !mode <> Errors
    then raise (Arg.Bad "only a single mode should be specified")
    else mode := x in
  let set_ai x = set_mode (Ai (Ai_options.prepare ~server:false x)) () in
  let safe_array = ref false in
  let safe_vector_array = ref false in
  let options = [
    "--ai",
      Arg.String (set_ai),
    " Run the abstract interpreter";
    "--all-errors",
      Arg.Unit (set_mode AllErrors),
      " List all errors not just the first one";
    "--auto-complete",
      Arg.Unit (set_mode Autocomplete),
      " Produce autocomplete suggestions";
    "--ffp-auto-complete",
      Arg.Unit (set_mode Ffp_autocomplete),
      " Produce autocomplete suggestions using the full-fidelity parse tree";
    "--colour",
      Arg.Unit (set_mode Color),
      " Produce colour output";
    "--color",
      Arg.Unit (set_mode Color),
      " Produce color output";
    "--coverage",
      Arg.Unit (set_mode Coverage),
      " Produce coverage output";
    "--dump-symbol-info",
      Arg.Unit (set_mode Dump_symbol_info),
      " Dump all symbol information";
    "--filter-positions",
      Arg.Set filter_positions,
      " Filter positions from print-outs of data-structures.";
    "--lint",
      Arg.Unit (set_mode Lint),
      " Produce lint errors";
    "--suggest",
      Arg.Unit (set_mode Suggest),
      " Suggest missing typehints";
    "--no-builtins",
      Arg.Set no_builtins,
      " Don't use builtins (e.g. ConstSet)";
    "--dump-deps",
      Arg.Unit (set_mode Dump_deps),
      " Print dependencies";
    "--dump-inheritance",
      Arg.Unit (set_mode Dump_inheritance),
      " Print inheritance";
    "--identify-symbol",
      Arg.Tuple ([
        Arg.Int (fun x -> line := x);
        Arg.Int (fun column -> set_mode (Identify_symbol (!line, column)) ());
      ]),
      "<pos> Show info about symbol at given line and column";
    "--find-local",
      Arg.Tuple ([
        Arg.Int (fun x -> line := x);
        Arg.Int (fun column -> set_mode (Find_local (!line, column)) ());
      ]),
      "<pos> Find all usages of local at given line and column";
    "--outline",
      Arg.Unit (set_mode Outline),
      " Print file outline";
    "--nast",
      Arg.Unit (set_mode Dump_nast),
      " Print out the named AST";
    "--tast",
      Arg.Unit (set_mode Dump_tast),
      " Print out the typed AST";
    "--stripped-tast",
      Arg.Unit (set_mode Dump_stripped_tast),
      " Print out the typed AST, stripped of type information." ^
      " This can be compared against the named AST to look for holes.";
    "--find-refs",
      Arg.Tuple ([
        Arg.Int (fun x -> line := x);
        Arg.Int (fun column -> set_mode (Find_refs (!line, column)) ());
      ]),
      "<pos> Find all usages of a symbol at given line and column";
    "--highlight-refs",
      Arg.Tuple ([
        Arg.Int (fun x -> line := x);
        Arg.Int (fun column -> set_mode (Highlight_refs (!line, column)) ());
      ]),
      "<pos> Highlight all usages of a symbol at given line and column";
    "--decl-compare",
      Arg.Unit (set_mode Decl_compare),
      " Test comparison functions used in incremental mode on declarations" ^
      " in provided file";
    "--safe_array",
      Arg.Set safe_array,
      " Enforce array subtyping relationships so that array<T> and array<Tk, \
      Tv> are each subtypes of array but not vice-versa.";
    "--safe_vector_array",
      Arg.Set safe_vector_array,
      " Enforce array subtyping relationships so that array<T> is not a \
      of array<int, T>.";
    "--infer-return-types",
      Arg.Unit (set_mode Infer_return_types),
      " Infers return types of functions and methods.";
    "--least-upper-bound",
        Arg.Unit (set_mode Least_upper_bound),
        " Gets the least upper bound of a list of types.";
  ] in
  let options = Arg.align ~limit:25 options in
  Arg.parse options (fun fn -> fn_ref := Some fn) usage;
  let fn = match !fn_ref with
    | Some fn -> fn
    | None -> die usage in
  let tcopt = {
    GlobalOptions.default with
      GlobalOptions.tco_safe_array = !safe_array;
      GlobalOptions.tco_safe_vector_array = !safe_vector_array;
  } in
  { filename = fn;
    mode = !mode;
    no_builtins = !no_builtins;
    filter_positions = !filter_positions;
    tcopt;
  }

let compute_least_type tcopt popt fn =
  let tenv = Typing_infer_return.typing_env_from_file tcopt fn in
  Option.iter (Parser_heap.find_fun_in_file popt fn "\\test")
    ~f:begin fun f ->
      let f = Naming.fun_ tcopt f in
      let {Nast.fnb_nast; _} = Typing_naming_body.func_body tcopt f in
      let types =
        Nast.(List.fold fnb_nast ~init:[]
          ~f:begin fun acc stmt ->
            match stmt with
            | Expr (_, New (CI ((_, "\\least_upper_bound"), hints), _, _)) ->
              (List.map hints
                (fun h -> snd (Typing_infer_return.type_from_hint tcopt fn h)))
              :: acc
            | _ -> acc
          end)
      in
      let types = List.rev types in
      List.iter types
        ~f:(begin fun tys ->
          let tyop = Typing_ops.LeastUpperBound.full tenv tys in
          let least_ty =
            Option.value_map tyop ~default:""
              ~f:(Typing_infer_return.print_type_locl tenv)
          in
          let str_tys =
            Typing_infer_return.(print_list ~f:(print_type_locl tenv) tys)
          in
          Printf.printf "Least upper bound of %s is %s \n" str_tys least_ty
        end)
      end

let infer_return tcopt fn info  =
  let names = FileInfo.simplify info in
  let fast = Relative_path.Map.singleton fn names in
  let files = Typing_suggest_service.keys fast in
  Typing_infer_return.(get_inferred_types tcopt files ~process:format_types)

let suggest_and_print tcopt fn info =
  let names = FileInfo.simplify info in
  let fast = Relative_path.Map.singleton fn names in
  let patch_map = Typing_suggest_service.go None fast tcopt in
  match Relative_path.Map.get patch_map fn with
    | None -> ()
    | Some l -> begin
      (* Sort so that the unit tests come out in a consistent order, normally
       * doesn't matter. *)
      let l = List.sort ~cmp: (fun (x, _, _) (y, _, _) -> x - y) l
      in
      List.iter ~f: (ServerConvert.print_patch fn tcopt) l

    end

(* This allows one to fake having multiple files in one file. This
 * is used only in unit test files.
 * Indeed, there are some features that require mutliple files to be tested.
 * For example, newtype has a different meaning depending on the file.
 *)
let rec make_files = function
  | [] -> []
  | Str.Delim header :: Str.Text content :: rl ->
      let pattern = Str.regexp "////" in
      let header = Str.global_replace pattern "" header in
      let pattern = Str.regexp "[ ]*" in
      let filename = Str.global_replace pattern "" header in
      (filename, content) :: make_files rl
  | _ -> assert false

(* We have some hacky "syntax extensions" to have one file contain multiple
 * files, which can be located at arbitrary paths. This is useful e.g. for
 * testing lint rules, some of which activate only on certain paths. It's also
 * useful for testing abstract types, since the abstraction is enforced at the
 * file boundary.
 * Takes the path to a single file, returns a map of filenames to file contents.
 *)
let file_to_files file =
  let abs_fn = Relative_path.to_absolute file in
  let content = cat abs_fn in
  let delim = Str.regexp "////.*" in
  if Str.string_match delim content 0
  then
    let contentl = Str.full_split delim content in
    let files = make_files contentl in
    List.fold_left ~f: begin fun acc (sub_fn, content) ->
      let file =
        Relative_path.create Relative_path.Dummy (abs_fn^"--"^sub_fn) in
      Relative_path.Map.add acc ~key:file ~data:content
    end ~init: Relative_path.Map.empty files
  else if string_starts_with content "// @directory " then
    let contentl = Str.split (Str.regexp "\n") content in
    let first_line = List.hd_exn contentl in
    let regexp = Str.regexp ("^// @directory *\\([^ ]*\\) \
      *\\(@file *\\([^ ]*\\)*\\)?") in
    let has_match = Str.string_match regexp first_line 0 in
    assert has_match;
    let dir = Str.matched_group 1 first_line in
    let file_name =
      try
        Str.matched_group 3 first_line
      with
        Not_found -> abs_fn in
    let file = Relative_path.create Relative_path.Dummy (dir ^ file_name) in
    let content = String.concat "\n" (List.tl_exn contentl) in
    Relative_path.Map.singleton file content
  else
    Relative_path.Map.singleton file content

(* Make readable test output *)
let replace_color input =
  match input with
  | (Some Unchecked, str) -> "<unchecked>"^str^"</unchecked>"
  | (Some Checked, str) -> "<checked>"^str^"</checked>"
  | (Some Partial, str) -> "<partial>"^str^"</partial>"
  | (None, str) -> str

let print_colored fn type_acc =
  let content = cat (Relative_path.to_absolute fn) in
  let results = ColorFile.go content type_acc in
  if Unix.isatty Unix.stdout
  then Tty.cprint (ClientColorFile.replace_colors results)
  else print_string (List.map ~f: replace_color results |> String.concat "")

let print_coverage fn type_acc =
  let counts = ServerCoverageMetric.count_exprs fn type_acc in
  ClientCoverageMetric.go ~json:false (Some (Coverage_level.Leaf counts))

let check_errors opts errors files_info =
  Relative_path.Map.fold files_info ~f:begin fun fn fileinfo errors ->
    errors @ Errors.get_error_list
        (Typing_check_utils.check_defs opts fn fileinfo)
  end ~init:errors

let create_nasts opts files_info =
  let open Result in
  let open Nast in
  let build_nast fn {FileInfo.funs; classes; typedefs; consts; _} =
    List.map ~f:Result.ok_or_failwith (
      List.map funs begin fun (_, x) ->
        Parser_heap.find_fun_in_file ~full:true opts fn x
        |> Result.of_option ~error:(Printf.sprintf "Couldn't find function %s" x)
        >>| Naming.fun_ opts
        >>| (fun f -> {f with f_body = (NamedBody (Typing_naming_body.func_body opts f))})
        >>| (fun f -> Nast.Fun f)
      end
      @
      List.map classes begin fun (_, x) ->
        Parser_heap.find_class_in_file ~full:true opts fn x
        |> Result.of_option ~error:(Printf.sprintf "Couldn't find class %s" x)
        >>| Naming.class_ opts
        >>| Typing_naming_body.class_meth_bodies opts
        >>| (fun c -> Nast.Class c)
      end
      @
      List.map typedefs begin fun (_, x) ->
        Parser_heap.find_typedef_in_file ~full:true opts fn x
        |> Result.of_option ~error:(Printf.sprintf "Couldn't find typedef %s" x)
        >>| Naming.typedef opts
        >>| (fun t -> Nast.Typedef t)
      end
      @
      List.map consts begin fun (_, x) ->
        Parser_heap.find_const_in_file ~full:true opts fn x
        |> Result.of_option ~error:(Printf.sprintf "Couldn't find const %s" x)
        >>| Naming.global_const opts
        >>| fun g -> Nast.Constant g
      end
    )
  in Relative_path.Map.mapi (build_nast) files_info

let nast_to_tast_tenv opts nast =
  let open Result in
  let open Nast in
  let def_conv = function
    | Fun f -> Ok f
      >>| Typing.fun_def opts
      >>| (fun (f, tenv) -> Tast.Fun f, tenv)
    | Class c -> Ok c
      >>| Typing.class_def opts
      >>= of_option ~error:(Printf.sprintf "Error with class %s definition" (snd c.c_name))
      >>| (fun (c, tenv) -> Tast.Class c, tenv)
    | Constant gc -> Ok gc
      >>| (fun x -> Typing.gconst_def x opts)
      >>| (fun (gc, tenv) -> Tast.Constant gc, tenv)
    | Typedef td -> Ok td
      >>| Typing.typedef_def opts
      >>| (fun (td, tenv) -> Tast.Typedef td, tenv)
  in
  List.map nast (Fn.compose Result.ok_or_failwith def_conv)

let with_named_body opts n_fun =
  (** In the naming heap, the function bodies aren't actually named yet, so
   * we need to invoke naming here.
   * See also docs in Naming.Make. *)
  let n_f_body = TNBody.func_body opts n_fun in
  { n_fun with Nast.f_body = Nast.NamedBody n_f_body }

let n_fun_fold opts fn acc (_, fun_name) =
  match Parser_heap.find_fun_in_file ~full:true opts fn fun_name with
  | None -> acc
  | Some f ->
    let n_fun = Naming.fun_ opts f in
    (with_named_body opts n_fun) :: acc

let n_class_fold _tcopt _fn acc _class_name = acc
let n_type_fold _tcopt _fn acc _type_name = acc
let n_const_fold _tcopt _fn acc _const_name = acc

(** Load the Nast for the file from the Nast heaps. *)
let nast_for_file opts fn
{ FileInfo.funs; classes; typedefs; consts; _} =
  List.fold_left funs ~init:[] ~f:(n_fun_fold opts fn),
  List.fold_left classes ~init:[] ~f:(n_class_fold opts fn),
  List.fold_left typedefs ~init:[] ~f:(n_type_fold opts fn),
  List.fold_left consts ~init:[] ~f:(n_const_fold opts fn)

let parse_name_and_decl popt files_contents tcopt =
  Errors.do_ begin fun () ->
    let parsed_files =
      Relative_path.Map.mapi
       (Parser_hack.program popt) files_contents in

    let files_info =
      Relative_path.Map.mapi begin fun fn parsed_file ->
        let {Parser_hack.file_mode; comments; ast; _} = parsed_file in
        Parser_heap.ParserHeap.add fn (ast, Parser_heap.Full);
        let funs, classes, typedefs, consts = Ast_utils.get_defs ast in
        { FileInfo.
          file_mode; funs; classes; typedefs; consts; comments = Some comments;
        }
      end parsed_files in

    Relative_path.Map.iter files_info begin fun fn fileinfo ->
      let {FileInfo.funs; classes; typedefs; consts; _} = fileinfo in
      NamingGlobal.make_env popt ~funs ~classes ~typedefs ~consts
    end;

    Relative_path.Map.iter files_info begin fun fn _ ->
      Decl.make_env tcopt fn
    end;

    files_info
  end

let add_newline contents =
  let x = String.index contents '\n' in
  String.((sub contents 0 x) ^ "\n" ^ (sub contents x ((length contents) - x)))

let get_decls defs =
  SSet.fold (fun x acc -> (Decl_heap.Typedefs.find_unsafe x)::acc)
  defs.FileInfo.n_types
  [],
  SSet.fold (fun x acc -> (Decl_heap.Funs.find_unsafe x)::acc)
  defs.FileInfo.n_funs
  [],
  SSet.fold (fun x acc -> (Decl_heap.Classes.find_unsafe x)::acc)
  defs.FileInfo.n_classes
  []

let fail_comparison s =
  raise (Failure (
    (Printf.sprintf "Comparing %s failed!\n" s) ^
    "It's likely that you added new positions to decl types " ^
    "without updating Decl_pos_utils.NormalizeSig\n"
  ))

let compare_typedefs t1 t2 =
  let t1 = Decl_pos_utils.NormalizeSig.typedef t1 in
  let t2 = Decl_pos_utils.NormalizeSig.typedef t2 in
  if t1 <> t2 then fail_comparison "typedefs"

let compare_funs f1 f2 =
  let f1 = Decl_pos_utils.NormalizeSig.fun_type f1 in
  let f2 = Decl_pos_utils.NormalizeSig.fun_type f2 in
  if f1 <> f2 then fail_comparison "funs"

let compare_classes c1 c2 =
  if Decl_compare.class_big_diff c1 c2 then fail_comparison "class_big_diff";

  let c1 = Decl_pos_utils.NormalizeSig.class_type c1 in
  let c2 = Decl_pos_utils.NormalizeSig.class_type c2 in
  let _, is_unchanged =
    Decl_compare.ClassDiff.compare c1.Decl_defs.dc_name c1 c2 in
  if not is_unchanged then fail_comparison "ClassDiff";

  let _, is_unchanged = Decl_compare.ClassEltDiff.compare c1 c2 in
  if is_unchanged = `Changed then fail_comparison "ClassEltDiff"

let test_decl_compare filename popt files_contents tcopt files_info =
  (* skip some edge cases that we don't handle now... ugly! *)
  if (Relative_path.suffix filename) = "capitalization3.php" then () else
  if (Relative_path.suffix filename) = "capitalization4.php" then () else
  (* do not analyze builtins over and over *)
  let files_info = Relative_path.Map.fold builtins
    ~f:begin fun k _ acc -> Relative_path.Map.remove acc k end
    ~init:files_info
  in

  let files = Relative_path.Map.fold files_info
    ~f:(fun k _ acc -> Relative_path.Set.add acc k)
    ~init:Relative_path.Set.empty
  in

  let defs = Relative_path.Map.fold files_info ~f:begin fun _ names1 names2 ->
      FileInfo.(merge_names (simplify names1) names2)
    end ~init:FileInfo.empty_names
  in

  let typedefs1, funs1, classes1 = get_decls defs in
  (* For the purpose of this test, we can ignore other heaps *)
  Parser_heap.ParserHeap.remove_batch files;

  (* We need to oldify, not remove, for ClassEltDiff to work *)
  Decl_redecl_service.oldify_type_decl
    None files_info ~bucket_size:1 FileInfo.empty_names defs
      ~collect_garbage:false;

  let files_contents = Relative_path.Map.map files_contents ~f:add_newline in
  let _, _, _ = parse_name_and_decl popt files_contents tcopt in

  let typedefs2, funs2, classes2 = get_decls defs in

  List.iter2_exn typedefs1 typedefs2 compare_typedefs;
  List.iter2_exn funs1 funs2 compare_funs;
  List.iter2_exn classes1 classes2 compare_classes;
  ()

(* Strip output of position information *)
let filter_positions s = (Str.global_replace
  (Str.regexp "\\[L[0-9]+:[0-9]+-L[0-9]+:[0-9]+\\]") "<p>" s)

(* Returns a list of Tast defs, along with associated type environments. *)
let get_tast_tenv opts filename files_info =
  let nasts = create_nasts opts files_info in
  let nast = Relative_path.Map.find filename nasts in
  nast_to_tast_tenv opts nast

let handle_mode mode filename opts popt files_contents files_info errors =
  let filter_output =
    (if opts.filter_positions then filter_positions else (fun x -> x)) in
  match mode with
  | Ai _ -> ()
  | Autocomplete ->
      let file = cat (Relative_path.to_absolute filename) in
      let result =
        ServerAutoComplete.auto_complete ~tcopt:opts.tcopt ~delimit_on_namespaces:false file in
      List.iter ~f: begin fun r ->
        let open AutocompleteService in
        Printf.printf "%s %s\n" r.res_name r.res_ty
      end result.Utils.With_complete_flag.value
  | Ffp_autocomplete ->
      let file_text = cat (Relative_path.to_absolute filename) in
      (* TODO: Use a magic word/symbol to identify autocomplete location instead *)
      let args_regex = Str.regexp "AUTOCOMPLETE [1-9][0-9]* [1-9][0-9]*" in
      let open Ide_api_types in
      let position = try
        let _ = Str.search_forward args_regex file_text 0 in
        let raw_flags = Str.matched_string file_text in
        match split ' ' raw_flags with
        | [ _; row; column] ->
          { line = int_of_string row; column = int_of_string column }
        | _ -> failwith "Invalid test file: no flags found"
      with
        Not_found -> failwith "Invalid test file: no flags found"
      in
      let result =
        FfpAutocompleteService.auto_complete opts.tcopt file_text position
        ~filter_by_token:true
      in begin
        match result with
        | [] -> Printf.printf "No result found\n"
        | res -> List.iter res ~f:begin fun r ->
            let open AutocompleteTypes in
            Printf.printf "%s\n" r.res_name
          end
      end
  | Color ->
      Relative_path.Map.iter files_info begin fun fn fileinfo ->
        if Relative_path.Map.mem builtins fn then () else begin
          let result = ServerColorFile.get_level_list begin fun () ->
            ignore @@ Typing_check_utils.check_defs opts.tcopt fn fileinfo;
            fn
          end in
          print_colored fn result;
        end
      end
  | Coverage ->
      Relative_path.Map.iter files_info begin fun fn fileinfo ->
        if Relative_path.Map.mem builtins fn then () else begin
          let type_acc =
            ServerCoverageMetric.accumulate_types fn fileinfo opts.tcopt in
          print_coverage fn type_acc;
        end
      end
  | Dump_symbol_info ->
      begin match Relative_path.Map.get files_info filename with
        | Some fileinfo ->
            let raw_result =
              SymbolInfoService.helper opts.tcopt [] [(filename, fileinfo)] in
            let result = SymbolInfoService.format_result raw_result in
            let result_json = ClientSymbolInfo.to_json result in
            print_endline (Hh_json.json_to_multiline result_json)
        | None -> ()
      end
  | Lint ->
      let lint_errors = Relative_path.Map.fold files_contents ~init:[]
        ~f:begin fun fn content lint_errors ->
          lint_errors @ fst (Lint.do_ begin fun () ->
            Linting_service.lint opts.tcopt fn content
          end)
        end in
      if lint_errors <> []
      then begin
        let lint_errors = List.sort ~cmp: begin fun x y ->
          Pos.compare (Lint.get_pos x) (Lint.get_pos y)
        end lint_errors in
        let lint_errors = List.map ~f: Lint.to_absolute lint_errors in
        ServerLint.output_text stdout lint_errors;
        exit 2
      end
      else Printf.printf "No lint errors\n"
  | Dump_deps ->
    Relative_path.Map.iter files_info begin fun fn fileinfo ->
      ignore @@ Typing_check_utils.check_defs opts.tcopt fn fileinfo
    end;
    Typing_deps.dump_deps stdout
  | Dump_inheritance ->
    Typing_deps.update_files files_info;
    Relative_path.Map.iter files_info begin fun fn fileinfo ->
      if Relative_path.Map.mem builtins fn then () else begin
        List.iter fileinfo.FileInfo.classes begin fun (_p, class_) ->
          Printf.printf "Ancestors of %s and their overridden methods:\n"
            class_;
          let ancestors = MethodJumps.get_inheritance opts.tcopt class_
            ~filter:MethodJumps.No_filter ~find_children:false files_info
            None in
          ClientMethodJumps.print_readable ancestors ~find_children:false;
          Printf.printf "\n";
        end;
        Printf.printf "\n";
        List.iter fileinfo.FileInfo.classes begin fun (_p, class_) ->
          Printf.printf "Children of %s and the methods they override:\n"
            class_;
          let children = MethodJumps.get_inheritance opts.tcopt class_
            ~filter:MethodJumps.No_filter ~find_children:true files_info None in
          ClientMethodJumps.print_readable children ~find_children:true;
          Printf.printf "\n";
        end;
      end
    end;
  | Identify_symbol (line, column) ->
    let file = cat (Relative_path.to_absolute filename) in
    begin match ServerIdentifyFunction.go_absolute file line column opts.tcopt with
      | [] -> print_endline "None"
      | result -> ClientGetDefinition.print_readable ~short_pos:true result
    end
  | Find_local (line, column) ->
    let file = cat (Relative_path.to_absolute filename) in
    let result = ServerFindLocals.go popt file line column in
    let print pos = Printf.printf "%s\n" (Pos.string_no_file pos) in
    List.iter result print
  | Outline ->
    let file = cat (Relative_path.to_absolute filename) in
    let results = FileOutline.outline popt file in
    FileOutline.print ~short_pos:true results
  | Dump_nast ->
    let nasts = create_nasts opts.tcopt files_info in
    let nast = Relative_path.Map.find filename nasts in
    Printf.printf "%s\n" (filter_output (Nast.show_program nast))
  | Dump_tast ->
    let tast_tenv = get_tast_tenv opts.tcopt filename files_info in
    let type_to_string tenv (_, ty) = match ty with
      | None -> "None"
      | Some ty -> "(Some " ^ Typing_print.full tenv ty ^ ")" in
    let program = List.map tast_tenv
      (fun (def, tenv) -> TASTStringMapper.map_def (type_to_string tenv) def) in
    Printf.printf "%s\n" (filter_output (StringNAST.show_program program))
  | Dump_stripped_tast ->
    let tast_tenv = get_tast_tenv opts.tcopt filename files_info in
    let program = List.map tast_tenv fst in
    let program = TASTTypeStripper.map_program fst program in
    Printf.printf "%s\n" (filter_output (Nast.show_program program))
  | Find_refs (line, column) ->
    Typing_deps.update_files files_info;
    let genv = ServerEnvBuild.default_genv in
    let env = {(ServerEnvBuild.make_env genv.ServerEnv.config) with
      ServerEnv.files_info;
      ServerEnv.tcopt = opts.tcopt;
    } in
    let file = cat (Relative_path.to_absolute filename) in
    let include_defs = false in
    let results = ServerFindRefs.go_from_file
      (file, line, column, include_defs) genv env in
    ClientFindRefs.print_ide_readable results;
  | Highlight_refs (line, column) ->
    let file = cat (Relative_path.to_absolute filename) in
    let results = ServerHighlightRefs.go (file, line, column) opts.tcopt  in
    ClientHighlightRefs.go results ~output_json:false;
  | Suggest
  | Infer_return_types
  | Errors ->
      (* Don't typecheck builtins *)
      let files_info = Relative_path.Map.fold builtins
        ~f:begin fun k _ acc -> Relative_path.Map.remove acc k end
        ~init:files_info
      in
      let errors = check_errors opts.tcopt errors files_info in
      if mode = Suggest
      then Relative_path.Map.iter files_info (suggest_and_print opts.tcopt);
      if mode = Infer_return_types
      then
        Option.iter ~f:(infer_return opts.tcopt filename)
          (Relative_path.Map.get files_info filename);
      if errors <> []
      then (error (List.hd_exn errors); exit 2)
      else Printf.printf "No errors\n"
  | AllErrors ->
      let errors = check_errors opts.tcopt errors files_info in
      if errors <> []
      then (List.iter ~f:(error ~indent:true) errors; exit 2)
      else Printf.printf "No errors\n"
  | Decl_compare ->
    test_decl_compare filename popt files_contents opts.tcopt files_info
  | Least_upper_bound-> compute_least_type opts.tcopt popt filename

(*****************************************************************************)
(* Main entry point *)
(*****************************************************************************)

let decl_and_run_mode ({filename; mode; no_builtins; tcopt; _} as opts) popt =
  if mode = Dump_deps then Typing_deps.debug_trace := true;
  Local_id.track_names := true;
  Ident.track_names := true;
  let builtins = if no_builtins then Relative_path.Map.empty else builtins in
  let filename = Relative_path.create Relative_path.Dummy filename in
  let files_contents = file_to_files filename in
  (* Merge in builtins *)
  let files_contents_with_builtins = Relative_path.Map.fold builtins
    ~f:begin fun k src acc -> Relative_path.Map.add acc ~key:k ~data:src end
    ~init:files_contents
  in

  let errors, files_info, _ =
    parse_name_and_decl popt files_contents_with_builtins tcopt in

  handle_mode mode filename opts popt files_contents files_info
    (Errors.get_error_list errors)

let main_hack ({filename; mode; no_builtins; _} as opts) =
  (* TODO: We should have a per file config *)
  let popt = ParserOptions.default in
  Sys_utils.signal Sys.sigusr1
    (Sys.Signal_handle Typing.debug_print_last_pos);
  EventLogger.init EventLogger.Event_logger_fake 0.0;
  let _handle = SharedMem.init GlobalConfig.default_sharedmem_config in
  let tmp_hhi = Path.concat (Path.make Sys_utils.temp_dir_name) "hhi" in
  Hhi.set_hhi_root_for_unit_test tmp_hhi;
  match mode with
  | Ai ai_options ->
    Ai.do_ Typing_check_utils.check_defs filename ai_options
  | _ ->
    decl_and_run_mode opts popt

(* command line driver *)
let _ =
  if ! Sys.interactive
  then ()
  else
    (* On windows, setting 'binary mode' avoids to output CRLF on
       stdout.  The 'text mode' would not hurt the user in general, but
       it breaks the testsuite where the output is compared to the
       expected one (i.e. in given file without CRLF). *)
    set_binary_mode_out stdout true;
    let options = parse_options () in
    Unix.handle_unix_error main_hack options
