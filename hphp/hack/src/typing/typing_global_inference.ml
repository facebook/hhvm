(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Typing_defs
open Typing_env_types
module Inf = Typing_inference_env
module Env = Typing_env
module Sub = Typing_subtype
module MakeType = Typing_make_type
module ITySet = Internal_type_set

module StateErrors = struct
  module IdentMap = WrappedMap.Make (Ident)

  (** Mapping from type variable id to a list of errors. *)
  type t = Errors.error list IdentMap.t ref

  let mk_empty () = ref IdentMap.empty

  let get_errors t id = Option.value (IdentMap.find_opt id !t) ~default:[]

  let add t id err = t := IdentMap.add id (err :: get_errors t id) !t

  let has_error t id = not @@ List.is_empty @@ get_errors t id

  let elements t = IdentMap.elements !t

  let cardinal t = IdentMap.fold (fun _ l acc -> acc + List.length l) !t 0
end

let convert_on_error on_error pos =
  let on_error ?code ?quickfixes reasons =
    let code =
      Option.value code ~default:Error_codes.Typing.(err_code UnifyError)
    in
    on_error (User_error.make code ?quickfixes (pos, "Typing_error") reasons)
  in
  (Typing_error.Reasons_callback.from_on_error on_error [@alert "-deprecated"])

let catch_exc
    pos (on_error : Errors.error -> unit) (r : 'a) ?(verbose = false) f : 'a =
  try
    let (other_errors, v) =
      Errors.do_with_context (Pos.filename pos) Errors.Typing (fun () ->
          f @@ Some (convert_on_error on_error pos))
    in
    List.iter (Errors.get_error_list other_errors) ~f:on_error;
    v
  with
  | Inf.InconsistentTypeVarState _ as exn ->
    let e = Exception.wrap exn in
    if verbose then prerr_endline (Exception.to_string e);
    on_error
      (User_error.make
         Error_codes.Typing.(err_code UnifyError)
         (pos, Exception.get_ctor_string e)
         []);
    r

let is_ordered_solving env = TypecheckerOptions.ordered_solving env.genv.tcopt

module type MarshalledData = sig
  type t
end

module StateFunctor (M : MarshalledData) = struct
  type t = M.t

  let load (path : string) : t =
    In_channel.with_file path ~f:Marshal.from_channel

  let save (path : string) (t : t) : unit =
    Out_channel.with_file path ~f:(fun channel ->
        Marshal.to_channel channel t [])
end

let artifacts_path : string ref = ref ""

type global_type_map = Typing_defs.locl_ty Pos.AbsolutePosMap.t

let build_ty_map (ctx : Provider_context.t) (tast : Tast.def) : global_type_map
    =
  let get_global_var_pos env (ty : locl_ty) =
    (object
       inherit [Pos_or_decl.t option] Type_visitor.locl_type_visitor

       method! on_tvar pos _ tvar =
         match pos with
         | Some pos -> Some pos
         | None ->
           (match Env.get_global_tyvar_reason env tvar with
           | Some r ->
             (* here we extract the position from the reason attached to the
              * type variable, instead of the type itself. This is currently
              * how patch positions are encoded. This should change *)
             Some (Reason.to_pos r)
           | None -> None)
    end)
      #on_type
      None
      ty
  in
  let builder =
    object (self)
      inherit [_] Tast_visitor.iter_with_state as super

      method private extract_type_hint (env, ty_map) type_hint =
        let hi = fst type_hint in
        match get_global_var_pos (Tast_env.tast_env_as_typing_env env) hi with
        | None -> ()
        | Some pos ->
          let pos = Pos.to_absolute @@ Pos_or_decl.unsafe_to_raw_pos pos in
          ty_map := Pos.AbsolutePosMap.add pos hi !ty_map

      method! on_fun_with_env state ({ Aast.f_ret; _ } as fun_) =
        self#extract_type_hint state f_ret;
        super#on_fun_with_env state fun_

      method! on_method_with_env state ({ Aast.m_ret; _ } as method_) =
        self#extract_type_hint state m_ret;
        super#on_method_with_env state method_

      (* note: method_.m_params is of type fun_param list, so this will
       * also be called for method parameters. *)
      method! on_fun_param state ({ Aast.param_type_hint; _ } as fun_param) =
        self#extract_type_hint state param_type_hint;
        super#on_fun_param state fun_param

      method! on_class_var_with_env state ({ Aast.cv_type; _ } as class_var) =
        self#extract_type_hint state cv_type;
        super#on_class_var_with_env state class_var
    end
  in
  let state = ref Pos.AbsolutePosMap.empty in
  let () = builder#go_def ctx state tast in
  !state

module StateSubConstraintGraphs = struct
  include StateFunctor (struct
    type t = global_type_map * Inf.t_global_with_pos list
  end)

  let global_tvenvs (t : t) : Typing_inference_env.t_global_with_pos list =
    snd t

  let global_type_map (t : t) : global_type_map = fst t

  let build
      (ctx : Provider_context.t)
      (tasts : Tast.def list)
      (genvs : Inf.t_global_with_pos list) : t =
    let tasts = Tast_expand.expand_program ctx tasts in
    let type_map =
      List.fold
        tasts
        ~f:(fun pos_map tast ->
          Pos.AbsolutePosMap.union pos_map @@ build_ty_map ctx tast)
        ~init:Pos.AbsolutePosMap.empty
    in
    (type_map, genvs)

  let save_to : string -> t -> unit = save

  let save (type_map, subconstraints) : unit =
    let subconstraints =
      List.map subconstraints ~f:(fun (p, env) -> (p, Inf.compress_g env))
    in
    let is_not_empty_graph (_p, g) = not @@ List.is_empty (Inf.get_vars_g g) in
    let subconstraints = List.filter ~f:is_not_empty_graph subconstraints in
    if List.is_empty subconstraints && Pos.AbsolutePosMap.is_empty type_map then
      ()
    else
      let path =
        Filename.concat
          !artifacts_path
          ("subgraph_" ^ string_of_int (Ident.tmp ()))
      in
      save path (type_map, subconstraints)

  let build_and_save
      (ctx : Provider_context.t)
      (tasts : Tast.program)
      (genvs : Inf.t_global_with_pos list) : unit =
    build ctx tasts genvs |> save
end

module StateConstraintGraph = struct
  include StateFunctor (struct
    type t = global_type_map * env * StateErrors.t
  end)

  let merge_var (env, errors) (pos, subgraph) var =
    Typing_log.GI.log_merging_var env pos var;
    Env.log_env_change_ "merge_var" env
    @@
    let catch_exc = catch_exc pos in
    let (env, var') =
      Env.copy_tyvar_from_genv_to_env var ~from:subgraph ~to_:env
    in
    let ty = MakeType.tyvar Reason.Rnone var
    and ty' = MakeType.tyvar Reason.Rnone var' in
    let on_err = StateErrors.add errors var in
    let env =
      catch_exc on_err env (fun on_err ->
          let (env, ty_err_opt) = Sub.sub_type env ty ty' on_err in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
          env)
    in
    let env =
      catch_exc on_err env (fun on_err ->
          let (env, ty_err_opt) = Sub.sub_type env ty' ty on_err in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
          env)
    in
    let (env, vars_in_lower_bounds, vars_in_upper_bounds) =
      let get_unsolved_tyvars tyset =
        ITySet.fold
          (fun ty (env, vars_acc) ->
            let (env, ty) = Env.expand_internal_type env ty in
            let vars_acc =
              Option.fold
                (InternalType.get_var ty)
                ~init:vars_acc
                ~f:(fun vars_acc v -> ISet.add v vars_acc)
            in
            (env, vars_acc))
          tyset
          (env, ISet.empty)
      in
      let (env, vars_in_lower_bounds) =
        get_unsolved_tyvars (Env.get_tyvar_lower_bounds env var')
      in
      let (env, vars_in_upper_bounds) =
        get_unsolved_tyvars (Env.get_tyvar_upper_bounds env var')
      in
      (env, vars_in_lower_bounds, vars_in_upper_bounds)
    in
    let env =
      catch_exc on_err env (fun _ ->
          let (env, ty_err_opt) =
            Typing_solver.try_bind_to_equal_bound env var'
          in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
          env)
    in
    let env =
      catch_exc on_err env (fun _ ->
          let (env, ty_err_opt) =
            Typing_solver.try_bind_to_equal_bound env var
          in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
          env)
    in
    let env =
      Env.remove_var
        env
        var'
        ~search_in_lower_bounds_of:vars_in_upper_bounds
        ~search_in_upper_bounds_of:vars_in_lower_bounds
    in
    (env, errors)

  let merge_subgraph (env, errors) ((pos, subgraph) : Inf.t_global_with_pos) =
    Typing_log.GI.log_merging_subgraph env pos;
    let vars = Inf.get_vars_g subgraph in
    List.fold vars ~init:(env, errors) ~f:(fun (env, errors) v ->
        merge_var (env, errors) (pos, subgraph) v)

  let merge_subgraphs
      (ctx : Provider_context.t) (subgraphs : StateSubConstraintGraphs.t list) :
      t =
    let (type_maps, subgraphs) = List.unzip subgraphs in
    let type_map =
      List.fold
        type_maps
        ~f:Pos.AbsolutePosMap.union
        ~init:Pos.AbsolutePosMap.empty
    in
    let subgraphs = List.concat subgraphs in
    let env = Typing_env_types.empty ctx Relative_path.default ~droot:None in
    let errors = StateErrors.mk_empty () in
    if is_ordered_solving env then
      let env = Typing_ordered_solver.merge_graphs env subgraphs in
      (type_map, env, errors)
    else
      (* Collect each global tyvar and map it to a global environment in
         * which it lives. Give preference to the global environment which also
         * has positional information for this type variable *)
      let initial_tyvar_sources : (Pos_or_decl.t * Inf.t_global) IMap.t =
        List.fold subgraphs ~init:IMap.empty ~f:(fun m (_, genv) ->
            List.fold (Inf.get_vars_g genv) ~init:m ~f:(fun m var ->
                IMap.update
                  var
                  (function
                    | Some (pos, genv)
                      when not (Pos_or_decl.equal pos Pos_or_decl.none) ->
                      Some (pos, genv)
                    | None
                    | Some (_, _) ->
                      Some (Inf.get_tyvar_pos_exn_g genv var, genv))
                  m))
      in
      (* copy each initial variable to the new environment *)
      let env =
        IMap.fold
          (fun var (_, genv) env ->
            Env.initialize_tyvar_as_in ~as_in:genv env var)
          initial_tyvar_sources
          env
      in
      let (env, errors) =
        List.fold ~f:merge_subgraph ~init:(env, errors) subgraphs
      in
      (type_map, env, errors)
end

module StateSolvedGraph = struct
  include StateFunctor (struct
    type t = env * StateErrors.t * global_type_map
  end)

  let save path t = save path t

  (** Solve the constraint graph. *)
  let from_constraint_graph ((type_map, env, errors) : StateConstraintGraph.t) :
      t =
    (* For any errors seen during the last step (that is graph merging), we bind
       the corresponding tyvar to Terr *)
    let vars = Env.get_all_tyvars env in
    let env =
      List.fold vars ~init:env ~f:(fun env var ->
          if StateErrors.has_error errors var then (
            let (env, ty) = Env.fresh_type_invariant env Pos.none in
            let (env, ty_err_opt) = Typing_solver.bind env var ty in
            Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
            env
          ) else
            env)
    in
    let make_on_error = StateErrors.add errors in
    let env =
      catch_exc Pos.none (make_on_error 0) env @@ fun _ ->
      let make_on_error v = convert_on_error (make_on_error v) Pos.none in
      if is_ordered_solving env then
        Typing_ordered_solver.solve_env env make_on_error
      else
        let (env, ty_err_opt) =
          Typing_solver.solve_all_unsolved_tyvars_gi env
        in
        Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
        env
    in
    (env, errors, type_map)
end

let set_path () =
  let tmp = Tmp.temp_dir GlobalConfig.tmp_dir "gi_artifacts" in
  artifacts_path := tmp

let get_path () = !artifacts_path

let restore_path s = artifacts_path := s

let init () =
  let path = !artifacts_path in
  Hh_logger.log "Global artifacts path: %s" path;
  Disk.rm_dir_tree path;
  if not @@ Disk.file_exists path then Disk.mkdir path 0o777
