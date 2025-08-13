(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*
 * SimpliHack Interpreter
 *
 * This module implements an interpreter for the SimpliHack DSL, a simplified subset
 * of Hack used for code generation and metaprogramming tasks. The interpreter
 * evaluates expressions in user attributes and produces string outputs that can be
 * used as prompts or code snippets.
 *
 * Key components:
 * - Value: Represents runtime values (strings, unit)
 * - Context: Maintains interpreter state during evaluation
 * - ControlFlow: Handles statement execution flow
 * - Expr/Stmt/Block: Handle evaluation of different language constructs
 * - ClassId/Call/Binop: Support specific language features
 *
 * The interpreter follows a recursive descent pattern where each language
 * construct is handled by a dedicated module.
 *)

open Hh_prelude

(* StackFrame module represents a single frame in the call stack *)
module StackFrame = struct
  type t = {
    name: string;  (** Name of the function or method *)
    location: Pos.t;  (** Source location of the call, if available *)
  }
end

exception
  EvalError of {
    msg: string;
    pos: Pos.t;
    stack_trace: StackFrame.t list;
  }

(* Value module represents the different types of values that can be produced during interpretation *)
module Value = struct
  type t =
    | Str of string  (** String values *)
    | Unit  (** Represents the unit value *)
end

(* Context module maintains the interpreter's state during evaluation *)
module Context = struct
  type t = {
    env: Tast_env.t;
    locals: Value.t Local_id.Map.t;
        (** Map of local variables to their values *)
    stack_trace: StackFrame.t list;  (** The current call stack trace *)
  }

  (** [init provider dependent_root] creates a new context with the given provider, dependent_root, and empty locals map

      Example:
      {[
        let ctx = Context.init my_provider dependent_root
        (* Creates new context with my_provider, dependent_root, and empty locals *)
      ]}
  *)
  let init env = { env; locals = Local_id.Map.empty; stack_trace = [] }

  let env ctx = ctx.env

  let provider ctx = (Tast_env.get_decl_env ctx.env).Decl_env.ctx

  let dependent_root ctx = (Tast_env.get_decl_env ctx.env).Decl_env.droot

  (* Helper for raising evaluation errors *)
  let eval_error ~ctx ~pos ~msg =
    raise @@ EvalError { msg; pos; stack_trace = ctx.stack_trace }

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
      Raises [EvalError] if the local is not found

      Example:
      {[
        let value = Context.load ~ctx local_id
        (* Use value, or handle EvalError if local not found *)
      ]}
  *)
  let load ~ctx (pos, local) =
    match Local_id.Map.find_opt local ctx.locals with
    | Some value -> value
    | None ->
      eval_error
        ~ctx
        ~pos
        ~msg:
          (Printf.sprintf "The local `%s` is not defined"
          @@ Local_id.get_name local)

  (** [fresh ctx] creates a new context with the same provider, dependent_root, and stack_trace but empty locals

      Example:
      {[
        let clean_ctx = Context.fresh old_ctx
        (* Creates new context with old_ctx's provider, dependent_root, and stack_trace but empty locals *)
      ]}
  *)
  let fresh ctx = { ctx with locals = Local_id.Map.empty }

  (** [push_stack_frame ~ctx name pos] returns a new context with a new stack frame with a fresh set of locals

      Example:
      {[
        let new_ctx = Context.push_stack_frame ~ctx "my_function" (Some pos)
        (* Creates new context with a new stack frame for "my_function" *)
      ]}
  *)
  let push_stack_frame ~ctx name location =
    let frame = { StackFrame.name; location } in
    { ctx with stack_trace = frame :: ctx.stack_trace } |> fresh

  let add_dep ctx dep =
    match dependent_root ctx with
    | None -> ()
    | Some root ->
      let mode = Provider_context.get_deps_mode (provider ctx) in
      Typing_deps.add_idep mode root dep

  let add_file_dep ctx path = add_dep ctx Typing_deps.Dep.(File path)

  (** [find_class ~ctx name] looks up a class definition by name using the context's provider
      Raises [EvalError] if the class is not found

      Example:
      {[
        let class_def = Context.find_class ~ctx "MyClass"
        (* Use class definition, or handle EvalError if class not found *)
      ]}
  *)
  let find_class ~ctx (pos, name) =
    add_dep ctx Typing_deps.Dep.(Type name);
    let cls =
      let open Option.Let_syntax in
      let provider = provider ctx in
      let* path = Naming_provider.get_class_path provider name in
      add_file_dep ctx path;
      let* class_ =
        Ast_provider.find_class_in_file ~full:true provider path name
      in
      return @@ Naming.class_ provider class_
    in
    match cls with
    | Some cls -> cls
    | None ->
      eval_error
        ~ctx
        ~pos
        ~msg:
          (Printf.sprintf "The type `%s` is not defined" @@ Utils.strip_ns name)

  (** [find_static_method ~ctx class_ name] finds a method with [name] in [class_]
      Raises [EvalError] if the method is not found

      Example:
      {[
        let method_def = Context.find_static_method ~ctx class_def "myMethod"
        (* Use method definition, or handle EvalError if method not found *)
      ]}

      Note: This will only find static methods declared directly on the class
      and not inherited methods. If we wanted to support inherited methods we
      would need to use [Folded_class.t] to look up the method.

      For now this is an intentional choice to avoid absorbing too many Hack
      specific concepts into the SimpliHack DSL.
  *)
  let find_static_method ~ctx class_ (pos, name) =
    add_dep ctx Typing_deps.Dep.(SMethod (snd class_.Aast.c_name, name));
    match
      List.find class_.Aast.c_methods ~f:(fun meth ->
          String.equal (snd meth.Aast.m_name) name)
    with
    | Some meth -> meth
    | None ->
      eval_error
        ~ctx
        ~pos
        ~msg:
          (Printf.sprintf "The method `%s` is not defined in `%s`" name
          @@ Utils.strip_ns (snd class_.Aast.c_name))

  (** [find_function ~ctx name] looks up a function definition by name using the context's provider
      Raises [EvalError] if the function is not found

      Example:
      {[
        let func_def = Context.find_function ~ctx "myFunction"
        (* Use function definition, or handle EvalError if function not found *)
      ]}
  *)
  let find_function ~ctx (pos, name) =
    add_dep ctx Typing_deps.Dep.(Fun name);
    let func =
      let open Option.Let_syntax in
      let provider = provider ctx in
      let* path = Naming_provider.get_fun_path provider name in
      add_file_dep ctx path;
      let* fun_def =
        Ast_provider.find_fun_in_file ~full:true provider path name
      in
      return @@ Naming.fun_def provider fun_def
    in
    match func with
    | Some func -> func
    | None ->
      eval_error
        ~ctx
        ~pos
        ~msg:(Printf.sprintf "Function `%s` not found" @@ Utils.strip_ns name)

  module Class = struct
    type ctx = t

    type t = ctx * Folded_class.t

    let find ~ctx (pos, name) =
      add_dep ctx Typing_deps.Dep.(Type name);
      let cls = Tast_env.get_class ctx.env name |> Decl_entry.to_option in
      match cls with
      | Some cls -> (ctx, cls)
      | None ->
        eval_error
          ~ctx
          ~pos
          ~msg:
            (Printf.sprintf "The type `%s` is not defined"
            @@ Utils.strip_ns name)

    let constructor (ctx, cls) =
      add_dep ctx Typing_deps.Dep.(Constructor (Folded_class.name cls));
      let tenv = Tast_env.tast_env_as_typing_env ctx.env in
      let (constructor, _) = Typing_env.get_construct tenv cls in
      constructor

    let methods (ctx, cls) =
      add_dep ctx Typing_deps.Dep.(AllMembers (Folded_class.name cls));
      Folded_class.methods cls

    let static_methods (ctx, cls) =
      add_dep ctx Typing_deps.Dep.(AllMembers (Folded_class.name cls));
      Folded_class.smethods cls

    let fields (ctx, cls) =
      add_dep ctx Typing_deps.Dep.(AllMembers (Folded_class.name cls));
      Folded_class.props cls

    let static_fields (ctx, cls) =
      add_dep ctx Typing_deps.Dep.(AllMembers (Folded_class.name cls));
      Folded_class.sprops cls
  end
end

module PromptCtx = Simplihack_prompt_context_provider.Make (Context)

(* ControlFlow module represents the possible outcomes of statement evaluation *)
module ControlFlow = struct
  type t =
    | Return of Value.t (* Return with a value *)
    | Next of Context.t (* Continue to next statement with updated context *)
end

(*
 * Note on mutual recursion:
 * The following modules (Expr, Stmt, Block, ClassId, Call, Binop) are mutually
 * recursive. This structure is required by OCaml to handle circular dependencies.
 *
 * Evaluation flow:
 * - Expr.eval: Evaluates expressions (may call Call.eval for function calls)
 * - Call.eval: Evaluates function calls (may call Block.eval for function bodies)
 * - Block.eval: Evaluates blocks of statements (calls Stmt.eval for each statement)
 * - Stmt.eval: Evaluates statements (may call Expr.eval for expressions)
 *)

(* Mutually recursive modules for evaluating different language constructs *)

(* Expr module handles expression evaluation *)
module rec Expr : sig
  val eval : Context.t -> (_, _) Aast.expr -> Value.t

  val eval_str : Context.t -> (_, _) Aast.expr -> string
end = struct
  type t = Expr : Pos.t * (_, _) Aast.expr_ -> t

  (* Main expression evaluation function *)
  let rec expr (ctx : Context.t) (Expr (pos, e)) =
    let str_expr ctx (_, pos, e) = str_expr ctx (Expr (pos, e)) in
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
            let rhs = str_expr ctx e in
            lhs ^ rhs)
          exprs
      in
      Value.Str value
    | Aast.Nameof (_, _, Aast.CI (_, name)) -> Value.Str name
    | Aast.Lvar name ->
      (* Looks up value of a local variable *)
      Context.load ~ctx name
    | Aast.(Call { func; targs = _; args; unpacked_arg = None }) ->
      (* Evaluates a function/method call *)
      Call.eval ctx pos func args
    | Aast.(Binop { bop; lhs; rhs }) ->
      (* Handles binary operations *)
      Binop.eval ctx pos bop lhs rhs
    | _ -> Context.eval_error ~ctx ~pos ~msg:"Unsupported expression"

  (* Helper functions to safely extract values from the Value.t type.
     These functions are primarily used during evaluation to safely assert
     that a value is of an expected type (e.g., when evaluating a string
     concatenation, we need to assert both operands are strings).
     Returns None if the value is not of the expected type. *)

  and str_expr ctx e =
    match expr ctx e with
    | Value.Str s -> s
    | Value.Unit ->
      let (Expr (pos, _)) = e in
      Context.eval_error ~ctx ~pos ~msg:"Expected string value but got no value"

  let eval (ctx : Context.t) (_, pos, e) = expr ctx (Expr (pos, e))

  let eval_str (ctx : Context.t) (_, pos, e) = str_expr ctx (Expr (pos, e))
