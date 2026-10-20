(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module T = Inline_method_types

let rec context_independent_default (_, _, expr) =
  let open Aast_defs in
  match expr with
  | Null
  | True
  | False
  | Int _
  | Float _
  | String _ ->
    true
  | Unop ((Ast_defs.Uplus | Ast_defs.Uminus), expr) ->
    context_independent_default expr
  | ValCollection (_, None, exprs)
  | Tuple exprs ->
    List.for_all exprs ~f:context_independent_default
  | KeyValCollection (_, None, entries) ->
    List.for_all entries ~f:(fun (key, value) ->
        context_independent_default key && context_independent_default value)
  | Shape fields ->
    List.for_all fields ~f:(function
        | SF_field (SFlit_str _, value) -> context_independent_default value
        | SF_field ((SFclassname _ | SFclass_const _), _)
        | SF_splat _ ->
          false)
  | _ -> false

let argument_evaluation_order args =
  let open Aast_defs in
  let indexed =
    List.mapi args ~f:(fun index arg ->
        let (name, (_, _, expr)) =
          match arg with
          | Anamed ((_, name), expr) -> (Some name, expr)
          | Anormal expr
          | Ainout (_, expr) ->
            (None, expr)
        in
        let effectful =
          match expr with
          | Int _
          | Float _
          | String _
          | EnumClassLabel _
          | Null
          | False
          | True
          | Lvar _
          | Id _
          | PrefixedString _ ->
            false
          | _ -> true
        in
        (index, name, effectful))
  in
  let named =
    List.filter_map indexed ~f:(fun (index, name, _) ->
        Option.map name ~f:(fun name -> (name, index)))
    |> List.sort ~compare:(fun (left, _) (right, _) ->
           String.compare left right)
    |> List.map ~f:snd
  in
  let positional =
    List.filter_map indexed ~f:(fun (index, name, _) ->
        Option.some_if (Option.is_none name) index)
  in
  let effectful =
    List.filter_map indexed ~f:(fun (index, _, effectful) ->
        Option.some_if effectful index)
  in
  let effectful_set = Int.Set.of_list effectful in
  let rec take_until index = function
    | [] -> failwith "inline_method: missing effectful argument"
    | current :: rest when Int.equal current index -> ([current], rest)
    | current :: rest ->
      let (before, rest) = take_until index rest in
      (current :: before, rest)
  in
  let rec order evaluated effectful = function
    | [] -> []
    | index :: rest when Set.mem evaluated index ->
      order evaluated effectful rest
    | index :: rest when not (Set.mem effectful_set index) ->
      index :: order evaluated effectful rest
    | index :: rest ->
      let (before, effectful) = take_until index effectful in
      let evaluated = List.fold before ~init:evaluated ~f:Set.add in
      before @ order evaluated effectful rest
  in
  order Int.Set.empty effectful (named @ positional)

let method_info_create (m : Tast.method_) : T.method_info =
  let open Aast_defs in
  let is_private =
    match m.m_visibility with
    | Private -> true
    | Public
    | Protected
    | Internal
    | ProtectedInternal ->
      false
  in
  let block = m.m_body.fb_ast in
  let last_stmt_is_return =
    List.last block
    |> Option.map ~f:(function
           | (_, Return _) -> true
           | _ -> false)
    |> Option.value ~default:false
  in
  let is_supported_param p =
    List.is_empty p.param_user_attributes
    && Option.is_none p.param_splat
    &&
    match p.param_callconv with
    | Ast_defs.Pinout _ -> false
    | Ast_defs.Pnormal -> true
  in
  let has_void_return =
    match snd m.Aast.m_ret with
    | Some Aast_defs.(_, Hprim Tvoid) -> true
    | None
    | Some _ ->
      false
  in
  let all_params_are_supported =
    m.m_params |> List.for_all ~f:is_supported_param
  in
  T.
    {
      method_pos = m.m_span;
      block;
      params =
        m.m_params
        |> List.map ~f:(fun p ->
               let default =
                 Option.bind (Aast_utils.get_param_default p) ~f:(fun expr ->
                     Option.some_if
                       (context_independent_default expr)
                       (Aast_utils.get_expr_pos expr))
               in
               T.
                 {
                   name = p.param_name;
                   named = Option.is_some p.param_named;
                   default;
                   variadic = Aast_utils.is_param_variadic p;
                 });
      (* only the following two are computed from the body *)
      var_names = [];
      return_cnt = 0;
      has_void_return;
      is_private;
      last_stmt_is_return;
      all_params_are_supported;
    }

let find_candidate ~(cursor : Pos.t) ~entry ctx : T.candidate option =
  let method_use_counts : int String.Map.t ref = ref String.Map.empty in
  let method_infos : T.method_info String.Map.t ref = ref String.Map.empty in
  let target_class : string option ref = ref None in
  let current_method : string option ref = ref None in
  let stmt_pos = ref Pos.none in
  let call_info : T.call_info option ref = ref None in

  let method_info_update method_name ~f =
    method_infos :=
      Map.update !method_infos method_name ~f:(fun v ->
          (* method_infos map is guaranteed to have all method names in it
             since we traverse methods after adding them to the map *)
          f @@ Option.value_exn v)
  in

  let visitor =
    object (self)
      inherit Tast_visitor.iter as super

      method! on_stmt env ((pos, _) as stmt) =
        if Option.is_some !current_method then stmt_pos := pos;
        super#on_stmt env stmt

      method! on_Lvar env lvar =
        match !current_method with
        | Some current_method ->
          let var = Local_id.get_name @@ snd lvar in
          method_info_update current_method ~f:(fun method_info ->
              T.{ method_info with var_names = var :: method_info.var_names });
          super#on_Lvar env lvar
        | None -> ()

      method! on_Return env return =
        match !current_method with
        | Some current_method ->
          method_info_update current_method ~f:(fun method_info ->
              T.{ method_info with return_cnt = method_info.return_cnt + 1 });
          super#on_Return env return
        | None -> ()

      method! on_expr env expr =
        let open Aast_defs in
        match !current_method with
        | Some current_method -> begin
          super#on_expr env expr;
          let (_, expr_pos, expr_) = expr in
          let on_call
              ~call_id_pos ~callee_name ~param_kind_arg_pairs ~unpacked_arg =
            method_use_counts :=
              Map.update !method_use_counts callee_name ~f:(fun count_opt ->
                  1 + Option.value count_opt ~default:0);
            if Pos.contains call_id_pos cursor && Option.is_none unpacked_arg
            then
              let call_args =
                List.map param_kind_arg_pairs ~f:(fun arg ->
                    let name =
                      match arg with
                      | Anamed ((_, name), _) -> Some name
                      | Anormal _
                      | Ainout _ ->
                        None
                    in
                    (name, Aast_utils.get_argument_pos arg))
              in
              call_info :=
                Some
                  T.
                    {
                      callee_name;
                      call_stmt_pos = !stmt_pos;
                      caller_name = current_method;
                      call_pos = expr_pos;
                      call_args;
                      arg_evaluation_order =
                        argument_evaluation_order param_kind_arg_pairs;
                    }
          in
          match expr_ with
          | Call
              {
                func =
                  ( _,
                    call_id_pos,
                    Class_const ((_, _, (CIself | CIstatic)), (_, callee_name))
                  );
                args = param_kind_arg_pairs;
                unpacked_arg;
                _;
              } -> begin
            on_call
              ~call_id_pos
              ~callee_name
              ~param_kind_arg_pairs
              ~unpacked_arg
          end
          | Call
              {
                func =
                  ( _,
                    _,
                    Obj_get
                      ((_, _, This), (_, _, Id (call_id_pos, callee_name)), _, _)
                  );
                args = param_kind_arg_pairs;
                unpacked_arg;
                _;
              } ->
            on_call
              ~call_id_pos
              ~callee_name
              ~param_kind_arg_pairs
              ~unpacked_arg
          | _ -> ()
        end
        | None -> ()

      method! on_class_ env class_ =
        let open Aast_defs in
        if Pos.contains class_.c_span cursor then begin
          target_class := Some (snd class_.c_name);
          let method_infos_res =
            class_.c_methods
            |> List.map ~f:(fun m -> (snd m.m_name, method_info_create m))
            |> String.Map.of_alist
          in
          match method_infos_res with
          | `Ok method_infos_ ->
            method_infos := method_infos_;
            class_.c_methods
            |> List.iter ~f:(fun m ->
                   current_method := Some (snd m.m_name);
                   self#on_method_ env m)
          | `Duplicate_key _ -> ()
        end

      method! on_fun_ _ _ = ()
    end
  in

  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_quarantined ~ctx ~entry
  in
  visitor#go ctx tast.Tast_with_dynamic.under_normal_assumptions;
  let open Option.Let_syntax in
  let* call = !call_info in
  let* (T.
          {
            is_private;
            all_params_are_supported;
            last_stmt_is_return;
            return_cnt;
            _;
          } as callee) =
    Map.find !method_infos call.T.callee_name
  in
  let* caller = Map.find !method_infos call.T.caller_name in
  let is_inlineable =
    let has_ok_returns =
      return_cnt = 0 || (return_cnt = 1 && last_stmt_is_return)
    in
    has_ok_returns && is_private && all_params_are_supported
  in
  let* called_count = Map.find !method_use_counts call.T.callee_name in
  if called_count = 1 && is_inlineable then
    Some T.{ call; callee; caller }
  else
    None
