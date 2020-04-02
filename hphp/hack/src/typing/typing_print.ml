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
    let fields = List.sort ~compare (Nast.ShapeMap.bindings fdm) in
    List.map fields f_field

  let rec fun_type ~ty to_doc st env ft =
    let params = List.map ft.ft_params (fun_param ~ty to_doc st env) in
    let variadic_param =
      match ft.ft_arity with
      | Fstandard _ -> None
      | Fellipsis _ -> Some (text "...")
      | Fvariadic (_, p) ->
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
        ( if show_verbose env && et_enforced then
          text "enforced" ^^ Space
        else
          Nothing );
        ty to_doc st env et_type;
      ]

  and fun_param ~ty to_doc st env { fp_name; fp_type; fp_kind; _ } =
    Concat
      [
        (match fp_kind with
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
    | Nast.Tatom s -> ":@" ^ s

  let tdarray k x y = list "darray<" k [x; y] ">"

  let tvarray k x = list "varray<" k [x] ">"

  let tvarray_or_darray k x y =
    let targs =
      match x with
      | Some x -> [x; y]
      | None -> [y]
    in
    list "varray_or_darray<" k targs ">"

  let akvarray_or_darray k x y = list "varray_or_darray<" k [x; y] ">"

  let tarray k x y =
    match (x, y) with
    | (None, None) -> text "array"
    | (Some x, None) -> list "array<" k [x] ">"
    | (Some x, Some y) -> list "array<" k [x; y] ">"
    | (None, Some _) -> assert false

  let tfun ~ty to_doc st env ft =
    Concat
      [
        text "(";
        ( if get_ft_is_coroutine ft then
          text "coroutine" ^^ Space
        else
          Nothing );
        text "function";
        fun_type ~ty to_doc st env ft;
        text ")";
        (match get_reason ft.ft_ret.et_type with
        | Reason.Rdynamic_yield _ -> Space ^^ text "[DynamicYield]"
        | _ -> Nothing);
      ]

  let ttuple k tyl = list "(" k tyl ")"

  let tshape k to_doc shape_kind fdm =
    let fields =
      let f_field (shape_map_key, { sft_optional; sft_ty }) =
        let key_delim =
          match shape_map_key with
          | Ast_defs.SFlit_str _ -> text "'"
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

  let pu_concat k ty access = k ty ^^ text (":@" ^ access)

  let thas_member k hm =
    let { hm_name = (_, name); hm_type; hm_class_id = _ } = hm in
    Concat
      [text "has_member"; text "("; text name; comma_sep; k hm_type; text ")"]

  let tdestructure k d =
    let { d_required; d_optional; d_variadic; d_kind } = d in
    let e_required = List.map d_required ~f:k in
    let e_optional =
      List.map d_optional ~f:(fun v -> Concat [text "opt"; k v])
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
    | Tarray (x, y) -> tarray k x y
    | Tapply ((_, s), []) -> to_doc s
    | Tgeneric s -> to_doc s
    | Taccess (root_ty, ids) ->
      Concat
        [
          k root_ty;
          to_doc
            (List.fold_left
               ids
               ~f:(fun acc (_, sid) -> acc ^ "::" ^ sid)
               ~init:"");
        ]
    | Toption x -> Concat [text "?"; k x]
    | Tlike x -> Concat [text "~"; k x]
    | Tprim x -> tprim x
    | Tvar x -> text (Printf.sprintf "#%d" x)
    | Tfun ft -> tfun ~ty to_doc st env ft
    (* Don't strip_ns here! We want the FULL type, including the initial slash.
      *)
    | Tapply ((_, s), tyl) -> to_doc s ^^ list "<" k tyl ">"
    | Ttuple tyl -> ttuple k tyl
    | Tunion tyl -> Concat [text "|"; ttuple k tyl]
    | Tintersection tyl -> Concat [text "&"; ttuple k tyl]
    | Tshape (shape_kind, fdm) -> tshape k to_doc shape_kind fdm
    | Tpu_access (ty', (_, access), _) -> pu_concat k ty' access

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
    | Tarraykind (AKvarray_or_darray (x, y)) -> akvarray_or_darray k x y
    | Tarraykind (AKvarray x) -> tvarray k x
    | Tarraykind (AKdarray (x, y)) -> tdarray k x y
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
    | Tgeneric s -> to_doc s
    | Tnewtype (s, [], _) -> to_doc s
    | Tnewtype (s, tyl, _) -> to_doc s ^^ list "<" k tyl ">"
    | Tdependent (dep, cstr) ->
      let cstr_info =
        if !debug_mode then
          Concat [Space; text "as"; Space; k cstr]
        else
          Nothing
      in
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
    | Tpu (base, (_, enum)) -> pu_concat k base enum
    | Tpu_type_access (base, (_, enum), (_, member), (_, tyname)) ->
      k base
      ^^ text (":@" ^ enum)
      ^^ text (":@" ^ member)
      ^^ text (":@" ^ tyname)

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

  (* For a given type parameter, construct a list of its constraints *)
  let get_constraints_on_tparam env tparam =
    let lower = Env.get_lower_bounds env tparam in
    let upper = Env.get_upper_bounds env tparam in
    let equ = Env.get_equal_bounds env tparam in
    (* If we have an equality we can ignore the other bounds *)
    if not (TySet.is_empty equ) then
      List.map (TySet.elements equ) (fun ty ->
          (tparam, Ast_defs.Constraint_eq, ty))
    else
      List.map (TySet.elements lower) (fun ty ->
          (tparam, Ast_defs.Constraint_super, ty))
      @ List.map (TySet.elements upper) (fun ty ->
            (tparam, Ast_defs.Constraint_as, ty))

  let to_string ~ty to_doc env x =
    ty to_doc ISet.empty env x
    |> Libhackfmt.format_doc_unbroken format_env
    |> String.strip

  let constraints_for_type to_doc env typ =
    let tparams = SSet.elements (Env.get_tparams env typ) in
    let constraints = List.concat_map tparams (get_constraints_on_tparam env) in
    if List.is_empty constraints then
      None
    else
      Some
        (Concat
           [
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
           ])

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
        | ({ type_ = Class; name; _ }, _) ->
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
        | ({ type_ = ClassConst _; name; _ }, _)
        | ({ type_ = GConst; name; _ }, _) ->
          Concat [ty text_strip_ns ISet.empty env x; Space; text_strip_ns name]
        | _ -> ty text_strip_ns ISet.empty env x)
    in
    let constraints =
      constraints_for_type text_strip_ns env x
      |> Option.value_map ~default:Nothing ~f:(fun x -> Concat [Newline; x])
    in
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
    | Nast.Tatom s -> "a PU atom " ^ s

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
    | Tarraykind (AKvarray_or_darray _) -> varray_or_darray
    | Tarraykind (AKvarray _) -> varray
    | Tarraykind (AKdarray (_, _)) -> darray
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
    | Tgeneric s when DependentKind.is_generic_dep_ty s ->
      "the expression dependent type " ^ s
    | Tgeneric x -> "a value of generic type " ^ x
    | Tnewtype (x, _, _) when String.equal x SN.Classes.cClassname ->
      "a classname string"
    | Tnewtype (x, _, _) when String.equal x SN.Classes.cTypename ->
      "a typename string"
    | Tnewtype (x, tyl, _) -> "a value of type " ^ strip_ns x ^ inst env tyl
    | Tdependent (dep, cstr) -> dependent env dep cstr
    | Tclass ((_, x), Exact, tyl) ->
      "an object of exactly the class " ^ strip_ns x ^ inst env tyl
    | Tclass ((_, x), Nonexact, tyl) ->
      "an object of type " ^ strip_ns x ^ inst env tyl
    | Tobject -> "an object"
    | Tshape _ -> "a shape"
    | Tpu (ty, (_, enum)) ->
      let ty =
        match get_node ty with
        | Tclass ((_, x), _, tyl) -> strip_ns x ^ inst env tyl
        | _ -> "..."
      in
      "the pocket universe " ^ ty ^ ":@" ^ enum
    | Tpu_type_access (ty, (_, enum), _, (_, tyname)) ->
      let ty =
        match get_node ty with
        | Tclass ((_, x), _, tyl) -> strip_ns x ^ inst env tyl
        | _ -> "..."
      in
      "the type "
      ^ tyname
      ^ " associated with a member of the pocket universe "
      ^ ty
      ^ ":@"
      ^ enum

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

  and dependent env dep cstr =
    let x = strip_ns @@ DependentKind.to_string dep in
    match dep with
    | DTcls c ->
      to_string env cstr
      ^ " (known to be exactly the class '"
      ^ strip_ns c
      ^ "')"
    | DTthis
    | DTexpr _ ->
      "the expression dependent type " ^ x

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
    | Nast.Tatom s -> s

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
    let kind p k =
      [("src_pos", Pos.json (Pos.to_absolute p)); ("kind", JSON_String k)]
    in
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
        | Ast_defs.SFlit_int (_, s) -> Hh_json.JSON_Number s
        | Ast_defs.SFlit_str (_, s) -> Hh_json.JSON_String s
        | Ast_defs.SFclass_const ((_, s1), (_, s2)) ->
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
    | (p, Tgeneric s) -> obj @@ kind p "generic" @ is_array true @ name s
    | (p, Tnewtype (s, _, ty)) when Typing_env.is_enum env s ->
      obj @@ kind p "enum" @ name s @ as_type ty
    | (p, Tnewtype (s, tys, ty)) ->
      obj @@ kind p "newtype" @ name s @ args tys @ as_type ty
    | (p, Tdependent (DTcls c, ty)) ->
      obj
      @@ kind p "path"
      @ [("type", obj @@ kind (get_pos ty) "class" @ name c @ args [])]
      @ as_type ty
    | (p, Tdependent (DTexpr _, ty)) ->
      obj
      @@ kind p "path"
      @ [("type", obj @@ kind (get_pos ty) "expr")]
      @ as_type ty
    | (p, Tdependent (DTthis, ty)) ->
      obj
      @@ kind p "path"
      @ [("type", obj @@ kind (get_pos ty) "this")]
      @ as_type ty
    | (p, Toption ty) ->
      begin
        match get_node ty with
        | Tnonnull -> obj @@ kind p "mixed"
        | _ -> obj @@ kind p "nullable" @ args [ty]
      end
    | (p, Tprim tp) -> obj @@ kind p "primitive" @ name (prim tp)
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
      @ fields (Nast.ShapeMap.bindings fl)
    | (p, Tunion []) -> obj @@ kind p "nothing"
    | (_, Tunion [ty]) -> from_type env ty
    | (p, Tunion tyl) -> obj @@ kind p "union" @ args tyl
    | (p, Tintersection []) -> obj @@ kind p "mixed"
    | (_, Tintersection [ty]) -> from_type env ty
    | (p, Tintersection tyl) -> obj @@ kind p "intersection" @ args tyl
    | (p, Tfun ft) ->
      let fun_kind p =
        if get_ft_is_coroutine ft then
          kind p "coroutine"
        else
          kind p "function"
      in
      let callconv cc =
        [("callConvention", JSON_String (param_mode_to_string cc))]
      in
      let param fp = obj @@ callconv fp.fp_kind @ typ fp.fp_type.et_type in
      let params fps = [("params", JSON_Array (List.map fps param))] in
      obj @@ fun_kind p @ params ft.ft_params @ result ft.ft_ret.et_type
    | (p, Tarraykind (AKvarray_or_darray (ty1, ty2))) ->
      obj @@ kind p "varray_or_darray" @ args [ty1; ty2]
    | (p, Tarraykind (AKdarray (ty1, ty2))) ->
      obj @@ kind p "darray" @ args [ty1; ty2]
    | (p, Tarraykind (AKvarray ty)) -> obj @@ kind p "varray" @ args [ty]
    | (p, Tpu (base, enum)) ->
      obj @@ kind p "pocket_universe" @ args [base] @ name (snd enum)
    | (p, Tpu_type_access (base, enum, member, typ)) ->
      obj
      @@ kind p "pocket_universe_type_access"
      @ args [base]
      @ name (snd member)
      @ name (snd enum)
      @ name (snd typ)

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
          if is_array then
            ty (Tgeneric name)
          else
            wrong_phase ~message:"Tgeneric is a decl-phase type." ~keytrace
        | "enum" ->
          get_string "name" (json, keytrace) >>= fun (name, _name_keytrace) ->
          aux_as json ~keytrace >>= fun as_ty -> ty (Tnewtype (name, [], as_ty))
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
            | "class" ->
              get_string "name" (type_json, type_keytrace)
              >>= fun (class_name, _class_name_keytrace) ->
              aux_as json ~keytrace >>= fun as_ty ->
              ty (Tdependent (DTcls class_name, as_ty))
            | "expr" ->
              not_supported
                ~message:
                  "Cannot deserialize path-dependent type involving an expression"
                ~keytrace
            | "this" ->
              aux_as json ~keytrace >>= fun as_ty ->
              ty (Tdependent (DTthis, as_ty))
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
              ty (Tarraykind (AKdarray (ty1, ty2)))
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
              ty (Tarraykind (AKvarray ty1))
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
              ty (Tarraykind (AKvarray_or_darray (ty1, ty2)))
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
              ty (Tarraykind (AKvarray_or_darray (tany, tany)))
            | [ty1] ->
              aux ty1 ~keytrace:("0" :: keytrace) >>= fun ty1 ->
              ty (Tarraykind (AKvarray ty1))
            | [ty1; ty2] ->
              aux ty1 ~keytrace:("0" :: keytrace) >>= fun ty1 ->
              aux ty2 ~keytrace:("1" :: keytrace) >>= fun ty2 ->
              ty (Tarraykind (AKdarray (ty1, ty2)))
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
              Pos.none
          in
          get_array "args" (json, keytrace) >>= fun (args, _args_keytrace) ->
          aux_args args ~keytrace >>= fun tyl ->
          (* NB: "class" could have come from either a `Tapply` or a `Tclass`. Right
      now, we always return a `Tclass`. *)
          ty (Tclass ((class_pos, name), Nonexact, tyl))
        | "object" -> ty Tobject
        | "shape" ->
          get_array "fields" (json, keytrace)
          >>= fun (fields, fields_keytrace) ->
          get_bool "is_array" (json, keytrace)
          >>= fun (is_array, _is_array_keytrace) ->
          let unserialize_field field_json ~keytrace :
              ( Ast_defs.shape_field_name
                * locl_phase Typing_defs.shape_field_type,
                deserialization_error )
              result =
            get_val "name" (field_json, keytrace)
            >>= fun (name, name_keytrace) ->
            (* We don't need position information for shape field names. They're
        only used for error messages and the like. *)
            let dummy_pos = Pos.none in
            begin
              match name with
              | Hh_json.JSON_Number name ->
                Ok (Ast_defs.SFlit_int (dummy_pos, name))
              | Hh_json.JSON_String name ->
                Ok (Ast_defs.SFlit_str (dummy_pos, name))
              | Hh_json.JSON_Array
                  [Hh_json.JSON_String name1; Hh_json.JSON_String name2] ->
                Ok
                  (Ast_defs.SFclass_const
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
        arrays. We're missing the keys in the shape map of the shape fields. *)
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
              List.fold
                fields
                ~init:Nast.ShapeMap.empty
                ~f:(fun shape_map (k, v) -> Nast.ShapeMap.add k v shape_map)
            in
            ty (Tshape (shape_kind, fields))
        | "union" ->
          get_array "args" (json, keytrace) >>= fun (args, keytrace) ->
          aux_args args ~keytrace >>= fun tyl -> ty (Tunion tyl)
        | "intersection" ->
          get_array "args" (json, keytrace) >>= fun (args, keytrace) ->
          aux_args args ~keytrace >>= fun tyl -> ty (Tintersection tyl)
        | ("function" | "coroutine") as kind ->
          let _ft_is_coroutine = String.equal kind "coroutine" in
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
                    fp_type = { et_type = param_type; et_enforced = false };
                    fp_kind = callconv;
                    (* Dummy values: these aren't currently serialized. *)
                    fp_pos = Pos.none;
                    fp_name = None;
                    fp_accept_disposable = false;
                    fp_mutability = None;
                    fp_rx_annotation = None;
                  })
          in
          params >>= fun ft_params ->
          get_obj "result" (json, keytrace) >>= fun (result, result_keytrace) ->
          aux result ~keytrace:result_keytrace >>= fun ft_ret ->
          ty
            (Tfun
               {
                 ft_params;
                 ft_ret = { et_type = ft_ret; et_enforced = false };
                 (* Dummy values: these aren't currently serialized. *)
                 ft_arity = Fstandard 0;
                 ft_tparams = [];
                 ft_where_constraints = [];
                 ft_flags = 0;
                 ft_reactive = Nonreactive;
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

  let sseq s =
    let contents = Sequence.fold s ~init:"" ~f:(fun acc x -> x ^ " " ^ acc) in
    Printf.sprintf "Seq( %s)" contents

  let pos p =
    let (line, start, end_) = Pos.info_pos p in
    Printf.sprintf "(line %d: chars %d-%d)" line start end_

  let class_kind = function
    | Ast_defs.Cabstract -> "Cabstract"
    | Ast_defs.Cnormal -> "Cnormal"
    | Ast_defs.Cinterface -> "Cinterface"
    | Ast_defs.Ctrait -> "Ctrait"
    | Ast_defs.Cenum -> "Cenum"

  let constraint_ty tcopt = function
    | (Ast_defs.Constraint_as, ty) -> "as " ^ Full.to_string_decl tcopt ty
    | (Ast_defs.Constraint_eq, ty) -> "= " ^ Full.to_string_decl tcopt ty
    | (Ast_defs.Constraint_super, ty) -> "super " ^ Full.to_string_decl tcopt ty

  let variance = function
    | Ast_defs.Covariant -> "+"
    | Ast_defs.Contravariant -> "-"
    | Ast_defs.Invariant -> ""

  let tparam
      tcopt
      {
        tp_variance = var;
        tp_name = (position, name);
        tp_constraints = cstrl;
        tp_reified = reified;
        tp_user_attributes = _;
      } =
    variance var
    ^ pos position
    ^ " "
    ^ name
    ^ " "
    ^ List.fold_right
        cstrl
        ~f:(fun x acc -> constraint_ty tcopt x ^ " " ^ acc)
        ~init:""
    ^
    match reified with
    | Nast.Erased -> ""
    | Nast.SoftReified -> " soft reified"
    | Nast.Reified -> " reified"

  let tparam_list ctx l =
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

  let class_elts tcopt m =
    List.fold m ~init:"" ~f:(fun acc (field, v) ->
        "(" ^ field ^ ": " ^ class_elt tcopt v ^ ") " ^ acc)

  let class_elts_with_breaks tcopt m =
    List.fold m ~init:"" ~f:(fun acc (field, v) ->
        "\n" ^ indent ^ field ^ ": " ^ class_elt tcopt v ^ acc)

  let class_consts tcopt m =
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
        ^ Full.to_string_decl tcopt cc.cc_type
        ^ ") "
        ^ acc)

  let typeconst
      tcopt
      {
        ttc_abstract = _;
        ttc_name = tc_name;
        ttc_constraint = tc_constraint;
        ttc_type = tc_type;
        ttc_origin = origin;
        ttc_enforceable = (_, enforceable);
        ttc_reifiable = reifiable;
      } =
    let name = snd tc_name in
    let ty x = Full.to_string_decl tcopt x in
    let constraint_ =
      match tc_constraint with
      | None -> ""
      | Some x -> " as " ^ ty x
    in
    let type_ =
      match tc_type with
      | None -> ""
      | Some x -> " = " ^ ty x
    in
    name
    ^ constraint_
    ^ type_
    ^ " (origin:"
    ^ origin
    ^ ")"
    ^ ( if enforceable then
        " (enforceable)"
      else
        "" )
    ^
    if Option.is_some reifiable then
      " (reifiable)"
    else
      ""

  let typeconsts tcopt m =
    List.fold m ~init:"" ~f:(fun acc (_, v) ->
        "\n(" ^ typeconst tcopt v ^ ")" ^ acc)

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

  let constructor tcopt (ce_opt, consist) =
    let consist_str =
      Format.asprintf "(%a)" Pp_type.pp_consistent_kind consist
    in
    let ce_str =
      match ce_opt with
      | None -> ""
      | Some ce -> class_elt tcopt ce
    in
    ce_str ^ consist_str

  let req_ancestors tcopt xs =
    List.fold xs ~init:"" ~f:(fun acc (_p, x) ->
        acc ^ Full.to_string_decl tcopt x ^ ", ")

  let class_type ctx c =
    let tenv = Typing_env.empty ctx (Pos.filename (Cls.pos c)) None in
    let tc_need_init = bool (Cls.need_init c) in
    let tc_members_fully_known = bool (Cls.members_fully_known c) in
    let tc_abstract = bool (Cls.abstract c) in
    let tc_deferred_init_members =
      sset
      @@
      if shallow_decl_enabled ctx then
        match Shallow_classes_provider.get ctx (Cls.name c) with
        | Some cls -> Typing_deferred_members.class_ tenv cls
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
    let tc_req_ancestors_extends = sseq (Cls.all_ancestor_req_names c) in
    let tc_extends = sseq (Cls.all_extends_ancestors c) in
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
  let typedef tcopt = function
    | {
        td_pos;
        td_vis = _;
        td_tparams;
        td_constraint;
        td_type;
        td_decl_errors = _;
      } ->
      let tparaml_s = PrintClass.tparam_list tcopt td_tparams in
      let constr_s =
        match td_constraint with
        | None -> "[None]"
        | Some constr -> Full.to_string_decl tcopt constr
      in
      let ty_s = Full.to_string_decl tcopt td_type in
      let pos_s = PrintClass.pos td_pos in
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

let gconst ctx gc = Full.to_string_decl ctx (fst gc)

let fun_ ctx { fe_type; _ } = Full.to_string_decl ctx fe_type

let fun_type ctx f = Full.fun_to_string ctx f

let typedef ctx td = PrintTypedef.typedef ctx td

let constraints_for_type env ty =
  Full.constraints_for_type Doc.text env ty
  |> Option.map ~f:(Libhackfmt.format_doc_unbroken Full.format_env)
  |> Option.map ~f:String.strip

let class_kind c_kind final = ErrorString.class_kind c_kind final

let subtype_prop env prop =
  let rec subtype_prop = function
    | Conj [] -> "TRUE"
    | Conj ps ->
      "(" ^ String.concat ~sep:" && " (List.map ~f:subtype_prop ps) ^ ")"
    | Disj (_, []) -> "FALSE"
    | Disj (_, ps) ->
      "(" ^ String.concat ~sep:" || " (List.map ~f:subtype_prop ps) ^ ")"
    | IsSubtype (ty1, ty2) -> debug_i env ty1 ^ " <: " ^ debug_i env ty2
    | Coerce (ty1, ty2) -> debug env ty1 ^ " ~> " ^ debug env ty2
  in
  let p_str = subtype_prop prop in
  p_str
