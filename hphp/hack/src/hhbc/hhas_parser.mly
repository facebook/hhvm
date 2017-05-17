%{
  (**
   * Copyright (c) 2017, Facebook, Inc.
   * All rights reserved.
   *
   * This source code is licensed under the BSD-style license found in the
   * LICENSE file in the "hack" directory of this source tree. An additional grant
   * of patent rights can be found in the PATENTS file in the same directory.
   *
  *)

open Hhas_parser_actions
%}

%token <int64> INT
%token <string> STRING
%token <string> ESCAPEDSTRING
%token <string> ID
%token <string> VNAME
%token <string> LVNAME
%token <string> DOUBLE
%token <string> TRIPLEQUOTEDSTRING
%token FUNCTIONDIRECTIVE MAINDIRECTIVE CLASSDIRECTIVE DECLVARSDIRECTIVE
%token DATADECLDIRECTIVE NUMITERSDIRECTIVE NUMCLSREFSLOTSDIRECTIVE
%token METHODDIRECTIVE CONSTDIRECTIVE ENUMTYDIRECTIVE USESDIRECTIVE
%token TRYFAULTDIRECTIVE PROPERTYDIRECTIVE FILEPATHDIRECTIVE
%token LANGLE
%token RANGLE
%token LPAR
%token RPAR
%token LBRACE LBRACK
%token RBRACE RBRACK
%token COMMA SEMI
%token COLON EQUALS
%token EOF DOLLAR NEWLINE AMPERSAND
%token QUOTE ATAUNDERSCORE UNDERSCORE
%token PLUS
%token TRYCATCHDIRECTIVE
%token TRYDIRECTIVE
%token CATCHDIRECTIVE
%token ALIASDIRECTIVE

%start program
%type <((int*Typed_value.t) list) * Hhas_program.t> program
%%
program:
    nl decllist nl EOF { splitdecllist $2 [] [] None [] []}
;
decl:
    | maindecl {Main_decl $1}
    | fundecl {Fun_decl $1}
    | classdecl {Class_decl $1}
    | datadecl {Data_decl $1}
    | aliasdecl {Alias_decl $1}
;
aliasdecl:
    | ALIASDIRECTIVE ID EQUALS aliastypeinfo SEMI nl
      {Hhas_typedef.make (Hhbc_id.Class.from_raw_string $2)  $4}
;
maindecl:
    | MAINDIRECTIVE LBRACE nl numiters declvars nl functionbody RBRACE nl
      {Hhas_body.make $7(*instrs*)
        $5(*declvars*) $4(*numiters*)
        0(*numclsrefslots*) [](*params*) None(*return type*)
      (* TODO: This is currently wrong, as it includes defcls and type alias
         instructions in the body. We strip the former later *)}
;
numiters:
    | /* empty */ {0}
    | NUMITERSDIRECTIVE INT SEMI nl {Int64.to_int $2}
;
datadecl:
    | DATADECLDIRECTIVE ID EQUALS nonclassattribute SEMI nl
      {if $2.[0] = 'A' && $2.[1]='_' then
        let num = int_of_string (String.sub $2 2 (String.length $2 - 2)) in
        (num,$4) (* should just be a typed-value now *)
       else report_error "datadecl variable should be A_n"
        }
;
nonclassattribute:
    | TRIPLEQUOTEDSTRING {attribute_from_string $1}
;
fundecl:
    | FUNCTIONDIRECTIVE functionattributes typeinfooption ID fparams
      functionflags LBRACE nl numiters numclsrefslots declvars functionbody RBRACE
        {Hhas_function.make $2(*attributes*)
           (Hhbc_id.Function.from_raw_string $4)(*name*)
             (Hhas_body.make $12(*instrs*)
             $11(*declvars*)
             $9 (*numiters*) $10 (*numclsrefslots *)
             $5(*params*) $3(*typeinfo*))
            (isasync $6)
            (isgenerator $6)
            (ispairgenerator $6)  }
;
nl:
    | /* empty */ {()}
    | NEWLINE nl  {()}
;
declvars:
    | /* empty */ {[]}
    | DECLVARSDIRECTIVE declvarlist SEMI nl {$2}