end

(* Stmt module handles statement evaluation *)
and Stmt : sig
  val eval : Context.t -> (_, _) Aast.stmt -> ControlFlow.t
end = struct
  type t = Stmt : Pos.t * (_, _) Aast.stmt_ -> t

  let stmt (ctx : Context.t) (Stmt (pos, s)) =
    match s with
    | Aast.Return (Some e) ->
      (* Return statement *)
      let result = Expr.eval ctx e in
      ControlFlow.Return result
    | Aast.Return None -> ControlFlow.Return Value.Unit
    | Aast.Noop -> ControlFlow.Next ctx (* No-op statement *)
    | Aast.(Expr (_, _, Assign ((_, _, Lvar (_, lid)), None, e))) ->
      (* Assignment *)
      let ctx = Context.store ~ctx lid (Expr.eval ctx e) in
      ControlFlow.Next ctx
    | Aast.(Expr e) ->
      (* Expression statement *)
      let _ = Expr.eval ctx e in
      ControlFlow.Next ctx
    | _ -> Context.eval_error ~ctx ~pos ~msg:"Unsupported statement"

  let eval (ctx : Context.t) (pos, s) = stmt ctx (Stmt (pos, s))
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
    end

  let eval (ctx : Context.t) b = block ctx (Block b)
end

