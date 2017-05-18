(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Hhbc_ast
open Parsing
(*
  TODO: proper error handling...
*)
let report_error s = (print_endline s; raise Parse_error)

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
 in
 pkv l []

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
           | 'd' -> Scanf.bscanf c "d:%g;" (fun f -> Some (Typed_value.Float f))
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
let my_unescape s =
  let num_chars = String.length s in
  let buf = Buffer.create num_chars in
  let rec copy_from i =
    if i = num_chars then Buffer.contents buf
    else match s.[i] with
      | '\\' -> (match s.[i+1] with
                  | '\\' -> Buffer.add_char buf '\\'
                  | 'r' -> Buffer.add_char buf '\r'
                  | 'n' -> Buffer.add_char buf '\n'
                  | 't' -> Buffer.add_char buf '\t'
                  | '?' -> Buffer.add_char buf '?'
                  | '"' -> Buffer.add_char buf '"'
                  | _ -> report_error "bad escaped character in triplequoted"
                  );
                  copy_from (i+2)
      | c -> Buffer.add_char buf c ; copy_from (i+1) in
  copy_from 0

let attribute_from_string s =
  match parse_attribute (Scanf.Scanning.from_string (my_unescape s)) with
   | Some a -> a
   | None -> report_error "attribute from string failed"

type decl =
 | Main_decl of Hhas_body.t
 | Fun_decl of Hhas_function.t
 | Class_decl of Hhas_class.t
 | Data_decl of (int*Typed_value.t)
 | Alias_decl of Hhas_typedef.t

(* Fixing up code streams to deal with data declarations. This is pretty ugly *)
let rec lookupdd n dds =
match dds with
 | [] -> None
 | (num,args) :: rest -> if n=num then Some args
                              else lookupdd n rest

let rewriteinstr dds i =
match i with
 | ILitConst (Array (Typed_value.Int n)) -> (match lookupdd (Int64.to_int n) dds with
                    | Some (Typed_value.Array args) ->
                              ILitConst (Array(Typed_value.Array args))
                    | Some _ -> report_error "expected an array in data decl"
                    | None -> report_error "array name missing")
 | ILitConst (Vec (Typed_value.Int n)) -> (match lookupdd (Int64.to_int n) dds with
                    | Some (Typed_value.Vec args) ->
                                  ILitConst (Vec(Typed_value.Vec args))
                    | Some _ -> report_error "expected a vector in data decl"
                    | None -> report_error "vec name missing")
 | ILitConst (Dict (Typed_value.Int n)) -> (match lookupdd (Int64.to_int n) dds with
                    | Some (Typed_value.Dict args) ->
                                  ILitConst (Dict(Typed_value.Dict args))
                    | Some _ -> report_error "expected dictionary in data decl"
                    | None -> report_error "dict name missing")
 | ILitConst (Keyset (Typed_value.Int n)) -> (match lookupdd (Int64.to_int n) dds with
                    | Some (Typed_value.Keyset args) ->
                                ILitConst (Keyset(Typed_value.Keyset args))
                    | Some _ -> report_error "expected a keyset in data decl"
                    | None -> report_error "keyset name missing")
 | _ -> i

let rewrite_instrs_in_body dds body =
  let instrs = Hhas_body.instrs body in
  let newinstrs = Instruction_sequence.InstrSeq.map instrs (rewriteinstr dds) in
  Hhas_body.with_instrs body newinstrs

let rewritefundecl dds fd =
  Hhas_function.with_body fd
    (rewrite_instrs_in_body dds (Hhas_function.body fd))

let rewritemethoddecl dds md =
  Hhas_method.with_body md
    (rewrite_instrs_in_body dds (Hhas_method.body md))

let rewriteclassdecl dds cd =
  Hhas_class.with_methods cd
                    (List.map (rewritemethoddecl dds) (Hhas_class.methods cd))

let rec splitdecllist ds funs classes optmain datadecls aliasdecls =
 match ds with
  | [] -> (match optmain with
           | None -> report_error "missing main"
           | Some m -> (datadecls, Hhas_program.make funs classes aliasdecls m))
  | Main_decl md :: rest ->
   (match optmain with
    | None -> splitdecllist rest funs classes
                  (Some (rewrite_instrs_in_body datadecls md))
                  datadecls aliasdecls
    | Some _ -> report_error "duplicate main")
  (* We rewrite functions according to current datadecls.
     Order is a bit dubious, but it seems to work ok when decls are at the top,
     which they are *)
  | Fun_decl fd :: rest -> splitdecllist rest
                                         (rewritefundecl datadecls fd :: funs)
                                         classes optmain datadecls aliasdecls
  | Class_decl cd :: rest -> splitdecllist rest funs
                                     (rewriteclassdecl datadecls cd :: classes)
                                     optmain datadecls aliasdecls
  | Data_decl dd :: rest -> splitdecllist rest funs classes optmain
                            (dd::datadecls) aliasdecls
  | Alias_decl ad :: rest -> splitdecllist rest funs classes optmain datadecls
                             (ad :: aliasdecls)

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
             else report_error "bad label"
    | _ -> report_error "bad label"
let makelabelinst s = ILabel (makelabel s)

(* TODO: replace stupidly big match with a hash table. Bootcampable? *)
let makenullaryinst s =
 match s with
 (* instruct_basic *)
 | "Nop" -> IBasic (Nop)
 | "EntryNop"-> IBasic (EntryNop)
 | "PopA" -> IBasic (PopA)
 | "PopC" -> IBasic (PopC)
 | "PopV" -> IBasic (PopV)
 | "PopR" -> IBasic (PopR)
 | "PopU" -> IBasic (PopU)
 | "Dup"  -> IBasic (Dup)
 | "Box"  -> IBasic (Box)
 | "Unbox" -> IBasic (Unbox)
 | "BoxR"  -> IBasic (BoxR)
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
 | "MapAddElemC"-> ILitConst (MapAddElemC)
 | "File"-> ILitConst (File)
 | "Dir"-> ILitConst (Dir)
 | "Method"-> ILitConst (Method)
 | "NameA"-> ILitConst (NameA)

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

 (* instruct_call *)
 | "CufSafeArray" -> ICall(CufSafeArray)
 | "CufSafeReturn" -> ICall(CufSafeReturn)
 | "FCallArray" -> ICall(FCallArray)

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
 | "VerifyRetTypeC" -> IMisc(VerifyRetTypeC)
 | "VerifyRetTypeV" -> IMisc(VerifyRetTypeV)
 | "NativeImpl" -> IMisc(NativeImpl)
 | "AKExists" -> IMisc(AKExists)
 | "Idx" -> IMisc(Idx)
 | "ArrayIdx" -> IMisc(ArrayIdx)
 | "BreakTraceHint" -> IMisc(BreakTraceHint)
 | "VarEnvDynCall" -> IMisc(VarEnvDynCall)
 | "IsUninit" -> IMisc(IsUninit)
 | "CGetCUNop" -> IMisc(CGetCUNop)
 | "UGetCUNop" -> IMisc(UGetCUNop)
 | "IsMemoType" -> IMisc(IsMemoType)
 | "MaybeMemoType" -> IMisc(MaybeMemoType)

 | _ -> IComment ("NYI nullary: " ^ s)

type iarg =
  | IAInt64 of int64
  | IAString of string
  | IAId of string
  | IADouble of string (* seems we don't parse these *)
  | IAArrayno of int
  | IAMemberkey of string*iarg (* these are not seriously recursive *)
  | IAArglist of iarg list

let class_id_of_iarg arg =
  match arg with
  | IAString s -> Hhbc_id.Class.from_raw_string s
  | _ -> report_error "expected quoted class identifier"

let class_id_of_int_iarg arg =
  match arg with
  | IAInt64 i -> Hhbc_id.Class.from_raw_string (Int64.to_string i)
  | _ -> report_error "expected quoted class identifier"

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
               | "Scalar" -> OpScalar
               | "Keyset" -> OpKeyset
               | "Dict" -> OpDict
               | "Vec" -> OpVec
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
               | _ -> report_error "bad eqop")
  | _ -> report_error "wrong kind of eqop arg"

let queryopofiarg arg =
match arg with
  | IAId s -> (match s with
                | "CGet" -> QueryOp.CGet
                | "Isset" -> QueryOp.Isset
                | "Empty" -> QueryOp.Empty
                | _ -> report_error "unknown queryop")
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
       | _ -> report_error "unknown memberkey string"
    )
  | IAId s' -> if s'="W" then MemberKey.W else report_error "bad memberkey"
  | _ -> report_error "bad memberkey"

