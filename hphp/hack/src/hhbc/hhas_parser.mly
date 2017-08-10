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
%token ISMEMOIZEWRAPPERDIRECTIVE STATICDIRECTIVE REQUIREDIRECTIVE
%token SRCLOCDIRECTIVE
%token LANGLE
%token RANGLE
%token COLONCOLON
%token DOTDOTDOT
%token LPAR
%token RPAR
%token LBRACE LBRACK
%token RBRACE RBRACK
%token COMMA SEMI
%token COLON EQUALS
%token EOF DOLLAR NEWLINE AMPERSAND
%token QUOTE AT UNDERSCORE
%token PLUS
%token TRYCATCHDIRECTIVE
%token TRYDIRECTIVE
%token CATCHDIRECTIVE
%token ALIASDIRECTIVE
%token STRICTDIRECTIVE

%start program
%type <Hhas_program.t> program
%%
program:
    nl decllist nl EOF { split_decl_list $2 [] [] None [] []}
;
decl:
    | maindecl {Main_decl $1}
    | fundecl {Fun_decl $1}
    | classdecl {Class_decl $1}
    | datadecl {Data_decl $1}
    | aliasdecl {Alias_decl $1}
;
aliasdecl:
    | ALIASDIRECTIVE ID EQUALS aliastypeinfo TRIPLEQUOTEDSTRING SEMI nl
      {Hhas_typedef.make (Hhbc_id.Class.from_raw_string $2)  $4 None}
;
maindecl:
    | MAINDIRECTIVE span LBRACE nl numiters ismemoizewrapper numclsrefslots declvars statics nl
      functionbody RBRACE nl
      {Hhas_body.make $11(*instrs*)
        $8(*declvars*) $5(*numiters*)
        $7(*numclsrefslots*) $6(*ismemoizewrapper*)
        [](*params*) None(*return type*) $9 (*static_inits*) None (* doc *)}
;
numiters:
    | /* empty */ {0}
    | NUMITERSDIRECTIVE INT SEMI nl {Int64.to_int $2}
;
datadecl:
    | DATADECLDIRECTIVE ID EQUALS nonclassattribute SEMI nl
      {  Hhas_adata.make $2 $4 }
;
nonclassattribute:
    | TRIPLEQUOTEDSTRING {attribute_from_string $1}
;
fundecl:
    | FUNCTIONDIRECTIVE attributes span typeinfooption ID fparams
      functionflags LBRACE nl numiters ismemoizewrapper numclsrefslots declvars statics
      functionbody RBRACE
        {
          let user_attrs, attrs = $2 in
          Hhas_function.make
            user_attrs (*attributes*)
            (Hhbc_id.Function.from_raw_string $5) (*name*)
            (Hhas_body.make
              $15 (*instrs*)
              $13 (*declvars*)
              $10 (*numiters*)
              $12 (*numclsrefslots *)
              $11 (*ismemoizewrapper*)
              $6 (*params*)
              $4 (*typeinfo*)
              $14 (*static_inits*)
              None (* doc *)
            )
            $3
            (isasync $7)
            (isgenerator $7)
            (ispairgenerator $7)
            (not (List.mem "nontop" attrs))
        }
;
nl:
    | /* empty */ {()}
    | NEWLINE nl  {()}
;
declvars:
    | /* empty */ {[]}
    | DECLVARSDIRECTIVE declvarlist SEMI nl {$2}
;
statics:
    | /* empty */ { [] }
    | STATICDIRECTIVE DOLLAR ID SEMI nl statics
      { $3 :: $6 }
;
requires:
    | /* empty */ { [] }
    | REQUIREDIRECTIVE ID LANGLE ID RANGLE SEMI nl requires
      {
        match $2 with
        | "implements" -> (Ast.MustImplement, $4) :: $8
        | "extends" -> (Ast.MustExtend, $4) :: $8
        | _ -> report_error "expected `implements` or `extends`"
      }
;
ismemoizewrapper:
    | /* empty */ {false}
    | ISMEMOIZEWRAPPERDIRECTIVE SEMI nl {true}
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
span:
    | LPAR INT COMMA INT RPAR { (Int64.to_int $2, Int64.to_int $4) }
    | /* empty */ { (-1, -1) }
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
is_variadic:
  | /* empty */ {false}
  | DOTDOTDOT {true}