;
declvarlist:
    | vname  {[$1]}
    | vname declvarlist {$1 :: $2}
;
functionflags:
    | /* empty */ {[]}
    | ID {[$1]}
    | ID ID {[$1; $2]}
    | ID ID ID {[$1; $2; $3]}
;
methodflags:
    | idlist {$1}
;
fparams:
    | LPAR param_list RPAR {$2}
;
param_list:
    | /* empty */ { [] }
    | param { [$1]}
    | param COMMA param_list {$1 :: $3}
;
possibleampersand:
   | /* empty */ {false}
   | AMPERSAND   {true}
;
paramdefaultvalueopt:
  | /* empty */  {None}
  | EQUALS ID LPAR TRIPLEQUOTEDSTRING RPAR
   {Some (makelabel $2, (Pos.none, Ast.String(Pos.none,$4)))}
;
param:
    | typeinfooption possibleampersand vname paramdefaultvalueopt
      {Hhas_param.make
        $3 (*name *)
        $2(*isreference*)
        $1 (* type info option *)
        $4(*defaultvalue*)}
;
vname:
    | DOLLAR ID {"$" ^ $2}
    | DOLLAR INT ID {"$" ^ (Int64.to_string $2) ^ $3 }
;
classdecl:
    | CLASSDIRECTIVE classattributes ID extendsimplements LBRACE
     nl classuses classenumty classtypeconstants
     classproperties methods nl RBRACE nl
        {Hhas_class.make (fst $2)(*attributes*)
          (fst $4) (*base*)
          (snd $4) (*implements*)
          (Hhbc_id.Class.from_raw_string $3)(*name*)
        (List.mem "final" (snd $2))(*isfinal*)
        (List.mem "abstract" (snd $2))(*isabstract*)
        (List.mem "interface" (snd $2))(*isinterface*)
        (List.mem "trait" (snd $2))(*istrait*)
        false(*isxhp*)
        $7(*uses*)
        $8(*enumtype*)
        $11(*methods*) $10(*properties*) (fst $9) (*constants*) (snd $9)(*typeconstants*)}
;
methods:
 | /* empty */ {[]}
 | methoddecl methods {$1 :: $2}
;
methodname:
 | ID {$1}
 | INT ID {if Int64.to_int $1 = 86 then "86" ^ $2 else "bad 86xxx"}
;
methoddecl:
 | METHODDIRECTIVE classattributes typeinfooption methodname fparams idlist LBRACE nl
    numclsrefslots numiters declvars functionbody RBRACE nl
  {Hhas_method.make
    (fst $2) (* attributes *)
    (List.mem "protected" (snd $2))
    (List.mem "public" (snd $2))
    (List.mem "private" (snd $2))
    (List.mem "static" (snd $2))
    (List.mem "final" (snd $2))
    (List.mem "abstract" (snd $2))
    (Hhbc_id.Method.from_raw_string $4) (* name *)
    (Hhas_body.make
      $12 (* method instructions *)
      $11 (* declvars *)
      $10 (* numiters *)
      $9 (* num cls ref slots *)
      $5 (* params *)
      $3 (* return type *)
      )
    (List.mem "isAsync" $6)
    (List.mem "isGenerator" $6)
    (List.mem "isPairGenerator" $6)
    (List.mem "isClosureBody" $6)
  }
;
numclsrefslots:
 | /* empty */ {0}
 | NUMCLSREFSLOTSDIRECTIVE INT SEMI nl {Int64.to_int $2}
;
classproperties:
  | /* empty */ {[]}
  | classproperty NEWLINE classproperties {$1 :: $3}
;
idorvname:
  | ID {$1}
  | vname {$1}
;
classproperty:
  | PROPERTYDIRECTIVE propertyattributes idorvname EQUALS nl propertyvalue
  {Hhas_property.make
    (List.mem "private" $2)
    (List.mem "protected" $2)
    (List.mem "public" $2)
    (List.mem "static" $2)
    (List.mem "deep_init" $2)
    (Hhbc_id.Prop.from_raw_string $3) (*name *)
    $6 (*initial value *)
    None (* initializer instructions. already been emitted elsewhere *)
  }
;
propertyattributes:
  | /* empty */ {[]}
  | LBRACK idlist RBRACK {$2}
