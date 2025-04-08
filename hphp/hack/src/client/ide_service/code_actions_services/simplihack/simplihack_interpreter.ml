(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(* Value module represents the different types of values that can be produced during interpretation *)
module Value = struct
  type t =
    | Str of string  (** String values *)
    | Error  (** Represents evaluation errors *)

  (* Helper functions to safely extract values from the Value.t type.
     These functions are primarily used during evaluation to safely assert
     that a value is of an expected type (e.g., when evaluating a string
     concatenation, we need to assert both operands are strings).
     Returns None if the value is not of the expected type. *)

  let str = function
    | Str s -> Some s
    | _ -> None
end

(* Context module maintains the interpreter's state during evaluation *)
module Context = struct
  type t = {
    provider: Provider_context.t;
        (** Provides access to class/method definitions *)
    locals: Value.t Local_id.Map.t;
        (** Map of local variables to their values *)
  }

  (** [init provider] creates a new context with the given provider and empty locals map

      Example:
      {[
        let ctx = Context.init my_provider
        (* Creates new context with my_provider and empty locals *)
      ]}
  *)
  let init provider = { provider; locals = Local_id.Map.empty }

  (** [store ~ctx local value] returns a new context with [value] stored at [local] in the locals map

      Example:
      {[
        let new_ctx = Context.store ~ctx local_id my_value
        (* Creates new context with my_value stored at local_id *)
      ]}
  *)
  let store ~ctx local value =
    { ctx with locals = Local_id.Map.add local value ctx.locals }

  (** [load ~ctx local] looks up the value associated with [local] in the context's locals map
      Returns [None] if the local is not found

      Example:
      {[
        match Context.load ~ctx local_id with
        | Some value -> (* Use value *)
        | None -> (* Handle missing local *)
      ]}
  *)
  let load ~ctx local = Local_id.Map.find_opt local ctx.locals

  (** [fresh ctx] creates a new context with the same provider but empty locals

      Example:
      {[
        let clean_ctx = Context.fresh old_ctx
        (* Creates new context with old_ctx's provider but empty locals *)
      ]}
  *)
  let fresh ctx = init ctx.provider

  (** [find_class ~ctx name] looks up a class definition by name using the context's provider
      Returns [None] if the class is not found

      Example:
      {[
        match Context.find_class ~ctx "MyClass" with
        | Some class_def -> (* Use class definition *)
        | None -> (* Handle missing class *)
      ]}
  *)
  let find_class ~ctx name =
    let open Option.Let_syntax in
    let* path = Naming_provider.get_class_path ctx.provider name in
    let* class_ =
      Ast_provider.find_class_in_file ~full:true ctx.provider path name
    in
    return @@ Naming.class_ ctx.provider class_

  (** [find_method ~ctx class_ name] finds a method with [name] in [class_]
      Returns [None] if the method is not found

      Example:
      {[
        match Context.find_method ~ctx class_def "myMethod" with
        | Some method_def -> (* Use method definition *)
        | None -> (* Handle missing method *)
      ]}
  *)
  let find_method ~ctx:_ class_ name =
    List.find class_.Aast.c_methods ~f:(fun meth ->
        String.equal (snd meth.Aast.m_name) name)

  (** [find_function ~ctx name] looks up a function definition by name using the context's provider
      Returns [None] if the function is not found

      Example:
      {[
        match Context.find_function ~ctx "myFunction" with
        | Some func_def -> (* Use function definition *)
        | None -> (* Handle missing function *)
      ]}
  *)
  let find_function ~ctx name =
    let open Option.Let_syntax in
    let* path = Naming_provider.get_fun_path ctx.provider name in
    let* fun_def =
      Ast_provider.find_fun_in_file ~full:true ctx.provider path name
    in
    return @@ Naming.fun_def ctx.provider fun_def
end

(* ControlFlow module represents the possible outcomes of statement evaluation *)
module ControlFlow = struct
  type t =
    | Return of Value.t (* Return with a value *)
    | Next of Context.t (* Continue to next statement with updated context *)
    | Exit (* Exit evaluation *)
end

(* Mutually recursive modules for evaluating different language constructs *)

(* Expr module handles expression evaluation *)
module rec Expr : sig
  val eval : Context.t -> (_, _) Aast.expr -> Value.t
end = struct
  type t = Expr : (_, _) Aast.expr_ -> t

  (* Main expression evaluation function *)
  let rec expr (ctx : Context.t) (Expr e) =
    let expr ctx (_, _, e) = expr ctx (Expr e) in
    let open Option.Let_syntax in
    Option.value ~default:Value.Error
    @@
    match e with
    | Aast.String str ->
      (* Evaluates a string literal *)
      return @@ Value.Str str
    | Aast.String2 exprs ->
      let* value =
        List.fold
          ~init:(Some "")
          ~f:(fun acc e ->
            let* lhs = acc in
            let* rhs = Value.str @@ expr ctx e in
            return @@ lhs ^ rhs)
          exprs
      in
      return @@ Value.Str value
    | Aast.Nameof (_, _, Aast.CI (_, name)) -> return @@ Value.Str name
    | Aast.Lvar (_, name) ->
      (* Looks up value of a local variable *)
      Context.load ~ctx name
    | Aast.(Call { func; targs = _; args; unpacked_arg = None }) ->
      (* Evaluates a function/method call *)
      Call.eval ctx func args
    | Aast.(Binop { bop; lhs; rhs }) ->
      (* Handles binary operations *)
      Binop.eval ctx bop lhs rhs
    | _ -> None

  let eval (ctx : Context.t) (_, _, e) = expr ctx (Expr e)
end

(* Stmt module handles statement evaluation *)
and Stmt : sig
  val eval : Context.t -> (_, _) Aast.stmt -> ControlFlow.t
end = struct
  type t = Stmt : (_, _) Aast.stmt_ -> t

  let stmt (ctx : Context.t) (Stmt s) =
    match s with
    | Aast.Return (Some e) ->
      (* Return statement *)
      let result = Expr.eval ctx e in
      ControlFlow.Return result
    | Aast.Noop -> ControlFlow.Next ctx (* No-op statement *)
    | Aast.(Expr (_, _, Assign ((_, _, Lvar (_, lid)), None, e))) ->
      (* Assignment *)
      let ctx = Context.store ~ctx lid (Expr.eval ctx e) in
      ControlFlow.Next ctx
    | _ -> ControlFlow.Exit (* Other statements exit *)

  let eval (ctx : Context.t) (_, s) = stmt ctx (Stmt s)
end

(* Block module handles evaluation of statement blocks *)
and Block : sig
  val eval : Context.t -> (_, _) Aast.block -> Value.t option
end = struct
  type t = Block : (_, _) Aast.block -> t

  (* Recursively evaluate statements in a block *)
  let rec block (ctx : Context.t) (Block b) =
    let block ctx b = block ctx (Block b) in
    match b with
    | [] -> None
    | statement :: rest -> begin
      let open ControlFlow in
      match Stmt.eval ctx statement with
      | Next ctx -> block ctx rest (* Continue with next statement *)
      | Return value -> Some value (* Return from block *)
      | Exit -> None (* Exit evaluation *)
    end

  let eval (ctx : Context.t) b = block ctx (Block b)
end

(* ClassId module handles class identifier evaluation *)
and ClassId : sig
  val eval : Context.t -> (_, _) Aast.class_id -> Nast.class_ option
end = struct
  type t = ClassId : (_, _) Aast.class_id_ -> t

  let class_id (ctx : Context.t) (ClassId cid) =
    match cid with
    | Aast.(CIparent | CIself | CIstatic | CIexpr _) ->
      None (* Special class refs not supported *)
    | Aast.CI (_, name) -> Context.find_class ~ctx name (* Named class *)

  let eval (ctx : Context.t) (_, _, cid) = class_id ctx (ClassId cid)
end

(* Call module handles function/method calls *)
and Call : sig
  val eval :
    Context.t -> (_, _) Aast.expr -> (_, _) Aast.argument list -> Value.t option
end = struct
  (* Build new context for function call with arguments *)
  let build_ctx ~ctx params args =
    let eval = Expr.eval ctx in
    let rec go args params ctx =
      match (args, params) with
      | (Aast.Anormal arg :: args, Aast.{ param_name; _ } :: params) ->
        go args params
        @@ Context.store ~ctx (Local_id.make_unscoped param_name) (eval arg)
      | _ -> ctx
    in
    go args params @@ Context.fresh ctx

  let invoke ctx (params, body) args =
    let ctx = build_ctx ~ctx params args in
    let { Aast.fb_ast } = body in
    Block.eval ctx fb_ast

  (* Evaluate function call *)
  let eval
      (ctx : Context.t)
      (func : (_, _) Aast.expr)
      (args : (_, _) Aast.argument list) =
    let open Option.Let_syntax in
    let (_, _, func) = func in
    match func with
    | Aast.Id (_, name) ->
      let* Aast.{ fd_fun = { f_params = params; f_body = body; _ }; _ } =
        Context.find_function ~ctx name
      in
      invoke ctx (params, body) args
    | Aast.Class_const (cid, (_, name)) ->
      let* class_ = ClassId.eval ctx cid in
      let* Aast.{ m_params = params; m_body = body; _ } =
        Context.find_method ~ctx class_ name
      in
      invoke ctx (params, body) args
    | _ -> None
end

(* Binop module handles binary operations *)
and Binop : sig
  val eval :
    Context.t ->
    Ast_defs.bop ->
    (_, _) Aast.expr ->
    (_, _) Aast.expr ->
    Value.t option
end = struct
  let eval (ctx : Context.t) bop lhs_expr rhs_expr =
    match bop with
    | Ast_defs.Dot ->
      let open Option.Let_syntax in
      let* lhs_str = Value.str @@ Expr.eval ctx lhs_expr in
      let* rhs_str = Value.str @@ Expr.eval ctx rhs_expr in
      Some (Value.Str (lhs_str ^ rhs_str))
    | _ -> None
end

(* Top-level evaluation function for user attributes *)
let eval provider (e : Tast.expr) =
  let ctx = Context.init provider in
  Value.str @@ Expr.eval ctx e
