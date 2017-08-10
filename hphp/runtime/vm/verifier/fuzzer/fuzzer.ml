(*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*)

(* This fuzzer works by applying semi-random mutations of different kinds to a
   hhas input file. Given a starter program, each mutation will generate a
   sample space of outcomes, the size of which is determined by user input. The
   process is structured around the non-determinism monad, so that each mutation
   is applied to every program in the output space of the previous mutation.
   Because the mutation includes its input in the output space, the output of
   the whole process includes every combination of the mutations applied at each
   step. *)

(*---------------------------------File I/0-----------------------------------*)

let out_dir = ref ""
let imm_reps = ref 1
let dup_reps = ref 1
let reorder_reps = ref 1
let replace_reps = ref 1
let remove_reps = ref 1
let insert_reps = ref 1
let exn_reps = ref 1
let metadata_reps = ref 1
let complete_reps = ref 0
let mut_prob = ref 0.1
let mag = ref 1
let debug = ref false

let options =
  [("-o",         Arg.Set_string out_dir,
      "The output directory for the mutations");
   ("-out",       Arg.Set_string out_dir,
      "The output directory for the mutations");
   ("-prob",      Arg.Set_float mut_prob,
      "The probability of a mutation occuring each pass (must be <= 1)");
   ("-magnitude", Arg.Set_int mag,
      "The magnitude of possible change for integer mutations (default 1)");
   ("-debug",     Arg.Set debug,
      "Turn on debug prints (default false)");
   ("-immediate", Arg.Set_int imm_reps,
      "Number of immediate mutations (default 1)");
   ("-duplicate", Arg.Set_int dup_reps,
      "Number of duplicate mutations (default 1)");
   ("-reorder",   Arg.Set_int reorder_reps,
      "Number of reorder mutations (default 1)");
   ("-replace",   Arg.Set_int replace_reps,
      "Number of replace mutations (default 1)");
   ("-remove",    Arg.Set_int remove_reps,
      "Number of remove mutations (default 1)");
   ("-insert",    Arg.Set_int insert_reps,
      "Number of insert mutations (default 1)");
   ("-exception",    Arg.Set_int exn_reps,
      "Number of exception mutations (default 1)");
   ("-metadata",  Arg.Set_int metadata_reps,
      "Number of metadata mutations (default 1)");
   ("-complete", Arg.Set_int complete_reps,
      "Number of complete mutations (default 0,
      overrides other mutation parameters)")
  ]

(* The mutations are printed in reverse order; mutation0 is the "most" mutated
   file, containing one of each enabled type, whereas the highest numbered
   file is identical to the input *)

let die : 'a . string -> 'a =
  fun str ->
  let oc = stderr in
  output_string oc str;
  close_out oc;
  exit 2

let print_output : Hhas_program.t -> unit =
  let m_no = ref 0 in
  fun (p : Hhas_program.t) ->
    let out =
      if !out_dir <> ""
      then open_out (!out_dir ^ "/mutation" ^ string_of_int(!m_no) ^ ".hhas")
      else stdout in
    begin try p |> Hhbc_hhas.to_string |> (Printf.fprintf out "%s\n")
    with _ -> () end;
    if (!out_dir <> "") then close_out out;
    m_no := !m_no + 1

let parse_file program_parser filename =
  let channel = open_in filename in
  let prog =
    try program_parser (Lexing.from_channel channel)
    with Parsing.Parse_error -> (
      Printf.eprintf "Parsing of file failed\n";
      raise Parsing.Parse_error
      ) in
  close_in channel; prog

let read_input () : Hhas_program.t =
  let filename = ref ""  in
  let purpose = "Hhas fuzzing tool" in
  let usage = Printf.sprintf "%s\nUsage: %s <file> -o <output dir> -prob <float>
                              -magnitude <int> -debug <bool> -immediate <int>
                              -duplicate <int> -reorder <int> -remove <int>
                              -insert <int> -exception <int> -metadata <int>
                              -complete <int> (more help in hphp/doc/fuzzer)\n"
              purpose Sys.argv.(0) in
  begin try Arg.parse options (fun file -> filename := file) usage
        with _ -> die usage end;
  let program_parser = Hhas_parser.program Hhas_lexer.read in
  if !filename = "" then die usage else
  parse_file program_parser !filename

let debug_print str = if !debug then print_endline str

(*---------------------------------Mutations----------------------------------*)

module IS = Instruction_sequence
module HP = Hhas_program
open Random_utils
open Instr_utils

type mutation_monad = HP.t Nondet.m
type mutation = HP.t -> mutation_monad

(* These couple of functions are just plumbing the Hhas_program data structure
   to access the instruction sequences, which is where the real work happens,
   and then reconstructing the program with the new, mutated sequences *)

let mutate_body (mut: IS.t -> IS.t) (b : Hhas_body.t) : Hhas_body.t =
  Hhas_body.instrs b |> mut |> Hhas_body.with_instrs b

let mutate_main (mut: IS.t -> IS.t) (p : HP.t) : Hhas_program.t =
  HP.main p |> mutate_body mut |> HP.with_main p

let mutate_functions (mut: IS.t -> IS.t) (p : HP.t) : Hhas_program.t =
  let mutate_function (f : Hhas_function.t) : Hhas_function.t =
    Hhas_function.body f |> mutate_body mut |> Hhas_function.with_body f in
  HP.functions p |> List.map mutate_function |> HP.with_fun p

