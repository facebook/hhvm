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
open Typing_env_types
open Typing_logic
module SN = Naming_special_names
module Reason = Typing_reason
module TySet = Typing_set
module Cls = Decl_provider.Class
module Nast = Aast
module ITySet = Internal_type_set

let strip_ns id = id |> Utils.strip_ns |> Hh_autoimport.reverse_type

let shallow_decl_enabled (ctx : Provider_context.t) : bool =
  TypecheckerOptions.shallow_class_decl (Provider_context.get_tcopt ctx)

(*****************************************************************************)
(* Pretty-printer of the "full" type.                                        *)
(* This is used in server/symbolTypeService and elsewhere                    *)
(* With debug_mode set it is used for hh_show_env                            *)
(*****************************************************************************)

module Full = struct
  module Env = Typing_env
  open Doc

  let format_env = Format_env.{ default with line_width = 60 }

  let text_strip_ns s = Doc.text (strip_ns s)

  let ( ^^ ) a b = Concat [a; b]

  let debug_mode = ref false

  let show_verbose env = Env.get_log_level env "show" > 1

  let blank_tyvars = ref false

  let comma_sep = Concat [text ","; Space]

  let id x = x

  let list_sep ?(split = true) (s : Doc.t) (f : 'a -> Doc.t) (l : 'a list) :
      Doc.t =
    let split =
      if split then
        Split
      else
        Nothing
    in
    let max_idx = List.length l - 1 in
    let elements =
      List.mapi l ~f:(fun idx element ->
          if Int.equal idx max_idx then
            f element
          else
            Concat [f element; s; split])
    in
    match elements with
    | [] -> Nothing
    | xs -> Nest [split; Concat xs; split]

  let delimited_list sep left_delimiter f l right_delimiter =
    Span
      [
        text left_delimiter;
        WithRule (Rule.Parental, Concat [list_sep sep f l; text right_delimiter]);
      ]

  let list : type c. _ -> (c -> Doc.t) -> c list -> _ -> _ =
   (fun ld x y rd -> delimited_list comma_sep ld x y rd)

  let shape_map fdm f_field =
    let compare (k1, _) (k2, _) =
      String.compare (Env.get_shape_field_name k1) (Env.get_shape_field_name k2)
    in
    let fields = List.sort ~compare (TShapeMap.bindings fdm) in
    List.map fields f_field

  let rec fun_type ~ty to_doc st env ft =
    let params = List.map ft.ft_params (fun_param ~ty to_doc st env) in
    let variadic_param =
      match ft.ft_arity with
      | Fstandard -> None
      | Fvariadic p ->
        Some
          (Concat
             [
               (match ty to_doc st env p.fp_type.et_type with
               | Text ("_", 1) ->
                 (* Handle the case of missing a type by not printing it *)
                 Nothing
               | _ -> fun_param ~ty to_doc st env p);
               text "...";
             ])
    in
    let params =
      match variadic_param with
      | None -> params
      | Some variadic_param -> params @ [variadic_param]
    in
    Span
      [
        (* only print tparams when they have been instantiated with targs
         * so that they correctly express reified parameterization *)
        (match (ft.ft_tparams, get_ft_ftk ft) with
        | ([], _)
        | (_, FTKtparams) ->
          Nothing
        | (l, FTKinstantiated_targs) ->
          list "<" (tparam ~ty to_doc st env) l ">");
        list "(" id params "):";
        Space;
        possibly_enforced_ty ~ty to_doc st env ft.ft_ret;
      ]

  and possibly_enforced_ty ~ty to_doc st env { et_enforced; et_type } =
    Concat
      [
        ( if show_verbose env then
          match et_enforced with
          | Enforced -> text "enforced" ^^ Space
          | PartiallyEnforced (_, (_, cn)) ->
            text ("partially enforced " ^ cn) ^^ Space
          | Unenforced -> Nothing
        else
          Nothing );
        ty to_doc st env et_type;
      ]

  and fun_param ~ty to_doc st env ({ fp_name; fp_type; _ } as fp) =
    Concat
      [
        (match get_fp_mode fp with
        | FPinout -> text "inout" ^^ Space
        | _ -> Nothing);
        (match (fp_name, ty to_doc st env fp_type.et_type) with
        | (None, _) -> possibly_enforced_ty ~ty to_doc st env fp_type
        | (Some param_name, Text ("_", 1)) ->
          (* Handle the case of missing a type by not printing it *)
          text param_name
        | (Some param_name, _) ->
          Concat
            [
              possibly_enforced_ty ~ty to_doc st env fp_type;
              Space;
              text param_name;
            ]);
        ( if get_fp_has_default fp then
          text "=_"
        else
          Nothing );
      ]

  and tparam
      ~ty
      to_doc
      st
      env
      { tp_name = (_, x); tp_constraints = cstrl; tp_reified = r; _ } =
    Concat
      [
        begin
          match r with
          | Nast.Erased -> Nothing
          | Nast.SoftReified -> text "<<__Soft>> reify" ^^ Space
          | Nast.Reified -> text "reify" ^^ Space
        end;
        text x;
        list_sep ~split:false Space (tparam_constraint ~ty to_doc st env) cstrl;
      ]

  and tparam_constraint ~ty to_doc st env (ck, cty) =
    Concat
      [
        Space;
        text
          (match ck with
          | Ast_defs.Constraint_as -> "as"
          | Ast_defs.Constraint_super -> "super"
          | Ast_defs.Constraint_eq -> "=");
        Space;
        ty to_doc st env cty;
      ]

  let terr () =
    text
      ( if !debug_mode then
        "err"
      else
        "_" )

  let tprim x =
    text
    @@
    match x with
    | Nast.Tnull -> "null"
    | Nast.Tvoid -> "void"
    | Nast.Tint -> "int"
    | Nast.Tbool -> "bool"
    | Nast.Tfloat -> "float"
    | Nast.Tstring -> "string"
    | Nast.Tnum -> "num"
    | Nast.Tresource -> "resource"
    | Nast.Tarraykey -> "arraykey"
    | Nast.Tnoreturn -> "noreturn"

  let tdarray k x y = list "darray<" k [x; y] ">"

  let tvarray k x = list "varray<" k [x] ">"

  let tvarray_or_darray k x y = list "varray_or_darray<" k [x; y] ">"

  let tfun ~ty to_doc st env ft =
    Concat
      [
        text "(";
        ( if get_ft_readonly_this ft then
          text "readonly "
        else
          Nothing );
        text "function";
        fun_type ~ty to_doc st env ft;
        text ")";
      ]

  let ttuple k tyl = list "(" k tyl ")"

  let tshape k to_doc shape_kind fdm =
    let fields =
      let f_field (shape_map_key, { sft_optional; sft_ty }) =
        let key_delim =
          match shape_map_key with
          | Typing_defs.TSFlit_str _ -> text "'"
          | _ -> Nothing
        in
        Concat
          [
            ( if sft_optional then
              text "?"
            else
              Nothing );
            key_delim;
            to_doc (Env.get_shape_field_name shape_map_key);
            key_delim;
            Space;
            text "=>";
            Space;
            k sft_ty;
          ]
      in
      shape_map fdm f_field
    in
    let fields =
      match shape_kind with
      | Closed_shape -> fields
      | Open_shape -> fields @ [text "..."]
    in
    list "shape(" id fields ")"

  let thas_member k hm =
    let { hm_name = (_, name); hm_type; hm_class_id = _; hm_explicit_targs } =
      hm
    in
    (* TODO: T71614503 print explicit type arguments appropriately *)
    let printed_explicit_targs =
      match hm_explicit_targs with
      | None -> text "None"
      | Some _ -> text "Some <targs>"
    in
    Concat
      [
        text "has_member";
        text "(";
        text name;
        comma_sep;
        k hm_type;
        comma_sep;
        printed_explicit_targs;
        text ")";
      ]

  let tdestructure k d =
    let { d_required; d_optional; d_variadic; d_kind } = d in
    let e_required = List.map d_required ~f:k in
    let e_optional =
      List.map d_optional ~f:(fun v -> Concat [text "=_"; k v])
    in
    let e_variadic =
      Option.value_map
        ~default:[]
        ~f:(fun v -> [Concat [text "..."; k v]])
        d_variadic
    in
    let prefix =
      match d_kind with
      | ListDestructure -> text "list"
      | SplatUnpack -> text "splat"
    in
    Concat [prefix; list "(" id (e_required @ e_optional @ e_variadic) ")"]

  let rec decl_ty to_doc st env x = decl_ty_ to_doc st env (get_node x)

  and decl_ty_ : _ -> _ -> _ -> decl_phase ty_ -> Doc.t =
   fun to_doc st env x ->
    let ty = decl_ty in
    let k x = ty to_doc st env x in
    match x with
    | Tany _ -> text "_"
    | Terr -> terr ()
    | Tthis -> text SN.Typehints.this
    | Tmixed -> text "mixed"
    | Tdynamic -> text "dynamic"
    | Tnonnull -> text "nonnull"
    | Tdarray (x, y) -> tdarray k x y
    | Tvarray x -> tvarray k x
    | Tvarray_or_darray (x, y) -> tvarray_or_darray k x y
    | Tvec_or_dict (x, y) -> list "vec_or_dict<" k [x; y] ">"
    | Tapply ((_, s), []) -> to_doc s
    | Tgeneric (s, []) -> to_doc s
    | Taccess (root_ty, id) -> Concat [k root_ty; text "::"; to_doc (snd id)]
    | Toption x -> Concat [text "?"; k x]
    | Tlike x -> Concat [text "~"; k x]
    | Tprim x -> tprim x
    | Tvar x -> text (Printf.sprintf "#%d" x)
    | Tfun ft -> tfun ~ty to_doc st env ft
    (* Don't strip_ns here! We want the FULL type, including the initial slash.
      *)
    | Tapply ((_, s), tyl)
    | Tgeneric (s, tyl) ->
      to_doc s ^^ list "<" k tyl ">"
    | Ttuple tyl -> ttuple k tyl
    | Tunion tyl -> Concat [text "|"; ttuple k tyl]
    | Tintersection tyl -> Concat [text "&"; ttuple k tyl]
    | Tshape (shape_kind, fdm) -> tshape k to_doc shape_kind fdm

  (* For a given type parameter, construct a list of its constraints *)
  let get_constraints_on_tparam env tparam =
    let kind_opt = Env.get_pos_and_kind_of_generic env tparam in
    match kind_opt with
    | None -> []
    | Some (_pos, kind) ->
      (* Use the names of the parameters themselves to present bounds
         depending on other parameters *)
      let param_names = Type_parameter_env.get_parameter_names kind in
      let params =
        List.map param_names (fun name ->
            Typing_make_type.generic Reason.none name)
      in
      let lower = Env.get_lower_bounds env tparam params in
      let upper = Env.get_upper_bounds env tparam params in
      let equ = Env.get_equal_bounds env tparam params in
      (* If we have an equality we can ignore the other bounds *)
      if not (TySet.is_empty equ) then
        List.map (TySet.elements equ) (fun ty ->
            (tparam, Ast_defs.Constraint_eq, ty))
      else
        List.map (TySet.elements lower) (fun ty ->
            (tparam, Ast_defs.Constraint_super, ty))
        @ List.map (TySet.elements upper) (fun ty ->
              (tparam, Ast_defs.Constraint_as, ty))

  let rec locl_ty : _ -> _ -> _ -> locl_ty -> Doc.t =
   fun to_doc st env ty ->
    let (r, x) = deref ty in
    let d = locl_ty_ to_doc st env x in
    match r with
    | Typing_reason.Rsolve_fail _ -> Concat [text "{suggest:"; d; text "}"]
    | _ -> d

  and locl_ty_ : _ -> _ -> _ -> locl_phase ty_ -> Doc.t =
   fun to_doc st env x ->
    let ty = locl_ty in
    let k x = ty to_doc st env x in
    match x with
    | Tany _ -> text "_"
    | Terr -> terr ()
    | Tdynamic -> text "dynamic"
    | Tnonnull -> text "nonnull"
    | Tvarray_or_darray (x, y) -> tvarray_or_darray k x y
    | Tvec_or_dict (x, y) -> list "vec_or_dict<" k [x; y] ">"
    | Tvarray x -> tvarray k x
    | Tdarray (x, y) -> tdarray k x y
    | Tclass ((_, s), Exact, []) when !debug_mode ->
      Concat [text "exact"; Space; to_doc s]
    | Tclass ((_, s), _, []) -> to_doc s
    | Toption ty ->
      begin
        match deref ty with
        | (_, Tnonnull) -> text "mixed"
        | (r, Tunion tyl)
          when TypecheckerOptions.like_type_hints (Env.get_tcopt env)
               && List.exists ~f:is_dynamic tyl ->
          (* Unions with null become Toption, which leads to the awkward ?~...
           * The Tunion case can better handle this *)
          k (mk (r, Tunion (mk (r, Tprim Nast.Tnull) :: tyl)))
        | _ -> Concat [text "?"; k ty]
      end
    | Tprim x -> tprim x
    | Tneg x -> Concat [text "not "; tprim x]
    | Tvar n ->
      let (_, ety) = Env.expand_type env (mk (Reason.Rnone, Tvar n)) in
      begin
        match deref ety with
        (* For unsolved type variables, always show the type variable *)
        | (_, Tvar n') ->
          if ISet.mem n' st then
            text "[rec]"
          else if !blank_tyvars then
            text "[unresolved]"
          else
            text ("#" ^ string_of_int n')
        | _ ->
          let prepend =
            if ISet.mem n st then
              text "[rec]"
            else if
              (* For hh_show_env we further show the type variable number *)
              show_verbose env
            then
              text ("#" ^ string_of_int n)
            else
              Nothing
          in
          let st = ISet.add n st in
          Concat [prepend; ty to_doc st env ety]
      end
    | Tfun ft -> tfun ~ty to_doc st env ft
    | Tclass ((_, s), exact, tyl) ->
      let d = to_doc s ^^ list "<" k tyl ">" in
      begin
        match exact with
        | Exact when !debug_mode -> Concat [text "exact"; Space; d]
        | _ -> d
      end
    | Tgeneric (s, []) when String.contains s '$' ->
      begin
        (* Saves a call to is_prefix then chop_prefix_exn *)
        match String.chop_prefix ~prefix:"Tctx" s with
        | Some var -> (* Tctx$f *) to_doc ("ctx " ^ var)
        | None ->
          begin
            match String.rsplit2 s '@' with
            | Some (tvar, cst) ->
              (* T$x@C *) to_doc (String.drop_prefix tvar 1 ^ "::" ^ cst)
            | None ->
              (* T$x *)
              begin
                match get_constraints_on_tparam env s with
                | [(_, Ast_defs.Constraint_as, ty)] -> locl_ty to_doc st env ty
                | _ -> (* this case shouldn't occur *) to_doc s
              end
          end
      end
    | Tunapplied_alias s
    | Tnewtype (s, [], _)
    | Tgeneric (s, []) ->
      to_doc s
    | Tnewtype (s, tyl, _)
    | Tgeneric (s, tyl) ->
      to_doc s ^^ list "<" k tyl ">"
    | Tdependent (dep, cstr) ->
      let cstr_info = Concat [Space; text "as"; Space; k cstr] in
      Concat [to_doc @@ DependentKind.to_string dep; cstr_info]
    (* Don't strip_ns here! We want the FULL type, including the initial slash.
      *)
    | Ttuple tyl -> ttuple k tyl
    | Tunion [] -> text "nothing"
    | Tunion tyl when TypecheckerOptions.like_type_hints (Env.get_tcopt env) ->
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
        | (false, false, []) -> text "nothing"
        (* type isn't nullable or dynamic *)
        | (false, false, [ty]) ->
          if show_verbose env then
            Concat [text "("; k ty; text ")"]
          else
            k ty
        | (false, false, _ :: _) ->
          delimited_list (Space ^^ text "|" ^^ Space) "(" k nonnull ")"
        (* Type only is null *)
        | (false, true, []) ->
          if show_verbose env then
            text "(null)"
          else
            text "null"
        (* Type only is dynamic *)
        | (true, false, []) ->
          if show_verbose env then
            text "(dynamic)"
          else
            text "dynamic"
        (* Type is nullable single type *)
        | (false, true, [ty]) ->
          if show_verbose env then
            Concat [text "(null |"; k ty; text ")"]
          else
            Concat [text "?"; k ty]
        (* Type is like single type *)
        | (true, false, [ty]) ->
          if show_verbose env then
            Concat [text "(dynamic |"; k ty; text ")"]
          else
            Concat [text "~"; k ty]
        (* Type is like null *)
        | (true, true, []) ->
          if show_verbose env then
            text "(dynamic | null)"
          else
            text "~null"
        (* Type is like nullable single type *)
        | (true, true, [ty]) ->
          if show_verbose env then
            Concat [text "(dynamic | null |"; k ty; text ")"]
          else
            Concat [text "~?"; k ty]
        | (true, false, _ :: _) ->
          Concat
            [
              text "~";
              delimited_list (Space ^^ text "|" ^^ Space) "(" k nonnull ")";
            ]
        | (false, true, _ :: _) ->
          Concat
            [
              text "?";
              delimited_list (Space ^^ text "|" ^^ Space) "(" k nonnull ")";
            ]
        | (true, true, _ :: _) ->
          Concat
            [
              text "~";
              text "?";
              delimited_list (Space ^^ text "|" ^^ Space) "(" k nonnull ")";
            ]
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
          if show_verbose env then
            Concat [text "("; k ty; text ")"]
          else
            k ty
        | ([], _) ->
          delimited_list (Space ^^ text "|" ^^ Space) "(" k nonnull ")"
        (* Type only is null *)
        | (_, []) ->
          if show_verbose env then
            text "(null)"
          else
            text "null"
        (* Type is nullable single type *)
        | (_, [ty]) ->
          if show_verbose env then
            Concat [text "(null |"; k ty; text ")"]
          else
            Concat [text "?"; k ty]
        (* Type is nullable union type *)
        | (_, _) ->
          Concat
            [
              text "?";
              delimited_list (Space ^^ text "|" ^^ Space) "(" k nonnull ")";
            ]
      end
    | Tintersection [] -> text "mixed"
    | Tintersection tyl ->
      delimited_list (Space ^^ text "&" ^^ Space) "(" k tyl ")"
    | Tobject -> text "object"
    | Tshape (shape_kind, fdm) -> tshape k to_doc shape_kind fdm
    | Taccess (root_ty, id) -> Concat [k root_ty; text "::"; to_doc (snd id)]

  let rec constraint_type_ to_doc st env x =
    let k lty = locl_ty to_doc st env lty in
    let k' cty = constraint_type to_doc st env cty in
    match x with
    | Thas_member hm -> thas_member k hm
    | Tdestructure d -> tdestructure k d
    | TCunion (lty, cty) -> Concat [text "("; k lty; text "|"; k' cty; text ")"]
    | TCintersection (lty, cty) ->
      Concat [text "("; k lty; text "&"; k' cty; text ")"]

  and constraint_type to_doc st env ty =
    let (r, x) = deref_constraint_type ty in
    let d = constraint_type_ to_doc st env x in
    match r with
    | Typing_reason.Rsolve_fail _ -> Concat [text "{suggest:"; d; text "}"]
    | _ -> d

  let internal_type to_doc st env ty =
    match ty with
    | LoclType ty -> locl_ty to_doc st env ty
    | ConstraintType ty -> constraint_type to_doc st env ty

  let to_string ~ty to_doc env x =
    ty to_doc ISet.empty env x
    |> Libhackfmt.format_doc_unbroken format_env
    |> String.strip

  (* Print a suffix for type parameters in typ that have constraints
   * If the type itself is a type parameter with a single constraint, just
   * represent this as `as t` or `super t`, otherwise use full `where` syntax
   *)
  let constraints_for_type to_doc env typ =
    let tparams = SSet.elements (Env.get_tparams env typ) in
    let constraints = List.concat_map tparams (get_constraints_on_tparam env) in
    let (_, typ) = Env.expand_type env typ in
    match (get_node typ, constraints) with
    | (_, []) -> Nothing
    | (Tgeneric (tparam, []), [(tparam', ck, typ)])
      when String.equal tparam tparam' ->
      tparam_constraint ~ty:locl_ty to_doc ISet.empty env (ck, typ)
    | _ ->
      Concat
        [
          Newline;
          text "where";
          Space;
          WithRule
            ( Rule.Parental,
              list_sep
                comma_sep
                begin
                  fun (tparam, ck, typ) ->
                  Concat
                    [
                      text tparam;
                      tparam_constraint
                        ~ty:locl_ty
                        to_doc
                        ISet.empty
                        env
                        (ck, typ);
                    ]
                end
                constraints );
        ]

  let to_string_rec env n x =
    locl_ty Doc.text (ISet.add n ISet.empty) env x
    |> Libhackfmt.format_doc_unbroken format_env
    |> String.strip

  let to_string_strip_ns ~ty env x = to_string ~ty text_strip_ns env x

  let to_string_decl ctx (x : decl_ty) =
    let ty = decl_ty in
    let env = Typing_env.empty ctx Relative_path.default ~droot:None in
    to_string ~ty Doc.text env x

  let fun_to_string ctx (x : decl_fun_type) =
    let ty = decl_ty in
    let env = Typing_env.empty ctx Relative_path.default ~droot:None in
    fun_type ~ty Doc.text ISet.empty env x
    |> Libhackfmt.format_doc_unbroken format_env
    |> String.strip

  let to_string_with_identity env x occurrence definition_opt =
    let ty = locl_ty in
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
            | ms -> Concat (List.map ms print_mod) ^^ SplitWith Cost.Base
          end)
    in
    let body =
      SymbolOccurrence.(
        match (occurrence, get_node x) with
        | ({ type_ = Class _; name; _ }, _) ->
          Concat [text "class"; Space; text_strip_ns name]
        | ({ type_ = Function; name; _ }, Tfun ft)
        | ({ type_ = Method (_, name); _ }, Tfun ft) ->
          (* Use short names for function types since they display a lot more
           information to the user. *)
          Concat
            [
              text "function";
              Space;
              text_strip_ns name;
              fun_type ~ty text_strip_ns ISet.empty env ft;
            ]
        | ({ type_ = Property _; name; _ }, _)
        | ({ type_ = XhpLiteralAttr _; name; _ }, _)
        | ({ type_ = ClassConst _; name; _ }, _)
        | ({ type_ = GConst; name; _ }, _)
        | ({ type_ = EnumClassLabel _; name; _ }, _) ->
          Concat [ty text_strip_ns ISet.empty env x; Space; text_strip_ns name]
        | _ -> ty text_strip_ns ISet.empty env x)
    in
    let constraints = constraints_for_type text_strip_ns env x in
    Concat [prefix; body; constraints]
    |> Libhackfmt.format_doc format_env
    |> String.strip
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
  module Env = Typing_env

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

  let varray = "a varray"

  let darray = "a darray"

  let varray_or_darray = "a varray_or_darray"

  let rec type_ ?(ignore_dynamic = false) env ty =
    match ty with
    | Tany _ -> "an untyped value"
    | Terr -> "a type error"
    | Tdynamic -> "a dynamic value"
    | Tunion l when ignore_dynamic ->
      union env (List.filter l ~f:(fun x -> not (is_dynamic x)))
    | Tunion l -> union env l
    | Tintersection [] -> "a mixed value"
    | Tintersection l -> intersection env l
    | Tvarray_or_darray _ -> varray_or_darray
    | Tvec_or_dict _ -> "a vec_or_dict"
    | Tvarray _ -> varray
    | Tdarray (_, _) -> darray
    | Ttuple l -> "a tuple of size " ^ string_of_int (List.length l)
    | Tnonnull -> "a nonnull value"
    | Toption x ->
      begin
        match get_node x with
        | Tnonnull -> "a mixed value"
        | _ -> "a nullable type"
      end
    | Tprim tp -> tprim tp
    | Tvar _ -> "some value"
    | Tfun _ -> "a function"
    | Tgeneric (s, tyl) when DependentKind.is_generic_dep_ty s ->
      "the expression dependent type " ^ s ^ inst env tyl
    | Tgeneric (x, tyl) -> "a value of generic type " ^ x ^ inst env tyl
    | Tnewtype (x, _, _) when String.equal x SN.Classes.cClassname ->
      "a classname string"
    | Tnewtype (x, _, _) when String.equal x SN.Classes.cTypename ->
      "a typename string"
    | Tnewtype (x, tyl, _) -> "a value of type " ^ strip_ns x ^ inst env tyl
    | Tdependent (dep, _cstr) -> dependent dep
    | Tclass ((_, x), Exact, tyl) ->
      "an object of exactly the class " ^ strip_ns x ^ inst env tyl
    | Tclass ((_, x), Nonexact, tyl) ->
      "an object of type " ^ strip_ns x ^ inst env tyl
    | Tobject -> "an object"
    | Tshape _ -> "a shape"
    | Tunapplied_alias _ ->
      (* FIXME it seems like this function is only for
         fully-applied types? Tunapplied_alias should only appear
         in a type argument position then, which inst below
         prints with a different function (namely Full.locl_ty)  *)
      failwith "Tunapplied_alias is not a type"
    | Taccess (_ty, _id) -> "a type constant"
    | Tneg p -> "not a " ^ tprim p

  and inst env tyl =
    if List.is_empty tyl then
      ""
    else
      with_blank_tyvars (fun () ->
          "<"
          ^ String.concat
              ~sep:", "
              (List.map tyl ~f:(Full.to_string_strip_ns ~ty:Full.locl_ty env))
          ^ ">")

  and dependent dep =
    let x = strip_ns @@ DependentKind.to_string dep in
    match dep with
    | DTexpr _ -> "the expression dependent type " ^ x

  and union env l =
    let (null, nonnull) =
      List.partition_tf l (fun ty ->
          equal_locl_ty_ (get_node ty) (Tprim Nast.Tnull))
    in
    let l = List.map nonnull (to_string env) in
    let s = List.fold_right l ~f:SSet.add ~init:SSet.empty in
    let l = SSet.elements s in
    if List.is_empty null then
      union_ l
    else
      "a nullable type"

  and union_ = function
    | [] -> "an undefined value"
    | [x] -> x
    | x :: rl -> x ^ " or " ^ union_ rl

  and intersection env l =
    let l = List.map l ~f:(to_string env) in
    String.concat l ~sep:" and "

  and class_kind c_kind final =
    let fs =
      if final then
        " final"
      else
        ""
    in
    match c_kind with
    | Ast_defs.Cabstract -> "an abstract" ^ fs ^ " class"
    | Ast_defs.Cnormal -> "a" ^ fs ^ " class"
    | Ast_defs.Cinterface -> "an interface"
    | Ast_defs.Ctrait -> "a trait"
    | Ast_defs.Cenum -> "an enum"

  and to_string ?(ignore_dynamic = false) env ty =
    let (_, ety) = Env.expand_type env ty in
    type_ ~ignore_dynamic env (get_node ety)
end

module Json = struct
  open Hh_json

  let prim = function
    | Nast.Tnull -> "null"
    | Nast.Tvoid -> "void"
    | Nast.Tint -> "int"
    | Nast.Tbool -> "bool"
    | Nast.Tfloat -> "float"
    | Nast.Tstring -> "string"
    | Nast.Tnum -> "num"
    | Nast.Tresource -> "resource"
    | Nast.Tarraykey -> "arraykey"
    | Nast.Tnoreturn -> "noreturn"

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
    let args tys = [("args", JSON_Array (List.map tys (from_type env)))] in
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
    let fields fl = [("fields", JSON_Array (List.map fl make_field))] in
    let as_type ty = [("as", from_type env ty)] in
    match (get_pos ty, get_node ty) with
    | (_, Tvar n) ->
      let (_, ty) = Typing_env.expand_type env (mk (get_reason ty, Tvar n)) in
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
    | (p, Tnewtype (s, _, ty)) when Typing_env.is_enum env s ->
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
    | (p, Tprim tp) -> obj @@ kind p "primitive" @ name (prim tp)
    | (p, Tneg tp) -> obj @@ kind p "negation" @ name (prim tp)
    | (p, Tclass ((_, cid), _, tys)) ->
      obj @@ kind p "class" @ name cid @ args tys
    | (p, Tobject) -> obj @@ kind p "object"
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
      let params fps = [("params", JSON_Array (List.map fps param))] in
      obj @@ fun_kind p @ params ft.ft_params @ result ft.ft_ret.et_type
    | (p, Tvarray_or_darray (ty1, ty2)) ->
      obj @@ kind p "varray_or_darray" @ args [ty1; ty2]
    | (p, Tvec_or_dict (ty1, ty2)) ->
      obj @@ kind p "vec_or_dict" @ args [ty1; ty2]
    | (p, Tdarray (ty1, ty2)) -> obj @@ kind p "darray" @ args [ty1; ty2]
    | (p, Tvarray ty) -> obj @@ kind p "varray" @ args [ty]
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
        | "darray" ->
          get_array "args" (json, keytrace) >>= fun (args, keytrace) ->
          begin
            match args with
            | [ty1; ty2] ->
              aux ty1 ~keytrace:("0" :: keytrace) >>= fun ty1 ->
              aux ty2 ~keytrace:("1" :: keytrace) >>= fun ty2 ->
              ty (Tdarray (ty1, ty2))
            | _ ->
              deserialization_error
                ~message:
                  (Printf.sprintf
                     "Invalid number of type arguments to darray (expected 2): %d"
                     (List.length args))
                ~keytrace
          end
        | "varray" ->
          get_array "args" (json, keytrace) >>= fun (args, keytrace) ->
          begin
            match args with
            | [ty1] ->
              aux ty1 ~keytrace:("0" :: keytrace) >>= fun ty1 ->
              ty (Tvarray ty1)
            | _ ->
              deserialization_error
                ~message:
                  (Printf.sprintf
                     "Invalid number of type arguments to varray (expected 1): %d"
                     (List.length args))
                ~keytrace
          end
        | "varray_or_darray" ->
          get_array "args" (json, keytrace) >>= fun (args, keytrace) ->
          begin
            match args with
            | [ty1; ty2] ->
              aux ty1 ~keytrace:("0" :: keytrace) >>= fun ty1 ->
              aux ty2 ~keytrace:("1" :: keytrace) >>= fun ty2 ->
              ty (Tvarray_or_darray (ty1, ty2))
            | _ ->
              deserialization_error
                ~message:
                  (Printf.sprintf
                     "Invalid number of type arguments to varray_or_darray (expected 2): %d"
                     (List.length args))
                ~keytrace
          end
        | "array" ->
          get_array "args" (json, keytrace) >>= fun (args, _args_keytrace) ->
          begin
            match args with
            | [] ->
              let tany = mk (Reason.Rnone, Typing_defs.make_tany ()) in
              ty (Tvarray_or_darray (tany, tany))
            | [ty1] ->
              aux ty1 ~keytrace:("0" :: keytrace) >>= fun ty1 ->
              ty (Tvarray ty1)
            | [ty1; ty2] ->
              aux ty1 ~keytrace:("0" :: keytrace) >>= fun ty1 ->
              aux ty2 ~keytrace:("1" :: keytrace) >>= fun ty2 ->
              ty (Tdarray (ty1, ty2))
            | _ ->
              deserialization_error
                ~message:
                  (Printf.sprintf
                     "Invalid number of type arguments to array (expected 0-2): %d"
                     (List.length args))
                ~keytrace
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
        | "object" -> ty Tobject
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
                        ~is_atom:false
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
                 ft_arity = Fstandard;
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
              ( "Invalid as-constraint: "
              ^ Hh_json.Access.access_failure_to_string access_failure )
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

module PrintClass = struct
  let indent = "    "

  let bool = string_of_bool

  let sset s =
    let contents = SSet.fold (fun x acc -> x ^ " " ^ acc) s "" in
    Printf.sprintf "Set( %s)" contents

  let pos_or_decl p =
    let (line, start, end_) = Pos_or_decl.line_start_end_columns p in
    Printf.sprintf "(line %d: chars %d-%d)" line start end_

  let class_kind = function
    | Ast_defs.Cabstract -> "Cabstract"
    | Ast_defs.Cnormal -> "Cnormal"
    | Ast_defs.Cinterface -> "Cinterface"
    | Ast_defs.Ctrait -> "Ctrait"
    | Ast_defs.Cenum -> "Cenum"

  let constraint_ty ctx = function
    | (Ast_defs.Constraint_as, ty) -> "as " ^ Full.to_string_decl ctx ty
    | (Ast_defs.Constraint_eq, ty) -> "= " ^ Full.to_string_decl ctx ty
    | (Ast_defs.Constraint_super, ty) -> "super " ^ Full.to_string_decl ctx ty

  let variance = function
    | Ast_defs.Covariant -> "+"
    | Ast_defs.Contravariant -> "-"
    | Ast_defs.Invariant -> ""

  let rec tparam
      ctx
      {
        tp_variance = var;
        tp_name = (position, name);
        tp_tparams = params;
        tp_constraints = cstrl;
        tp_reified = reified;
        tp_user_attributes = _;
      } =
    let params_string =
      if List.is_empty params then
        ""
      else
        "<" ^ tparam_list ctx params ^ ">"
    in
    variance var
    ^ pos_or_decl position
    ^ " "
    ^ name
    ^ params_string
    ^ " "
    ^ List.fold_right
        cstrl
        ~f:(fun x acc -> constraint_ty ctx x ^ " " ^ acc)
        ~init:""
    ^
    match reified with
    | Nast.Erased -> ""
    | Nast.SoftReified -> " soft reified"
    | Nast.Reified -> " reified"

  and tparam_list ctx l =
    List.fold_right l ~f:(fun x acc -> tparam ctx x ^ ", " ^ acc) ~init:""

  let class_elt ctx ({ ce_visibility; ce_type = (lazy ty); _ } as ce) =
    let vis =
      match ce_visibility with
      | Vpublic -> "public"
      | Vprivate _ -> "private"
      | Vprotected _ -> "protected"
    in
    let synth =
      if get_ce_synthesized ce then
        "synthetic "
      else
        ""
    in
    let type_ = Full.to_string_decl ctx ty in
    synth ^ vis ^ " " ^ type_

  let class_elts ctx m =
    List.fold m ~init:"" ~f:(fun acc (field, v) ->
        "(" ^ field ^ ": " ^ class_elt ctx v ^ ") " ^ acc)

  let class_elts_with_breaks ctx m =
    List.fold m ~init:"" ~f:(fun acc (field, v) ->
        "\n" ^ indent ^ field ^ ": " ^ class_elt ctx v ^ acc)

  let class_consts ctx m =
    List.fold m ~init:"" ~f:(fun acc (field, cc) ->
        let synth =
          if cc.cc_synthesized then
            "synthetic "
          else
            ""
        in
        "("
        ^ field
        ^ ": "
        ^ synth
        ^ Full.to_string_decl ctx cc.cc_type
        ^ ") "
        ^ acc)

  let typeconst
      ctx
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
    let ty x = Full.to_string_decl ctx x in
    let type_info =
      match kind with
      | TCConcrete { tc_type = t } -> Printf.sprintf " = %s" (ty t)
      | TCPartiallyAbstract { patc_constraint = c; patc_type = t } ->
        Printf.sprintf " as %s = %s" (ty c) (ty t)
      | TCAbstract
          { atc_as_constraint = a; atc_super_constraint = s; atc_default = d }
        ->
        let m = Option.value_map ~default:"" in
        let a = m a (fun x -> Printf.sprintf " as %s" (ty x)) in
        let s = m s (fun x -> Printf.sprintf " super %s" (ty x)) in
        let d = m d (fun x -> Printf.sprintf " = %s" (ty x)) in
        a ^ s ^ d
    in
    name
    ^ type_info
    ^ " (origin:"
    ^ origin
    ^ ")"
    ^ ( if synthetic then
        " (synthetic)"
      else
        "" )
    ^ ( if enforceable then
        " (enforceable)"
      else
        "" )
    ^
    if Option.is_some reifiable then
      " (reifiable)"
    else
      ""

  let typeconsts ctx m =
    List.fold m ~init:"" ~f:(fun acc (_, v) ->
        "\n(" ^ typeconst ctx v ^ ")" ^ acc)

  let ancestors ctx m =
    (* Format is as follows:
     *    ParentKnownToHack
     *  ! ParentCompletelyUnknown
     *  ~ ParentPartiallyKnown  (interface|abstract|trait)
     *
     * ParentPartiallyKnown must inherit one of the ! Unknown parents, so that
     * sigil could be omitted *)
    List.fold m ~init:"" ~f:(fun acc (field, v) ->
        let (sigil, kind) =
          match Decl_provider.get_class ctx field with
          | None -> ("!", "")
          | Some cls ->
            ( ( if Cls.members_fully_known cls then
                " "
              else
                "~" ),
              " (" ^ class_kind (Cls.kind cls) ^ ")" )
        in
        let ty_str = Full.to_string_decl ctx v in
        "\n" ^ indent ^ sigil ^ " " ^ ty_str ^ kind ^ acc)

  let constructor ctx (ce_opt, (consist : consistent_kind)) =
    let consist_str = Format.asprintf "(%a)" pp_consistent_kind consist in
    let ce_str =
      match ce_opt with
      | None -> ""
      | Some ce -> class_elt ctx ce
    in
    ce_str ^ consist_str

  let req_ancestors ctx xs =
    List.fold xs ~init:"" ~f:(fun acc (_p, x) ->
        acc ^ Full.to_string_decl ctx x ^ ", ")

  let class_type ctx c =
    let tenv = Typing_env.empty ctx Relative_path.default None in
    let tc_need_init = bool (Cls.need_init c) in
    let tc_members_fully_known = bool (Cls.members_fully_known c) in
    let tc_abstract = bool (Cls.abstract c) in
    let tc_deferred_init_members =
      sset
      @@
      if shallow_decl_enabled ctx then
        match Shallow_classes_provider.get ctx (Cls.name c) with
        | Some cls -> snd (Typing_deferred_members.class_ tenv cls)
        | None -> SSet.empty
      else
        Cls.deferred_init_members c
    in
    let tc_kind = class_kind (Cls.kind c) in
    let tc_name = Cls.name c in
    let tc_tparams = tparam_list ctx (Cls.tparams c) in
    let tc_consts = class_consts ctx (Cls.consts c) in
    let tc_typeconsts = typeconsts ctx (Cls.typeconsts c) in
    let tc_props = class_elts ctx (Cls.props c) in
    let tc_sprops = class_elts ctx (Cls.sprops c) in
    let tc_methods = class_elts_with_breaks ctx (Cls.methods c) in
    let tc_smethods = class_elts_with_breaks ctx (Cls.smethods c) in
    let tc_construct = constructor ctx (Cls.construct c) in
    let tc_ancestors = ancestors ctx (Cls.all_ancestors c) in
    let tc_req_ancestors = req_ancestors ctx (Cls.all_ancestor_reqs c) in
    let tc_req_ancestors_extends =
      String.concat ~sep:" " (Cls.all_ancestor_req_names c)
    in
    let tc_extends = String.concat ~sep:" " (Cls.all_extends_ancestors c) in
    "tc_need_init: "
    ^ tc_need_init
    ^ "\n"
    ^ "tc_members_fully_known: "
    ^ tc_members_fully_known
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
    ^ "tc_extends: "
    ^ tc_extends
    ^ "\n"
    ^ "tc_req_ancestors: "
    ^ tc_req_ancestors
    ^ "\n"
    ^ "tc_req_ancestors_extends: "
    ^ tc_req_ancestors_extends
    ^ "\n"
    ^ ""
end

module PrintTypedef = struct
  let typedef ctx = function
    | { td_pos; td_module = _; td_vis = _; td_tparams; td_constraint; td_type }
      ->
      let tparaml_s = PrintClass.tparam_list ctx td_tparams in
      let constr_s =
        match td_constraint with
        | None -> "[None]"
        | Some constr -> Full.to_string_decl ctx constr
      in
      let ty_s = Full.to_string_decl ctx td_type in
      let pos_s = PrintClass.pos_or_decl td_pos in
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
      ^ ""
end

(*****************************************************************************)
(* User API *)
(*****************************************************************************)

let error ?(ignore_dynamic = false) env ty =
  ErrorString.to_string ~ignore_dynamic env ty

let full env ty = Full.to_string ~ty:Full.locl_ty Doc.text env ty

let full_i env ty = Full.to_string ~ty:Full.internal_type Doc.text env ty

let full_rec env n ty = Full.to_string_rec env n ty

let full_strip_ns env ty = Full.to_string_strip_ns ~ty:Full.locl_ty env ty

let full_strip_ns_i env ty =
  Full.to_string_strip_ns ~ty:Full.internal_type env ty

let full_strip_ns_decl env ty = Full.to_string_strip_ns ~ty:Full.decl_ty env ty

let full_with_identity = Full.to_string_with_identity

let full_decl = Full.to_string_decl

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

let class_ ctx c = PrintClass.class_type ctx c

let gconst ctx gc = Full.to_string_decl ctx gc.cd_type

let fun_ ctx { fe_type; _ } = Full.to_string_decl ctx fe_type

let fun_type ctx f = Full.fun_to_string ctx f

let typedef ctx td = PrintTypedef.typedef ctx td

let constraints_for_type env ty =
  Full.constraints_for_type Full.text_strip_ns env ty
  |> Libhackfmt.format_doc_unbroken Full.format_env
  |> String.strip

let class_kind c_kind final = ErrorString.class_kind c_kind final

let coercion_direction cd =
  match cd with
  | CoerceToDynamic -> "to"
  | CoerceFromDynamic -> "from"
  | PartialCoerceFromDynamic (_, (_, cn)) -> "partial from " ^ cn

let subtype_prop env prop =
  let rec subtype_prop = function
    | Conj [] -> "TRUE"
    | Conj ps ->
      "(" ^ String.concat ~sep:" && " (List.map ~f:subtype_prop ps) ^ ")"
    | Disj (_, []) -> "FALSE"
    | Disj (_, ps) ->
      "(" ^ String.concat ~sep:" || " (List.map ~f:subtype_prop ps) ^ ")"
    | IsSubtype (ty1, ty2) -> debug_i env ty1 ^ " <: " ^ debug_i env ty2
    | Coerce (cd, ty1, ty2) ->
      debug env ty1 ^ " " ^ coercion_direction cd ^ "~> " ^ debug env ty2
  in
  let p_str = subtype_prop prop in
  p_str

let coeffects env ty =
  let to_string ty =
    with_blank_tyvars (fun () ->
        Full.to_string
          ~ty:Full.locl_ty
          (fun s -> Doc.text (Utils.strip_all_ns s))
          env
          ty)
  in
  let exception UndesugarableCoeffect of locl_ty in
  let exception Defaults in
  let rec desugar_simple_intersection (ty : locl_ty) : string list =
    match snd @@ deref ty with
    | Tvar v ->
      (* We are interested in the upper bounds because coeffects are parameters (contravariant).
       * Similar to Typing_subtype.describe_ty_super, we ignore Tvars appearing in bounds *)
      let upper_bounds =
        ITySet.elements (Typing_env.get_tyvar_upper_bounds env v)
        |> List.filter_map ~f:(function
               | LoclType lty ->
                 (match deref lty with
                 | (_, Tvar _) -> None
                 | _ -> Some lty)
               | ConstraintType _ -> None)
      in
      List.concat_map ~f:desugar_simple_intersection upper_bounds
    | Tintersection tyl -> List.concat_map ~f:desugar_simple_intersection tyl
    | Tunion [] ->
      (* TODO(coeffects) delete this special case when defaults is no longer equal to nothing *)
      raise Defaults
    | Tunion [ty] -> desugar_simple_intersection ty
    | Tunion _
    | Tnonnull
    | Tdynamic ->
      raise (UndesugarableCoeffect ty)
    | Toption ty' ->
      begin
        match deref ty' with
        | (_, Tnonnull) -> [] (* another special case of `mixed` *)
        | _ -> raise (UndesugarableCoeffect ty)
      end
    | _ -> [to_string ty]
  in

  try
    let (env, ty) = Typing_env.expand_type env ty in
    let ty =
      match deref ty with
      | (r, Tvar v) ->
        (* We are interested in the upper bounds because coeffects are parameters (contravariant).
         * Similar to Typing_subtype.describe_ty_super, we ignore Tvars appearing in bounds *)
        let upper_bounds =
          ITySet.elements (Typing_env.get_tyvar_upper_bounds env v)
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
    match desugar_simple_intersection ty with
    | [cap] -> "the capability " ^ cap
    | caps ->
      "the capability set {"
      ^ ( caps
        |> List.dedup_and_sort ~compare:String.compare
        |> String.concat ~sep:", " )
      ^ "}"
  with
  | UndesugarableCoeffect _ -> to_string ty
  | Defaults ->
    "the default capability set {AccessGlobals, Output, WriteProperty}"