param:
    | is_variadic typeinfooption possibleampersand vname paramdefaultvalueopt
      {Hhas_param.make
        $4 (* name *)
        $3 (* is_reference*)
        $1 (* variadic *)
        $2 (* type info option *)
        $5 (* default_value *)}
;
vname:
    | DOLLAR ID {"$" ^ $2}
    | DOLLAR INT ID {"$" ^ (Int64.to_string $2) ^ $3 }
;
classdecl:
    | CLASSDIRECTIVE attributes ID span extendsimplements LBRACE
     nl classuses classenumty requires classtypeconstants
     classproperties methods nl RBRACE nl
        {
          let user_attrs, attrs = $2 in
          Hhas_class.make
          user_attrs (*attributes*)
          (fst $5) (*base*)
          (snd $5) (*implements*)
          (Hhbc_id.Class.from_raw_string $3)(*name*)
          $4 (*span *)
          (List.mem "final"     attrs) (*isfinal*)
          (List.mem "abstract"  attrs) (*isabstract*)
          (List.mem "interface" attrs) (*isinterface*)
          (List.mem "trait"     attrs) (*istrait*)
          false (*isxhp*)
          (not (List.mem "nontop" attrs)) (*istop*)
          ((fun (x, _, _) -> x) $8)(*uses*)
          ((fun (_, x, _) -> x) $8)(*use_aliases*)
          ((fun (_, _, x) -> x) $8)(*use_precedences*)
          $9(*enumtype*)
          $13(*methods*)
          $12(*properties*)
          (fst $11) (*constants*)
          (snd $11) (*typeconstants*)
          $10 (* requirements *)
          None (* doc *) }
;
methods:
 | /* empty */ {[]}
 | methoddecl methods {$1 :: $2}
;
methodname:
 | ID {$1}
;
methoddecl:
 | METHODDIRECTIVE attributes span typeinfooption methodname fparams idlist LBRACE nl
    numiters ismemoizewrapper numclsrefslots declvars statics functionbody RBRACE nl
  {Hhas_method.make
    (fst $2) (* attributes *)
    (List.mem "protected" (snd $2))
    (List.mem "public" (snd $2))
    (List.mem "private" (snd $2))
    (List.mem "static" (snd $2))
    (List.mem "final" (snd $2))
    (List.mem "abstract" (snd $2))
    (List.mem "no_injection" (snd $2))
    (Hhbc_id.Method.from_raw_string $5) (* name *)
    (Hhas_body.make
      $15 (* method instructions *)
      $13 (* declvars *)
      $10 (* numiters *)
      $12 (* num cls ref slots *)
      $11 (* ismemoizewrapper *)
      $6 (* params *)
      $4 (* return type *)
      $14 (* static_inits *)
      None (* doc *)
      )
    $3
    (List.mem "isAsync" $7)
    (List.mem "isGenerator" $7)
    (List.mem "isPairGenerator" $7)
    (List.mem "isClosureBody" $7)
  }
;
numclsrefslots:
 | /* empty */ {0}
 | NUMCLSREFSLOTSDIRECTIVE INT SEMI nl {Int64.to_int $2}
;
srcloc:
 | SRCLOCDIRECTIVE INT COLON INT COMMA INT COLON INT SEMI
   { (($2, $4), ($6, $8)) }