let incdecopofiarg arg =
match arg with
 | IAId s -> (match s with
              | "PreInc" -> PreInc
              | "PostInc" -> PostInc
              | "PreDec" -> PreDec
              | "PostDec" -> PostDec
              | "PreIncO" -> PreIncO
              | "PostIncO" -> PostIncO
              | "PreDecO" -> PreDecO
              | "PostDecO" -> PostDecO
              | _ -> report_error "bad incdecop")
  | _ -> report_error "wrong kind of incdecop arg"

let paramidofiarg arg =
match arg with
 | IAId s -> Param_named s
 | IAInt64 n -> Param_unnamed (Int64.to_int n)
 | _ -> report_error "bad param id to instruction"

let barethisopofiarg arg =
match arg with
 | IAId "Notice" -> Notice
 | IAId "NoNotice" -> NoNotice
 | _ -> report_error "bad bare this op"

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

let memberopmodeofiarg arg =
match stringofiarg arg with
 | "None" -> MemberOpMode.ModeNone
 | "Warn" -> MemberOpMode.Warn
 | "Define" -> MemberOpMode.Define
 | "Unset" -> MemberOpMode.Unset
 | _ -> report_error ("bad member op mode" ^ stringofiarg arg)

 let listofiteratorsofiarg arg =
 match arg with
 | IAArglist l -> List.map iterofiarg l
 | _ -> report_error "bad list of iterators"

