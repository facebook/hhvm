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

let die str =
  let oc = stderr in
  output_string oc str;
  close_out oc;
  exit 2

let error l = die (Utils.pmsg_l l)

(**********************************************************************)

let put str =
  output_string stdout str

let rec intersperse : 'a -> 'a list -> 'a list = fun v -> function
  | [] -> []
  | x :: [] -> [x]
  | x :: xs -> x :: (v :: (intersperse v xs))

let rec dropn n xs = if n <> 0 then dropn (n - 1) (List.tl xs) else xs

(**********************************************************************)

(*
 * Symbolic stack.
 *
 * While walking the Ast, we maintain a virtual execution stack that
 * tracks types of elements on the stack.  (Called "proto-flavors" in
 * the HHBC definition.)  These types are represented by the sym type.
 *
 * This allows emission to decide what sort of instruction to emit
 * based on this stack.
 *)

type sym = SVar
         | SCell
         | SRet
         | SFarg
         | SLoc of string

type env = {
  eval_stack: sym list;
  next_label: int;

  (* We need to save the evaluation stack at the site of any forward jump,
     and when we get to the jump target restore the stack.  This is
     necessary because the target might not otherwise be reachable, so we
     won't know how to proceed.

     (It can also be reachable---in that case the state of the evaluation
     stack must be identical to the one at the jump source, but we don't
     verify this currently and leave it up to a bytecode verifier, but it
     should probably be added... *)

  (* XXX: would've used an int map, but getting Map.Make to work (even with
     Int32) wasn't working.  strings seems fine to me. *)
  saved_eval_stacks: sym list SMap.t;
}

type label = string

let clean_env = {
  eval_stack = [];
  next_label = 0;
  saved_eval_stacks = SMap.empty;
}

let string_from_sym = function
  | SVar   -> "V"
  | SCell  -> "C"
  | SRet   -> "R"
  | SFarg  -> "F"
  | SLoc n -> "L:" ^ n

let stack_depth : env -> int = fun e -> List.length e.eval_stack
let stack_empty : env -> bool = fun e -> stack_depth e == 0
let stack_top : env -> sym = fun e -> List.hd e.eval_stack
let stack_nth : env -> int -> sym = fun e -> List.nth e.eval_stack
let stack_pop : env -> env = fun e ->
  { e with eval_stack = (List.tl e.eval_stack) }

let stack_push : env -> sym -> env = fun e s ->
  { e with eval_stack = [s] @ e.eval_stack }
let stack_pushc env = stack_push env SCell
let stack_pushv env = stack_push env SVar
let stack_pushr env = stack_push env SRet
let stack_pushf env = stack_push env SFarg
let stack_pushl env n = stack_push env (SLoc n)

let stack_string env =
  List.fold_left (^) ""
    (intersperse " " (List.map string_from_sym env.eval_stack))

let invariant_failure env desc =
  let str = stack_string env in
    die ("eval_stack in invalid state: [" ^ str ^ "]\n" ^
         "during: " ^ desc)

let stack_pop_expect : env -> sym -> env = fun env s ->
  if stack_empty env then invariant_failure env "pop of empty";
  if (stack_top env) <> s then invariant_failure env "pop expect";
  stack_pop env

let stack_pop_expectl : env -> env = fun env ->
  if stack_empty env then invariant_failure env "popl of empty";
  (match stack_top env with
     | SLoc _ -> ()
     | _      -> invariant_failure env "popl expect");
  stack_pop env

let clear_stack e = { e with eval_stack = [] }

let find_stack_location : env -> int -> int = fun env start ->
  let rec impl = fun list i ->
    match List.hd list with
      | SLoc _ -> i
      | _      -> impl (List.tl list) (i + 1)
  in impl (dropn start env.eval_stack) start

let create_label : env -> env * label = fun env ->
  let num = env.next_label
  and env = { env with next_label = env.next_label + 1 }
  in (env, Printf.sprintf "L%03d" num)

(* Helper for creating true,false,done label triples *)
let create_label_triple : env -> env * label * label * label = fun e ->
  let (e,l1) = create_label e in
  let (e,l2) = create_label e in
  let (e,l3) = create_label e in
  (e,l1,l2,l3)

let record_jmp : env -> label -> env = fun e label ->
  { e with saved_eval_stacks =
        SMap.add label e.eval_stack e.saved_eval_stacks }

(**********************************************************************)

let indent_op      = "       "
let indent_comment = "     "
let indent_inclass = "  "
let indent_label   = " "

let put_pseudo_main = put (List.fold_left (^) "" [
  "# hh_hhbc-generated .hhas output\n";
  ".main {\n";
  indent_op; "Int 1\n";
  indent_op; "RetC\n";
  "}\n\n"
])

let put_comment env str = put (indent_comment ^ "# " ^ str ^ "\n")

let trace_op _ = ()
(* let trace_op env = put_comment env ("trace: " ^ stack_string env) *)

let put_op env str = trace_op env;
                     put (indent_op ^ str ^ "\n")

let put_op2 env str arg = trace_op env;
                          put (indent_op  ^ str ^ " " ^ arg ^ "\n")

let put_op3 env str a1 a2 = trace_op env;
                            put (indent_op ^ str ^ " " ^ a1 ^ " " ^
                                 a2 ^ "\n")

(**********************************************************************)

let param_name : Ast.fun_param -> string = 
  fun param -> snd param.Ast.param_id

let begin_function : Ast.fun_ -> unit = fun f ->
  put (".function " ^ (snd f.Ast.f_name) ^ "(");
  (* XXX: what is f_params vs f_params? *)
  (* TODO: handle default params labels,
           support for type hints *)
  put (
    List.fold_left (^) ""
      (intersperse ", " (List.map param_name f.Ast.f_params))
  );
  put ") {\n"

let end_function : Ast.fun_ -> unit = fun f -> put "}\n\n"

let begin_method : Ast.method_ -> unit = fun m ->
  (* Would be nice to share code with begin_function, but all
     the record field names are different. *)
  put (indent_inclass ^ ".method " ^ (snd m.Ast.m_name) ^ "(");
  put (
    List.fold_left (^) ""
      (intersperse ", " (List.map param_name m.Ast.m_params))
  );
  put ") {\n"

let end_method : Ast.method_ -> unit = fun m ->
  put (indent_inclass ^ "}\n\n")

let begin_class : Ast.class_ -> unit = fun c ->
  (* TODO: extends, inheritance, traits, etc etc *)
  put (".class " ^ (snd c.Ast.c_name) ^ " {\n");
  (* We could only gen .default_ctor when there's no __construct,
     but it must be generated for classes without this. *)
  put ("  .default_ctor;\n\n")

let end_class : Ast.class_ -> unit = fun c -> put "}\n\n"

(**********************************************************************)

let escape_str : string -> string = fun s ->
  (* TODO: escape string C-style *)
  "\"" ^ s ^ "\""

(**********************************************************************)
(* Wrappers to emit specific bytecodes while maintaining the symbolic
 * stack.
 *
 * FIXME: We should consider using the C preprocessor and generating
 * these via the same metaprogramming that is used in hhbc.h?  The unusual
 * CamelCase stuff in the function names is because of the suspicion we'll
 * do that later and the fact that it will make it a lot easier.
 *)

let emit_RetV env = put_op env "RetV";
                    stack_pop_expect env SVar

let emit_RetC env = put_op env "RetC";
                    stack_pop_expect env SCell

let emit_UnboxR env = put_op env "UnboxR";
                      stack_pushc (stack_pop_expect env SRet)

let emit_Unbox env = put_op env "Unbox";
                     stack_pushc (stack_pop_expect env SRet)

let emit_CGetL env loc = put_op2 env "CGetL" loc;
                         stack_pushc (stack_pop_expectl env)

let emit_CGetL2 env loc =
  put_op2 env "CGetL2" loc;
  let top = stack_top env in
    stack_push
      (stack_pushc (stack_pop (stack_pop env)))
      top

let emit_String env s = put_op2 env "String" (escape_str s);
                        stack_pushc env

let emit_Null  env = put_op env "Null";  stack_pushc env
let emit_True  env = put_op env "True";  stack_pushc env
let emit_False env = put_op env "False"; stack_pushc env

let emit_Int    env s = put_op2 env "Int" s;    stack_pushc env
let emit_Double env s = put_op2 env "Double" s; stack_pushc env

let emit_SetL env loc = put_op2 env "SetL" loc;
                        stack_pushc
                          (stack_pop_expectl
                            (stack_pop_expect env SCell))

let emit_Not env = put_op env "Not";
                   stack_pushc (stack_pop_expect env SCell)

let emit_binop_impl what env =
  put_op env what;
  stack_pushc
    (stack_pop_expect (stack_pop_expect env SCell) SCell)

let emit_Sub    = emit_binop_impl "Sub"
let emit_Add    = emit_binop_impl "Add"
let emit_Eq     = emit_binop_impl "Eq"
let emit_Same   = emit_binop_impl "Same"
let emit_Div    = emit_binop_impl "Div"
let emit_Lt     = emit_binop_impl "Lt"
let emit_Lte    = emit_binop_impl "Lte"
let emit_Gt     = emit_binop_impl "Gt"
let emit_Gte    = emit_binop_impl "Gte"
let emit_Mul    = emit_binop_impl "Mul"
let emit_BitAnd = emit_binop_impl "BitAnd"
let emit_BitOr  = emit_binop_impl "BitOr"
let emit_Shl    = emit_binop_impl "Shl"
let emit_Shr    = emit_binop_impl "Shr"
let emit_Mod    = emit_binop_impl "Mod"
let emit_Xor    = emit_binop_impl "Xor"
let emit_NEq    = emit_binop_impl "NEq"
let emit_NSame  = emit_binop_impl "NSame"
let emit_Concat = emit_binop_impl "Concat"

let emit_PopC env = put_op env "PopC";
                    stack_pop_expect env SCell

let emit_PopV env = put_op env "PopV";
                    stack_pop_expect env SVar

let emit_jmp_impl kind env l =
  put_op2 env kind l;
  stack_pop_expect env SCell


let emit_JmpZ  = emit_jmp_impl "JmpZ"
let emit_JmpNZ = emit_jmp_impl "JmpNZ"

let emit_Jmp env l =
  put_op2 env "Jmp" l;
  (* Record the stack for the target, but we have to drop it going to
     the next instruction since this instruction can't fall through *)
  let env = record_jmp env l in
  { env with eval_stack = [] }

let emit_FPushFuncD env nargs name =
  put_op3 env "FPushFuncD" (string_of_int nargs) (escape_str name);
  env

let emit_FCall env nargs =
  put_op2 env "FCall" (string_of_int nargs);
  let rec dopop e = function
    | 0 -> e
    | n -> dopop (stack_pop_expect env SFarg) (n - 1)
  in let env = dopop env nargs
  in stack_pushr env

let emit_fpass_impl op expect env n =
  put_op2 env op (string_of_int n);
  stack_pushf (stack_pop_expect env expect)

let emit_FPassC = emit_fpass_impl "FPassC" SCell
let emit_FPassR = emit_fpass_impl "FPassR" SRet
let emit_FPassV = emit_fpass_impl "FPassV" SVar
let emit_FPassL env n l =
  put_op3 env "FPassL" (string_of_int n) l;
  stack_pushf (stack_pop_expectl env)

(**********************************************************************)

let emit_conv_cell : env -> env = fun env ->
  match stack_top env with
    | SLoc n -> emit_CGetL env n
    | SCell  -> env
    | SRet   -> emit_UnboxR env
    | SVar   -> emit_Unbox env
    | SFarg  -> invariant_failure env "emit_conv_cell can't convert F"

let emit_conv_cell_or_loc : env -> env = fun env ->
  match stack_top env with
    | SLoc _  -> env
    | _       -> emit_conv_cell env

let emit_conv_second_cell : env -> env = fun env ->
  match stack_nth env 1 with
    | SLoc loc -> emit_CGetL2 env loc
    | SCell    -> env
    | _        -> invariant_failure env "emit_conv_second_cell"

(**********************************************************************)
(* ADT predicates *)

let is_binop_assign : Ast.bop -> bool = function
  | Ast.Eq _ -> true
  | _        -> false

let is_binop_short_circuit : Ast.bop -> bool = function
  | Ast.BArbar -> true
  | Ast.AMpamp -> true
  | _          -> false

(**********************************************************************)

let emit_label : label -> env -> env = fun label env ->
  put (indent_label ^ label ^ ":\n");
  let saved =
    try SMap.find_unsafe label env.saved_eval_stacks
    with Not_found -> env.eval_stack
  in { env with eval_stack = saved }

let emit_ret : env -> env = fun env ->
  if stack_depth env <> 1 then
    invariant_failure env "return with stack_depth <> 1"
  else match stack_top env with
    | SVar   -> emit_RetV env
    | SCell  -> emit_RetC env
    | SRet   -> emit_RetC (emit_UnboxR env)
    | SLoc n -> emit_RetC (emit_CGetL env n)
    | SFarg  -> invariant_failure env "read a SFarg in emit_ret"

let emit_set : env -> env = fun env ->
  (* TODO: *M support *)
  let loc_i = find_stack_location env 1 in
  let set_sz = (stack_depth env) - loc_i - 1 in
  (* TODO: SetS support means checking set_sz == 1 also *)
  if set_sz == 0 then
    match stack_nth env loc_i with
      | SLoc loc  -> emit_SetL env loc
      (* SetN probably not needed *)
      (* SetG needed *)
      | _ -> die "emit_set default"
  else
    invariant_failure env "emit_set vector"

let emit_pop : env -> env = fun env ->
  match stack_top env with
    | SLoc _ -> invariant_failure env "pop of Loc" (* XXX is this possible? *)
    | SCell  -> emit_PopC env
    | SRet   -> emit_PopC (emit_UnboxR env)
    | SVar   -> emit_PopV env
    | SFarg  -> invariant_failure env "read a SFarg in emit_pop"

let emit_fpass : env -> int -> env = fun env n ->
  match stack_top env with
    | SLoc l -> emit_FPassL env n l
    | SCell  -> emit_FPassC env n
    | SRet   -> emit_FPassR env n
    | SVar   -> emit_FPassV env n
    | SFarg  -> invariant_failure env "read a SFarg in emit_fpass"

(**********************************************************************)

let rec emit_assign : env -> Ast.expr_ -> Ast.expr_ -> Ast.bop -> env =
fun env e1 e2 -> function
  | (Ast.Eq eqop) ->
      (* TODO: SetOp *)
      let env = emit_expr env e1 in
      (* TODO: will need emit AGet if sprop base *)
      let env = emit_expr env e2 in
      (* XXX FIXME: it looks like the parser rolls reference assignments
         into the same token as Eq, so we can't handle it correctly? *)
      emit_set (emit_conv_cell env)
  | _ -> invariant_failure env "emit_assign SetOp unimplemented"

(* Handle short-circuiting semantics for && and ||, delegating to
   emit_expr for the things inside. *)
and emit_if_condition : env -> Ast.expr_ -> label ->
                        label -> bool -> env =
fun env expression ltrue lfalse true_fallthrough ->
  let handle_normal = fun env expression ->
    let env = emit_conv_cell (emit_expr env expression) in
    if true_fallthrough then emit_JmpZ env lfalse
    else emit_JmpNZ env ltrue
  in match expression with
    | Ast.Binop (bo,(_,e1),(_,e2)) ->
        if not (is_binop_short_circuit bo) then handle_normal env expression
        else (
          let (env, lcontinue) = create_label env in
          let is_or = (match bo with
                         | Ast.BArbar -> true
                         | Ast.AMpamp -> false
                         | _ -> die "impossible") in
          let env =
            if is_or then
              emit_if_condition env e1 ltrue lcontinue false
            else
              emit_if_condition env e1 lcontinue lfalse true
          in
            emit_label lcontinue (
              emit_if_condition env e2 ltrue lfalse true_fallthrough
            )
        )
    | Ast.Unop (uo,(_,e)) ->
        if uo <> Ast.Unot then handle_normal env expression
        else
          (* Invert false and true labels, and whether true means to
             fall through *)
          emit_if_condition env e lfalse ltrue (not true_fallthrough)
    | _ -> handle_normal env expression

and emit_shortcircuit : env -> Ast.expr_ -> env = fun env expression ->
  let (env,ltrue,lfalse,ldone) = create_label_triple env in
  let env = emit_if_condition env expression ltrue lfalse
    false (* expression will be reinspected *) in
  emit_label lfalse (
    let env = emit_Jmp (emit_False env) ldone in
    emit_label ltrue (
      let env = emit_True env in
     emit_label ldone env
    )
  )

and emit_binop : env -> Ast.expr_ -> env = fun env -> function
  | Ast.Binop (bo,(_,e1),(_,e2)) as expression ->
      if is_binop_short_circuit bo then emit_shortcircuit env expression
      else if is_binop_assign bo then emit_assign env e1 e2 bo
      else
        let env = emit_conv_cell_or_loc (emit_expr env e1) in
        let env = emit_conv_cell (emit_expr env e2) in
        let env = emit_conv_second_cell env in
        (match bo with
           | Ast.Plus    -> emit_Add
           | Ast.Minus   -> emit_Sub
           | Ast.Eqeq    -> emit_Eq
           | Ast.EQeqeq  -> emit_Same
           | Ast.Slash   -> emit_Div
           | Ast.Lt      -> emit_Lt
           | Ast.Lte     -> emit_Lte
           | Ast.Gt      -> emit_Gt
           | Ast.Gte     -> emit_Gte
           | Ast.Star    -> emit_Mul
           | Ast.Amp     -> emit_BitAnd
           | Ast.Bar     -> emit_BitOr
           | Ast.Ltlt    -> emit_Shl
           | Ast.Gtgt    -> emit_Shr
           | Ast.Percent -> emit_Mod
           | Ast.Xor     -> emit_Xor
           | Ast.Diff    -> emit_NEq
           | Ast.Diff2   -> emit_NSame
           | Ast.Dot     -> emit_Concat
           (* The following are handled above *)
           | Ast.Eq _    -> die "unexpected"
           | Ast.AMpamp  -> die "unexpected"
           | Ast.BArbar  -> die "unexpected"
        ) env
  | _ -> invariant_failure env "emit_binop recieved non Binop"

and emit_unop : env -> Ast.expr_ -> Ast.uop -> env = fun env e -> function
  | Ast.Unot   -> emit_Not (emit_conv_cell (emit_expr env e))
  | Ast.Uminus -> let env = emit_Int env "0" in
                  emit_Sub (emit_conv_cell (emit_expr env e))
(*TODO | Utild *)
(*| Uincr
| Udecr | Upincr | Updecr*)
  | _ -> die "todo unop"

(* XXX: code duplication with emit_if ... *)
and emit_eif : env -> Ast.expr_ -> Ast.expr_ -> Ast.expr_ -> env =
fun env econd etrue efalse ->
  let (env,ltrue,lfalse,ldone) = create_label_triple env in
  let env = emit_if_condition env econd ltrue lfalse
    true (* true_fallthrough *) in
  emit_label ltrue (
    let env = emit_Jmp (emit_expr env etrue) ldone in
    emit_label lfalse (
      let env = emit_expr env efalse in
      emit_label ldone env;
    )
  )

and emit_if : env -> Ast.expr_ -> Ast.stmt -> Ast.stmt -> env =
fun env econd etrue efalse ->
  let (env,ltrue,lfalse,ldone) = create_label_triple env in
  let env = emit_if_condition env econd ltrue lfalse
    true (* true_fallthrough *) in
  emit_label ltrue (
    let env = emit_Jmp (emit_statement env etrue) ldone in
    emit_label lfalse (
      let env = emit_statement env efalse in
      emit_label ldone env;
    )
  )

and emit_call : env -> Ast.expr_ -> Ast.expr list -> env =
fun env fexpr args ->
  let nargs = List.length args in
  let env =
    (match fexpr with
       | Ast.Id (_,s)  -> emit_FPushFuncD env nargs s
       | _ -> die "unimplemented FCall type")
  in let rec emit_arg = fun env n xs ->
    if n == nargs then env
    else let env = emit_expr env (snd (List.hd xs)) in
         let env = emit_fpass env n in
         emit_arg env (n + 1) (List.tl xs)
  in emit_FCall (emit_arg env 0 args ) nargs

and emit_expr : env -> Ast.expr_ -> env = fun env expression ->
  match expression with
    (*TODO: Ast.Array*)
    | Ast.Null        -> emit_Null env
    | Ast.True        -> emit_True env
    | Ast.False       -> emit_False env
    (*TODO: Id*)
    | Ast.Lvar (_,s)  -> stack_pushl env s;
    (*TODO | Ast.Clone *)
    (*TODO
    | Obj_get of expr * expr
    | Array_get of expr * expr option
    | Class_get of id * pstring
    | Class_const of id * pstring *)
    | Ast.Call ((_,fe),args) -> emit_call env fe args
    | Ast.Int (_,s)     -> emit_Int env s
    | Ast.Float (_,s)   -> emit_Double env s
    | Ast.String (_,s)  -> emit_String env s
    (* XXX: if String2 doesn't have internal variables already
     * expanded into concat operators, this is wrong. *)
    | Ast.String2 (_,(_,s)) -> emit_String env s
    (*TODO
    | Yield of expr
    | Hashtable of key * field list
    | List of expr list
    | Vector of expr list
    | Cast of hint * expr
    *)
    | Ast.Unop (uop,(_,e)) -> emit_unop env e uop
    | Ast.Binop _ -> emit_binop env expression
    | Ast.Eif ((_,econd),None, (_,efals)) -> emit_eif env econd econd efals
    | Ast.Eif ((_,econd),Some (_,etru),(_,efals)) -> emit_eif env econd etru efals
    (*TODO
    | InstanceOf of expr * id
    | New of id * expr list
    | Efun of fun_ * id list
    | Xml of id * (id * expr) list * expr list
    *)
    | _ -> die "todo expression"

and emit_statement : env -> Ast.stmt -> env = fun env -> function
  | Ast.Unsafe      -> put_comment env "unsafe"; env
  | Ast.Expr (_,e)  -> emit_pop (emit_expr env e)
  | Ast.Block stmts -> List.fold_left emit_statement env stmts
  (*TODO
  | Break
  | Continue
  | Throw of expr
  *)
  | Ast.Return (_,Some (_,e)) -> emit_ret (emit_expr env e)
  | Ast.If ((_,econd),btrue,bfalse) ->
      emit_if env econd (Ast.Block btrue) (Ast.Block bfalse)
  (* TODO
  | Static_var of expr list
  | Do of block * expr
  | While of expr * block
  | For of stmt * expr * stmt * block
  | Switch of expr * case list
  | Foreach of expr * as_expr * block
  | Try of block * catch list *)
  | Ast.Noop -> env (* what is this *)
  | _ -> die "TODO: stmt"

let emit_implicit_ret : env -> env = fun env ->
  emit_RetC (emit_Null env)

let emit_function_body : env -> Ast.block -> env = fun env stmts ->
  let env = List.fold_left emit_statement env stmts in
  (* FIXME: would be nice not to emit this when it is unreachable *)
  emit_implicit_ret env

let check_stack_empty env where =
  if stack_depth env <> 0 then
    invariant_failure env ("stack nonempty between " ^ where)

let emit_func env f =
  begin_function f;
  let env = emit_function_body env f.Ast.f_body in
  check_stack_empty env "functions";
  end_function f;
  env

let emit_method : env -> Ast.method_ -> env = fun env m ->
  begin_method m;
  let env = emit_function_body env m.Ast.m_body in
  check_stack_empty env "methods";
  end_method m;
  env

let emit_class_def env = function
  | Ast.Method m -> emit_method env m
  | _ -> die "unimplemented class_def"

let emit_class env c =
  begin_class c;
  let env = List.fold_left emit_class_def env c.Ast.c_body in
  check_stack_empty env "classes";
  end_class c;
  env

let emit_def env = function
  | Ast.Constant _ -> die "constants not implemented"
  | Ast.Fun f   -> emit_func env f
  | Ast.Stmt s  -> die "top-level statement unimplemented"
  | Ast.Class c -> emit_class env c
  | Ast.Typedef _ -> die "typedefs not implemented"

let rec emit_program : env -> Ast.program -> unit =
  fun env p ->
    if List.length p <> 0 then
      let env = emit_def env (List.hd p) in
      emit_program env (List.tl p)

(**********************************************************************)

(*
Let's keep the code, we will fix that later.


let main_hhbc fn =
  Pos.file := fn;
  let ic = open_in fn in
  let lb = Lexing.from_channel ic in
  let shared_heap = Shared.make Path.dummy_path 1 in
  Shared.connect shared_heap;
  try
    let lexer = HackedLexer.make() in
    let builtins = Parser.program lexer (Lexing.from_string Ast.builtins) in
    let ast_no_builtins = Parser.program lexer lb in
    let ast = builtins @ ast_no_builtins in
    let funs, classes = List.fold_right begin fun def (funs, classes) ->
      match def with
      | Ast.Fun f -> f.Ast.f_name :: funs, classes
      | Ast.Class c -> funs, c.Ast.c_name :: classes
      | _ -> funs, classes
    end ast ([], []) in
    let nenv = Naming.make_env Naming.empty funs classes in
    let nast = Naming.program nenv fn in
    let all_classes = List.fold_right begin fun (_, cname) acc ->
      SMap.add cname fn acc
    end classes SMap.empty in
    Typing_decl.make_env nenv all_classes fn;
    let _ = Typing.program (Typing_env.empty()) nast in
    if List.length ast == 0 then die "empty program\n";
    let start_env = clean_env in
    put_pseudo_main;
    emit_program start_env ast_no_builtins
  with
    | Parsing.Parse_error -> error [Pos.make lb, "Syntax error"]
    | Utils.Error l       -> error l

let _ =
  if Array.length Sys.argv < 2 then
    die "usage: hh_hhbc filename\n";
  let fn = Sys.argv.(1) in
  main_hhbc fn
*)
