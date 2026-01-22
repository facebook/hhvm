(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Pretty printing of types *)
(*****************************************************************************)

open Hh_prelude
open Typing_defs
open Typing_defs_constraints
open Typing_env_types
open Typing_logic
module SN = Naming_special_names
module Reason = Typing_reason
module TySet = Typing_set
module Cls = Folded_class
module Nast = Aast
module ITySet = Internal_type_set

(** Fuel ensures that types are curtailed while printing them. This avoids
   performance regressions and increases readibility of errors overall. *)
module Fuel : sig
  type t

  val init : int -> t

  val has_enough : t -> bool

  (** [provide fuel f] consumes one unit of fuel to call f, plus fuel consumed by f itself,
    provided there's enough fuel. If not, it returns the text doc "[truncated]". *)
  val provide : t -> (fuel:t -> t * Doc.t) -> t * Doc.t

  (** [provide_list fuel f inputs ~out_of_fuel_message] consumes fuel to call f
    on each element of a list of inputs until we're out of fuel.
    It consumes one unit of fuel per call to f plus the fuel consumed by f itself.
    When out of fuel, it appends the text doc "[out_of_fuel_message]" to the results. *)
  val provide_list :
    t ->
    ('input -> fuel:t -> t * Doc.t) ->
    'input list ->
    out_of_fuel_message:string ->
    t * Doc.t list
end = struct
  type t = int

  let init fuel = fuel

  let deplete fuel = fuel - 1

  let has_enough fuel = fuel >= 0

  let provide_ (fuel : t) (f : fuel:t -> t * Doc.t) : t * Doc.t option =
    if has_enough fuel then
      let fuel = deplete fuel in
      let (fuel, doc) = f ~fuel in
      (fuel, Some doc)
    else
      (fuel, None)

  let provide_list
      (fuel : t)
      (f : 'input -> fuel:t -> t * Doc.t)
      (inputs : 'input list)
      ~(out_of_fuel_message : string) : t * Doc.t list =
    let ((fuel, ran_out), docs) =
      List.fold
        inputs
        ~init:((fuel, false), [])
        ~f:(fun ((fuel, _), docs) input ->
          let (fuel, doc) = provide_ fuel (f input) in
          match doc with
          | Some doc -> ((fuel, false), doc :: docs)
          | None -> ((fuel, true), docs))
    in
    let docs =
      if ran_out then
        Doc.text (Printf.sprintf "[%s]" out_of_fuel_message) :: docs
      else
        docs
    in
    let docs = List.rev docs in
    (fuel, docs)

  let provide fuel f =
    let (fuel, doc) = provide_ fuel f in
    let doc = Option.value doc ~default:(Doc.text "[truncated]") in
    (fuel, doc)
end

(** For sake of typing_print, either we wish to print a locl_ty in which case we need
the env to look up the typing environment and constraints and the like, or a decl_ty
in which case we don't need anything. [penv] stands for "printing env". *)
type penv =
  | Loclenv of env
  | Declenv

let split_desugared_ctx_tparams_gen ~tparams ~param_name =
  let (generated_tparams, other_tparams) =
    List.partition_tf tparams ~f:(fun param ->
        param_name param |> SN.Coeffects.is_generated_generic)
  in
  let desugared_tparams =
    List.map generated_tparams ~f:(fun param ->
        param_name param |> SN.Coeffects.unwrap_generated_generic)
  in
  (desugared_tparams, other_tparams)

let strip_ns id =
  id |> Utils.strip_ns |> Hh_autoimport.strip_HH_namespace_if_autoimport

let show_supportdyn env =
  (not (TypecheckerOptions.everything_sdt env.genv.tcopt))
  || Typing_env_types.get_log_level env "show" >= 1

(*****************************************************************************)
(* Pretty-printer of the "full" type.                                        *)
(* This is used in server/symbolTypeService and elsewhere                    *)
(*****************************************************************************)

module Full = struct
  open Doc

  let format_env = Format_env.{ default with line_width = 60 }

  let text_strip_ns s = Doc.text (strip_ns s)

  let text_strip_all_ns s = Doc.text (Utils.strip_all_ns s)

  let ( ^^ ) a b = Concat [a; b]

  let show_verbose penv =
    match penv with
    | Loclenv env -> Typing_env_types.get_log_level env "show" > 1
    | Declenv -> false

  (* Until we support NoAutoDynamic, all types under everything-sdt
   * support dynamic so displaying supportdyn is redundant
   *)
  let show_supportdyn_penv penv =
    match penv with
    | Loclenv env -> show_supportdyn env
    | Declenv -> false

  let comma_sep = Concat [text ","; Space]

  let semi_sep = Concat [text ";"; Space]

  let id ~fuel x = (fuel, x)

  let list_sep
      ~fuel
      ?(split = true)
      (s : Doc.t)
      (f : fuel:Fuel.t -> 'a -> Fuel.t * Doc.t)
      (l : 'a list) : Fuel.t * Doc.t =
    let split =
      if split then
        Split
      else
        Nothing
    in
    let max_idx = List.length l - 1 in
    let (fuel, elements) =
      List.fold_mapi l ~init:fuel ~f:(fun idx fuel element ->
          let (fuel, d) = f ~fuel element in
          if Int.equal idx max_idx then
            (fuel, d)
          else
            (fuel, Concat [d; s; split]))
    in
    let d =
      match elements with
      | [] -> Nothing
      | xs -> Nest [split; Concat xs; split]
    in
    (fuel, d)

  let delimited_list
      ~fuel sep ~skip_brackets_for_singleton left_delimiter f l right_delimiter
      : Fuel.t * Doc.t =
    let (fuel, doc) = list_sep ~fuel sep f l in
    let doc =
      if skip_brackets_for_singleton && Int.equal 1 (List.length l) then
        doc
      else
        Span
          [
            text left_delimiter;
            WithRule (Rule.Parental, Concat [doc; text right_delimiter]);
          ]
    in
    (fuel, doc)

  let list
      ~fuel
      ld
      (printer : fuel:Fuel.t -> 'a -> Fuel.t * Doc.t)
      (items : 'a list)
      rd : Fuel.t * Doc.t =
    delimited_list
      ~fuel
      ~skip_brackets_for_singleton:false
      comma_sep
      ld
      printer
      items
      rd

  let shape_map
      ~fuel
      (fdm : 'field TShapeMap.t)
      (f_field : tshape_field_name * 'field -> fuel:Fuel.t -> Fuel.t * t) :
      Fuel.t * t list =
    let compare (k1, _) (k2, _) =
      String.compare
        (Typing_defs.TShapeField.name k1)
        (Typing_defs.TShapeField.name k2)
    in
    let fields = List.sort ~compare (TShapeMap.bindings fdm) in
    Fuel.provide_list
      fuel
      f_field
      fields
      ~out_of_fuel_message:"shape was truncated"

  (* Split type parameters [tparams] into normal user-denoted type
     parameters, and a list of polymorphic context names that were
     desugared to type parameters. *)
  let split_desugared_ctx_tparams (tparams : 'a tparam list) :
      string list * 'a tparam list =
    let param_name { tp_name = (_, name); _ } = name in
    split_desugared_ctx_tparams_gen ~tparams ~param_name

  let where_constraint ~fuel ~ty to_doc st penv (where : 'a ty where_constraint)
      =
    let (left_ty, (kind : Ast_defs.constraint_kind), right_ty) = where in
    let (fuel, left_ty_doc) = ty ~fuel to_doc st penv left_ty in
    let kind_doc =
      match kind with
      | Ast_defs.Constraint_as -> text "as"
      | Ast_defs.Constraint_eq -> text "="
      | Ast_defs.Constraint_super -> text "super"
    in
    let (fuel, right_ty_doc) = ty ~fuel to_doc st penv right_ty in
    (fuel, Concat [left_ty_doc; Space; kind_doc; Space; right_ty_doc])

  let rec is_supportdyn_mixed : type a. penv -> a ty -> bool =
   fun env t ->
    match get_node t with
    | Tnewtype (n, [ty], _)
      when String.equal n SN.Classes.cSupportDyn
           && not (show_supportdyn_penv env) ->
      is_supportdyn_mixed env ty
    | Tapply ((_, n), [ty])
      when String.equal n SN.Classes.cSupportDyn
           && not (show_supportdyn_penv env) ->
      is_supportdyn_mixed env ty
    | Toption t ->
      (match get_node t with
      | Tnonnull -> true
      | _ -> false)
    | Tmixed -> true
    | _ -> false

  let rec fun_type
      ~fuel
      ~ty
      to_doc
      st
      penv
      ~verbose
      (ft : 'a ty fun_type)
      fun_implicit_params =
    let {
      ft_tparams;
      ft_where_constraints;
      ft_params;
      ft_implicit_params;
      ft_ret;
      ft_flags;
      ft_instantiated = _;
    } =
      ft
    in
    let {
      Typing_defs_flags.Fun.return_disposable;
      async;
      generator = _;
      fun_kind = _;
      is_function_pointer = _;
      returns_readonly = _;
      readonly_this = _;
      support_dynamic_type = _;
      is_memoized = _;
      variadic = _;
    } =
      Typing_defs_flags.Fun.as_record ft_flags
    in
    let return_disposable_doc =
      if verbose && return_disposable then
        Concat [text "<<__ReturnDisposable>>"; Space]
      else
        Nothing
    in
    let async_doc =
      if verbose && async then
        Concat [text "async"; Space]
      else
        Nothing
    in
    let (fuel, params) =
      let param_count = List.length ft_params in
      List.fold_mapi ft_params ~init:fuel ~f:(fun i fuel p ->
          let (fuel, d) = fun_param ~fuel ~ty to_doc st penv ~verbose p in
          ( fuel,
            if get_ft_variadic ft && i + 1 = param_count then
              Concat [d; text "..."]
            else
              d ))
    in

    let (fuel, tparams_doc) =
      let (_, tparams) = split_desugared_ctx_tparams ft_tparams in
      if List.is_empty tparams then
        (fuel, Nothing)
      else
        list ~fuel "<" (tparam ~ty to_doc st penv) tparams ">"
    in
    let (fuel, return_doc) = ty ~fuel to_doc st penv ft_ret in
    let (fuel, capabilities_doc) =
      fun_implicit_params ~fuel to_doc st penv ft_tparams ft_implicit_params
    in
    let (fuel, params_doc) = list ~fuel "(" id params ")" in
    let (fuel, where_doc) =
      if verbose then
        match ft_where_constraints with
        | [] -> (fuel, Nothing)
        | _ ->
          let (fuel, where_doc) =
            list_sep
              ~fuel
              comma_sep
              (where_constraint ~ty to_doc st penv)
              ft_where_constraints
          in
          (fuel, Concat [Space; text "where"; Space; where_doc])
      else
        (fuel, Nothing)
    in
    let tparams_doc =
      Span
        [
          tparams_doc;
          params_doc;
          capabilities_doc;
          Space;
          (if get_ft_returns_readonly ft then
            text "readonly" ^^ Space
          else
            Nothing);
          return_doc;
          where_doc;
        ]
    in
    (fuel, return_disposable_doc, async_doc, tparams_doc)

  and fun_param
      ~fuel
      ~ty
      to_doc
      st
      penv
      ~verbose
      ({ fp_name; fp_type; fp_flags; fp_pos = _; fp_def_value = _ } :
        'a ty fun_param) =
    let {
      Typing_defs_flags.FunParam.accept_disposable;
      inout;
      is_optional;
      readonly;
      ignore_readonly_error = _;
      splat;
      named;
    } =
      Typing_defs_flags.FunParam.as_record fp_flags
    in
    let (fuel, d) = ty ~fuel to_doc st penv fp_type in
    let (fuel, d) =
      match (fp_name, d) with
      | (None, _) -> ty ~fuel to_doc st penv fp_type
      | (Some param_name, Text ("_", 1)) ->
        (* Handle the case of missing a type by not printing it *)
        (fuel, text param_name)
      | (Some param_name, _) ->
        let (fuel, d) = ty ~fuel to_doc st penv fp_type in
        let d = Concat [d; Space; text param_name] in
        (fuel, d)
    in
    let d =
      Concat
        [
          (if verbose && accept_disposable then
            text "<<__AcceptDisposable>>" ^^ Space
          else
            Nothing);
          (if named then
            text "named" ^^ Space
          else
            Nothing);
          (if is_optional then
            text "optional" ^^ Space
          else
            Nothing);
          (if inout then
            text "inout" ^^ Space
          else
            Nothing);
          (if readonly then
            text "readonly" ^^ Space
          else
            Nothing);
          (if splat then
            text "..."
          else
            Nothing);
          d;
        ]
    in
    (fuel, d)

  and tparam
      ~fuel
      ~ty
      to_doc
      st
      env
      { tp_name = (_, x); tp_constraints = cstrl; tp_reified = r; _ } =
    let cstrl =
      List.filter cstrl ~f:(fun cstr ->
          match cstr with
          | (Ast_defs.Constraint_as, ty) when is_supportdyn_mixed env ty ->
            false
          | _ -> true)
    in
    let (fuel, tparam_constraints_doc) =
      list_sep
        ~fuel
        ~split:false
        Space
        (tparam_constraint ~ty to_doc st env)
        cstrl
    in
    let tparam_doc =
      Concat
        [
          begin
            match r with
            | Nast.Erased -> Nothing
            | Nast.SoftReified -> text "<<__Soft>> reify" ^^ Space
            | Nast.Reified -> text "reify" ^^ Space
          end;
          text x;
          tparam_constraints_doc;
        ]
    in
    (fuel, tparam_doc)

  and tparam_constraint ~fuel ~ty to_doc st penv (ck, cty) =
    let (fuel, constraint_ty_doc) = ty ~fuel to_doc st penv cty in
    let constraint_doc =
      Concat
        [
          Space;
          text
            (match ck with
            | Ast_defs.Constraint_as -> "as"
            | Ast_defs.Constraint_super -> "super"
            | Ast_defs.Constraint_eq -> "=");
          Space;
          constraint_ty_doc;
        ]
    in
    (fuel, constraint_doc)

  let tprim x = text @@ Aast_defs.string_of_tprim x

  let tfun ~fuel ~ty to_doc st penv ~verbose ft fun_implicit_params =
    let (fuel, return_disposable_doc, async_doc, fun_type_doc) =
      fun_type ~fuel ~ty to_doc st penv ~verbose ft fun_implicit_params
    in
    let tfun_doc =
      Concat
        [
          return_disposable_doc;
          text "(";
          async_doc;
          (if get_ft_readonly_this ft then
            text "readonly "
          else
            Nothing);
          text "function";
          fun_type_doc;
          text ")";
        ]
    in
    (fuel, tfun_doc)

  let ttuple ~fuel k t_required t_optional t_extra =
    let tuple_elem ~fuel (is_optional, is_variadic, is_splat, ty) =
      let (fuel, doc) = k ~fuel ty in
      ( fuel,
        if is_optional then
          Concat [text "optional "; doc]
        else if is_variadic then
          Concat [doc; text "..."]
        else if is_splat then
          Concat [text "..."; doc]
        else
          doc )
    in
    let optional_list =
      List.map t_optional ~f:(fun ty -> (true, false, false, ty))
    in
    let extra =
      match t_extra with
      | Tsplat t_splat -> [(false, false, true, t_splat)]
      | Tvariadic t_variadic ->
        if Typing_defs.is_nothing t_variadic then
          []
        else
          [(false, true, false, t_variadic)]
    in
    let required =
      List.map t_required ~f:(fun ty -> (false, false, false, ty))
    in
    list ~fuel "(" tuple_elem (required @ optional_list @ extra) ")"

  let tshape ~fuel k to_doc penv s is_open_mixed =
    let { s_origin = _; s_unknown_value = shape_kind; s_fields = fdm } = s in
    let open_mixed = is_open_mixed penv shape_kind in
    let (fuel, fields_doc) =
      let f_field (shape_map_key, { sft_optional; sft_ty }) ~fuel =
        let key_delim =
          match shape_map_key with
          | Typing_defs.TSFlit_str _ -> text "'"
          | _ -> Nothing
        in
        let (fuel, sft_ty_doc) = k ~fuel sft_ty in
        let field_doc =
          Concat
            [
              (if sft_optional then
                text "?"
              else
                Nothing);
              key_delim;
              to_doc (Typing_defs.TShapeField.name shape_map_key);
              key_delim;
              Space;
              text "=>";
              Space;
              sft_ty_doc;
            ]
        in
        (fuel, field_doc)
      in
      shape_map ~fuel fdm f_field
    in
    let (fuel, fields_doc) =
      begin
        if Typing_defs.is_nothing shape_kind then
          (fuel, fields_doc)
        else if open_mixed then
          (fuel, fields_doc @ [text "..."])
        else
          let (fuel, ty_doc) = k ~fuel shape_kind in
          ( fuel,
            fields_doc @ [Concat [text "_"; Space; text "=>"; Space; ty_doc]] )
      end
    in
    list ~fuel "shape(" id fields_doc ")"

  let refinement (type a) ~fuel k ({ cr_consts } : a class_refinement) =
    let f_rc ~fuel (name, (rc : a refined_const)) =
      let (fuel, rc_doc) =
        match rc.rc_bound with
        | TRexact ty ->
          let (fuel, ty_doc) = k ~fuel ty in
          (fuel, Concat [text "= "; ty_doc])
        | TRloose { tr_lower = ls; tr_upper = us } ->
          let bound kind (fuel, docs) ty =
            let (fuel, ty_doc) = k ~fuel ty in
            (fuel, (text (kind ^ " ") ^^ ty_doc) :: docs)
          in
          let (fuel, docs) = List.fold ~init:(fuel, []) ~f:(bound "super") ls in
          let (fuel, docs) = List.fold ~init:(fuel, docs) ~f:(bound "as") us in
          list_sep ~fuel Space id docs
      in
      (fuel, text (refined_const_kind_str rc ^ " " ^ name ^ " ") ^^ rc_doc)
    in
    delimited_list
      ~fuel
      ~skip_brackets_for_singleton:false
      semi_sep
      "{ "
      f_rc
      (SMap.bindings cr_consts)
      " }"

  let refinements ~fuel k e =
    match e with
    | Exact -> (fuel, Nothing)
    | Nonexact r ->
      if Class_refinement.is_empty r then
        (fuel, Nothing)
      else
        let (fuel, r_doc) = refinement ~fuel k r in
        (fuel, Concat [Space; text "with"; Space; r_doc])

  let thas_member ~fuel k hm =
    let { hm_name = (_, name); hm_type; hm_class_id = _; hm_method } = hm in
    (* TODO: T71614503 print explicit type arguments appropriately *)
    let printed_explicit_targs =
      match hm_method with
      | None -> text "None"
      | Some _ -> text "Some <targs>"
    in
    let (fuel, hm_ty_doc) = k ~fuel hm_type in
    let has_member_doc =
      Concat
        [
          text "has_member";
          text "(";
          text name;
          comma_sep;
          hm_ty_doc;
          comma_sep;
          printed_explicit_targs;
          text ")";
        ]
    in
    (fuel, has_member_doc)

  let tcan_index ~fuel k ci =
    let (fuel, key_doc) = k ~fuel ci.ci_key in
    let (fuel, val_doc) = k ~fuel ci.ci_val in
    ( fuel,
      Concat [text "can_index"; text "("; key_doc; comma_sep; val_doc; text ")"]
    )

  let tcan_index_assign ~fuel k cia =
    let (fuel, key_doc) = k ~fuel cia.cia_key in
    let (fuel, write_doc) = k ~fuel cia.cia_write in
    let (fuel, val_doc) = k ~fuel cia.cia_val in
    ( fuel,
      Concat
        [
          text "can_index_assign";
          text "(";
          key_doc;
          comma_sep;
          write_doc;
          comma_sep;
          val_doc;
          text ")";
        ] )

  let tcan_traverse ~fuel k ct =
    match ct.ct_key with
    | None ->
      let (fuel, val_doc) = k ~fuel ct.ct_val in
      ( fuel,
        Concat
          [
            text "can_traverse";
            text "(";
            val_doc;
            comma_sep;
            text "is_await:";
            text (string_of_bool ct.ct_is_await);
            text ")";
          ] )
    | Some ct_key ->
      let (fuel, key_doc) = k ~fuel ct_key in
      let (fuel, val_doc) = k ~fuel ct.ct_val in

      ( fuel,
        Concat
          [
            text "can_traverse";
            text "(";
            key_doc;
            comma_sep;
            val_doc;
            comma_sep;
            text "is_await:";
            text (string_of_bool ct.ct_is_await);
            text ")";
          ] )

  let thas_type_member ~fuel k htm =
    let { htm_id = id; htm_lower = lo; htm_upper = up } = htm in
    let subtype = Concat [Space; text "<:"; Space] in
    let (fuel, lo_doc) = k ~fuel lo in
    let (fuel, up_doc) = k ~fuel up in
    let has_type_member_doc =
      Concat
        [
          text "has_type_member(";
          lo_doc;
          subtype;
          text id;
          subtype;
          up_doc;
          text ")";
        ]
    in
    (fuel, has_type_member_doc)

  let tdestructure ~fuel (k : fuel:Fuel.t -> locl_ty -> Fuel.t * Doc.t) d =
    let { d_required; d_optional; d_variadic; d_kind } = d in
    let (fuel, e_required) =
      List.fold_map d_required ~f:(fun fuel ty -> k ~fuel ty) ~init:fuel
    in
    let (fuel, e_optional) =
      List.fold_map d_optional ~init:fuel ~f:(fun fuel v ->
          let (fuel, ty_doc) = k ~fuel v in
          let doc = Concat [text "=_"; ty_doc] in
          (fuel, doc))
    in
    let (fuel, e_variadic) =
      Option.value_map
        ~default:(fuel, [])
        ~f:(fun v ->
          let (fuel, ty_doc) = k ~fuel v in
          let doc = [Concat [text "..."; ty_doc]] in
          (fuel, doc))
        d_variadic
    in
    let prefix =
      match d_kind with
      | ListDestructure -> text "list"
      | SplatUnpack -> text "splat"
    in
    let (fuel, doc) =
      list ~fuel "(" id (e_required @ e_optional @ e_variadic) ")"
    in
    let doc = Concat [prefix; doc] in
    (fuel, doc)

  let rec is_open_mixed_decl penv t =
    match get_node t with
    | Tapply ((_, n), [ty])
      when String.equal n SN.Classes.cSupportDyn
           && not (show_supportdyn_penv penv) ->
      is_open_mixed_decl penv ty
    | Toption t ->
      (match get_node t with
      | Tnonnull -> true
      | _ -> false)
    | _ -> false

  let fun_implicit_params
      ty
      default_capability
      ~fuel
      to_doc
      st
      penv
      (tparams : 'a ty tparam list)
      ({ capability } : 'a ty fun_implicit_params) =
    let (fuel, capabilities) =
      match capability with
      | CapDefaults _ -> (fuel, None)
      | CapTy t ->
        if ty_equal ~normalize_lists:true t default_capability then
          (fuel, None)
        else (
          match get_node t with
          | Tintersection [] -> (fuel, Some [])
          | Toption t -> begin
            match deref t with
            | (_, Tnonnull) -> (fuel, Some [])
            | _ ->
              let (fuel, cap) = ty ~fuel to_doc st penv t in
              (fuel, Some [cap])
          end
          | _ ->
            let (fuel, cap) = ty ~fuel to_doc st penv t in
            (fuel, Some [cap])
        )
    in
    let (ctx_names, _) = split_desugared_ctx_tparams tparams in
    let ctx_names = List.map ctx_names ~f:(fun name -> text name) in

    match (capabilities, ctx_names) with
    | (None, []) -> (fuel, text ":")
    | (None, ctx_names) -> (fuel, Concat [text "["; Concat ctx_names; text "]:"])
    | (Some capabilities, ctx_names) ->
      (fuel, Concat [text "["; Concat (capabilities @ ctx_names); text "]:"])

  (* Prints a decl_ty. If there isn't enough fuel, the type is omitted. Each
     recursive call to print a type depletes the fuel by one. *)
  let rec decl_ty ~fuel :
      _ ->
      _ ->
      _ ->
      verbose_fun:bool ->
      omit_likes:bool ->
      decl_ty ->
      Fuel.t * Doc.t =
   fun to_doc st penv ~verbose_fun ~omit_likes x ->
    Fuel.provide
      fuel
      (decl_ty_ to_doc st penv ~verbose_fun ~omit_likes (get_node x))

  and decl_ty_ ~fuel :
      _ ->
      _ ->
      _ ->
      verbose_fun:bool ->
      omit_likes:bool ->
      decl_phase ty_ ->
      Fuel.t * Doc.t =
   fun to_doc st penv ~verbose_fun ~omit_likes x ->
    let ty = decl_ty ~verbose_fun ~omit_likes in
    let k ~fuel x = ty ~fuel to_doc st penv x in
    match x with
    | Tany _ -> (fuel, text "_")
    | Tthis -> (fuel, text SN.Typehints.this)
    | Tmixed -> (fuel, text "mixed")
    | Twildcard -> (fuel, text "_")
    | Tdynamic -> (fuel, text "dynamic")
    | Tnonnull -> (fuel, text "nonnull")
    | Tvec_or_dict (x, y) -> list ~fuel "vec_or_dict<" k [x; y] ">"
    | Tapply ((_, s), []) ->
      ( fuel,
        if String.equal s SN.Classes.cString then
          to_doc (strip_ns s)
        else
          to_doc s )
    | Tgeneric s -> (fuel, to_doc s)
    | Taccess (root_ty, id) ->
      let (fuel, root_ty_doc) = k ~fuel root_ty in
      let access_doc = Concat [root_ty_doc; text "::"; to_doc (snd id)] in
      (fuel, access_doc)
    | Trefinement (root_ty, r) ->
      let (fuel, root_ty_doc) = k ~fuel root_ty in
      let (fuel, rs_doc) = refinement ~fuel k r in
      (fuel, root_ty_doc ^^ text " with " ^^ rs_doc)
    | Toption x ->
      let (fuel, ty_doc) = k ~fuel x in
      let option_doc = Concat [text "?"; ty_doc] in
      (fuel, option_doc)
    | Tlike x ->
      let (fuel, ty_doc) = k ~fuel x in
      let like_doc =
        if omit_likes then
          ty_doc
        else
          Concat [text "~"; ty_doc]
      in
      (fuel, like_doc)
    | Tprim x -> (fuel, tprim x)
    | Tfun ft ->
      tfun
        ~fuel
        ~ty
        to_doc
        st
        penv
        ~verbose:verbose_fun
        ft
        (fun_decl_implicit_params ~verbose_fun ~omit_likes)
    | Tapply ((_, n), [ty])
      when String.equal n SN.Classes.cSupportDyn
           && not (show_supportdyn_penv penv) ->
      k ~fuel ty
    (* Don't strip_ns here! We want the FULL type, including the initial slash.
      *)
    | Tapply ((_, s), tyl) ->
      let (fuel, tys_doc) = list ~fuel "<" k tyl ">" in
      let generic_doc = to_doc s ^^ tys_doc in
      (fuel, generic_doc)
    | Ttuple { t_required; t_optional; t_extra } ->
      ttuple ~fuel k t_required t_optional t_extra
    | Tunion [] -> (fuel, text "nothing")
    | Tunion tyl ->
      let (fuel, tys_doc) =
        ttuple ~fuel k tyl [] (Tvariadic (Typing_make_type.nothing Reason.none))
      in
      let union_doc = Concat [text "|"; tys_doc] in
      (fuel, union_doc)
    | Tintersection [] -> (fuel, text "mixed")
    | Tintersection tyl ->
      delimited_list
        ~fuel
        ~skip_brackets_for_singleton:true
        (Space ^^ text "&" ^^ Space)
        "("
        k
        tyl
        ")"
    | Tshape s -> tshape ~fuel k to_doc penv s is_open_mixed_decl
    | Tclass_ptr x ->
      let (fuel, ty_doc) = k ~fuel x in
      let class_ptr_doc = Concat [text "class<"; ty_doc; text ">"] in
      (fuel, class_ptr_doc)

  and fun_decl_implicit_params ~fuel ~verbose_fun ~omit_likes =
    fun_implicit_params
      (decl_ty ~verbose_fun ~omit_likes)
      (Typing_make_type.default_capability_decl Pos_or_decl.none)
      ~fuel

  (* For a given type parameter, construct a list of its constraints *)
  let get_constraints_on_tparam (penv : Typing_env_types.env) tparam =
    let lower = Typing_env_types.get_lower_bounds penv tparam in
    let upper = Typing_env_types.get_upper_bounds penv tparam in
    let equ = Typing_env_types.get_equal_bounds penv tparam in
    let upper =
      if show_supportdyn penv then
        upper
      else
        (* Don't show "as mixed" if we're not printing supportdyn *)
        TySet.remove
          Typing_make_type.(supportdyn Reason.none (mixed Reason.none))
          upper
    in
    (* If we have an equality we can ignore the other bounds *)
    if not (TySet.is_empty equ) then
      List.map (TySet.elements equ) ~f:(fun ty ->
          (tparam, Ast_defs.Constraint_eq, ty))
    else
      List.map (TySet.elements lower) ~f:(fun ty ->
          (tparam, Ast_defs.Constraint_super, ty))
      @ List.map (TySet.elements upper) ~f:(fun ty ->
            (tparam, Ast_defs.Constraint_as, ty))

  let rec is_open_mixed env t =
    match get_node t with
    | Tnewtype (n, _, ty)
      when String.equal n SN.Classes.cSupportDyn && not (show_supportdyn env) ->
      is_open_mixed env ty
    | Toption t ->
      let (_, t) = Typing_inference_env.expand_type env.inference_env t in
      (match get_node t with
      | Tnonnull -> true
      | _ -> false)
    | _ -> false

  (* Prints a locl_ty. If there isn't enough fuel, the type is omitted. Each
     recursive call to print a type depletes the fuel by one. *)
  let rec locl_ty ~fuel ~hide_internals :
      _ -> _ -> _ -> locl_ty -> Fuel.t * Doc.t =
   fun to_doc st penv ty ->
    Fuel.provide fuel (fun ~fuel ->
        let (_r, x) = deref ty in
        locl_ty_ ~fuel ~hide_internals to_doc st penv x)

  and locl_ty_ ~fuel ~hide_internals :
      _ -> _ -> penv -> locl_phase ty_ -> Fuel.t * Doc.t =
   fun to_doc st penv x ->
    let ty = locl_ty in
    let verbose = show_verbose penv in
    let env =
      match penv with
      | Declenv -> failwith "must provide a locl-env here"
      | Loclenv env -> env
    in
    let k ~fuel x = ty ~fuel ~hide_internals to_doc st (Loclenv env) x in
    match x with
    | Tany _ -> (fuel, text "_")
    | Tdynamic -> (fuel, text "dynamic")
    | Tnonnull -> (fuel, text "nonnull")
    | Tvec_or_dict (x, y) -> list ~fuel "vec_or_dict<" k [x; y] ">"
    | Toption ty -> begin
      let (_, ty) = Typing_inference_env.expand_type env.inference_env ty in
      match deref ty with
      | (_, Tnonnull) -> (fuel, text "mixed")
      | (r, Tunion tyl) when List.exists ~f:is_dynamic tyl ->
        (* Unions with null become Toption, which leads to the awkward ?~...
         * The Tunion case can better handle this *)
        k ~fuel (mk (r, Tunion (mk (r, Tprim Nast.Tnull) :: tyl)))
      (* Drop the extra option *)
      | (_, Toption _) -> locl_ty ~fuel ~hide_internals to_doc st penv ty
      | _ ->
        let (fuel, d) = k ~fuel ty in
        (fuel, Concat [text "?"; d])
    end
    | Tprim x -> (fuel, tprim x)
    | Tneg predicate ->
      type_predicate ~fuel ~negate:true ~hide_internals to_doc st penv predicate
    | Tvar n ->
      let (_, ety) =
        Typing_inference_env.expand_type
          env.inference_env
          (mk (Reason.none, Tvar n))
      in
      begin
        match deref ety with
        (* For unsolved type variables, always show the type variable *)
        | (_, Tvar n') ->
          let tvar_doc =
            if Tvid.Set.mem n' st then
              text "[rec]"
            else if hide_internals then
              text "_"
            else
              text ("#" ^ Tvid.show n')
          in
          (fuel, tvar_doc)
        | _ ->
          let prepend =
            if Tvid.Set.mem n st then
              text "[rec]"
            else if
              (* For hh_show_env we further show the type variable number *)
              show_verbose penv
            then
              text ("#" ^ Tvid.show n)
            else
              Nothing
          in
          let st = Tvid.Set.add n st in
          let (fuel, ty_doc) = ty ~fuel ~hide_internals to_doc st penv ety in
          (fuel, Concat [prepend; ty_doc])
      end
    | Tfun ft ->
      tfun
        ~fuel
        ~ty:(ty ~hide_internals)
        to_doc
        st
        penv
        ~verbose:false
        ft
        (fun_locl_implicit_params ~hide_internals)
    | Tclass ((_, s), exact, tyl) ->
      let (fuel, targs_doc) =
        if List.is_empty tyl then
          (fuel, Nothing)
        else
          list ~fuel "<" k tyl ">"
      in
      let (fuel, with_doc) = refinements ~fuel k exact in
      let class_doc = to_doc (strip_ns s) ^^ targs_doc ^^ with_doc in
      let class_doc =
        match exact with
        | Exact when not hide_internals ->
          Concat [text "exact"; Space; class_doc]
        | _ -> class_doc
      in
      (fuel, class_doc)
    | Tgeneric s when SN.Coeffects.is_generated_generic s -> begin
      match String.get s 2 with
      | '[' ->
        (* has the form T/[...] *)
        (fuel, to_doc (String.sub s ~pos:3 ~len:(String.length s - 4)))
      | '$' -> begin
        (* Generic replacement type for parameter used for dependent context *)
        match get_constraints_on_tparam env s with
        | [(_, Ast_defs.Constraint_as, ty)] ->
          locl_ty ~fuel ~hide_internals to_doc st (Loclenv env) ty
        | _ -> (* this case shouldn't occur *) (fuel, to_doc s)
      end
      | _ -> (fuel, to_doc s)
    end
    | Tnewtype (s, [], _)
    | Tgeneric s ->
      (fuel, to_doc s)
    | Tnewtype (n, [ty], _)
      when String.equal n SN.Classes.cSupportDyn
           && not (show_supportdyn_penv penv) ->
      k ~fuel ty
    | Tnewtype (s, tyl, _) ->
      let (fuel, tys_doc) = list ~fuel "<" k tyl ">" in
      let generic_doc = to_doc s ^^ tys_doc in
      (fuel, generic_doc)
    | Tdependent (dep, cstr) ->
      let (fuel, cstr_doc) = k ~fuel cstr in
      let cstr_info = Concat [Space; text "as"; Space; cstr_doc] in
      let dependent_doc =
        Concat [to_doc @@ DependentKind.to_string dep; cstr_info]
      in
      (fuel, dependent_doc)
    (* Don't strip_ns here! We want the FULL type, including the initial slash.
      *)
    | Ttuple { t_required; t_optional; t_extra } ->
      ttuple ~fuel k t_required t_optional t_extra
    | Tunion [] -> (fuel, text "nothing")
    | Tunion tyl ->
      let tyl =
        List.fold_right tyl ~init:TySet.empty ~f:TySet.add |> TySet.elements
      in
      let (dynamic, null, nonnull) =
        List.partition3_map tyl ~f:(fun t ->
            match get_node t with
            | Tdynamic -> `Fst t
            | Tprim Nast.Tnull -> `Snd t
            | _ -> `Trd t)
      in
      begin
        match
          (not @@ List.is_empty dynamic, not @@ List.is_empty null, nonnull)
        with
        | (false, false, []) -> (fuel, text "nothing")
        (* type isn't nullable or dynamic *)
        | (false, false, [ty]) ->
          if verbose then
            let (fuel, ty_doc) = k ~fuel ty in
            let doc = Concat [text "("; ty_doc; text ")"] in
            (fuel, doc)
          else
            k ~fuel ty
        | (false, false, _ :: _) ->
          delimited_list
            ~fuel
            ~skip_brackets_for_singleton:true
            (Space ^^ text "|" ^^ Space)
            "("
            k
            nonnull
            ")"
        (* Type only is null *)
        | (false, true, []) ->
          let doc =
            if verbose then
              text "(null)"
            else
              text "null"
          in
          (fuel, doc)
        (* Type only is dynamic *)
        | (true, false, []) ->
          let doc =
            if verbose then
              text "(dynamic)"
            else
              text "dynamic"
          in
          (fuel, doc)
        (* Type is nullable single type *)
        | (false, true, [ty]) ->
          let (fuel, ty_doc) = k ~fuel ty in
          let doc =
            if verbose then
              Concat [text "(null |"; ty_doc; text ")"]
            else
              Concat [text "?"; ty_doc]
          in
          (fuel, doc)
        (* Type is like single type *)
        | (true, false, [ty]) ->
          let (fuel, ty_doc) = k ~fuel ty in
          let doc =
            if verbose then
              Concat [text "(dynamic |"; ty_doc; text ")"]
            else
              Concat [text "~"; ty_doc]
          in
          (fuel, doc)
        (* Type is like null *)
        | (true, true, []) ->
          let doc =
            if verbose then
              text "(dynamic | null)"
            else
              text "~null"
          in
          (fuel, doc)
        (* Type is like nullable single type *)
        | (true, true, [ty]) ->
          let (fuel, ty_doc) = k ~fuel ty in
          let doc =
            if verbose then
              Concat [text "(dynamic | null |"; ty_doc; text ")"]
            else
              Concat [text "~?"; ty_doc]
          in
          (fuel, doc)
        | (true, false, _ :: _) ->
          let (fuel, tys_doc) =
            delimited_list
              ~fuel
              ~skip_brackets_for_singleton:true
              (Space ^^ text "|" ^^ Space)
              "("
              k
              nonnull
              ")"
          in
          let doc = Concat [text "~"; tys_doc] in
          (fuel, doc)
        | (false, true, _ :: _) ->
          let (fuel, tys_doc) =
            delimited_list
              ~fuel
              ~skip_brackets_for_singleton:true
              (Space ^^ text "|" ^^ Space)
              "("
              k
              nonnull
              ")"
          in
          let doc = Concat [text "?"; tys_doc] in
          (fuel, doc)
        | (true, true, _ :: _) ->
          let (fuel, tys_doc) =
            delimited_list
              ~fuel
              ~skip_brackets_for_singleton:true
              (Space ^^ text "|" ^^ Space)
              "("
              k
              nonnull
              ")"
          in
          let doc = Concat [text "~"; text "?"; tys_doc] in
          (fuel, doc)
      end
    | Tintersection [] -> (fuel, text "mixed")
    | Tintersection tyl ->
      delimited_list
        ~fuel
        ~skip_brackets_for_singleton:true
        (Space ^^ text "&" ^^ Space)
        "("
        k
        tyl
        ")"
    | Tshape s -> tshape ~fuel k to_doc env s is_open_mixed
    | Taccess (root_ty, id) ->
      let (fuel, root_ty_doc) = k ~fuel root_ty in
      let access_doc = Concat [root_ty_doc; text "::"; to_doc (snd id)] in
      (fuel, access_doc)
    | Tlabel name ->
      let label_doc = Concat [text "#"; text name] in
      (fuel, label_doc)
    | Tclass_ptr ty ->
      let (fuel, ty_doc) = k ~fuel ty in
      (fuel, Concat [text "class<"; ty_doc; text ">"])

  and type_predicate ~fuel ~negate ~hide_internals to_doc st penv predicate =
    let k ~fuel lty = locl_ty ~fuel ~hide_internals to_doc st penv lty in
    let g ~fuel g =
      match g with
      | Wildcard id ->
        ( fuel,
          text
            ("_"
            ^
            if hide_internals then
              ""
            else
              string_of_int id) )
      | Filled ty -> k ~fuel ty
    in
    let tag_doc fuel tag =
      match tag with
      | BoolTag -> (fuel, text "bool")
      | IntTag -> (fuel, text "int")
      | ArraykeyTag -> (fuel, text "arraykey")
      | FloatTag -> (fuel, text "float")
      | NumTag -> (fuel, text "num")
      | ResourceTag -> (fuel, text "resource")
      | NullTag -> (fuel, text "null")
      | ClassTag (s, generics) ->
        let (fuel, targs_doc) =
          if List.is_empty generics then
            (fuel, Nothing)
          else
            list ~fuel "<" g generics ">"
        in
        (fuel, to_doc s ^^ targs_doc)
    in
    let rec predicate_doc fuel predicate =
      match snd predicate with
      | IsTag tag -> tag_doc fuel tag
      | IsTupleOf { tp_required } ->
        let (fuel, texts) =
          List.fold_map ~init:fuel tp_required ~f:predicate_doc
        in
        ( fuel,
          Concat
            ([text "("] @ List.intersperse texts ~sep:(text ", ") @ [text ")"])
        )
      | IsShapeOf { sp_fields; sp_allows_unknown_fields } ->
        let (fuel, texts) =
          List.fold_map
            ~init:fuel
            (TShapeMap.elements sp_fields)
            ~f:(fun fuel (key, { sfp_predicate; sfp_optional }) ->
              let key_delim =
                match key with
                | Typing_defs.TSFlit_str _ -> text "'"
                | _ -> Nothing
              in
              let (fuel, pdoc) = predicate_doc fuel sfp_predicate in
              ( fuel,
                Concat
                  [
                    (if sfp_optional then
                      text "?"
                    else
                      Nothing);
                    key_delim;
                    text_strip_ns @@ Typing_defs.TShapeField.name key;
                    key_delim;
                    Space;
                    text "=>";
                    Space;
                    pdoc;
                  ] ))
        in
        let texts =
          if sp_allows_unknown_fields then
            texts @ [text "..."]
          else
            texts
        in
        ( fuel,
          Concat
            ([text "shape("]
            @ List.intersperse texts ~sep:(text ", ")
            @ [text ")"]) )
      | IsUnionOf predicates ->
        let (fuel, texts) =
          List.fold_map ~init:fuel predicates ~f:predicate_doc
        in
        ( fuel,
          Concat (List.intersperse texts ~sep:(Concat [Space; text "|"; Space]))
        )
      (* TODO: T196048813 fuel? *)
    in
    let (fuel, pdoc) = predicate_doc fuel predicate in
    let doc =
      Concat
        [
          (text
          @@
          if negate then
            "not "
          else
            "is ");
          pdoc;
        ]
    in
    (fuel, doc)

  and fun_locl_implicit_params ~fuel ~hide_internals =
    fun_implicit_params
      (locl_ty ~hide_internals)
      (Typing_make_type.default_capability Pos_or_decl.none)
      ~fuel

  let rec constraint_type_ ~fuel ~hide_internals to_doc st penv x =
    let k ~fuel lty = locl_ty ~fuel ~hide_internals to_doc st penv lty in
    match x with
    | Thas_member hm -> thas_member ~fuel k hm
    | Thas_type_member htm -> thas_type_member ~fuel k htm
    | Tdestructure d -> tdestructure ~fuel k d
    | Tcan_index ci -> tcan_index ~fuel k ci
    | Tcan_index_assign cia -> tcan_index_assign ~fuel k cia
    | Tcan_traverse ct -> tcan_traverse ~fuel k ct
    | Ttype_switch { predicate; ty_true; ty_false } ->
      let (fuel, predicate_doc) =
        type_predicate
          ~fuel
          ~negate:false
          ~hide_internals
          to_doc
          st
          penv
          predicate
      in
      let (fuel, ty_true_doc) = k ~fuel ty_true in
      let (fuel, ty_false_doc) = k ~fuel ty_false in
      let ttype_switch_doc =
        Concat
          [
            text "(";
            predicate_doc;
            text "?";
            ty_true_doc;
            text ":";
            ty_false_doc;
            text ")";
          ]
      in
      (fuel, ttype_switch_doc)
    | Thas_const { name; ty } ->
      let (fuel, ty_doc) = k ~fuel ty in
      let has_const_doc =
        Concat [text "has_const("; text name; text ":"; Space; ty_doc; text ")"]
      in
      (fuel, has_const_doc)

  and constraint_type ~fuel ~hide_internals to_doc st penv ty =
    let (_r, x) = deref_constraint_type ty in
    constraint_type_ ~fuel ~hide_internals to_doc st penv x

  let internal_type ~fuel ~hide_internals to_doc st penv ty =
    match ty with
    | LoclType ty -> locl_ty ~fuel ~hide_internals to_doc st penv ty
    | ConstraintType ty ->
      constraint_type ~fuel ~hide_internals to_doc st penv ty

  let to_string ~fuel ~ty to_doc env x =
    let (fuel, doc) = ty ~fuel to_doc Tvid.Set.empty env x in
    let str = Libhackfmt.format_doc_unbroken format_env doc |> String.strip in
    (fuel, str)

  (** Print a suffix for type parameters in [typ] that have constraints
      If the type itself is a type parameter with a single constraint, just
      represent this as `as t` or `super t`, otherwise use full `where` syntax *)
  let constraints_for_type ~fuel ~hide_internals to_doc env typ =
    let tparams =
      SSet.elements
        (Typing_env_types.get_tparams_in_ty_and_acc env SSet.empty typ)
    in
    let constraints =
      List.concat_map tparams ~f:(get_constraints_on_tparam env)
    in
    let (_, typ) = Typing_inference_env.expand_type env.inference_env typ in
    let penv = Loclenv env in
    match (get_node typ, constraints) with
    | (_, []) -> (fuel, Nothing)
    | (Tgeneric tparam, [(tparam', ck, typ)]) when String.equal tparam tparam'
      ->
      tparam_constraint
        ~fuel
        ~ty:(locl_ty ~hide_internals)
        to_doc
        Tvid.Set.empty
        penv
        (ck, typ)
    | _ ->
      let to_tparam_constraint_doc ~fuel (tparam, ck, typ) =
        let (fuel, tparam_constraint_doc) =
          tparam_constraint
            ~fuel
            ~ty:(locl_ty ~hide_internals)
            to_doc
            Tvid.Set.empty
            penv
            (ck, typ)
        in
        let doc = Concat [text tparam; tparam_constraint_doc] in
        (fuel, doc)
      in
      let (fuel, tparam_constraints_doc) =
        list_sep ~fuel comma_sep to_tparam_constraint_doc constraints
      in
      let doc =
        Concat
          [
            Newline;
            text "where";
            Space;
            WithRule (Rule.Parental, tparam_constraints_doc);
          ]
      in
      (fuel, doc)

  let to_string_rec ~fuel ~hide_internals penv n x =
    let (fuel, doc) =
      locl_ty
        ~fuel
        ~hide_internals
        text_strip_ns
        (Tvid.Set.add n Tvid.Set.empty)
        penv
        x
    in
    let str = Libhackfmt.format_doc_unbroken format_env doc |> String.strip in
    (fuel, str)

  let to_string_strip_ns ~fuel ~ty env x =
    to_string ~fuel ~ty text_strip_ns env x

  let to_string_decl ~fuel (x : decl_ty) =
    let ty = decl_ty ~verbose_fun:false ~omit_likes:false in
    to_string ~fuel ~ty Doc.text Declenv x

  (** For functions and methods, interpret supportdyn as use of <<__SupportDynamicType>> attribute *)
  let add_supportdyn_prefix (type ph) env (x : ph ty) occurrence prefix :
      _ * ph ty =
    let add_prefix prefix =
      text "<<__SupportDynamicType>>" ^^ Newline ^^ prefix
    in
    match (occurrence, get_node x) with
    | ( SymbolOccurrence.{ type_ = Function | Method _; _ },
        Tnewtype (name, [tyarg], _) )
      when String.equal name SN.Classes.cSupportDyn ->
      if show_supportdyn env then
        (add_prefix prefix, tyarg)
      else
        (prefix, tyarg)
    | ( SymbolOccurrence.{ type_ = Function | Method _; _ },
        Tapply ((_, name), [tyarg]) )
      when String.equal name SN.Classes.cSupportDyn ->
      if show_supportdyn env then
        (add_prefix prefix, tyarg)
      else
        (prefix, tyarg)
    | (_, _) -> (prefix, x)

  let to_string_with_identity
      (type ph)
      ~fuel
      env
      (x : ph ty)
      (ty :
        fuel:Fuel.t ->
        (string -> t) ->
        Tvid.Set.t ->
        penv ->
        ph ty ->
        Fuel.t * t)
      ~default_capability
      ~constraints
      occurrence
      definition_opt =
    let open SymbolOccurrence in
    let penv = Loclenv env in
    let prefix =
      let print_mod m = text (SymbolDefinition.string_of_modifier m) ^^ Space in
      match (definition_opt, occurrence.SymbolOccurrence.type_) with
      | (None, _) -> Nothing
      | (_, XhpLiteralAttr _) -> Nothing
      | (Some def, _) -> begin
        match def.SymbolDefinition.modifiers with
        | [] -> Nothing
        (* It looks weird if we line break after a single modifier. *)
        | [m] -> print_mod m
        | ms -> Concat (List.map ms ~f:print_mod) ^^ SplitWith Cost.Base
      end
    in
    let (prefix, x) = add_supportdyn_prefix env x occurrence prefix in
    let (fuel, body_doc) =
      match (occurrence, get_node x) with
      | ({ type_ = Class class_type_id; name; _ }, _) ->
        let keyword =
          match
            Decl_provider.get_class
              ?tracing_info:env.tracing_info
              env.decl_env.Decl_env.ctx
              name
            [@alert "-dependencies"]
            (* effectively inlined the definition of Typing_env.get_class here
             * to avoid a circular dependency between build targets
             *)
          with
          | Decl_entry.Found decl ->
            (match Cls.kind decl with
            | Ast_defs.Cclass _ -> "class"
            | Ast_defs.Cinterface -> "interface"
            | Ast_defs.Ctrait -> "trait"
            | Ast_defs.Cenum -> "enum"
            | Ast_defs.Cenum_class _ -> "enum class")
          | Decl_entry.DoesNotExist
          | Decl_entry.NotYetAvailable ->
            "class"
        in
        (match class_type_id with
        | ExpressionTreeVisitor ->
          let (fuel, ty_doc) = ty ~fuel text_strip_ns Tvid.Set.empty penv x in
          ( fuel,
            Concat [text keyword; Space; text_strip_ns name; Newline; ty_doc] )
        | _ -> (fuel, Concat [text keyword; Space; text_strip_ns name]))
      | ({ type_ = Function; name; _ }, Tfun ft)
      | ({ type_ = Method (_, name); _ }, Tfun ft) ->
        (* Use short names for function types since they display a lot more
           information to the user. *)
        let (fuel, _, _async_doc, fun_ty_doc) =
          fun_type
            ~fuel
            ~ty
            text_strip_ns
            Tvid.Set.empty
            penv
            ~verbose:false
            ft
            (fun_implicit_params ty default_capability)
        in
        let fun_doc =
          Concat [text "function"; Space; text_strip_all_ns name; fun_ty_doc]
        in
        (fuel, fun_doc)
      | ({ type_ = Property (_, name); _ }, _) ->
        let (fuel, ty_doc) = ty ~fuel text_strip_ns Tvid.Set.empty penv x in
        let name =
          if String.is_prefix name ~prefix:"$" then
            (* Static property *)
            name
          else
            (* Instance property *)
            "$" ^ name
        in
        let doc = Concat [ty_doc; Space; Doc.text name] in
        (fuel, doc)
      | ({ type_ = XhpLiteralAttr _; name; _ }, _) ->
        let (fuel, ty_doc) = ty ~fuel text_strip_ns Tvid.Set.empty penv x in
        let doc =
          Concat
            [Doc.text "attribute"; Space; ty_doc; Space; text_strip_ns name]
        in
        (fuel, doc)
      | ({ type_ = ClassConst (_, name); _ }, _) ->
        let (fuel, ty_doc) = ty ~fuel text_strip_ns Tvid.Set.empty penv x in
        let doc =
          Concat [Doc.text "const"; Space; ty_doc; Space; text_strip_ns name]
        in
        (fuel, doc)
      | ({ type_ = GConst; name; _ }, _)
      | ({ type_ = EnumClassLabel _; name; _ }, _) ->
        let (fuel, ty_doc) = ty ~fuel text_strip_ns Tvid.Set.empty penv x in
        let doc = Concat [ty_doc; Space; text_strip_ns name] in
        (fuel, doc)
      | _ -> ty ~fuel text_strip_ns Tvid.Set.empty penv x
    in
    let str =
      Concat [prefix; body_doc; constraints]
      |> Libhackfmt.format_doc format_env
      |> String.strip
    in
    (fuel, str)

  let to_string_locl_with_identity
      ~fuel ~hide_internals env (x : locl_ty) occurrence definition_opt =
    let ty = locl_ty ~hide_internals in
    let (fuel, constraints) =
      constraints_for_type ~fuel ~hide_internals text_strip_ns env x
    in
    to_string_with_identity
      ~fuel
      env
      x
      ty
      ~default_capability:(Typing_make_type.default_capability Pos_or_decl.none)
      ~constraints
      occurrence
      definition_opt

  let to_string_decl_with_identity
      ~fuel ~verbose_fun ~omit_likes env (x : decl_ty) occurrence definition_opt
      =
    to_string_with_identity
      ~fuel
      env
      x
      (decl_ty ~verbose_fun ~omit_likes)
      ~default_capability:
        (Typing_make_type.default_capability_decl Pos_or_decl.none)
      ~constraints:Nothing
      occurrence
      definition_opt

  let to_string_with_identity
      ~fuel ~hide_internals env (x : locl_ty) occurrence definition_opt =
    to_string_locl_with_identity
      ~fuel
      ~hide_internals
      env
      x
      occurrence
      definition_opt
end

(*****************************************************************************)
(* Computes the string representing a type in an error message.              *)
(*****************************************************************************)

module ErrorString = struct
  let tprim = function
    | Nast.Tnull -> "null"
    | Nast.Tvoid -> "void"
    | Nast.Tint -> "an int"
    | Nast.Tbool -> "a bool"
    | Nast.Tfloat -> "a float"
    | Nast.Tnum -> "a num (int | float)"
    | Nast.Tresource -> "a resource"
    | Nast.Tarraykey -> "an array key (int | string)"
    | Nast.Tnoreturn -> "noreturn (throws or exits)"

  let rec type_ ~fuel ?(ignore_dynamic = false) env ety =
    let ety_to_string ety =
      Full.to_string_strip_ns
        ~fuel
        ~ty:(Full.locl_ty ~hide_internals:true)
        (Loclenv env)
        ety
    in
    match get_node ety with
    | Tany _ -> (fuel, "an untyped value")
    | Tdynamic -> (fuel, "a dynamic value")
    | Tunion l when ignore_dynamic ->
      union ~fuel env (List.filter l ~f:(fun x -> not (is_dynamic x)))
    | Tunion l -> union ~fuel env l
    | Tintersection [] -> (fuel, "a mixed value")
    | Tintersection l -> intersection ~fuel env l
    | Tvec_or_dict _ -> (fuel, "a vec_or_dict")
    | Ttuple { t_required; t_optional; t_extra = Tvariadic t_variadic } ->
      ( fuel,
        (if List.is_empty t_optional && is_nothing t_variadic then
          "a tuple of size "
        else
          "a tuple of size at least ")
        ^ string_of_int (List.length t_required) )
    | Ttuple { t_required; t_optional = _; t_extra = Tsplat _ } ->
      ( fuel,
        "a tuple of size at least " ^ string_of_int (List.length t_required) )
    | Tnonnull -> (fuel, "a nonnull value")
    | Toption x ->
      let str =
        match get_node x with
        | Tnonnull -> "a mixed value"
        | _ -> "a nullable type"
      in
      (fuel, str)
    | Tprim tp -> (fuel, tprim tp)
    | Tvar _ -> (fuel, "some value")
    | Tfun _ -> (fuel, "a function")
    | Tgeneric s when DependentKind.is_generic_dep_ty s ->
      let (fuel, ty_str) = ety_to_string ety in
      (fuel, "the expression dependent type " ^ ty_str)
    | Tgeneric _ ->
      let (fuel, ty_str) = ety_to_string ety in
      (fuel, "a value of generic type " ^ ty_str)
    | Tnewtype (n, [ty], _)
      when String.equal n SN.Classes.cSupportDyn && not (show_supportdyn env) ->
      type_ ~fuel env ty
    | Tnewtype (x, [ty], _) when String.equal x SN.Classes.cClassname ->
      let (fuel, ty_str) = ety_to_string ty in
      (fuel, "a classname string for " ^ ty_str)
    | Tclass_ptr ty ->
      let (fuel, ty_str) = ety_to_string ty in
      (fuel, "a class pointer for " ^ ty_str)
    | Tnewtype (x, [ty], _) when String.equal x SN.Classes.cTypename ->
      let (fuel, ty_str) = ety_to_string ty in
      (fuel, "a typename string for " ^ ty_str)
    | Tnewtype _ ->
      let (fuel, ty_str) = ety_to_string ety in
      (fuel, "a value of type " ^ ty_str)
    | Tdependent (dep, _cstr) -> (fuel, dependent dep)
    | Tclass ((_, id), _, _) when String.equal SN.Classes.cString id ->
      (fuel, "a string")
    | Tclass (_, e, _) ->
      let (fuel, ty_str) = ety_to_string ety in
      let prefix =
        match e with
        | Exact -> "an object of exactly the class "
        | Nonexact _ -> "an object of type "
      in
      (fuel, prefix ^ ty_str)
    | Tshape _ -> (fuel, "a shape")
    | Taccess (_ty, _id) -> (fuel, "a type constant")
    | Tlabel name -> (fuel, Printf.sprintf "a label (#%s)" name)
    | Tneg predicate ->
      let tag_str tag =
        match tag with
        | BoolTag -> "a bool"
        | IntTag -> "an int"
        | ArraykeyTag -> "an arraykey"
        | FloatTag -> "a float"
        | NumTag -> "a num"
        | ResourceTag -> "a resource"
        | NullTag -> "null"
        | ClassTag (s, _) -> "a " ^ strip_ns s
      in
      let rec str predicate =
        match predicate with
        | IsTag tag -> tag_str tag
        | IsTupleOf { tp_required } ->
          let strings = List.map tp_required ~f:(fun (_, pred) -> str pred) in
          "(" ^ String.concat ~sep:", " strings ^ ")"
        | IsShapeOf { sp_fields; sp_allows_unknown_fields } ->
          let texts =
            List.map
              (TShapeMap.elements sp_fields)
              ~f:(fun (key, { sfp_predicate; sfp_optional }) ->
                let key_delim =
                  match key with
                  | Typing_defs.TSFlit_str _ -> "'"
                  | _ -> ""
                in
                String.concat
                  ~sep:""
                  [
                    (if sfp_optional then
                      "?"
                    else
                      "");
                    key_delim;
                    Typing_defs.TShapeField.name key;
                    key_delim;
                    " => ";
                    str (snd sfp_predicate);
                  ])
          in
          let texts =
            if sp_allows_unknown_fields then
              texts @ ["..."]
            else
              texts
          in
          "shape(" ^ String.concat texts ~sep:", " ^ ")"
        | IsUnionOf predicates ->
          let strings = List.map predicates ~f:(fun (_, pred) -> str pred) in
          String.concat ~sep:" | " strings
        (* TODO: T196048813, dedupe?, fuel? *)
      in
      (fuel, "anything but " ^ str @@ snd predicate)

  and dependent dep =
    let x = strip_ns @@ DependentKind.to_string dep in
    match dep with
    | DTexpr _ -> "the expression dependent type " ^ x

  and union ~fuel env l =
    let (null, nonnull) =
      List.partition_tf l ~f:(fun ty ->
          equal_locl_ty_ (get_node ty) (Tprim Nast.Tnull))
    in
    let (fuel, l) =
      List.fold_map nonnull ~init:fuel ~f:(fun fuel -> to_string ~fuel env)
    in
    let s = List.fold_right l ~f:SSet.add ~init:SSet.empty in
    let l = SSet.elements s in
    let str =
      if List.is_empty null then
        union_ l
      else
        "a nullable type"
    in
    (fuel, str)

  and union_ = function
    | [] -> "an undefined value"
    | [x] -> x
    | x :: rl -> x ^ " or " ^ union_ rl

  and intersection ~fuel env l =
    let (fuel, l) =
      List.fold_map l ~init:fuel ~f:(fun fuel -> to_string ~fuel env)
    in
    let str = String.concat l ~sep:" and " in
    (fuel, str)

  and classish_kind c_kind final =
    let fs =
      if final then
        " final"
      else
        ""
    in
    match c_kind with
    | Ast_defs.Cclass k ->
      (match k with
      | Ast_defs.Abstract -> "an abstract" ^ fs ^ " class"
      | Ast_defs.Concrete -> "a" ^ fs ^ " class")
    | Ast_defs.Cinterface -> "an interface"
    | Ast_defs.Ctrait -> "a trait"
    | Ast_defs.Cenum -> "an enum"
    | Ast_defs.Cenum_class k ->
      (match k with
      | Ast_defs.Abstract -> "an abstract enum class"
      | Ast_defs.Concrete -> "an enum class")

  and to_string ~fuel ?(ignore_dynamic = false) env ty =
    let (_, ety) = Typing_inference_env.expand_type env.inference_env ty in
    type_ ~fuel ~ignore_dynamic env ety
end

(*****************************************************************************)
(* Prints the internal type of a class, this code is meant to be used for
 * debugging purposes only.
 *)
(*****************************************************************************)

let supply_fuel ?(msg = true) tcopt printer =
  let type_printer_fuel = TypecheckerOptions.type_printer_fuel tcopt in
  let fuel = Fuel.init type_printer_fuel in
  let (fuel, str) = printer ~fuel in
  if Fuel.has_enough fuel || not msg then
    str
  else
    Printf.sprintf
      "%s [This type was truncated. If you need more of the type, use `hh stop; hh --config type_printer_fuel=N`. At this time, N is %d.]"
      str
      type_printer_fuel

(*****************************************************************************)
(* User API *)
(*****************************************************************************)

let error ?(ignore_dynamic = false) env ty =
  supply_fuel env.genv.tcopt (ErrorString.to_string ~ignore_dynamic env ty)

let full ~hide_internals env ty =
  supply_fuel
    env.genv.tcopt
    (Full.to_string
       ~ty:(Full.locl_ty ~hide_internals)
       Doc.text
       (Loclenv env)
       ty)

let full_i ~hide_internals env ty =
  supply_fuel
    env.genv.tcopt
    (Full.to_string
       ~ty:(Full.internal_type ~hide_internals)
       Doc.text
       (Loclenv env)
       ty)

let full_rec ~hide_internals env n ty =
  supply_fuel
    env.genv.tcopt
    (Full.to_string_rec ~hide_internals (Loclenv env) n ty)

let full_strip_ns ~hide_internals env ty =
  supply_fuel
    env.genv.tcopt
    (Full.to_string_strip_ns
       ~ty:(Full.locl_ty ~hide_internals)
       (Loclenv env)
       ty)

let full_strip_ns_i ~hide_internals env ty =
  supply_fuel
    env.genv.tcopt
    (Full.to_string_strip_ns
       ~ty:(Full.internal_type ~hide_internals)
       (Loclenv env)
       ty)

let full_strip_ns_decl ?(msg = true) ~verbose_fun env ty =
  supply_fuel
    ~msg
    env.genv.tcopt
    (Full.to_string_strip_ns
       ~ty:(Full.decl_ty ~verbose_fun ~omit_likes:false)
       (Loclenv env)
       ty)

let full_with_identity ~hide_internals env x occurrence definition_opt =
  supply_fuel
    env.genv.tcopt
    (Full.to_string_with_identity
       ~hide_internals
       env
       x
       occurrence
       definition_opt)

let full_decl_with_identity
    env ~omit_likes ~verbose_fun x occurrence definition_opt =
  supply_fuel
    env.genv.tcopt
    (Full.to_string_decl_with_identity
       ~verbose_fun
       ~omit_likes
       env
       x
       occurrence
       definition_opt)

let full_decl ?(msg = true) tcopt ty =
  supply_fuel ~msg tcopt (Full.to_string_decl ty)

let debug env ty = full_strip_ns ~hide_internals:false env ty

let debug_decl env ty = full_strip_ns_decl ~verbose_fun:true env ty

let debug_i env ty = full_strip_ns_i ~hide_internals:false env ty

let constraints_for_type ~hide_internals env ty =
  supply_fuel env.genv.tcopt (fun ~fuel ->
      let (fuel, doc) =
        Full.constraints_for_type
          ~fuel
          ~hide_internals
          Full.text_strip_ns
          env
          ty
      in
      let str =
        Libhackfmt.format_doc_unbroken Full.format_env doc |> String.strip
      in
      (fuel, str))

let classish_kind c_kind final = ErrorString.classish_kind c_kind final

let subtype_prop env prop =
  let rec subtype_prop = function
    | Conj [] -> "TRUE"
    | Conj ps ->
      "(" ^ String.concat ~sep:" && " (List.map ~f:subtype_prop ps) ^ ")"
    | Disj (_, []) -> "FALSE"
    | Disj (_, ps) ->
      "(" ^ String.concat ~sep:" || " (List.map ~f:subtype_prop ps) ^ ")"
    | IsSubtype (false, ty1, ty2) -> debug_i env ty1 ^ " <: " ^ debug_i env ty2
    | IsSubtype (true, ty1, ty2) -> debug_i env ty1 ^ " <D: " ^ debug_i env ty2
  in
  let p_str = subtype_prop prop in
  p_str

let coeffects env ty =
  supply_fuel env.genv.tcopt @@ fun ~fuel ->
  let to_string ~fuel ty =
    Full.to_string
      ~fuel
      ~ty:(Full.locl_ty ~hide_internals:true)
      (fun s -> Doc.text (Utils.strip_all_ns s))
      (Loclenv env)
      ty
  in
  let exception UndesugarableCoeffect of locl_ty in
  let rec desugar_simple_intersection ~fuel (ty : locl_ty) :
      Fuel.t * string list =
    match snd @@ deref ty with
    | Tvar v ->
      (* We are interested in the upper bounds because coeffects are parameters (contravariant).
       * Similar to Typing_subtype.describe_ty_super, we ignore Tvars appearing in bounds *)
      let upper_bounds =
        ITySet.elements
          (Typing_inference_env.get_tyvar_upper_bounds env.inference_env v)
        |> List.filter_map ~f:(function
               | LoclType lty ->
                 (match deref lty with
                 | (_, Tvar _) -> None
                 | _ -> Some lty)
               | ConstraintType _ -> None)
      in
      let (fuel, tyll) =
        List.fold_map
          ~init:fuel
          ~f:(fun fuel -> desugar_simple_intersection ~fuel)
          upper_bounds
      in
      (fuel, List.concat tyll)
    | Tintersection tyl ->
      let (fuel, tyll) =
        List.fold_map
          ~init:fuel
          ~f:(fun fuel -> desugar_simple_intersection ~fuel)
          tyl
      in
      (fuel, List.concat tyll)
    | Tunion [ty] -> desugar_simple_intersection ~fuel ty
    | Tunion _
    | Tnonnull
    | Tdynamic ->
      raise (UndesugarableCoeffect ty)
    | Toption ty' -> begin
      match deref ty' with
      | (_, Tnonnull) -> (fuel, []) (* another special case of `mixed` *)
      | _ -> raise (UndesugarableCoeffect ty)
    end
    | _ ->
      let (fuel, str) = to_string ~fuel ty in
      (fuel, [str])
  in

  try
    let (inference_env, ty) =
      Typing_inference_env.expand_type env.inference_env ty
    in
    let ty =
      match deref ty with
      | (r, Tvar v) ->
        (* We are interested in the upper bounds because coeffects are parameters (contravariant).
         * Similar to Typing_subtype.describe_ty_super, we ignore Tvars appearing in bounds *)
        let upper_bounds =
          ITySet.elements
            (Typing_inference_env.get_tyvar_upper_bounds inference_env v)
          |> List.filter_map ~f:(function
                 | LoclType lty ->
                   (match deref lty with
                   | (_, Tvar _) -> None
                   | _ -> Some lty)
                 | ConstraintType _ -> None)
        in
        (match upper_bounds with
        | [] -> raise (UndesugarableCoeffect ty)
        | _ -> Typing_make_type.intersection r upper_bounds)
      | _ -> ty
    in
    let (fuel, tyl) = desugar_simple_intersection ~fuel ty in
    let coeffect_str =
      match tyl with
      | [cap] -> "the capability " ^ cap
      | caps ->
        "the capability set {"
        ^ (caps
          |> List.dedup_and_sort ~compare:String.compare
          |> String.concat ~sep:", ")
        ^ "}"
    in
    (fuel, coeffect_str)
  with
  | UndesugarableCoeffect _ -> to_string ~fuel ty