(* ClassId module handles class identifier evaluation *)
and ClassId : sig
  val eval : Context.t -> (_, _) Aast.class_id -> Nast.class_
end = struct
  type t = ClassId : Pos.t * (_, _) Aast.class_id_ -> t

  let class_id (ctx : Context.t) (ClassId (pos, cid)) =
    match cid with
    | Aast.(CIparent | CIself | CIstatic | CIexpr _) ->
      (* Special class refs not supported *)
      Context.eval_error ~ctx ~pos ~msg:"Unsupported class reference"
    | Aast.CI name -> Context.find_class ~ctx name (* Named class *)

  let eval (ctx : Context.t) (_, pos, cid) = class_id ctx (ClassId (pos, cid))
end

(* Call module handles function/method calls *)
and Call : sig
  val eval :
    Context.t ->
    Pos.t ->
    (_, _) Aast.expr ->
    (_, _) Aast.argument list ->
    Value.t
end = struct
  (* Build new context for function call with arguments *)
  let build_ctx ~ctx ~name ~pos params args =
    let eval = Expr.eval ctx in
    let ctx = Context.push_stack_frame ~ctx name pos in
    let rec go args params ctx =
      match (args, params) with
      | (Aast.Anormal arg :: args, Aast.{ param_name; _ } :: params) ->
        go args params
        @@ Context.store ~ctx (Local_id.make_unscoped param_name) (eval arg)
      (* TODO(named_params): Better handling needed here. *)
      | (Aast.Anamed (_, _) :: _, _) -> ctx
      | _ -> ctx
    in
    go args params ctx

  let invoke ctx ~name ~pos (params, body) args =
    let ctx = build_ctx ~ctx ~name ~pos params args in
    let { Aast.fb_ast } = body in
    let result = Block.eval ctx fb_ast in
    result

  (* Evaluate function call *)
  let eval
      (ctx : Context.t)
      (pos : Pos.t)
      (func : (_, _) Aast.expr)
      (args : (_, _) Aast.argument list) =
    let (_, _, func) = func in
    let invoke = invoke ctx ~pos in
    match (func, args) with
    | (Aast.Id (_, id), [Aast.Anormal arg])
      when String.equal id Naming_special_names.SimpliHack.file ->
      let path = Expr.eval_str ctx arg in
      let path = Relative_path.from_root ~suffix:path in
      Context.add_file_dep ctx path;
      let contents =
        match File_provider.get_contents path with
        | Some contents -> contents
        | None ->
          Context.eval_error
            ~ctx
            ~pos
            ~msg:
              (Printf.sprintf "Could not read %s"
              @@ Relative_path.to_absolute path)
      in
      Value.Str contents
    | (Aast.Id (_, id), [Aast.Anormal arg])
      when String.equal id Naming_special_names.SimpliHack.constructor ->
      let classname = Expr.eval_str ctx arg in
      let data = PromptCtx.Class.constructor ctx (snd3 arg, classname) in
      Value.Str data
    | (Aast.Id (_, id), [Aast.Anormal arg])
      when String.equal id Naming_special_names.SimpliHack.methods ->
      let classname = Expr.eval_str ctx arg in
      let data = PromptCtx.Class.methods ctx (snd3 arg, classname) in
      Value.Str data
    | (Aast.Id (_, id), [Aast.Anormal arg])
      when String.equal id Naming_special_names.SimpliHack.static_methods ->
      let classname = Expr.eval_str ctx arg in
      let data = PromptCtx.Class.static_methods ctx (snd3 arg, classname) in
      Value.Str data
    | (Aast.Id (_, id), [Aast.Anormal arg])
      when String.equal id Naming_special_names.SimpliHack.fields ->
      let classname = Expr.eval_str ctx arg in
      let data = PromptCtx.Class.fields ctx (snd3 arg, classname) in
      Value.Str data
    | (Aast.Id (_, id), [Aast.Anormal arg])
      when String.equal id Naming_special_names.SimpliHack.static_fields ->
      let classname = Expr.eval_str ctx arg in
      let data = PromptCtx.Class.static_fields ctx (snd3 arg, classname) in
      Value.Str data
    | (Aast.Id name, args) ->
      let Aast.{ fd_fun = { f_params = params; f_body = body; _ }; _ } =
        Context.find_function ~ctx name
      in
      invoke ~name:(snd name) (params, body) args
    | (Aast.Class_const (cid, name), args) ->
      let class_ = ClassId.eval ctx cid in
      let class_name = snd class_.Aast.c_name in
      let method_name = class_name ^ "::" ^ snd name in
      let Aast.{ m_params = params; m_body = body; _ } =
        Context.find_static_method ~ctx class_ name
      in
      invoke ~name:method_name (params, body) args
    | _ -> Context.eval_error ~ctx ~pos ~msg:"Unsupported call"
