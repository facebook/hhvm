(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open File_content
open String_utils
open Sys_utils

module TNBody       = Typing_naming_body

module StringAnnotation = struct
  type t = string
  let pp fmt str = Format.pp_print_string fmt str
end

module StringNASTAnnotations = struct
  module ExprAnnotation = StringAnnotation
  module EnvAnnotation = Nast.UnitAnnotation
end

module StringNAST = Nast.AnnotatedAST(StringNASTAnnotations)

module TASTStringMapper =
  Aast_mapper.MapAnnotatedAST(Tast_expand.ExpandedTypeAnnotations)(StringNASTAnnotations)

module PS = Full_fidelity_positioned_syntax
module PositionedTree = Full_fidelity_syntax_tree
  .WithSyntax(PS)

module TS = Full_fidelity_typed_positioned_syntax
module TypedTree = Full_fidelity_syntax_tree
  .WithSyntax(TS)


(*****************************************************************************)
(* Types, constants *)
(*****************************************************************************)

type mode =
  | Ai of Ai_options.t
  | Autocomplete
  | Autocomplete_manually_invoked
  | Ffp_autocomplete
  | Color
  | Coverage
  | Cst_search
  | Dump_symbol_info
  | Dump_inheritance
  | Errors
  | AllErrors
  | Lint
  | Suggest
  | Dump_deps
  | Dump_bazooka_deps
  | Dump_toplevel_deps
  | Identify_symbol of int * int
  | Find_local of int * int
  | Outline
  | Dump_nast
  | Dump_stripped_tast
  | Dump_tast
  | Dump_typed_full_fidelity_json
  | Find_refs of int * int
  | Highlight_refs of int * int
  | Decl_compare
  | Infer_return_types
  | Least_upper_bound
  | Linearization

type options = {
  filename : string;
  mode : mode;
  no_builtins : bool;
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
    "namespace {\n" ^
    "function gena();\n" ^
    "function genva();\n" ^
    "function gen_array_rec();\n" ^
    "function hh_show(<<__AcceptDisposable>> $val) {}\n" ^
    "function hh_show_env() {}\n" ^
    "function hh_log_level($level) {}\n" ^
    "}\n" ^
    "namespace HH\\Lib\\Tuple{\n" ^
    "function gen();\n" ^
    "function from_async();\n" ^
    "}\n"
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

let exit_on_parent_exit () = Parent.exit_on_parent_exit 10 60

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
  let line = ref 0 in
  let set_mode x () =
    if !mode <> Errors
    then raise (Arg.Bad "only a single mode should be specified")
    else mode := x in
  let set_ai x = set_mode (Ai (Ai_options.prepare ~server:false x)) () in
  let safe_array = ref false in
  let safe_vector_array = ref false in
  let forbid_nullable_cast = ref false in
  let deregister_attributes = ref false in
  let disable_optional_and_unknown_shape_fields = ref false in
  let disallow_ambiguous_lambda = ref false in
  let disallow_array_typehint = ref false in
  let disallow_array_literal = ref false in
  let disallow_reified_generics = ref false in
  let untyped_nonstrict_lambda_parameters = ref false in
  let no_fallback_in_namespaces = ref false in
  let dynamic_view = ref false in
  let allow_array_as_tuple = ref false in
  let allow_return_by_ref = ref false in
  let allow_array_cell_pass_by_ref = ref false in
  let void_is_type_of_null = ref false in
  let disallow_unset_on_varray = ref false in
  let auto_namespace_map = ref [] in
  let dont_assume_php = ref false in
  let unsafe_rx = ref false in
  let disable_primitive_refinement = ref false in
  let options = [
    "--ai",
      Arg.String (set_ai),
    " Run the abstract interpreter";
    "--all-errors",
      Arg.Unit (set_mode AllErrors),
      " List all errors not just the first one";
    "--deregister-attributes",
      Arg.Set deregister_attributes,
      " Ignore all functions with attribute '__PHPStdLib'";
    "--auto-complete",
      Arg.Unit (set_mode Autocomplete),
      " Produce autocomplete suggestions as if triggered by trigger character";
    "--auto-complete-manually-invoked",
      Arg.Unit (set_mode Autocomplete_manually_invoked),
      " Produce autocomplete suggestions as if manually triggered by user";
    "--auto-namespace-map",
      Arg.String (fun m ->
        auto_namespace_map := ServerConfig.convert_auto_namespace_to_map m),
      " Alias namespaces";
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
    "--cst-search",
        Arg.Unit (set_mode Cst_search),
        " Search the concrete syntax tree of the given file using the pattern"^
        " given on stdin."^
        " (The pattern is a JSON object adhering to the search DSL.)";
    "--disable_optional_and_unknown_shape_fields",
      Arg.Set disable_optional_and_unknown_shape_fields,
      "Disables optional and unknown shape fields syntax and typechecking.";
    "--dump-symbol-info",
      Arg.Unit (set_mode Dump_symbol_info),
      " Dump all symbol information";
    "--lint",
      Arg.Unit (set_mode Lint),
      " Produce lint errors";
    "--suggest",
      Arg.Unit (set_mode Suggest),
      " Suggest missing typehints";
    "--no-builtins",
      Arg.Set no_builtins,
      " Don't use builtins (e.g. ConstSet)";
    "--dont-assume-php",
      Arg.Set dont_assume_php,
      " Don't assume that undefined names are in PHP files";
    "--dump-deps",
      Arg.Unit (set_mode (Dump_deps)),
      " Print dependencies";
    "--dump-bazooka-deps",
      Arg.Unit (set_mode (Dump_bazooka_deps)),
      " Print dependencies, simplified to toplevel";
    "--dump-toplevel-deps",
      Arg.Unit (set_mode Dump_toplevel_deps),
      " Print toplevel dependencies";
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
    "--typed-full-fidelity-json",
      Arg.Unit (set_mode Dump_typed_full_fidelity_json),
      " (mode) show full fidelity parse tree with types in json format.";
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
    "--forbid_nullable_cast",
      Arg.Set forbid_nullable_cast,
      " Forbid casting from nullable values.";
    "--disallow-ambiguous-lambda",
      Arg.Set disallow_ambiguous_lambda,
      " Disallow definition of lambdas that require use-site checking.";
    "--untyped-nonstrict-lambda-parameters",
      Arg.Set untyped_nonstrict_lambda_parameters,
      " In non-strict files treat lambda parameters without a type hint as untyped.";
    "--disallow-array-typehint",
      Arg.Set disallow_array_typehint,
      " Disallow usage of array typehints.";
    "--disallow-array-literal",
      Arg.Set disallow_array_literal,
      " Disallow usage of array literals.";
    "--disallow-reified-generics",
      Arg.Set disallow_reified_generics,
      " Disallow usage of reified generics.";
    "--no-fallback-in-namespaces",
      Arg.Set no_fallback_in_namespaces,
      " Treat foo() as namespace\\foo() and MY_CONST as namespace\\MY_CONST.";
    "--infer-return-types",
      Arg.Unit (set_mode Infer_return_types),
      " Infers return types of functions and methods.";
    "--least-upper-bound",
        Arg.Unit (set_mode Least_upper_bound),
        " Gets the least upper bound of a list of types.";
    "--dynamic-view",
        Arg.Set (dynamic_view),
        " Turns on dynamic view, replacing Tany with dynamic";
    "--allow-array-as-tuple",
        Arg.Set allow_array_as_tuple,
        " Allow tuples to be passed as untyped arrays and vice versa";
    "--allow-return-by-ref",
        Arg.Set allow_return_by_ref,
        " Allow returning references from functions";
    "--allow-array-cell-pass-by-ref",
        Arg.Set allow_array_cell_pass_by_ref,
        " Allow binding of array cells by reference as arguments to function calls";
    "--void-is-type-of-null",
        Arg.Set void_is_type_of_null,
        " Make void the type of null";
    "--disallow-unset-on-varray",
        Arg.Set disallow_unset_on_varray,
        " Disallow unsetting indices from varrays";
    "--unsafe-rx",
        Arg.Set unsafe_rx,
        " Disables reactivity related errors";
    "--disable-primitive-refinement",
      Arg.Set disable_primitive_refinement,
      " Disable type refinement of is_{bool, float, int, string, etc.}.";
    "--mro",
        Arg.Unit (set_mode Linearization),
        " Grabs the linearization of all classes in a file.";
  ] in
  let options = Arg.align ~limit:25 options in
  Arg.parse options (fun fn -> fn_ref := Some fn) usage;
  let fn = match !fn_ref with
    | Some fn -> fn
    | None -> die usage in
  let tcopt = {
    GlobalOptions.default with
      GlobalOptions.tco_assume_php = not !dont_assume_php;
      GlobalOptions.tco_unsafe_rx = !unsafe_rx;
      GlobalOptions.tco_user_attrs = Some SSet.empty;
      GlobalOptions.tco_disable_primitive_refinement = !disable_primitive_refinement;
      GlobalOptions.tco_safe_array = !safe_array;
      GlobalOptions.tco_safe_vector_array = !safe_vector_array;
      GlobalOptions.po_deregister_php_stdlib = !deregister_attributes;
      GlobalOptions.tco_disallow_ambiguous_lambda = !disallow_ambiguous_lambda;
      GlobalOptions.tco_untyped_nonstrict_lambda_parameters =
        !untyped_nonstrict_lambda_parameters;
      GlobalOptions.tco_disallow_array_typehint = !disallow_array_typehint;
      GlobalOptions.tco_disallow_array_literal = !disallow_array_literal;
      GlobalOptions.tco_dynamic_view = !dynamic_view;
      GlobalOptions.tco_disallow_array_as_tuple = not !allow_array_as_tuple;
      GlobalOptions.tco_disallow_return_by_ref = not !allow_return_by_ref;
      GlobalOptions.tco_disallow_array_cell_pass_by_ref = not !allow_array_cell_pass_by_ref;
      GlobalOptions.tco_disallow_unset_on_varray = !disallow_unset_on_varray;
      GlobalOptions.po_auto_namespace_map = !auto_namespace_map;
  } in
  let tcopt = {
    tcopt with
      GlobalOptions.tco_experimental_features = SSet.filter begin fun x ->
        if x = GlobalOptions.tco_experimental_forbid_nullable_cast
        then !forbid_nullable_cast
        else if x = GlobalOptions.tco_experimental_disable_optional_and_unknown_shape_fields
        then !disable_optional_and_unknown_shape_fields
        else if x = GlobalOptions.tco_experimental_reified_generics
        then not (!disallow_reified_generics)
        else if x = GlobalOptions.tco_experimental_void_is_type_of_null
        then !void_is_type_of_null
        else true
      end tcopt.GlobalOptions.tco_experimental_features;
  } in
  { filename = fn;
    mode = !mode;
    no_builtins = !no_builtins;
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
            | Expr (_, New ((_, CI ((_, "\\least_upper_bound"), hints)), _, _)) ->
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
  let delim = Str.regexp "////.*\n" in
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
  let open Ide_api_types in
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

let print_coverage type_acc =
  ClientCoverageMetric.go ~json:false (Some (Coverage_level.Leaf type_acc))

let check_errors opts errors files_info =
  Relative_path.Map.fold files_info ~f:begin fun fn fileinfo errors ->
    errors @ Errors.get_error_list
        (Typing_check_utils.check_defs opts fn fileinfo)
  end ~init:errors

let create_nasts opts files_info =
  let open Core_result in
  let open Nast in
  let build_nast fn {FileInfo.funs; classes; typedefs; consts; _} =
    List.map ~f:Core_result.ok_or_failwith (
      List.map funs begin fun (_, x) ->
        Parser_heap.find_fun_in_file ~full:true opts fn x
        |> Core_result.of_option ~error:(Printf.sprintf "Couldn't find function %s" x)
        >>| Naming.fun_ opts
        >>| (fun f -> {f with f_body = (NamedBody (Typing_naming_body.func_body opts f))})
        >>| (fun f -> Nast.Fun f)
      end
      @
      List.map classes begin fun (_, x) ->
        Parser_heap.find_class_in_file ~full:true opts fn x
        |> Core_result.of_option ~error:(Printf.sprintf "Couldn't find class %s" x)
        >>| Naming.class_ opts
        >>| Typing_naming_body.class_meth_bodies opts
        >>| (fun c -> Nast.Class c)
      end
      @
      List.map typedefs begin fun (_, x) ->
        Parser_heap.find_typedef_in_file ~full:true opts fn x
        |> Core_result.of_option ~error:(Printf.sprintf "Couldn't find typedef %s" x)
        >>| Naming.typedef opts
        >>| (fun t -> Nast.Typedef t)
      end
      @
      List.map consts begin fun (_, x) ->
        Parser_heap.find_const_in_file ~full:true opts fn x
        |> Core_result.of_option ~error:(Printf.sprintf "Couldn't find const %s" x)
        >>| Naming.global_const opts
        >>| fun g -> Nast.Constant g
      end
    )
  in Relative_path.Map.mapi ~f:(build_nast) files_info

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

let parse_name_and_decl popt files_contents tcopt =
  Errors.do_ begin fun () ->
    let parsed_files =
      Relative_path.Map.mapi files_contents ~f:begin fun fn contents ->
        Errors.run_in_context fn Errors.Parsing begin fun () ->
          Full_fidelity_ast.defensive_program popt fn contents
        end
      end
    in
    let files_info =
      Relative_path.Map.mapi ~f:begin fun fn parsed_file ->
        let {Parser_return.file_mode; comments; ast; _} = parsed_file in
        let ast = if ParserOptions.deregister_php_stdlib popt then
          Ast_utils.deregister_ignored_attributes ast else ast in

        Parser_heap.ParserHeap.add fn (ast, Parser_heap.Full);
        (* If the feature is turned on, deregister functions with attribute
        __PHPStdLib. This does it for all functions, not just hhi files *)
        let funs, classes, typedefs, consts = Ast_utils.get_defs ast in
        { FileInfo.
          file_mode; funs; classes; typedefs; consts; comments = Some comments;
          hash = None;
        }
      end parsed_files in

    Relative_path.Map.iter files_info begin fun fn fileinfo ->
      Errors.run_in_context fn Errors.Naming begin fun () ->
        let {FileInfo.funs; classes; typedefs; consts; _} = fileinfo in
        NamingGlobal.make_env popt ~funs ~classes ~typedefs ~consts
      end
    end;

    Relative_path.Map.iter files_info begin fun fn _ ->
      Errors.run_in_context fn Errors.Decl begin fun () ->
        Decl.make_env tcopt fn
      end
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
  let _, _= parse_name_and_decl popt files_contents tcopt in

  let typedefs2, funs2, classes2 = get_decls defs in

  List.iter2_exn typedefs1 typedefs2 compare_typedefs;
  List.iter2_exn funs1 funs2 compare_funs;
  List.iter2_exn classes1 classes2 compare_classes;
  ()

(* Returns a list of Tast defs, along with associated type environments. *)
let get_tast opts filename files_info =
  Errors.do_ begin fun () ->
    let nasts = create_nasts opts files_info in
    let nast = Relative_path.Map.find filename nasts in
    Typing.nast_to_tast opts nast
  end

let handle_mode
  mode filename tcopt popt files_contents files_info errors =
  match mode with
  | Ai _ -> ()
  | Autocomplete
  | Autocomplete_manually_invoked ->
      let token = "AUTO332" in
      let token_len = String.length token in
      let file = cat (Relative_path.to_absolute filename) in
      (* Search backwards: there should only be one /real/ case. If there's multiple, *)
      (* guess that the others are preceding explanation comments *)
      let offset = Str.search_backward (Str.regexp token) file (String.length file) in
      let pos = File_content.offset_to_position file offset in
      let file = (Str.string_before file offset) ^ (Str.string_after file (offset + token_len)) in
      let is_manually_invoked = mode = Autocomplete_manually_invoked in

      let result = ServerAutoComplete.auto_complete_at_position
        ~tcopt ~pos ~is_manually_invoked ~delimit_on_namespaces:false ~file_content:file in
      List.iter ~f: begin fun r ->
        let open AutocompleteTypes in
        Printf.printf "%s %s\n" r.res_name r.res_ty
      end result.Utils.With_complete_flag.value
  | Ffp_autocomplete ->
      begin try
        let file_text = cat (Relative_path.to_absolute filename) in
        (* TODO: Use a magic word/symbol to identify autocomplete location instead *)
        let args_regex = Str.regexp "AUTOCOMPLETE [1-9][0-9]* [1-9][0-9]*" in
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
          FfpAutocompleteService.auto_complete tcopt file_text position
          ~filter_by_token:true
        in
        match result with
        | [] -> Printf.printf "No result found\n"
        | res -> List.iter res ~f:begin fun r ->
            let open AutocompleteTypes in
            Printf.printf "%s\n" r.res_name
          end
      with
      | Failure msg
      | Invalid_argument msg ->
        Printf.printf "%s\n" msg;
        exit 1
      end
  | Color ->
      Relative_path.Map.iter files_info begin fun fn fileinfo ->
        if Relative_path.Map.mem builtins fn then () else begin
          let tast, _ = Typing_check_utils.type_file tcopt fn fileinfo in
          let result = Coverage_level.get_levels tast filename in
          print_colored fn result;
        end
      end
  | Coverage ->
      let p_tcopt = TypecheckerOptions.make_permissive tcopt in
      Relative_path.Map.iter files_info begin fun fn fileinfo ->
        if Relative_path.Map.mem builtins fn then () else begin
          let tast, _ = Typing_check_utils.type_file p_tcopt fn fileinfo in
          let type_acc = ServerCoverageMetric.accumulate_types tast filename in
          print_coverage type_acc;
        end
      end
  | Cst_search ->
    let open Core_result.Monad_infix in
    let source_text = Full_fidelity_source_text.from_file filename in
    let syntax_tree = PositionedTree.make source_text in

    let result = Sys_utils.read_stdin_to_string ()
      |> Hh_json.json_of_string
      |> CstSearchService.compile_pattern
      >>| CstSearchService.search ~syntax_tree
      >>| CstSearchService.result_to_json ~sort_results:true
      >>| Hh_json.json_to_string ~pretty:true
    in
    begin match result with
    | Ok result -> Printf.printf "%s\n" result
    | Error message ->
      Printf.printf "%s\n" message;
      exit 1
    end
  | Dump_symbol_info ->
      begin match Relative_path.Map.get files_info filename with
        | Some fileinfo ->
            let raw_result =
              SymbolInfoService.helper tcopt [] [(filename, fileinfo)] in
            let result = SymbolInfoService.format_result raw_result in
            let result_json = ClientSymbolInfo.to_json result in
            print_endline (Hh_json.json_to_multiline result_json)
        | None -> ()
      end
  | Lint ->
      let lint_errors = Relative_path.Map.fold files_contents ~init:[]
        ~f:begin fun fn content lint_errors ->
          lint_errors @ fst (Lint.do_ begin fun () ->
            Linting_service.lint tcopt fn content
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
  | Dump_bazooka_deps
  | Dump_deps ->
    (* Don't typecheck builtins *)
    let files_info = Relative_path.Map.fold builtins
      ~f:begin fun k _ acc -> Relative_path.Map.remove acc k end
      ~init:files_info
    in
    Relative_path.Map.iter files_info begin fun fn fileinfo ->
      ignore @@ Typing_check_utils.check_defs tcopt fn fileinfo
    end;
    Typing_deps.dump_debug_deps ()

  | Dump_toplevel_deps ->
    (* Don't typecheck builtins *)
    let files_info = Relative_path.Map.fold builtins
      ~f:begin fun k _ acc -> Relative_path.Map.remove acc k end
      ~init:files_info
    in
    Relative_path.Map.iter files_info begin fun fn _fileinfo ->
      let ast = Parser_heap.get_from_parser_heap popt fn in
      Dependency_visitors.print_deps popt ast
    end
  | Dump_inheritance ->
    let open ServerCommandTypes.Method_jumps in
    Typing_deps.update_files files_info;
    Relative_path.Map.iter files_info begin fun fn fileinfo ->
      if Relative_path.Map.mem builtins fn then () else begin
        List.iter fileinfo.FileInfo.classes begin fun (_p, class_) ->
          Printf.printf "Ancestors of %s and their overridden methods:\n"
            class_;
          let ancestors = MethodJumps.get_inheritance tcopt class_
            ~filter:No_filter ~find_children:false files_info
            None in
          ClientMethodJumps.print_readable ancestors ~find_children:false;
          Printf.printf "\n";
        end;
        Printf.printf "\n";
        List.iter fileinfo.FileInfo.classes begin fun (_p, class_) ->
          Printf.printf "Children of %s and the methods they override:\n"
            class_;
          let children = MethodJumps.get_inheritance tcopt class_
            ~filter:No_filter ~find_children:true files_info None in
          ClientMethodJumps.print_readable children ~find_children:true;
          Printf.printf "\n";
        end;
      end
    end;
  | Identify_symbol (line, column) ->
    let file = cat (Relative_path.to_absolute filename) in
    begin match ServerIdentifyFunction.go_absolute file line column tcopt with
      | [] -> print_endline "None"
      | result -> ClientGetDefinition.print_readable ~short_pos:true result
    end
  | Find_local (line, column) ->
    let file = cat (Relative_path.to_absolute filename) in
    let result = ServerFindLocals.go popt filename file line column in
    let print pos = Printf.printf "%s\n" (Pos.string_no_file pos) in
    List.iter result print
  | Outline ->
    let file = cat (Relative_path.to_absolute filename) in
    let results = FileOutline.outline popt file in
    FileOutline.print ~short_pos:true results
  | Dump_nast ->
    let nasts = create_nasts tcopt files_info in
    let nast = Relative_path.Map.find filename nasts in
    Printf.printf "%s\n" (Nast.show_program nast)
  | Dump_tast ->
    let errors, tast = get_tast tcopt filename files_info in
    (match Errors.get_error_list errors with
    | [] -> ()
    | errors ->
      Printf.printf "Errors:\n";
      List.iter errors (fun err ->
        List.iter (Errors.to_list err) (fun (pos, msg) ->
          Format.printf "  %a %s" Pos.pp pos msg;
          Format.print_newline ()))
    );
    let env = Typing_env.empty tcopt filename ~droot:None in
    let stringify_types tast =
      let tast = Tast_expand.expand_program tast in
      let print_pos_and_ty (pos, ty) =
        Format.asprintf "(%a, %s)" Pos.pp pos (Typing_print.full_strip_ns env ty)
      in
      TASTStringMapper.map_program tast
        ~map_env_annotation:(fun () -> ())
        ~map_expr_annotation:print_pos_and_ty
    in
    let string_ast = stringify_types tast in
    Printf.printf "%s\n" (StringNAST.show_program string_ast)

  | Dump_typed_full_fidelity_json ->
    (*
      Ideally we'd reuse ServerTypedAst.go here. Unfortunately relative file
      paths are not compatible between hh_single_type_check and server modules.
      So we copy here instead.
    *)

    (* get the typed ast *)
    let _, tast = get_tast tcopt filename files_info in

    (* get the parse tree *)
    let source_text = Full_fidelity_source_text.from_file filename in
    let positioned_tree = PositionedTree.make source_text in
    let typed_tree = ServerTypedAst.create_typed_parse_tree
      ~filename ~positioned_tree ~tast in

    let result = ServerTypedAst.typed_parse_tree_to_json typed_tree in
    Printf.printf "%s\n" (Hh_json.json_to_string result);
  | Dump_stripped_tast ->
    let _, tast = get_tast tcopt filename files_info in
    let nast = Tast.to_nast tast in
    Printf.printf "%s\n" (Nast.show_program nast)
  | Find_refs (line, column) ->
    Typing_deps.update_files files_info;
    Relative_path.set_path_prefix Relative_path.Root (Path.make "/");
    Relative_path.set_path_prefix Relative_path.Hhi (Path.make "hhi");
    Relative_path.set_path_prefix Relative_path.Tmp (Path.make "tmp");
    let genv = ServerEnvBuild.default_genv in
    let env = {(ServerEnvBuild.make_env genv.ServerEnv.config) with
      ServerEnv.files_info;
      ServerEnv.tcopt = tcopt;
    } in
    let filename = Relative_path.to_absolute filename in
    let content = cat filename in
    let include_defs = true in
    let labelled_file = ServerCommandTypes.LabelledFileContent { filename; content; } in
    let open Option.Monad_infix in
    let open ServerCommandTypes.Done_or_retry in
    let results = ServerFindRefs.(
      go_from_file (labelled_file, line, column) env >>= fun (name, action) ->
      go action include_defs genv env |>
      map_env ~f:(to_ide name) |>
      snd |> function
        | Done r -> r
        | Retry -> failwith @@ "should only happen with prechecked files " ^
                               "which are not a thing in hh_single_type_check"
    ) in
    ClientFindRefs.print_ide_readable results;
  | Highlight_refs (line, column) ->
    let file = cat (Relative_path.to_absolute filename) in
    let results = ServerHighlightRefs.go (file, line, column) tcopt  in
    ClientHighlightRefs.go results ~output_json:false;
  | Suggest
  | Infer_return_types
  | Errors ->
      (* Don't typecheck builtins *)
      let files_info = Relative_path.Map.fold builtins
        ~f:begin fun k _ acc -> Relative_path.Map.remove acc k end
        ~init:files_info
      in
      let errors = check_errors tcopt errors files_info in
      if mode = Suggest
      then Relative_path.Map.iter files_info (suggest_and_print tcopt);
      if mode = Infer_return_types
      then
        Option.iter ~f:(infer_return tcopt filename)
          (Relative_path.Map.get files_info filename);
      if errors <> []
      then (error (List.hd_exn errors); exit 2)
      else Printf.printf "No errors\n"
  | AllErrors ->
      let errors = check_errors tcopt errors files_info in
      if errors <> []
      then (List.iter ~f:(error ~indent:true) errors; exit 2)
      else Printf.printf "No errors\n"
  | Decl_compare ->
    test_decl_compare filename popt files_contents tcopt files_info
  | Least_upper_bound-> compute_least_type tcopt popt filename
  | Linearization ->
    if errors <> [] then  (error (List.hd_exn errors); exit 2);
    let files_info = Relative_path.Map.fold builtins
      ~f:begin fun k _ acc -> Relative_path.Map.remove acc k end
      ~init:files_info
    in
    Relative_path.Map.iter files_info ~f:(fun _ info ->
      let { FileInfo.classes; _} = info in
      (* For each class, grab the declaration from the decl_heap*)
      let linearizations = List.map classes
        begin fun (_, c) ->
            let class_decl = Decl_heap.Classes.find_unsafe c in
            c, class_decl.Decl_defs.dc_linearization
        end in
      List.iter linearizations ~f:(fun (classname, linearization) ->
        Printf.printf "Linearization for class %s:\n" classname;
        Printf.printf "[%s]\n" (String.concat ", " linearization)
      )
    )



(*****************************************************************************)
(* Main entry point *)
(*****************************************************************************)

let decl_and_run_mode {filename; mode; no_builtins; tcopt } popt =
  if mode = Dump_deps then Typing_deps.debug_trace := Typing_deps.Full
  else if mode = Dump_bazooka_deps
    then Typing_deps.debug_trace := Typing_deps.Bazooka;
  Ident.track_names := true;
  let builtins = if no_builtins then Relative_path.Map.empty else builtins in
  let filename = Relative_path.create Relative_path.Dummy filename in
  let files_contents = file_to_files filename in
  (* Merge in builtins *)
  let files_contents_with_builtins = Relative_path.Map.fold builtins
    ~f:begin fun k src acc -> Relative_path.Map.add acc ~key:k ~data:src end
    ~init:files_contents
  in

  let errors, files_info =
    parse_name_and_decl popt files_contents_with_builtins tcopt in

  handle_mode mode filename tcopt popt files_contents files_info
    (Errors.get_error_list errors)

let main_hack ({filename; mode; tcopt; _} as opts) =
  (* TODO: We should have a per file config *)
  Sys_utils.signal Sys.sigusr1
    (Sys.Signal_handle Typing.debug_print_last_pos);
  EventLogger.init ~exit_on_parent_exit EventLogger.Event_logger_fake 0.0;
  let _handle = SharedMem.init GlobalConfig.default_sharedmem_config in
  let tmp_hhi = Path.concat (Path.make Sys_utils.temp_dir_name) "hhi" in
  Hhi.set_hhi_root_for_unit_test tmp_hhi;
  match mode with
  | Ai ai_options ->
    Ai.do_ Typing_check_utils.type_file filename ai_options
  | _ ->
    decl_and_run_mode opts tcopt

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
