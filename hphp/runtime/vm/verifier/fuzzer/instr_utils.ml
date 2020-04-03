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

open Hhbc_ast
module IS = Instruction_sequence
type stack = string list
type stack_sig = stack * stack

(* Performs f n times, the input to f at each state being the output of the
 * previous. Basically Church numerals:
 * num_fold f 0 acc = acc
 * num_fold f 1 acc = f acc
 * num_fold f 2 acc = f (f acc)
 * ... *)
let rec num_fold f n acc =
  if n <= 0 then acc else num_fold f (n - 1) (f acc)

(* Produces a list of operations necessary to produce the top 'n' flavors of
 * 'req' *)
let rec rebalance_stk n (req : stack) : instruct list * stack =
  if n = 0 then [], [] else
  if List.length req < 0 then failwith "cannot rebalance empty stack" else
  match List.hd req, List.tl req |> rebalance_stk (n - 1) with
  | "C", (buf, extra) -> ILitConst (Int (Int64.of_int 1)) :: buf, "C" :: extra
  | "V", _ -> failwith "not supported, V flavor being removed"
  | "U", (buf, extra) -> ILitConst NullUninit :: buf, "U" :: extra
  | _ -> [], [] (* Impossible *)

(* Produces a list of operations necessary to reduce the height of the input
 * stack to the value of 'remaining' *)
let rec empty_stk (stk : stack) (remaining : int) : instruct list =
  if List.length stk <= remaining then [] else
  match stk with
  | [] -> []
  | "C" :: t -> IBasic PopC :: empty_stk t remaining
  | "U" :: t -> IBasic PopU :: empty_stk t remaining
  | _   :: t -> remaining - 1 |> empty_stk t

(* Outputs a sequence of dummy instructions to make stk equal to target *)
let equate_stk (stk : stack) (target : stack) : instruct list =
  let stklen, targetlen = List.length stk, List.length target in
  if stklen = targetlen then []
  else if stklen > targetlen then empty_stk stk targetlen
  else rebalance_stk (targetlen - stklen) target |> fst

(* produces a list of a particular stack flavor of length 'n' *)
let produce flavor n = num_fold (List.cons flavor) n []

let string_of_stack (stk : stack) : string =
  List.fold_right (fun x acc -> x ^ "; " ^ acc) stk ""

(* Determines how an instruction changes the stack, and how many
 * cells it consumes. Return format is (required, produced).
 * TODO(T20108993): autogenerate this from the bytecode spec. *)
let stk_data : instruct -> stack_sig = function
  | IMutator UnsetL _
  | IIncludeEvalDefine DefClsNop _
  | IIncludeEvalDefine DefCls _
  | IIncludeEvalDefine DefTypeAlias _
  | IGenerator ContCheck _                 -> [], []
  | IOp Fatal _
  | IContFlow JmpZ _
  | IContFlow JmpNZ _
  | IContFlow Switch _
  | IContFlow SSwitch _
  | IContFlow RetC
  | IContFlow Throw
  | IMutator UnsetG
  | IMutator InitProp _
  | IIterator IterInit _
  | IMisc CheckReifiedGenericMismatch
  | IBasic PopC                            -> ["C"], []
  | IBasic PopU                            -> ["U"], []
  | IGet CGetL2 _
  | IBasic Dup                             -> ["C"], ["C"; "C"]
  | IMisc CGetCUNop                        -> ["U"], ["C"]
  | IMisc UGetCUNop                        -> ["C"], ["U"]
  | ILitConst NullUninit                   -> [], ["U"]
  | ILitConst NewVecArray n
  | ILitConst NewKeysetArray n
  (*| IOp ConcatN n *)
  | IFinal QueryM (n, _, _)
  | IFinal IncDecM (n, _, _)
  | IMisc CreateCl (n, _)
  | ILitConst NewPackedArray n             -> produce "C" n, ["C"]
  | IFinal SetOpM (n, _, _)
  | IFinal SetM (n, _)                     -> produce "C" (n + 1), ["C"]
  | IFinal UnsetM (n, _)                   -> produce "C" n, []
  | ILitConst NewStructArray v             -> produce "C" (List.length v), ["C"]
  | IMisc Idx
  | IMisc ArrayIdx
  | ILitConst AddElemC                     -> ["C"; "C"; "C"], ["C"]
  | IGet CGetL _
  | IGet PushL _
  | IGet CUGetL _
  | IIsset IssetL _
  | IIsset IsUnsetL _
  | IIsset IsTypeL _
  | IMutator IncDecL _
  | IMutator CheckProp _
  | IMisc This
  | IMisc BareThis _
  | IMisc GetMemoKeyL _
  | IGenerator CreateCont
  | IGenerator ContValid
  | IGenerator ContKey
  | IGenerator ContGetReturn
  | ICall NewObjD _
  | ICall NewObjRD _
  | IGet CGetQuietL _                      -> [], ["C"]
  | IMutator SetG
  | IMutator SetOpG _
  | IMutator SetOpS _
  | IMisc OODeclExists _
  | IMisc AKExists
  | IGenerator YieldK                      -> ["C"; "C"], ["C"]
  | IOp Abs
  | IOp Not
  | IOp Floor
  | IOp Ceil
  | IOp Sqrt
  | IOp CastBool
  | IOp CastInt
  | IOp CastDouble
  | IOp CastString
  | IOp CastArray
  | IOp CastVec
  | IOp CastDict
  | IOp CastKeyset
  | IOp InstanceOfD _
  | IOp IsTypeStructC _
  | IOp Print
  | IOp Clone
  | IOp BitNot
  | IOp Hhbc_ast.Exit
  | IBase BaseSC _
  | IGet _
  | IIsset _
  | IMutator _
  | IIncludeEvalDefine _
  | IMisc VerifyRetTypeC
  | IMisc VerifyRetTypeTS
  | IGenerator _
  | IAsync _
  | IMisc RecordReifiedGeneric
  | ILitConst ColFromArray _               -> ["C"], ["C"]
  | IOp CombineAndResolveTypeStruct n      -> produce "C" n, ["C"]
  | ILitConst NewPair
  | IOp _
  | ILitConst AddNewElemC                  -> ["C"; "C"], ["C"]
  | ICall FCall ((f, n, r, _, _, _))       ->
    produce "C" (n + (if f.has_unpack then 1 else 0)),
    produce "C" r
  | ICall FCallBuiltin (n, _, _, _)        -> produce "C" n, ["C"]
  | ILitConst _                            -> [], ["C"]
  | ICall _                                -> ["C"], []
  | _ -> [], []