;
staticdirective:
 | STATICDIRECTIVE DOLLAR ID EQUALS ID
   { ($3, $5) }
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
  | PROPERTYDIRECTIVE propertyattributes typeinfo idorvname EQUALS nl propertyvalue
  {Hhas_property.make
    (List.mem "private" $2)
    (List.mem "protected" $2)
    (List.mem "public" $2)
    (List.mem "static" $2)
    (List.mem "deep_init" $2)
    (List.mem "no_serialize" $2)
    (Hhbc_id.Prop.from_raw_string $4) (*name *)
    $7 (*initial value *)
    None (* initializer instructions. already been emitted elsewhere *)
    $3 (* type_info *)
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
     {( (Hhas_constant.make $2 (Some (attribute_from_string $4)) None) :: (fst $7),
        snd $7)}
  | CONSTDIRECTIVE ID EQUALS ID SEMI nl classtypeconstants
     {if $4="uninit" then
     ( (Hhas_constant.make $2 (Some Typed_value.Uninit) None) :: (fst $7),
       snd $7)
      else report_error "bad class constant"}
  | CONSTDIRECTIVE ID SEMI nl classtypeconstants
    { (Hhas_constant.make $2 None None :: fst $5,
       snd $5)
    }
  | CONSTDIRECTIVE ID ID EQUALS TRIPLEQUOTEDSTRING SEMI nl classtypeconstants
    {if $3 = "isType" then
     (fst $8,
      (Hhas_type_constant.make $2 (Some (attribute_from_string $5))) :: (snd $8))
     else report_error "expected type constant"}
  | CONSTDIRECTIVE ID ID SEMI nl classtypeconstants
    {if $3 = "isType" then
    (fst $6,
     (Hhas_type_constant.make $2 None :: (snd $6)))
    else report_error "expected type constant"}

classconstants:
  | /* empty */ {[]}
  | classconstant NEWLINE classconstants {$1 :: $3}
;
classconstant:
  | CONSTDIRECTIVE ID EQUALS TRIPLEQUOTEDSTRING SEMI
     {Hhas_constant.make $2 (Some (attribute_from_string $4)) None}
  | CONSTDIRECTIVE ID EQUALS ID SEMI
     {if $4="uninit" then Hhas_constant.make $2 (Some Typed_value.Uninit) None
      else report_error "bad class constant"}
;
typeconstants:
  | /* empty */ {[]}
  | typeconstant NEWLINE typeconstants {$1 :: $3}
;
typeconstant:
  | CONSTDIRECTIVE ID ID EQUALS TRIPLEQUOTEDSTRING SEMI
    {if $3 = "isType" then
     Hhas_type_constant.make $2 (Some (attribute_from_string $5))
     else report_error "expected type constant"}
;
classenumty:
  | /* empty */ {None}
  | ENUMTYDIRECTIVE enumtypeinfo SEMI nl {Some $2}
;
classuses:
  | /* empty */ {[], [], []}
  | USESDIRECTIVE idlist SEMI nl {$2, [], []}
  | USESDIRECTIVE idlist LBRACE nl classconflictlist nl RBRACE nl
   {let (aliases,precedences) = split_classconflicts $5
    in ($2, aliases, precedences)}
;
classconflictlist:
  | /* empty */ {[]}
  | idlist nl SEMI nl classconflictlist {parse_precedence_or_alias $1 :: $5}
  | ID ID LBRACK idlist RBRACK idoption nl SEMI nl classconflictlist
    {parse_alias $1 $2 $4 $6 :: $10}
;
idoption:
 | /* empty */ {None}
 | ID {Some $1}
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
attributes:
   | /* empty */ {([],[])}
   | LBRACK attributelist RBRACK {$2}
;
attributelist:
   | userattribute {([$1],[])}
   | ID {([],[$1])}
   | userattribute attributelist {($1 :: fst $2, snd $2)}
   | ID attributelist {(fst $2, $1 :: snd $2)}
;
userattribute:
    | STRING LPAR TRIPLEQUOTEDSTRING RPAR
      {match attribute_from_string $3 with
        | Typed_value.Array args ->
          Hhas_attribute.make $1 (unpack_key_values args)
        | _ ->
          report_error "user attributes should be arrays"
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
    | NEWLINE functionbody { $2 }
    | srcloc functionbody { $2 }
    | /* Label: Instruction */ ID COLON functionbody {
      Instruction_sequence.gather
        [Instruction_sequence.instr (makelabelinst $1); $3]}
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
    | AT ID  {IAArrayno $2}
    | ID COLON iarg {IAMemberkey ($1,$3)}
    | LANGLE iarglist RANGLE {IAArglist $2}
    | LANGLE iterbreaklist RANGLE {IAArglist $2}
;
iarglist:
    | /* empty */ {[]}
    | iarg iarglist {$1 :: $2}
    | iarg COMMA iarglist {$1 :: $3}
;
iterbreaklist:
    | iterbreak {[$1]}
    | iterbreak COMMA iterbreaklist {$1 :: $3}
;
iterbreak:
    | LPAR ID RPAR INT {IAIteratorid ($2,$4)}
;
decllist:
    /* empty */ { [] }
    | decl nl decllist {$1 :: $3}
    | FILEPATHDIRECTIVE STRING SEMI nl decllist {$5}
    | STRICTDIRECTIVE INT SEMI nl decllist {$5}
;