let mutate_methods (mut: IS.t -> IS.t) (p : HP.t) : Hhas_program.t =
  let mutate_method (m : Hhas_method.t) : Hhas_method.t =
    Hhas_method.body m |> mutate_body mut |> Hhas_method.with_body m in
  let mutate_class (c : Hhas_class.t) : Hhas_class.t =
    Hhas_class.methods c |> List.map mutate_method |>
    Hhas_class.with_methods c in
  HP.classes p |> List.map mutate_class |> HP.with_classes p

let mutate_program (mut: IS.t -> IS.t) (p : HP.t) : Hhas_program.t =
  p |> mutate_main mut |> mutate_functions mut |> mutate_methods mut

(* Given a transformation on instruction sequences, an input program and a
 * number of times to perform the transformation, lifts the transform to work
 * on program monads *)
let mutate (mut: IS.t -> IS.t) (prog : HP.t) : int ->
    (mutation_monad -> mutation_monad) =
  num_fold (fun a -> mutate_program mut prog |> Nondet.add_event a)

(* produces sublist (i, k] of input lst *)
let slice lst i k = Core.List.take (Core.List.drop lst (i + 1)) (k - i)

open Hhbc_ast

let option_lift f opt = Option.map opt f

let should_mutate () = Random.float 1.0 < !mut_prob

let mutate_bool b = if should_mutate() then not b else b

(* adds a random integer in range [-c, c] to n, produces the result if it is
   positive, or 0 otherwise *)
let mutate_int n c =
  if should_mutate () then n + Random.int (2 * (c + 1)) - c |> max 0 else n

let mutate_local_id (id : Local.t) c =
  match id with
  | Local.Unnamed i -> Local.Unnamed (mutate_int i c)
  | _ -> id

(* Changes the input label to another with the same stack height. If none
   exists, produces the original label *)
let mutate_label data (label : Label.t) =
  try
    if not (should_mutate()) then label else
    let height l = data.stack_history |> List.assoc l |> List.length in
    let init_height = height (ILabel label) in
    List.filter (fun l -> init_height = height @@ ILabel l) data.labels |>
    rand_elt
  with
  | Not_found -> label

(* produces a new member key, swapping key types and values, if applicable *)
let mutate_key (k : MemberKey.t) c =
  let new_key_type n =
    [MemberKey.EC n; MemberKey.EL (Local.Unnamed n); MemberKey.PC n;
     MemberKey.EI (Int64.of_int n); MemberKey.PL (Local.Unnamed n)] |>
     rand_elt in
  let swap_key_type k =
    match k with
    | MemberKey.EC n -> if should_mutate() then new_key_type n else k
    | MemberKey.PC n -> if should_mutate() then new_key_type n else k
    | MemberKey.EI n ->
        if should_mutate() then new_key_type (Int64.to_int n)  else k
    | MemberKey.EL (Local.Unnamed n) ->
        if should_mutate() then new_key_type n                 else k
    | MemberKey.PL (Local.Unnamed n) ->
        if should_mutate() then new_key_type n                 else k
    | _ -> k in
  match swap_key_type k with
  | MemberKey.EC i   -> MemberKey.EC (mutate_int      i  c)
  | MemberKey.EL id  -> MemberKey.EL (mutate_local_id id c)
  | MemberKey.PC i   -> MemberKey.PC (mutate_int      i  c)
  | MemberKey.PL id  -> MemberKey.PL (mutate_local_id id c)
  | MemberKey.EI i   ->
      MemberKey.EI (Int64.of_int (mutate_int (Int64.to_int i) c))
  | _ -> k

(* TODO: Given the fact that we explicitly check this in the verifier as of
   D5169792, is this even worth doing here? It will always produce non-verifying
   code*)
let mutate_mode (m : MemberOpMode.t) =
  if should_mutate() then random_mode () else m

(* When given stack data about an instruction sequences, and applied as a
 * wrapper around 'op', this will ensure that the mutations occurring as part
 * of 'op' don't unbalance the evaluation stack. It does this by calculating
 * what values and what flavors each instruction consumes and produces, and
 * either inserting trivial ops to create missing values or Pop* instructions
 * to consume excess values. Providing the trivial op (\x.[x]) and applying
 * this to an instruction will just balance the stack of that sequence, if
 * it was unbalanced *)