end

(* Binop module handles binary operations *)
and Binop : sig
  val eval :
    Context.t ->
    Pos.t ->
    Ast_defs.bop ->
    (_, _) Aast.expr ->
    (_, _) Aast.expr ->
    Value.t
end = struct
  let eval (ctx : Context.t) pos bop lhs_expr rhs_expr =
    match bop with
    | Ast_defs.Dot ->
      let lhs_str = Expr.eval_str ctx lhs_expr in
      let rhs_str = Expr.eval_str ctx rhs_expr in
      Value.Str (lhs_str ^ rhs_str)
    | _ -> Context.eval_error ~ctx ~pos ~msg:"Unsupported binary operation"
end

(* Top-level evaluation function for user attributes *)
let eval env (e : Tast.expr) =
  let ctx = Context.init env in
  try Result.Ok (Expr.eval_str ctx e) with
  | EvalError { msg; pos; stack_trace } ->
    let f { StackFrame.name; location } =
      (Pos_or_decl.of_raw_pos location, "via " ^ Utils.strip_ns name ^ "(...)")
    in
    Result.Error
      (Typing_error.Primary.SimpliHack.Evaluation_error
         {
           pos = Tast.get_position e;
           stack_trace =
             lazy ((Pos_or_decl.of_raw_pos pos, msg) :: List.map ~f stack_trace);
         })
