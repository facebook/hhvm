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

let on_expr_ (env, expr_, err_acc) =
  let res =
    match expr_ with
    | Aast.(
        Call
          ( ((_, _, Id (_, fn_name)) as fn_expr),
            targs,
            fn_param_exprs,
            fn_unpacked_expr_opt ))
      when String.equal fn_name SN.SpecialFunctions.echo ->
      let err =
        Option.map
          ~f:(fun (_, pos, _) ->
            Err.naming @@ Naming_error.Too_few_arguments pos)
          fn_unpacked_expr_opt
      in
      Ok (Aast.Call (fn_expr, targs, fn_param_exprs, fn_unpacked_expr_opt), err)
    | Aast.(Call ((_, fn_expr_pos, Id (_, fn_name)), targs, fn_param_exprs, _))
      when String.equal fn_name SN.StdlibFunctions.call_user_func ->
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
      begin
        match fn_param_exprs with
        | [] ->
          let args_err =
            Err.naming @@ Naming_error.Too_few_arguments fn_expr_pos
          in
          Error (fn_expr_pos, Some (Err.Free_monoid.plus depr_err args_err))
        | (Ast_defs.Pnormal, fn_expr) :: fn_param_exprs ->
          (* TODO[mjt] why are we dropping the unpacked variadic arg here? *)
          Ok (Aast.Call (fn_expr, targs, fn_param_exprs, None), Some depr_err)
        | (Ast_defs.Pinout pk_pos, fn_expr) :: fn_param_exprs ->
          let (_, fn_expr_pos, _) = fn_expr in
          let pos = Pos.merge pk_pos fn_expr_pos in
          let inout_err =
            Err.nast_check
            @@ Nast_check_error.Inout_in_transformed_pseudofunction
                 { pos; fn_name = "call_user_func" }
          in
          let err = Err.Free_monoid.plus depr_err inout_err in
          (* TODO[mjt] why are we dropping the unpacked variadic arg here? *)
          Ok (Aast.Call (fn_expr, targs, fn_param_exprs, None), Some err)
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
      let unpacked_err =
        Option.map
          ~f:(fun (_, pos, _) ->
            Err.naming @@ Naming_error.Too_few_arguments pos)
          fn_unpacked_expr_opt
      in
      begin
        match fn_param_exprs with
        | [] ->
          let args_err =
            Err.naming @@ Naming_error.Too_few_arguments fn_expr_pos
          in
          Error
            ( fn_expr_pos,
              Some
                (Option.value_map
                   ~default:args_err
                   ~f:(Err.Free_monoid.plus args_err)
                   unpacked_err) )
        | [(Ast_defs.Pnormal, (_, pos, Aast.String nm))] ->
          Ok (Aast.Fun_id (pos, nm), unpacked_err)
        | [(_, (_, pos, _))] ->
          let illegal_fn_err = Err.naming @@ Naming_error.Illegal_fun pos in
          let err =
            Option.value_map
              unpacked_err
              ~default:illegal_fn_err
              ~f:(Err.Free_monoid.plus illegal_fn_err)
          in
          Error (pos, Some err)
        | _ ->
          let args_err =
            Err.naming @@ Naming_error.Too_many_arguments fn_expr_pos
          in
          let err =
            Option.value_map
              unpacked_err
              ~default:args_err
              ~f:(Err.Free_monoid.plus args_err)
          in
          Error (fn_expr_pos, Some err)
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
      let unpacked_err =
        Option.map
          ~f:(fun (_, pos, _) ->
            Err.naming @@ Naming_error.Too_few_arguments pos)
          fn_unpacked_expr_opt
      in
      begin
        match fn_param_exprs with
        | []
        | [_] ->
          let args_err =
            Err.naming @@ Naming_error.Too_few_arguments fn_expr_pos
          in
          let err =
            Option.value_map
              ~default:args_err
              ~f:(Err.Free_monoid.plus args_err)
              unpacked_err
          in
          Error (fn_expr_pos, Some err)
        | [
         (Ast_defs.Pnormal, instance);
         (Ast_defs.Pnormal, (_, p, Aast.String meth));
        ] ->
          Ok (Aast.Method_id (instance, (p, meth)), unpacked_err)
        | [(_, (_, p, _)); _] ->
          let inst_err = Err.naming @@ Naming_error.Illegal_inst_meth p in
          let err =
            Option.value_map
              ~default:inst_err
              ~f:(Err.Free_monoid.plus inst_err)
              unpacked_err
          in
          Error (p, Some err)
        | _ ->
          let args_err =
            Err.naming @@ Naming_error.Too_many_arguments fn_expr_pos
          in
          let err =
            Option.value_map
              ~default:args_err
              ~f:(Err.Free_monoid.plus args_err)
              unpacked_err
          in
          Error (fn_expr_pos, Some err)
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
      let unpacked_err =
        Option.map
          ~f:(fun (_, pos, _) ->
            Err.naming @@ Naming_error.Too_few_arguments pos)
          fn_unpacked_expr_opt
      in
      begin
        match fn_param_exprs with
        | []
        | [_] ->
          let args_err =
            Err.naming @@ Naming_error.Too_few_arguments fn_expr_pos
          in
          let err =
            Option.value_map
              unpacked_err
              ~default:args_err
              ~f:(Err.Free_monoid.plus args_err)
          in
          Error (fn_expr_pos, Some err)
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
              let meth_err = Err.naming @@ Naming_error.Illegal_meth_caller p in
              let err =
                Option.value_map
                  unpacked_err
                  ~default:meth_err
                  ~f:(Err.Free_monoid.plus meth_err)
              in
              Error (p, Some err)
          end
        | [(Ast_defs.Pinout _, _); _]
        | [_; (Ast_defs.Pinout _, _)] ->
          let meth_err =
            Err.naming @@ Naming_error.Illegal_meth_caller fn_expr_pos
          in
          let err =
            Option.value_map
              unpacked_err
              ~default:meth_err
              ~f:(Err.Free_monoid.plus meth_err)
          in
          Error (fn_expr_pos, Some err)
        | _ ->
          let args_err =
            Err.naming @@ Naming_error.Too_many_arguments fn_expr_pos
          in
          let err =
            Option.value_map
              unpacked_err
              ~default:args_err
              ~f:(Err.Free_monoid.plus args_err)
          in
          Error (fn_expr_pos, Some err)
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
      let unpacked_err =
        Option.map
          ~f:(fun (_, pos, _) ->
            Err.naming @@ Naming_error.Too_few_arguments pos)
          fn_unpacked_expr_opt
      in
      begin
        match fn_param_exprs with
        | []
        | [_] ->
          let args_err =
            Err.naming @@ Naming_error.Too_few_arguments fn_expr_pos
          in
          let err =
            Option.value_map
              unpacked_err
              ~default:args_err
              ~f:(Err.Free_monoid.plus args_err)
          in
          Error (fn_expr_pos, Some err)
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
              (match Env.current_class env with
              | Some (cid, _, true) ->
                let cid = Aast.CI (pc, snd cid) in
                Ok
                  ( Aast.(Smethod_id (((), fn_expr_pos, cid), (pm, meth))),
                    unpacked_err )
              | Some (cid, kind, false) ->
                let is_trait = Ast_defs.is_c_trait kind in
                let class_name = snd cid in
                let non_final_err =
                  Err.naming
                  @@ Naming_error.Class_meth_non_final_CLASS
                       { pos = fn_expr_pos; is_trait; class_name }
                in
                let err =
                  Option.value_map
                    unpacked_err
                    ~default:non_final_err
                    ~f:(Err.Free_monoid.plus non_final_err)
                in
                Error (fn_expr_pos, Some err)
              | None ->
                let meth_err =
                  Err.naming @@ Naming_error.Illegal_class_meth fn_expr_pos
                in
                let err =
                  Option.value_map
                    unpacked_err
                    ~default:meth_err
                    ~f:(Err.Free_monoid.plus meth_err)
                in
                Error (fn_expr_pos, Some err))
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
              (match Env.current_class env with
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
                let err =
                  Option.value_map
                    unpacked_err
                    ~default:non_final_err
                    ~f:(Err.Free_monoid.plus non_final_err)
                in
                Error (p, Some err)
              | None ->
                let meth_err =
                  Err.naming @@ Naming_error.Illegal_class_meth p
                in
                let err =
                  Option.value_map
                    unpacked_err
                    ~default:meth_err
                    ~f:(Err.Free_monoid.plus meth_err)
                in
                Error (p, Some err))
            | Aast.
                ( (_, p, Class_const ((_, pc, CIstatic), (_, mem))),
                  (_, pm, String meth) )
              when String.equal mem SN.Members.mClass ->
              (match Env.current_class env with
              | Some (_cid, _, _) ->
                Ok
                  ( Aast.(Smethod_id (((), pc, CIstatic), (pm, meth))),
                    unpacked_err )
              | None ->
                let meth_err =
                  Err.naming @@ Naming_error.Illegal_class_meth p
                in
                let err =
                  Option.value_map
                    unpacked_err
                    ~default:meth_err
                    ~f:(Err.Free_monoid.plus meth_err)
                in
                Error (p, Some err))
            | ((_, p, _), _) ->
              let meth_err = Err.naming @@ Naming_error.Illegal_class_meth p in
              let err =
                Option.value_map
                  unpacked_err
                  ~default:meth_err
                  ~f:(Err.Free_monoid.plus meth_err)
              in
              Error (p, Some err)
          end
        | [(Ast_defs.Pinout _, _); _]
        | [_; (Ast_defs.Pinout _, _)] ->
          let meth_err =
            Err.naming @@ Naming_error.Illegal_class_meth fn_expr_pos
          in
          let err =
            Option.value_map
              unpacked_err
              ~default:meth_err
              ~f:(Err.Free_monoid.plus meth_err)
          in
          Error (fn_expr_pos, Some err)
        | _ ->
          let args_err =
            Err.naming @@ Naming_error.Too_many_arguments fn_expr_pos
          in
          let err =
            Option.value_map
              unpacked_err
              ~default:args_err
              ~f:(Err.Free_monoid.plus args_err)
          in
          Error (fn_expr_pos, Some err)
      end
    | _ -> Ok (expr_, None)
  in
  match res with
  | Ok (expr_, err_opt) ->
    Naming_phase_pass.Cont.next
      ( env,
        expr_,
        Option.value_map
          ~default:err_acc
          ~f:(Err.Free_monoid.plus err_acc)
          err_opt )
  | Error (pos, err_opt) ->
    Naming_phase_pass.Cont.finish
      ( env,
        Err.invalid_expr_ pos,
        Option.value_map
          ~default:err_acc
          ~f:(Err.Free_monoid.plus err_acc)
          err_opt )

let on_class_ (env, c, err) =
  Naming_phase_pass.Cont.next (Env.in_class env c, c, err)

let top_down_pass =
  Naming_phase_pass.(top_down { identity with on_class_ = Some on_class_ })

let bottom_up_pass =
  Naming_phase_pass.(bottom_up { identity with on_expr_ = Some on_expr_ })
