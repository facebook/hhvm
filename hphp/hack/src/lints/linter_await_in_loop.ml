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

class await_visitor =
  object (this)
    inherit [unit] Nast.Visitor_DEPRECATED.visitor as parent

    method on_elfun f =
      let block = f.f_body.fb_ast in
      (new loop_visitor)#on_block () block

    (* Skip lambdas created inside for loops *)
    method! on_efun () efun = this#on_elfun efun.ef_fun

    method! on_lfun () f _ = this#on_elfun f

    method! on_stmt () stmt =
      begin
        match snd stmt with
        | Expr
            (_, p, Binop { bop = Ast_defs.Eq _; lhs = _; rhs = (_, _, Await _) })
        | Expr (_, p, Await _) ->
          Lints_errors.await_in_loop p
        | _ -> ()
      end;
      parent#on_stmt () stmt
  end
  [@alert "-deprecated"]

and loop_visitor =
  object
    inherit [unit] Nast.Visitor_DEPRECATED.visitor as parent

    method! on_stmt () stmt =
      match snd stmt with
      | Do (block, _)
      | While (_, block)
      | For (_, _, _, block)
      | Foreach (_, As_v _, block)
      | Foreach (_, As_kv _, block) ->
        (new await_visitor)#on_block () block
      | _ -> parent#on_stmt () stmt
  end
  [@alert "-deprecated"]

module VisitorFunctor (Parent : BodyVisitorModule) : BodyVisitorModule = struct
  class visitor env =
    object
      inherit Parent.visitor env as parent

      method! on_body () block =
        (new loop_visitor)#on_block () block;
        parent#on_body () block
    end
end

let go = lint_all_bodies (module VisitorFunctor)
