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
open Option.Monad_infix
open Typing_defs
open Typing_env_types
open Typing_logic
module SN = Naming_special_names
module Reason = Typing_reason
module TySet = Typing_set
module Cls = Decl_provider.Class
module Nast = Aast
module ITySet = Internal_type_set

(* Fuel ensures that types are curtailed while printing them. This avoids
   performance regressions and increases readibility of errors overall. *)
module Fuel : sig
  type t

  val init : int -> t

  val deplete : t -> t

  val has_enough : t -> bool
end = struct
  type t = int

  let init fuel = fuel

  let deplete fuel = fuel - 1

  let has_enough fuel = fuel >= 0
end

(** For sake of typing_print, either we wish to print a locl_ty in which case we need
the env to look up the typing environment and constraints and the like, or a decl_ty
in which case we don't need anything. [penv] stands for "printing env". *)
type penv =
  | Loclenv of env
  | Declenv

let strip_ns id =
  id |> Utils.strip_ns |> Hh_autoimport.strip_HH_namespace_if_autoimport

let shallow_decl_enabled (ctx : Provider_context.t) : bool =
  TypecheckerOptions.shallow_class_decl (Provider_context.get_tcopt ctx)

(*****************************************************************************)
(* Pretty-printer of the "full" type.                                        *)
(* This is used in server/symbolTypeService and elsewhere                    *)
(* With debug_mode set it is used for hh_show_env                            *)
(*****************************************************************************)

module Full = struct
  open Doc

  let format_env = Format_env.{ default with line_width = 60 }

  let text_strip_ns s = Doc.text (strip_ns s)

  let ( ^^ ) a b = Concat [a; b]

  let debug_mode = ref false

  let show_verbose penv =
    match penv with
    | Loclenv env -> Typing_env_types.get_log_level env "show" > 1
    | Declenv -> false

  let blank_tyvars = ref false

  let comma_sep = Concat [text ","; Space]

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

  let delimited_list ~fuel sep left_delimiter f l right_delimiter :
      Fuel.t * Doc.t =
    let (fuel, doc) = list_sep ~fuel sep f l in
    let doc =
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
    delimited_list ~fuel comma_sep ld printer items rd

  let shape_map ~fuel fdm f_field =
    let compare (k1, _) (k2, _) =
      String.compare
        (Typing_defs.TShapeField.name k1)
        (Typing_defs.TShapeField.name k2)
    in
    let fields = List.sort ~compare (TShapeMap.bindings fdm) in
    List.fold_map fields ~f:f_field ~init:fuel

  let rec fun_type ~fuel ~ty to_doc st penv ft =
    let n = List.length ft.ft_params in
    let (fuel, params) =
      List.fold_mapi ft.ft_params ~init:fuel ~f:(fun i fuel p ->
          let (fuel, d) = fun_param ~fuel ~ty to_doc st penv p in
          ( fuel,
            if get_ft_variadic ft && i + 1 = n then
              Concat [d; text "..."]
            else
              d ))
    in
    let (fuel, tparams_doc) =
      (* only print tparams when they have been instantiated with targs
       * so that they correctly express reified parameterization *)
      match (ft.ft_tparams, get_ft_ftk ft) with
      | ([], _)
      | (_, FTKtparams) ->
        (fuel, Nothing)
      | (l, FTKinstantiated_targs) ->
        list ~fuel "<" (tparam ~ty to_doc st penv) l ">"
    in
    let (fuel, params_doc) = list ~fuel "(" id params "):" in
    let (fuel, return_doc) =
      possibly_enforced_ty ~fuel ~ty to_doc st penv ft.ft_ret
    in
    let tparams_doc =
      Span
        [
          tparams_doc;
          params_doc;
          Space;
          (if get_ft_returns_readonly ft then
            text "readonly" ^^ Space
          else
            Nothing);
          return_doc;
        ]
    in
    (fuel, tparams_doc)

  and possibly_enforced_ty ~fuel ~ty to_doc st penv { et_enforced; et_type } =
    let (fuel, d) = ty ~fuel to_doc st penv et_type in
    let d =
      Concat
        [
          (if show_verbose penv then
            match et_enforced with
            | Enforced -> text "enforced" ^^ Space
            | Unenforced -> Nothing
          else
            Nothing);
          d;
        ]
    in
    (fuel, d)

  and fun_param ~fuel ~ty to_doc st penv ({ fp_name; fp_type; _ } as fp) =
    let (fuel, d) = ty ~fuel to_doc st penv fp_type.et_type in
    let (fuel, d) =
      match (fp_name, d) with
      | (None, _) -> possibly_enforced_ty ~fuel ~ty to_doc st penv fp_type
      | (Some param_name, Text ("_", 1)) ->
        (* Handle the case of missing a type by not printing it *)
        (fuel, text param_name)
      | (Some param_name, _) ->
        let (fuel, d) = possibly_enforced_ty ~fuel ~ty to_doc st penv fp_type in
        let d = Concat [d; Space; text param_name] in
        (fuel, d)
    in
    let d =
      Concat
        [
          (match get_fp_mode fp with
          | FPinout -> text "inout" ^^ Space
          | _ -> Nothing);
          (match get_fp_readonly fp with
          | true -> text "readonly" ^^ Space
          | false -> Nothing);
          d;
          (if get_fp_has_default fp then
            text "=_"
          else
            Nothing);
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
    let (fuel, contraint_ty_doc) = ty ~fuel to_doc st penv cty in
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
          contraint_ty_doc;
        ]
    in
    (fuel, constraint_doc)

  let terr () =
    text
      (if !debug_mode then
        "err"
      else
        "_")

  let tprim x = text @@ Aast_defs.string_of_tprim x

  let tfun ~fuel ~ty to_doc st penv ft =
    let sdt =
      match penv with
      | Loclenv env when TypecheckerOptions.enable_sound_dynamic env.genv.tcopt
        ->
        if get_ft_support_dynamic_type ft then
          text "<<__SupportDynamicType>> "
        else
          Nothing
      | _ -> Nothing
    in
    let (fuel, fun_type_doc) = fun_type ~fuel ~ty to_doc st penv ft in
    let tfun_doc =
      Concat
        [
          text "(";
          (if get_ft_readonly_this ft then
            text "readonly "
          else
            Nothing);
          sdt;
          text "function";
          fun_type_doc;
          text ")";
        ]
    in
    (fuel, tfun_doc)

  let ttuple ~fuel k tyl = list ~fuel "(" k tyl ")"

  let tshape ~fuel k to_doc shape_kind fdm =
    let (fuel, fields_doc) =
      let f_field fuel (shape_map_key, { sft_optional; sft_ty }) =
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
    let fields_doc =
      match shape_kind with
      | Closed_shape -> fields_doc
      | Open_shape -> fields_doc @ [text "..."]
    in
    list ~fuel "shape(" id fields_doc ")"

  let thas_member ~fuel k hm =
    let { hm_name = (_, name); hm_type; hm_class_id = _; hm_explicit_targs } =
      hm
    in
    (* TODO: T71614503 print explicit type arguments appropriately *)
    let printed_explicit_targs =
      match hm_explicit_targs with
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

  (* Prints a decl_ty. If there isn't enough fuel, the type is omitted. Each
     recursive call to print a type depletes the fuel by one. *)
  let rec decl_ty ~fuel : _ -> _ -> _ -> decl_ty -> Fuel.t * Doc.t =
   fun to_doc st penv x ->
    if Fuel.has_enough fuel then
      let fuel = Fuel.deplete fuel in
      decl_ty_ ~fuel to_doc st penv (get_node x)
    else
      (fuel, text "[truncated]")

  and decl_ty_ ~fuel : _ -> _ -> _ -> decl_phase ty_ -> Fuel.t * Doc.t =
   fun to_doc st penv x ->
    let ty = decl_ty in
    let k ~fuel x = ty ~fuel to_doc st penv x in
    match x with
    | Tany _ -> (fuel, text "_")
    | Terr -> (fuel, terr ())
    | Tthis -> (fuel, text SN.Typehints.this)
    | Tmixed -> (fuel, text "mixed")
    | Tdynamic -> (fuel, text "dynamic")
    | Tnonnull -> (fuel, text "nonnull")
    | Tvec_or_dict (x, y) -> list ~fuel "vec_or_dict<" k [x; y] ">"
    | Tapply ((_, s), []) -> (fuel, to_doc s)
    | Tgeneric (s, []) -> (fuel, to_doc s)
    | Taccess (root_ty, id) ->
      let (fuel, root_ty_doc) = k ~fuel root_ty in
      let access_doc = Concat [root_ty_doc; text "::"; to_doc (snd id)] in
      (fuel, access_doc)
    | Toption x ->
      let (fuel, ty_doc) = k ~fuel x in
      let option_doc = Concat [text "?"; ty_doc] in
      (fuel, option_doc)
    | Tlike x ->
      let (fuel, ty_doc) = k ~fuel x in
      let like_doc = Concat [text "~"; ty_doc] in
      (fuel, like_doc)
    | Tprim x -> (fuel, tprim x)
    | Tvar x -> (fuel, text (Printf.sprintf "#%d" x))
    | Tfun ft -> tfun ~fuel ~ty to_doc st penv ft
    (* Don't strip_ns here! We want the FULL type, including the initial slash.
      *)
    | Tapply ((_, s), tyl)
    | Tgeneric (s, tyl) ->
      let (fuel, tys_doc) = list ~fuel "<" k tyl ">" in
      let generic_doc = to_doc s ^^ tys_doc in
      (fuel, generic_doc)
    | Ttuple tyl -> ttuple ~fuel k tyl
    | Tunion tyl ->
      let (fuel, tys_doc) = ttuple ~fuel k tyl in
      let union_doc = Concat [text "|"; tys_doc] in
      (fuel, union_doc)
    | Tintersection tyl ->
      let (fuel, tys_doc) = ttuple ~fuel k tyl in
      let intersection_doc = Concat [text "&"; tys_doc] in
      (fuel, intersection_doc)
    | Tshape (shape_kind, fdm) -> tshape ~fuel k to_doc shape_kind fdm

  (* For a given type parameter, construct a list of its constraints *)
  let get_constraints_on_tparam penv tparam =
    let kind_opt = Typing_env_types.get_pos_and_kind_of_generic penv tparam in
    match kind_opt with
    | None -> []
    | Some (_pos, kind) ->
      (* Use the names of the parameters themselves to present bounds
         depending on other parameters *)
      let param_names = Type_parameter_env.get_parameter_names kind in
      let params =
        List.map param_names ~f:(fun name ->
            Typing_make_type.generic Reason.none name)
      in
      let lower = Typing_env_types.get_lower_bounds penv tparam params in
      let upper = Typing_env_types.get_upper_bounds penv tparam params in
      let equ = Typing_env_types.get_equal_bounds penv tparam params in
      (* If we have an equality we can ignore the other bounds *)
      if not (TySet.is_empty equ) then
        List.map (TySet.elements equ) ~f:(fun ty ->
            (tparam, Ast_defs.Constraint_eq, ty))
      else
        List.map (TySet.elements lower) ~f:(fun ty ->
            (tparam, Ast_defs.Constraint_super, ty))
        @ List.map (TySet.elements upper) ~f:(fun ty ->
              (tparam, Ast_defs.Constraint_as, ty))

  (* Prints a locl_ty. If there isn't enough fuel, the type is omitted. Each
     recursive call to print a type depletes the fuel by one. *)
  let rec locl_ty ~fuel : _ -> _ -> _ -> locl_ty -> Fuel.t * Doc.t =
   fun to_doc st penv ty ->
    if Fuel.has_enough fuel then
      let fuel = Fuel.deplete fuel in
      let (r, x) = deref ty in
      let (fuel, d) = locl_ty_ ~fuel to_doc st penv x in
      let d =
        match r with
        | Typing_reason.Rsolve_fail _ -> Concat [text "{suggest:"; d; text "}"]
        | _ -> d
      in
      (fuel, d)
    else
      (fuel, text "[truncated]")

  and locl_ty_ ~fuel : _ -> _ -> _ -> locl_phase ty_ -> Fuel.t * Doc.t =
   fun to_doc st penv x ->
    let ty = locl_ty in
    let verbose = show_verbose penv in
    let env =
      match penv with
      | Declenv -> failwith "must provide a locl-env here"
      | Loclenv env -> env
    in
    let k ~fuel x = ty ~fuel to_doc st (Loclenv env) x in
    match x with
    | Tany _ -> (fuel, text "_")
    | Terr -> (fuel, terr ())
    | Tdynamic -> (fuel, text "dynamic")
    | Tnonnull -> (fuel, text "nonnull")
    | Tvec_or_dict (x, y) -> list ~fuel "vec_or_dict<" k [x; y] ">"
    | Tclass ((_, s), Exact, []) when !debug_mode ->
      (fuel, Concat [text "exact"; Space; to_doc s])
    | Tclass ((_, s), _, []) -> (fuel, to_doc s)
    | Toption ty ->
      begin
        match deref ty with
        | (_, Tnonnull) -> (fuel, text "mixed")
        | (r, Tunion tyl)
          when TypecheckerOptions.like_type_hints env.genv.tcopt
               && List.exists ~f:is_dynamic tyl ->
          (* Unions with null become Toption, which leads to the awkward ?~...
           * The Tunion case can better handle this *)
          k ~fuel (mk (r, Tunion (mk (r, Tprim Nast.Tnull) :: tyl)))
        | _ ->
          let (fuel, d) = k ~fuel ty in
          (fuel, Concat [text "?"; d])
      end
    | Tprim x -> (fuel, tprim x)
    | Tneg (Neg_prim x) -> (fuel, Concat [text "not "; tprim x])
    | Tneg (Neg_class c) -> (fuel, Concat [text "not "; to_doc (snd c)])
    | Tvar n ->
      let (_, ety) =
        Typing_inference_env.expand_type
          env.inference_env
          (mk (Reason.Rnone, Tvar n))
      in
      begin
        match deref ety with
        (* For unsolved type variables, always show the type variable *)
        | (_, Tvar n') ->
          let tvar_doc =
            if ISet.mem n' st then
              text "[rec]"
            else if !blank_tyvars then
              text "[unresolved]"
            else
              text ("#" ^ string_of_int n')
          in
          (fuel, tvar_doc)
        | _ ->
          let prepend =
            if ISet.mem n st then
              text "[rec]"
            else if
              (* For hh_show_env we further show the type variable number *)
              show_verbose penv
            then
              text ("#" ^ string_of_int n)
            else
              Nothing
          in
          let st = ISet.add n st in
          let (fuel, ty_doc) = ty ~fuel to_doc st penv ety in
          (fuel, Concat [prepend; ty_doc])
      end
    | Tfun ft -> tfun ~fuel ~ty to_doc st penv ft
    | Tclass ((_, s), exact, tyl) ->
      let (fuel, targs_doc) = list ~fuel "<" k tyl ">" in
      let class_doc = to_doc s ^^ targs_doc in
      let class_doc =
        match exact with
        | Exact when !debug_mode -> Concat [text "exact"; Space; class_doc]
        | _ -> class_doc
      in
      (fuel, class_doc)
    | Tgeneric (s, []) when SN.Coeffects.is_generated_generic s ->
      begin
        match String.get s 2 with
        | '[' ->
          (* has the form T/[...] *)
          (fuel, to_doc (String.sub s ~pos:3 ~len:(String.length s - 4)))
        | '$' ->
          (* Generic replacement type for parameter used for dependent context *)
          begin
            match get_constraints_on_tparam env s with
            | [(_, Ast_defs.Constraint_as, ty)] ->
              locl_ty ~fuel to_doc st (Loclenv env) ty
            | _ -> (* this case shouldn't occur *) (fuel, to_doc s)
          end
        | _ -> (fuel, to_doc s)
      end
    | Tunapplied_alias s
    | Tnewtype (s, [], _)
    | Tgeneric (s, []) ->
      (fuel, to_doc s)
    | Tnewtype (s, tyl, _)
    | Tgeneric (s, tyl) ->
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
    | Ttuple tyl -> ttuple ~fuel k tyl
    | Tunion [] -> (fuel, text "nothing")
    | Tunion tyl when TypecheckerOptions.like_type_hints env.genv.tcopt ->
      let tyl =
        List.fold_right tyl ~init:Typing_set.empty ~f:Typing_set.add
        |> Typing_set.elements
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
          delimited_list ~fuel (Space ^^ text "|" ^^ Space) "(" k nonnull ")"
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
            delimited_list ~fuel (Space ^^ text "|" ^^ Space) "(" k nonnull ")"
          in
          let doc = Concat [text "~"; tys_doc] in
          (fuel, doc)
        | (false, true, _ :: _) ->
          let (fuel, tys_doc) =
            delimited_list ~fuel (Space ^^ text "|" ^^ Space) "(" k nonnull ")"
          in
          let doc = Concat [text "?"; tys_doc] in
          (fuel, doc)
        | (true, true, _ :: _) ->
          let (fuel, tys_doc) =
            delimited_list ~fuel (Space ^^ text "|" ^^ Space) "(" k nonnull ")"
          in
          let doc = Concat [text "~"; text "?"; tys_doc] in
          (fuel, doc)
      end
    | Tunion tyl ->
      let tyl =
        List.fold_right tyl ~init:Typing_set.empty ~f:Typing_set.add
        |> Typing_set.elements
      in
      let (null, nonnull) =
        List.partition_tf tyl ~f:(fun ty ->
            equal_locl_ty_ (get_node ty) (Tprim Nast.Tnull))
      in
      begin
        match (null, nonnull) with
        (* type isn't nullable *)
        | ([], [ty]) ->
          let (fuel, ty_doc) = k ~fuel ty in
          let doc =
            if verbose then
              Concat [text "("; ty_doc; text ")"]
            else
              ty_doc
          in
          (fuel, doc)
        | ([], _) ->
          delimited_list ~fuel (Space ^^ text "|" ^^ Space) "(" k nonnull ")"
        (* Type only is null *)
        | (_, []) ->
          let doc =
            if verbose then
              text "(null)"
            else
              text "null"
          in
          (fuel, doc)
        (* Type is nullable single type *)
        | (_, [ty]) ->
          let (fuel, ty_doc) = k ~fuel ty in
          let doc =
            if verbose then
              Concat [text "(null |"; ty_doc; text ")"]
            else
              Concat [text "?"; ty_doc]
          in
          (fuel, doc)
        (* Type is nullable union type *)
        | (_, _) ->
          let (fuel, tys_doc) =
            delimited_list ~fuel (Space ^^ text "|" ^^ Space) "(" k nonnull ")"
          in
          let doc = Concat [text "?"; tys_doc] in
          (fuel, doc)
      end
    | Tintersection [] -> (fuel, text "mixed")
    | Tintersection tyl ->
      delimited_list ~fuel (Space ^^ text "&" ^^ Space) "(" k tyl ")"
    | Tshape (shape_kind, fdm) -> tshape ~fuel k to_doc shape_kind fdm
    | Taccess (root_ty, id) ->
      let (fuel, root_ty_doc) = k ~fuel root_ty in
      let access_doc = Concat [root_ty_doc; text "::"; to_doc (snd id)] in
      (fuel, access_doc)

  let rec constraint_type_ ~fuel to_doc st penv x =
    let k ~fuel lty = locl_ty ~fuel to_doc st penv lty in
    let k' ~fuel cty = constraint_type ~fuel to_doc st penv cty in
    match x with
    | Thas_member hm -> thas_member ~fuel k hm
    | Tdestructure d -> tdestructure ~fuel k d
    | TCunion (lty, cty) ->
      let (fuel, lty_doc) = k ~fuel lty in
      let (fuel, cty_doc) = k' ~fuel cty in
      let cunion_doc =
        Concat [text "("; lty_doc; text "|"; cty_doc; text ")"]
      in
      (fuel, cunion_doc)
    | TCintersection (lty, cty) ->
      let (fuel, lty_doc) = k ~fuel lty in
      let (fuel, cty_doc) = k' ~fuel cty in
      let cintersection_doc =
        Concat [text "("; lty_doc; text "&"; cty_doc; text ")"]
      in
      (fuel, cintersection_doc)

  and constraint_type ~fuel to_doc st penv ty =
    let (r, x) = deref_constraint_type ty in
    let (fuel, constraint_ty_doc) = constraint_type_ ~fuel to_doc st penv x in
    let constraint_ty_doc =
      match r with
      | Typing_reason.Rsolve_fail _ ->
        Concat [text "{suggest:"; constraint_ty_doc; text "}"]
      | _ -> constraint_ty_doc
    in
    (fuel, constraint_ty_doc)

  let internal_type ~fuel to_doc st penv ty =
    match ty with
    | LoclType ty -> locl_ty ~fuel to_doc st penv ty
    | ConstraintType ty -> constraint_type ~fuel to_doc st penv ty

  let to_string ~fuel ~ty to_doc env x =
    let (fuel, doc) = ty ~fuel to_doc ISet.empty env x in
    let str = Libhackfmt.format_doc_unbroken format_env doc |> String.strip in
    (fuel, str)

  (* Print a suffix for type parameters in typ that have constraints
   * If the type itself is a type parameter with a single constraint, just
   * represent this as `as t` or `super t`, otherwise use full `where` syntax
   *)
  let constraints_for_type ~fuel to_doc env typ =
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
    | (Tgeneric (tparam, []), [(tparam', ck, typ)])
      when String.equal tparam tparam' ->
      tparam_constraint ~fuel ~ty:locl_ty to_doc ISet.empty penv (ck, typ)
    | _ ->
      let to_tparam_constraint_doc ~fuel (tparam, ck, typ) =
        let (fuel, tparam_constraint_doc) =
          tparam_constraint ~fuel ~ty:locl_ty to_doc ISet.empty penv (ck, typ)
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

  let to_string_rec ~fuel penv n x =
    let (fuel, doc) = locl_ty ~fuel Doc.text (ISet.add n ISet.empty) penv x in
    let str = Libhackfmt.format_doc_unbroken format_env doc |> String.strip in
    (fuel, str)

  let to_string_strip_ns ~fuel ~ty env x =
    to_string ~fuel ~ty text_strip_ns env x

  let to_string_decl ~fuel (x : decl_ty) =
    let ty = decl_ty in
    to_string ~fuel ~ty Doc.text Declenv x

  let fun_to_string ~fuel (x : decl_fun_type) =
    let ty = decl_ty in
    let (fuel, doc) = fun_type ~fuel ~ty Doc.text ISet.empty Declenv x in
    let str = Libhackfmt.format_doc_unbroken format_env doc |> String.strip in
    (fuel, str)

  let to_string_with_identity ~fuel env x occurrence definition_opt =
    let ty = locl_ty in
    let penv = Loclenv env in
    let prefix =
      SymbolDefinition.(
        let print_mod m = text (string_of_modifier m) ^^ Space in
        match definition_opt with
        | None -> Nothing
        | Some def ->
          begin
            match def.modifiers with
            | [] -> Nothing
            (* It looks weird if we line break after a single modifier. *)
            | [m] -> print_mod m
            | ms -> Concat (List.map ms ~f:print_mod) ^^ SplitWith Cost.Base
          end)
    in
    let (fuel, body_doc) =
      SymbolOccurrence.(
        match (occurrence, get_node x) with
        | ({ type_ = Class _; name; _ }, _) ->
          (fuel, Concat [text "class"; Space; text_strip_ns name])
        | ({ type_ = Function; name; _ }, Tfun ft)
        | ({ type_ = Method (_, name); _ }, Tfun ft) ->
          (* Use short names for function types since they display a lot more
             information to the user. *)
          let (fuel, fun_ty_doc) =
            fun_type ~fuel ~ty text_strip_ns ISet.empty penv ft
          in
          let fun_doc =
            Concat [text "function"; Space; text_strip_ns name; fun_ty_doc]
          in
          (fuel, fun_doc)
        | ({ type_ = Property _; name; _ }, _)
        | ({ type_ = XhpLiteralAttr _; name; _ }, _)
        | ({ type_ = ClassConst _; name; _ }, _)
        | ({ type_ = GConst; name; _ }, _)
        | ({ type_ = EnumClassLabel _; name; _ }, _) ->
          let (fuel, ty_doc) = ty ~fuel text_strip_ns ISet.empty penv x in
          let doc = Concat [ty_doc; Space; text_strip_ns name] in
          (fuel, doc)
        | _ -> ty ~fuel text_strip_ns ISet.empty penv x)
    in
    let (fuel, constraints) = constraints_for_type ~fuel text_strip_ns env x in
    let str =
      Concat [prefix; body_doc; constraints]
      |> Libhackfmt.format_doc format_env
      |> String.strip
    in
    (fuel, str)
end

let with_blank_tyvars f =
  Full.blank_tyvars := true;
  let res = f () in
  Full.blank_tyvars := false;
  res

(*****************************************************************************)
(* Computes the string representing a type in an error message.
 *)
(*****************************************************************************)

module ErrorString = struct
  let tprim = function
    | Nast.Tnull -> "null"
    | Nast.Tvoid -> "void"
    | Nast.Tint -> "an int"
    | Nast.Tbool -> "a bool"
    | Nast.Tfloat -> "a float"
    | Nast.Tstring -> "a string"
    | Nast.Tnum -> "a num (int | float)"
    | Nast.Tresource -> "a resource"
    | Nast.Tarraykey -> "an array key (int | string)"
    | Nast.Tnoreturn -> "noreturn (throws or exits)"

  let rec type_ ~fuel ?(ignore_dynamic = false) env ty =
    match ty with
    | Tany _ -> (fuel, "an untyped value")
    | Terr -> (fuel, "a type error")
    | Tdynamic -> (fuel, "a dynamic value")
    | Tunion l when ignore_dynamic ->
      union ~fuel env (List.filter l ~f:(fun x -> not (is_dynamic x)))
    | Tunion l -> union ~fuel env l
    | Tintersection [] -> (fuel, "a mixed value")
    | Tintersection l -> intersection ~fuel env l
    | Tvec_or_dict _ -> (fuel, "a vec_or_dict")
    | Ttuple l -> (fuel, "a tuple of size " ^ string_of_int (List.length l))
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
    | Tgeneric (s, tyl) when DependentKind.is_generic_dep_ty s ->
      let (fuel, tyl_str) = inst ~fuel env tyl in
      (fuel, "the expression dependent type " ^ s ^ tyl_str)
    | Tgeneric (x, tyl) ->
      let (fuel, tyl_str) = inst ~fuel env tyl in
      (fuel, "a value of generic type " ^ x ^ tyl_str)
    | Tnewtype (x, _, _) when String.equal x SN.Classes.cClassname ->
      (fuel, "a classname string")
    | Tnewtype (x, _, _) when String.equal x SN.Classes.cTypename ->
      (fuel, "a typename string")
    | Tnewtype (x, tyl, _) ->
      let (fuel, tyl_str) = inst ~fuel env tyl in
      (fuel, "a value of type " ^ strip_ns x ^ tyl_str)
    | Tdependent (dep, _cstr) -> (fuel, dependent dep)
    | Tclass ((_, x), Exact, tyl) ->
      let (fuel, tyl_str) = inst ~fuel env tyl in
      (fuel, "an object of exactly the class " ^ strip_ns x ^ tyl_str)
    | Tclass ((_, x), Nonexact, tyl) ->
      let (fuel, tyl_str) = inst ~fuel env tyl in
      (fuel, "an object of type " ^ strip_ns x ^ tyl_str)
    | Tshape _ -> (fuel, "a shape")
    | Tunapplied_alias _ ->
      (* FIXME it seems like this function is only for
         fully-applied types? Tunapplied_alias should only appear
         in a type argument position then, which inst below
         prints with a different function (namely Full.locl_ty) *)
      failwith "Tunapplied_alias is not a type"
    | Taccess (_ty, _id) -> (fuel, "a type constant")
    | Tneg (Neg_prim p) -> (fuel, "anything but a " ^ tprim p)
    | Tneg (Neg_class (_, c)) -> (fuel, "anything but a " ^ strip_ns c)

  and inst ~fuel env tyl =
    if List.is_empty tyl then
      (fuel, "")
    else
      with_blank_tyvars (fun () ->
          let (fuel, arg_strs) =
            List.fold_map tyl ~init:fuel ~f:(fun fuel ->
                Full.to_string_strip_ns ~fuel ~ty:Full.locl_ty (Loclenv env))
          in
          let str = "<" ^ String.concat ~sep:", " arg_strs ^ ">" in
          (fuel, str))

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
    type_ ~fuel ~ignore_dynamic env (get_node ety)
end

module Json = struct
  open Hh_json

  let param_mode_to_string = function
    | FPnormal -> "normal"
    | FPinout -> "inout"

  let string_to_param_mode = function
    | "normal" -> Some FPnormal
    | "inout" -> Some FPinout
    | _ -> None

  let rec from_type : env -> locl_ty -> json =
   fun env ty ->
    (* Helpers to construct fields that appear in JSON rendering of type *)
    let kind p k = [("src_pos", Pos_or_decl.json p); ("kind", JSON_String k)] in
    let args tys = [("args", JSON_Array (List.map tys ~f:(from_type env)))] in
    let typ ty = [("type", from_type env ty)] in
    let result ty = [("result", from_type env ty)] in
    let obj x = JSON_Object x in
    let name x = [("name", JSON_String x)] in
    let optional x = [("optional", JSON_Bool x)] in
    let is_array x = [("is_array", JSON_Bool x)] in
    let make_field (k, v) =
      let shape_field_name_to_json shape_field =
        (* TODO: need to update userland tooling? *)
        match shape_field with
        | Typing_defs.TSFlit_int (_, s) -> Hh_json.JSON_Number s
        | Typing_defs.TSFlit_str (_, s) -> Hh_json.JSON_String s
        | Typing_defs.TSFclass_const ((_, s1), (_, s2)) ->
          Hh_json.JSON_Array [Hh_json.JSON_String s1; Hh_json.JSON_String s2]
      in
      obj
      @@ [("name", shape_field_name_to_json k)]
      @ optional v.sft_optional
      @ typ v.sft_ty
    in
    let fields fl = [("fields", JSON_Array (List.map fl ~f:make_field))] in
    let as_type ty = [("as", from_type env ty)] in
    match (get_pos ty, get_node ty) with
    | (_, Tvar n) ->
      let (_, ty) =
        Typing_inference_env.expand_type
          env.inference_env
          (mk (get_reason ty, Tvar n))
      in
      begin
        match (get_pos ty, get_node ty) with
        | (p, Tvar _) -> obj @@ kind p "var"
        | _ -> from_type env ty
      end
    | (p, Ttuple tys) -> obj @@ kind p "tuple" @ is_array false @ args tys
    | (p, Tany _)
    | (p, Terr) ->
      obj @@ kind p "any"
    | (p, Tnonnull) -> obj @@ kind p "nonnull"
    | (p, Tdynamic) -> obj @@ kind p "dynamic"
    | (p, Tgeneric (s, tyargs)) ->
      obj @@ kind p "generic" @ is_array true @ name s @ args tyargs
    | (p, Tunapplied_alias s) -> obj @@ kind p "unapplied_alias" @ name s
    | (p, Tnewtype (s, _, ty))
      when Decl_provider.get_class env.decl_env.Decl_env.ctx s
           >>| Cls.enum_type
           |> Option.is_some ->
      obj @@ kind p "enum" @ name s @ as_type ty
    | (p, Tnewtype (s, tys, ty)) ->
      obj @@ kind p "newtype" @ name s @ args tys @ as_type ty
    | (p, Tdependent (DTexpr _, ty)) ->
      obj
      @@ kind p "path"
      @ [("type", obj @@ kind (get_pos ty) "expr")]
      @ as_type ty
    | (p, Toption ty) ->
      begin
        match get_node ty with
        | Tnonnull -> obj @@ kind p "mixed"
        | _ -> obj @@ kind p "nullable" @ args [ty]
      end
    | (p, Tprim tp) ->
      obj @@ kind p "primitive" @ name (Aast_defs.string_of_tprim tp)
    | (p, Tneg (Neg_prim tp)) ->
      obj @@ kind p "negation" @ name (Aast_defs.string_of_tprim tp)
    | (p, Tneg (Neg_class (_, c))) -> obj @@ kind p "negation" @ name c
    | (p, Tclass ((_, cid), _, tys)) ->
      obj @@ kind p "class" @ name cid @ args tys
    | (p, Tshape (shape_kind, fl)) ->
      let fields_known =
        match shape_kind with
        | Closed_shape -> true
        | Open_shape -> false
      in
      obj
      @@ kind p "shape"
      @ is_array false
      @ [("fields_known", JSON_Bool fields_known)]
      @ fields (TShapeMap.bindings fl)
    | (p, Tunion []) -> obj @@ kind p "nothing"
    | (_, Tunion [ty]) -> from_type env ty
    | (p, Tunion tyl) -> obj @@ kind p "union" @ args tyl
    | (p, Tintersection []) -> obj @@ kind p "mixed"
    | (_, Tintersection [ty]) -> from_type env ty
    | (p, Tintersection tyl) -> obj @@ kind p "intersection" @ args tyl
    | (p, Tfun ft) ->
      let fun_kind p = kind p "function" in
      let callconv cc =
        [("callConvention", JSON_String (param_mode_to_string cc))]
      in
      let param fp =
        obj @@ callconv (get_fp_mode fp) @ typ fp.fp_type.et_type
      in
      let params fps = [("params", JSON_Array (List.map fps ~f:param))] in
      obj @@ fun_kind p @ params ft.ft_params @ result ft.ft_ret.et_type
    | (p, Tvec_or_dict (ty1, ty2)) ->
      obj @@ kind p "vec_or_dict" @ args [ty1; ty2]
    (* TODO akenn *)
    | (p, Taccess (ty, _id)) -> obj @@ kind p "type_constant" @ args [ty]

  type deserialized_result = (locl_ty, deserialization_error) result

  let wrap_json_accessor f x =
    match f x with
    | Ok value -> Ok value
    | Error access_failure ->
      Error
        (Deserialization_error
           (Hh_json.Access.access_failure_to_string access_failure))

  let get_string x = wrap_json_accessor (Hh_json.Access.get_string x)

  let get_bool x = wrap_json_accessor (Hh_json.Access.get_bool x)

  let get_array x = wrap_json_accessor (Hh_json.Access.get_array x)

  let get_val x = wrap_json_accessor (Hh_json.Access.get_val x)

  let get_obj x = wrap_json_accessor (Hh_json.Access.get_obj x)

  let deserialization_error ~message ~keytrace =
    Error
      (Deserialization_error
         (message ^ Hh_json.Access.keytrace_to_string keytrace))

  let not_supported ~message ~keytrace =
    Error (Not_supported (message ^ Hh_json.Access.keytrace_to_string keytrace))

  let wrong_phase ~message ~keytrace =
    Error (Wrong_phase (message ^ Hh_json.Access.keytrace_to_string keytrace))

  (* TODO(T36532263) add PU stuff in here *)
  let to_locl_ty
      ?(keytrace = []) (ctx : Provider_context.t) (json : Hh_json.json) :
      deserialized_result =
    let reason = Reason.none in
    let ty (ty : locl_phase ty_) : deserialized_result = Ok (mk (reason, ty)) in
    let rec aux (json : Hh_json.json) ~(keytrace : Hh_json.Access.keytrace) :
        deserialized_result =
      Result.Monad_infix.(
        get_string "kind" (json, keytrace) >>= fun (kind, kind_keytrace) ->
        match kind with
        | "this" ->
          not_supported ~message:"Cannot deserialize 'this' type." ~keytrace
        | "any" -> ty (Typing_defs.make_tany ())
        | "mixed" -> ty (Toption (mk (reason, Tnonnull)))
        | "nonnull" -> ty Tnonnull
        | "dynamic" -> ty Tdynamic
        | "generic" ->
          get_string "name" (json, keytrace) >>= fun (name, _name_keytrace) ->
          get_bool "is_array" (json, keytrace)
          >>= fun (is_array, _is_array_keytrace) ->
          get_array "args" (json, keytrace) >>= fun (args, args_keytrace) ->
          aux_args args ~keytrace:args_keytrace >>= fun args ->
          if is_array then
            ty (Tgeneric (name, args))
          else
            wrong_phase ~message:"Tgeneric is a decl-phase type." ~keytrace
        | "enum" ->
          get_string "name" (json, keytrace) >>= fun (name, _name_keytrace) ->
          aux_as json ~keytrace >>= fun as_ty -> ty (Tnewtype (name, [], as_ty))
        | "unapplied_alias" ->
          get_string "name" (json, keytrace) >>= fun (name, name_keytrace) ->
          begin
            match Decl_provider.get_typedef ctx name with
            | Some _typedef -> ty (Tunapplied_alias name)
            | None ->
              deserialization_error
                ~message:("Unknown type alias: " ^ name)
                ~keytrace:name_keytrace
          end
        | "newtype" ->
          get_string "name" (json, keytrace) >>= fun (name, name_keytrace) ->
          begin
            match Decl_provider.get_typedef ctx name with
            | Some _typedef ->
              (* We end up only needing the name of the typedef. *)
              Ok name
            | None ->
              if String.equal name "HackSuggest" then
                not_supported
                  ~message:"HackSuggest types for lambdas are not supported"
                  ~keytrace
              else
                deserialization_error
                  ~message:("Unknown newtype: " ^ name)
                  ~keytrace:name_keytrace
          end
          >>= fun typedef_name ->
          get_array "args" (json, keytrace) >>= fun (args, args_keytrace) ->
          aux_args args ~keytrace:args_keytrace >>= fun args ->
          aux_as json ~keytrace >>= fun as_ty ->
          ty (Tnewtype (typedef_name, args, as_ty))
        | "path" ->
          get_obj "type" (json, keytrace) >>= fun (type_json, type_keytrace) ->
          get_string "kind" (type_json, type_keytrace)
          >>= fun (path_kind, path_kind_keytrace) ->
          get_array "path" (json, keytrace) >>= fun (ids_array, ids_keytrace) ->
          let ids =
            map_array
              ids_array
              ~keytrace:ids_keytrace
              ~f:(fun id_str ~keytrace ->
                match id_str with
                | JSON_String id -> Ok id
                | _ ->
                  deserialization_error ~message:"Expected a string" ~keytrace)
          in
          ids >>= fun _ids ->
          begin
            match path_kind with
            | "expr" ->
              not_supported
                ~message:
                  "Cannot deserialize path-dependent type involving an expression"
                ~keytrace
            | "this" ->
              aux_as json ~keytrace >>= fun _as_ty -> ty (Tgeneric ("this", []))
            | path_kind ->
              deserialization_error
                ~message:("Unknown path kind: " ^ path_kind)
                ~keytrace:path_kind_keytrace
          end
        | "tuple" ->
          get_array "args" (json, keytrace) >>= fun (args, args_keytrace) ->
          aux_args args ~keytrace:args_keytrace >>= fun args -> ty (Ttuple args)
        | "nullable" ->
          get_array "args" (json, keytrace) >>= fun (args, keytrace) ->
          begin
            match args with
            | [nullable_ty] ->
              aux nullable_ty ~keytrace:("0" :: keytrace) >>= fun nullable_ty ->
              ty (Toption nullable_ty)
            | _ ->
              deserialization_error
                ~message:
                  (Printf.sprintf
                     "Unsupported number of args for nullable type: %d"
                     (List.length args))
                ~keytrace
          end
        | "primitive" ->
          get_string "name" (json, keytrace) >>= fun (name, keytrace) ->
          begin
            match name with
            | "void" -> Ok Nast.Tvoid
            | "int" -> Ok Nast.Tint
            | "bool" -> Ok Nast.Tbool
            | "float" -> Ok Nast.Tfloat
            | "string" -> Ok Nast.Tstring
            | "resource" -> Ok Nast.Tresource
            | "num" -> Ok Nast.Tnum
            | "arraykey" -> Ok Nast.Tarraykey
            | "noreturn" -> Ok Nast.Tnoreturn
            | _ ->
              deserialization_error
                ~message:("Unknown primitive type: " ^ name)
                ~keytrace
          end
          >>= fun prim_ty -> ty (Tprim prim_ty)
        | "class" ->
          get_string "name" (json, keytrace) >>= fun (name, _name_keytrace) ->
          let class_pos =
            match Decl_provider.get_class ctx name with
            | Some class_ty -> Cls.pos class_ty
            | None ->
              (* Class may not exist (such as in non-strict modes). *)
              Pos_or_decl.none
          in
          get_array "args" (json, keytrace) >>= fun (args, _args_keytrace) ->
          aux_args args ~keytrace >>= fun tyl ->
          (* NB: "class" could have come from either a `Tapply` or a `Tclass`. Right
           * now, we always return a `Tclass`. *)
          ty (Tclass ((class_pos, name), Nonexact, tyl))
        | "shape" ->
          get_array "fields" (json, keytrace)
          >>= fun (fields, fields_keytrace) ->
          get_bool "is_array" (json, keytrace)
          >>= fun (is_array, _is_array_keytrace) ->
          let unserialize_field field_json ~keytrace :
              ( Typing_defs.tshape_field_name
                * locl_phase Typing_defs.shape_field_type,
                deserialization_error )
              result =
            get_val "name" (field_json, keytrace)
            >>= fun (name, name_keytrace) ->
            (* We don't need position information for shape field names. They're
             * only used for error messages and the like. *)
            let dummy_pos = Pos_or_decl.none in
            begin
              match name with
              | Hh_json.JSON_Number name ->
                Ok (Typing_defs.TSFlit_int (dummy_pos, name))
              | Hh_json.JSON_String name ->
                Ok (Typing_defs.TSFlit_str (dummy_pos, name))
              | Hh_json.JSON_Array
                  [Hh_json.JSON_String name1; Hh_json.JSON_String name2] ->
                Ok
                  (Typing_defs.TSFclass_const
                     ((dummy_pos, name1), (dummy_pos, name2)))
              | _ ->
                deserialization_error
                  ~message:"Unexpected format for shape field name"
                  ~keytrace:name_keytrace
            end
            >>= fun shape_field_name ->
            (* Optional field may be absent for shape-like arrays. *)
            begin
              match get_val "optional" (field_json, keytrace) with
              | Ok _ ->
                get_bool "optional" (field_json, keytrace)
                >>| fun (optional, _optional_keytrace) -> optional
              | Error _ -> Ok false
            end
            >>= fun optional ->
            get_obj "type" (field_json, keytrace)
            >>= fun (shape_type, shape_type_keytrace) ->
            aux shape_type ~keytrace:shape_type_keytrace
            >>= fun shape_field_type ->
            let shape_field_type =
              { sft_optional = optional; sft_ty = shape_field_type }
            in
            Ok (shape_field_name, shape_field_type)
          in
          map_array fields ~keytrace:fields_keytrace ~f:unserialize_field
          >>= fun fields ->
          if is_array then
            (* We don't have enough information to perfectly reconstruct shape-like
             * arrays. We're missing the keys in the shape map of the shape fields. *)
            not_supported
              ~message:"Cannot deserialize shape-like array type"
              ~keytrace
          else
            get_bool "fields_known" (json, keytrace)
            >>= fun (fields_known, _fields_known_keytrace) ->
            let shape_kind =
              if fields_known then
                Closed_shape
              else
                Open_shape
            in
            let fields =
              List.fold fields ~init:TShapeMap.empty ~f:(fun shape_map (k, v) ->
                  TShapeMap.add k v shape_map)
            in
            ty (Tshape (shape_kind, fields))
        | "union" ->
          get_array "args" (json, keytrace) >>= fun (args, keytrace) ->
          aux_args args ~keytrace >>= fun tyl -> ty (Tunion tyl)
        | "intersection" ->
          get_array "args" (json, keytrace) >>= fun (args, keytrace) ->
          aux_args args ~keytrace >>= fun tyl -> ty (Tintersection tyl)
        | "function" ->
          get_array "params" (json, keytrace)
          >>= fun (params, params_keytrace) ->
          let params =
            map_array
              params
              ~keytrace:params_keytrace
              ~f:(fun param ~keytrace ->
                get_string "callConvention" (param, keytrace)
                >>= fun (callconv, callconv_keytrace) ->
                begin
                  match string_to_param_mode callconv with
                  | Some callconv -> Ok callconv
                  | None ->
                    deserialization_error
                      ~message:("Unknown calling convention: " ^ callconv)
                      ~keytrace:callconv_keytrace
                end
                >>= fun callconv ->
                get_obj "type" (param, keytrace)
                >>= fun (param_type, param_type_keytrace) ->
                aux param_type ~keytrace:param_type_keytrace
                >>= fun param_type ->
                Ok
                  {
                    fp_type = { et_type = param_type; et_enforced = Unenforced };
                    fp_flags =
                      make_fp_flags
                        ~mode:callconv
                        ~accept_disposable:false
                        ~has_default:false
                        ~ifc_external:false
                        ~ifc_can_call:false
                        ~readonly:false;
                    (* Dummy values: these aren't currently serialized. *)
                    fp_pos = Pos_or_decl.none;
                    fp_name = None;
                  })
          in
          params >>= fun ft_params ->
          get_obj "result" (json, keytrace) >>= fun (result, result_keytrace) ->
          aux result ~keytrace:result_keytrace >>= fun ft_ret ->
          ty
            (Tfun
               {
                 ft_params;
                 ft_implicit_params =
                   { capability = CapDefaults Pos_or_decl.none };
                 ft_ret = { et_type = ft_ret; et_enforced = Unenforced };
                 (* Dummy values: these aren't currently serialized. *)
                 ft_tparams = [];
                 ft_where_constraints = [];
                 ft_flags = 0;
                 ft_ifc_decl = default_ifc_fun_decl;
               })
        | _ ->
          deserialization_error
            ~message:
              (Printf.sprintf
                 "Unknown or unsupported kind '%s' to convert to locl phase"
                 kind)
            ~keytrace:kind_keytrace)
    and map_array :
        type a.
        Hh_json.json list ->
        f:
          (Hh_json.json ->
          keytrace:Hh_json.Access.keytrace ->
          (a, deserialization_error) result) ->
        keytrace:Hh_json.Access.keytrace ->
        (a list, deserialization_error) result =
     fun array ~f ~keytrace ->
      let array =
        List.mapi array ~f:(fun i elem ->
            f elem ~keytrace:(string_of_int i :: keytrace))
      in
      Result.all array
    and aux_args
        (args : Hh_json.json list) ~(keytrace : Hh_json.Access.keytrace) :
        (locl_ty list, deserialization_error) result =
      map_array args ~keytrace ~f:aux
    and aux_as (json : Hh_json.json) ~(keytrace : Hh_json.Access.keytrace) :
        (locl_ty, deserialization_error) result =
      Result.Monad_infix.(
        (* as-constraint is optional, check to see if it exists. *)
        match Hh_json.Access.get_obj "as" (json, keytrace) with
        | Ok (as_json, as_keytrace) ->
          aux as_json ~keytrace:as_keytrace >>= fun as_ty -> Ok as_ty
        | Error (Hh_json.Access.Missing_key_error _) ->
          Ok (mk (Reason.none, Toption (mk (Reason.none, Tnonnull))))
        | Error access_failure ->
          deserialization_error
            ~message:
              ("Invalid as-constraint: "
              ^ Hh_json.Access.access_failure_to_string access_failure)
            ~keytrace)
    in
    aux json ~keytrace
end

let to_json = Json.from_type

let json_to_locl_ty = Json.to_locl_ty

(*****************************************************************************)
(* Prints the internal type of a class, this code is meant to be used for
 * debugging purposes only.
 *)
(*****************************************************************************)

let deferred_member_inits_ref :
    (Typing_env_types.env -> Shallow_decl_defs.shallow_class -> SSet.t * SSet.t)
    ref =
  ref (fun _ _ -> failwith "deferred_member_inits_ref not initialized!")

let set_deferred_member_inits f = deferred_member_inits_ref := f

let deferred_member_inits x y = !deferred_member_inits_ref x y

module PrintClass = struct
  let indent = "    "

  let bool = string_of_bool

  let sset s =
    let contents = SSet.fold (fun x acc -> x ^ " " ^ acc) s "" in
    Printf.sprintf "Set( %s)" contents

  let pos_or_decl p =
    let (line, start, end_) = Pos_or_decl.line_start_end_columns p in
    Printf.sprintf "(line %d: chars %d-%d)" line start end_

  let classish_kind = function
    | Ast_defs.Cclass k ->
      (match k with
      | Ast_defs.Abstract -> "Cabstract"
      | Ast_defs.Concrete -> "Cnormal")
    | Ast_defs.Cinterface -> "Cinterface"
    | Ast_defs.Ctrait -> "Ctrait"
    | Ast_defs.Cenum -> "Cenum"
    (* TODO: do we need to distinguish ? *)
    | Ast_defs.Cenum_class _ -> "Cenum_class"

  let constraint_ty ~fuel = function
    | (Ast_defs.Constraint_as, ty) ->
      let (fuel, ty_str) = Full.to_string_decl ~fuel ty in
      (fuel, "as " ^ ty_str)
    | (Ast_defs.Constraint_eq, ty) ->
      let (fuel, ty_str) = Full.to_string_decl ~fuel ty in
      (fuel, "= " ^ ty_str)
    | (Ast_defs.Constraint_super, ty) ->
      let (fuel, ty_str) = Full.to_string_decl ~fuel ty in
      (fuel, "super " ^ ty_str)

  let variance = function
    | Ast_defs.Covariant -> "+"
    | Ast_defs.Contravariant -> "-"
    | Ast_defs.Invariant -> ""

  let rec tparam
      ~fuel
      {
        tp_variance = var;
        tp_name = (position, name);
        tp_tparams = params;
        tp_constraints = cstrl;
        tp_reified = reified;
        tp_user_attributes = _;
      } =
    let (fuel, params_string) =
      if List.is_empty params then
        (fuel, "")
      else
        let (fuel, tparam_list_str) = tparam_list ~fuel params in
        (fuel, "<" ^ tparam_list_str ^ ">")
    in
    let (fuel, constraints_str) =
      let handle_constraint x (fuel, acc) =
        let (fuel, constraint_str) = constraint_ty ~fuel x in
        (fuel, constraint_str ^ " " ^ acc)
      in
      List.fold_right cstrl ~f:handle_constraint ~init:(fuel, "")
    in
    let tparam_str =
      variance var
      ^ pos_or_decl position
      ^ " "
      ^ name
      ^ params_string
      ^ " "
      ^ constraints_str
      ^
      match reified with
      | Nast.Erased -> ""
      | Nast.SoftReified -> " soft reified"
      | Nast.Reified -> " reified"
    in
    (fuel, tparam_str)

  and tparam_list ~fuel l =
    let handle_tparam x (fuel, acc) =
      let (fuel, tparam_str) = tparam ~fuel x in
      (fuel, tparam_str ^ ", " ^ acc)
    in
    let (fuel, str) = List.fold_right l ~f:handle_tparam ~init:(fuel, "") in
    (fuel, str)

  let class_elt ~fuel ({ ce_visibility; ce_type = (lazy ty); _ } as ce) =
    let vis =
      match ce_visibility with
      | Vpublic -> "public"
      | Vprivate _ -> "private"
      | Vprotected _ -> "protected"
      | Vinternal _ -> "internal"
    in
    let synth =
      if get_ce_synthesized ce then
        "synthetic "
      else
        ""
    in
    let (fuel, type_str) = Full.to_string_decl ~fuel ty in
    (fuel, synth ^ vis ^ " " ^ type_str)

  let class_elts ~fuel m =
    let handle_class_elt (fuel, acc) (field, v) =
      let (fuel, class_elt_str) = class_elt ~fuel v in
      let class_elt_str = "(" ^ field ^ ": " ^ class_elt_str ^ ") " ^ acc in
      (fuel, class_elt_str)
    in
    let (fuel, class_elts_str) =
      List.fold m ~init:(fuel, "") ~f:handle_class_elt
    in
    (fuel, class_elts_str)

  let class_elts_with_breaks ~fuel m =
    let handle_class_elt (fuel, acc) (field, v) =
      let (fuel, class_elt_str) = class_elt ~fuel v in
      let class_elt_str = "\n" ^ indent ^ field ^ ": " ^ class_elt_str ^ acc in
      (fuel, class_elt_str)
    in
    let (fuel, elts_str) = List.fold m ~init:(fuel, "") ~f:handle_class_elt in
    (fuel, elts_str)

  let class_consts ~fuel m =
    let handle_class_const (fuel, acc) (field, cc) =
      let synth =
        if cc.cc_synthesized then
          "synthetic "
        else
          ""
      in
      let (fuel, class_const_ty_str) = Full.to_string_decl ~fuel cc.cc_type in
      let class_const_ty_str =
        "(" ^ field ^ ": " ^ synth ^ class_const_ty_str ^ ") " ^ acc
      in
      (fuel, class_const_ty_str)
    in
    let (fuel, class_consts_str) =
      List.fold m ~init:(fuel, "") ~f:handle_class_const
    in
    (fuel, class_consts_str)

  let typeconst
      ~fuel
      {
        ttc_synthesized = synthetic;
        ttc_name = tc_name;
        ttc_kind = kind;
        ttc_origin = origin;
        ttc_enforceable = (_, enforceable);
        ttc_reifiable = reifiable;
        ttc_concretized = _;
        ttc_is_ctx = _;
      } =
    let name = snd tc_name in
    let ty ~fuel x = Full.to_string_decl ~fuel x in
    let (fuel, type_info) =
      match kind with
      | TCConcrete { tc_type = t } ->
        let (fuel, ty_str) = ty ~fuel t in
        let type_info_str = Printf.sprintf " = %s" ty_str in
        (fuel, type_info_str)
      | TCAbstract
          { atc_as_constraint = a; atc_super_constraint = s; atc_default = d }
        ->
        let m ~fuel printer =
          Option.value_map ~default:(fuel, "") ~f:(fun x ->
              let (fuel, ty_str) = ty ~fuel x in
              (fuel, printer ty_str))
        in
        let (fuel, a) = m ~fuel (Printf.sprintf " as %s") a in
        let (fuel, s) = m ~fuel (Printf.sprintf " super %s") s in
        let (fuel, d) = m ~fuel (Printf.sprintf " = %s") d in
        (fuel, a ^ s ^ d)
    in
    let typeconst_str =
      name
      ^ type_info
      ^ " (origin:"
      ^ origin
      ^ ")"
      ^ (if synthetic then
          " (synthetic)"
        else
          "")
      ^ (if enforceable then
          " (enforceable)"
        else
          "")
      ^
      if Option.is_some reifiable then
        " (reifiable)"
      else
        ""
    in
    (fuel, typeconst_str)

  let typeconsts ~fuel m =
    let handle_typeconst (fuel, acc) (_, v) =
      let (fuel, typeconst_str) = typeconst ~fuel v in
      let typeconst_str = "\n(" ^ typeconst_str ^ ")" ^ acc in
      (fuel, typeconst_str)
    in
    let (fuel, typeconsts_str) =
      List.fold m ~init:(fuel, "") ~f:handle_typeconst
    in
    (fuel, typeconsts_str)

  let ancestors ~fuel ctx m =
    (* Format is as follows:
     *    ParentKnownToHack
     *  ! ParentCompletelyUnknown
     *)
    let handle_ancestor (fuel, acc) (field, v) =
      let (sigil, kind) =
        match Decl_provider.get_class ctx field with
        | None -> ("!", "")
        | Some cls -> (" ", " (" ^ classish_kind (Cls.kind cls) ^ ")")
      in
      let (fuel, ty_str) = Full.to_string_decl ~fuel v in
      (fuel, "\n" ^ indent ^ sigil ^ " " ^ ty_str ^ kind ^ acc)
    in
    let (fuel, str) = List.fold m ~init:(fuel, "") ~f:handle_ancestor in
    (fuel, str)

  let constructor ~fuel (ce_opt, (consist : consistent_kind)) =
    let consist_str = Format.asprintf "(%a)" pp_consistent_kind consist in
    let (fuel, ce_str) =
      match ce_opt with
      | None -> (fuel, "")
      | Some ce -> class_elt ~fuel ce
    in
    (fuel, ce_str ^ consist_str)

  let req_ancestors ~fuel xs =
    let handle_req_ancestor (fuel, acc) (_p, x) =
      let (fuel, req_ancestor_str) = Full.to_string_decl ~fuel x in
      let req_ancestor_str = acc ^ req_ancestor_str ^ ", " in
      (fuel, req_ancestor_str)
    in
    let (fuel, str) = List.fold xs ~init:(fuel, "") ~f:handle_req_ancestor in
    (fuel, str)

  let class_type ~fuel ctx c =
    let tenv = Typing_env_types.empty ctx Relative_path.default ~droot:None in
    let tc_need_init = bool (Cls.need_init c) in
    let tc_abstract = bool (Cls.abstract c) in
    let tc_deferred_init_members =
      sset
      @@
      if shallow_decl_enabled ctx then
        match Shallow_classes_provider.get ctx (Cls.name c) with
        | Some cls -> snd (deferred_member_inits tenv cls)
        | None -> SSet.empty
      else
        Cls.deferred_init_members c
    in
    let tc_kind = classish_kind (Cls.kind c) in
    let tc_name = Cls.name c in
    let (fuel, tc_tparams) = tparam_list ~fuel (Cls.tparams c) in
    let (fuel, tc_consts) = class_consts ~fuel (Cls.consts c) in
    let (fuel, tc_typeconsts) = typeconsts ~fuel (Cls.typeconsts c) in
    let (fuel, tc_props) = class_elts ~fuel (Cls.props c) in
    let (fuel, tc_sprops) = class_elts ~fuel (Cls.sprops c) in
    let (fuel, tc_methods) = class_elts_with_breaks ~fuel (Cls.methods c) in
    let (fuel, tc_smethods) = class_elts_with_breaks ~fuel (Cls.smethods c) in
    let (fuel, tc_construct) = constructor ~fuel (Cls.construct c) in
    let (fuel, tc_ancestors) = ancestors ~fuel ctx (Cls.all_ancestors c) in
    let (fuel, tc_req_ancestors) =
      req_ancestors ~fuel (Cls.all_ancestor_reqs c)
    in
    let tc_req_ancestors_extends =
      String.concat ~sep:" " (Cls.all_ancestor_req_names c)
    in
    let class_ty_str =
      "tc_need_init: "
      ^ tc_need_init
      ^ "\n"
      ^ "tc_abstract: "
      ^ tc_abstract
      ^ "\n"
      ^ "tc_deferred_init_members: "
      ^ tc_deferred_init_members
      ^ "\n"
      ^ "tc_kind: "
      ^ tc_kind
      ^ "\n"
      ^ "tc_name: "
      ^ tc_name
      ^ "\n"
      ^ "tc_tparams: "
      ^ tc_tparams
      ^ "\n"
      ^ "tc_consts: "
      ^ tc_consts
      ^ "\n"
      ^ "tc_typeconsts: "
      ^ tc_typeconsts
      ^ "\n"
      ^ "tc_props: "
      ^ tc_props
      ^ "\n"
      ^ "tc_sprops: "
      ^ tc_sprops
      ^ "\n"
      ^ "tc_methods: "
      ^ tc_methods
      ^ "\n"
      ^ "tc_smethods: "
      ^ tc_smethods
      ^ "\n"
      ^ "tc_construct: "
      ^ tc_construct
      ^ "\n"
      ^ "tc_ancestors: "
      ^ tc_ancestors
      ^ "\n"
      ^ "tc_req_ancestors: "
      ^ tc_req_ancestors
      ^ "\n"
      ^ "tc_req_ancestors_extends: "
      ^ tc_req_ancestors_extends
      ^ "\n"
      ^ ""
    in
    (fuel, class_ty_str)
end

module PrintTypedef = struct
  let typedef ~fuel = function
    | {
        td_pos;
        td_module = _;
        td_vis = _;
        td_attributes = _;
        td_tparams;
        td_constraint;
        td_type;
        td_is_ctx;
      } ->
      let (fuel, tparaml_s) = PrintClass.tparam_list ~fuel td_tparams in
      let (fuel, constr_s) =
        match td_constraint with
        | None -> (fuel, "[None]")
        | Some constr -> Full.to_string_decl ~fuel constr
      in
      let (fuel, ty_s) = Full.to_string_decl ~fuel td_type in
      let pos_s = PrintClass.pos_or_decl td_pos in
      let is_ctx_s = Bool.to_string td_is_ctx in
      let typedef_str =
        "ty: "
        ^ ty_s
        ^ "\n"
        ^ "tparaml: "
        ^ tparaml_s
        ^ "\n"
        ^ "constraint: "
        ^ constr_s
        ^ "\n"
        ^ "pos: "
        ^ pos_s
        ^ "\n"
        ^ "is_ctx: "
        ^ is_ctx_s
        ^ "\n"
        ^ ""
      in
      (fuel, typedef_str)
end

let supply_fuel tcopt printer =
  let type_printer_fuel = TypecheckerOptions.type_printer_fuel tcopt in
  let fuel = Fuel.init type_printer_fuel in
  let (fuel, str) = printer ~fuel in
  if Fuel.has_enough fuel then
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

let full env ty =
  supply_fuel
    env.genv.tcopt
    (Full.to_string ~ty:Full.locl_ty Doc.text (Loclenv env) ty)

let full_i env ty =
  supply_fuel
    env.genv.tcopt
    (Full.to_string ~ty:Full.internal_type Doc.text (Loclenv env) ty)

let full_rec env n ty =
  supply_fuel env.genv.tcopt (Full.to_string_rec (Loclenv env) n ty)

let full_strip_ns env ty =
  supply_fuel
    env.genv.tcopt
    (Full.to_string_strip_ns ~ty:Full.locl_ty (Loclenv env) ty)

let full_strip_ns_i env ty =
  supply_fuel
    env.genv.tcopt
    (Full.to_string_strip_ns ~ty:Full.internal_type (Loclenv env) ty)

let full_strip_ns_decl env ty =
  supply_fuel
    env.genv.tcopt
    (Full.to_string_strip_ns ~ty:Full.decl_ty (Loclenv env) ty)

let full_with_identity env x occurrence definition_opt =
  supply_fuel
    env.genv.tcopt
    (Full.to_string_with_identity env x occurrence definition_opt)

let full_decl tcopt ty = supply_fuel tcopt (Full.to_string_decl ty)

let debug env ty =
  Full.debug_mode := true;
  let f_str = full_strip_ns env ty in
  Full.debug_mode := false;
  f_str

let debug_decl env ty =
  Full.debug_mode := true;
  let f_str = full_strip_ns_decl env ty in
  Full.debug_mode := false;
  f_str

let debug_i env ty =
  Full.debug_mode := true;
  let f_str = full_strip_ns_i env ty in
  Full.debug_mode := false;
  f_str

let class_ ctx c =
  supply_fuel (Provider_context.get_tcopt ctx) (PrintClass.class_type ctx c)

let gconst tcopt gc = supply_fuel tcopt (Full.to_string_decl gc.cd_type)

let fun_ tcopt { fe_type; _ } = supply_fuel tcopt (Full.to_string_decl fe_type)

let fun_type tcopt f = supply_fuel tcopt (Full.fun_to_string f)

let typedef tcopt td = supply_fuel tcopt (PrintTypedef.typedef td)

let constraints_for_type env ty =
  supply_fuel env.genv.tcopt (fun ~fuel ->
      let (fuel, doc) =
        Full.constraints_for_type ~fuel Full.text_strip_ns env ty
      in
      let str =
        Libhackfmt.format_doc_unbroken Full.format_env doc |> String.strip
      in
      (fuel, str))

let classish_kind c_kind final = ErrorString.classish_kind c_kind final

let coercion_direction cd =
  match cd with
  | CoerceToDynamic -> "to"
  | CoerceFromDynamic -> "from"

let subtype_prop env prop =
  let rec subtype_prop = function
    | Conj [] -> "TRUE"
    | Conj ps ->
      "(" ^ String.concat ~sep:" && " (List.map ~f:subtype_prop ps) ^ ")"
    | Disj (_, []) -> "FALSE"
    | Disj (_, ps) ->
      "(" ^ String.concat ~sep:" || " (List.map ~f:subtype_prop ps) ^ ")"
    | IsSubtype (None, ty1, ty2) -> debug_i env ty1 ^ " <: " ^ debug_i env ty2
    | IsSubtype (Some cd, ty1, ty2) ->
      debug_i env ty1 ^ " " ^ coercion_direction cd ^ "~> " ^ debug_i env ty2
  in
  let p_str = subtype_prop prop in
  p_str

let coeffects env ty =
  supply_fuel env.genv.tcopt @@ fun ~fuel ->
  let to_string ~fuel ty =
    with_blank_tyvars (fun () ->
        Full.to_string
          ~fuel
          ~ty:Full.locl_ty
          (fun s -> Doc.text (Utils.strip_all_ns s))
          (Loclenv env)
          ty)
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
    | Toption ty' ->
      begin
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