let maintain_stack (data : Instr_utils.seq_data) :
                   (Hhbc_ast.instruct -> Hhbc_ast.instruct list) ->
                   (Hhbc_ast.instruct -> Hhbc_ast.instruct list) =
  let pc, stk = ref 0, ref [] in (* This is a cool scoping trick that lets *)
  fun op (i : instruct) ->   (* these refs get defined before the op + instr *)
  pc := !pc + 1;             (* get applied. Thus when this function is *)
  let helper instr buffer =  (* is used later as an argument to *)
    let ctrl_buffer =        (* flat_map, the internal state is not exposed *)
      match instr with
      | IContFlow RetC (* These require a stack height of 1 *)
      | IContFlow RetV   ->
          let old_stk = !stk in
          stk := num_fold List.tl (List.length !stk - 1) !stk;
          empty_stk old_stk 1
      | IContFlow Unwind (* require a stack height of 0 *)
      | IMisc NativeImpl -> let old_stk = !stk in
                            stk := []; empty_stk old_stk 0
      | ILabel _
      | IIterator _
      | IContFlow _ -> (* Stack height needs to match across block boundaries *)
        let orig_stack = List.nth data.stack_history (!pc - 1) |> snd in
        let res = equate_stk !stk orig_stack in
        stk := orig_stack; res
      | _ -> [] in
    let stk_req, stk_prod = stk_data instr in
    let num_req = List.length stk_req - List.length !stk in
    let pre_buffer, stk_extra =
      if num_req > 0 then rebalance_stk num_req stk_req else [],[] in
    if !debug then begin
      Printf.printf "Instruction %s stack is [%s],
                     consumed [%s], produced [%s]\n"
      (Hhbc_hhas.string_of_instruction instr) (string_of_stack !stk)
      (string_of_stack stk_req) (string_of_stack stk_prod) end;
    (* Append any dummy values inserted to the front of the stack, then
     * remove all values consumed by the instruction, and append all values
     * produced *)
    stk := stk_extra @ !stk;
    stk := List.fold_left (fun acc _ -> List.tl acc) !stk stk_req;
    stk := List.fold_left (fun acc x -> x :: acc)    !stk stk_prod;
    (* add any necessary buffer instructions in front of the original *)
    buffer @ pre_buffer @ ctrl_buffer @ [instr] in
  List.fold_right helper (op i) []

