(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

exception EvalError

(* Value module represents the different types of values that can be produced during interpretation *)
module Value = struct
  type t =
    | Str of string  (** String values *)
    | Unit  (** Represents the unit value *)

  (* Helper functions to safely extract values from the Value.t type.
     These functions are primarily used during evaluation to safely assert
     that a value is of an expected type (e.g., when evaluating a string
     concatenation, we need to assert both operands are strings).
     Returns None if the value is not of the expected type. *)

  let str = function
    | Str s -> s
    | _ -> raise EvalError
end

(* Context module maintains the interpreter's state during evaluation *)
module Context = struct
  type t = {
    provider: Provider_context.t;
        (** Provides access to class/method definitions *)
    locals: Value.t Local_id.Map.t;
        (** Map of local variables to their values *)
    dependent_root: Typing_deps.Dep.dependent Typing_deps.Dep.variant option;
        (** The dependent node we will add dependencies to *)
  }

  (** [init provider dependent_root] creates a new context with the given provider, dependent_root, and empty locals map

      Example:
      {[
        let ctx = Context.init my_provider dependent_root
        (* Creates new context with my_provider, dependent_root, and empty locals *)
      ]}
  *)
  let init provider dependent_root =
    { provider; locals = Local_id.Map.empty; dependent_root }

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
  let load ~ctx local =
    match Local_id.Map.find_opt local ctx.locals with
    | Some value -> value
    | None -> raise EvalError

  (** [fresh ctx] creates a new context with the same provider and dependent_root but empty locals

      Example:
      {[
        let clean_ctx = Context.fresh old_ctx
        (* Creates new context with old_ctx's provider and dependent_root but empty locals *)
      ]}
  *)
  let fresh ctx = { ctx with locals = Local_id.Map.empty }

  let add_dep ctx dep =
    match ctx.dependent_root with
    | None -> ()
    | Some root ->
      let mode = Provider_context.get_deps_mode ctx.provider in
      Typing_deps.add_idep mode root dep

  let add_file_dep ctx path = add_dep ctx Typing_deps.Dep.(File path)

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
    add_dep ctx Typing_deps.Dep.(Type name);
    let cls =
      let open Option.Let_syntax in
      let* path = Naming_provider.get_class_path ctx.provider name in
      add_file_dep ctx path;
      let* class_ =
        Ast_provider.find_class_in_file ~full:true ctx.provider path name
      in
      return @@ Naming.class_ ctx.provider class_
    in
    match cls with
    | Some cls -> cls
    | None -> raise EvalError

  (** [find_static_method ~ctx class_ name] finds a method with [name] in [class_]
      Returns [None] if the method is not found

      Example:
      {[
        match Context.find_static_method ~ctx class_def "myMethod" with
        | Some method_def -> (* Use method definition *)
        | None -> (* Handle missing method *)
      ]}

      Note: This will only find static methods declared directly on the class
      and not inherited methods. If we wanted to support inherited methods we
      would need to use [Folded_class.t] to look up the method.

      For now this is an intentional choice to avoid absorbing too many Hack
      specific concepts into the SimpliHack DSL.
  *)
  let find_static_method ~ctx class_ name =
    add_dep ctx Typing_deps.Dep.(SMethod (snd class_.Aast.c_name, name));
    match
      List.find class_.Aast.c_methods ~f:(fun meth ->
          String.equal (snd meth.Aast.m_name) name)
    with
    | Some meth -> meth
    | None -> raise EvalError

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
    add_dep ctx Typing_deps.Dep.(Fun name);
    let func =
      let open Option.Let_syntax in
      let* path = Naming_provider.get_fun_path ctx.provider name in
      add_file_dep ctx path;
      let* fun_def =
        Ast_provider.find_fun_in_file ~full:true ctx.provider path name
      in
      return @@ Naming.fun_def ctx.provider fun_def
    in
    match func with
    | Some func -> func
    | None -> raise EvalError
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
    match e with
    | Aast.String str ->
      (* Evaluates a string literal *)
      Value.Str str
    | Aast.String2 exprs ->
      let value =
        List.fold
          ~init:""
          ~f:(fun acc e ->
            let lhs = acc in
            let rhs = Value.str @@ expr ctx e in
            lhs ^ rhs)
          exprs
      in
      Value.Str value
    | Aast.Nameof (_, _, Aast.CI (_, name)) -> Value.Str name
    | Aast.Lvar (_, name) ->
      (* Looks up value of a local variable *)
      Context.load ~ctx name
    | Aast.(Call { func; targs = _; args; unpacked_arg = None }) ->
      (* Evaluates a function/method call *)
      Call.eval ctx func args
    | Aast.(Binop { bop; lhs; rhs }) ->
      (* Handles binary operations *)
      Binop.eval ctx bop lhs rhs
    | _ -> raise EvalError

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
    | Aast.(Expr e) ->
      (* Expression statement *)
      let _ = Expr.eval ctx e in
      ControlFlow.Next ctx
    | _ -> ControlFlow.Exit (* Other statements exit *)

  let eval (ctx : Context.t) (_, s) = stmt ctx (Stmt s)
end

(* Block module handles evaluation of statement blocks *)
and Block : sig
  val eval : Context.t -> (_, _) Aast.block -> Value.t
end = struct
  type t = Block : (_, _) Aast.block -> t

  (* Recursively evaluate statements in a block *)
  let rec block (ctx : Context.t) (Block b) =
    let block ctx b = block ctx (Block b) in
    match b with
    | [] -> Value.Unit
    | statement :: rest -> begin
      let open ControlFlow in
      match Stmt.eval ctx statement with
      | Next ctx -> block ctx rest (* Continue with next statement *)
      | Return value -> value (* Return from block *)
      | Exit -> Value.Unit (* Exit evaluation *)
    end

  let eval (ctx : Context.t) b = block ctx (Block b)
end

(* ClassId module handles class identifier evaluation *)
and ClassId : sig
  val eval : Context.t -> (_, _) Aast.class_id -> Nast.class_
end = struct
  type t = ClassId : (_, _) Aast.class_id_ -> t

  let class_id (ctx : Context.t) (ClassId cid) =
    match cid with
    | Aast.(CIparent | CIself | CIstatic | CIexpr _) ->
      raise EvalError (* Special class refs not supported *)
    | Aast.CI (_, name) -> Context.find_class ~ctx name (* Named class *)

  let eval (ctx : Context.t) (_, _, cid) = class_id ctx (ClassId cid)
end

(* Call module handles function/method calls *)
and Call : sig
  val eval :
    Context.t -> (_, _) Aast.expr -> (_, _) Aast.argument list -> Value.t
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
    let (_, _, func) = func in
    match (func, args) with
    | (Aast.Id (_, id), [Aast.Anormal arg])
      when String.equal id Naming_special_names.SimpliHack.file ->
      let path = Value.str @@ Expr.eval ctx arg in
      let path = Relative_path.from_root ~suffix:path in
      Context.add_file_dep ctx path;
      let contents =
        match File_provider.get_contents path with
        | Some contents -> contents
        | None -> raise EvalError
      in
      Value.Str contents
    | (Aast.Id (_, name), args) ->
      let Aast.{ fd_fun = { f_params = params; f_body = body; _ }; _ } =
        Context.find_function ~ctx name
      in
      invoke ctx (params, body) args
    | (Aast.Class_const (cid, (_, name)), args) ->
      let class_ = ClassId.eval ctx cid in
      let Aast.{ m_params = params; m_body = body; _ } =
        Context.find_static_method ~ctx class_ name
      in
      invoke ctx (params, body) args
    | _ -> raise EvalError
end

(* Binop module handles binary operations *)
and Binop : sig
  val eval :
    Context.t -> Ast_defs.bop -> (_, _) Aast.expr -> (_, _) Aast.expr -> Value.t
end = struct
  let eval (ctx : Context.t) bop lhs_expr rhs_expr =
    match bop with
    | Ast_defs.Dot ->
      let lhs_str = Value.str @@ Expr.eval ctx lhs_expr in
      let rhs_str = Value.str @@ Expr.eval ctx rhs_expr in
      Value.Str (lhs_str ^ rhs_str)
    | _ -> raise EvalError
end

(* Top-level evaluation function for user attributes *)
let eval env (e : Tast.expr) =
  let Decl_env.{ ctx; droot; _ } = Tast_env.get_decl_env env in
  let ctx = Context.init ctx droot in
  try Some (Value.str @@ Expr.eval ctx e) with
  | EvalError -> None
