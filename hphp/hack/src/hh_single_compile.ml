(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core
open String_utils
open Sys_utils

(*****************************************************************************)
(* Types, constants *)
(*****************************************************************************)

type options = {
  filename : string;
  no_builtins : bool;
}

let builtins_filename =
  Relative_path.create Relative_path.Dummy "builtins.hhi"

let builtins =
  "<?hh // decl\n"^
  "interface Traversable<+Tv> {}\n"^
  "interface Container<+Tv> extends Traversable<Tv> {}\n"^
  "interface Iterator<+Tv> extends Traversable<Tv> {}\n"^
  "interface Iterable<+Tv> extends Traversable<Tv> {}\n"^
  "interface KeyedTraversable<+Tk, +Tv> extends Traversable<Tv> {}\n"^
  "interface KeyedContainer<+Tk, +Tv> extends Container<Tv>, KeyedTraversable<Tk,Tv> {}\n"^
  "interface KeyedIterator<+Tk, +Tv> extends KeyedTraversable<Tk, Tv>, Iterator<Tv> {}\n"^
  "interface KeyedIterable<Tk, +Tv> extends KeyedTraversable<Tk, Tv>, Iterable<Tv> {}\n"^
  "interface Awaitable<+T> {"^
  "  public function getWaitHandle(): WaitHandle<T>;"^
  "}\n"^
  "interface WaitHandle<+T> extends Awaitable<T> {}\n"^
  "interface ConstVector<+Tv> extends KeyedIterable<int, Tv>, KeyedContainer<int, Tv>{"^
  "  public function map<Tu>((function(Tv): Tu) $callback): ConstVector<Tu>;"^
  "}\n"^
  "interface ConstSet<+Tv> extends KeyedIterable<mixed, Tv>, Container<Tv>{}\n"^
  "interface ConstMap<Tk, +Tv> extends KeyedIterable<Tk, Tv>, KeyedContainer<Tk, Tv>{"^
  "  public function map<Tu>((function(Tv): Tu) $callback): ConstMap<Tk, Tu>;"^
  "  public function mapWithKey<Tu>((function(Tk, Tv): Tu) $fn): ConstMap<Tk, Tu>;"^
  "}\n"^
  "final class Vector<Tv> implements ConstVector<Tv>{\n"^
  "  public function map<Tu>((function(Tv): Tu) $callback): Vector<Tu>;\n"^
  "  public function filter((function(Tv): bool) $callback): Vector<Tv>;\n"^
  "  public function reserve(int $sz): void;"^
  "  public function add(Tv $value): Vector<Tv>;"^
  "  public function addAll(?Traversable<Tv> $it): Vector<Tv>;"^
  "}\n"^
  "final class ImmVector<+Tv> implements ConstVector<Tv> {"^
  "  public function map<Tu>((function(Tv): Tu) $callback): ImmVector<Tu>;"^
  "}\n"^
  "final class Map<Tk, Tv> implements ConstMap<Tk, Tv> {"^
  "  /* HH_FIXME[3007]: This is intentional; not a constructor */"^
  "  public function map<Tu>((function(Tv): Tu) $callback): Map<Tk, Tu>;"^
  "  public function mapWithKey<Tu>((function(Tk, Tv): Tu) $fn): Map<Tk, Tu>;"^
  "  public function contains(Tk $k): bool;"^
  "}\n"^
  "final class ImmMap<Tk, +Tv> implements ConstMap<Tk, Tv>{"^
  "  public function map<Tu>((function(Tv): Tu) $callback): ImmMap<Tk, Tu>;"^
  "  public function mapWithKey<Tu>((function(Tk, Tv): Tu) $fn): ImmMap<Tk, Tu>;"^
  "}\n"^
  "final class StableMap<Tk, Tv> implements ConstMap<Tk, Tv> {"^
  "  public function map<Tu>((function(Tv): Tu) $callback): StableMap<Tk, Tu>;"^
  "  public function mapWithKey<Tu>((function(Tk, Tv): Tu) $fn): StableMap<Tk, Tu>;"^
  "}\n"^
  "final class Set<Tv> implements ConstSet<Tv> {}\n"^
  "final class ImmSet<+Tv> implements ConstSet<Tv> {}\n"^
  "class Exception {"^
  "  public function __construct(string $x) {}"^
  "  public function getMessage(): string;"^
  "}\n"^
  "class Generator<Tk, +Tv, -Ts> implements KeyedIterator<Tk, Tv> {\n"^
  "  public function next(): void;\n"^
  "  public function current(): Tv;\n"^
  "  public function key(): Tk;\n"^
  "  public function rewind(): void;\n"^
  "  public function valid(): bool;\n"^
  "  public function send(?Ts $v): void;\n"^
  "}\n"^
  "final class Pair<+Tk, +Tv> implements KeyedContainer<int,mixed> {public function isEmpty(): bool {}}\n"^
  "interface Stringish {public function __toString(): string {}}\n"^
  "interface XHPChild {}\n"^
  "function hh_show($val) {}\n"^
  "function hh_show_env() {}\n"^
  "interface Countable { public function count(): int; }\n"^
  "interface AsyncIterator<+Tv> {}\n"^
  "interface AsyncKeyedIterator<+Tk, +Tv> extends AsyncIterator<Tv> {}\n"^
  "class AsyncGenerator<Tk, +Tv, -Ts> implements AsyncKeyedIterator<Tk, Tv> {\n"^
  "  public function next(): Awaitable<?(Tk, Tv)> {}\n"^
  "  public function send(?Ts $v): Awaitable<?(Tk, Tv)> {}\n"^
  "  public function raise(Exception $e): Awaitable<?(Tk, Tv)> {}"^
  "}\n"^
  "function isset($x): bool;"^
  "function empty($x): bool;"^
  "function unset($x): void;"^
  "namespace HH {\n"^
  "abstract class BuiltinEnum<T> {\n"^
  "  final public static function getValues(): array<string, T>;\n"^
  "  final public static function getNames(): array<T, string>;\n"^
  "  final public static function coerce(mixed $value): ?T;\n"^
  "  final public static function assert(mixed $value): T;\n"^
  "  final public static function isValid(mixed $value): bool;\n"^
  "  final public static function assertAll(Traversable<mixed> $values): Container<T>;\n"^
  "}\n"^
  "}\n"^
  "function array_map($x, $y, ...);\n"^
  "function idx<Tk, Tv>(?KeyedContainer<Tk, Tv> $c, $i, $d = null) {}\n"^
  "final class stdClass {}\n" ^
  "function rand($x, $y): int;\n" ^
  "function invariant($x, ...): void;\n" ^
  "function exit(int $exit_code_or_message = 0): noreturn;\n" ^
  "function invariant_violation(...): noreturn;\n" ^
  "function get_called_class(): string;\n" ^
  "abstract final class Shapes {\n" ^
  "  public static function idx(shape() $shape, arraykey $index, $default = null) {}\n" ^
  "  public static function keyExists(shape() $shape, arraykey $index): bool {}\n" ^
  "  public static function removeKey(shape() $shape, arraykey $index): void {}\n" ^
  "  public static function toArray(shape() $shape): array<arraykey, mixed> {}\n" ^
  "}\n" ^
  "newtype typename<+T> as string = string;\n"^
  "newtype classname<+T> as typename<T> = typename<T>;\n" ^
 "function var_dump($x): void;\n" ^
  "function gena();\n" ^
  "function genva();\n" ^
  "function gen_array_rec();\n"^
  "function is_int(mixed $x): bool {}\n"^
  "function is_bool(mixed $x): bool {}\n"^
  "function is_float(mixed $x): bool {}\n"^
  "function is_string(mixed $x): bool {}\n"^
  "function is_null(mixed $x): bool {}\n"^
  "function is_array(mixed $x): bool {}\n"^
  "function is_vec(mixed $x): bool {}\n"^
  "function is_dict(mixed $x): bool {}\n"^
  "function is_keyset(mixed $x): bool {}\n"^
  "function is_resource(mixed $x): bool {}\n"^
  "interface IMemoizeParam {\n"^
  "  public function getInstanceKey(): string;\n"^
  "}\n"^
  "newtype TypeStructure<T> as shape(\n"^
  "  'kind'=> int,\n"^
  "  'nullable'=>?bool,\n"^
  "  'classname'=>?classname<T>,\n"^
  "  'elem_types' => ?array,\n"^
  "  'param_types' => ?array,\n"^
  "  'return_type' => ?array,\n"^
  "  'generic_types' => ?array,\n"^
  "  'fields' => ?array,\n"^
  "  'name' => ?string,\n"^
  "  'alias' => ?string,\n"^
  ") = shape(\n"^
  "  'kind'=> int,\n"^
  "  'nullable'=>?bool,\n"^
  "  'classname'=>?classname<T>,\n"^
  "  'elem_types' => ?array,\n"^
  "  'param_types' => ?array,\n"^
  "  'return_type' => ?array,\n"^
  "  'generic_types' => ?array,\n"^
  "  'fields' => ?array,\n"^
  "  'name' => ?string,\n"^
  "  'alias' => ?string,\n"^
  ");\n"^
  "function type_structure($x, $y);\n"^
  "const int __LINE__ = 0;\n"^
  "const string __CLASS__ = '';\n"^
  "const string __TRAIT__ = '';\n"^
  "const string __FILE__ = '';\n"^
  "const string __DIR__ = '';\n"^
  "const string __FUNCTION__ = '';\n"^
  "const string __METHOD__ = '';\n"^
  "const string __NAMESPACE__ = '';\n"^
  "interface Indexish<+Tk, +Tv> extends KeyedContainer<Tk, Tv> {}\n"^
  "abstract final class dict<+Tk, +Tv> implements Indexish<Tk, Tv> {}\n"^
  "function dict<Tk, Tv>(KeyedTraversable<Tk, Tv> $arr): dict<Tk, Tv> {}\n"^
  "abstract final class keyset<+T as arraykey> implements Indexish<T, T> {}\n"^
  "abstract final class vec<+Tv> implements Indexish<int, Tv> {}\n"^
  "function meth_caller(string $cls_name, string $meth_name);\n"^
  "namespace HH\\Asio {"^
  "  function va(...$args);\n"^
  "}\n"^
  "function hh_log_level(int $level) {}\n"

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let die str =
  let oc = stderr in
  output_string oc str;
  close_out oc;
  exit 2

let parse_options () =
  let fn_ref = ref None in
  let usage = Printf.sprintf "Usage: %s filename\n" Sys.argv.(0) in
  let no_builtins = ref false in
  let options = [
    "--no-builtins",
      Arg.Set no_builtins,
      " Don't use builtins (e.g. ConstSet)";
  ] in
  let options = Arg.align ~limit:25 options in
  Arg.parse options (fun fn -> fn_ref := Some fn) usage;
  let fn = match !fn_ref with
    | Some fn -> fn
    | None -> die usage in
  { filename = fn;
    no_builtins = !no_builtins;
  }

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
          consider_names_just_for_autoload = false }
      end parsed_files in

    Relative_path.Map.iter files_info begin fun _ fileinfo ->
      let {FileInfo.funs; classes; typedefs; consts; _} = fileinfo in
      NamingGlobal.make_env popt ~funs ~classes ~typedefs ~consts
    end;

    Relative_path.Map.iter files_info begin fun fn _ ->
      Decl.make_env tcopt fn
    end;

    files_info
  end