(* Mutate immediates according to their types *)
let mut_imms (is : IS.t) : IS.t =
  let mutate_get s =
    match s with    (*TODO: clean this up if OCaml ever gets macros *)
    | CGetL       id     -> CGetL      (mutate_local_id id !mag)
    | CGetQuietL  id     -> CGetQuietL (mutate_local_id id !mag)
    | CGetL2      id     -> CGetL2     (mutate_local_id id !mag)
    | CGetL3      id     -> CGetL3     (mutate_local_id id !mag)
    | PushL       id     -> PushL      (mutate_local_id id !mag)
    | CGetS       i      -> CGetS      (mutate_int      i  !mag)
    | VGetL       id     -> VGetL      (mutate_local_id id !mag)
    | VGetS       i      -> VGetS      (mutate_int      i  !mag)
    | ClsRefGetC  i      -> ClsRefGetC (mutate_int      i  !mag)
    | ClsRefGetL (id, i) ->
        ClsRefGetL ((mutate_local_id id !mag), (mutate_int i !mag))
    | _ -> s in  (*TODO: in general it might be worthwhile to get rid of wild
                   card cases like this. It would make the code more verbose,
                   but it will make adding new bytecodes easier since this will
                   just stop compiling whenever that happens *)
  let mutate_isset s =
    match s with
    | IssetL   id        -> IssetL  (mutate_local_id id !mag)
    | IssetS   i         -> IssetS  (mutate_int      i  !mag)
    | EmptyL   id        -> EmptyL  (mutate_local_id id !mag)
    | EmptyS   i         -> EmptyS  (mutate_int      i  !mag)
    | IsTypeL (id, op)   -> IsTypeL (mutate_local_id id !mag, op)
    | _ -> s in
  let mutate_mutator s =
    let mutate_eq_op op = if should_mutate () then random_eq_op () else op in
    let mutate_inc_op op =
      if should_mutate () then random_incdec_op () else op in
    match s with
    | SetL      id       -> SetL    (mutate_local_id id !mag)
    | SetS      i        -> SetS    (mutate_int      i  !mag)
    | SetOpL   (id, op)  -> SetOpL  (mutate_local_id id !mag, mutate_eq_op op)
    | SetOpN    op       -> SetOpN  (mutate_eq_op op)
    | SetOpG    op       -> SetOpG  (mutate_eq_op op)
    | SetOpS   (op, i )  -> SetOpS  (mutate_eq_op op,         mutate_int i !mag)
    | IncDecL  (id, op)  -> IncDecL (mutate_local_id id !mag, mutate_inc_op op)
    | IncDecN   op       -> IncDecN (mutate_inc_op op)
    | IncDecG   op       -> IncDecG (mutate_inc_op op)
    | IncDecS  (op, i )  -> IncDecS (mutate_inc_op op,        mutate_int i !mag)
    | BindL     id       -> BindL   (mutate_local_id id !mag)
    | BindS     i        -> BindS   (mutate_int      i  !mag)
    | UnsetL    id       -> UnsetL  (mutate_local_id id !mag)
    | CheckProp p        -> CheckProp p
    | InitProp (p, Static) ->
        InitProp(p, if should_mutate() then NonStatic else Static)
    | InitProp (p, NonStatic) ->
        InitProp(p, if should_mutate() then Static else NonStatic)
    | _ -> s in
  let mutate_call data s =
    match s with (*It's not worth mutating arg numbers for Push* or Call*,
                   because we already know it will fail the verifier/assembler*)
    | FPushObjMethod   (i, Ast_defs.OG_nullthrows)    ->
         FPushObjMethod(i,    if should_mutate()
                              then Ast_defs.OG_nullsafe
                              else Ast_defs.OG_nullthrows)
    | FPushObjMethod   (i, Ast_defs.OG_nullsafe)      ->
         FPushObjMethod(i,    if should_mutate()
                              then Ast_defs.OG_nullthrows
                              else Ast_defs.OG_nullsafe)
    | FPushObjMethodD  (i, m, Ast_defs.OG_nullthrows) ->
        FPushObjMethodD(i, m, if should_mutate()
                              then Ast_defs.OG_nullsafe
                              else Ast_defs.OG_nullthrows)
    | FPushObjMethodD  (i, m, Ast_defs.OG_nullsafe)   ->
        FPushObjMethodD(i, m, if should_mutate()
                              then Ast_defs.OG_nullthrows
                              else Ast_defs.OG_nullsafe)
    | FPushCtor     (i, id) -> FPushCtor  (i, mutate_int        id !mag)
    | FPushCtorI    (i, id) -> FPushCtorI (i, mutate_int        id !mag)
    | DecodeCufIter (i, id) -> DecodeCufIter (mutate_int        i  !mag,
                                              mutate_label data id)
    | FPassS        (i, id) -> FPassS        (mutate_int        i  !mag,
                                              mutate_int        id !mag)
    | FPassL        (i, id) -> FPassL        (mutate_int        i  !mag,
                                              mutate_local_id   id !mag)
    | FPassC         i      -> FPassC        (mutate_int        i  !mag)
    | FPassCW        i      -> FPassCW       (mutate_int        i  !mag)
    | FPassCE        i      -> FPassCE       (mutate_int        i  !mag)
    | FPassV         i      -> FPassV        (mutate_int        i  !mag)
    | FPassVNop      i      -> FPassVNop     (mutate_int        i  !mag)
    | FPassR         i      -> FPassR        (mutate_int        i  !mag)
    | FPassN         i      -> FPassN        (mutate_int        i  !mag)
    | FPassG         i      -> FPassG        (mutate_int        i  !mag)
    | _ -> s in
  let mutate_base s =
    match s with
    | BaseNC    (idx, mode) -> BaseNC (mutate_int      idx !mag,
                                       mutate_mode         mode)
    | BaseNL    (id,  mode) -> BaseNL (mutate_local_id id  !mag,
                                       mutate_mode         mode)
    | BaseGC    (idx, mode) -> BaseGC (mutate_int      idx !mag,
                                       mutate_mode         mode)
    | BaseGL    (id,  mode) -> BaseGL (mutate_local_id id  !mag,
                                       mutate_mode         mode)
    | BaseL     (id,  mode) -> BaseL  (mutate_local_id id  !mag,
                                       mutate_mode         mode)
    | FPassBaseNC (i,  idx) -> FPassBaseNC (i, mutate_int      idx !mag)
    | FPassBaseNL (i,   id) -> FPassBaseNL (i, mutate_local_id id  !mag)
    | FPassBaseGC (i,  idx) -> FPassBaseGC (i, mutate_int      idx !mag)
    | FPassBaseGL (i,   id) -> FPassBaseGL (i, mutate_local_id id  !mag)
    | BaseSC      (idx,  i) -> BaseSC         (mutate_int      idx !mag,
                                               mutate_int      i   !mag)
    | BaseSL      (id, idx) -> BaseSL         (mutate_local_id id  !mag,
                                               mutate_int      idx !mag)
    | FPassBaseL  (i,   id) -> FPassBaseL  (i, mutate_local_id id  !mag)
    | BaseC        idx      -> BaseC    (mutate_int idx !mag)
    | BaseR        idx      -> BaseR    (mutate_int idx !mag)
    | Dim       (mode, key) -> Dim      (mutate_mode  mode, mutate_key key !mag)
    | FPassDim    (i,  key) -> FPassDim (mutate_int i !mag, mutate_key key !mag)
    | _ -> s in
  let mutate_ctrl_flow data s =
    match s with
    | Switch (Bounded, n, v) ->
        Switch ((if should_mutate() then Unbounded else Bounded),
        mutate_int n !mag, v)
    | Switch (Unbounded, n, v) ->
        Switch ((if should_mutate() then Bounded else Unbounded),
        mutate_int n !mag, v)
    | Jmp   l -> Jmp   (mutate_label data l)
    | JmpNS l -> JmpNS (mutate_label data l)
    | JmpZ  l -> JmpZ  (mutate_label data l)
    | JmpNZ l -> JmpNZ (mutate_label data l)
    | SSwitch lst ->
        SSwitch (List.map (fun (id, l) -> (id, mutate_label data l)) lst)
    | _ -> s in
  let mutate_final s =
    let mutate_op op = if should_mutate () then random_query_op () else op in
    match s with
    | QueryM  (i, op, k) ->
        QueryM (mutate_int i !mag,   mutate_op      op,      mutate_key k !mag)
    | FPassM  (i, i', k) ->
        FPassM (mutate_int i !mag,   mutate_int i' !mag,     mutate_key k !mag)
    | VGetM   (i,     k) -> VGetM   (mutate_int i  !mag,     mutate_key k !mag)
    | SetM    (i,     k) -> SetM    (mutate_int i  !mag,     mutate_key k !mag)
    | IncDecM (i, op, k) -> IncDecM (mutate_int i  !mag, op, mutate_key k !mag)
    | SetOpM  (i, op, k) -> SetOpM  (mutate_int i  !mag, op, mutate_key k !mag)
    | BindM   (i,     k) -> BindM   (mutate_int i  !mag,     mutate_key k !mag)
    | UnsetM  (i,     k) -> UnsetM  (mutate_int i  !mag,     mutate_key k !mag)
    | SetWithRefLML (id, id') ->
        SetWithRefLML (mutate_local_id id !mag, mutate_local_id id' !mag)
    | SetWithRefRML  id  -> SetWithRefRML (mutate_local_id id !mag) in
  let mutate_iterator data s =
    match s with
    | IterInit   (i, l, id)      ->
        IterInit   (i, mutate_label data l,      mutate_local_id id  !mag)
    | IterInitK  (i, l, id, id') ->
        IterInitK  (i, mutate_label data l,
                       mutate_local_id  id !mag, mutate_local_id id' !mag)
    | WIterInit  (i, l, id)      ->
        WIterInit  (i, mutate_label data l,      mutate_local_id id  !mag)
    | WIterInitK (i, l, id, id') ->
        WIterInitK (i, mutate_label data l,
                       mutate_local_id  id !mag, mutate_local_id id' !mag)
    | MIterInit  (i, l, id)      ->
        MIterInit  (i, mutate_label data l,      mutate_local_id id  !mag)
    | MIterInitK (i, l, id, id') ->
        MIterInitK (i, mutate_label data l,
                       mutate_local_id  id !mag, mutate_local_id id' !mag)
    | IterNext   (i, l, id)      ->
        IterNext   (i, mutate_label data l,      mutate_local_id id  !mag)
    | IterNextK  (i, l, id, id') ->
        IterNextK  (i, mutate_label data l,
                       mutate_local_id  id !mag, mutate_local_id id' !mag)
    | WIterNext  (i, l, id)      ->
        WIterNext  (i, mutate_label data l,      mutate_local_id id  !mag)
    | WIterNextK (i, l, id, id') ->
        WIterNextK (i, mutate_label data l,
                       mutate_local_id  id !mag, mutate_local_id id' !mag)
    | MIterNext  (i, l, id)      ->
        MIterNext  (i, mutate_label data l,      mutate_local_id id  !mag)
    | MIterNextK (i, l, id, id') ->
        MIterNextK (i, mutate_label data l,
                       mutate_local_id  id !mag, mutate_local_id id' !mag)
    | IterBreak  (l, lst)        ->
        IterBreak     (mutate_label data l,
                       List.map (fun (b, i) -> (mutate_bool b, i)) lst)
    | _ -> s in
  let mutate_misc s =
    let mutate_bare op = if should_mutate() then random_bare_op    () else op in
    let mutate_kind k =  if should_mutate() then random_class_kind () else k  in
    let mutate_param_id id c =
      match id with
      | Param_unnamed i -> Param_unnamed (mutate_int i c)
      | _ -> id in (*TODO: is it worth it to mutate named params? I think this
                     would create trivially non-verifying programs in almost all
                     cases, so it wouldn't be that interesting *)
     let mutate_silence op =
        if should_mutate() then random_silence () else op in
    match s with
    | BareThis        b       -> BareThis        (mutate_bare            b)
    | InitThisLoc    id       -> InitThisLoc     (mutate_local_id id  !mag)
    | StaticLocCheck (id, str) -> StaticLocCheck (mutate_local_id id  !mag, str)
    | StaticLocInit  (id, str) -> StaticLocInit  (mutate_local_id id  !mag, str)
    | StaticLocDef  (id, str)  -> StaticLocDef   (mutate_local_id id  !mag, str)
    | OODeclExists    k       -> OODeclExists    (mutate_kind            k)
    | VerifyParamType p       -> VerifyParamType (mutate_param_id p   !mag)
    | Self            i       -> Self            (mutate_int      i   !mag)
    | Parent          i       -> Parent          (mutate_int      i   !mag)
    | LateBoundCls    i       -> LateBoundCls    (mutate_int      i   !mag)
    | ClsRefName      i       -> ClsRefName      (mutate_int      i   !mag)
    | IncStat        (i,  i') -> IncStat         (mutate_int      i   !mag,
                                                  mutate_int      i'  !mag)
    | CreateCl       (i,  i') -> CreateCl        (mutate_int      i   !mag,
                                                  mutate_int      i'  !mag)
    | GetMemoKeyL    id       -> GetMemoKeyL     (mutate_local_id id !mag)
    | Silence (id, op) -> Silence (mutate_local_id id !mag, mutate_silence op)
    | MemoSet     (i, id, i') ->
        MemoSet (mutate_int i !mag, mutate_local_id id !mag, mutate_int i' !mag)
    | MemoGet     (i, id, i') ->
        MemoGet (mutate_int i !mag, mutate_local_id id !mag, mutate_int i' !mag)
    | _ -> s in
  let change_imms data (i : instruct) : instruct list =
    [match i with
     | IContFlow s -> IContFlow (mutate_ctrl_flow data s)
     | IMutator  s -> IMutator  (mutate_mutator        s)
     | IGet      s -> IGet      (mutate_get            s)
     | IIsset    s -> IIsset    (mutate_isset          s)
     | ICall     s -> ICall     (mutate_call      data s)
     | IBase     s -> IBase     (mutate_base           s)
     | IFinal    s -> IFinal    (mutate_final          s)
     | IIterator s -> IIterator (mutate_iterator  data s)
     | IMisc     s -> IMisc     (mutate_misc           s)
     | _ -> i] (*no immediates, or none worth mutating*) in
  let data = seq_data is in
  IS.InstrSeq.flat_map is (change_imms data |> maintain_stack data)

(* Mutates the immediates of the opcodes in the input program. Currently
   this is just naive, randomly changing integer values by a user specified
   magnitude, and changing enum-style values by choosing a random new value
   in the possiblity space. *)
let mutate_immediate (input : HP.t) : mutation_monad =
  debug_print "Mutating immediates";
  Nondet.return input |> mutate mut_imms input !imm_reps

(* This will randomly duplicate an instruction by inserting another copy into
   its instruction sequences. *)
let mutate_duplicate (input : HP.t) : mutation_monad =
  debug_print "Duplicating";
  let mut (is : IS.t) =
    let duplicate i =
      match i with
      | ILabel _
      | ICall _
      | ITry _
      | IContFlow _ -> [i] (*duplicating control flow just breaks the stack*)
      | _ -> if should_mutate() then [i; i] else [i] in
    IS.InstrSeq.flat_map is (maintain_stack (seq_data is) duplicate) in
  Nondet.return input |> mutate mut input !dup_reps

(* This will randomly swap the position of instruction subsequences with
   the same stack signatures. *)
let mutate_reorder (input : HP.t) : mutation_monad =
  debug_print "Reordering";
  let with_height_after map h idx : int list =
    if Hashtbl.mem map h
    then Hashtbl.find map h |> List.filter ((<) idx)
    else [] in
  let mut (is : IS.t) =
    if not (should_mutate()) then is else
    let instrs = IS.instr_seq_to_list is in
    if List.length instrs < 5 then is else
    let subinstrs = slice instrs in
    let heights, by_height = stack_history is |> height_map in
    (*finds (start1, end1], (start2, end2] indices of same stack signature *)
    let rec find_sequence lim runs =
      if runs > lim then -1, -1, -1, -1 else begin
        let next_pos h idx = with_height_after by_height h idx |> rand_elt in
        try
          let start_h = rand_elt heights in
          let end_h   = rand_elt heights in
          let start1  = Hashtbl.find by_height start_h |> rand_elt in
          let end1    = next_pos end_h start1 in
          let start2  = next_pos start_h end1 in
          start1, end1, start2, next_pos end_h start2
        with Invalid_argument _ -> find_sequence lim (runs + 1) end in
    let start1, end1, start2, end2 = find_sequence 10 0 in
    if start1 < 0 then is else
    let newseq = subinstrs (-1) start1 @ subinstrs start2 end2 @
                 subinstrs end1 start2 @ subinstrs start1 end1 @
                 subinstrs end2 (List.length instrs - 1) in
    (* Reattach fault regions that were discarded by converting to list *)
    IS.gather [IS.InstrSeq.flat_map (IS.Instr_list newseq)
               (maintain_stack (seq_data is) Core.List.return);
               IS.extract_fault_instructions is] in
  Nondet.return input |> mutate mut input !reorder_reps

(* This will randomly remove some of the instructions in a sequence. *)
let mutate_remove (input : HP.t) : mutation_monad =
  debug_print "Removing";
  let remove i =
    match i with
    | ILabel _
    | ICall _
    | ITry _
    | IContFlow _ -> [i] (*removing control flow just breaks the stack*)
    | _ -> if should_mutate() then [] else [i] in
  let mut (is : IS.t) =
    IS.InstrSeq.flat_map is (maintain_stack (seq_data is) remove) in
  Nondet.return input |> mutate mut input !remove_reps

(* This will randomly wrap instruction sequences in try/fault or try/catch
   blocks with trivial handlers. *)
let mutate_exceptions (input : HP.t) : mutation_monad =
  let rec new_exn_region (f : IS.t -> IS.t * IS.t) (is : IS.t) : IS.t =
    let primary = IS.instr_seq_to_list is in
    let primary_len = List.length primary in
    let subinstrs = slice primary in
    if primary_len <= 1 then is
    else try
           let start = Random.int (primary_len - 1) - 1 in
           let end_pos = Random.int (primary_len - start - 1) + start + 1 in
           let body, faults = subinstrs start end_pos |> IS.instrs |> f in
           IS.gather [subinstrs (-1) start |> IS.instrs; body;
                      subinstrs end_pos (primary_len - 1) |> IS.instrs;
                      IS.extract_fault_instructions is; faults]
         with Invalid_argument _ -> new_exn_region f is in
  let make_fault (label : Label.t) (body : IS.t) : IS.t * IS.t =
    let open IS in gather [ITry (TryFaultBegin label) |> instr; body;
                           ITry TryFaultEnd |> instr],
                   gather [ILabel label |> instr; instr_unwind] in
  let make_catch (label : Label.t) (body : IS.t) : IS.t * IS.t =
    let open IS in gather [instr_try_catch_begin; body; instr_jmp label;
                           instr_try_catch_middle; instr_throw;
                           instr_try_catch_end; instr_label label], IS.empty in
  let resume_label () =
    Label.get_next_label () |> string_of_int |> (^) "resume" |> Label.named in
  let new_catch = resume_label ()       |> make_catch |> new_exn_region in
  let new_fault = random_fault_label () |> make_fault |> new_exn_region in
  let mut (is : IS.t) =
    if not (should_mutate()) then is
    else if Random.int 2 < 1
      then new_catch is
      else new_fault is in
  Nondet.return input |> mutate mut input !exn_reps

(* This will replace random instructions with others of the
   same stack signature *)
let mutate_replace (input : HP.t) : mutation_monad =
  debug_print "Replacing";
  let mut (is : IS.t) =
    let sig_base, sig_final, sig_fpass, sig_all =
      sig_map_base is, sig_map_final is, sig_map_fpass is, sig_map_all is in
    let equiv_instr (i : instruct) : instruct =
      let equiv_map = match i with
                      | IBase _ -> sig_base
                      | IFinal _ -> sig_final
                      | ICall _ -> sig_fpass
                      | _ -> sig_all in
      match i with
      | IContFlow _
      | ITry _ -> i
      | _ -> try (List.assoc (stk_data i) equiv_map |> rand_elt) ()
             with | Not_found | Invalid_argument _ -> i in
    let replace i = if should_mutate() then [equiv_instr i] else [i] in
    IS.InstrSeq.flat_map is (maintain_stack (seq_data is) replace) in
  Nondet.return input |> mutate mut input !replace_reps

(* This will add random instructions, with arbitrary stack signatures, to the
   input program, rebalancing the stack as it goes *)
let mutate_insert (input : HP.t) : mutation_monad =
  debug_print "Inserting";
  let mut (is : IS.t) =
    let new_instr : unit -> instruct =
      let instrs = all_instrs is in
      fun () -> (instrs |> rand_elt) () in
    let insert i = match i with
                   | ITry _ -> []
                   | _ -> if should_mutate() then [new_instr (); i] else [i] in
    IS.InstrSeq.flat_map is (maintain_stack (seq_data is) insert) in
  Nondet.return input |> mutate mut input !insert_reps

(* This will charge random aspects of the input program metadata *)
let mutate_metadata (input : HP.t)  =
  debug_print "Mutating metadata";
  let delete_map f l =
    Core.List.filter_map l
      ~f:(fun x -> if should_mutate() then None else Some (f x)) in
  let mutate_option opt =
    if should_mutate () then None else opt in
  let mutate_typed_value (v : Typed_value.t) : Typed_value.t =
    if should_mutate () then random_typed_value () else v in
  let module HTC = Hhas_type_constraint in
  let mutate_flag (f : HTC.type_constraint_flag) : HTC.type_constraint_flag =
    if should_mutate () then random_flag () else f in
  let mutate_constraint (const : HTC.t) : HTC.t =
    HTC.make
      (const |> HTC.name  |> mutate_option)
      (const |> HTC.flags |> delete_map mutate_flag) in
  let mutate_adata_id (id : adata_id) : adata_id =
    let pre = "A_" in
    let num = String.length pre |> Str.string_after id |> int_of_string in
    pre ^ (mutate_int num !mag |> string_of_int) in
  let mutate_type_info (info : Hhas_type_info.t) : Hhas_type_info.t =
    Hhas_type_info.make
      (info |> Hhas_type_info.user_type       |> mutate_option)
      (info |> Hhas_type_info.type_constraint |> mutate_constraint) in
  let mutate_attribute (att : Hhas_attribute.t) : Hhas_attribute.t =
    Hhas_attribute.make
      (att |> Hhas_attribute.name)
      (att |> Hhas_attribute.arguments |> delete_map mutate_typed_value) in
  let mutate_param (param : Hhas_param.t) : Hhas_param.t =
    Hhas_param.make
      (param |> Hhas_param.name)
      (param |> Hhas_param.is_reference |> mutate_bool)
      (param |> Hhas_param.is_variadic  |> mutate_bool)
      (param |> Hhas_param.type_info    |> option_lift mutate_type_info)
      (param |> Hhas_param.default_value) in
  let mutate_body_data (body : Hhas_body.t) : Hhas_body.t =
    let mutate_static_init s = s in
    Hhas_body.make
      (body |> Hhas_body.instrs)
      (body |> Hhas_body.decl_vars)
      (body |> Hhas_body.num_iters)
      (body |> Hhas_body.num_cls_ref_slots  |> fun n -> mutate_int n !mag)
      (body |> Hhas_body.is_memoize_wrapper |> mutate_bool)
      (body |> Hhas_body.params             |> delete_map mutate_param)
      (body |> Hhas_body.return_type        |> option_lift mutate_type_info)
      (body |> Hhas_body.static_inits       |> delete_map mutate_static_init)
      (body |> Hhas_body.doc_comment) in
  let mutate_class_data (ids : Hhbc_id.Class.t list) (cls : Hhas_class.t) =
    let module HC = Hhas_class in
    let mutate_cls_id (id : Hhbc_id.Class.t) : Hhbc_id.Class.t =
       if should_mutate () then rand_elt ids else id in
    let mutate_method_data (m : Hhas_method.t) : Hhas_method.t =
      Hhas_method.make
        (m |> Hhas_method.attributes        |> delete_map mutate_attribute)
        (m |> Hhas_method.is_protected      |> mutate_bool)
        (m |> Hhas_method.is_public         |> mutate_bool)
        (m |> Hhas_method.is_private        |> mutate_bool)
        (m |> Hhas_method.is_static         |> mutate_bool)
        (m |> Hhas_method.is_final          |> mutate_bool)
        (m |> Hhas_method.is_abstract       |> mutate_bool)
        (m |> Hhas_method.no_injection      |> mutate_bool)
        (m |> Hhas_method.name)
        (m |> Hhas_method.body              |> mutate_body_data)
        (m |> Hhas_method.span)
        (m |> Hhas_method.is_async          |> mutate_bool)
        (m |> Hhas_method.is_generator      |> mutate_bool)
        (m |> Hhas_method.is_pair_generator |> mutate_bool)
        (m |> Hhas_method.is_closure_body   |> mutate_bool) in
    let mutate_property (prop : Hhas_property.t) : Hhas_property.t =
      Hhas_property.make
        (prop |> Hhas_property.is_private         |> mutate_bool)
        (prop |> Hhas_property.is_protected       |> mutate_bool)
        (prop |> Hhas_property.is_public          |> mutate_bool)
        (prop |> Hhas_property.is_static          |> mutate_bool)
        (prop |> Hhas_property.is_deep_init       |> mutate_bool)
        (prop |> Hhas_property.no_serialize       |> mutate_bool)
        (prop |> Hhas_property.name)
        (prop |> Hhas_property.initial_value |> option_lift mutate_typed_value)
        (prop |> Hhas_property.initializer_instrs |> mutate_option)
        (prop |> Hhas_property.type_info          |> mutate_type_info) in
    let mutate_constant (const : Hhas_constant.t) : Hhas_constant.t =
      Hhas_constant.make
        (const |> Hhas_constant.name)
        (const |> Hhas_constant.value |> option_lift mutate_typed_value)
        (const |> Hhas_constant.initializer_instrs |> mutate_option) in
    let mutate_typed_constant (const : Hhas_type_constant.t) =
      Hhas_type_constant.make
        (const |> Hhas_type_constant.name)
        (const |> Hhas_type_constant.initializer_t
                                          |> option_lift mutate_typed_value) in
    let mutate_req ((trait, str) as pair) =
      if should_mutate() then match trait with
                              | Ast.MustExtend -> Ast.MustImplement, str
                              | Ast.MustImplement -> Ast.MustExtend, str
                         else pair in
    HC.make
      (cls |> HC.attributes         |> delete_map mutate_attribute)
      (cls |> HC.base               |> option_lift mutate_cls_id)
      (cls |> HC.implements         |> delete_map mutate_cls_id)
      (cls |> HC.name)
      (cls |> HC.span)
      (cls |> HC.is_final           |> mutate_bool)
      (cls |> HC.is_abstract        |> mutate_bool)
      (cls |> HC.is_interface       |> mutate_bool)
      (cls |> HC.is_trait           |> mutate_bool)
      (cls |> HC.is_xhp             |> mutate_bool)
      (cls |> HC.is_top             |> mutate_bool)
      (cls |> HC.class_uses)
      (cls |> HC.class_use_aliases)
      (cls |> HC.class_use_precedences)
      (cls |> HC.enum_type          |> option_lift mutate_type_info)
      (cls |> HC.methods            |> delete_map mutate_method_data)
      (cls |> HC.properties         |> delete_map mutate_property)
      (cls |> HC.constants          |> delete_map mutate_constant)
      (cls |> HC.type_constants     |> delete_map mutate_typed_constant)
      (cls |> HC.requirements       |> delete_map mutate_req)
      (cls |> HC.doc_comment) in
  let mutate_fun_data (f : Hhas_function.t) : Hhas_function.t =
    Hhas_function.make
      (f |> Hhas_function.attributes        |> delete_map mutate_attribute)
      (f |> Hhas_function.name)
      (f |> Hhas_function.body              |> mutate_body_data)
      (f |> Hhas_function.span)
      (f |> Hhas_function.is_async          |> mutate_bool)
      (f |> Hhas_function.is_generator      |> mutate_bool)
      (f |> Hhas_function.is_pair_generator |> mutate_bool)
      (f |> Hhas_function.is_top            |> mutate_bool) in
  let mutate_typedef (typedef : Hhas_typedef.t) : Hhas_typedef.t =
    Hhas_typedef.make
      (typedef |> Hhas_typedef.name)
      (typedef |> Hhas_typedef.type_info      |> mutate_type_info)
      (typedef |> Hhas_typedef.type_structure |>
                                            option_lift mutate_typed_value) in
  let mutate_adata (data : Hhas_adata.t) : Hhas_adata.t =
    Hhas_adata.make
      (data |> Hhas_adata.id    |> mutate_adata_id)
      (data |> Hhas_adata.value |> mutate_typed_value) in
  let mut_data (prog : HP.t) : HP.t =
    let ids = prog |> HP.classes |> delete_map Hhas_class.name in
    HP.make
      (prog |> HP.adata     |> delete_map mutate_adata)
      (prog |> HP.functions |> delete_map mutate_fun_data)
      (prog |> HP.classes   |> delete_map (mutate_class_data ids))
      (prog |> HP.typedefs  |> delete_map mutate_typedef)
      (prog |> HP.main      |> mutate_body_data) in
  let open Nondet in
  return input |> num_fold
    (fun a -> mut_data input |> add_event a) !metadata_reps

(*---------------------------------Execution----------------------------------*)

open Nondet

let fuzz (input : HP.t) (mutations : mutation list): unit =
  mutations |> List.fold_left (>>=) (return input) >>| print_output |> ignore

(* command line driver *)
let _ =
  Random.self_init ();
  if !Sys.interactive then ()
  else
    set_binary_mode_out stdout true;
    let input = read_input () in
    let mutations = [mutate_immediate;  mutate_duplicate; mutate_reorder;
                     mutate_remove;     mutate_replace;   mutate_insert;
                     mutate_exceptions; mutate_metadata] in
    if !complete_reps > 0
    then (imm_reps := 1; dup_reps := 1; reorder_reps := 1; remove_reps := 1;
         replace_reps := 1; insert_reps := 1; exn_reps := 1; metadata_reps := 1)
    else complete_reps := 1;
    for n = 1 to !complete_reps do
      fuzz input mutations
    done
