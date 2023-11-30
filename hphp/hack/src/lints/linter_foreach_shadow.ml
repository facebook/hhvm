(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Linting_visitors
open Aast

(**
   Lint on loop or catch variables that shadow local variables.

   function foo(): void {
     $x = 99;

     // Lint on this $x:
     foreach (vec[1, 2] as $x) {
     }

     // $x is 2 here.
     $x;
   }
 *)

let lvar_visitor =
  (object
     inherit [lid list] Nast.Visitor_DEPRECATED.visitor

     method! on_lvar ids id = id :: ids
  end
  [@alert "-deprecated"])

let expr_lvars = lvar_visitor#on_expr []

let as_expr_lvars = lvar_visitor#on_as_expr []

module VisitorFunctor (Parent : BodyVisitorModule) : BodyVisitorModule = struct
  class visitor env =
    object (this)
      inherit Parent.visitor env as parent

      val mutable frames : lid list list = [[]]

      method private env = List.concat frames

      method private extend_env vars =
        frames <- List.rev_append vars (List.hd frames) :: List.tl frames

      method private push_empty_frame = frames <- [] :: frames

      method private pop_frame = frames <- List.tl frames

      method private on_block_with_local_vars vars block =
        this#push_empty_frame;
        this#extend_env vars;
        parent#on_block () block;
        this#pop_frame

      method! on_block () = this#on_block_with_local_vars []

      method! on_stmt () stmt =
        begin
          match snd stmt with
          | Expr (_, _, Binop { bop = Ast_defs.Eq _; lhs = e; _ }) ->
            this#extend_env (expr_lvars e)
          | _ -> ()
        end;
        parent#on_stmt () stmt

      method! on_foreach () expr as_expr block =
        this#on_expr () expr;
        this#on_as_expr () as_expr;
        this#on_block_with_local_vars (as_expr_lvars as_expr) block

      method! on_as_expr () as_expr =
        let env = this#env in
        List.iter
          (fun (p1, id1) ->
            if String.equal (Local_id.get_name id1) "$_" then
              ()
            else
              match List.find_opt (fun (_, id2) -> id1 = id2) env with
              | None -> ()
              | Some (p2, _) ->
                Lints_errors.loop_variable_shadows_local_variable p1 id1 p2)
          (as_expr_lvars as_expr);
        parent#on_as_expr () as_expr

      method! on_catch () (_, var, block) =
        this#on_block_with_local_vars [var] block

      method! on_expr () ((_, _, e_) as e) =
        match e_ with
        | Efun _
        | Lfun _ ->
          let old_frames = frames in
          frames <- [[]];
          parent#on_expr () e;
          frames <- old_frames
        | _ -> parent#on_expr () e
    end
end

let go = lint_all_bodies (module VisitorFunctor)
