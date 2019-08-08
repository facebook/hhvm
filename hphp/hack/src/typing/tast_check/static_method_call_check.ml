(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Aast
open Typing_defs

module Env = Tast_env
module Cls = Decl_provider.Class

let rec is_abstract_ft fty = match fty with
  | _, Tfun { ft_abstract = true; _ } -> true
  | _r, Tintersection tyl -> List.for_all tyl ~f:is_abstract_ft
  | _ -> false

(* This check prevents calls to static methods that are abstract, except in cases where the method
 * is called with self::m() in a trait, in which case it is referring to method m on the class that
 * uses the trait. It also prevents calling static methods that are not declared, such as a static
 * method in a trait that comes from a require extends.
 * *)
let handler = object
  inherit Tast_visitor.handler_base

  method! at_expr env e =
    match e with
    | (p, _), Call (_, ((_, fty), Class_const ((_, e1), m)), _, _, _) ->
      begin match e1 with
      | CIself when is_abstract_ft fty ->
        begin match Env.get_self env with
          | Some (_, Tclass ((_, self), _, _)) ->
            (* at runtime, self:: in a trait is a call to whatever
             * self:: is in the context of the non-trait "use"-ing
             * the trait's code *)
            begin match Env.get_class env self with
              | Some cls when Cls.kind cls = Ast_defs.Ctrait -> ()
              | _ -> Errors.self_abstract_call (snd m) p (Reason.to_pos (fst fty))
            end
          | _ -> ()
        end
      | CIparent when is_abstract_ft fty ->
        Errors.parent_abstract_call (snd m) p (Reason.to_pos (fst fty))
      | CI c when is_abstract_ft fty ->
        Errors.classname_abstract_call (snd c) (snd m) p (Reason.to_pos (fst fty))
      | CI (_, classname) ->
        begin match Decl_provider.get_class classname with
        | Some class_def ->
          let (_, method_name) = m in
          begin match Cls.get_smethod class_def method_name with
          | None -> ()
          | Some elt ->
            if elt.ce_synthesized then
              Errors.static_synthetic_method classname (snd m) p (Reason.to_pos (fst fty))
          end
        | None ->
          (* This technically should be an error, but if we throw here we'll break a ton of our
          tests since they reference classes that only exist in www, and any missing classes will
          get caught elsewhere in the pipeline. *)
          ()
        end
      | _ -> ()
      end
    | _ -> ()
end