let listoflabelsofiarg arg =
 match arg with
 | IAArglist l -> List.map labelofiarg l
 | _ -> report_error "bad list of labels"

let opsilenceofiarg arg =
 match stringofiarg arg with
  | "Start" -> Start
  | "End" -> End
  | _ -> report_error "bad op_silence"

let switchkindofiarg arg =
match stringofiarg arg with
 | "Bounded" -> Bounded
 | "Unbounded" -> Unbounded
 | _ -> report_error "bad switch kind"

let doubleofiarg arg =
 match arg with
  | IADouble sd -> sd
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
       | IAArrayno n -> ILitConst (Array (Typed_value.Int (Int64.of_int n)))
       | _ -> report_error "bad array lit cst")
                                (* Q: where's the real data?
                                   A: it's in the adata declaration, which
                                    we'll splice in here in a later pass.
                                *)
   | "Vec" -> (match arg with
       | IAArrayno n -> ILitConst (Vec (Typed_value.Int (Int64.of_int n)))
       | _ -> report_error "bad vec lit cst")
   | "Dict" -> (match arg with
       | IAArrayno n -> ILitConst (Dict (Typed_value.Int (Int64.of_int n)))
       | _ -> report_error "bad dict lit cst")
   | "Keyset" -> (match arg with
       | IAArrayno n -> ILitConst (Keyset (Typed_value.Int (Int64.of_int n)))
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
   | "NewMIArray" -> (match arg with
       | IAInt64 n -> ILitConst (NewMIArray (Int64.to_int n))
       | _ -> report_error "bad array size")
   | "NewMSArray" -> ILitConst (NewMSArray (intofiarg arg))
   | "NewPackedArray" -> (match arg with
       | IAInt64 n -> ILitConst (NewPackedArray (Int64.to_int n))
       | _ -> report_error "bad array size")
   | "NewStructArray" ->
        (ILitConst(NewStructArray (listofshapefieldsofiarg arg)))
   | "NewVecArray" -> (match arg with
       | IAInt64 n -> ILitConst (NewVecArray (Int64.to_int n))
       | _ -> report_error "bad array size")
   | "NewKeysetArray" -> (match arg with
       | IAInt64 n -> ILitConst (NewKeysetArray (Int64.to_int n))
       | _ -> report_error "bad array size")
   | "NewCol" -> (match arg with
       | IAInt64 n -> ILitConst (NewCol (Int64.to_int n))
       | _ -> report_error "bad collection type")
   | "ColFromArray" -> (match arg with
       | IAInt64 n -> ILitConst (ColFromArray (Int64.to_int n))
       | _ -> report_error "bad collection type")
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
   | "InstanceOfD"-> (match arg with
       | IAString sa -> IOp (InstanceOfD (Hhbc_id.Class.from_raw_string sa))
       | _ -> report_error "bad InstanceOfD arg")
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

  (* instruct_get *)
   | "CGetL" -> IGet (CGetL (localidofiarg arg))
   | "CGetQuietL" -> IGet (CGetQuietL (localidofiarg arg))
   | "CGetL2" -> IGet (CGetL2 (localidofiarg arg))
   | "CGetL3" -> IGet (CGetL3 (localidofiarg arg))
   | "CUGetL" -> IGet (CUGetL (localidofiarg arg))
   | "PushL" -> IGet (PushL (localidofiarg arg))
   | "CGetS" -> IGet (CGetS (intofiarg arg))
   | "VGetL" -> IGet (VGetL (localidofiarg arg))
   | "VgetS" -> IGet (VGetS (intofiarg arg))
   | "ClsRefGetC" -> IGet (ClsRefGetC (intofiarg arg))

   (*instruct_isset *)
   | "IssetL" -> IIsset (IssetL (localidofiarg arg))
   | "IssetS" -> IIsset (IssetS (intofiarg arg))
   | "EmptyL" -> IIsset (EmptyL (localidofiarg arg))
   | "EmptyS" -> IIsset (EmptyS (intofiarg arg))
   | "IsTypeC" -> IIsset (IsTypeC (typeopofiarg arg))

   (* instruct_mutator *)
   | "SetL" -> IMutator (SetL (localidofiarg arg))
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
   | "FPushFunc" -> ICall(FPushFunc (intofiarg arg))
   | "FPushCuf" -> ICall(FPushCuf (intofiarg arg))
   | "FPushCufF" -> ICall(FPushCufF (intofiarg arg))
   | "FPushCufSafe" -> ICall(FPushCufSafe (intofiarg arg))
   | "FPassC" -> ICall(FPassC (intofiarg arg))
   | "FPassCW" -> ICall(FPassCW (intofiarg arg))
   | "FPassCE" -> ICall(FPassCE (intofiarg arg))
   | "FPassV" -> ICall(FPassV (intofiarg arg))
   | "FPassVNop" -> ICall(FPassVNop (intofiarg arg))
   | "FPassR" -> ICall(FPassR (intofiarg arg))
   | "FPassN" -> ICall(FPassN (intofiarg arg))
   | "FPassG" -> ICall(FPassG (intofiarg arg))
   | "FCall" -> ICall(FCall (intofiarg arg))
   | "FCallUnpack" -> ICall(FCallUnpack (intofiarg arg))

   (* instruct_base *)
   | "BaseC" -> IBase(BaseC (intofiarg arg))
   | "BaseR" -> IBase(BaseR (intofiarg arg))

   (* instruct_final *)
   | "SetWithRefRML" -> IFinal(SetWithRefRML(localidofiarg arg))

   (* instruct_iterator *)
   | "IterFree" -> IIterator(IterFree (Iterator.Id (intofiarg arg)))
   | "MIterFree" ->IIterator(MIterFree (Iterator.Id (intofiarg arg)))
   | "CIterFree" ->IIterator(CIterFree (Iterator.Id (intofiarg arg)))

   (* instruct_include_eval_define *)
   | "DefFunc" -> IIncludeEvalDefine(DefFunc (function_id_of_iarg arg))
   | "DefCls" -> IIncludeEvalDefine(DefCls (class_id_of_int_iarg arg))
      (* TODO: Mismatch - that should be an integer, not a string *)
   | "DefClsNop" -> IIncludeEvalDefine(DefClsNop (class_id_of_int_iarg arg))
   | "DefCns" -> IIncludeEvalDefine(DefCns (const_id_of_iarg arg))
   | "DefTypeAlias" ->
     IIncludeEvalDefine(DefTypeAlias(class_id_of_int_iarg arg))
     (* TODO: Mismatch here too *)

   (* instruct_misc *)
   | "BareThis" -> IMisc(BareThis (barethisopofiarg arg))
   | "InitThisLoc" -> IMisc(InitThisLoc (localidofiarg arg))
   | "OODeclExists" -> IMisc(OODeclExists (classkindofiarg arg))
   | "VerifyParamType" -> IMisc(VerifyParamType (paramidofiarg arg))
   | "Self" -> IMisc(Self (intofiarg arg))
   | "Parent" -> IMisc(Parent (intofiarg arg))
   | "LateBoundCls" -> IMisc(LateBoundCls (intofiarg arg))
   | "ClsRefName" -> IMisc(ClsRefName (intofiarg arg))
   | "GetMemoKeyL" -> IMisc(GetMemoKeyL (localidofiarg arg))

   (* Note: The TryCatch/TryFault instructions don't show up here because the
      textual bytecode format represents them using directives and braces rather
      than instructions
   *)
   | _ -> IComment ("NYI unary: " ^ s)

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
 | "FPushFuncD" -> ICall (FPushFuncD (intofiarg arg1, function_id_of_iarg arg2))
 | "FPushObjMethod" ->
    ICall(FPushObjMethod (intofiarg arg1, nullflavorofiarg arg2))
 | "FPushClsMethod" -> ICall (FPushClsMethod (intofiarg arg1, intofiarg arg2))
 | "FPushClsMethodF" -> ICall (FPushClsMethodF (intofiarg arg1, intofiarg arg2))
 | "FPushCtor" -> ICall (FPushCtor (intofiarg arg1, intofiarg arg2))
 | "FPushCtorD" -> ICall (FPushCtorD (intofiarg arg1, class_id_of_iarg arg2))
 | "FPushCtorI" -> ICall (FPushCtorI (intofiarg arg1, intofiarg arg2))
 | "DecodeCufIter" -> ICall (DecodeCufIter (intofiarg arg1, labelofiarg arg2))
 | "FPushCufIter" -> ICall (FPushCufIter (intofiarg arg1, iterofiarg arg2))
 | "FPassL" -> ICall (FPassL (intofiarg arg1, localidofiarg arg2))
 | "FPassS" -> ICall(FPassS (intofiarg arg1, intofiarg arg2))
 (* instruct_base *)
 | "BaseNC" -> IBase (BaseNC (intofiarg arg1, memberopmodeofiarg arg2))
 | "BaseNL" -> IBase (BaseNL (localidofiarg arg1, memberopmodeofiarg arg2))
 | "FPassBaseNC" -> IBase (FPassBaseNC (intofiarg arg1, intofiarg arg2))
 | "FPassBaseNL" -> IBase (FPassBaseNL (intofiarg arg1, localidofiarg arg2))
 | "BaseGC" -> IBase (BaseGC (intofiarg arg1, memberopmodeofiarg arg2))
 | "BaseGL" -> IBase (BaseGL (localidofiarg arg1, memberopmodeofiarg arg2))
 | "FPassBaseGC" -> IBase (FPassBaseGC (intofiarg arg1, intofiarg arg2))
 | "FPassBaseGL" -> IBase (FPassBaseGL (intofiarg arg1, localidofiarg arg2))
 | "BaseSC" -> IBase(BaseSC (intofiarg arg1, intofiarg arg2))
 | "BaseSL" -> IBase (BaseSL (localidofiarg arg1, intofiarg arg2))
 | "BaseL" -> IBase (BaseL (localidofiarg arg1, memberopmodeofiarg arg2))
 | "FPassBaseL" -> IBase (FPassBaseL (intofiarg arg1, localidofiarg arg2))
 | "Dim" -> IBase (Dim (memberopmodeofiarg arg1, memberkeyofiarg arg2))
 | "FPassDim" -> IBase (FPassDim (intofiarg arg1, memberkeyofiarg arg2))

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

 (* instruct_misc *)
 | "StaticLoc" -> IMisc (StaticLoc (localidofiarg arg1, stringofiarg arg2))
 | "StaticLocInit" ->
    IMisc (StaticLocInit (localidofiarg arg1, stringofiarg arg2))
 | "IncStat" -> IMisc (IncStat (intofiarg arg1, intofiarg arg2))
 | "CreateCl" -> IMisc (CreateCl (intofiarg arg1, intofiarg arg2))
 | "AssertRATL" -> IMisc (AssertRATL (localidofiarg arg1, stringofiarg arg2))
 | "AssertRATStk" -> IMisc (AssertRATStk (intofiarg arg1, stringofiarg arg2))
 | "Silence" -> IMisc (Silence (localidofiarg arg1, opsilenceofiarg arg2))
 | _ -> IComment ("NYI binary: " ^ s)

let maketernaryinst s arg1 arg2 arg3 =
 match s with
 (* instruct_control_flow *)
 | "Switch" ->
    IContFlow(Switch (switchkindofiarg arg1, intofiarg arg2,
                                             listoflabelsofiarg arg3))

 (* instruct_call *)
 | "FPushFuncU" ->
    ICall(FPushFuncU (intofiarg arg1, function_id_of_iarg arg2, stringofiarg arg3))
 | "FPushObjMethodD" -> ICall(FPushObjMethodD
                    (intofiarg arg1, method_id_of_iarg arg2, nullflavorofiarg arg3))
 | "FPushClsMethodD" -> ICall(FPushClsMethodD
                    (intofiarg arg1, method_id_of_iarg arg2, class_id_of_iarg arg3))
 | "FCallD" ->
    ICall(FCallD (intofiarg arg1,
      class_id_of_iarg arg2, function_id_of_iarg arg3))
 | "FCallAwait" ->
    ICall(FCallAwait (intofiarg arg1,
      class_id_of_iarg arg2, function_id_of_iarg arg3))
 | "FCallBuiltin" ->
    ICall(FCallBuiltin (intofiarg arg1, intofiarg arg2, stringofiarg arg3))

(* instruct_final *)
 | "QueryM" ->
    IFinal (QueryM (intofiarg arg1, queryopofiarg arg2, memberkeyofiarg arg3))
 | "FPassM" ->
    IFinal(FPassM (intofiarg arg1, intofiarg arg2, memberkeyofiarg arg3))
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

(* instruct_misc *)
 | "MemoSet" ->
    IMisc(MemoSet(intofiarg arg1, localidofiarg arg2, intofiarg arg3 + 1))
 | "MemoGet" ->
    IMisc(MemoGet(intofiarg arg1, localidofiarg arg2, intofiarg arg3 + 1))
 | _ -> IComment ("NYI ternary: " ^ s)

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
 | _ -> IComment ("NYI quaternary: " ^ s)