;
propertyvalue:
  | ID SEMI {if $1 = "uninit" then None else report_error "bad property value"}
  | TRIPLEQUOTEDSTRING SEMI
    {Some (attribute_from_string $1)}
;
classtypeconstants:
  | /* empty */ {([],[])}
  | CONSTDIRECTIVE ID EQUALS TRIPLEQUOTEDSTRING SEMI nl classtypeconstants
     {( (Hhas_constant.make $2 (attribute_from_string $4) None) :: (fst $7),
        snd $7)}
  | CONSTDIRECTIVE ID EQUALS ID SEMI nl classtypeconstants
     {if $4="uninit" then
     ( (Hhas_constant.make $2 Typed_value.Uninit None) :: (fst $7),
       snd $7)
      else report_error "bad class constant"}
  | CONSTDIRECTIVE ID ID EQUALS TRIPLEQUOTEDSTRING SEMI nl classtypeconstants
    {if $3 = "isType" then
     (fst $8,
      (Hhas_type_constant.make $2 (attribute_from_string $5)) :: (snd $8))
     else report_error "expected type constant"}
classconstants:
  | /* empty */ {[]}
  | classconstant NEWLINE classconstants {$1 :: $3}
;
classconstant:
  | CONSTDIRECTIVE ID EQUALS TRIPLEQUOTEDSTRING SEMI
     {Hhas_constant.make $2 (attribute_from_string $4) None}
  | CONSTDIRECTIVE ID EQUALS ID SEMI
     {if $4="uninit" then Hhas_constant.make $2 Typed_value.Uninit None
      else report_error "bad class constant"}
;
typeconstants:
  | /* empty */ {[]}
  | typeconstant NEWLINE typeconstants {$1 :: $3}
;
typeconstant:
  | CONSTDIRECTIVE ID ID EQUALS TRIPLEQUOTEDSTRING SEMI
    {if $3 = "isType" then
     Hhas_type_constant.make $2 (attribute_from_string $5)
     else report_error "expected type constant"}
;
classenumty:
  | /* empty */ {None}
  | ENUMTYDIRECTIVE enumtypeinfo SEMI nl {Some $2}
;
classuses:
  | /* empty */ {[]}
  | USESDIRECTIVE idlist SEMI nl {$2}
;
extendsimplements:
  | /* empty */ {(None,[])}
  | ID ID {if $1 = "extends"
           then (Some (Hhbc_id.Class.from_raw_string $2), [])
           else report_error "bad extends implements 1"}
  | ID LPAR idlist RPAR {if $1 = "implements"
                then (None, List.map Hhbc_id.Class.from_raw_string $3)
                else report_error "bad extends implements 2"}
  | ID ID ID LPAR idlist RPAR {if $1="extends" && $3 = "implements"
     then (Some (Hhbc_id.Class.from_raw_string $2),
        List.map Hhbc_id.Class.from_raw_string $5)
      else report_error "bad extends implements 3"}
idlist:
  | /* empty */ {[]}
  | ID idlist {$1 :: $2}
;
classattributes:
   | /* empty */ {([],[])}
   | LBRACK classattributelist RBRACK {$2}
;
classattributelist:
   | functionattribute {([$1],[])}
   | ID {([],[$1])}
   | functionattribute classattributelist {($1 :: fst $2, snd $2)}
   | ID classattributelist {(fst $2, $1 :: snd $2)}
;
functionattributes:
    /* empty */ { [] }
    | LBRACK functionattributelist RBRACK
      {$2}
;
functionattributelist:
    | functionattribute {[$1]}
    | functionattribute functionattributelist {$1 :: $2}
;
functionattribute:
    | STRING LPAR TRIPLEQUOTEDSTRING RPAR
      {match attribute_from_string $3 with
        | Typed_value.Array args -> Hhas_attribute.make $1 (unpack_key_values args)
        | _ -> report_error "function attributes should be arrays"
      }
;
typeinfooption:
    /* empty */ { None }
    | typeinfo  { Some $1}
;
quotestringoption:
    | ID  {if $1="N" then None else report_error "bad quote_string_option"}
    | STRING {Some $1}
