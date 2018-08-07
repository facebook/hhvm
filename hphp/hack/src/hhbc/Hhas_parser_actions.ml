(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Hhbc_ast
open Parsing
module TV = Typed_value

let check_srcloc = ref false

(*
  TODO: proper error handling...
*)
let report_error s = (Printf.eprintf "%s\n" s;
  raise Parse_error)

(* Just because parameter default values contain chunks of php
 source code, we have to lex triplequoted strings as strings. We
 then have to be able to parse those strings as attributes. Rather
 than define a subsidiary lex/yacc parser for that, we just do it
 by hand, 'cos it's not very complex *)
exception Pkv
let pair_key_values l =
  let rec pkv l sofar =
    match l with
    | [] -> List.rev sofar
    | k::(v::rest) -> pkv rest ((k,v)::sofar)
    | _ -> raise Pkv
  in pkv l []

let rec unpack_key_values l =
match l with
 | [] -> []
 | (k,v)::rest -> k::(v:: unpack_key_values rest)

(* changing this to return a Typed_value.t instead of an
   instruct_lit_const.t list *)
let rec parse_attribute c =
 try
   Scanf.bscanf c "%0c" (fun ch -> match ch with
           | 'N' -> Scanf.bscanf c "N;" (Some Typed_value.Null)
           | 'i' -> Scanf.bscanf c "i:%Ld;" (fun n -> Some (Typed_value.Int n))
           | 'b' -> Scanf.bscanf c "b:%d;"
                   (fun n -> match n with
                              | 0 -> Some (Typed_value.Bool false)
                              | 1 -> Some (Typed_value.Bool true)
                              | _ -> None)
           | 'd' -> Scanf.bscanf c "d:%0c"
                   ( let read_float c =
                       Scanf.bscanf c "%g;" (fun f -> Some (Typed_value.Float f)) in
                     fun ch -> match ch with
                            | 'N' -> Scanf.bscanf c "NAN;" (Some (Typed_value.Float nan))
                            | 'I' -> Scanf.bscanf c "INF;" (Some (Typed_value.Float infinity))
                            | _ ->
                                (try Scanf.bscanf c "-INF;" (Some (Typed_value.Float neg_infinity))
                                with _ -> read_float c))
           | 's' -> Scanf.bscanf c "s:%d:\""
                   (fun n -> let myfmt =
                                 Scanf.format_from_string
                                 ("%" ^ (string_of_int n) ^ "[\000-\255]\";")
                                 ("%4[\000-\255]\"") in
                                 Scanf.bscanf c myfmt (fun s ->
                                   (Some (Typed_value.String s))))
           | 'a' -> Scanf.bscanf c "a:%d:{" (fun _n ->
                                     let al = parse_attribute_list c [] in
                                     let pkvl = pair_key_values al in
                                     Scanf.bscanf c "}"
                                         (Some (Typed_value.Array pkvl)))
           | 'y' -> Scanf.bscanf c "y:%d:{" (fun _n ->
                                     let al = parse_attribute_list c [] in
                                     Scanf.bscanf c "}"
                                         (Some (Typed_value.VArray al)))
           | 'v' -> Scanf.bscanf c "v:%d:{" (fun _n ->
                                     let al = parse_attribute_list c [] in
                                     Scanf.bscanf c "}"
                                         (Some (Typed_value.Vec al)))
           | 'k' -> Scanf.bscanf c "k:%d:{" (fun _n ->
                                     let al = parse_attribute_list c [] in
                                     Scanf.bscanf c "}"
                                         (Some (Typed_value.Keyset al)))
           | 'D' -> Scanf.bscanf c "D:%d:{" (fun _n ->
                                     let al = parse_attribute_list c [] in
                                     let pkvl = pair_key_values al in
                                     Scanf.bscanf c "}"
                                         (Some (Typed_value.Dict pkvl)))
           | 'Y' -> Scanf.bscanf c "Y:%d:{" (fun _n ->
                                     let al = parse_attribute_list c [] in
                                     let pkvl = pair_key_values al in
                                     Scanf.bscanf c "}"
                                         (Some (Typed_value.DArray pkvl)))
           | _ -> None)
 with
  | _ -> None
 and parse_attribute_list c sofar =
 match parse_attribute c with
 | None -> List.rev sofar
 | Some a -> parse_attribute_list c (a::sofar)

(* Now that lexing triplequoted strings doesn't do any unescaping at all,
   we need a separate unescaper to run before we scan them in attributes.
   It would be more efficient to fold this pass into the scanning done
   by parse_attribute above. But this is easier, because of the fact that
   we use the length to decide how many characters to read in the 's' case.
   Note that match s[i+1] can throw if the string ends with a naked backslash.
*)
let is_octal_digit c = '0' <= c && c <= '7'
let convert_octal_digit c = if is_octal_digit c
                            then (int_of_char c - int_of_char '0')
                            else invalid_arg "bad octal digit"
let convert_octal_digits c1 c2 c3 =
  64*(convert_octal_digit c1)+ 8*(convert_octal_digit c2)+(convert_octal_digit c3)

let my_unescape s =
  let num_chars = String.length s in
  let buf = Buffer.create num_chars in
  let rec copy_from i =
    let single_char_escape c = Buffer.add_char buf c; copy_from (i+2) in
    if i = num_chars then Buffer.contents buf
    else match s.[i] with
      | '\\' -> (match s.[i+1] with
                  | '\\' -> single_char_escape '\\'
                  | 'r' -> single_char_escape '\r'
                  | 'n' -> single_char_escape '\n'
                  | 't' -> single_char_escape '\t'
                  | '?' -> single_char_escape '?'
                  | '"' -> single_char_escape '"'
                  | c1 when is_octal_digit c1 ->
                     (match convert_octal_digits s.[i+1] s.[i+2] s.[i+3] with
                        | n -> Buffer.add_char buf (char_of_int n); copy_from (i+4)
                        | exception _ -> report_error "bad octal in triplequoted")
                  | _ ->
                    report_error
                      @@ Printf.sprintf
                        "bad escaped character in triplequoted ('%c')"
                        s.[i+1]
                  | exception _ -> report_error "partial escape in triplequoted"
                  )
      | c -> Buffer.add_char buf c ; copy_from (i+1) in
  copy_from 0

let attribute_from_string s =
  match parse_attribute (Scanf.Scanning.from_string (my_unescape s)) with
  | Some a -> a
  | None -> report_error ("attribute from string failed on " ^ s)

type decl =
  | Main_decl of Hhas_body.t
  | Fun_decl of Hhas_function.t
  | Class_decl of Hhas_class.t
  | Data_decl of Hhas_adata.t
  | Alias_decl of Hhas_typedef.t
  | Includes_decl of Hhas_symbol_refs.IncludePathSet.t
  | ConstantRefs_decl of SSet.t
  | ClassRefs_decl of SSet.t
  | FunctionRefs_decl of SSet.t
  | HHFile_decl of bool
  | StrictTypes_decl of bool

let rec split_decl_list ds hh_file strict_types funs classes optmain datadecls aliasdecls
    includesdecls constantrefsdecls classrefsdecls functionrefsdecls =
  match ds with
    | [] ->
      begin match optmain with
        | None -> report_error "missing main"
        | Some m ->
          let symbol_refs =
            { Hhas_symbol_refs.includes  = includesdecls
            ; Hhas_symbol_refs.constants = constantrefsdecls
            ; Hhas_symbol_refs.classes   = classrefsdecls
            ; Hhas_symbol_refs.functions = functionrefsdecls
            } in
          Hhas_program.make hh_file (List.rev datadecls) (List.rev funs)
            (List.rev classes) (List.rev aliasdecls) m symbol_refs (Some strict_types)
      end
    | Main_decl md :: rest ->
      begin match optmain with
        | None ->
          split_decl_list rest hh_file strict_types funs classes (Some md) datadecls aliasdecls
            includesdecls constantrefsdecls classrefsdecls functionrefsdecls
        | Some _ -> report_error "duplicate main"
      end
    | Fun_decl fd :: rest ->
      split_decl_list rest hh_file strict_types (fd :: funs) classes optmain datadecls aliasdecls
        includesdecls constantrefsdecls classrefsdecls functionrefsdecls
    | Class_decl cd :: rest ->
      split_decl_list rest hh_file strict_types funs (cd :: classes) optmain datadecls aliasdecls
        includesdecls constantrefsdecls classrefsdecls functionrefsdecls
    | Data_decl dd :: rest ->
      split_decl_list rest hh_file strict_types funs classes optmain (dd :: datadecls) aliasdecls
        includesdecls constantrefsdecls classrefsdecls functionrefsdecls
    | Alias_decl ad :: rest ->
      split_decl_list rest hh_file strict_types funs classes optmain datadecls (ad :: aliasdecls)
        includesdecls constantrefsdecls classrefsdecls functionrefsdecls
    | Includes_decl ids :: rest ->
      let includes = Hhas_symbol_refs.IncludePathSet.union ids includesdecls in
      split_decl_list rest hh_file strict_types funs classes optmain datadecls aliasdecls
         includes constantrefsdecls classrefsdecls functionrefsdecls
    | ConstantRefs_decl crs :: rest ->
      let constant_refs = SSet.union crs constantrefsdecls in
      split_decl_list rest hh_file strict_types funs classes optmain datadecls aliasdecls
        includesdecls constant_refs classrefsdecls functionrefsdecls
    | ClassRefs_decl crs :: rest ->
      let class_refs = SSet.union crs classrefsdecls in
      split_decl_list rest hh_file strict_types funs classes optmain datadecls aliasdecls
        includesdecls constantrefsdecls class_refs functionrefsdecls
    | FunctionRefs_decl frs :: rest ->
      let function_refs = SSet.union frs functionrefsdecls in
      split_decl_list rest hh_file strict_types funs classes optmain datadecls aliasdecls
        includesdecls constantrefsdecls classrefsdecls function_refs
    | HHFile_decl hh_file :: rest ->
      split_decl_list rest hh_file strict_types funs classes optmain datadecls aliasdecls
        includesdecls constantrefsdecls classrefsdecls functionrefsdecls
    | StrictTypes_decl strict_types :: rest ->
      split_decl_list rest hh_file strict_types funs classes optmain datadecls aliasdecls
        includesdecls constantrefsdecls classrefsdecls functionrefsdecls

(* This is a pretty poor way to deal with these flags on functions, and
   doesn't Throw if there's an illegal one in the list, but it'll do for now.
*)
let isasync ss = List.mem "isAsync" ss
let isgenerator ss = List.mem "isGenerator" ss
let ispairgenerator ss = List.mem "isPairGenerator" ss

let makelabel s =
 let len = String.length s in
   match s.[0] with
    | 'L' -> (Label.Regular (int_of_string (String.sub s 1 (len-1))))
    | 'C' -> (Label.Catch (int_of_string (String.sub s 1 (len-1))))
    | 'F' -> (Label.Fault (int_of_string (String.sub s 1 (len-1))))
    | 'D' -> if s.[1] = 'V'
             then (Label.DefaultArg (int_of_string (String.sub s 2 (len-2))))
             else report_error "bad label: 'D', s.[1] <> 'V'"
    | _ -> Label.Named s
let makelabelinst s = ILabel (makelabel s)

type precedence_or_alias =
 | Precedence of (string * string * string list)
 | Alias of (string option * string * string option * Ast.kind list)

let vis_of s = match s with
 | "public" ->  Ast.Public
 | "private" -> Ast.Private
 | "protected" -> Ast.Protected
 | _ -> report_error "bad visibility attribute in alias"

let colon_split s = match Str.split (Str.regexp "::") s with
 | [first_id; second_id] -> Some (first_id, second_id)
 | [_one_id] -> None
 | _ -> report_error "bad double colon split in precedence or alias"

let parse_precedence_or_alias xs = match xs with
 | [] -> report_error "empty idlist for precedence or alias"
 | x :: rest ->
   (match colon_split x with
     | Some (first_id, second_id) ->
       (match rest with
         | ["as"; lastid] -> Alias (Some first_id, second_id, Some lastid, [])
         | "insteadof" :: rest2 -> Precedence (first_id, second_id, rest2)
         | _ -> report_error "bad idlist after colonsplit in precedence or alias")
     | None ->
       (match rest with
         | ["as"; lastid] -> Alias (None, x, Some lastid, [])
         | _ -> report_error "bad idlist after id in precedence or alias"))

let parse_alias x y vislist opt_lastid =
   let (first, second) =
     match colon_split x with
      | Some (first_id, second_id) -> (Some first_id, second_id)
      | None -> (None, x) in
   let vis = List.map (function "final" -> Ast.Final | x -> vis_of x) vislist in
   if y = "as" then
    Alias (first, second, opt_lastid, vis)
   else report_error "missing as in alias"

(* TODO: replace with list library function *)
let rec split_classconflicts xss = match xss with
 | [] -> ([],[])
 | xs :: rest ->
   let (aliases, precedences) = split_classconflicts rest
   in match xs with
       | Alias tup -> (tup :: aliases, precedences)
       | Precedence tup -> (aliases, tup :: precedences)

(* TODO: replace stupidly big match with a hash table. Bootcampable? *)
let make_nullary_inst s =
 match s with
 (* instruct_basic *)
 | "Nop" -> IBasic (Nop)
 | "EntryNop"-> IBasic (EntryNop)
 | "PopC" -> IBasic (PopC)
 | "PopV" -> IBasic (PopV)
 | "PopR" -> IBasic (PopR)
 | "PopU" -> IBasic (PopU)
 | "Dup"  -> IBasic (Dup)
 | "Box"  -> IBasic (Box)
 | "Unbox" -> IBasic (Unbox)
 | "BoxR"  -> IBasic (BoxR)
 | "BoxRNop"  -> IBasic (BoxRNop)
 | "UnboxR" -> IBasic (UnboxR)
 | "UnboxRNop" -> IBasic (UnboxRNop)
 | "RGetCNop" -> IBasic (RGetCNop)

 (* instruct_lit_const *)
 | "Null" -> ILitConst (Null)
 | "True" -> ILitConst (True)
 | "False" -> ILitConst (False)
 | "NullUninit" -> ILitConst (NullUninit)
 | "AddElemC"-> ILitConst (AddElemC)
 | "AddElemV"-> ILitConst (AddElemV)
 | "AddNewElemC"-> ILitConst (AddNewElemC)
 | "AddNewElemV"-> ILitConst (AddNewElemV)
 | "File"-> ILitConst (File)
 | "Dir"-> ILitConst (Dir)
 | "Method"-> ILitConst (Method)
 | "NewPair" -> ILitConst NewPair

 (* instruct_operator *)
 | "Concat"-> IOp (Concat)
 | "Abs" -> IOp (Abs)
 | "Add" -> IOp (Add)
 | "Sub" -> IOp (Sub)
 | "Mul" -> IOp (Mul)
 | "AddO" -> IOp (AddO)
 | "SubO" -> IOp (SubO)
 | "MulO" -> IOp (MulO)
 | "Div" -> IOp (Div)
 | "Mod" -> IOp (Mod)
 | "Pow" -> IOp (Pow)
 | "Sqrt" -> IOp (Sqrt)
 | "Xor" -> IOp (Xor)
 | "Not" -> IOp (Not)
 | "Same" -> IOp (Same)
 | "NSame" -> IOp (NSame)
 | "Eq" -> IOp (Eq)
 | "Neq" -> IOp (Neq)
 | "Lt" -> IOp (Lt)
 | "Lte" -> IOp (Lte)
 | "Gt" -> IOp (Gt)
 | "Gte" -> IOp (Gte)
 | "Cmp" -> IOp (Cmp)
 | "BitAnd" -> IOp (BitAnd)
 | "BitOr" -> IOp (BitOr)
 | "BitXor" -> IOp (BitXor)
 | "BitNot" -> IOp (BitNot)
 | "Shl" -> IOp (Shl)
 | "Shr" -> IOp (Shr)
 | "Floor" -> IOp (Floor)
 | "Ceil" -> IOp (Ceil)
 | "CastBool" -> IOp (CastBool)
 | "CastInt" -> IOp (CastInt)
 | "CastDouble" -> IOp (CastDouble)
 | "CastString" -> IOp (CastString)
 | "CastArray" -> IOp (CastArray)
 | "CastObject" -> IOp (CastObject)
 | "CastVec" -> IOp (CastVec)
 | "CastDict" -> IOp (CastDict)
 | "CastKeyset" -> IOp (CastKeyset)
 | "CastVArray" -> IOp (CastVArray)
 | "CastDArray" -> IOp (CastDArray)
 | "InstanceOf" -> IOp (InstanceOf)
 | "Print" -> IOp (Print)
 | "Clone" -> IOp (Clone)
 | "Exit" -> IOp (Hhbc_ast.Exit) (* Need to qualify because of shadowing *)

 (* instruct_control_flow *)
 | "RetC" -> IContFlow (RetC)
 | "RetV" -> IContFlow (RetV)
 | "Unwind" -> IContFlow (Unwind)
 | "Throw" -> IContFlow (Throw)

 (* instruct_get *)
 | "CGetN" -> IGet (CGetN)
 | "CGetQuietN" -> IGet (CGetQuietN)
 | "CGetG" -> IGet (CGetG)
 | "CGetQuietG" -> IGet (CGetQuietG)
 | "VGetN" -> IGet (VGetN)
 | "VGetG" -> IGet (VGetG)

 (* instruct_isset *)
 | "IssetC" -> IIsset (IssetC)
 | "IssetN" -> IIsset (IssetN)
 | "IssetG" -> IIsset (IssetG)
 | "EmptyN" -> IIsset (EmptyN)
 | "EmptyG" -> IIsset (EmptyG)

 (* instruct_mutator *)
 | "SetN" -> IMutator (SetN)
 | "SetG" -> IMutator (SetG)
 | "BindN" -> IMutator (BindN)
 | "BindG" -> IMutator (BindG)
 | "UnsetN" -> IMutator (UnsetN)
 | "UnsetG" -> IMutator (UnsetG)

 (* instruct_base *)
 | "BaseH" -> IBase(BaseH)

 (* instruct_include_eval_define *)
 | "Incl" -> IIncludeEvalDefine(Incl)
 | "InclOnce" -> IIncludeEvalDefine(InclOnce)
 | "Req" -> IIncludeEvalDefine(Req)
 | "ReqOnce" -> IIncludeEvalDefine(ReqOnce)
 | "Eval" -> IIncludeEvalDefine(Eval)

 (* instruct_misc *)
 | "This" -> IMisc(This)
 | "CheckThis" -> IMisc(CheckThis)
 | "Catch" -> IMisc(Catch)
 | "ChainFaults" -> IMisc(ChainFaults)
 | "VerifyRetTypeC" -> IMisc(VerifyRetTypeC)
 | "VerifyRetTypeV" -> IMisc(VerifyRetTypeV)
 | "NativeImpl" -> IMisc(NativeImpl)
 | "AKExists" -> IMisc(AKExists)
 | "Idx" -> IMisc(Idx)
 | "ArrayIdx" -> IMisc(ArrayIdx)
 | "BreakTraceHint" -> IMisc(BreakTraceHint)
 | "CGetCUNop" -> IMisc(CGetCUNop)
 | "UGetCUNop" -> IMisc(UGetCUNop)

 (* async_functions *)
 | "Await" -> IAsync Await
 | "WHResult" -> IAsync WHResult

 (* generator functions *)
 | "CreateCont" -> IGenerator CreateCont
 | "ContEnter" -> IGenerator ContEnter
 | "ContRaise" -> IGenerator ContRaise
 | "Yield" -> IGenerator Yield
 | "YieldK" -> IGenerator YieldK
 | "ContValid" -> IGenerator ContValid
 | "ContKey" -> IGenerator ContKey
 | "ContCurrent" -> IGenerator ContCurrent
 | "ContGetReturn" -> IGenerator ContGetReturn
 | "ContStarted" -> IGenerator ContStarted

 | "ContEnterDelegate" -> IGenDelegation ContEnterDelegate
 | _ -> failwith ("NYI nullary: " ^ s)

type iarg =
  | IAInt64 of int64
  | IAString of string
  | IAId of string
  | IADouble of string (* seems we don't parse these *)
  | IAArrayno of adata_id
  | IAMemberkey of string*iarg (* these are not seriously recursive *)
  | IAArglist of iarg list
  | IAIteratorid of string*int64
  | IASswitchcase of string*string (* second should be a label *)

let class_id_of_iarg arg =
  match arg with
  | IAString s -> Hhbc_id.Class.from_raw_string s
  | _ -> report_error "expected quoted class identifier"

let class_num_of_iarg arg =
  match arg with
  | IAInt64 i -> Int64.to_int i
  | _ -> report_error "expected class number"

let function_num_of_iarg arg =
  match arg with
  | IAInt64 i -> Int64.to_int i
  | _ -> report_error "expected function number"

let typedef_num_of_iarg arg =
  match arg with
  | IAInt64 i -> Int64.to_int i
  | _ -> report_error "expected typedef number"

let prop_id_of_iarg arg =
  match arg with
  | IAString s -> Hhbc_id.Prop.from_raw_string s
  | _ -> report_error "expected quoted property identifier"

let const_id_of_iarg arg =
  match arg with
  | IAString s -> Hhbc_id.Const.from_raw_string s
  | _ -> report_error "expected quoted const identifier"

let function_id_of_iarg arg =
  match arg with
  | IAString s -> Hhbc_id.Function.from_raw_string s
  | IAInt64 n -> Hhbc_id.Function.from_raw_string (Int64.to_string n)
  | _ -> report_error "expected quoted function identifier"

let method_id_of_iarg arg =
  match arg with
  | IAString s -> Hhbc_id.Method.from_raw_string s
  | _ -> report_error "expected quoted method identifier"

let stringofiarg arg =
match arg with
  | IAString s -> s
  | IAId s -> s
  | _ -> report_error "expected string arg"

let int64ofiarg arg =
match arg with
  | IAInt64 n -> n
  | _ -> report_error "expected int64 arg"

let localidofiarg arg =
match arg with
 | IAId s -> if s.[0] = '_'
             then Local.Unnamed
              (int_of_string (String.sub s 1 (String.length s - 1)))
             else Local.Named s
 | IAMemberkey (s,IAInt64 n) ->
             if s="L" then Local.Unnamed (Int64.to_int n)
             else report_error "expected L:n member key"
 | _ -> report_error "bad local arg"

let intofiarg arg =
match arg with
 | IAInt64 n -> Int64.to_int n
 | _ -> report_error "expected integer instruction argument"

let typeopofiarg arg =
match arg with
  | IAId s -> (match s with
               | "Null" -> OpNull
               | "Bool" -> OpBool
               | "Int" -> OpInt
               | "Dbl" -> OpDbl
               | "Str" -> OpStr
               | "Arr" -> OpArr
               | "Obj" -> OpObj
               | "Res" -> OpRes
               | "Scalar" -> OpScalar
               | "Keyset" -> OpKeyset
               | "Dict" -> OpDict
               | "Vec" -> OpVec
               | "ArrLike" -> OpArrLike
               | "VArray" -> OpVArray
               | "DArray" -> OpDArray
               | _ -> report_error ("bad istype_op" ^ s))
  | _ -> report_error "bad arg to istype_op"

let eqopofiarg arg =
  match arg with
  | IAId s -> (match s with
    | "PlusEqual" -> PlusEqual
    | "MinusEqual" -> MinusEqual
    | "MulEqual" -> MulEqual
    | "ConcatEqual" -> ConcatEqual
    | "DivEqual" -> DivEqual
    | "PowEqual" -> PowEqual
    | "ModEqual" -> ModEqual
    | "AndEqual" -> AndEqual
    | "OrEqual" -> OrEqual
    | "XorEqual" -> XorEqual
    | "SlEqual" -> SlEqual
    | "SrEqual" -> SrEqual
    | "PlusEqualO" -> PlusEqualO
    | "MinusEqualO" -> MinusEqualO
    | "MulEqualO" -> MulEqualO
    | _ ->
      report_error
      @@ Printf.sprintf "bad eqop: '%s'" s
  )
  | _ -> report_error "wrong kind of eqop arg"

let collectiontypeofiarg arg =
  match arg with
  | IAId s -> (match s with
    | "Vector" -> CollectionType.Vector
    | "Map" -> CollectionType.Map
    | "Set" -> CollectionType.Set
    | "Pair" -> CollectionType.Pair
    | "ImmVector" -> CollectionType.ImmVector
    | "ImmMap" -> CollectionType.ImmMap
    | "ImmSet" -> CollectionType.ImmSet
    | _ ->
      report_error
      @@ Printf.sprintf "bad collection type: '%s'" s
  )
  | _ -> report_error "wrong kind of collection type arg"

let queryopofiarg arg =
  match arg with
  | IAId s -> (
    match s with
    | "CGet" -> QueryOp.CGet
    | "CGetQuiet" -> QueryOp.CGetQuiet
    | "Isset" -> QueryOp.Isset
    | "Empty" -> QueryOp.Empty
    | "InOut" -> QueryOp.InOut
    | _ ->
      report_error
      @@ Printf.sprintf "unknown queryop: '%s'" s
  )
  | _ -> report_error "bad query op arg type"

let memberkeyofiarg arg =
  match arg with
  | IAMemberkey (s',arg') -> (
    match s' with
    | "EC" -> MemberKey.EC (intofiarg arg')
    | "EL" -> MemberKey.EL (localidofiarg arg')
    | "ET" -> MemberKey.ET (stringofiarg arg')
    | "EI" -> MemberKey.EI (int64ofiarg arg')
    | "PC" -> MemberKey.PC (intofiarg arg')
    | "PL" -> MemberKey.PL (localidofiarg arg')
    | "PT" -> MemberKey.PT (prop_id_of_iarg arg')
    | "QT" -> MemberKey.QT (prop_id_of_iarg arg')
    | _ ->
      report_error
      @@ Printf.sprintf "unknown memberkey string: '%s'" s'
    )
  | IAId s' -> if s'="W" then MemberKey.W else report_error "bad memberkey"
  | _ -> report_error "bad memberkey"

let memoargofiarg arg =
  match arg with
  | IAMemberkey ("L", IAArglist [IAInt64 n; IAInt64 m]) ->
     let l = Int64.to_int n in
     let c = Int64.to_int m in
     if (c = 0) then None else Some (Local.Unnamed l, c)
  | _ -> report_error "bad memo arg"

let incdecopofiarg arg =
  match arg with
  | IAId s -> (
    match s with
    | "PreInc" -> PreInc
    | "PostInc" -> PostInc
    | "PreDec" -> PreDec
    | "PostDec" -> PostDec
    | "PreIncO" -> PreIncO
    | "PostIncO" -> PostIncO
    | "PreDecO" -> PreDecO
    | "PostDecO" -> PostDecO
    | _ ->
      report_error
      @@ Printf.sprintf "bad incdecop: '%s'" s
  )
  | _ -> report_error "wrong kind of incdecop arg"

let fpasshintof arg =
  match arg with
  | IAId s -> (
    match s with
    | "Any" -> Any
    | "Cell" -> Cell
    | "Ref" -> Ref
    | _ -> report_error @@ Printf.sprintf "bad fpasshint: '%s'" s
  )
  | _ -> report_error "wrong kind of fpasshint arg"

let paramidofiarg arg =
  match arg with
  | IAId s -> Param_named s
  | IAInt64 n -> Param_unnamed (Int64.to_int n)
  | _ -> report_error "bad param id to instruction"

let has_unpack_of_iarg arg =
  match arg with
  | IAInt64 n when n = Int64.zero -> false
  | IAInt64 n when n = Int64.one -> true
  | _ -> report_error "bad has_param"

let barethisopofiarg arg =
  match arg with
  | IAId "Notice" -> Notice
  | IAId "NoNotice" -> NoNotice
  | IAId "NeverNull" -> NeverNull
  | _ ->
    report_error "bad bare this op"

let classkindofiarg arg =
  match arg with
  | IAId "Class" -> KClass
  | IAId "Interface" -> KInterface
  | IAId "Trait" -> KTrait
  | _ -> report_error "bad class kind"

let listofshapefieldsofiarg arg =
  match arg with
  | IAArglist args -> List.map stringofiarg args
  | _ -> report_error "expected list of shape fields"

let listofintofiarg arg =
  match arg with
  | IAArglist args -> List.map intofiarg args
  | _ -> report_error "expected list of ints"

let listofboolofiarg arg =
  match arg with
  | IAString args ->
    let parse_char c =
      match c with
      | '0' -> false
      | '1' -> true
      | _ -> report_error "expected list of bools"
    in
    let len = String.length args in
    let rec aux i =
      if i == len then []
      else (parse_char (String.get args i)) :: (aux (i + 1))
    in aux 0
  | _ -> report_error "expected list of bools"

let initpropopofiarg arg =
  match arg with
  | IAId "Static" -> Static
  | IAId "NonStatic" -> NonStatic
  | _ -> report_error "bad initprop_op"

let nullflavorofiarg arg =
  match arg with
  | IAId "NullThrows" -> Ast.OG_nullthrows
  | IAId "NullSafe" -> Ast.OG_nullsafe
  | _ -> report_error "bad null flavor"

let labelofiarg arg =
  match arg with
  | IAId l -> makelabel l
  | _ -> report_error "bad label"

let iterofiarg arg = Iterator.Id (intofiarg arg)

let checkstarted_of_arg arg =
  match arg with
  | IAId "IgnoreStarted" -> IgnoreStarted
  | IAId "CheckStarted" -> CheckStarted
  | _ -> report_error "bad check_started"

let iterwithkindofiarg arg =
  match arg with
  | IAIteratorid (kind, id) -> kind = "MIter", Iterator.Id (Int64.to_int id)
  | _ -> report_error "bad iterator"

let memberopmodeofiarg arg =
  match stringofiarg arg with
  | "None" -> MemberOpMode.ModeNone
  | "Warn" -> MemberOpMode.Warn
  | "Define" -> MemberOpMode.Define
  | "Unset" -> MemberOpMode.Unset
  | "InOut" -> MemberOpMode.InOut
  | _ -> report_error ("bad member op mode" ^ stringofiarg arg)

let specialclsrefofiarg arg =
  match stringofiarg arg with
  | "Static" -> SpecialClsRef.Static
  | "Self" -> SpecialClsRef.Self
  | "Parent" -> SpecialClsRef.Parent
  | _ -> report_error ("bad special cls-ref" ^ stringofiarg arg)

let freeiteratorofiarg arg =
  match stringofiarg arg with
  | "IgnoreIter" -> IgnoreIter
  | "FreeIter" -> FreeIter
  | _ -> report_error ("bad free_iterator type " ^ stringofiarg arg)

let listofiteratorsofiarg arg =
   match arg with
   | IAArglist l -> List.map iterwithkindofiarg l
   | _ -> report_error "bad list of iterators"

let listoflabelsofiarg arg =
  match arg with
  | IAArglist l -> List.map labelofiarg l
  | _ -> report_error "bad list of labels"

let opsilenceofiarg arg =
  match stringofiarg arg with
  | "Start" -> Start
  | "End" -> End
  | _ ->
    report_error
    @@ Printf.sprintf "bad op_silence: '%s'" @@ stringofiarg arg

let switchkindofiarg arg =
  match stringofiarg arg with
  | "Bounded" -> Bounded
  | "Unbounded" -> Unbounded
  | _ ->
    report_error
    @@ Printf.sprintf "bad switch kind: '%s'" @@ stringofiarg arg

let to_inf_nan s =
 match String.uppercase_ascii s with
   | "NAN" -> Some "NAN"
   | "INF" -> Some "INF"
   | _ -> None

let doubleofiarg arg =
 match arg with
  | IADouble sd -> sd
  | IAId s -> (match to_inf_nan s with
                | Some s -> s
                | None -> report_error "bad double lit cst")
  (* Remark: the way we use to_inf_nan in two different places is pretty nasty, but seems the
     quickest way to deal with -INF and variants *)
  | IAInt64 n -> (Int64.to_string n) ^ "." (* ugh *)
  | _ -> report_error "bad double lit cst"

let makeunaryinst s arg = match s with
  (* instruct_lit_const *)
   | "Int" -> (match arg with | IAInt64 n -> ILitConst (Int n)
                              | _ -> report_error "bad int lit cst")
   | "Double" -> ILitConst (Double (doubleofiarg arg))
   | "String" ->
    (match arg with | IAString sa -> ILitConst (String sa)
                      | _ -> report_error "bad string lit cst")
   | "Array" -> (match arg with
       | IAArrayno n -> ILitConst (Array n)
       | _ -> report_error "bad array lit cst")
                                (* Q: where's the real data?
                                   A: it's in the adata declaration, which
                                    we'll splice in here in a later pass.
                                *)
   | "Vec" -> (match arg with
       | IAArrayno n -> ILitConst (Vec n)
       | _ -> report_error "bad vec lit cst")
   | "Dict" -> (match arg with
       | IAArrayno n -> ILitConst (Dict n)
       | _ -> report_error "bad dict lit cst")
   | "Keyset" -> (match arg with
       | IAArrayno n -> ILitConst (Keyset n)
       | _ -> report_error "bad keyset lit cst")
   | "NewArray" -> (match arg with
       | IAInt64 n -> ILitConst (NewArray (Int64.to_int n))
       | _ -> report_error "bad array size")
   | "NewMixedArray" -> (match arg with
       | IAInt64 n -> ILitConst (NewMixedArray (Int64.to_int n))
       | _ -> report_error "bad array size")
   | "NewDictArray" -> (match arg with
       | IAInt64 n -> ILitConst (NewDictArray (Int64.to_int n))
       | _ -> report_error "bad array size")
   | "NewPackedArray" -> (match arg with
       | IAInt64 n -> ILitConst (NewPackedArray (Int64.to_int n))
       | _ -> report_error "bad array size")
   | "NewStructArray" ->
        (ILitConst(NewStructArray (listofshapefieldsofiarg arg)))
   | "NewStructDArray" ->
        (ILitConst(NewStructDArray (listofshapefieldsofiarg arg)))
   | "NewStructDict" ->
        (ILitConst(NewStructDict (listofshapefieldsofiarg arg)))
   | "NewVecArray" -> (match arg with
       | IAInt64 n -> ILitConst (NewVecArray (Int64.to_int n))
       | _ -> report_error "bad array size")
   | "NewKeysetArray" -> (match arg with
       | IAInt64 n -> ILitConst (NewKeysetArray (Int64.to_int n))
       | _ -> report_error "bad array size")
   | "NewVArray" -> (match arg with
       | IAInt64 n -> ILitConst (NewVArray (Int64.to_int n))
       | _ -> report_error "bad array size")
   | "NewDArray" -> (match arg with
       | IAInt64 n -> ILitConst (NewDArray (Int64.to_int n))
       | _ -> report_error "bad array size")
   | "NewCol" -> ILitConst (NewCol (collectiontypeofiarg arg))
   | "ColFromArray" -> ILitConst (ColFromArray (collectiontypeofiarg arg))
   | "Cns" -> (match arg with
       | IAString sa -> ILitConst (Cns (Hhbc_id.Const.from_raw_string sa))
       | _ -> report_error "bad cns arg")
   | "CnsE" -> (match arg with
       | IAString sa -> ILitConst (CnsE (Hhbc_id.Const.from_raw_string sa))
       | _ -> report_error "bad cnse arg")

 (* instruct_operator *)
   | "Fatal" -> (match arg with
       | IAId op -> IOp (Fatal
                (match op with | "Parse" -> FatalOp.Parse
                               | "Runtime" -> FatalOp.Runtime
                               | "RuntimeOmitFrame" -> FatalOp.RuntimeOmitFrame
                               | _ -> report_error "bad fatal op arg"))
       | _ -> report_error "bad fatal op arg")
   | "InstanceOfD" -> (match arg with
       | IAString sa -> IOp (InstanceOfD (Hhbc_id.Class.from_raw_string sa))
       | _ -> report_error "bad InstanceOfD arg")
   | "IsTypeStruct" -> (match arg with
       | IAArrayno n -> IOp (IsTypeStruct n)
       | _ -> report_error "bad array lit cst")
   | "AsTypeStruct" -> (match arg with
       | IAArrayno n -> IOp (AsTypeStruct n)
       | _ -> report_error "bad array lit cst")
   | "ConcatN" -> IOp (ConcatN (intofiarg arg))

(* instruct_control_flow *)
   | "Jmp" -> (match arg with
       | IAId l -> IContFlow (Jmp (makelabel l))
       | _ -> report_error "bad jmp label")
   | "JmpNS" -> (match arg with
       | IAId l -> IContFlow (JmpNS (makelabel l))
       | _ -> report_error "bad jmp label")
   | "JmpZ" -> (match arg with
       | IAId l -> IContFlow (JmpZ (makelabel l))
       | _ -> report_error "bad jmp label")
   | "JmpNZ" -> (match arg with
       | IAId l -> IContFlow (JmpNZ (makelabel l))
       | _ -> report_error "bad jmp label")
   | "SSwitch" -> (match arg with
       | IAArglist cases ->
          let args = List.map (function IASswitchcase (s, l) -> (s, makelabel l)
                                        | _ -> report_error "bad sswitch case") cases
          in IContFlow (SSwitch args)
       | _ -> report_error "sswitch expects list of cases")

  (* instruct_get *)
   | "CGetL" -> IGet (CGetL (localidofiarg arg))
   | "CGetQuietL" -> IGet (CGetQuietL (localidofiarg arg))
   | "CGetL2" -> IGet (CGetL2 (localidofiarg arg))
   | "CUGetL" -> IGet (CUGetL (localidofiarg arg))
   | "PushL" -> IGet (PushL (localidofiarg arg))
   | "CGetS" -> IGet (CGetS (intofiarg arg))
   | "VGetL" -> IGet (VGetL (localidofiarg arg))
   | "VGetS" -> IGet (VGetS (intofiarg arg))
   | "ClsRefGetC" -> IGet (ClsRefGetC (intofiarg arg))

   (*instruct_isset *)
   | "IssetL" -> IIsset (IssetL (localidofiarg arg))
   | "IssetS" -> IIsset (IssetS (intofiarg arg))
   | "EmptyL" -> IIsset (EmptyL (localidofiarg arg))
   | "EmptyS" -> IIsset (EmptyS (intofiarg arg))
   | "IsTypeC" -> IIsset (IsTypeC (typeopofiarg arg))

   (* instruct_mutator *)
   | "SetL" -> IMutator (SetL (localidofiarg arg))
   | "PopL" -> IMutator (PopL (localidofiarg arg))
   | "SetS" -> IMutator (SetS (intofiarg arg))
   | "SetOpN" -> IMutator(SetOpN (eqopofiarg arg))
   | "SetOpG" -> IMutator(SetOpG (eqopofiarg arg))
   | "IncDecN" -> IMutator(IncDecN (incdecopofiarg arg))
   | "IncDecG" -> IMutator(IncDecG (incdecopofiarg arg))
   | "BindL" -> IMutator(BindL (localidofiarg arg))
   | "BindS" -> IMutator(BindS (intofiarg arg))
   | "UnsetL" -> IMutator(UnsetL (localidofiarg arg))
   | "CheckProp" -> IMutator(CheckProp (prop_id_of_iarg arg))

   (* instruct_call *)
   | "FPushFunc" -> ICall(FPushFunc (intofiarg arg, []))
   | "FThrowOnRefMismatch" -> ICall(FThrowOnRefMismatch (listofboolofiarg arg))
   | "RetM" -> IContFlow(RetM (intofiarg arg))
   | "ResolveFunc" -> IOp(ResolveFunc (function_id_of_iarg arg))
   | "ResolveObjMethod" -> IOp (ResolveObjMethod)
   | "ResolveClsMethod" -> IOp (ResolveClsMethod)

   (* instruct_final *)
   | "SetWithRefRML" -> IFinal(SetWithRefRML(localidofiarg arg))

   (* instruct_iterator *)
   | "IterFree" -> IIterator(IterFree (Iterator.Id (intofiarg arg)))
   | "MIterFree" ->IIterator(MIterFree (Iterator.Id (intofiarg arg)))
   | "CIterFree" ->IIterator(CIterFree (Iterator.Id (intofiarg arg)))

   (* async_functions
      TODO: double-check the +1 in this case
   *)
   | "AwaitAll" -> let l = memoargofiarg arg in IAsync(AwaitAll l)

   (* instruct_include_eval_define *)
   | "DefFunc" -> IIncludeEvalDefine(DefFunc (function_num_of_iarg arg))
   | "DefCls" -> IIncludeEvalDefine(DefCls (class_num_of_iarg arg))
      (* TODO: Mismatch - that should be an integer, not a string *)
   | "DefClsNop" -> IIncludeEvalDefine(DefClsNop (class_num_of_iarg arg))
   | "DefCns" -> IIncludeEvalDefine(DefCns (const_id_of_iarg arg))
   | "DefTypeAlias" ->
     IIncludeEvalDefine(DefTypeAlias(typedef_num_of_iarg arg))
     (* TODO: Mismatch here too *)

   (* instruct_misc *)
   | "BareThis" -> IMisc(BareThis (barethisopofiarg arg))
   | "InitThisLoc" -> IMisc(InitThisLoc (localidofiarg arg))
   | "OODeclExists" -> IMisc(OODeclExists (classkindofiarg arg))
   | "VerifyParamType" -> IMisc(VerifyParamType (paramidofiarg arg))
   | "VerifyOutType" -> IMisc(VerifyOutType (paramidofiarg arg))
   | "Self" -> IMisc(Self (intofiarg arg))
   | "Parent" -> IMisc(Parent (intofiarg arg))
   | "LateBoundCls" -> IMisc(LateBoundCls (intofiarg arg))
   | "ClsRefName" -> IMisc(ClsRefName (intofiarg arg))
   | "GetMemoKeyL" -> IMisc(GetMemoKeyL (localidofiarg arg))
   | "MemoSet" -> IMisc (MemoSet (memoargofiarg arg))

   | "ContAssignDelegate" -> IGenDelegation (ContAssignDelegate (iterofiarg arg))
   (* Note: The TryCatch/TryFault instructions don't show up here because the
      textual bytecode format represents them using directives and braces rather
      than instructions
   *)
   | "ContCheck" -> IGenerator (ContCheck (checkstarted_of_arg arg))
   | _ -> failwith ("NYI unary: " ^ s)

let makebinaryinst s arg1 arg2 =
match s with
 (* instruct_lit_const *)
 | "NewLikeArrayL" ->
     ILitConst(NewLikeArrayL(localidofiarg arg1, intofiarg arg2))
 | "CnsU" -> ILitConst(CnsU (const_id_of_iarg arg1, stringofiarg arg2))
 | "ClsCns" -> ILitConst(ClsCns (const_id_of_iarg arg1, intofiarg arg2))
 | "ClsCnsD" ->
   ILitConst(ClsCnsD (const_id_of_iarg arg1, class_id_of_iarg arg2))

 (* instruct_get *)
 | "ClsRefGetL" -> IGet (ClsRefGetL (localidofiarg arg1, intofiarg arg2))

 (* instruct_isset *)
 | "IsTypeL" -> IIsset (IsTypeL (localidofiarg arg1, typeopofiarg arg2))

 (* instruct_mutator *)
 | "SetOpL" -> IMutator (SetOpL (localidofiarg arg1, eqopofiarg arg2))
 | "SetOpS" -> IMutator (SetOpS (eqopofiarg arg1, intofiarg arg2))
 | "IncDecL" -> IMutator (IncDecL (localidofiarg arg1, incdecopofiarg arg2))
 | "IncDecS" -> IMutator (IncDecS (incdecopofiarg arg1, intofiarg arg2))
 | "InitProp" ->
   IMutator (InitProp (prop_id_of_iarg arg1, initpropopofiarg arg2))

 (* instruct_call *)
 | "FPushObjMethod" ->
    ICall(FPushObjMethod (intofiarg arg1, nullflavorofiarg arg2, []))
 | "FPushClsMethod" -> ICall (FPushClsMethod (intofiarg arg1, intofiarg arg2, []))
 | "FPushFunc" -> ICall(FPushFunc (intofiarg arg1, listofintofiarg arg2))
 | "FPushFuncD" -> ICall (FPushFuncD (intofiarg arg1, function_id_of_iarg arg2))
 | "FPushClsMethodS" -> ICall (FPushClsMethodS (intofiarg arg1, specialclsrefofiarg arg2))
 | "FPushCtor" -> ICall (FPushCtor (intofiarg arg1, intofiarg arg2))
 | "FPushCtorD" -> ICall (FPushCtorD (intofiarg arg1, class_id_of_iarg arg2))
 | "FPushCtorI" -> ICall (FPushCtorI (intofiarg arg1, intofiarg arg2))
 | "FPushCtorS" -> ICall (FPushCtorS (intofiarg arg1, specialclsrefofiarg arg2))
 | "DecodeCufIter" -> ICall (DecodeCufIter (iterofiarg arg1, labelofiarg arg2))
 | "FPushCufIter" -> ICall (FPushCufIter (intofiarg arg1, iterofiarg arg2))
 | "FIsParamByRef" -> ICall (FIsParamByRef (intofiarg arg1, fpasshintof arg2))

 (* instruct_base *)
 | "BaseNC" -> IBase (BaseNC (intofiarg arg1, memberopmodeofiarg arg2))
 | "BaseNL" -> IBase (BaseNL (localidofiarg arg1, memberopmodeofiarg arg2))
 | "BaseGC" -> IBase (BaseGC (intofiarg arg1, memberopmodeofiarg arg2))
 | "BaseGL" -> IBase (BaseGL (localidofiarg arg1, memberopmodeofiarg arg2))
 | "BaseC" -> IBase(BaseC (intofiarg arg1, memberopmodeofiarg arg2))
 | "BaseR" -> IBase(BaseR (intofiarg arg1, memberopmodeofiarg arg2))
 | "BaseL" -> IBase (BaseL (localidofiarg arg1, memberopmodeofiarg arg2))
 | "Dim" -> IBase (Dim (memberopmodeofiarg arg1, memberkeyofiarg arg2))

 (* instruct_final *)
 | "VGetM" -> IFinal (VGetM (intofiarg arg1, memberkeyofiarg arg2))
 | "SetM" -> IFinal (SetM (intofiarg arg1, memberkeyofiarg arg2))
 | "BindM" -> IFinal  (BindM (intofiarg arg1, memberkeyofiarg arg2))
 | "UnsetM" -> IFinal (UnsetM (intofiarg arg1, memberkeyofiarg arg2))
 | "SetWithRefLML" ->
    IFinal (SetWithRefLML (localidofiarg arg1, localidofiarg arg2))

 (* instruct_iterator *)
 | "IterBreak" ->
    IIterator (IterBreak (labelofiarg arg1, listofiteratorsofiarg arg2))
 | "LIterFree" -> IIterator(LIterFree (Iterator.Id (intofiarg arg1), localidofiarg arg2))

 (* instruct_misc *)
 | "StaticLocCheck" ->
    IMisc (StaticLocCheck (localidofiarg arg1, stringofiarg arg2))
 | "StaticLocDef" ->
    IMisc (StaticLocDef (localidofiarg arg1, stringofiarg arg2))
 | "StaticLocInit" ->
    IMisc (StaticLocInit (localidofiarg arg1, stringofiarg arg2))
 | "CreateCl" -> IMisc (CreateCl (intofiarg arg1, intofiarg arg2))
 | "AssertRATL" -> IMisc (AssertRATL (localidofiarg arg1, stringofiarg arg2))
 | "AssertRATStk" -> IMisc (AssertRATStk (intofiarg arg1, stringofiarg arg2))
 | "Silence" -> IMisc (Silence (localidofiarg arg1, opsilenceofiarg arg2))

 | "YieldFromDelegate" ->
   IGenDelegation (YieldFromDelegate (iterofiarg arg1, labelofiarg arg2))
 | "ContUnsetDelegate" ->
   IGenDelegation (ContUnsetDelegate (freeiteratorofiarg arg1, iterofiarg arg2))

 (* instruct_misc *)
 | "MemoGet" ->
   let l = memoargofiarg arg2 in
   let lab =
     (match arg1 with
      | IAId l -> makelabel l
      | _ -> report_error "bad label")
   in
   IMisc (MemoGet(lab, l))

 | "AliasCls" ->
   IIncludeEvalDefine (AliasCls(stringofiarg arg1, stringofiarg arg2))

 | _ -> failwith ("NYI binary: " ^ s)

let maketernaryinst s arg1 arg2 arg3 =
 match s with
 (* instruct_control_flow *)
 | "Switch" ->
    IContFlow(Switch (switchkindofiarg arg1, intofiarg arg2,
                                             listoflabelsofiarg arg3))

 (* instruct_call *)
 | "FPushFuncU" ->
    ICall(FPushFuncU (intofiarg arg1, function_id_of_iarg arg2, stringofiarg arg3))
 | "FPushObjMethod" ->
    ICall(FPushObjMethod (intofiarg arg1, nullflavorofiarg arg2, listofintofiarg arg3))
 | "FPushClsMethod" -> ICall (FPushClsMethod (intofiarg arg1, intofiarg arg2, listofintofiarg arg3))
 | "FPushObjMethodD" -> ICall(FPushObjMethodD
                    (intofiarg arg1, method_id_of_iarg arg2, nullflavorofiarg arg3))
 | "FPushClsMethodD" -> ICall(FPushClsMethodD
                                (intofiarg arg1, method_id_of_iarg arg2, class_id_of_iarg arg3))
 | "FPushClsMethodSD" -> ICall(FPushClsMethodSD
                                 (intofiarg arg1, specialclsrefofiarg arg2, method_id_of_iarg arg3))
 | "FIsParamByRefCufIter" ->
    ICall (FIsParamByRefCufIter (intofiarg arg1, fpasshintof arg2, iterofiarg arg3))
 | "FCallAwait" ->
    ICall(FCallAwait (intofiarg arg1,
      class_id_of_iarg arg2, function_id_of_iarg arg3))
 | "FCallBuiltin" ->
    ICall(FCallBuiltin (intofiarg arg1, intofiarg arg2, stringofiarg arg3))
 | "FHandleRefMismatch" ->
    ICall (FHandleRefMismatch (intofiarg arg1, fpasshintof arg2, stringofiarg arg3))

 (* instruct_base *)
 | "BaseSC" -> IBase(BaseSC (intofiarg arg1, intofiarg arg2, memberopmodeofiarg arg3))
 | "BaseSL" -> IBase (BaseSL (localidofiarg arg1, intofiarg arg2, memberopmodeofiarg arg3))

(* instruct_final *)
 | "QueryM" ->
    IFinal (QueryM (intofiarg arg1, queryopofiarg arg2, memberkeyofiarg arg3))
 | "IncDecM" ->
    IFinal(IncDecM (intofiarg arg1, incdecopofiarg arg2, memberkeyofiarg arg3))
 | "SetOpM" ->
    IFinal(SetOpM (intofiarg arg1, eqopofiarg arg2, memberkeyofiarg arg3))

(* instruct_iterator *)
 | "IterInit" -> IIterator(IterInit (iterofiarg arg1,
                                         labelofiarg arg2, localidofiarg arg3))
 | "WIterInit" -> IIterator(WIterInit (iterofiarg arg1,
                                         labelofiarg arg2, localidofiarg arg3))
 | "MIterInit" -> IIterator(MIterInit (iterofiarg arg1,
                                         labelofiarg arg2, localidofiarg arg3))
 | "IterNext" -> IIterator(IterNext (iterofiarg arg1,
                                         labelofiarg arg2, localidofiarg arg3))
 | "WIterNext" -> IIterator(WIterNext (iterofiarg arg1,
                                         labelofiarg arg2, localidofiarg arg3))
 | "MIterNext" -> IIterator(MIterNext (iterofiarg arg1,
                                         labelofiarg arg2, localidofiarg arg3))

 | _ -> failwith ("NYI ternary: " ^ s)

let makequaternaryinst s arg1 arg2 arg3 arg4 =
match s with
 | "IterInitK" -> IIterator(IterInitK(iterofiarg arg1,
                     labelofiarg arg2, localidofiarg arg3, localidofiarg arg4))
 | "WIterInitK" -> IIterator(WIterInitK(iterofiarg arg1,
                     labelofiarg arg2, localidofiarg arg3, localidofiarg arg4))
 | "MIterInitK" -> IIterator(MIterInitK(iterofiarg arg1,
                     labelofiarg arg2, localidofiarg arg3, localidofiarg arg4))
 | "IterNextK" -> IIterator(IterNextK(iterofiarg arg1,
                     labelofiarg arg2, localidofiarg arg3, localidofiarg arg4))
 | "WIterNextK" -> IIterator(WIterNextK(iterofiarg arg1,
                     labelofiarg arg2, localidofiarg arg3, localidofiarg arg4))
 | "MIterNextK" -> IIterator(MIterNextK(iterofiarg arg1,
                     labelofiarg arg2, localidofiarg arg3, localidofiarg arg4))
 | "LIterInit" -> IIterator(LIterInit (iterofiarg arg1, localidofiarg arg2,
                                       labelofiarg arg3, localidofiarg arg4))
 | "LIterNext" -> IIterator(LIterNext (iterofiarg arg1, localidofiarg arg2,
                                       labelofiarg arg3, localidofiarg arg4))
 | _ -> failwith ("NYI quaternary: " ^ s)

let makequinaryinst s arg1 arg2 arg3 arg4 arg5 =
match s with
  | "LIterInitK" -> IIterator(LIterInitK (iterofiarg arg1, localidofiarg arg2,
                                          labelofiarg arg3, localidofiarg arg4,
                                          localidofiarg arg5))
  | "LIterNextK" -> IIterator(LIterNextK (iterofiarg arg1, localidofiarg arg2,
                                          labelofiarg arg3, localidofiarg arg4,
                                          localidofiarg arg5))
  | "FCall" ->
    ICall(FCall (intofiarg arg1, has_unpack_of_iarg arg2, intofiarg arg3,
      class_id_of_iarg arg4, function_id_of_iarg arg5))
  | _ -> failwith ("NYI quinary: " ^ s)
