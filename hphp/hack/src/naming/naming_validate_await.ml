(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast

type await_usage =
  | NoAwait
  | ContainsAwait
  | ConcurrentAwait
  | Sequential
  | Error of Pos.t option

let combine_con await1 await2 =
  match (await1, await2) with
  | (Sequential, NoAwait)
  | (NoAwait, Sequential) ->
    Sequential
  | (Error (Some pos), _) -> Error (Some pos)
  | (_, Error (Some pos)) -> Error (Some pos)
  | (Error None, _)
  | (_, Error None) ->
    Error None
  | (Sequential, _)
  | (_, Sequential) ->
    Error None
  | (ConcurrentAwait, _)
  | (_, ConcurrentAwait)
  | (ContainsAwait, ContainsAwait) ->
    ConcurrentAwait
  | (NoAwait, ContainsAwait)
  | (ContainsAwait, NoAwait) ->
    ContainsAwait
  | (NoAwait, NoAwait) -> NoAwait

let rec await_fold arg await = combine_con (check_await_usage arg) await

and await_fold_tuple : 'a. 'a * (unit, _) expr -> await_usage -> await_usage =
 (fun (_, arg) await -> combine_con (check_await_usage arg) await)

and check_await_usage expr =
  let ((), pos, e) = expr in
  let await_usage =
    match e with
    (* Expressions with no sub-expressions that we can lift an await out of *)
    | Dollardollar _
    | Null
    | This
    | True
    | False
    | Omitted
    | Id _
    | Lvar _
    | Int _
    | Float _
    | String _
    | Efun _
    | Lfun _
    | Lplaceholder _
    | Method_caller _
    | Invalid None
    | EnumClassLabel _
    | Class_const _
    | Nameof _
    | FunctionPointer _
    | Class_get _
    | Package _ ->
      NoAwait
    (* Expressions with exactly one sub-expression that we can lift an await out of *)
    | Invalid (Some expr)
    | Clone expr
    | PrefixedString (_, expr)
    | ReadonlyExpr expr
    | Cast (_, expr)
    | Is (expr, _)
    | As { expr; _ }
    | Upcast (expr, _)
    | Import (_, expr)
    | Hole (expr, _, _, _)
    | Yield (AFvalue expr)
    | ET_Splice expr
    | Unop (_, expr)
    | Array_get (expr, None) ->
      check_await_usage expr
    (* Expressions with exactly two sub-expressions that we can lift an await out of
       and run concurrently *)
    | Array_get (expr1, Some expr2)
    | Pair (_, expr1, expr2)
    | Obj_get (expr1, expr2, _, _)
    | Yield (AFkvalue (expr1, expr2))
    | Binop
        {
          bop = _;
          (* Can't have await in || or && (parse error) *)
          lhs = expr1;
          rhs = expr2;
        } ->
      combine_con (check_await_usage expr1) (check_await_usage expr2)
    (* Expressions with lists of sub-expressions that we can lift an await out of
       and run concurrently *)
    | KeyValCollection (_, _, args) ->
      List.fold_right args ~init:NoAwait ~f:(fun (arg_k, arg_v) await1 ->
          let await2 =
            combine_con (check_await_usage arg_k) (check_await_usage arg_v)
          in
          combine_con await1 await2)
    | Tuple args
    | String2 args
    | ValCollection (_, _, args) ->
      List.fold_right args ~init:NoAwait ~f:await_fold
    | Shape fields -> List.fold_right fields ~init:NoAwait ~f:await_fold_tuple
    | Call { func; targs = _; args; unpacked_arg } ->
      let res =
        List.fold_right args ~init:(check_await_usage func) ~f:await_fold_tuple
      in
      (match unpacked_arg with
      | None -> res
      | Some arg -> combine_con (check_await_usage arg) res)
    | New ((_, _, cid), _, args, unpacked_arg, _) ->
      let res =
        match cid with
        | CIexpr expr -> check_await_usage expr
        | CIparent
        | CIself
        | CIstatic
        | CI _ ->
          NoAwait
      in
      let res = List.fold_right args ~init:res ~f:await_fold in
      (match unpacked_arg with
      | None -> res
      | Some arg -> combine_con (check_await_usage arg) res)
    | Xml (_, attrs, exprs) ->
      let res =
        List.fold_right attrs ~init:NoAwait ~f:(fun attr await ->
            match attr with
            | Xhp_spread expr
            | Xhp_simple { xs_expr = expr; _ } ->
              combine_con (check_await_usage expr) await)
      in
      List.fold_right exprs ~init:res ~f:await_fold
    | Collection (_, _, afields) ->
      List.fold_right afields ~init:NoAwait ~f:(fun afield await ->
          let await1 =
            match afield with
            | AFvalue expr -> check_await_usage expr
            | AFkvalue (expr1, expr2) ->
              combine_con (check_await_usage expr1) (check_await_usage expr2)
          in
          combine_con await1 await)
    | Await expr1 ->
      (match check_await_usage expr1 with
      | NoAwait -> ContainsAwait
      | ContainsAwait
      | Sequential
      | ConcurrentAwait ->
        Sequential
      | Error p -> Error p)
    | Pipe (_, expr1, expr2) ->
      (match check_await_usage expr2 with
      | Error p -> Error p
      (* If the rhs contains an await, then we need to finish the lhs
         prior to doing the await *)
      | ContainsAwait
      | ConcurrentAwait
      | Sequential ->
        Sequential
      | NoAwait -> check_await_usage expr1)
    (* Can't have await in the branches (parse error) *)
    | Eif (cond, _, _) -> check_await_usage cond
    | ExpressionTree
        {
          et_class = _;
          et_splices = splices;
          et_function_pointers = _;
          et_virtualized_expr = _;
          et_runtime_expr = runtime_expr;
          et_dollardollar_pos = _;
        } ->
      List.fold_right
        splices
        ~init:(check_await_usage runtime_expr)
        ~f:(fun stmt await ->
          match stmt with
          | ( _,
              Expr ((), _, Binop { bop = Ast_defs.Eq None; lhs = _; rhs = expr })
            ) ->
            combine_con (check_await_usage expr) await
          | _ -> await)
    (* lvalues: shouldn't contain await or $$ *)
    | List _ -> NoAwait
  in
  match await_usage with
  | Error None -> Error (Some pos)
  | _ -> await_usage

let check_expr on_error expr =
  match check_await_usage expr with
  | Error p ->
    on_error
      (Naming_phase_error.parsing
         Parsing_error.(
           Parsing_error
             {
               pos =
                 (match p with
                 | Some pos -> pos
                 | None ->
                   let (_, pos, _) = expr in
                   pos);
               msg =
                 "Nested or piped `await` expressions cannot be used alongside other concurrent awaits";
               quickfixes = [];
             }))
  | NoAwait
  | ContainsAwait
  | ConcurrentAwait
  | Sequential ->
    ()

let visitor =
  object (_s)
    inherit [_] Aast.iter as super

    method! on_stmt on_error stmt =
      let (stmt_pos, stmt_) = stmt in
      match stmt_ with
      | Expr expr
      | Return (Some expr) ->
        super#on_expr on_error expr;
        check_expr on_error expr
      | Throw expr ->
        super#on_expr on_error expr;
        check_expr on_error expr
      | Return None -> ()
      | Using
          {
            us_is_block_scoped = _;
            us_has_await = _;
            us_exprs = (_, exprs);
            us_block;
          } ->
        List.iter exprs ~f:(fun expr ->
            super#on_expr on_error expr;
            check_expr on_error expr);
        super#on_block on_error us_block
      | If (expr, b1, b2) ->
        super#on_expr on_error expr;
        check_expr on_error expr;
        super#on_block on_error b1;
        super#on_block on_error b2
      | For (init_exprs, test, update, block) ->
        List.iter init_exprs ~f:(fun expr ->
            super#on_expr on_error expr;
            check_expr on_error expr);
        (* No await in the test or update positions (parse error) *)
        Option.iter ~f:(super#on_expr on_error) test;
        List.iter update ~f:(super#on_expr on_error);
        super#on_block on_error block
      | Switch (case, block, default) ->
        (* No await in the case expression positions (parse error) *)
        super#on_expr on_error case;
        check_expr on_error case;
        List.iter block ~f:(super#on_case on_error);
        Option.iter default ~f:(super#on_default_case on_error)
      | Foreach (expr, as_expr, block) ->
        super#on_expr on_error expr;
        check_expr on_error expr;
        super#on_as_expr on_error as_expr;
        super#on_block on_error block
      | Declare_local (_, _, expr_opt) ->
        (match expr_opt with
        | Some expr ->
          super#on_expr on_error expr;
          check_expr on_error expr
        | None -> ())
      (* await cannot appear in expression in do or while (parse error) *)
      | Do _
      | While _
      | Block _
      | Try _
      | Match _
      | Awaitall _ ->
        super#on_stmt on_error stmt
      | Noop
      | Fallthrough
      | Break
      | Continue
      | Yield_break
      | Markup _ ->
        ()
      | Concurrent stmts ->
        List.iter stmts ~f:(fun stmt ->
            match stmt with
            | (_, Expr expr)
            | (_, Declare_local (_, _, Some expr)) ->
              super#on_expr on_error expr;
              (match check_await_usage expr with
              | Sequential
              | Error None ->
                on_error
                  (Naming_phase_error.parsing
                     Parsing_error.(
                       Parsing_error
                         {
                           pos =
                             (let (_, pos, _) = expr in
                              pos);
                           msg =
                             "Nested or piped `await` expressions cannot be used in concurrent block";
                           quickfixes = [];
                         }))
              | Error (Some pos) ->
                on_error
                  (Naming_phase_error.parsing
                     Parsing_error.(
                       Parsing_error
                         {
                           pos;
                           msg =
                             "Nested or piped `await` expressions cannot be used in concurrent block";
                           quickfixes = [];
                         }))
              | _ -> ())
            | (_, Declare_local (_, _, None)) -> ()
            | _ ->
              failwith
                ("Concurrent block contains a statement that is not an expression-statement. "
                ^ Pos.string_no_file stmt_pos))
  end

let validate_fun_def on_error elem = visitor#on_fun_def on_error elem

let validate_class on_error elem = visitor#on_class_ on_error elem

let validate_program on_error elem = visitor#on_program on_error elem

let validate_stmt on_error elem = visitor#on_stmt on_error elem
