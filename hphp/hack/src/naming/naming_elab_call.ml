(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error
module SN = Naming_special_names

module Env = struct
  type t = { current_cls: (Ast_defs.id * Ast_defs.classish_kind * bool) option }

  let empty = { current_cls = None }

  let in_class Aast.{ c_name; c_kind; c_final; _ } =
    { current_cls = Some (c_name, c_kind, c_final) }
end

let visitor =
  object (self)
    inherit [_] Aast_defs.mapreduce as super

    inherit Err.monoid

    method! on_Call
        env
        ((_, fn_expr_pos, fn_expr_) as fn_expr)
        targs
        fn_param_exprs
        fn_unpacked_expr_opt =
      match fn_expr_ with
      | Aast.Id (_, fn_name) ->
        let res =
          if String.equal fn_name SN.SpecialFunctions.echo then
            let unpacked_err =
              Option.value_map
                ~default:self#zero
                ~f:(fun (_, pos, _) ->
                  Err.naming @@ Naming_error.Too_few_arguments pos)
                fn_unpacked_expr_opt
            in
            let (fn_param_exprs, fn_param_err) =
              super#on_list (self#on_snd super#on_expr) env fn_param_exprs
            and (fn_unpacked_expr_opt, fn_unpacked_expr_err) =
              super#on_option super#on_expr env fn_unpacked_expr_opt
            in
            let err =
              self#plus_all [unpacked_err; fn_param_err; fn_unpacked_expr_err]
            in
            let expr_ =
              Aast.Call (fn_expr, targs, fn_param_exprs, fn_unpacked_expr_opt)
            in
            Ok (expr_, err)
          else if String.equal fn_name SN.StdlibFunctions.call_user_func then
            let depr_err =
              Err.typing
              @@ Typing_error.(
                   Primary.Deprecated_use
                     {
                       pos = fn_expr_pos;
                       decl_pos_opt = None;
                       msg =
                         "The builtin "
                         ^ Markdown_lite.md_codify (Utils.strip_ns fn_name)
                         ^ " is deprecated.";
                     })
            in
            match fn_param_exprs with
            | [] ->
              let args_err =
                Err.naming @@ Naming_error.Too_few_arguments fn_expr_pos
              in
              Error (fn_expr_pos, self#plus depr_err args_err)
            | (Ast_defs.Pnormal, fn_expr) :: fn_param_exprs ->
              let (fn_expr, expr_err) = super#on_expr env fn_expr
              and (fn_param_exprs, exprs_err) =
                super#on_list (self#on_snd super#on_expr) env fn_param_exprs
              in
              (* TODO[mjt] why are we dropping the unpacked variadic arg here? *)
              let expr_ = Aast.Call (fn_expr, targs, fn_param_exprs, None) in
              let err = self#plus_all [depr_err; expr_err; exprs_err] in
              Ok (expr_, err)
            | (Ast_defs.Pinout pk_pos, fn_expr) :: fn_param_exprs ->
              let (_, fn_expr_pos, _) = fn_expr in
              let pos = Pos.merge pk_pos fn_expr_pos in
              let inout_err =
                Err.nast_check
                @@ Nast_check_error.Inout_in_transformed_pseudofunction
                     { pos; fn_name = "call_user_func" }
              in
              let (fn_param_exprs, fn_param_err) =
                super#on_list (self#on_snd super#on_expr) env fn_param_exprs
              in
              (* TODO[mjt] why are we dropping the unpacked variadic arg here? *)
              let expr_ = Aast.Call (fn_expr, targs, fn_param_exprs, None) in
              let err = self#plus_all [depr_err; inout_err; fn_param_err] in
              Ok (expr_, err)
          else if String.equal fn_name SN.AutoimportedFunctions.fun_ then
            let unpacked_err =
              Option.value_map
                ~default:self#zero
                ~f:(fun (_, pos, _) ->
                  Err.naming @@ Naming_error.Too_few_arguments pos)
                fn_unpacked_expr_opt
            in
            match fn_param_exprs with
            | [] ->
              let args_err =
                Err.naming @@ Naming_error.Too_few_arguments fn_expr_pos
              in
              Error (fn_expr_pos, self#plus unpacked_err args_err)
            | [(Ast_defs.Pnormal, (_, pos, Aast.String nm))] ->
              Ok (Aast.Fun_id (pos, nm), unpacked_err)
            | [(_, (_, pos, _))] ->
              let illegal_fn_err = Err.naming @@ Naming_error.Illegal_fun pos in
              Error (pos, self#plus unpacked_err illegal_fn_err)
            | _ ->
              let args_err =
                Err.naming @@ Naming_error.Too_many_arguments fn_expr_pos
              in
              Error (fn_expr_pos, self#plus unpacked_err args_err)
          else if String.equal fn_name SN.AutoimportedFunctions.inst_meth then
            let unpacked_err =
              Option.value_map
                ~default:self#zero
                ~f:(fun (_, pos, _) ->
                  Err.naming @@ Naming_error.Too_few_arguments pos)
                fn_unpacked_expr_opt
            in
            match fn_param_exprs with
            | []
            | [_] ->
              let args_err =
                Err.naming @@ Naming_error.Too_few_arguments fn_expr_pos
              in
              Error (fn_expr_pos, self#plus unpacked_err args_err)
            | [
             (Ast_defs.Pnormal, instance);
             (Ast_defs.Pnormal, (_, p, Aast.String meth));
            ] ->
              Ok (Aast.Method_id (instance, (p, meth)), unpacked_err)
            | [(_, (_, p, _)); _] ->
              let inst_err = Err.naming @@ Naming_error.Illegal_inst_meth p in
              Error (p, self#plus unpacked_err inst_err)
            | _ ->
              let args_err =
                Err.naming @@ Naming_error.Too_many_arguments fn_expr_pos
              in
              Error (fn_expr_pos, self#plus unpacked_err args_err)
          else if String.equal fn_name SN.AutoimportedFunctions.meth_caller then
            let unpacked_err =
              Option.value_map
                ~default:self#zero
                ~f:(fun (_, pos, _) ->
                  Err.naming @@ Naming_error.Too_few_arguments pos)
                fn_unpacked_expr_opt
            in
            match fn_param_exprs with
            | []
            | [_] ->
              let args_err =
                Err.naming @@ Naming_error.Too_few_arguments fn_expr_pos
              in
              Error (fn_expr_pos, self#plus unpacked_err args_err)
            | [(Ast_defs.Pnormal, e1); (Ast_defs.Pnormal, e2)] ->
              begin
                match (e1, e2) with
                | Aast.((_, pc, String cl), (_, pm, String meth)) ->
                  Ok (Aast.Method_caller ((pc, cl), (pm, meth)), unpacked_err)
                | Aast.
                    ( (_, _, Class_const ((_, _, CI cl), (_, mem))),
                      (_, pm, String meth) )
                  when String.equal mem SN.Members.mClass ->
                  Ok (Aast.Method_caller (cl, (pm, meth)), unpacked_err)
                | ((_, p, _), _) ->
                  let meth_err =
                    Err.naming @@ Naming_error.Illegal_meth_caller p
                  in
                  Error (p, self#plus unpacked_err meth_err)
              end
            | [(Ast_defs.Pinout _, _); _]
            | [_; (Ast_defs.Pinout _, _)] ->
              let meth_err =
                Err.naming @@ Naming_error.Illegal_meth_caller fn_expr_pos
              in
              Error (fn_expr_pos, self#plus unpacked_err meth_err)
            | _ ->
              let args_err =
                Err.naming @@ Naming_error.Too_many_arguments fn_expr_pos
              in
              Error (fn_expr_pos, self#plus unpacked_err args_err)
          else if String.equal fn_name SN.AutoimportedFunctions.class_meth then
            let unpacked_err =
              Option.value_map
                ~default:self#zero
                ~f:(fun (_, pos, _) ->
                  Err.naming @@ Naming_error.Too_few_arguments pos)
                fn_unpacked_expr_opt
            in
            match fn_param_exprs with
            | []
            | [_] ->
              let args_err =
                Err.naming @@ Naming_error.Too_few_arguments fn_expr_pos
              in
              Error (fn_expr_pos, self#plus unpacked_err args_err)
            | [(Ast_defs.Pnormal, e1); (Ast_defs.Pnormal, e2)] ->
              begin
                match (e1, e2) with
                | Aast.((_, pc, String cl), (_, pm, String meth)) ->
                  let cid = Aast.CI (pc, cl) in
                  Ok (Aast.Smethod_id (((), pc, cid), (pm, meth)), unpacked_err)
                | Aast.((_, _, Id (pc, const)), (_, pm, String meth))
                  when String.equal const SN.PseudoConsts.g__CLASS__ ->
                  (* All of these that use current_cls aren't quite correct
                   * inside a trait, as the class should be the using class.
                   * It's sufficient for typechecking purposes (we require
                   * subclass to be compatible with the trait member/method
                   * declarations).
                   *)
                  (match env.Env.current_cls with
                  | Some (cid, _, true) ->
                    let cid = Aast.CI (pc, snd cid) in
                    Ok
                      ( Aast.Smethod_id (((), fn_expr_pos, cid), (pm, meth)),
                        unpacked_err )
                  | Some (cid, kind, false) ->
                    let is_trait = Ast_defs.is_c_trait kind in
                    let class_name = snd cid in
                    let non_final_err =
                      Err.naming
                      @@ Naming_error.Class_meth_non_final_CLASS
                           { pos = fn_expr_pos; is_trait; class_name }
                    in

                    Error (fn_expr_pos, self#plus unpacked_err non_final_err)
                  | None ->
                    let meth_err =
                      Err.naming @@ Naming_error.Illegal_class_meth fn_expr_pos
                    in
                    Error (fn_expr_pos, self#plus unpacked_err meth_err))
                | Aast.
                    ( (_, _, Class_const ((_, pc, CI cl), (_, mem))),
                      (_, pm, String meth) )
                  when String.equal mem SN.Members.mClass ->
                  let cid = Aast.CI cl in
                  Ok (Aast.Smethod_id (((), pc, cid), (pm, meth)), unpacked_err)
                | Aast.
                    ( (_, p, Class_const ((_, pc, CIself), (_, mem))),
                      (_, pm, String meth) )
                  when String.equal mem SN.Members.mClass ->
                  (match env.Env.current_cls with
                  | Some (_cid, _, true) ->
                    Ok
                      ( Aast.(Smethod_id (((), pc, CIself), (pm, meth))),
                        unpacked_err )
                  | Some (cid, _, false) ->
                    let class_name = snd cid in
                    let non_final_err =
                      Err.naming
                      @@ Naming_error.Class_meth_non_final_self
                           { pos = p; class_name }
                    in
                    Error (p, self#plus unpacked_err non_final_err)
                  | None ->
                    let meth_err =
                      Err.naming @@ Naming_error.Illegal_class_meth p
                    in
                    Error (p, self#plus unpacked_err meth_err))
                | Aast.
                    ( (_, p, Class_const ((_, pc, CIstatic), (_, mem))),
                      (_, pm, String meth) )
                  when String.equal mem SN.Members.mClass ->
                  (match env.Env.current_cls with
                  | Some (_cid, _, _) ->
                    Ok
                      ( Aast.(Smethod_id (((), pc, CIstatic), (pm, meth))),
                        unpacked_err )
                  | None ->
                    let meth_err =
                      Err.naming @@ Naming_error.Illegal_class_meth p
                    in
                    Error (p, self#plus unpacked_err meth_err))
                | ((_, p, _), _) ->
                  let meth_err =
                    Err.naming @@ Naming_error.Illegal_class_meth p
                  in
                  Error (p, self#plus unpacked_err meth_err)
              end
            | [(Ast_defs.Pinout _, _); _]
            | [_; (Ast_defs.Pinout _, _)] ->
              let meth_err =
                Err.naming @@ Naming_error.Illegal_class_meth fn_expr_pos
              in
              Error (fn_expr_pos, self#plus unpacked_err meth_err)
            | _ ->
              let args_err =
                Err.naming @@ Naming_error.Too_many_arguments fn_expr_pos
              in
              Error (fn_expr_pos, self#plus unpacked_err args_err)
          else
            let (fn_param_exprs, fn_param_err) =
              super#on_list (self#on_snd super#on_expr) env fn_param_exprs
            and (fn_unpacked_expr_opt, fn_unpacked_expr_err) =
              super#on_option super#on_expr env fn_unpacked_expr_opt
            in
            let err = self#plus fn_param_err fn_unpacked_expr_err in
            Ok
              ( Aast.Call (fn_expr, targs, fn_param_exprs, fn_unpacked_expr_opt),
                err )
        in
        begin
          match res with
          | Ok (expr_, err) -> (expr_, err)
          | Error (pos, err) -> (Err.invalid_expr_ pos, err)
        end
      | _ -> super#on_Call env fn_expr targs fn_param_exprs fn_unpacked_expr_opt

    method! on_class_ _env c =
      let env = Env.in_class c in
      super#on_class_ env c

    method private plus_all errs =
      List.fold_left ~init:self#zero ~f:self#plus errs

    method private on_snd f env (fst, snd) =
      let (snd, err) = f env snd in
      ((fst, snd), err)
  end

let elab f ?init ?(env = Env.empty) elem =
  Tuple2.map_snd ~f:(Err.from_monoid ?init) @@ f env elem

let elab_fun_def ?init ?env elem = elab visitor#on_fun_def ?init ?env elem

let elab_typedef ?init ?env elem = elab visitor#on_typedef ?init ?env elem

let elab_module_def ?init ?env elem = elab visitor#on_module_def ?init ?env elem

let elab_gconst ?init ?env elem = elab visitor#on_gconst ?init ?env elem

let elab_class ?init ?env elem = elab visitor#on_class_ ?init ?env elem

let elab_program ?init ?env elem = elab visitor#on_program ?init ?env elem
