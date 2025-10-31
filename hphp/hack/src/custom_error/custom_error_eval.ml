(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core
module Ty = Typing_defs

exception Invalid_pattern of string * Validation_err.t list

exception Illegal_name of string

module Value = struct
  type t =
    | Ty of (Ty.locl_ty[@compare.ignore] [@sexp.opaque])
    | Name of ((Pos_or_decl.t[@opaque]) * string)
    | File of (Relative_path.t[@opaque])
    | Member_name of string
  [@@deriving compare, sexp]
end

module Match = struct
  module X = struct
    (* TODO[mjt] - decompose into safe and unsafe match? *)
    type ('a, 'err) t =
      | Matched of 'a
      | No_match
      | Match_err of 'err

    let map =
      `Custom
        (fun t ~f ->
          match t with
          | Matched a -> Matched (f a)
          | No_match -> No_match
          | Match_err err -> Match_err err)

    let map_err t ~f =
      match t with
      | Match_err err -> Match_err (f err)
      | Matched a -> Matched a
      | No_match -> No_match

    let bind t ~f =
      match t with
      | Matched x -> f x
      | No_match -> No_match
      | Match_err err -> Match_err err

    let return x = Matched x

    let matched x = Matched x

    let no_match = No_match

    let match_err err = Match_err err
  end

  include X
  include Core.Monad.Make2 (X)
end

module Env = struct
  type t = Env of Value.t String.Map.t

  let empty = Env String.Map.empty

  let add (Env t) ~lbl ~data = Env (Map.add_exn t ~key:lbl ~data)

  let add_name t ~lbl ~name = add t ~lbl ~data:(Value.Name name)

  let add_ty t ~lbl ~ty = add t ~lbl ~data:(Value.Ty ty)

  let add_file t ~lbl ~file = add t ~lbl ~data:(Value.File file)

  let add_member_name t ~lbl ~member_name =
    add t ~lbl ~data:(Value.Member_name member_name)

  let get (Env t) lbl = Map.find_exn t lbl
end

(* -- Helpers --------------------------------------------------------------- *)

let match_opt match_with patt ~scrut ~env =
  match scrut with
  | None -> Match.matched env
  | Some scrut -> match_with patt ~scrut ~env

let match_exists patt ~matches ~scruts ~env =
  let rec aux = function
    | [] -> Match.no_match
    | scrut :: scruts ->
      (match matches patt scrut ~env with
      | Match.Matched _ as m -> m
      | _ -> aux scruts)
  in
  aux scruts

(* -- Member not found pattern helpers -------------------------------------- *)

let matches_static_pattern patt_is_static ~scrut ~env =
  let open Patt_typing_error in
  match patt_is_static with
  | None -> Match.matched env (* No static pattern means match any *)
  | Some Static_only ->
    (match scrut with
    | `static -> Match.matched env
    | `instance -> Match.no_match)
  | Some Instance_only ->
    (match scrut with
    | `instance -> Match.matched env
    | `static -> Match.no_match)

let matches_member_kind_instance patt_kind ~scrut ~env =
  let open Patt_typing_error in
  let scrut_kind_matches = function
    | Any_member_kind -> true
    | Method_only ->
      (match scrut with
      | `method_ -> true
      | _ -> false)
    | Property_only ->
      (match scrut with
      | `property -> true
      | _ -> false)
    | Class_constant_only
    | Class_typeconst_only ->
      false (* These don't exist for instance members *)
  in
  if scrut_kind_matches patt_kind then
    Match.matched env
  else
    Match.no_match

let matches_member_kind_static patt_kind ~scrut ~env =
  let open Patt_typing_error in
  let scrut_kind_matches = function
    | Any_member_kind -> true
    | Method_only ->
      (match scrut with
      | `static_method -> true
      | _ -> false)
    | Property_only ->
      (match scrut with
      | `class_variable -> true
      | _ -> false)
    | Class_constant_only ->
      (match scrut with
      | `class_constant -> true
      | _ -> false)
    | Class_typeconst_only ->
      (match scrut with
      | `class_typeconst -> true
      | _ -> false)
  in
  if scrut_kind_matches patt_kind then
    Match.matched env
  else
    Match.no_match

(* -- Strings --------------------------------------------------------------- *)

let matches_patt_string_help t ~scrut =
  let open Patt_string in
  let rec aux = function
    | Wildcard -> true
    | Exactly str -> String.equal scrut str
    | Starts_with prefix -> String.is_prefix scrut ~prefix
    | Ends_with suffix -> String.is_suffix scrut ~suffix
    | Contains substring -> String.is_substring scrut ~substring
    | Or ts -> List.exists ~f:aux ts
    | And ts -> List.for_all ~f:aux ts
    | Not t -> not @@ aux t
  in
  aux t

(* Since this pattern can never bind a variable we can't produce a match
   error. We use this invariant to simply the inner matching function *)
let matches_string t ~scrut ~env =
  if matches_patt_string_help t ~scrut then
    Match.matched env
  else
    Match.no_match

(* -- Names ----------------------------------------------------------------- *)

let rec matches_member_name t ~scrut ~env =
  let open Patt_member_name in
  match t with
  | Wildcard -> Match.matched env
  | Member_name { patt_string } -> matches_string patt_string ~scrut ~env
  | As { lbl; patt } ->
    Match.(
      matches_member_name patt ~scrut ~env
      |> map_err ~f:(fun _ -> assert false)
      |> map ~f:(Env.add_member_name ~lbl ~member_name:scrut))
  | Invalid { errs; _ } -> Match.match_err errs

let split_namespace (_pos, name) =
  let ls =
    List.rev
    @@ List.filter ~f:(fun s -> not @@ String.is_empty s)
    @@ String.split ~on:'\\' name
  in
  match ls with
  | name :: namespace -> Some (name, namespace)
  | _ -> None

let rec matches_name ?(env = Env.empty) t ~scrut =
  let open Patt_name in
  match t with
  | Wildcard -> Match.matched env
  | Name { patt_namespace; patt_name } ->
    (match split_namespace scrut with
    | Some (name, namespace) ->
      Match.(
        matches_string patt_name ~scrut:name ~env >>= fun _ ->
        matches_namespace patt_namespace ~scrut:namespace ~env)
    | _ -> raise (Illegal_name (snd scrut)))
  | As { lbl; patt } ->
    Match.(
      matches_name patt ~scrut ~env
      |> map_err ~f:(fun _ -> assert false)
      |> map ~f:(Env.add_name ~lbl ~name:scrut))
  | Invalid { errs; _ } -> Match.match_err errs

and matches_namespace patt_namespace ~scrut ~env =
  let open Patt_name in
  match (patt_namespace, scrut) with
  | (Root, []) -> Match.return env
  | (Slash { prefix; elt }, next :: rest) ->
    Match.(
      matches_string elt ~scrut:next ~env >>= fun _ ->
      matches_namespace prefix ~scrut:rest ~env)
  | _ -> Match.no_match

(* -- Files ----------------------------------------------------------------- *)

let rec matches_file_path t ~scrut ~env =
  let open Patt_file in
  match (t, scrut) with
  | (Dot, []) -> Match.matched env
  | (Dot, _) -> Match.no_match
  | (Slash { prefix; segment }, next :: rest) ->
    Match.(
      matches_string segment ~scrut:next ~env >>= fun env ->
      matches_file_path prefix ~scrut:rest ~env)
  | (Slash _, []) -> Match.no_match

let rec matches_file t ~(scrut : Relative_path.t) ~env =
  let open Patt_file in
  match t with
  | Wildcard -> Match.matched env
  | As { lbl; patt } ->
    Match.(
      matches_file patt ~scrut ~env
      |> map_err ~f:(fun _ -> assert false)
      |> map ~f:(Env.add_file ~lbl ~file:scrut))
  | Invalid { errs; _ } -> Match.match_err errs
  | Name { patt_file_path; patt_file_name; patt_file_extension } ->
    let path = String.split ~on:'/' Relative_path.(suffix scrut) in
    (match List.rev path with
    | name :: scrut_path ->
      let (scrut_name, scrut_extensionopt) =
        match String.lsplit2 ~on:'.' name with
        | Some (name, ext) -> (name, Some ext)
        | None -> (name, None)
      in
      Match.(
        matches_string patt_file_name ~scrut:scrut_name ~env >>= fun env ->
        match_opt
          matches_string
          patt_file_extension
          ~scrut:scrut_extensionopt
          ~env
        >>= fun env ->
        Option.value_map
          patt_file_path
          ~default:(Match.matched env)
          ~f:(fun patt_file_path ->
            matches_file_path patt_file_path ~scrut:scrut_path ~env))
    | _ -> Match.no_match)

(* -- Types ----------------------------------------------------------------- *)

type shape_fields_split = {
  groups: Ty.locl_phase Ty.shape_field_type String.Map.t;
  strs: Ty.locl_phase Ty.shape_field_type String.Map.t;
  cconsts: Ty.locl_phase Ty.shape_field_type String.Map.t;
}

let split_fields flds =
  let init =
    {
      groups = String.Map.empty;
      strs = String.Map.empty;
      cconsts = String.Map.empty;
    }
  in
  List.fold
    ~f:(fun acc (k, ty) ->
      match k with
      | Ty.TSFregex_group (_, n) ->
        { acc with groups = Map.add_exn ~key:n ~data:ty acc.groups }
      | Ty.TSFlit_str (_, nm) ->
        { acc with strs = Map.add_exn ~key:nm ~data:ty acc.strs }
      | Ty.TSFclass_const ((_, cls_nm), (_, cnst_nm)) ->
        {
          acc with
          cconsts =
            Map.add_exn ~key:(cls_nm ^ "::" ^ cnst_nm) ~data:ty acc.cconsts;
        })
    ~init
  @@ Ty.TShapeMap.elements flds

let matches_locl_ty ?(env = Env.empty) t ~scrut =
  let open Patt_locl_ty in
  let rec aux t ty ~env =
    let ty_pos = Ty.get_pos ty in
    match (t, Ty.get_node ty) with
    (* -- Type wildcards ---------------------------------------------------- *)
    | (Any, _) -> Match.Matched env
    (* -- Type constructor like types --------------------------------------- *)
    | (Apply { patt_name; patt_params }, Ty.Tclass (name, _, tys)) ->
      Match.(
        matches_name patt_name ~scrut:name ~env >>= fun env ->
        aux_params patt_params tys ~env)
    | (Apply { patt_name; patt_params }, Ty.Tnewtype (id, tys, _)) ->
      Match.(
        matches_name patt_name ~scrut:(ty_pos, id) ~env >>= fun env ->
        aux_params patt_params tys ~env)
    | (Apply { patt_name; patt_params }, Ty.Tvec_or_dict (ty_key, ty_val)) ->
      Match.(
        matches_name patt_name ~scrut:(ty_pos, "vec_or_dict") ~env
        >>= fun env -> aux_params patt_params [ty_key; ty_val] ~env)
    | (Option t, Ty.Toption ty) -> aux t ty ~env
    (* TODO optional and variadic fields T201398626 T201398652 *)
    | ( Tuple ts,
        Ty.(
          Ttuple
            { t_required; t_extra = Textra { t_optional = []; t_variadic } }) )
      when Ty.is_nothing t_variadic ->
      aux_tuple ts t_required ~env
    (* -- Primitives & other base types ------------------------------------- *)
    | (Prim Null, Ty.Tprim Ast_defs.Tnull)
    | (Prim Void, Ty.Tprim Ast_defs.Tvoid)
    | (Prim Int, Ty.Tprim Ast_defs.Tint)
    | (Prim Bool, Ty.Tprim Ast_defs.Tbool)
    | (Prim Float, Ty.Tprim Ast_defs.Tfloat)
    | (Prim String, Ty.Tprim Ast_defs.Tstring)
    | (Prim Resource, Ty.Tprim Ast_defs.Tresource)
    | (Prim Num, Ty.Tprim Ast_defs.Tnum)
    | (Prim Arraykey, Ty.Tprim Ast_defs.Tarraykey)
    | (Prim Noreturn, Ty.Tprim Ast_defs.Tnoreturn)
    | (Dynamic, Ty.Tdynamic)
    | (Nonnull, Ty.Tnonnull) ->
      Match.matched env
    (* -- Variable binding -------------------------------------------------- *)
    | (As { lbl; patt }, _) ->
      Match.(aux patt ty ~env |> map ~f:(Env.add_ty ~lbl ~ty))
    (* -- Or patterns ------------------------------------------------------- *)
    | (Or { patt_fst; patt_snd }, _) ->
      (match aux patt_fst ty ~env with
      | Match.Matched _ as m -> m
      | _ -> aux patt_snd ty ~env)
    (* -- Shapes ------------------------------------------------------------ *)
    | (Shape patt_flds, Ty.(Tshape { s_fields = flds; _ })) ->
      (* Split [TShapeMap.t] into three [String.Map.t]s, eliminating positional
         information *)
      let splits = split_fields flds in
      aux_shape patt_flds splits ~env
    (* -- Mismatches -------------------------------------------------------- *)
    | ((Apply _ | Option _ | Tuple _ | Prim _ | Dynamic | Nonnull | Shape _), _)
      ->
      Match.no_match
    | (Invalid (errs, _), _) -> Match.match_err errs
  and aux_shape t splits ~env =
    match t with
    | Fld { patt_fld; patt_rest } ->
      (match aux_shape_field patt_fld splits ~env with
      | Match.Matched (splits, env) -> aux_shape patt_rest splits ~env
      | Match.No_match -> Match.No_match
      | Match.Match_err err -> Match.match_err err)
    (* -- Open matches any remaining shape fields --------------------------- *)
    | Open -> Match.matched env
    (* -- Closed matches when we have no remaining shape fields ------------- *)
    | Closed
      when Map.is_empty splits.groups
           && Map.is_empty splits.strs
           && Map.is_empty splits.cconsts ->
      Match.matched env
    | Closed -> Match.no_match
  and aux_shape_field fld splits ~env =
    let aux ~get ~set str =
      let split = get splits in
      Option.value_map
        ~default:Match.no_match
        ~f:(fun Ty.{ sft_optional; sft_ty } ->
          if Bool.(sft_optional = fld.optional) then
            match aux fld.patt sft_ty ~env with
            | Match.Matched env ->
              Match.matched (set splits (Map.remove split str), env)
            | Match.No_match -> Match.No_match
            | Match.Match_err err -> Match.Match_err err
          else
            Match.no_match)
      @@ Map.find split str
    in
    (* These funcs can be derived with ppx_sexp_conv *)
    match fld.lbl with
    | RegGroupLabel n ->
      aux
        ~get:(fun splits -> splits.groups)
        ~set:(fun splits groups -> { splits with groups })
        n
    | StrLbl str ->
      aux
        ~get:(fun splits -> splits.strs)
        ~set:(fun splits strs -> { splits with strs })
        str
    | CConstLbl { cls_nm; cnst_nm } ->
      let str = cls_nm ^ "::" ^ cnst_nm in
      aux
        ~get:(fun splits -> splits.cconsts)
        ~set:(fun splits cconsts -> { splits with cconsts })
        str
  and aux_params t tys ~env =
    match (t, tys) with
    | (Cons { patt_hd; patt_tl }, ty :: tys) ->
      Match.(aux patt_hd ty ~env >>= fun env -> aux_params patt_tl tys ~env)
    | (Exists t, _) -> match_exists t ~matches:aux ~scruts:tys ~env
    | (Nil, [])
    | (Wildcard, _) ->
      Match.matched env
    | (Nil, _)
    | (Cons _, []) ->
      Match.no_match
  and aux_tuple ts t_required ~env =
    let rec aux_tys ts tys ~env =
      match (ts, tys) with
      | (t :: ts, ty :: tys) ->
        Match.(aux t ty ~env >>= fun env -> aux_tys ts tys ~env)
      | ([], []) -> Match.matched env
      | _ -> Match.no_match
    in
    aux_tys ts t_required ~env
  in
  aux t scrut ~env

(* -- Typing errors --------------------------------------------------------- *)

let matches_typing_error t ~scrut ~env =
  let open Patt_typing_error in
  (* -- Top-level typing errors --------------------------------------------- *)
  let rec aux t err ~env =
    match (t, err) with
    | (_, Typing_error.Error.Intersection errs)
    | (_, Typing_error.Error.Union errs)
    | (_, Typing_error.Error.Multiple errs) ->
      match_exists t ~matches:aux ~scruts:errs ~env
    (* -- Primary errors ---------------------------------------------------- *)
    | (Primary patt_prim, Typing_error.Error.Primary err_prim) ->
      aux_primary patt_prim err_prim ~env
    (* -- Callback application ---------------------------------------------- *)
    | (Apply { patt_cb; patt_err }, Typing_error.Error.Apply (cb, err)) ->
      Match.(aux_callback patt_cb cb ~env >>= fun env -> aux patt_err err ~env)
    (* -- Reasons callback application -------------------------------------- *)
    | ( Apply_reasons { patt_rsns_cb; patt_secondary },
        Typing_error.Error.Apply_reasons (err_rsns_cb, err_secondary) ) ->
      Match.(
        aux_reasons_callback patt_rsns_cb err_rsns_cb ~env >>= fun env ->
        aux_secondary patt_secondary err_secondary ~env)
    (* -- Or patterns ------------------------------------------------------- *)
    | (Or { patt_fst; patt_snd }, _) ->
      (match aux patt_fst err ~env with
      | Match.Matched _ as m -> m
      | _ -> aux patt_snd err ~env)
    (* -- Mismatches -------------------------------------------------------- *)
    | (Primary _, _)
    | (Apply _, _)
    | (Apply_reasons _, _) ->
      Match.no_match
    | (Invalid_typing { errs; _ }, _) -> Match.match_err errs
  (* -- Primary errors ------------------------------------------------------ *)
  and aux_primary t err_prim ~env =
    match (t, err_prim) with
    | (Any_prim, _) -> Match.matched env
    (* -- Member not found patterns ----------------------------------------- *)
    | ( Member_not_found
          {
            patt_is_static;
            patt_kind;
            patt_class_name;
            patt_member_name;
            patt_visibility = _;
          },
        Typing_error.Primary.Member_not_found
          { kind; class_pos; class_name; member_name; _ } ) ->
      Match.(
        matches_static_pattern patt_is_static ~scrut:`instance ~env
        >>= fun env ->
        matches_member_kind_instance patt_kind ~scrut:kind ~env >>= fun env ->
        matches_name patt_class_name ~scrut:(class_pos, class_name) ~env
        >>= fun env ->
        matches_member_name patt_member_name ~scrut:member_name ~env)
    | ( Member_not_found
          {
            patt_is_static;
            patt_kind;
            patt_class_name;
            patt_member_name;
            patt_visibility = _;
          },
        Typing_error.Primary.Smember_not_found
          { kind; class_pos; class_name; member_name; _ } ) ->
      Match.(
        matches_static_pattern patt_is_static ~scrut:`static ~env >>= fun env ->
        matches_member_kind_static patt_kind ~scrut:kind ~env >>= fun env ->
        matches_name patt_class_name ~scrut:(class_pos, class_name) ~env
        >>= fun env ->
        matches_member_name patt_member_name ~scrut:member_name ~env)
    | (Member_not_found _, _) -> Match.no_match
    (* -- Package errors ---------------------------------------------------- *)
    | ( Cross_pkg_access { patt_use_file; patt_decl_file },
        Typing_error.Primary.(
          Package (Package.Cross_pkg_access { pos; decl_pos; _ })) ) ->
      Match.(
        matches_file patt_use_file ~scrut:(Pos.filename pos) ~env >>= fun env ->
        matches_file patt_decl_file ~scrut:(Pos_or_decl.filename decl_pos) ~env)
    | (Cross_pkg_access _, _) -> Match.no_match
    | ( Cross_pkg_access_with_requirepackage { patt_use_file; patt_decl_file },
        Typing_error.Primary.(
          Package
            (Package.Cross_pkg_access_with_requirepackage { pos; decl_pos; _ }))
      ) ->
      Match.(
        matches_file patt_use_file ~scrut:(Pos.filename pos) ~env >>= fun env ->
        matches_file patt_decl_file ~scrut:(Pos_or_decl.filename decl_pos) ~env)
    | (Cross_pkg_access_with_requirepackage _, _) -> Match.no_match
  (* -- Secondary errors ---------------------------------------------------- *)
  and aux_secondary t err_snd ~env =
    match (t, err_snd) with
    | (Of_error patt_err, Typing_error.Secondary.Of_error err) ->
      aux patt_err err ~env
    | (Of_error _, _) -> Match.no_match
    (* We don't expose our internal `constraint_ty` so we handle this here *)
    | ( Violated_constraint { patt_cstr; patt_ty_sub; patt_ty_sup },
        Typing_error.Secondary.Violated_constraint { cstrs; ty_sub; ty_sup; _ }
      ) ->
      Match.(
        match_exists patt_cstr ~matches:aux_param ~scruts:cstrs ~env
        >>= fun env ->
        aux_internal_ty patt_ty_sub ty_sub ~env >>= fun env ->
        aux_internal_ty patt_ty_sup ty_sup ~env)
    | (Violated_constraint _, _) -> Match.no_match
    | ( Subtyping_error { patt_ty_sub; patt_ty_sup },
        Typing_error.Secondary.Subtyping_error { ty_sub; ty_sup; _ } ) ->
      Match.(
        aux_internal_ty patt_ty_sub ty_sub ~env >>= fun env ->
        aux_internal_ty patt_ty_sup ty_sup ~env)
    | (Subtyping_error _, _) -> Match.no_match
    | (Any_snd, _) -> Match.matched env
  (* -- Primary callbacks --------------------------------------------------- *)
  and aux_callback t err_callback ~env =
    match (t, err_callback) with
    | (Any_callback, _) -> Match.matched env
  (* -- Secondary / reasons callbacks --------------------------------------- *)
  and aux_reasons_callback t err_reasons_callback ~env =
    match (t, err_reasons_callback) with
    | (Any_reasons_callback, _) -> Match.matched env
  (* -- Internal types: we don't expose contraint types so handle here ------ *)
  and aux_internal_ty patt_locl_ty internal_ty ~env =
    match internal_ty with
    | Typing_defs_constraints.LoclType scrut ->
      matches_locl_ty patt_locl_ty ~scrut ~env
    | _ -> Match.no_match
  (* -- Type parameters ----------------------------------------------------- *)
  and aux_param patt_name (_, (_, scrut)) ~env =
    matches_string patt_name ~scrut ~env
  in
  aux t scrut ~env

(* -- Naming errors --------------------------------------------------------- *)

let matches_name_context patt_name_context ~scrut ~env =
  let open Patt_naming_error in
  match patt_name_context with
  | Any_name_context -> Match.matched env
  | One_of_name_context ctxts
    when List.mem ctxts scrut ~equal:Name_context.equal ->
    Match.matched env
  | _ -> Match.no_match

let matches_naming_error t ~scrut ~env =
  let open Patt_naming_error in
  match (t, scrut) with
  | ( Unbound_name { patt_name_context; patt_name },
      Naming_error.Unbound_name { pos; name; kind } ) ->
    Match.(
      matches_name_context patt_name_context ~scrut:kind ~env >>= fun env ->
      let pos_or_decl = Pos_or_decl.of_raw_pos pos in
      let scrut = (pos_or_decl, name) in
      matches_name patt_name ~scrut ~env)
  | _ -> Match.no_match
(* -- Top level error matches ----------------------------------------------- *)

type phase_error =
  | Typing_phase of Typing_error.t
  | Naming_phase of Naming_error.t

let matches_error ?(env = Env.empty) t ~scrut =
  match (t, scrut) with
  | (Patt_error.Typing patt_typing_error, Typing_phase scrut) ->
    matches_typing_error patt_typing_error ~scrut ~env
  | (Patt_error.Naming patt_naming_error, Naming_phase scrut) ->
    matches_naming_error patt_naming_error ~scrut ~env
  | _ -> Match.no_match

let eval_error_message Error_message.{ message } ~env =
  let open Error_message in
  let f t =
    match t with
    | Lit str -> Either.First str
    | File_var var
    | Ty_var var
    | Name_var var
    | Member_name_var var ->
      Either.Second (Env.get env var)
  in
  List.map ~f message

let eval_custom_error_v1 patt_typing_err error_message ~err =
  match matches_typing_error patt_typing_err ~scrut:err ~env:Env.empty with
  | Match.Matched env -> Some (eval_error_message error_message ~env)
  | _ -> None

let eval_custom_error_v2 patt_err error_message ~err =
  match matches_error patt_err ~scrut:err with
  | Match.Matched env -> Some (eval_error_message error_message ~env)
  | _ -> None

let eval_custom_error
    Custom_error.{ patt; error_message = Message_v1 error_message; _ }
    ~(err : phase_error) =
  match (patt, err) with
  | (Error_v1 patt, Typing_phase err) ->
    eval_custom_error_v1 patt error_message ~err
  | (Error_v1 _, _) ->
    (* Version 1 only supports typing errors *)
    None
  | (Error_v2 patt, _) -> eval_custom_error_v2 patt error_message ~err

let eval_naming_error Custom_error_config.{ valid; _ } ~(err : Naming_error.t) =
  let err = Naming_phase err in
  List.filter_map ~f:(eval_custom_error ~err) valid

let eval_typing_error Custom_error_config.{ valid; _ } ~(err : Typing_error.t) =
  let err = Typing_phase err in
  List.filter_map ~f:(eval_custom_error ~err) valid
