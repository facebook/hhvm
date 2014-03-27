(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Utils

(*****************************************************************************)
(* Types, constants *)
(*****************************************************************************)

type options = { filename : string; suggest : bool; flow : bool }

let builtins =
  "<?hh // decl\n"^
  "class Object { public function get_class(): string { } "^
  "public function get_parent_class(): ?string { } }"^
  "interface Traversable<Tv> {}"^
  "interface Iterator<Tv> extends Traversable<Tv> {}"^
  "interface Iterable<Tv> extends Traversable<Tv> {}"^
  "interface KeyedTraversable<Tk, Tv> extends Traversable<Tv> {}"^
  "interface Indexish<Tk, Tv> extends KeyedTraversable<Tk, Tv> {}"^
  "interface KeyedIterator<Tk, Tv> extends KeyedTraversable<Tk, Tv>, Iterator<Tv> {}"^
  "interface KeyedIterable<Tk, Tv> extends KeyedTraversable<Tk, Tv>, Iterable<Tv> {}"^
  "interface Awaitable<T> { }"^
  "interface WaitHandle<T> extends Awaitable<T> { }"^
  "final class Vector<Tv> implements KeyedIterable<int, Tv>, Indexish<int, Tv> {}"^
  "final class ImmVector<Tv> implements KeyedIterable<int, Tv> {}"^
  "final class Map<Tk, Tv> implements KeyedIterable<Tk, Tv>, Indexish<Tk, Tv> {}"^
  "final class ImmMap<Tk, Tv> implements KeyedIterable<Tk, Tv> {}"^
  "final class StableMap<Tk, Tv> implements KeyedIterable<Tk, Tv>, Indexish<Tk, Tv> {}"^
  "final class Set<Tv> extends Iterable<Tv> {}"^
  "final class ImmSet<Tv> extends Iterable<Tv> {}"^
  "class Exception { public function __construct(string $x) {} }"^
  "interface Continuation<Tv> implements Iterator<Tv> { "^
  "  public function next(): void;"^
  "  public function current(): Tv;"^
  "  public function key(): int;"^
  "  public function rewind(): void;"^
  "  public function valid(): bool;"^
  "}"^
  "final class Pair<Tk, Tv> {public function isEmpty(): bool {}}"^
  "interface Stringish {public function __toString(): string {}}"^
  "interface XHPChild {}"^
  "interface ConstVector<Tv> {}"^
  "interface ConstMap<Tk, Tv> {}"^
  "function hh_show($val) {}"

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let die str =
  let oc = stderr in
  output_string oc str;
  close_out oc;
  exit 2

let error l = die (Utils.pmsg_l l)


let parse_options () =
  let fn_ref = ref None in
  let suggest = ref false in
  let flow = ref false in
  let usage = Printf.sprintf "Usage: %s filename\n" Sys.argv.(0) in
  let options = [
    "--suggest",
      Arg.Set suggest,
      "Suggest missing typehints";
    "--flow",
      Arg.Set flow,
      "";
  ] in
  Arg.parse options (fun fn -> fn_ref := Some fn) usage;
  let fn = match !fn_ref with
    | Some fn -> fn
    | None -> die usage in
  { filename = fn; suggest = !suggest; flow = !flow }

let suggest_and_print fn funs classes typedefs consts =
  let make_set =
    List.fold_left (fun acc (_, x) -> SSet.add x acc) SSet.empty in
  let n_funs = make_set funs in
  let n_classes = make_set classes in
  let n_types = make_set typedefs in
  let n_consts = make_set consts in
  let names = { FileInfo.n_funs; n_classes; n_types; n_consts } in
  let fast = SMap.add fn names SMap.empty in
  let patch_map = Typing_suggest_service.go None fast in
  match SMap.get fn patch_map with
    | None -> ()
    | Some l -> begin
      (* Sort so that the unit tests come out in a consistent order, normally
       * doesn't matter. *)
      let l = List.sort (fun (x, _, _) (y, _, _) -> x - y) l in
      List.iter (ServerConvert.print_patch fn) l
    end

(* This allows to fake having multiple files in one file. This
 * is used only in unit test files. Indeed
 * There are some features that require mutliple files to be tested.
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

let parse_file fn =
  let ic = open_in fn in
  let buf = Buffer.create 256 in
  Buffer.add_channel buf ic (in_channel_length ic);
  let content = Buffer.contents buf in
  close_in ic;
  let delim = Str.regexp "////.*" in
  if Str.string_match delim content 0
  then
    let contentl = Str.full_split delim content in
    let files = make_files contentl in
    List.fold_right begin fun (sub_fn, content) ast ->
      Pos.file := fn^"--"^sub_fn ;
      Parser_hack.program content @ ast
    end files []
  else begin
    Pos.file := fn ;
    Parser_hack.program content
  end

(* collect definition names from parsed ast *)
let collect_defs ast =
  List.fold_right begin fun def (funs, classes, typedefs, consts) ->
    match def with
    | Ast.Fun f -> f.Ast.f_name :: funs, classes, typedefs, consts
    | Ast.Class c -> funs, c.Ast.c_name :: classes, typedefs, consts
    | Ast.Typedef td -> funs, classes, td.Ast.t_id :: typedefs, consts
    | Ast.Constant cst -> funs, classes, typedefs, cst.Ast.cst_name :: consts
    | _ -> funs, classes, typedefs, consts
  end ast ([], [], [], [])

(*****************************************************************************)
(* Main entry point *)
(*****************************************************************************)

(* This was the original main ... before there was a daemon.
 * This function can also be called interactively from top_single to
 * populate the global typing environment (see typing_env.ml) for
 * a given file. You can then inspect this typing environment, e.g.
 * with 'Typing_env.Classes.get "Foo";;'
 *)
let main_hack { filename; suggest } =
  SharedMem.init();
  Typing.debug := true;
  try
    Pos.file := filename;
    let ast_builtins = Parser_hack.program builtins in
    let ast = ast_builtins @ parse_file filename in
    let ast = Namespaces.elaborate_defs ast in
    Parser_heap.ParserHeap.add filename ast;
    let funs, classes, typedefs, consts = collect_defs ast in
    let nenv = Naming.make_env Naming.empty ~funs ~classes ~typedefs ~consts in
    let all_classes = List.fold_right begin fun (_, cname) acc ->
      SMap.add cname (SSet.singleton filename) acc
    end classes SMap.empty in
    Typing_decl.make_env nenv all_classes filename;
    List.iter (fun (_, fname) -> Typing_check_service.type_fun fname) funs;
    List.iter (fun (_, cname) -> Typing_check_service.type_class cname) classes;
    List.iter (fun (_, x) -> Typing_check_service.check_typedef x) typedefs;
    Printf.printf "No errors\n";
    if suggest
    then suggest_and_print filename funs classes typedefs consts
  with
  | Utils.Error l -> error l

(* flow single-file entry point *)
let main_flow { filename; suggest } =
  SharedMem.init();
  try
    Flow.main [filename]
  with
  | Utils.Error l -> error l

(* command line driver *)
let _ =
  if ! Sys.interactive
  then ()
  else
    let options = parse_options () in
    if options.flow
    then main_flow options
    else main_hack options