;
typeconstraintflag:
    | ID {match $1 with
          | "nullable" -> Hhas_type_constraint.Nullable
          | "hh_type"  -> Hhas_type_constraint.HHType
          | "extended_hint" -> Hhas_type_constraint.ExtendedHint
          | "type_var" -> Hhas_type_constraint.TypeVar
          | "soft" -> Hhas_type_constraint.Soft
          | "type_constant" -> Hhas_type_constraint.TypeConstant
          | _ -> report_error "bad type constraint flag"}
;
flaglist:
    /* empty */ { [] }
    | typeconstraintflag flaglist {$1 :: $2}
;
aliastypeinfo:
    | LANGLE quotestringoption flaglist RANGLE
     {Hhas_type_info.make None (Hhas_type_constraint.make $2 $3)}
;
typeinfo:
    | LANGLE quotestringoption quotestringoption flaglist RANGLE
      {Hhas_type_info.make $2 (Hhas_type_constraint.make $3 $4)}
;
enumtypeinfo:
    | LANGLE quotestringoption flaglist RANGLE
      {Hhas_type_info.make $2 (Hhas_type_constraint.make None $3)}
;
functionbody:
    | /* empty */ {Instruction_sequence.empty}
    | instruction NEWLINE functionbody {
        Instruction_sequence.gather [Instruction_sequence.instr $1; $3]}
    | TRYFAULTDIRECTIVE ID LBRACE NEWLINE functionbody nl RBRACE nl
       functionbody
       { Instruction_sequence.gather [
           Instruction_sequence.instr (
             Hhbc_ast.ITry(Hhbc_ast.TryFaultBegin (makelabel $2)));
           $5;
           Instruction_sequence.instr (
             Hhbc_ast.ITry(Hhbc_ast.TryFaultEnd));
           $9
        ]
      }
    | TRYCATCHDIRECTIVE ID LBRACE NEWLINE functionbody nl RBRACE nl
       functionbody
       { Instruction_sequence.gather
         [
           Instruction_sequence.instr (
             Hhbc_ast.ITry(Hhbc_ast.TryCatchLegacyBegin (makelabel $2)));
           $5;
           Instruction_sequence.instr (
             Hhbc_ast.ITry(Hhbc_ast.TryCatchLegacyEnd));
           $9
         ] }
    | TRYDIRECTIVE LBRACE NEWLINE functionbody nl RBRACE
       CATCHDIRECTIVE LBRACE NEWLINE functionbody nl RBRACE nl
       functionbody
       { Instruction_sequence.gather
         [
           Instruction_sequence.instr (
             Hhbc_ast.ITry(Hhbc_ast.TryCatchBegin));
           $4;
           Instruction_sequence.instr (
             Hhbc_ast.ITry(Hhbc_ast.TryCatchMiddle));
           $10;
           Instruction_sequence.instr (
             Hhbc_ast.ITry(Hhbc_ast.TryCatchEnd));
           $14
         ] }
;
instruction:
    | ID COLON            {makelabelinst $1}
    | ID                  {makenullaryinst $1}
    | ID iarg             {makeunaryinst $1 $2}
    | ID iarg iarg        {makebinaryinst $1 $2 $3}
    | ID iarg iarg iarg   {maketernaryinst $1 $2 $3 $4}
    | ID iarg iarg PLUS INT {maketernaryinst $1 $2 $3 (IAInt64 $5)}
    | ID iarg iarg iarg iarg {makequaternaryinst $1 $2 $3 $4 $5}
;
iarg:
    | ID     {IAId $1}
    | vname  {IAId $1}
    | INT    {IAInt64 $1}
    | STRING {IAString $1}
    | DOUBLE {IADouble $1}
    | ATAUNDERSCORE INT {IAArrayno (Int64.to_int $2)}
    | ID COLON iarg {IAMemberkey ($1,$3)}
    | LANGLE iarglist RANGLE {IAArglist $2}
;
iarglist:
    | /* empty */ {[]}
    | iarg iarglist {$1 :: $2}
;
decllist:
    /* empty */ { [] }
    | decl nl decllist {$1 :: $3}
    | FILEPATHDIRECTIVE STRING SEMI nl decllist {$5}
;