type seq_data = {labels : Label.t list;
                 stack_history : (instruct * stack) list; }

(* produces the sequence of stacks corresponding to the input instruction
   sequence. The nth value in the result is a pair of the nth instruction in the
   input and the stack after that instruction. A Nop is placed at index 0 with
   an empty stack. This way one can access the stack before a instruction n
   safely by indexing into position n-1 in the list.

   Input precondition: the input sequence would assemble in HHVM, meaning
   the stack depth never goes negative, and stack height is the same across
   control flow boundaries *)
let stack_history (seq : IS.t) : (instruct * stack) list =
  let f hist i =
    let stk_req, stk_prod = stk_data i in
    match hist with
    | [] -> [i, stk_prod]
    | (_, h) :: _ ->
      let removed = List.fold_left (fun acc _ -> List.tl acc)    h stk_req  in
      let added   = List.fold_left (fun acc x -> x :: acc) removed stk_prod in
      (i, added) :: hist in
  (IBasic Nop, []) :: IS.InstrSeq.fold_left seq ~f:f ~init:[] |> List.rev

(* Produces a map from stack height to indices in the instruction sequence
   with that height, as well as a list of all the heights in the table. This
   map doesn't include the last instruction, since swapping the position of a
   Ret* instruction is not desirable. *)
let height_map (lst : (instruct * stack) list) :
    int list * (int, int list) Hashtbl.t =
  if List.length lst < 0 then failwith "cannot get history of empty sequence";
  let hist = List.tl lst in (*remove the nop beginning of default hists*)
  let tbl = List.length hist |> Hashtbl.create in
  let heights = ref [] in
  let add tbl height idx : unit =
    if idx = List.length hist - 1 then () else begin
      if not (List.mem height !heights) then heights := height :: !heights;
      if not (Hashtbl.mem tbl height) then Hashtbl.add tbl height [idx] else
      Hashtbl.find tbl height |> List.cons idx |> Hashtbl.add tbl height end in
  List.iteri (fun idx (_,stk) -> add tbl (List.length stk) idx) hist;
  !heights, tbl

(* produces a list of all the labels in the input sequence *)
let collect_labels (seq : IS.t) : Label.t list =
  let f labels = function
  | ILabel l when List.mem l labels |> not -> l :: labels
  | _ -> labels in
  IS.InstrSeq.fold_left seq ~f:f ~init:[]

(* produces a record containing the stack history and the labels of the input
 * sequence *)
let seq_data (seq : IS.t) : seq_data =
  {labels = collect_labels seq; stack_history = stack_history seq}