let do_compile opts files_info = begin
  let f_fold fn fileinfo text = begin
    let hhas_text = if (Relative_path.S.to_string fn) = "|builtins.hhi" then
      ""
    else
      let (named_functions, named_classes, _named_typedefs, _named_consts) =
        Typing_check_utils.get_nast_from_fileinfo opts fn fileinfo in
      let compiled_funs = Hhbc_from_nast.from_functions named_functions in
      let compiled_classes = Hhbc_from_nast.from_classes named_classes in
      let _compiled_typedefs = [] in (* TODO *)
      let _compiled_consts = [] in (* TODO *)
      let hhas_prog = Hhas_program.make compiled_funs compiled_classes in
      Hhbc_hhas.to_string hhas_prog in
    text ^ hhas_text
  end in
  let hhas_text = Relative_path.Map.fold files_info ~f:f_fold ~init:"" in
  Printf.printf "%s" hhas_text
end

(*****************************************************************************)
(* Main entry point *)
(*****************************************************************************)

let decl_and_run_mode {filename; no_builtins} popt tcopt =
  Local_id.track_names := true;
  Ident.track_names := true;
  let builtins = if no_builtins then "" else builtins in
  let filename = Relative_path.create Relative_path.Dummy filename in
  let files_contents = file_to_files filename in
  let files_contents_with_builtins = Relative_path.Map.add files_contents
    ~key:builtins_filename ~data:builtins in

  let _, files_info, _ =
    parse_name_and_decl popt files_contents_with_builtins tcopt in

  do_compile tcopt files_info

let main_hack opts =
  let popt = ParserOptions.default in
  let tcopt = TypecheckerOptions.default in
  Sys_utils.signal Sys.sigusr1
    (Sys.Signal_handle Typing.debug_print_last_pos);
  EventLogger.init EventLogger.Event_logger_fake 0.0;
  let _handle = SharedMem.init GlobalConfig.default_sharedmem_config in
  let tmp_hhi = Path.concat (Path.make Sys_utils.temp_dir_name) "hhi" in
  Hhi.set_hhi_root_for_unit_test tmp_hhi;
  decl_and_run_mode opts popt tcopt

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
