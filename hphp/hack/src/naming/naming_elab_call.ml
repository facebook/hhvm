(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module SN = Naming_special_names

module Env = struct
  let in_class t Aast.{ c_name; c_kind; c_final; _ } =
    Naming_phase_env.
      {
        t with
        elab_call = Elab_call.{ current_class = Some (c_name, c_kind, c_final) };
      }

  let current_class
      Naming_phase_env.{ elab_call = Elab_call.{ current_class }; _ } =
    current_class
end

let on_expr on_error ((annot, pos, expr_) as expr) ~ctx =
  let (res, errs) =
    match expr_ with
    | Aast.(
        Call
          ( ((_, _, Id (_, fn_name)) as fn_expr),
            targs,
            fn_param_exprs,
            fn_unpacked_expr_opt ))
      when String.equal fn_name SN.SpecialFunctions.echo ->
      let errs =
        Option.to_list
        @@ Option.map
             ~f:(fun (_, pos, _) ->
               Naming_phase_error.naming @@ Naming_error.Too_few_arguments pos)
             fn_unpacked_expr_opt
      in
      ( Ok (Aast.Call (fn_expr, targs, fn_param_exprs, fn_unpacked_expr_opt)),
        errs )
    | Aast.(Call ((_, fn_expr_pos, Id (_, fn_name)), targs, fn_param_exprs, _))
      when String.equal fn_name SN.StdlibFunctions.call_user_func ->
      let errs =
        [
          Naming_phase_error.naming
          @@ Naming_error.Deprecated_use
               {
                 pos = fn_expr_pos;
                 msg =
                   "The builtin "
                   ^ Markdown_lite.md_codify (Utils.strip_ns fn_name)
                   ^ " is deprecated.";
               };
        ]
      in
      begin
        match fn_param_exprs with
        | [] ->
          let args_err =
            Naming_phase_error.naming
            @@ Naming_error.Too_few_arguments fn_expr_pos
          in
          (Error fn_expr_pos, args_err :: errs)
        | (Ast_defs.Pnormal, fn_expr) :: fn_param_exprs ->
          (* TODO[mjt] why are we dropping the unpacked variadic arg here? *)
          (Ok (Aast.Call (fn_expr, targs, fn_param_exprs, None)), errs)
        | (Ast_defs.Pinout pk_pos, fn_expr) :: fn_param_exprs ->
          let (_, fn_expr_pos, _) = fn_expr in
          let pos = Pos.merge pk_pos fn_expr_pos in
          let inout_err =
            Naming_phase_error.nast_check
            @@ Nast_check_error.Inout_in_transformed_pseudofunction
                 { pos; fn_name = "call_user_func" }
          in
          (* TODO[mjt] why are we dropping the unpacked variadic arg here? *)
          ( Ok (Aast.Call (fn_expr, targs, fn_param_exprs, None)),
            inout_err :: errs )
      end
    | Aast.(
        Call
          ( (_, fn_expr_pos, Id (_, fn_name)),
            _targs,
            fn_param_exprs,
            fn_unpacked_expr_opt ))
      when String.equal fn_name SN.AutoimportedFunctions.fun_ ->
      (* TODO[mjt] targs is ignored entirely here - shouldn't we generate
         and error to say they are invalid for this function? *)
      let errs =
        Option.to_list
        @@ Option.map
             ~f:(fun (_, pos, _) ->
               Naming_phase_error.naming @@ Naming_error.Too_few_arguments pos)
             fn_unpacked_expr_opt
      in
      begin
        match fn_param_exprs with
        | [] ->
          let args_err =
            Naming_phase_error.naming
            @@ Naming_error.Too_few_arguments fn_expr_pos
          in
          (Error fn_expr_pos, args_err :: errs)
        | [(Ast_defs.Pnormal, (_, pos, Aast.String nm))] ->
          (Ok (Aast.Fun_id (pos, nm)), errs)
        | [(_, (_, pos, _))] ->
          let illegal_fn_err =
            Naming_phase_error.naming @@ Naming_error.Illegal_fun pos
          in
          (Error pos, illegal_fn_err :: errs)
        | _ ->
          let args_err =
            Naming_phase_error.naming
            @@ Naming_error.Too_many_arguments fn_expr_pos
          in
          (Error fn_expr_pos, args_err :: errs)
      end
    | Aast.(
        Call
          ( (_, fn_expr_pos, Id (_, fn_name)),
            _targs,
            fn_param_exprs,
            fn_unpacked_expr_opt ))
      when String.equal fn_name SN.AutoimportedFunctions.inst_meth ->
      (* TODO[mjt] targs is ignored entirely here - shouldn't we generate
         and error to say they are invalid for this function? *)
      let errs =
        Option.to_list
        @@ Option.map
             ~f:(fun (_, pos, _) ->
               Naming_phase_error.naming @@ Naming_error.Too_few_arguments pos)
             fn_unpacked_expr_opt
      in
      begin
        match fn_param_exprs with
        | []
        | [_] ->
          let args_err =
            Naming_phase_error.naming
            @@ Naming_error.Too_few_arguments fn_expr_pos
          in
          (Error fn_expr_pos, args_err :: errs)
        | [
         (Ast_defs.Pnormal, instance);
         (Ast_defs.Pnormal, (_, p, Aast.String meth));
        ] ->
          (Ok (Aast.Method_id (instance, (p, meth))), errs)
        | [(_, (_, p, _)); _] ->
          let inst_err =
            Naming_phase_error.naming @@ Naming_error.Illegal_inst_meth p
          in
          (Error p, inst_err :: errs)
        | _ ->
          let args_err =
            Naming_phase_error.naming
            @@ Naming_error.Too_many_arguments fn_expr_pos
          in
          (Error fn_expr_pos, args_err :: errs)
      end
    | Aast.(
        Call
          ( (_, fn_expr_pos, Id (_, fn_name)),
            _targs,
            fn_param_exprs,
            fn_unpacked_expr_opt ))
      when String.equal fn_name SN.AutoimportedFunctions.meth_caller ->
      (* TODO[mjt] targs is ignored entirely here - shouldn't we generate
         and error to say they are invalid for this function? *)
      let errs =
        Option.to_list
        @@ Option.map
             ~f:(fun (_, pos, _) ->
               Naming_phase_error.naming @@ Naming_error.Too_few_arguments pos)
             fn_unpacked_expr_opt
      in
      begin
        match fn_param_exprs with
        | []
        | [_] ->
          let args_err =
            Naming_phase_error.naming
            @@ Naming_error.Too_few_arguments fn_expr_pos
          in
          (Error fn_expr_pos, args_err :: errs)
        | [(Ast_defs.Pnormal, e1); (Ast_defs.Pnormal, e2)] -> begin
          match (e1, e2) with
          | Aast.((_, pc, String cl), (_, pm, String meth)) ->
            (Ok (Aast.Method_caller ((pc, cl), (pm, meth))), errs)
          | Aast.
              ( (_, _, Class_const ((_, _, CI cl), (_, mem))),
                (_, pm, String meth) )
            when String.equal mem SN.Members.mClass ->
            (Ok (Aast.Method_caller (cl, (pm, meth))), errs)
          | ((_, p, _), _) ->
            let meth_err =
              Naming_phase_error.naming @@ Naming_error.Illegal_meth_caller p
            in
            (Error p, meth_err :: errs)
        end
        | [(Ast_defs.Pinout _, _); _]
        | [_; (Ast_defs.Pinout _, _)] ->
          let meth_err =
            Naming_phase_error.naming
            @@ Naming_error.Illegal_meth_caller fn_expr_pos
          in
          (Error fn_expr_pos, meth_err :: errs)
        | _ ->
          let args_err =
            Naming_phase_error.naming
            @@ Naming_error.Too_many_arguments fn_expr_pos
          in
          (Error fn_expr_pos, args_err :: errs)
      end
    | Aast.(
        Call
          ( (_, fn_expr_pos, Id (_, fn_name)),
            _targs,
            fn_param_exprs,
            fn_unpacked_expr_opt ))
      when String.equal fn_name SN.AutoimportedFunctions.class_meth ->
      (* TODO[mjt] targs is ignored entirely here - shouldn't we generate
         and error to say they are invalid for this function? *)
      let errs =
        Option.to_list
        @@ Option.map
             ~f:(fun (_, pos, _) ->
               Naming_phase_error.naming @@ Naming_error.Too_few_arguments pos)
             fn_unpacked_expr_opt
      in
      begin
        match fn_param_exprs with
        | []
        | [_] ->
          let args_err =
            Naming_phase_error.naming
            @@ Naming_error.Too_few_arguments fn_expr_pos
          in
          (Error fn_expr_pos, args_err :: errs)
        | [(Ast_defs.Pnormal, e1); (Ast_defs.Pnormal, e2)] -> begin
          match (e1, e2) with
          | Aast.((annot, pc, String cl), (_, pm, String meth)) ->
            let cid = Aast.CI (pc, cl) in
            (Ok (Aast.Smethod_id ((annot, pc, cid), (pm, meth))), errs)
          | Aast.((annot, _, Id (pc, const)), (_, pm, String meth))
            when String.equal const SN.PseudoConsts.g__CLASS__ ->
            (* All of these that use current_cls aren't quite correct
             * inside a trait, as the class should be the using class.
             * It's sufficient for typechecking purposes (we require
             * subclass to be compatible with the trait member/method
             * declarations).
             *)
            (match Env.current_class ctx with
            | Some (cid, _, true) ->
              let cid = Aast.CI (pc, snd cid) in
              ( Ok Aast.(Smethod_id ((annot, fn_expr_pos, cid), (pm, meth))),
                errs )
            | Some (cid, kind, false) ->
              let is_trait = Ast_defs.is_c_trait kind in
              let class_name = snd cid in
              let non_final_err =
                Naming_phase_error.naming
                @@ Naming_error.Class_meth_non_final_CLASS
                     { pos = fn_expr_pos; is_trait; class_name }
              in
              (Error fn_expr_pos, non_final_err :: errs)
            | None ->
              let meth_err =
                Naming_phase_error.naming
                @@ Naming_error.Illegal_class_meth fn_expr_pos
              in
              (Error fn_expr_pos, meth_err :: errs))
          | Aast.
              ( (_, _, Class_const ((annot, pc, CI cl), (_, mem))),
                (_, pm, String meth) )
            when String.equal mem SN.Members.mClass ->
            let cid = Aast.CI cl in
            (Ok (Aast.Smethod_id ((annot, pc, cid), (pm, meth))), errs)
          | Aast.
              ( (_, p, Class_const ((annot, pc, CIself), (_, mem))),
                (_, pm, String meth) )
            when String.equal mem SN.Members.mClass ->
            (match Env.current_class ctx with
            | Some (_cid, _, true) ->
              (Ok Aast.(Smethod_id ((annot, pc, CIself), (pm, meth))), errs)
            | Some (cid, _, false) ->
              let class_name = snd cid in
              let non_final_err =
                Naming_phase_error.naming
                @@ Naming_error.Class_meth_non_final_self
                     { pos = p; class_name }
              in
              (Error p, non_final_err :: errs)
            | None ->
              let meth_err =
                Naming_phase_error.naming @@ Naming_error.Illegal_class_meth p
              in
              (Error p, meth_err :: errs))
          | Aast.
              ( (_, p, Class_const ((annot, pc, CIstatic), (_, mem))),
                (_, pm, String meth) )
            when String.equal mem SN.Members.mClass ->
            (match Env.current_class ctx with
            | Some (_cid, _, _) ->
              (Ok Aast.(Smethod_id ((annot, pc, CIstatic), (pm, meth))), errs)
            | None ->
              let meth_err =
                Naming_phase_error.naming @@ Naming_error.Illegal_class_meth p
              in
              (Error p, meth_err :: errs))
          | ((_, p, _), _) ->
            let meth_err =
              Naming_phase_error.naming @@ Naming_error.Illegal_class_meth p
            in
            (Error p, meth_err :: errs)
        end
        | [(Ast_defs.Pinout _, _); _]
        | [_; (Ast_defs.Pinout _, _)] ->
          let meth_err =
            Naming_phase_error.naming
            @@ Naming_error.Illegal_class_meth fn_expr_pos
          in
          (Error fn_expr_pos, meth_err :: errs)
        | _ ->
          let args_err =
            Naming_phase_error.naming
            @@ Naming_error.Too_many_arguments fn_expr_pos
          in
          (Error fn_expr_pos, args_err :: errs)
      end
    | _ -> (Ok expr_, [])
  in
  List.iter ~f:on_error errs;
  match res with
  | Ok expr_ -> (ctx, Ok (annot, pos, expr_))
  | Error _pos -> (ctx, Error (Naming_phase_error.invalid_expr expr))

let on_class_ c ~ctx = (Env.in_class ctx c, Ok c)

let top_down_pass =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.
      { id with on_ty_class_ = Some (fun elem ~ctx -> on_class_ elem ~ctx) }

let bottom_up_pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.bottom_up
    Aast.Pass.
      {
        id with
        on_ty_expr = Some (fun elem ~ctx -> on_expr on_error elem ~ctx);
      }
