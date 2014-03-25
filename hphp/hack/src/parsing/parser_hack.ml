(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
open Lexer_hack
open Ast

module SMap = Utils.SMap
module L = Lexer_hack

(*****************************************************************************)
(* Environment *)
(*****************************************************************************)

type env = {
    mode      : Ast.mode;
    priority  : int;
    lb        : Lexing.lexbuf;
    errors    : (Pos.t * string) list ref;
  }

let init_env lb = {
  mode     = Ast.Mpartial;
  priority = 0;
  lb       = lb;
  errors   = ref [];
}

(*****************************************************************************)
(* Lexer (with backtracking) *)
(*****************************************************************************)

type comments = (Pos.t * string) list

type saved_lb = {
  (* no need to save refill_buff because it's constant *)
  lex_abs_pos     : int;
  lex_start_pos   : int;
  lex_curr_pos    : int;
  lex_last_pos    : int;
  lex_last_action : int;
  lex_eof_reached : bool;
  lex_mem         : int array;
  lex_start_p     : Lexing.position;
  lex_curr_p      : Lexing.position;
}

let save_lexbuf_state (lb : Lexing.lexbuf) : saved_lb =
  {
    lex_abs_pos     = lb.Lexing.lex_abs_pos;
    lex_start_pos   = lb.Lexing.lex_start_pos;
    lex_curr_pos    = lb.Lexing.lex_curr_pos;
    lex_last_pos    = lb.Lexing.lex_last_pos;
    lex_last_action = lb.Lexing.lex_last_action;
    lex_eof_reached = lb.Lexing.lex_eof_reached;
    lex_mem         = lb.Lexing.lex_mem;
    lex_start_p     = lb.Lexing.lex_start_p;
    lex_curr_p      = lb.Lexing.lex_curr_p;
  }

let restore_lexbuf_state (lb : Lexing.lexbuf) (saved : saved_lb) : unit =
  begin
    lb.Lexing.lex_abs_pos     <- saved.lex_abs_pos;
    lb.Lexing.lex_start_pos   <- saved.lex_start_pos;
    lb.Lexing.lex_curr_pos    <- saved.lex_curr_pos;
    lb.Lexing.lex_last_pos    <- saved.lex_last_pos;
    lb.Lexing.lex_last_action <- saved.lex_last_action;
    lb.Lexing.lex_eof_reached <- saved.lex_eof_reached;
    lb.Lexing.lex_mem         <- saved.lex_mem;
    lb.Lexing.lex_start_p     <- saved.lex_start_p;
    lb.Lexing.lex_curr_p      <- saved.lex_curr_p;
  end

(*
 * Call a function with a forked lexing environment, and return its
 * result.
 *)
let look_ahead (env : env) (f : env -> 'a) : 'a =
  let saved = save_lexbuf_state env.lb in
  let ret = f env in
  restore_lexbuf_state env.lb saved;
  ret

(*
 * Conditionally parse, saving lexer state in case we need to backtrack.
 * The function parameter returns any optional type.  If it's None, pop
 * lexer state on the way out.
 *
 * Note that you shouldn't add any errors to the environment before
 * you've committed to returning Some something.  The error state is not
 * popped.
 *)
let try_parse (env : env) (f : env -> 'a option) : 'a option =
  let saved = save_lexbuf_state env.lb in
  match f env with
  | Some x -> Some x
  | None   -> (restore_lexbuf_state env.lb saved; None)

(* Return the next token without updating lexer state *)
let peek env =
  let saved = save_lexbuf_state env.lb in
  let ret = L.token env.lb in
  restore_lexbuf_state env.lb saved;
  ret

(* Drop the next token unconditionally *)
let drop (env : env) : unit = match L.token env.lb with _ -> ()

let btw (p1, _) (p2, _) = Pos.btw p1 p2

let is_hh_file = ref false

(*****************************************************************************)
(* Errors *)
(*****************************************************************************)

let error_at env pos msg =
  env.errors := (pos, msg) :: !(env.errors)

let error env msg =
  error_at env (Pos.make env.lb) msg

let error_continue env =
  error env
    "Yeah...we're not going to support continue/break N. \
    It makes static analysis tricky and it's not really essential"

let error_expect env expect =
  let pos = Pos.make env.lb in
  L.back env.lb;
  env.errors := (pos, "Expected "^expect) :: !(env.errors)

let expect env x =
  if L.token env.lb = x
  then ()
  else error_expect env (L.token_to_string x)

let expect_word env name =
  let tok = L.token env.lb in
  let value = Lexing.lexeme env.lb in
  if tok <> Tword || value <> name
  then error_expect env ("Was expecting: '"^name^ "' (not '"^value^"')");
  ()

let rec filter_duplicate_errors acc = function
  | [_] | [] as l -> l
  | (pos1, _) :: ((pos2, _) :: _ as rl) when Pos.compare pos1 pos2 = 0 ->
      filter_duplicate_errors acc rl
  | x :: rl -> filter_duplicate_errors (x :: acc) rl

(*****************************************************************************)
(* Modifiers checks (public private, final abstract etc ...)  *)
(*****************************************************************************)

let rec check_modifiers env pos abstract final = function
  | [] -> ()
  | Final :: _ when abstract ->
      error_at env pos "Parse error. Cannot mix final and abstract"
  | Abstract :: _ when final ->
      error_at env pos "Parse error. Cannot mix final and abstract"
  | Final :: rl -> check_modifiers env pos abstract true rl
  | Abstract :: rl -> check_modifiers env pos true final rl
  | _ :: rl -> check_modifiers env pos abstract final rl

let check_visibility env pos l =
  if List.exists begin function
    | Private | Public | Protected | Static -> true
    | _ -> false
  end l
  then ()
  else error_at env pos
      "Parse error. You are missing public, private or protected."

let rec check_mix_visibility env pos last_vis = function
  | [] -> ()
  | (Private | Public | Protected as vis) :: rl ->
      (match last_vis with
      | Some vis2 when vis <> vis2 ->
          error_at env pos
            "Parse error. Cannot mix different visibilities."
      | _ ->
          check_mix_visibility env pos (Some vis) rl
      )
  | _ :: rl -> check_mix_visibility env pos last_vis rl

let rec check_duplicates env pos = function
  | [_] | [] -> ()
  | Private :: rl -> check_duplicates env pos rl
  | x :: (y :: _) when x = y ->
      error_at env pos "Parse error. Duplicate modifier"
  | _ :: rl -> check_duplicates env pos rl

let check_modifiers env pos l =
  check_visibility env pos l;
  check_modifiers env pos false false l;
  check_duplicates env pos (List.sort compare l);
  check_mix_visibility env pos None l;
  ()

let check_not_final env pos modifiers =
  if List.exists (function Final -> true | _ -> false) modifiers
  then error_at env pos "class variable cannot be final";
  ()

let check_toplevel env pos =
  if env.mode = Ast.Mstrict
  then error_at env pos "Remove all toplevel statements except for requires"

(*****************************************************************************)
(* Check expressions. *)
(*****************************************************************************)

let rec check_lvalue env = function
  | _, (Lvar _ | Obj_get _ | Array_get _ | Class_get _) -> ()
  | pos, Call ((_, Id (_, "tuple")), _) ->
      error_at env pos
        "Tuple cannot be used as an lvalue. Maybe you meant List?"
  | _, List el -> List.iter (check_lvalue env) el
  | pos, (Array _ | Shape _ | Collection _
  | Null | True | False | Id _ | Clone _
  | Class_const _ | Call _ | Int _ | Float _
  | String _ | String2 _ | Yield _ | Yield_break
  | Await _ | Expr_list _ | Cast _ | Unop _ |
    Binop _ | Eif _ | InstanceOf _ | New _ | Efun _ | Lfun _ | Xml _) ->
      error_at env pos "Invalid lvalue"

(*****************************************************************************)
(* Operator priorities.
 *
 * It is annoying to deal with priorities by hand (although it's possible).
 * This list mimics what would typically look like yacc rules, defining
 * the operators priorities (from low to high), and associativity (left, right
 * or non-assoc).
 *
 * The priorities are then used by the "reducer" to auto-magically parse
 * expressions in the right order (left, right, non-assoc) and with the right
 * priority. Checkout the function "reduce" for more details.
 *)
(*****************************************************************************)

type assoc =
  | Left       (* a <op> b <op> c = ((a <op> b) <op> c) *)
  | Right      (* a <op> b <op> c = (a <op> (b <op> c)) *)
  | NonAssoc   (* a <op> b <op> c = error *)

let priorities = [
  (* Lowest priority *)
  (Left, [Tinclude; Tinclude_once; Teval; Trequire; Trequire_once]);
  (Left, [Tcomma]);
  (Right, [Tprint]);
  (Left, [Tqm; Tcolon]);
  (Left, [Tbarbar]);
  (Left, [Txor]);
  (Left, [Tampamp]);
  (Left, [Tbar]);
  (Left, [Tamp]);
  (NonAssoc, [Teqeq; Tdiff; Teqeqeq; Tdiff2]);
  (NonAssoc, [Tlt; Tlte; Tgt; Tgte]);
  (Left, [Tltlt; Tgtgt]);
  (Left, [Tplus; Tminus; Tdot]);
  (Left, [Tstar; Tslash; Tpercent]);
  (Right, [Tem]);
  (NonAssoc, [Tinstanceof]);
  (Right, [Ttild; Tincr; Tdecr; Tcast]);
  (Right, [Tat; Tref]);
  (NonAssoc, [Tyield]);
  (NonAssoc, [Tawait]);
  (Left, [Tlp]);
  (NonAssoc, [Tnew; Tclone]);
  (Left, [Tlb]);
  (Right, [Teq; Tpluseq; Tminuseq; Tstareq;
           Tslasheq; Tdoteq; Tpercenteq;
           Tampeq; Tbareq; Txoreq; Tlshifteq; Trshifteq]);
  (Left, [Tarrow]);
  (Left, [Telseif]);
  (Left, [Telse]);
  (Left, [Tendif]);
  (Left, [Tcolcol]);
  (Left, [Tdollar]);
  (* Highest priority *)
]

let get_priority =
  (* Creating the table of assocs/priorities at initialization time. *)
  let ptable = Hashtbl.create 23 in
  (* Lowest priority = 0 *)
  let priority = ref 0 in
  List.iter begin fun (assoc, tokl) ->
    List.iter begin fun token ->
      (* Associates operator => (associativity, priority) *)
      Hashtbl.add ptable token (assoc, !priority)
    end tokl;
    (* This is a bit subtle:
     *
     * The difference in priority between 2 lines should be 2, not 1.
     *
     * It's because of a trick we use in the reducer.
     * For something to be left-associative, we just pretend
     * that the right hand side expression has a higher priority.
     *
     * An example:
     * expr "1 + 2 + 3"
     * reduce (e1 = 1) "2 + 3"  // priority = 0
     * reduce (e1 = 1) (expr "2 + 3" with priority+1)
     * reduce (e1 = 1) (2, "+ 3") <--- this is where the trick is:
     *                                 because we made the priority higher
     *                                 the reducer stops when it sees the
     *                                 "+" sign.
     *)
    priority := !priority + 2
  end priorities;
  fun tok ->
    assert (Hashtbl.mem ptable tok);
    Hashtbl.find ptable tok

let with_priority env op f =
  let _, prio = get_priority op in
  let env = { env with priority = prio } in
  f env

let with_base_priority env f =
  let env = { env with priority = 0 } in
  f env

(*****************************************************************************)
(* References *)
(*****************************************************************************)

let ref_opt env =
  match L.token env.lb with
  | Tamp when env.mode = Ast.Mstrict ->
      error env "Don't use references!"
  | Tamp ->
      ()
  | _ ->
      L.back env.lb

(*****************************************************************************)
(* Identifiers *)
(*****************************************************************************)

(* identifier *)
let identifier env =
  match L.token env.lb with
  | Tword ->
      let pos = Pos.make env.lb in
      let name = Lexing.lexeme env.lb in
      pos, name
  | Tcolon ->
      (match L.xhpname env.lb with
      | Txhpname ->
          Pos.make env.lb, ":"^Lexing.lexeme env.lb
      | _ ->
          error_expect env "identifier";
          Pos.make env.lb, "*Unknown*"
      )
  | _ ->
      error_expect env "identifier";
      Pos.make env.lb, "*Unknown*"

(* $variable *)
let variable env =
  match L.token env.lb with
  | Tlvar ->
      Pos.make env.lb, Lexing.lexeme env.lb
  | _ ->
      error_expect env "variable";
      Pos.make env.lb, "$_"

(* &$variable *)
let ref_variable env =
  ref_opt env;
  variable env

(*****************************************************************************)
(* Entry point *)
(*****************************************************************************)

let rec entry ~fail content =
  is_hh_file := false;
  L.comment_list := [];
  let lb = Lexing.from_string content in
  let env = init_env lb in
  let ast = header env in
  if fail && !(env.errors) <> []
  then raise (Utils.Error (filter_duplicate_errors [] !(env.errors)));
  if not !is_hh_file
  then raise (Utils.Error [Pos.none, "PHP FILE"]);
  let comments = !L.comment_list in
  L.comment_list := [];
  comments, ast

(*****************************************************************************)
(* Hack headers (strict, decl, partial) *)
(*****************************************************************************)

and header env =
  match get_header env with
  | None ->
      []
  | Some Ast.Mdecl ->
      let env = { env with mode = Ast.Mdecl } in
      let attr = SMap.empty in
      let result = ignore_toplevel ~attr [] env (fun x -> x = Teof) in
      expect env Teof;
      is_hh_file := true;
      result
  | Some mode ->
      let result = toplevel [] { env with mode = mode } (fun x -> x = Teof) in
      expect env Teof;
      is_hh_file := true;
      result

and get_header env =
  match L.header env.lb with
  | `error -> None
  | `default_mode -> Some Ast.Mpartial
  | `explicit_mode ->
      let _token = L.token env.lb in
      (match Lexing.lexeme env.lb with
      | "strict" when !(Silent.is_silent_mode) || !(Ide.is_ide_mode) -> Some Ast.Mpartial
      | "strict" -> Some Ast.Mstrict
      | ("decl"|"only-headers") -> Some Ast.Mdecl
      | "partial" -> Some Ast.Mpartial
      | _ ->
          error env
 "Incorrect comment; possible values include strict, decl, partial or empty";
          Some Ast.Mdecl
      )

(*****************************************************************************)
(* Decl mode *)
(*****************************************************************************)

and ignore_toplevel ~attr acc env terminate =
  match L.token env.lb with
  | x when terminate x ->
      L.back env.lb;
      acc
  | Tltlt ->
      (* Parsing attribute << .. >> *)
      let attr = attribute_remain env SMap.empty in
      ignore_toplevel ~attr acc env terminate
  | Tlcb ->
      let acc = ignore_toplevel ~attr acc env terminate in
      ignore_toplevel ~attr acc env terminate
  | Tquote ->
      let pos = Pos.make env.lb in
      let abs_pos = env.lb.Lexing.lex_curr_pos in
      ignore (expr_string env pos abs_pos);
      ignore_toplevel ~attr acc env terminate
  | Tdquote ->
      let pos = Pos.make env.lb in
      ignore (expr_encapsed env pos);
      ignore_toplevel ~attr acc env terminate
  | Theredoc ->
      ignore (expr_heredoc env);
      ignore_toplevel ~attr acc env terminate
  | Tlt when is_xhp env ->
      ignore (xhp env);
      ignore_toplevel ~attr acc env terminate
  | Tword ->
      (match Lexing.lexeme env.lb with
      | "function" ->
          (match L.token env.lb with
          | Tword ->
              L.back env.lb;
              let def = toplevel_word ~attr env "function" in
              ignore_toplevel ~attr:SMap.empty (def @ acc) env terminate
          | _ ->
              ignore_toplevel ~attr acc env terminate
          )
      | "abstract" | "final"
      | "class"| "trait" | "interface"
      | "namespace"
      | "async" | "newtype"| "type"| "const" ->
          (* Parsing toplevel declarations (class, function etc ...) *)
          let def = toplevel_word ~attr env (Lexing.lexeme env.lb) in
          ignore_toplevel ~attr:SMap.empty (def @ acc) env terminate
      | _ -> ignore_toplevel ~attr acc env terminate
      )
  | _ -> ignore_toplevel ~attr acc env terminate

(*****************************************************************************)
(* Toplevel statements. *)
(*****************************************************************************)

and toplevel acc env terminate =
  match L.token env.lb with
  | x when terminate x ->
      L.back env.lb;
      List.rev acc
  | Tsc ->
      (* Ignore extra semicolons at toplevel (important so we don't yell about
       * them in strict mode). *)
      toplevel acc env terminate
  | Tltlt ->
      (* Parsing attribute << .. >> *)
      let attr = attribute_remain env SMap.empty in
      let _ = L.token env.lb in
      let def = toplevel_word ~attr env (Lexing.lexeme env.lb) in
      toplevel (def @ acc) env terminate
  | Tword ->
      (* Parsing toplevel declarations (class, function etc ...) *)
      let attr = SMap.empty in
      let def = toplevel_word ~attr env (Lexing.lexeme env.lb) in
      toplevel (def @ acc) env terminate
  | Tclose_php ->
      error env "Hack does not allow the closing ?> tag";
      List.rev acc
  | _ ->
      (* All the other statements. *)
      let pos = Pos.make env.lb in
      L.back env.lb;
      let error_state = !(env.errors) in
      let stmt = Stmt (statement env) in
      check_toplevel env pos;
      if error_state != !(env.errors)
      then ignore_toplevel ~attr:SMap.empty (stmt :: acc) env terminate
      else toplevel (stmt :: acc) env terminate

and toplevel_word ~attr env = function
  | "abstract" ->
      expect_word env "class";
      let class_ = class_ ~attr ~final:false ~kind:Cabstract env in
      [Class class_]
  | "final" ->
      expect_word env "class";
      let class_ = class_ ~attr ~final:true ~kind:Cnormal env in
      [Class class_]
  | "class" ->
      let class_ = class_ ~attr ~final:false ~kind:Cnormal env in
      [Class class_]
  | "trait" ->
      let class_ = class_ ~attr ~final:false ~kind:Ctrait env in
      [Class class_]
  | "interface" ->
      let class_ = class_ ~attr ~final:false ~kind:Cinterface env in
      [Class class_]
  | "async" ->
      expect_word env "function";
      let fun_ = fun_ ~attr ~sync:FAsync env in
      [Fun fun_]
  | "function" ->
      let fun_ = fun_ ~attr ~sync:FSync env in
      [Fun fun_]
  | "newtype" ->
      let id, tparaml, tconstraint, typedef = typedef env in
      [Typedef {
          t_id = id;
          t_tparams = tparaml;
          t_constraint = tconstraint;
          t_kind = NewType typedef;
          t_namespace = Namespace_env.empty;
          t_mode = env.mode;
      }]
  | "type" ->
      let id, tparaml, tconstraint, typedef = typedef env in
      [Typedef {
          t_id = id;
          t_tparams = tparaml;
          t_constraint = tconstraint;
          t_kind = Alias typedef;
          t_namespace = Namespace_env.empty;
          t_mode = env.mode;
      }]
  | "namespace" ->
      let id, body = namespace env in
      [Namespace (id, body)]
  | "use" ->
      let usel = namespace_use_list env [] in
      [NamespaceUse usel]
  | "const" ->
      let consts = class_const_def env in
      (match consts with
      | Const (h, cstl) ->
          List.map (fun (x, y) -> Constant {
            cst_mode = env.mode;
            cst_kind = Cst_const;
            cst_name = x;
            cst_type = h;
            cst_value = y;
            cst_namespace = Namespace_env.empty;
          }) cstl
      | _ -> assert false)
  | "require" | "require_once" ->
      let _ = expr env in
      expect env Tsc;
      [Stmt Noop]
  | _ ->
      let pos = Pos.make env.lb in
      L.back env.lb;
      let stmt = statement env in
      check_toplevel env pos;
      [define_or_stmt env stmt]

and define_or_stmt env = function
  | Expr (_, Call ((_, Id (_, "define")), [(_, String name); value])) ->
      Constant {
      cst_mode = env.mode;
      cst_kind = Cst_define;
      cst_name = name;
      cst_type = None;
      cst_value = value;
      cst_namespace = Namespace_env.empty;
    }
  | stmt ->
      Stmt stmt

(*****************************************************************************)
(* Attributes: <<_>> *)
(*****************************************************************************)

(* <<_>> *)
and attribute env =
  let acc = SMap.empty in
  if look_ahead env (fun env -> L.token env.lb = Tltlt)
  then begin
    expect env Tltlt;
    attribute_remain env acc;
  end
  else acc

(* _>> *)
and attribute_remain env acc =
  match L.token env.lb with
  | Tword ->
      let attr_name = Lexing.lexeme env.lb in
      let acc = attribute_parameter attr_name acc env in
      attribute_list_remain acc env
  | _ ->
      error_expect env "attribute name";
      acc

(* empty | (parameter_list) *)
and attribute_parameter attr_name acc env =
  match L.token env.lb with
  | Tlp ->
      let el = expr_list_remain env in
      SMap.add attr_name el acc
  | _ ->
      let acc = SMap.add attr_name [] acc in
      L.back env.lb;
      acc

(* ,_,>> *)
and attribute_list_remain acc env =
  match L.token env.lb with
  | Tgtgt -> acc
  | Tcomma -> attribute_remain env acc
  | _ ->
      error_expect env ">>";
      acc

(*****************************************************************************)
(* Functions *)
(*****************************************************************************)

and fun_ ~attr ~sync env =
  ref_opt env;
  let name = identifier env in
  let tparams = class_params env in
  let params = parameter_list env in
  let ret = hint_return_opt env in
  let body = function_body env in
  { f_name = name;
    f_tparams = tparams;
    f_params = params;
    f_ret = ret;
    f_body = body;
    f_user_attributes = attr;
    f_type = sync;
    f_mode = env.mode;
    f_mtime = 0.0;
    f_namespace = Namespace_env.empty;
  }

(*****************************************************************************)
(* Classes *)
(*****************************************************************************)

and class_ ~attr ~final ~kind env =
  let cname       = identifier env in
  let is_xhp      = (snd cname).[0] = ':' in
  let tparams     = class_params env in
  let cextends    = class_extends env in
  let cimplements = class_implements env in
  let cbody       = class_body env in
  let result =
    { c_mode            = env.mode;
      c_final           = final;
      c_kind            = kind;
      c_is_xhp          = is_xhp;
      c_implements      = cimplements;
      c_tparams         = tparams;
      c_user_attributes = attr;
      c_name            = cname;
      c_extends         = cextends;
      c_body            = cbody;
      c_mtime           = 0.0;
      c_namespace       = Namespace_env.empty;
    }
  in
  class_implicit_fields result

(*****************************************************************************)
(* Extends/Implements *)
(*****************************************************************************)

and class_extends env =
  match L.token env.lb with
  | Tword ->
      (match Lexing.lexeme env.lb with
      | "extends" -> class_extends_list env
      | "implements" -> L.back env.lb; []
      | _ -> error env "Expected: extends"; []
      )
  | Tlcb ->
      L.back env.lb;
      []
  | _ ->
      error_expect env "{";
      []

and class_implements env =
  match L.token env.lb with
  | Tword ->
      (match Lexing.lexeme env.lb with
      | "implements" -> class_extends_list env
      | "extends" -> L.back env.lb; []
      | _ -> error env "Expected: implements"; []
      )
  | Tlcb ->
      L.back env.lb;
      []
  | _ ->
      error_expect env "{";
      []

and class_extends_list env =
  let error_state = !(env.errors) in
  let c = class_hint env in
  match L.token env.lb with
  | Tlcb ->
      L.back env.lb; [c]
  | Tcomma ->
      if !(env.errors) != error_state
      then [c]
      else c :: class_extends_list env
  | Tword ->
      (match Lexing.lexeme env.lb with
      | "implements" | "extends" -> L.back env.lb; [c]
      | _ -> error_expect env "{"; []
      )
  | _ -> error_expect env "{"; []

(*****************************************************************************)
(* Class parameters class A<T as X ..> *)
(*****************************************************************************)

and class_params env =
  match L.token env.lb with
  | Tlt -> class_param_list env
  | _ -> L.back env.lb; []

and class_param_list env =
  let error_state = !(env.errors) in
  let cst = class_param env in
  match L.gt_or_comma env.lb with
  | Tgt ->
      [cst]
  | Tcomma ->
      if !(env.errors) != error_state
      then [cst]
      else cst :: class_param_list_remain env
  | _ ->
      error_expect env ">";
      [cst]

and class_param_list_remain env =
  match L.gt_or_comma env.lb with
  | Tgt -> []
  | _ ->
      L.back env.lb;
      let error_state = !(env.errors) in
      let cst = class_param env in
      match L.gt_or_comma env.lb with
      | Tgt ->
          [cst]
      | Tcomma ->
          if !(env.errors) != error_state
          then [cst]
          else cst :: class_param_list_remain env
      | _ -> error_expect env ">"; [cst]

and class_param env =
  match L.token env.lb with
  | Tword ->
      let parameter_name = Pos.make env.lb, Lexing.lexeme env.lb in
      let parameter_constraint = class_parameter_constraint env in
      parameter_name, parameter_constraint
  | _ ->
      error_expect env "type parameter";
      let parameter_name = Pos.make env.lb, "T*unknown*" in
      parameter_name, None


and class_parameter_constraint env =
  match L.token env.lb with
  | Tword when Lexing.lexeme env.lb = "as" ->
      Some (hint env)
  | _ -> L.back env.lb; None

(*****************************************************************************)
(* Class hints (A<T> etc ...) *)
(*****************************************************************************)

and class_hint env =
  let pname = identifier env in
  class_hint_with_name env pname

and class_hint_with_name env pname =
  let params = class_hint_params env in
  (fst pname), Happly (pname, params)

and class_hint_params env =
  match L.token env.lb with
  | Tlt -> class_hint_param_list env
  | _ -> L.back env.lb; []

and class_hint_param_list env =
  let error_state = !(env.errors) in
  let h = hint env in
  match L.gt_or_comma env.lb with
  | Tgt ->
      [h]
  | Tcomma ->
      if !(env.errors) != error_state
      then [h]
      else h :: class_hint_param_list_remain env
  | _ ->
      error_expect env ">"; [h]

and class_hint_param_list_remain env =
  match L.gt_or_comma env.lb with
  | Tgt -> []
  | _ ->
      L.back env.lb;
      let error_state = !(env.errors) in
      let h = hint env in
      match L.gt_or_comma env.lb with
      | Tgt ->
          [h]
      | Tcomma ->
          if !(env.errors) != error_state
          then [h]
          else h :: class_hint_param_list_remain env
      | _ -> error_expect env ">"; [h]

(*****************************************************************************)
(* Type hints: int, ?int, A<T>, array<...> etc ... *)
(*****************************************************************************)

and hint env =
  match L.token env.lb with
  (* ?_ *)
  | Tqm ->
      let start = Pos.make env.lb in
      let e = hint env in
      Pos.btw start (fst e), Hoption e
  (* A<_> | function(_):_ *)
  | Tword ->
      let pos = Pos.make env.lb in
      let word = Lexing.lexeme env.lb in
      hint_word env pos word
  (* :XHPNAME *)
  | Tcolon ->
      L.back env.lb;
      let cname = identifier env in
      class_hint_with_name env cname
  (* (_) *)
  | Tlp ->
      let start_pos = Pos.make env.lb in
      hint_paren start_pos env
  (* @_ *)
  | Tat ->
      let start = Pos.make env.lb in
      let h = hint env in
      Pos.btw start (fst h), snd h
  | _ ->
      error_expect env "type";
      let pos = Pos.make env.lb in
      pos, Happly ((pos, "*Unknown*"), [])

and hint_word env pos word =
 (* function(_): _ *)
  match word with
  | "function" ->
      hint_function pos env
  (* A<_> *)
  | name ->
      class_hint_with_name env (pos, name)

(* (_) *)
and hint_paren start env =
  let hintl = hint_list env in
  let end_ = Pos.make env.lb in
  let pos = Pos.btw start end_ in
  match hintl with
  | []  -> assert false
  | [_, Hfun _ as h] -> pos, snd h
  | [_] ->
      error_at env pos "Tuples of one element are not allowed";
      pos, Happly ((pos, "*Unkown*"), [])
  | hl  -> pos, Htuple hl

and hint_list env =
  let error_state = !(env.errors) in
  let h = hint env in
  match L.token env.lb with
  | Trp ->
      [h]
  | Tcomma ->
      if !(env.errors) != error_state
      then [h]
      else h :: hint_list_remain env
  | _ ->
      error_expect env ">"; [h]

and hint_list_remain env =
  match L.token env.lb with
  | Trp -> []
  | _ ->
      L.back env.lb;
      let error_state = !(env.errors) in
      let h = hint env in
      match L.token env.lb with
      | Trp ->
          [h]
      | Tcomma ->
          if !(env.errors) != error_state
          then [h]
          else h :: hint_list_remain env
      | _ ->
          error_expect env ">"; [h]

(*****************************************************************************)
(* Function hint (function(_): _) *)
(*****************************************************************************)

(* function(_): _ *)
and hint_function start env =
  expect env Tlp;
  let params, has_dots = hint_function_params env in
  let ret = hint_return env in
  Pos.btw start (fst ret), Hfun (params, has_dots, ret)

(* (parameter_1, .., parameter_n) *)
and hint_function_params env =
  match L.token env.lb with
  | Trp ->
      ([], false)
  | Topt_args ->
      hint_function_params_close env;
      ([], true)
  | _ ->
      L.back env.lb;
      hint_function_params_remain env

(* ) | ,) *)
and hint_function_params_close env =
  match L.token env.lb with
  | Trp ->
      ()
  | Tcomma ->
      expect env Trp
  | _ ->
      error_expect env ")";
      ()

(* _, parameter_list | _) | ...) | ...,) *)
and hint_function_params_remain env =
  let error_state = !(env.errors) in
  let h = hint env in
  match L.token env.lb with
  | Tcomma ->
      if !(env.errors) != error_state
      then ([h], false)
      else
        let hl, has_dots = hint_function_params env in
        (h :: hl, has_dots)
  | Trp ->
      ([h], false)
  | Topt_args ->
      hint_function_params_close env;
      ([h], true)
  | _ ->
      error_expect env ")";
      ([h], false)

(* : _ *)
and hint_return env =
  expect env Tcolon;
  hint env

and hint_return_opt env =
  match L.token env.lb with
  | Tcolon -> Some (hint env)
  | _ -> L.back env.lb; None

(*****************************************************************************)
(* Class statements *)
(*****************************************************************************)

(* { ... *)
and class_body env =
  let error_state = !(env.errors) in
  expect env Tlcb;
  if error_state != !(env.errors)
  then L.look_for_open_cb env.lb;
  class_defs env

and class_defs env =
  match L.token env.lb with
  (* ... } *)
  | Trcb ->
      []
  (* xhp_format | const | use *)
  | Tword ->
      let word = Lexing.lexeme env.lb in
      class_toplevel_word env word
  | Tltlt ->
  (* variable | method *)
      L.back env.lb;
      let error_state = !(env.errors) in
      let m = class_member_def env in
      if !(env.errors) != error_state
      then [m]
      else m :: class_defs env
  | _ ->
      error_expect env "class member";
      let start = Pos.make env.lb in
      look_for_next_method start env;
      let _ = L.token env.lb in
      let word = Lexing.lexeme env.lb in
      class_toplevel_word env word

and class_toplevel_word env word =
  match word with
  | "category" | "children" | "attribute" ->
      xhp_format env;
      class_defs env
  | "const" ->
      let error_state = !(env.errors) in
      let def = class_const_def env in
      if !(env.errors) != error_state
      then [def]
      else def :: class_defs env
  | "use" ->
      let traitl = class_use_list env in
      traitl @ class_defs env
  | "require" ->
      let traitl = trait_require env in
      traitl @ class_defs env
  | "abstract" | "public" | "protected" | "private" | "final" | "static"  ->
      (* variable | method *)
      L.back env.lb;
      let start = Pos.make env.lb in
      let error_state = !(env.errors) in
      let m = class_member_def env in
      if !(env.errors) != error_state
      then look_for_next_method start env;
      m :: class_defs env
  | _ ->
      error_expect env "modifier";
      []

and look_for_next_method previous_pos env =
  match L.token env.lb with
  | Teof -> ()
  | Trcb -> ()
  | Tword ->
      (match Lexing.lexeme env.lb with
      | "abstract" | "public" | "protected"
      | "private" | "final" | "static" ->
          let pos = Pos.make env.lb in
          if Pos.compare pos previous_pos = 0
          then (* we are stuck in a circle *)
            look_for_next_method pos env
          else
            (L.back env.lb; ())
      | _ -> look_for_next_method previous_pos env
      )
  | _ -> look_for_next_method previous_pos env

(*****************************************************************************)
(* Use (for traits) *)
(*****************************************************************************)

and class_use_list env =
  let error_state = !(env.errors) in
  let cst = ClassUse (class_hint env) in
  match L.token env.lb with
  | Tsc ->
      [cst]
  | Tcomma ->
      if !(env.errors) != error_state
      then [cst]
      else cst :: class_use_list_remain env
  | _ ->
      error_expect env ";"; [cst]

and class_use_list_remain env =
  match L.token env.lb with
  | Tsc -> []
  | _ ->
      L.back env.lb;
      let error_state = !(env.errors) in
      let cst = ClassUse (class_hint env) in
      match L.token env.lb with
      | Tsc ->
          [cst]
      | Tcomma ->
          if !(env.errors) != error_state
          then [cst]
          else cst :: class_use_list_remain env
      | _ -> error_expect env ";"; [cst]

and trait_require env =
  match L.token env.lb with
  | Tword ->
    let req_type = Lexing.lexeme env.lb in
    let ret = (match req_type with
      | "implements" -> [ClassTraitRequire (MustImplement, class_hint env)]
      | "extends" -> [ClassTraitRequire (MustExtend, class_hint env)]
      | _ -> error env "Expected: implements or extends"; []
    ) in
    (match L.token env.lb with
      | Tsc -> ret
      | _ -> error_expect env ";"; [])
  | _ -> error env "Expected: implements or extends"; []

(*****************************************************************************)
(* Class xhp_fromat *)
(*
 * within a class body -->
 *    children ...;
 *    attribute ...;
 *    category ...;
 *)
(*****************************************************************************)

and xhp_format env =
  match L.token env.lb with
  | Tsc -> ()
  | Tquote ->
      let pos = Pos.make env.lb in
      let abs_pos = env.lb.Lexing.lex_curr_pos in
      ignore (expr_string env pos abs_pos);
      xhp_format env
  | Tdquote ->
      let pos = Pos.make env.lb in
      ignore (expr_encapsed env pos);
      xhp_format env
  | x ->
      xhp_format env

(*****************************************************************************)
(* Class constants *)
(*
 *  within a class body -->
 *    const ...;
 *)
(*****************************************************************************)

(* const_hint const_name1 = value1, ..., const_name_n = value_n; *)
and class_const_def env =
  let h = class_const_hint env in
  let consts = class_const_list env in
  Const (h, consts)

(* const _ X = ...; *)
and class_const_hint env =
  if class_const_has_hint env
  then Some (hint env)
  else None

(* Determines if there is a type-hint by looking ahead. *)
and class_const_has_hint env =
  look_ahead env begin fun env ->
    match L.token env.lb with
    (* const_name = ... | hint_name const_name = ... *)
    | Tword ->
        (* If we see 'name =', there is no type hint *)
        L.token env.lb <> Teq
    | _ -> true
  end

and class_const_list env =
  let error_state = !(env.errors) in
  let cst = class_const env in
  match L.token env.lb with
  | Tsc ->
      [cst]
  | Tcomma ->
      if !(env.errors) != error_state
      then [cst]
      else cst :: class_const_list_remain env
  | _ ->
      error_expect env ";"; [cst]

and class_const_list_remain env =
  match L.token env.lb with
  | Tsc -> []
  | _ ->
      L.back env.lb;
      let error_state = !(env.errors) in
      let cst = class_const env in
      match L.token env.lb with
      | Tsc ->
          [cst]
      | Tcomma ->
          if !(env.errors) != error_state
          then [cst]
          else  cst :: class_const_list_remain env
      | _ ->
          error_expect env ";"; [cst]

(* const_name = const_value *)
and class_const env =
  let id = identifier env in
  expect env Teq;
  let e = expr env in
  id, e

(*****************************************************************************)
(* Modifiers *)
(*****************************************************************************)

and mandatory_modifier_list env =
  match L.token env.lb with
  | Tword ->
      let word = Lexing.lexeme env.lb in
      (match modifier_word env word with
      | None -> error_expect env "modifier"; []
      | Some v -> v :: optional_modifier_list env
      )
  | _ ->
      error_expect env "modifier"; []

and optional_modifier_list env =
  match L.token env.lb with
  | Tword ->
      let word = Lexing.lexeme env.lb in
      (match modifier_word env word with
      | None -> L.back env.lb; []
      | Some v -> v :: optional_modifier_list env
      )
  | _ ->
      L.back env.lb; []

and modifier_word env = function
  | "final"     -> Some Final
  | "static"    -> Some Static
  | "abstract"  -> Some Abstract
  | "private"   -> Some Private
  | "public"    -> Some Public
  | "protected" -> Some Protected
  | _ -> None

(*****************************************************************************)
(* Class variables/methods. *)
(*
 *  within a class body -->
 *    modifier_list ...;
 *)
(*****************************************************************************)

and class_member_def env =
  let attrs = attribute env in
  let modifier_start = Pos.make env.lb in
  let modifiers = mandatory_modifier_list env in
  let modifier_end = Pos.make env.lb in
  let modifier_pos = Pos.btw modifier_start modifier_end in
  check_modifiers env modifier_pos modifiers;
  match L.token env.lb with
  (* modifier_list $_ *)
  | Tlvar ->
      L.back env.lb;
      check_not_final env modifier_pos modifiers;
      let cvars = class_var_list env in
      ClassVars (modifiers, None, cvars)
  | Tword ->
      let word = Lexing.lexeme env.lb in
      class_member_word env ~modifiers ~attrs word
  | _ ->
      L.back env.lb;
      check_not_final env modifier_pos modifiers;
      let h = hint env in
      let cvars = class_var_list env in
      ClassVars (modifiers, Some h, cvars)

(*****************************************************************************)
(* Class variables *)
(*
 *  within a class body -->
 *    modifier_list $x;
 *    modifier_list hint $x;
 *)
(*****************************************************************************)

and class_var_list env =
  let error_state = !(env.errors) in
  let cvar = class_var env in
  if !(env.errors) != error_state
  then [cvar]
  else cvar :: class_var_list_remain env

and class_var_list_remain env =
  match L.token env.lb with
  | Tsc ->
      []
  | Tcomma ->
      (match L.token env.lb with
      | Tsc ->
          []
      | _ ->
          L.back env.lb;
          let error_state = !(env.errors) in
          let var = class_var env in
          if !(env.errors) != error_state
          then [var]
          else var :: class_var_list_remain env
      )
  | _ -> error_expect env ";"; []

and class_var env =
  let pos, name = variable env in
  let name = class_var_name name in
  let default = parameter_default env in
  (pos, name), default

and class_var_name name =
    String.sub name 1 (String.length name - 1)

(*****************************************************************************)
(* Methods *)
(*
 *  within a class body -->
 *    modifier_list async function ...
 *    modifier_list function ...
 *)
(*****************************************************************************)

and class_member_word env ~attrs ~modifiers = function
  | "async" ->
      expect_word env "function";
      ref_opt env;
      let fun_name = identifier env in
      let method_ = method_ env ~modifiers ~attrs ~sync:FAsync fun_name in
      Method method_
  | "function" ->
      ref_opt env;
      let fun_name = identifier env in
      let method_ = method_ env ~modifiers ~attrs ~sync:FSync fun_name in
      Method method_
  | _ ->
      L.back env.lb;
      let h = hint env in
      let cvars = class_var_list env in
      ClassVars (modifiers, Some h, cvars)

and method_ env ~modifiers ~attrs ~sync pname =
  let pos, name = pname in
  let tparams = class_params env in
  let params = parameter_list env in
  let ret = hint_return_opt env in
  let body = function_body env in
  let ret = method_implicit_return env pname ret in
  if name = "__destruct" && params <> []
  then error_at env pos "Destructor must not have any parameters.";
  { m_name = pname;
    m_tparams = tparams;
    m_params = params;
    m_ret = ret;
    m_body = body;
    m_kind = modifiers;
    m_user_attributes = attrs;
    m_type = sync;
  }

(*****************************************************************************)
(* Constructor/Destructors special cases. *)
(*****************************************************************************)

and method_implicit_return env (pos, name) ret =
  match name, ret with
  | ("__construct" | "__destruct"), None ->
      Some (pos, Happly((pos, "void"), []))
  | _, Some (_, Happly ((_, "void"), [])) -> ret
  | "__construct", Some _ ->
      error_at env pos "Constructor return type must be void or elided.";
      None
  | "__destruct", Some _ ->
      error_at env pos "Destructor return type must be void or elided.";
      None
  | _ -> ret

(*****************************************************************************)
(* Implicit class fields __construct(public int $x). *)
(*****************************************************************************)

and class_implicit_fields class_ =
  let class_body = method_implicit_fields class_.c_body in
  { class_ with c_body = class_body }

and method_implicit_fields members =
  match members with
  | [] -> []
  | Method ({ m_name = _, "__construct"; _ } as m) :: rl ->
      let fields, assigns = param_implicit_fields m.m_params in
      fields @ Method { m with m_body = assigns @ m.m_body } :: rl
  | x :: rl ->
      x :: method_implicit_fields rl

and param_implicit_fields params =
  match params with
  | [] -> [], []
  | { param_modifier = Some vis; _ } as p :: rl ->
      let member, stmt = param_implicit_field vis p in
      let members, assigns = param_implicit_fields rl in
      member :: members, stmt :: assigns
  | _ :: rl ->
      param_implicit_fields rl

and param_implicit_field vis p =
  (* Building the implicit field (for example: private int $x;) *)
  let pos, name = p.param_id in
  let cvname = pos, class_var_name name in
  let member = ClassVars ([vis], p.param_hint, [cvname, None]) in
  (* Building the implicit assignment (for example: $this->x = $x;) *)
  let this = pos, "$this" in
  let stmt =
    Expr (pos, Binop (Eq None, (pos, Obj_get((pos, Lvar this),
                                             (pos, Id cvname))),
                      (pos, Lvar p.param_id)))
  in
  member, stmt

(*****************************************************************************)
(* Function/Method bodies. *)
(*****************************************************************************)

and function_body env =
  match L.token env.lb with
  | Tsc -> []
  | Tlcb ->
      (match env.mode with
      | Mdecl ->
          ignore_body env;
          (* This is a hack for the type-checker to make a distinction
           * Between function foo(); and function foo() {}
           *)
          [Noop]
      | _ ->
          (match statement_list env with
          | [] -> [Noop]
          | x -> x)
      )
  | _ -> error_expect env "{"; []

and ignore_body env =
  match L.token env.lb with
  | Tlcb -> ignore_body env; ignore_body env
  | Trcb -> ()
  | Tquote ->
      let pos = Pos.make env.lb in
      let abs_pos = env.lb.Lexing.lex_curr_pos in
      ignore (expr_string env pos abs_pos);
      ignore_body env
  | Tdquote ->
      let pos = Pos.make env.lb in
      ignore (expr_encapsed env pos);
      ignore_body env
  | Theredoc ->
      ignore (expr_heredoc env);
      ignore_body env
  | Tlt when is_xhp env ->
      ignore (xhp env);
      ignore_body env
  | Teof -> error_expect env "}"; ()
  | _ -> ignore_body env

(*****************************************************************************)
(* Statements *)
(*****************************************************************************)

and statement_list env =
  match L.token env.lb with
  | Trcb -> []
  | Tlcb ->
      let block = statement_list env in
      Block block :: statement_list env
  | Tsc ->
      statement_list env
  | Teof ->
      error_expect env "}";
      []
  | _ ->
      L.back env.lb;
      let error_state = !(env.errors) in
      let stmt = statement env in
      if !(env.errors) != error_state
      then L.next_newline_or_close_cb env.lb;
      stmt :: statement_list env

and statement env =
  match L.token env.lb with
  | Tword ->
      let word = Lexing.lexeme env.lb in
      let stmt = statement_word env word in
      stmt
  | Tlcb ->
      Block (statement_list env)
  | Tsc ->
      Noop
  | Tunsafe ->
      Unsafe
  | Tfallthrough ->
      Fallthrough
  | _ ->
      L.back env.lb;
      let e = expr env in
      expect env Tsc;
      Expr e

and statement_word env = function
  | "break"    -> statement_break env
  | "continue" -> statement_continue env
  | "throw"    -> statement_throw env
  | "return"   -> statement_return env
  | "static"   -> statement_static env
  | "print"    -> statement_echo env
  | "echo"     -> statement_echo env
  | "if"       -> statement_if env
  | "do"       -> statement_do env
  | "while"    -> statement_while env
  | "for"      -> statement_for env
  | "switch"   -> statement_switch env
  | "foreach"  -> statement_foreach env
  | "try"      -> statement_try env
  | "function" | "class" | "trait" | "interface" | "const"
  | "async" | "abstract" | "final" ->
      error env
          "Parse error: declarations are not supported outside global scope";
      ignore (ignore_toplevel SMap.empty [] env (fun _ -> true));
      Noop
  | "require" | "require_once" ->
      if env.mode = Ast.Mstrict
      then
        error env
          "Parse error: require_once is not supported outside global scope";
      let _ = expr env in
      expect env Tsc;
      Noop
  | x ->
      L.back env.lb;
      let e = expr env in
      expect env Tsc;
      Expr e

(*****************************************************************************)
(* Break statement *)
(*****************************************************************************)

and statement_break env =
  check_continue env;
  Break

(*****************************************************************************)
(* Continue statement *)
(*****************************************************************************)

and statement_continue env =
  check_continue env;
  Continue;

and check_continue env =
  match L.token env.lb with
  | Tsc -> ()
  | Tint -> error_continue env
  | _ -> error_expect env ";"

(*****************************************************************************)
(* Throw statement *)
(*****************************************************************************)

and statement_throw env =
  let e = expr env in
  expect env Tsc;
  Throw e

(*****************************************************************************)
(* Return statement *)
(*****************************************************************************)

and statement_return env =
  let pos = Pos.make env.lb in
  let value = return_value env in
  Return (pos, value)

and return_value env =
  match L.token env.lb with
  | Tsc -> None
  | _ ->
      L.back env.lb;
      let e = expr env in
      expect env Tsc;
      Some e

(*****************************************************************************)
(* Static variables *)
(*****************************************************************************)

and statement_static env =
  let pos = Pos.make env.lb in
  match L.token env.lb with
  | Tlvar ->
      L.back env.lb;
      let el = static_var_list env in
      Static_var el
  | _ ->
      L.back env.lb;
      let id = pos, Id (pos, "static") in
      let e = expr_remain env id in
      Expr e

and static_var_list env =
  let error_state = !(env.errors) in
  let cst = static_var env in
  match L.token env.lb with
  | Tsc ->
      [cst]
  | Tcomma ->
      if !(env.errors) != error_state
      then [cst]
      else cst :: static_var_list_remain env
  | _ -> error_expect env ";"; [cst]

and static_var_list_remain env =
  match L.token env.lb with
  | Tsc -> []
  | _ ->
      L.back env.lb;
      let error_state = !(env.errors) in
      let cst = static_var env in
      match L.token env.lb with
      | Tsc ->
          [cst]
      | Tcomma ->
          if !(env.errors) != error_state
          then [cst]
          else cst :: static_var_list_remain env
      | _ ->
          error_expect env ";"; [cst]

and static_var env =
  expr env

(*****************************************************************************)
(* Switch statement *)
(*****************************************************************************)

and statement_switch env =
  let e = paren_expr env in
  expect env Tlcb;
  let casel = switch_body env in
  Switch (e, casel)

(* switch(...) { _ } *)
and switch_body env =
  match L.token env.lb with
  | Trcb ->
      []
  | Tword ->
      let word = Lexing.lexeme env.lb in
      switch_body_word env word
  | _ ->
      error_expect env "}";
      []

and switch_body_word env = function
  | "case" ->
      let e = expr env in
      expect env Tcolon;
      let stl = case_body env in
      Case (e, stl) :: switch_body env
  | "default" ->
      expect env Tcolon;
      let stl = case_body env in
      Default stl :: switch_body env
  | _ -> error_expect env "case"; []

(* switch(...) { case/default: _ } *)
and case_body env =
  match L.token env.lb with
  | Tword ->
      (match Lexing.lexeme env.lb with
      | "case" | "default" -> L.back env.lb; []
      | _ ->
          L.back env.lb;
          let error_state = !(env.errors) in
          let st = statement env in
          if !(env.errors) != error_state
          then [st]
          else st :: case_body env
      )
  | Trcb ->
      L.back env.lb; []
  | _ ->
      L.back env.lb;
      let error_state = !(env.errors) in
      let st = statement env in
      if !(env.errors) != error_state
      then [st]
      else st :: case_body env

(*****************************************************************************)
(* If statement *)
(*****************************************************************************)

and statement_if env =
  let e = paren_expr env in
  let st1 = statement env in
  let st2 = statement_else env in
  If (e, [st1], [st2])

and statement_else env =
  match L.token env.lb with
  | Tword ->
      (match Lexing.lexeme env.lb with
      | "else" -> statement env
      | "elseif" -> statement_if env
      | _ -> L.back env.lb; Noop
      )
  | _ -> L.back env.lb; Noop

(*****************************************************************************)
(* Do/While do statement *)
(*****************************************************************************)

and statement_do env =
  let st = statement env in
  expect_word env "while";
  let e = paren_expr env in
  expect env Tsc;
  Do ([st], e)

and statement_while env =
  let e = paren_expr env in
  let st = statement env in
  While (e, [st])

(*****************************************************************************)
(* For statement *)
(*****************************************************************************)

and statement_for env =
  expect env Tlp;
  let start = Pos.make env.lb in
  let _ = L.token env.lb in
  let _ = L.back env.lb in
  let last, el = for_expr env in
  let e1 = Pos.btw start last, Expr_list el in
  let start = last in
  let last, el = for_expr env in
  let e2 = Pos.btw start last, Expr_list el in
  let start = last in
  let last, el = for_last_expr env in
  let e3 = Pos.btw start last, Expr_list el in
  let st = statement env in
  For (e1, e2, e3, [st])

and for_expr env =
  match L.token env.lb with
  | Tsc ->
      Pos.make env.lb, []
  | _ ->
      L.back env.lb;
      let error_state = !(env.errors) in
      let e = expr env in
      match L.token env.lb with
      | Tsc ->
          Pos.make env.lb, [e]
      | _ when !(env.errors) != error_state ->
          L.back env.lb;
          Pos.make env.lb, [e]
      | Tcomma ->
            let last, el = for_expr env in
            last, e :: el
      | _ ->
          error_expect env ";";
          Pos.make env.lb, [e]

and for_last_expr env =
  match L.token env.lb with
  | Trp ->
      Pos.make env.lb, []
  | _ ->
      L.back env.lb;
      let error_state = !(env.errors) in
      let e = expr env in
      match L.token env.lb with
      | Trp ->
          Pos.make env.lb, [e]
      | _ when !(env.errors) != error_state ->
          L.back env.lb;
          Pos.make env.lb, [e]
      | Tcomma ->
          let last, el = for_last_expr env in
          last, e :: el
      | _ ->
          error_expect env ")";
          Pos.make env.lb, [e]

(*****************************************************************************)
(* Foreach statement *)
(*****************************************************************************)

and statement_foreach env =
  expect env Tlp;
  let e = expr env in
  expect_word env "as";
  let as_expr = foreach_as env in
  let st = statement env in
  Foreach (e, as_expr, [st])

and foreach_as env =
  let e1 = expr env in
  match L.token env.lb with
  | Tsarrow ->
      let e2 = expr env in
      expect env Trp;
      As_kv (e1, e2)
  | Trp ->
      As_id e1
  | _ ->
      error_expect env ")";
      As_id e1

(*****************************************************************************)
(* Try statement *)
(*****************************************************************************)

and statement_try env =
  let st = statement env in
  let cl = catch_list env in
  let fin = finally env in
  Try ([st], cl, fin)

and catch_list env =
  match L.token env.lb with
  | Tword when Lexing.lexeme env.lb = "catch" ->
      expect env Tlp;
      let name = identifier env in
      let e = variable env in
      expect env Trp;
      let st = statement env in
      (name, e, [st]) :: catch_list env
  | _ -> L.back env.lb; []

and finally env =
  match L.token env.lb with
  | Tword when Lexing.lexeme env.lb = "finally" ->
    let st = statement env in
    [st]
  | _ -> L.back env.lb; []

(*****************************************************************************)
(* Echo statement *)
(*****************************************************************************)

and statement_echo env =
  let pos = Pos.make env.lb in
  let args = echo_args env in
  let f = pos, Id (pos, "echo") in
  Expr (pos, Call (f, args))

and echo_args env =
  let e = expr env in
  match L.token env.lb with
  | Tsc ->
      [e]
  | Tcomma ->
      e :: echo_args env
  | _ ->
      error_expect env ";"; []

(*****************************************************************************)
(* Function/Method parameters *)
(*****************************************************************************)

and parameter_list env =
  expect env Tlp;
  parameter_list_remain env

and parameter_list_remain env =
  match L.token env.lb with
  | Trp -> []
  | Topt_args ->
      [parameter_varargs env]
  | _ ->
      L.back env.lb;
      let error_state = !(env.errors) in
      let p = param env in
      match L.token env.lb with
      | Trp ->
          [p]
      | Topt_args ->
          [p ; parameter_varargs env]
      | Tcomma ->
          if !(env.errors) != error_state
          then [p]
          else p :: parameter_list_remain env
      | _ ->
          error_expect env ")"; [p]

and parameter_varargs env =
  let pos = Pos.make env.lb in
  (match L.token env.lb with
  | Tcomma ->
      expect env Trp
  | Trp ->
      ()
  | _ -> error_expect env ")"
  );
  { param_hint = None;
    param_is_reference = false;
    param_id = (pos, "...");
    param_expr = None;
    param_modifier = None;
    param_user_attributes = SMap.empty;
  }

and param env =
  let attrs = attribute env in
  let modifs = parameter_modifier env in
  let h = parameter_hint env in
  ref_opt env;
  let name = variable env in
  let default = parameter_default env in
  { param_hint = h;
    param_is_reference = false;
    param_id = name;
    param_expr = default;
    param_modifier = modifs;
    param_user_attributes = attrs;
  }

and parameter_modifier env =
  match L.token env.lb with
  | Tword ->
      (match Lexing.lexeme env.lb with
      | "private" -> Some Private
      | "public" -> Some Public
      | "protected" -> Some Protected
      | _ -> L.back env.lb; None
      )
  | _ -> L.back env.lb; None

and parameter_hint env =
  if parameter_has_hint env
  then Some (hint env)
  else None

and parameter_has_hint env =
  look_ahead env begin fun env ->
    match L.token env.lb with
    | Topt_args | Tamp | Tlvar -> false
    | _ -> true
  end

and parameter_default env =
  match L.token env.lb with
  | Teq ->
      let default = expr env in
      Some default
  | _ -> L.back env.lb; None

(*****************************************************************************)
(* Expressions *)
(*****************************************************************************)

and expr env =
  let e1 = expr_atomic env in
  let e2 = expr_remain env e1 in
  e2

and expr_list env =
  expect env Tlp;
  expr_list_remain env

and expr_list_remain env =
  match L.token env.lb with
  | Trp -> []
  | _ ->
      L.back env.lb;
      let error_state = !(env.errors) in
      let e = expr { env with priority = 0 } in
      match L.token env.lb with
      | Trp ->
          [e]
      | Tcomma ->
          if !(env.errors) != error_state
          then [e]
          else e :: expr_list_remain env
      | _ -> error_expect env ")"; [e]

and expr_remain env e1 =
  match L.token env.lb with
  | Tplus ->
      expr_binop env Tplus Plus e1
  | Tminus ->
      expr_binop env Tminus Minus e1
  | Tstar ->
      expr_binop env Tstar Star e1
  | Tslash ->
      expr_binop env Tslash Slash e1
  | Teq ->
      expr_assign env Teq (Eq None) e1
  | Tbareq ->
      expr_assign env Tbareq (Eq (Some Bar)) e1
  | Tpluseq ->
      expr_assign env Tpluseq (Eq (Some Plus)) e1
  | Tstareq ->
      expr_assign env Tstareq (Eq (Some Star)) e1
  | Tslasheq ->
      expr_assign env Tslasheq (Eq (Some Slash)) e1
  | Tdoteq ->
      expr_assign env Tdoteq (Eq (Some Dot)) e1
  | Tminuseq ->
      expr_assign env Tminuseq (Eq (Some Minus)) e1
  | Tpercenteq ->
      expr_assign env Tpercenteq (Eq (Some Percent)) e1
  | Txoreq ->
      expr_assign env Txoreq (Eq (Some Xor)) e1
  | Tampeq ->
      expr_assign env Tampeq (Eq (Some Amp)) e1
  | Tlshifteq ->
      expr_assign env Tlshifteq (Eq (Some Ltlt)) e1
  | Trshifteq ->
      expr_assign env Trshifteq (Eq (Some Gtgt)) e1
  | Teqeqeq ->
      expr_binop env Teqeqeq EQeqeq e1
  | Tgt ->
      expr_binop env Tgt Gt e1
  | Tpercent ->
      expr_binop env Tpercent Percent e1
  | Tdot ->
      expr_binop env Tdot Dot e1
  | Teqeq ->
      expr_binop env Teqeq Eqeq e1
  | Tampamp ->
      expr_binop env Tampamp AMpamp e1
  | Tbarbar ->
      expr_binop env Tbarbar BArbar e1
  | Tdiff ->
      expr_binop env Tdiff Diff e1
  | Tlt ->
      expr_binop env Tlt Lt e1
  | Tdiff2 ->
      expr_binop env Tdiff2 Diff2 e1
  | Tgte ->
      expr_binop env Tgte Gte e1
  | Tlte ->
      expr_binop env Tlte Lte e1
  | Tamp ->
      expr_binop env Tamp Amp e1
  | Tbar ->
      expr_binop env Tbar Bar e1
  | Tltlt ->
      expr_binop env Tltlt Ltlt e1
  | Tgtgt ->
      expr_binop env Tgtgt Gtgt e1
  | Txor ->
      expr_binop env Txor Xor e1
  | Tincr | Tdecr as uop  ->
      expr_postfix_unary env uop e1
  | Tarrow ->
      expr_arrow env e1
  | Tcolcol ->
      expr_colcol env e1
  | Tlp ->
      expr_call env e1
  | Tlb ->
      expr_array_get env e1
  | Tqm ->
      expr_if env e1
  | Tword when Lexing.lexeme env.lb = "instanceof" ->
      expr_instanceof env e1
  | Tword when Lexing.lexeme env.lb = "and" ->
      error env ("Do not use \"and\", it has surprising precedence. "^
        "Use \"&&\" instead");
      expr_binop env Tampamp AMpamp e1
  | Tword when Lexing.lexeme env.lb = "or" ->
      error env ("Do not use \"or\", it has surprising precedence. "^
        "Use \"||\" instead");
      expr_binop env Tbarbar BArbar e1
  | Tword when Lexing.lexeme env.lb = "xor" ->
      error env ("Do not use \"xor\", it has surprising precedence. "^
        "Cast to bool and use \"^\" instead");
      expr_binop env Txor Xor e1
  | _ ->
      L.back env.lb; e1

(*****************************************************************************)
(* Expression reducer *)
(*****************************************************************************)

and reduce env e1 op make =
  let e, continue = reduce_ env e1 op make in
  if continue then expr_remain env e else e

and reduce_ env e1 op make =
  let current_prio = env.priority in
  let assoc, prio = get_priority op in
  let env = { env with priority = prio } in
  if prio = current_prio
  then
    match assoc with
    | Left ->
        let e = make e1 { env with priority = env.priority + 1 } in
        expr_remain env e, true
    | Right ->
        let e = make e1 env in
        e, false
    | NonAssoc ->
        error env "This operator is not associative, add parentheses";
        let e = make e1 env in
        e, false
  else if prio < current_prio
  then begin
    L.back env.lb;
    e1, false
  end
  else begin
    assert (prio > current_prio);
    if assoc = NonAssoc
    then make e1 env, true
    else reduce_ env e1 op make
  end

(*****************************************************************************)
(* lambda expressions *)
(*****************************************************************************)

and lambda_expr_body : env -> block = fun env ->
  let (p, e1) = expr env in
  [Return (p, (Some (p, e1)))]

and lambda_body env params =
  let body =
    if peek env = Tlcb
    then function_body env
    else lambda_expr_body env
  in
  let f = {
    f_name = (Pos.none, ";anonymous");
    f_tparams = [];
    f_params = params;
    f_ret = None;
    f_body = body;
    f_user_attributes = Utils.SMap.empty;
    f_type = FSync;
    f_mode = env.mode;
    f_mtime = 0.0;
    f_namespace = Namespace_env.empty;
  }
  in Lfun f;

and make_lambda_param : id -> fun_param = fun var_id ->
  {
    param_hint = None;
    param_is_reference = false;
    param_id = var_id;
    param_expr = None;
    param_modifier = None;
    param_user_attributes = Utils.SMap.empty;
  }

and lambda_single_arg env var_id =
  expect env Tlambda;
  lambda_body env [make_lambda_param var_id]

and short_lambda_param_list env acc =
  let tok = L.token env.lb in
  match tok with
  | Trp -> Some acc;
  | Tlvar ->
    let next_tok = peek env in
    (match next_tok with
    | Tcomma | Trp ->
        let pos = Pos.make env.lb in
        let var_id = (pos, Lexing.lexeme env.lb) in
        let acc = (make_lambda_param var_id) :: acc in
        drop env;
        if next_tok = Tcomma
        then short_lambda_param_list env acc
        else Some (List.rev acc)
    | _ -> None)
  | _ -> None

and try_short_lambda env =
  try_parse env begin fun env ->
    match short_lambda_param_list env [] with
    | Some param_list ->
        if not (peek env = Tlambda)
        then None
        else begin
          drop env;
          Some (lambda_body env param_list)
        end
    | None -> None
  end

(*****************************************************************************)
(* Expressions *)
(*****************************************************************************)

and expr_atomic ?(allow_class=false) env  =
  let tok = L.token env.lb in
  let pos = Pos.make env.lb in
  match tok with
  | Tint ->
      let tok_value = Lexing.lexeme env.lb in
      pos, Int (pos, tok_value)
  | Tfloat ->
      let tok_value = Lexing.lexeme env.lb in
      pos, Float (pos, tok_value)
  | Tquote ->
      let absolute_pos = env.lb.Lexing.lex_curr_pos in
      expr_string env pos absolute_pos
  | Tdquote ->
      expr_encapsed env pos
  | Tlvar ->
      let tok_value = Lexing.lexeme env.lb in
      let var_id = (pos, tok_value) in
      pos, if peek env = Tlambda
           then lambda_single_arg env var_id
           else Lvar var_id
  | Tcolon ->
      L.back env.lb;
      let name = identifier env in
      fst name, Id name
  | Tem | Tincr | Tdecr | Ttild | Tplus | Tminus as op ->
      expr_prefix_unary env pos op
  | Tamp ->
      with_priority env Tref expr
  | Tat ->
      with_priority env Tat expr
  | Tword ->
      let word = Lexing.lexeme env.lb in
      expr_atomic_word ~allow_class env pos word
  | Tlp ->
      (match try_short_lambda env with
      | None ->
          if is_cast env
          then expr_cast env pos
          else with_base_priority env begin fun env ->
            let e = expr env in
            expect env Trp;
            let end_ = Pos.make env.lb in
            Pos.btw pos end_, snd e
          end
      | Some l -> pos, l
      )
  | Tlb ->
      expr_short_array env pos
  | Tlt when is_xhp env ->
      xhp env
  | Theredoc ->
      expr_heredoc env
  | Tdollar ->
      error env ("A valid variable name starts with a letter or underscore,"^
        "followed by any number of letters, numbers, or underscores");
      expr env
  | _ ->
      error_expect env "expression";
      pos, Null

and expr_atomic_word ~allow_class env pos = function
  | "class" when not allow_class ->
      error_expect env "expression";
      pos, Null
  | "final" | "abstract" | "interface" | "trait" ->
      error_expect env "expression";
      pos, Null
  | "true"  ->
      pos, True
  | "false" ->
      pos, False
  | "null"  ->
      pos, Null
  | "array" ->
      expr_array env pos
  | "shape" ->
      expr_shape env pos
  | "new" ->
      expr_new env pos
  | "async" ->
      expr_anon_async env pos
  | "function" ->
      expr_anon_fun env pos ~sync:FSync
  | name when is_collection env ->
      expr_collection env pos name
  | "await" ->
      expr_await env pos
  | "yield" ->
      expr_yield env pos
  | "clone" ->
      expr_clone env pos
  | "list" ->
      expr_php_list env pos
  | x ->
      pos, Id (pos, x)

(*****************************************************************************)
(* Expressions in parens. *)
(*****************************************************************************)

and paren_expr env =
  with_base_priority env begin fun env ->
    expect env Tlp;
    let e = expr env in
    expect env Trp;
    e
  end

(*****************************************************************************)
(* Assignments (=, +=, -=, ...) *)
(*****************************************************************************)

and expr_assign env bop ast_bop e1 =
  reduce env e1 bop begin fun e1 env ->
    check_lvalue env e1;
    let e2 = expr { env with priority = 0 } in
    btw e1 e2, Binop (ast_bop, e1, e2)
  end

(*****************************************************************************)
(* Binary operations (+, -, /, ...) *)
(*****************************************************************************)

and expr_binop env bop ast_bop e1 =
  reduce env e1 bop begin fun e1 env ->
    let e2 = expr env in
    btw e1 e2, Binop (ast_bop, e1, e2)
  end

(*****************************************************************************)
(* Object Access ($obj->method) *)
(*****************************************************************************)

and expr_arrow env e1 =
  reduce env e1 Tarrow begin fun e1 env ->
    let e2 =
      match L.token env.lb with
      | Tword ->
          let name = Lexing.lexeme env.lb in
          let pos = Pos.make env.lb in
          pos, Id (pos, name)
      | _ -> L.back env.lb; expr env
    in
    btw e1 e2, Obj_get (e1, e2)
  end

(*****************************************************************************)
(* Class Access (ClassName::method_name) *)
(*****************************************************************************)

and expr_colcol env e1 =
  reduce env e1 Tcolcol begin fun e1 env ->
    (match e1 with
    | (_, Id cname) ->
        (* XYZ::class is OK ... *)
        expr_colcol_remain ~allow_class:true env e1 cname
    | pos, Lvar cname when env.mode = Ast.Mstrict ->
        error_at env pos "Don't use dynamic classes";
        e1
    | pos, Lvar cname  ->
        (* ... but get_class($x) should be used instead of $x::class *)
        expr_colcol_remain ~allow_class:false env e1 cname
    | pos, _ ->
        error_at env pos "Expected class name";
        e1
    )
  end

and expr_colcol_remain ~allow_class env e1 cname =
  match expr_atomic env ~allow_class with
  | _, Lvar x ->
      btw e1 x, Class_get (cname, x)
  | _, Id x ->
      btw e1 x, Class_const (cname, x)
  | pos, _ ->
      error_at env pos "Expected identifier";
      e1

(*****************************************************************************)
(* Function call (foo(params)) *)
(*****************************************************************************)

and expr_call env e1 =
  reduce env e1 Tlp begin fun e1 env ->
    L.back env.lb;
    let args = expr_list env in
    let end_ = Pos.make env.lb in
    Pos.btw (fst e1) end_, Call (e1, args)
  end

(*****************************************************************************)
(* Collections *)
(*****************************************************************************)

and is_collection env = peek env = Tlcb

and expr_collection env pos name =
  if is_collection env
  then build_collection env pos name
  else pos, Id (pos, name)

and build_collection env pos name =
  let name = pos, name in
  let fds = collection_field_list env in
  let end_ = Pos.make env.lb in
  Pos.btw pos end_, Collection (name, fds)

and collection_field_list env =
  expect env Tlcb;
  collection_field_list_remain env

and collection_field_list_remain env =
  match L.token env.lb with
  | Trcb -> []
  | _ ->
      L.back env.lb;
      let error_state = !(env.errors) in
      let fd = array_field env in
      match L.token env.lb with
      | Trcb ->
          [fd]
      | Tcomma ->
          if !(env.errors) != error_state
          then [fd]
          else fd :: collection_field_list_remain env
      | _ ->
          error_expect env "}"; []

(*****************************************************************************)
(* InstanceOf *)
(*****************************************************************************)

and expr_instanceof env e1 =
  reduce env e1 Tinstanceof begin fun e1 env ->
    let e2 = expr env in
    btw e1 e2, InstanceOf (e1, e2)
  end

(*****************************************************************************)
(* Yield/Await *)
(*****************************************************************************)

and expr_yield env start =
  with_priority env Tyield begin fun env ->
    match L.token env.lb with
    | Tword when Lexing.lexeme env.lb = "break" ->
        let end_ = Pos.make env.lb in
        Pos.btw start end_, Yield_break
    | _ ->
        L.back env.lb;
        let e = expr env in
        Pos.btw start (fst e), Yield e
  end

and expr_await env start =
  with_priority env Tawait begin fun env ->
    let e = expr env in
    Pos.btw start (fst e), Await e
  end

(*****************************************************************************)
(* Clone *)
(*****************************************************************************)

and expr_clone env start =
  with_base_priority env begin fun env ->
    let e = expr env in
    Pos.btw start (fst e), Clone e
  end

(*****************************************************************************)
(* List *)
(*****************************************************************************)

and expr_php_list env start =
  let el = expr_list env in
  let end_ = Pos.make env.lb in
  Pos.btw start end_, List el

(*****************************************************************************)
(* Anonymous functions *)
(*****************************************************************************)

and is_function env =
  look_ahead env begin fun env ->
    let tok = L.token env.lb in
    tok = Tword &&
    Lexing.lexeme env.lb = "function"
  end

and expr_anon_async env pos =
  if is_function env
  then begin
    expect_word env "function";
    expr_anon_fun env pos ~sync:FAsync
  end
  else pos, Id (pos, "async")

and expr_anon_fun env pos ~sync =
  let env = { env with priority = 0 } in
  let params = parameter_list env in
  let ret = hint_return_opt env in
  let use = function_use env in
  let body = function_body env in
  let f = {
    f_name = (Pos.none, ";anonymous");
    f_tparams = [];
    f_params = params;
    f_ret = ret;
    f_body = body;
    f_user_attributes = Utils.SMap.empty;
    f_type = sync;
    f_mode = env.mode;
    f_mtime = 0.0;
    f_namespace = Namespace_env.empty;
  }
  in
  pos, Efun (f, use)

(*****************************************************************************)
(* Use (for functions) *)
(*****************************************************************************)

and function_use env =
  match L.token env.lb with
  | Tword when Lexing.lexeme env.lb = "use" ->
      expect env Tlp;
      use_list env
  | _ -> L.back env.lb; []

and use_list env =
  match L.token env.lb with
  | Trp -> []
  | _ ->
      L.back env.lb;
      let error_state = !(env.errors) in
      let var = ref_variable env in
      match L.token env.lb with
      | Tcomma ->
          if !(env.errors) != error_state
          then [var]
          else var :: use_list env
      | Trp ->
          [var]
      | _ ->
          error_expect env ")";
          [var]

(*****************************************************************************)
(* New: new ClassName(...) *)
(*****************************************************************************)

and expr_new env pos_start =
  with_priority env Tnew begin fun env ->
    let cname =
      let e = expr env in
      match e with
      | p, Lvar _
      | p, Array_get _
      | p, Obj_get _
      | p, Class_get _
      | p, Call _ ->
          if env.mode = Ast.Mstrict
          then error env "Cannot use dynamic new in strict mode";
          p, "*Unknown*"
      | _, Id x -> x
      | p, _ ->
          error_expect env "class name";
          p, "*Unknown*"
    in
    let args = expr_list env in
    let pos_end = Pos.make env.lb in
    Pos.btw pos_start pos_end, New (cname, args)
  end

(*****************************************************************************)
(* Casts: (int|..|float) expr *)
(*****************************************************************************)

and is_cast_type = function
  | "int" | "float" | "double" | "string"
  | "array" | "object" | "bool" | "unset" -> true
  | _ -> false

and is_cast env =
  look_ahead env begin fun env ->
    let _ = L.token env.lb in
    is_cast_type (Lexing.lexeme env.lb) &&
    L.token env.lb = Trp
  end

and expr_cast env start_pos =
  with_priority env Tcast begin fun env ->
    let tok = L.token env.lb in
    let cast_type = Lexing.lexeme env.lb in
    assert (tok = Tword);
    assert (is_cast_type cast_type);
    expect env Trp;
    let ty = Pos.make env.lb, cast_type in
    let ty = fst ty, Happly (ty, []) in
    let e = expr env in
    match cast_type with
    | "unset" -> e
    | _ -> Pos.btw start_pos (fst e), Cast (ty, e)
  end

(*****************************************************************************)
(* Unary operators $i++ etc ... *)
(*****************************************************************************)

and unary_priority = function
  | Tplus | Tminus -> Tincr
  | x -> x

and expr_prefix_unary env start op =
  with_priority env (unary_priority op) begin fun env ->
    let e = expr env in
    let op =
      match op with
      | Tem -> Unot
      | Tincr -> Uincr
      | Tdecr -> Udecr
      | Ttild -> Utild
      | Tplus -> Uplus
      | Tminus -> Uminus
      | _ -> assert false
    in
    Pos.btw start (fst e), Unop (op, e)
  end

and expr_postfix_unary env uop e1 =
  let end_ = Pos.make env.lb in
  let e =
    reduce env e1 (unary_priority uop) begin fun e1 env ->
      let op =
        match uop with
        | Tincr -> Upincr
        | Tdecr -> Updecr
        | _ -> assert false
      in
      Pos.btw (fst e1) end_, Unop (op, e1)
    end
  in
  let x = L.token env.lb in
  if x = uop
  then expr_remain env e
  else (L.back env.lb; expr_remain env e)

(*****************************************************************************)
(* If expression: _?_:_ *)
(*****************************************************************************)

and is_colon_if env =
  look_ahead env begin fun env ->
    let tok = L.token env.lb in
    tok = Tcolon &&
    (* At this point, we might still be dealing with an xhp identifier *)
    L.no_space_id env.lb <> Tword
  end

and expr_if env e1 =
  reduce env e1 Tqm begin fun e1 env ->
    if is_colon_if env
    then colon_if env e1
    else ternary_if env e1
  end

and ternary_if env e1 =
  let e2 = expr { env with priority = 0 } in
  expect env Tcolon;
  let e3 = expr env in
  (match e1 with
  | pos, Eif _ ->
      error_at env pos "You should add parentheses"
  | _ -> ());
  Pos.btw (fst e1) (fst e3), Eif (e1, Some e2, e3)

and colon_if env e1 =
  expect env Tcolon;
  let e2 = expr env in
  Pos.btw (fst e1) (fst e2), Eif (e1, None, e2)


(*****************************************************************************)
(* Strings *)
(*****************************************************************************)

and expr_string env start abs_start =
  match L.string env.lb with
  | Tquote ->
      let pos = Pos.btw start (Pos.make env.lb) in
      let len = env.lb.Lexing.lex_curr_pos - abs_start - 1 in
      let content = String.sub env.lb.Lexing.lex_buffer abs_start len in
      pos, String (pos, content)
  | Teof ->
      error_at env start "string not closed";
      start, String (start, "")
  | _ -> assert false

and expr_encapsed env start =
  let abs_start = env.lb.Lexing.lex_curr_pos in
  let pos_start = Pos.make env.lb in
  let el = encapsed_nested pos_start env in
  let pos_end = Pos.make env.lb in
  let pos = Pos.btw pos_start pos_end in
  let len = env.lb.Lexing.lex_curr_pos - abs_start - 1 in
  let content = String.sub env.lb.Lexing.lex_buffer abs_start len in
  pos, String2 (el, (pos, content))

and encapsed_nested start env =
  match L.string2 env.lb with
  | Tdquote ->
      []
  | Teof ->
      error_at env start "string not properly closed";
      []
  | Tlcb when env.mode = Ast.Mdecl ->
      encapsed_nested start env
  | Tlcb ->
      (match L.string2 env.lb with
      | Tdollar ->
          error env "{ not supported";
          L.back env.lb;
          encapsed_nested start env
      | Tlvar ->
          L.back env.lb;
          let error_state = !(env.errors) in
          let e = encapsed_expr env in
          (match L.string2 env.lb with
          | Trcb -> ()
          | _ -> error_expect env "}");
          if !(env.errors) != error_state
          then [e]
          else e :: encapsed_nested start env
      | _ ->
          L.back env.lb;
          encapsed_nested start env
      )
  | Trcb ->
      encapsed_nested start env
  | Tdollar ->
      (match L.string2 env.lb with
      | Tlcb ->
          if env.mode = Ast.Mstrict
          then error env "${ not supported";
          let error_state = !(env.errors) in
          let result = (match L.string2 env.lb with
          | Tword ->
              (* The first token after ${ will lex as a word, but is actually
               * an lvar, so we need to fix it up. For example, "${foo}" should
               * be Lvar $foo, but will lex as Tdollar-Tlcb-Tword foo. *)
              let pos = Pos.make env.lb in
              let lvar = pos, Lvar (pos, "$" ^ Lexing.lexeme env.lb) in
              encapsed_expr_reduce pos env lvar
          | _ ->
              error_expect env "variable";
              Pos.make env.lb, Null) in
          expect env Trcb;
          if !(env.errors) != error_state
          then [result]
          else result :: encapsed_nested start env
      | _ ->
          L.back env.lb;
          encapsed_nested start env
      )
  | Tlvar ->
      L.back env.lb;
      let error_state = !(env.errors) in
      let e = encapsed_expr env in
      if !(env.errors) != error_state
      then [e]
      else e :: encapsed_nested start env
  | _ -> encapsed_nested start env

and encapsed_expr env =
  match L.string2 env.lb with
  | Tlcb when env.mode = Ast.Mdecl ->
      Pos.make env.lb, Null
  | Tquote ->
      let pos = Pos.make env.lb in
      let absolute_pos = env.lb.Lexing.lex_curr_pos in
      expr_string env pos absolute_pos
  | Tint ->
      let pos = Pos.make env.lb in
      let tok_value = Lexing.lexeme env.lb in
      pos, Int (pos, tok_value)
  | Tword ->
      let pid = Pos.make env.lb in
      let id = Lexing.lexeme env.lb in
      pid, (Id (pid, id))
  | Tlvar ->
      let pos = Pos.make env.lb in
      let lvar = pos, Lvar (pos, Lexing.lexeme env.lb) in
      encapsed_expr_reduce pos env lvar
  | _ ->
      error_expect env "expression";
      Pos.make env.lb, Null

and encapsed_expr_reduce start env e1 =
  let e1, continue = encapsed_expr_reduce_left start env e1 in
  if continue
  then encapsed_expr_reduce start env e1
  else e1

and encapsed_expr_reduce_left start env e1 =
  match L.string2 env.lb with
  | Tlb ->
      let e2 =
        match L.string2 env.lb with
        | Tword ->
            (* We need to special case this because any identifier
             * (including keywords) is allowed in this context.
             * For example: $x[function] is legal.
             *)
            let pid = Pos.make env.lb in
            let id = Lexing.lexeme env.lb in
            pid, (Id (pid, id))
        | _ ->
            L.back env.lb;
            expr { env with priority = 0 }
      in
      (match L.string2 env.lb with
      | Trb -> ()
      | _ -> error_expect env "]"
      );
      let pos = Pos.btw start (Pos.make env.lb) in
      (pos, Array_get (e1, Some e2)), true
  | Tarrow ->
      (match L.string2 env.lb with
      | Tword ->
          L.back env.lb;
          let e2 = encapsed_expr env in
          let pos = Pos.btw start (Pos.make env.lb) in
          (pos, Obj_get (e1, e2)), true
      | _ ->
          L.back env.lb;
          e1, false
      )
  | _ ->
      L.back env.lb;
      e1, false

(*****************************************************************************)
(* Heredocs *)
(*****************************************************************************)

and expr_heredoc env =
  let abs_start = env.lb.Lexing.lex_curr_pos in
  let tag = heredoc_tag env in
  heredoc_body tag env;
  let len = env.lb.Lexing.lex_curr_pos - abs_start - 1 in
  let content = String.sub env.lb.Lexing.lex_buffer abs_start len in
  fst tag, String (fst tag, content)

and heredoc_tag env =
  match L.token env.lb with
  | Tword ->
      Pos.make env.lb, Lexing.lexeme env.lb
  | Tquote ->
      let pos = Pos.make env.lb in
      let abs_pos = env.lb.Lexing.lex_curr_pos in
      (match expr_string env pos abs_pos with
      | _, String x -> x
      | _ -> assert false)
  | _ ->
      error_expect env "heredoc or nowdoc identifier";
      Pos.make env.lb, "HEREDOC"

and heredoc_body (pos, tag_value as tag) env =
  match L.heredoc_token env.lb with
  | Tnewline ->
      heredoc_end tag env
  | Teof ->
      error_expect env tag_value
  | _ ->
      heredoc_body tag env

and heredoc_end (pos, tag_value as tag) env =
  match L.heredoc_token env.lb with
  | Tword ->
      let tag2 = Lexing.lexeme env.lb in
      (match L.heredoc_token env.lb with
      | Tnewline when tag2 = tag_value ->
          ()
      | Tnewline ->
          heredoc_end tag env
      | Tsc when tag2 = tag_value ->
          L.back env.lb;
          ()
      | _ ->
          heredoc_body tag env
      )
  | Tnewline ->
      heredoc_end tag env
  | _ ->
      heredoc_body tag env


(*****************************************************************************)
(* Arrays *)
(*****************************************************************************)

and expr_array env pos =
  let fields = array_field_list env in
  pos, Array fields

and array_field_list env =
  expect env Tlp;
  array_field_list_remain env Trp []

and expr_short_array env pos =
  let fields = array_field_list_remain env Trb [] in
  pos, Array fields

and array_field_list_remain env terminal acc =
  match L.token env.lb with
  | x when x = terminal -> List.rev acc
  | _ ->
      L.back env.lb;
      let error_state = !(env.errors) in
      let fd = array_field env in
      let acc = fd :: acc in
      match L.token env.lb with
      | x when x = terminal ->
          List.rev acc
      | Tcomma ->
          if !(env.errors) != error_state
          then List.rev acc
          else array_field_list_remain env terminal acc
      | _ -> error_expect env ")"; [fd]

and array_field env =
  let env = { env with priority = 0 } in
  let e1 = expr env in
  match L.token env.lb with
  | Tsarrow ->
      let e2 = expr env in
      AFkvalue (e1, e2)
  | _ ->
      L.back env.lb;
      AFvalue e1

(*****************************************************************************)
(* Shapes *)
(*****************************************************************************)

and expr_shape env pos =
  let fields = shape_field_list env in
  pos, Shape fields

and shape_field_list env =
  expect env Tlp;
  shape_field_list_remain env

and shape_field_list_remain env =
  match L.token env.lb with
  | Trp -> []
  | _ ->
      L.back env.lb;
      let error_state = !(env.errors) in
      let fd = shape_field env in
      match L.token env.lb with
      | Trp ->
          [fd]
      | Tcomma ->
          if !(env.errors) != error_state
          then [fd]
          else fd :: shape_field_list_remain env
      | _ -> error_expect env ")"; [fd]

and shape_field env =
  let name = shape_field_name env in
  expect env Tsarrow;
  let value = expr { env with priority = 0 } in
  name, value

and shape_field_name env =
  expect env Tquote;
  let pos = Pos.make env.lb in
  let absolute_pos = env.lb.Lexing.lex_curr_pos in
  match expr_string env pos absolute_pos with
  | _, String s -> s
  | _ -> assert false

(*****************************************************************************)
(* Array access ($my_array[]|$my_array[_]) *)
(*****************************************************************************)

and expr_array_get env e1 =
  reduce env e1 Tlb begin fun e1 env ->
    match L.token env.lb with
    | Trb ->
        let end_ = Pos.make env.lb in
        Pos.btw (fst e1) end_, Array_get (e1, None)
    | _ ->
        L.back env.lb;
        let e2 = expr { env with priority = 0 } in
        expect env Trb;
        let end_ = Pos.make env.lb in
        Pos.btw (fst e1) end_, Array_get (e1, Some e2)
  end


(*****************************************************************************)
(* XHP *)
(*****************************************************************************)

and is_xhp env =
  look_ahead env begin fun env ->
    let tok = L.xhpname env.lb in
    tok = Txhpname &&
    let tok2 = L.xhpattr env.lb in
    tok2 = Tgt || tok2 = Tword ||
    (tok2 = Tslash && L.xhpattr env.lb = Tgt)
  end

and xhp env =
  match L.xhpname env.lb with
  | Txhpname ->
      let start = Pos.make env.lb in
      let name = Lexing.lexeme env.lb in
      let pname = start, ":"^name in
      let attrl, closed = xhp_attributes env in
      let end_tag = Pos.make env.lb in
      if closed
      then Pos.btw start end_tag, Xml (pname, attrl, [])
      else
        let tag_pos = Pos.btw start end_tag in
        let el = xhp_body tag_pos name env in
        let end_ = Pos.make env.lb in
        Pos.btw start end_, Xml (pname, attrl, el)
  | _ ->
      error_expect env "xhpname";
      let pos = Pos.make env.lb in
      pos, Xml ((pos, "xhp"), [], [])

and xhp_attributes env =
  match L.xhpattr env.lb with
  | Tslash ->
      if L.xhpattr env.lb <> Tgt
      then error_expect env ">";
      [], true
  | Tgt ->
      [], false
  | Tword ->
      let error_state = !(env.errors) in
      let attr_name = Pos.make env.lb, Lexing.lexeme env.lb in
      expect env Teq;
      let attr_value = xhp_attribute_value env in
      if !(env.errors) != error_state
      then
        [attr_name, attr_value], true
      else
        let rl, closed = xhp_attributes env in
        (attr_name, attr_value) :: rl, closed
  | _ ->
      error_expect env ">";
      [], true

and xhp_attribute_value env =
  match L.xhpattr env.lb with
  | Tlcb when env.mode = Ast.Mdecl ->
      ignore_body env;
      Pos.none, Null
  | Tlcb ->
      let result = expr { env with priority = 0 } in
      expect env Trcb;
      result
  | Tdquote ->
      let start = Pos.make env.lb in
      let abs_start = env.lb.Lexing.lex_curr_pos in
      xhp_attribute_string env start abs_start
  | _ ->
      error_expect env "attribute value";
      let pos = Pos.make env.lb in
      pos, String (pos, "")

and xhp_attribute_string env start abs_start =
  match L.string2 env.lb with
  | Teof ->
      error_at env start "Xhp attribute not closed";
      start, String (start, "")
  | Tdquote ->
      let len = env.lb.Lexing.lex_curr_pos - abs_start - 1 in
      let content = String.sub env.lb.Lexing.lex_buffer abs_start len in
      let pos = Pos.btw start (Pos.make env.lb) in
      pos, String (pos, content)
  | _ ->
      xhp_attribute_string env start abs_start

and xhp_body pos name env =
  match L.xhptoken env.lb with
  | Tlcb when env.mode = Ast.Mdecl ->
      ignore_body env;
      xhp_body pos name env
  | Tlcb ->
      let error_state = !(env.errors) in
      let e = expr { env with priority = 0 } in
      expect env Trcb;
      if !(env.errors) != error_state
      then [e]
      else e :: xhp_body pos name env
  | Tlt ->
      if is_xhp env
      then
        (match xhp env with
        | _, Xml (_, _, el) ->
            el @ xhp_body pos name env
        | _ -> xhp_body pos name env)
      else
        (match L.xhptoken env.lb with
        | Tslash ->
            let closing_tok = L.xhpname env.lb in
            let closing_name = Lexing.lexeme env.lb in
            if closing_tok = Txhpname &&
              (L.xhptoken env.lb = Tgt)
            then
              if closing_name = name
              then []
              else begin
                error_expect env name;
                []
              end
            else xhp_body pos name env
        | _ ->
            L.back env.lb;
            xhp_body pos name env
        )
  | Teof ->
      error_at env pos "Xhp tag not closed";
      []
  | Tword ->
      xhp_body pos name env
  | _ -> xhp_body pos name env

(*****************************************************************************)
(* Typedefs *)
(*****************************************************************************)

and typedef env =
  let id = identifier env in
  let tparams = class_params env in
  let tconstraint = typedef_constraint env in
  expect env Teq;
  let td = typedef_body env in
  expect env Tsc;
  id, tparams, tconstraint, td

and typedef_constraint env =
  match L.token env.lb with
  | Tword when Lexing.lexeme env.lb = "as" ->
      Some (hint env)
  | _ ->
      L.back env.lb;
      None

and typedef_body env =
  match L.token env.lb with
  | Tword when Lexing.lexeme env.lb = "shape" ->
      let pos = Pos.make env.lb in
      pos, Hshape (typedef_shape_field_list env)
  | _ -> L.back env.lb; hint env

and typedef_shape_field_list env =
  expect env Tlp;
  typedef_shape_field_list_remain env

and typedef_shape_field_list_remain env =
  match L.token env.lb with
  | Trp -> []
  | _ ->
      L.back env.lb;
      let error_state = !(env.errors) in
      let fd = typedef_shape_field env in
      match L.token env.lb with
      | Trp ->
          [fd]
      | Tcomma ->
          if !(env.errors) != error_state
          then [fd]
          else fd :: typedef_shape_field_list_remain env
      | _ ->
          error_expect env ")";
          [fd]

and typedef_shape_field env =
  let name = typedef_shape_field_name env in
  expect env Tsarrow;
  let ty = hint env in
  name, ty

and typedef_shape_field_name env =
  expect env Tquote;
  let pos = Pos.make env.lb in
  let absolute_pos = env.lb.Lexing.lex_curr_pos in
  match expr_string env pos absolute_pos with
  | _, String s -> s
  | _ -> assert false

(*****************************************************************************)
(* Namespaces *)
(*****************************************************************************)

and namespace env =
  (* The safety of the recursive calls here is slightly subtle. Normally, we
   * check for errors when making a recursive call to make sure we don't get
   * stuck in a loop. Here, we actually don't need to do that, since the only
   * time we make a recursive call is when we see (and thus consume) a token
   * that we like. So every time we recurse we'll consume at least one token,
   * so we can't get stuck in an infinite loop. *)
  let tl = match env.mode with
    | Ast.Mdecl -> ignore_toplevel ~attr:SMap.empty
    | _ -> toplevel in
  let id = identifier env in
  match L.token env.lb with
  | Tlcb ->
      let body = tl [] env (fun x -> x = Trcb) in
      expect env Trcb;
      id, body
  | Tsc ->
      let terminate = function
        | Tword -> Lexing.lexeme env.lb = "namespace"
        | Teof -> true
        | _ -> false in
      let body = tl [] env terminate in
      id, body
  | _ ->
      error_expect env "{ or ;";
      id, []

and namespace_use_list env acc =
  let p1, s1 = identifier env in
  let id1 = p1, if s1.[0] = '\\' then s1 else "\\" ^ s1 in
  let id2 =
    match L.token env.lb with
    | Tword when Lexing.lexeme env.lb = "as" ->
        identifier env
    | _ ->
        L.back env.lb;
        let str = snd id1 in
        let start = try (String.rindex str '\\') + 1 with Not_found -> 0 in
        let len = (String.length str) - start in
        fst id1, String.sub str start len
  in
  let acc = (id1, id2) :: acc in
  match L.token env.lb with
    | Tsc -> acc
    | Tcomma -> namespace_use_list env acc
    | _ ->
      error_expect env "Namespace use list";
      acc

(*****************************************************************************)
(* Helper *)
(*****************************************************************************)

let program ?(fail=true) content =
  snd (entry ~fail content)

let from_file_with_comments filename =
  let ic = open_in filename in
  let buf = Buffer.create 256 in
  Buffer.add_channel buf ic (in_channel_length ic);
  let content = Buffer.contents buf in
  close_in ic;
  entry ~fail:true content

let from_file filename =
  snd (from_file_with_comments filename)
