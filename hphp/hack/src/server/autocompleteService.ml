(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
open Utils
open String_utils
open SearchUtils
include AutocompleteTypes
open Tast
module Tast = Aast
module Phase = Typing_phase
module Cls = Decl_provider.Class

let ac_env = ref None

let autocomplete_results : autocomplete_result list ref = ref []

let autocomplete_is_complete : bool ref = ref true

(* The position we're autocompleting at. This is used when computing completions
 * for global identifiers. *)
let autocomplete_identifier : (Pos.t * string) option ref = ref None

(*
 * Take the results, look them up, and add file position information.
 *)
let add_position_to_results
    (ctx : Provider_context.t) (raw_results : SearchUtils.si_results) :
    SearchUtils.result =
  SearchUtils.(
    List.filter_map raw_results ~f:(fun r ->
        match SymbolIndexCore.get_pos_for_item_opt ctx r with
        | Some pos ->
          Some { name = r.si_fullname; pos; result_type = r.si_kind }
        | None -> None))

let (argument_global_type : autocomplete_type option ref) = ref None

let auto_complete_for_global = ref ""

let strip_suffix s =
  String.sub s 0 (String.length s - AutocompleteTypes.autocomplete_token_length)

let matches_auto_complete_suffix x =
  String.length x >= AutocompleteTypes.autocomplete_token_length
  &&
  let suffix =
    String.sub
      x
      (String.length x - AutocompleteTypes.autocomplete_token_length)
      AutocompleteTypes.autocomplete_token_length
  in
  String.equal suffix AutocompleteTypes.autocomplete_token

let is_auto_complete x =
  if List.is_empty !autocomplete_results then
    matches_auto_complete_suffix x
  else
    false

let get_replace_pos_exn () =
  match !autocomplete_identifier with
  | None -> failwith "No autocomplete position was set."
  | Some (pos, text) ->
    let (_, name) = Utils.split_ns_from_name text in
    let name =
      if matches_auto_complete_suffix name then
        strip_suffix name
      else
        name
    in
    Ide_api_types.(
      let range = pos_to_range pos in
      let ed =
        {
          range.ed with
          column = range.ed.column - AutocompleteTypes.autocomplete_token_length;
        }
      in
      let st = { range.st with column = ed.column - String.length name } in
      { st; ed })

let autocomplete_result_to_json res =
  let func_param_to_json param =
    Hh_json.JSON_Object
      [
        ("name", Hh_json.JSON_String param.param_name);
        ("type", Hh_json.JSON_String param.param_ty);
        ("variadic", Hh_json.JSON_Bool param.param_variadic);
      ]
  in
  let func_details_to_json details =
    match details with
    | Some fd ->
      Hh_json.JSON_Object
        [
          ("min_arity", Hh_json.int_ fd.min_arity);
          ("return_type", Hh_json.JSON_String fd.return_ty);
          ("params", Hh_json.JSON_Array (List.map fd.params func_param_to_json));
        ]
    | None -> Hh_json.JSON_Null
  in
  let name = res.res_name in
  let pos = res.res_pos in
  let ty = res.res_ty in
  Hh_json.JSON_Object
    [
      ("name", Hh_json.JSON_String name);
      ("type", Hh_json.JSON_String ty);
      ("pos", Pos.json pos);
      ("func_details", func_details_to_json res.func_details);
      ("expected_ty", Hh_json.JSON_Bool false);
      (* legacy field, left here in case clients need it *)
    ]

let get_partial_result name ty kind class_opt =
  let base_class = Option.map ~f:(fun class_ -> Cls.name class_) class_opt in
  Partial { ty; name; kind_ = kind; base_class }

let add_res (res : autocomplete_result) : unit =
  autocomplete_results := res :: !autocomplete_results

let add_partial_result name ty kind class_opt =
  add_res (get_partial_result name ty kind class_opt)

let autocomplete_token ac_type env x =
  if is_auto_complete (snd x) then (
    ac_env := env;
    autocomplete_identifier := Some x;
    argument_global_type := Some ac_type;
    auto_complete_for_global := snd x
  )

let autocomplete_id id env = autocomplete_token Acid (Some env) id

let autocomplete_hint = autocomplete_token Actype None

let autocomplete_trait_only = autocomplete_token Actrait_only None

let autocomplete_new cid env =
  match cid with
  | Aast.CI sid -> autocomplete_token Acnew (Some env) sid
  | _ -> ()

let get_class_elt_types env class_ cid elts =
  let is_visible (_, elt) =
    Tast_env.is_visible env (elt.ce_visibility, get_ce_lsb elt) cid class_
  in
  elts
  |> List.filter ~f:is_visible
  |> List.map ~f:(fun (id, { ce_type = (lazy ty); _ }) -> (id, ty))

let autocomplete_shape_key env fields id =
  if is_auto_complete (snd id) then (
    ac_env := Some env;
    autocomplete_identifier := Some id;
    argument_global_type := Some Acshape_key;

    (* not the same as `prefix == ""` in namespaces *)
    let have_prefix =
      Pos.length (fst id) > AutocompleteTypes.autocomplete_token_length
    in
    let prefix = strip_suffix (snd id) in
    let add (name : Typing_defs.tshape_field_name) =
      let (code, kind, ty) =
        match name with
        | Typing_defs.TSFlit_int (pos, str) ->
          let reason = Typing_reason.Rwitness_from_decl pos in
          let ty = Typing_defs.Tprim Aast_defs.Tint in
          (str, SI_Literal, Typing_defs.mk (reason, ty))
        | Typing_defs.TSFlit_str (pos, str) ->
          let reason = Typing_reason.Rwitness_from_decl pos in
          let ty = Typing_defs.Tprim Aast_defs.Tstring in
          let quote =
            if have_prefix then
              Str.first_chars prefix 1
            else
              "'"
          in
          (quote ^ str ^ quote, SI_Literal, Typing_defs.mk (reason, ty))
        | Typing_defs.TSFclass_const ((pos, cid), (_, mid)) ->
          ( Printf.sprintf "%s::%s" cid mid,
            SI_ClassConstant,
            Typing_defs.mk
              (Reason.Rwitness_from_decl pos, Typing_defs.make_tany ()) )
      in
      if (not have_prefix) || string_starts_with code prefix then
        add_partial_result code (Phase.decl ty) kind None
    in
    List.iter (TShapeMap.keys fields) ~f:add
  )

let autocomplete_member ~is_static env class_ cid id =
  (* This is used for instance "$x->|" and static "Class1::|" members. *)
  if is_auto_complete (snd id) then (
    (* Detect usage of "parent::|" which can use both static and instance *)
    let match_both_static_and_instance =
      match cid with
      | Some Aast.CIparent -> true
      | _ -> false
    in
    ac_env := Some env;
    autocomplete_identifier := Some id;
    argument_global_type := Some Acclass_get;
    let add kind (name, ty) =
      add_partial_result name (Phase.decl ty) kind (Some class_)
    in
    let sort : 'a. (string * 'a) list -> (string * 'a) list =
     fun list ->
      List.sort ~compare:(fun (a, _) (b, _) -> String.compare a b) list
    in
    (* There's no reason for us to sort -- we can expect our client to do its
    own sorting of our results -- but having a sorted list here makes our tests
    more stable. *)
    if is_static || match_both_static_and_instance then (
      List.iter
        (get_class_elt_types env class_ cid (Cls.smethods class_ |> sort))
        ~f:(add SearchUtils.SI_ClassMethod);
      List.iter
        (get_class_elt_types env class_ cid (Cls.sprops class_ |> sort))
        ~f:(add SearchUtils.SI_Property);
      List.iter
        (Cls.consts class_ |> sort)
        ~f:(fun (name, cc) ->
          add SearchUtils.SI_ClassConstant (name, cc.cc_type))
    );
    if (not is_static) || match_both_static_and_instance then (
      List.iter
        (get_class_elt_types env class_ cid (Cls.methods class_ |> sort))
        ~f:(add SearchUtils.SI_ClassMethod);
      List.iter
        (get_class_elt_types env class_ cid (Cls.props class_ |> sort))
        ~f:(add SearchUtils.SI_Property)
    )
  )

let autocomplete_xhp_attributes env class_ cid id =
  (* This is used for "<nt:fb:text |" XHP attributes, in which case  *)
  (* class_ is ":nt:fb:text" and its attributes are in tc_props.     *)
  if is_auto_complete (snd id) && Cls.is_xhp class_ then (
    ac_env := Some env;
    autocomplete_identifier := Some id;
    argument_global_type := Some Acprop;
    List.iter
      (get_class_elt_types env class_ cid (Cls.props class_))
      ~f:(fun (name, ty) ->
        add_partial_result
          name
          (Phase.decl ty)
          SearchUtils.SI_Property
          (Some class_))
  )

let autocomplete_xhp_bool_value attr_ty id_id env =
  if is_auto_complete (snd id_id) then begin
    ac_env := Some env;
    argument_global_type := Some Acprop;
    autocomplete_identifier := Some id_id;

    let is_bool_or_bool_option ty : bool =
      let is_bool = function
        | Tprim Tbool -> true
        | _ -> false
      in
      let (_, ty_) = Typing_defs_core.deref ty in
      match ty_ with
      | Toption ty -> is_bool (get_node ty)
      | _ -> is_bool ty_
    in

    if is_bool_or_bool_option attr_ty then (
      add_partial_result "true" (Phase.locl attr_ty) SearchUtils.SI_Literal None;
      add_partial_result
        "false"
        (Phase.locl attr_ty)
        SearchUtils.SI_Literal
        None
    )
  end

let autocomplete_xhp_enum_value attr_ty id_id env =
  if is_auto_complete (snd id_id) then begin
    ac_env := Some env;
    argument_global_type := Some Acprop;
    autocomplete_identifier := Some id_id;

    let get_class_name ty : string option =
      let get_name : type phase. phase Typing_defs.ty_ -> _ = function
        | Tnewtype (name, _, _) -> Some name
        | Tapply ((_, name), _) -> Some name
        | _ -> None
      in
      let (_, ty_) = Typing_defs_core.deref ty in
      match ty_ with
      | Toption ty -> get_name (get_node ty)
      | _ -> get_name ty_
    in

    let get_enum_constants (class_decl : Decl_provider.class_decl) :
        (string * Typing_defs.class_const) list =
      let all_consts = Cls.consts class_decl in
      let is_correct_class = function
        | Some name -> String.equal name (Cls.name class_decl)
        | None -> false
      in
      all_consts
      |> List.filter ~f:(fun (_, class_const) ->
             is_correct_class (get_class_name class_const.cc_type))
    in

    let attr_type_name = get_class_name attr_ty in

    attr_type_name
    |> Option.iter ~f:(fun class_name ->
           let enum_class = Tast_env.get_enum env class_name in

           let enum_constants =
             Option.value
               ~default:[]
               (Option.map enum_class ~f:get_enum_constants)
           in

           enum_constants
           |> List.iter ~f:(fun (const_name, ty) ->
                  add_partial_result
                    (Utils.strip_ns class_name ^ "::" ^ const_name)
                    (Phase.decl ty.cc_type)
                    SearchUtils.SI_Enum
                    enum_class))
  end

let autocomplete_lvar id env =
  (* This is used for "$|" and "$x = $|" local variables. *)
  let text = Local_id.get_name (snd id) in
  if is_auto_complete text then (
    argument_global_type := Some Acprop;
    ac_env := Some env;
    autocomplete_identifier := Some (fst id, text)
  )

(* Print a descriptive "detail" element *)
let get_desc_string_for env ty kind =
  match kind with
  | SearchUtils.SI_ClassMethod
  | SearchUtils.SI_ClassConstant
  | SearchUtils.SI_Property
  | SearchUtils.SI_Function
  | SearchUtils.SI_Constructor ->
    let result =
      match ty with
      | DeclTy declt -> Tast_env.print_decl_ty env declt
      | LoclTy loclt -> Tast_env.print_ty env loclt
    in
    result
  | _ -> kind_to_string kind

(* Convert a `TFun` into a func details structure *)
let tfun_to_func_details (env : Tast_env.t) (ft : Typing_defs.locl_fun_type) :
    func_details_result =
  let param_to_record ?(is_variadic = false) param =
    {
      param_name =
        (match param.fp_name with
        | Some n -> n
        | None -> "");
      param_ty = Tast_env.print_ty env param.fp_type.et_type;
      param_variadic = is_variadic;
    }
  in
  {
    return_ty = Tast_env.print_ty env ft.ft_ret.et_type;
    min_arity = arity_min ft;
    params =
      ( List.map ft.ft_params param_to_record
      @
      match ft.ft_arity with
      | Fvariadic p -> [param_to_record ~is_variadic:true p]
      | Fstandard -> [] );
  }

(* Convert a `ty` into a func details structure *)
let get_func_details_for env ty =
  let (env, ty) =
    match ty with
    | DeclTy ty -> Tast_env.localize_with_self env ~ignore_errors:true ty
    | LoclTy ty -> (env, ty)
  in
  match Typing_defs.get_node ty with
  | Tfun ft -> Some (tfun_to_func_details env ft)
  | _ -> None

(* Here we turn partial_autocomplete_results into complete_autocomplete_results *)
(* by using typing environment to convert ty information into strings. *)
let resolve_ty
    (env : Tast_env.t)
    (autocomplete_context : legacy_autocomplete_context)
    (x : partial_autocomplete_result)
    (replace_pos : Ide_api_types.range) : complete_autocomplete_result =
  (* XHP class+attribute names are stored internally with a leading colon.    *)
  (* We'll render them without it if and only if we're in an XHP context.     *)
  (*   $x = new :class1() -- do strip the colon in front of :class1           *)
  (*   $x = <:class1      -- don't strip it                                   *)
  (*   $x->:attr1         -- don't strip the colon in front of :attr1         *)
  (*   <class1 :attr="a"  -- do strip it                                      *)
  (* The logic is thorny here because we're relying upon regexes to figure    *)
  (* out the context. Once we switch autocomplete to FFP, it'll be cleaner.   *)
  let (res_name, res_fullname) =
    match (x.kind_, autocomplete_context, !argument_global_type) with
    | ( SearchUtils.SI_Property,
        { AutocompleteTypes.is_instance_member = false; _ },
        _ )
    | (SearchUtils.SI_XHP, { AutocompleteTypes.is_xhp_classname = true; _ }, _)
    | (SearchUtils.SI_Class, { AutocompleteTypes.is_xhp_classname = true; _ }, _)
      ->
      let newname = lstrip x.name ":" in
      (newname, x.name)
    | ( SearchUtils.SI_Literal,
        { AutocompleteTypes.is_before_apostrophe = true; _ },
        Some Acshape_key ) ->
      let n = rstrip x.name "'" in
      (n, x.name)
    | _ -> (x.name, x.name)
  in
  let pos =
    match x.ty with
    | LoclTy loclt ->
      loclt |> Typing_defs.get_pos |> ServerPos.resolve env |> Pos.to_absolute
    | DeclTy declt ->
      declt |> Typing_defs.get_pos |> ServerPos.resolve env |> Pos.to_absolute
  in
  {
    res_pos = pos;
    res_replace_pos = replace_pos;
    res_base_class = x.base_class;
    res_ty = get_desc_string_for env x.ty x.kind_;
    res_name;
    res_fullname;
    res_kind = x.kind_;
    func_details = get_func_details_for env x.ty;
    ranking_details = None;
  }

let autocomplete_typed_member ~is_static env class_ty cid mid =
  Tast_env.get_class_ids env class_ty
  |> List.iter ~f:(fun cname ->
         Decl_provider.get_class (Tast_env.get_ctx env) cname
         |> Option.iter ~f:(fun class_ ->
                let cid = Option.map cid to_nast_class_id_ in
                autocomplete_member ~is_static env class_ cid mid))

let autocomplete_static_member env ((_, ty), cid) mid =
  autocomplete_typed_member ~is_static:true env ty (Some cid) mid

let autocomplete_enum_atom env f pos_atomname =
  ac_env := Some env;
  autocomplete_identifier := Some pos_atomname;
  argument_global_type := Some Acclass_get;
  let suggest_members cls =
    match cls with
    | Some cls ->
      List.iter (Cls.consts cls) ~f:(fun (name, cc) ->
          (* Filter out the constant used for ::class if present *)
          if String.(name <> Naming_special_names.Members.mClass) then
            add_partial_result
              name
              (Phase.decl cc.cc_type)
              SearchUtils.SI_ClassConstant
              (Some cls))
    | _ -> ()
  in
  let suggest_members_from_ty env ty =
    match get_node ty with
    | Tclass ((_, enum_name), _, _) when Tast_env.is_enum_class env enum_name ->
      suggest_members (Tast_env.get_class env enum_name)
    | _ -> ()
  in

  let (_, ty) = fst f in
  let open Typing_defs in
  match get_node ty with
  | Tfun { ft_params = { fp_type = { et_type = t; _ }; fp_flags; _ } :: _; _ }
    ->
    let is_enum_atom_ty_name name =
      Typing_defs_flags.(is_set fp_flags_atom fp_flags)
      && String.equal Naming_special_names.Classes.cMemberOf name
      || String.equal Naming_special_names.Classes.cLabel name
    in
    (match get_node t with
    | Tnewtype (ty_name, [enum_ty; _member_ty], _)
      when is_enum_atom_ty_name ty_name ->
      suggest_members_from_ty env enum_ty
    | _ -> ())
  | _ -> ()

let visitor =
  object (self)
    inherit Tast_visitor.iter as super

    method! on_Id env id =
      autocomplete_id id env;
      super#on_Id env id

    method! on_Call env f targs args unpack_arg =
      (match args with
      | ((p, _), Tast.EnumAtom n) :: _ when is_auto_complete n ->
        autocomplete_enum_atom env f (p, n)
      | _ -> ());

      super#on_Call env f targs args unpack_arg

    method! on_Fun_id env id =
      autocomplete_id id env;
      super#on_Fun_id env id

    method! on_New env cid el unpacked_element =
      autocomplete_new (to_nast_class_id_ (snd cid)) env;
      super#on_New env cid el unpacked_element

    method! on_Happly env sid hl =
      autocomplete_hint sid;
      super#on_Happly env sid hl

    method! on_Lvar env lid =
      autocomplete_lvar lid env;
      super#on_Lvar env lid

    method! on_Class_get env cid mid in_parens =
      match mid with
      | Tast.CGstring p -> autocomplete_static_member env cid p
      | Tast.CGexpr _ -> super#on_Class_get env cid mid in_parens

    method! on_Class_const env cid mid =
      autocomplete_static_member env cid mid;
      super#on_Class_const env cid mid

    method! on_Obj_get env obj mid ognf =
      (match mid with
      | (_, Tast.Id mid) ->
        autocomplete_typed_member ~is_static:false env (get_type obj) None mid
      | _ -> ());
      super#on_Obj_get env obj mid ognf

    method! on_expr env expr =
      (match expr with
      | (_, Tast.Array_get (arr, Some ((pos, _), key))) ->
        let ty = get_type arr in
        let (_, ty) = Tast_env.expand_type env ty in
        begin
          match Typing_defs.get_node ty with
          | Typing_defs.Tshape (_, fields) ->
            (match key with
            | Tast.Id (_, mid) -> autocomplete_shape_key env fields (pos, mid)
            | Tast.String mid ->
              (* autocomplete generally assumes that there's a token ending with the suffix; *)
              (* This isn't the case for `$shape['a`, unless it's at the end of the file *)
              let offset =
                String_utils.substring_index
                  AutocompleteTypes.autocomplete_token
                  mid
              in
              if Int.equal offset (-1) then
                autocomplete_shape_key env fields (pos, mid)
              else
                let mid =
                  Str.string_before
                    mid
                    (offset + AutocompleteTypes.autocomplete_token_length)
                in
                let (line, bol, cnum) = Pos.line_beg_offset pos in
                let pos =
                  Pos.make_from_lnum_bol_cnum
                    ~pos_file:(Pos.filename pos)
                    ~pos_start:(line, bol, cnum)
                    ~pos_end:
                      ( line,
                        bol,
                        cnum
                        + offset
                        + AutocompleteTypes.autocomplete_token_length )
                in
                autocomplete_shape_key env fields (pos, mid)
            | _ -> ())
          | _ -> ()
        end
      | _ -> ());
      super#on_expr env expr

    method! on_Xml env sid attrs el =
      (* In the case where we're autocompleting an open XHP bracket but haven't
       typed anything beyond that yet:

          $x = <

       we'll end up with the following AST:

          (Xml () () (:AUTO332))

       In order to handle this, we strip off the leading `:` if one exists and
       use that as the search term. *)
      let trimmed_sid = (fst sid, snd sid |> Utils.strip_both_ns) in
      autocomplete_id trimmed_sid env;
      let cid = Aast.CI sid in
      Decl_provider.get_class (Tast_env.get_ctx env) (snd sid)
      |> Option.iter ~f:(fun (c : Cls.t) ->
             List.iter attrs ~f:(function
                 | Tast.Xhp_simple
                     { Aast.xs_name = id; xs_expr = value; xs_type = ty } ->
                   (match value with
                   | (_, Tast.Id id_id) ->
                     (* This handles the situation
                          <foo:bar my-attribute={AUTO332}
                        *)
                     autocomplete_xhp_enum_value ty id_id env;
                     autocomplete_xhp_bool_value ty id_id env
                   | _ -> ());
                   if Cls.is_xhp c then
                     autocomplete_xhp_attributes env c (Some cid) id
                   else
                     autocomplete_member ~is_static:false env c (Some cid) id
                 | Tast.Xhp_spread _ -> ()));
      super#on_Xml env sid attrs el

    method! on_class_ env cls =
      List.iter cls.Tast.c_uses ~f:(fun hint ->
          match snd hint with
          | Aast.Happly (sid, params) ->
            autocomplete_trait_only sid;
            List.iter params (self#on_hint env)
          | _ -> ());

      (* If we don't clear out c_uses we'll end up overwriting the trait
       completion as soon as we get to on_Happly. *)
      super#on_class_ env { cls with Tast.c_uses = [] }
  end

let auto_complete_suffix_finder =
  object
    inherit [_] Tast.reduce

    method zero = false

    method plus = ( || )

    method! on_Lvar () (_, id) =
      matches_auto_complete_suffix (Local_id.get_name id)
  end

let method_contains_cursor = auto_complete_suffix_finder#on_method_ ()

let fun_contains_cursor = auto_complete_suffix_finder#on_fun_ ()

class local_types =
  object (self)
    inherit Tast_visitor.iter as super

    val mutable results = Local_id.Map.empty

    val mutable after_cursor = false

    method get_types ctx tast =
      self#go ctx tast;
      results

    method add id ty =
      (* If we already have a type for this identifier, don't overwrite it with
       results from after the cursor position. *)
      if not (Local_id.Map.mem id results && after_cursor) then
        results <- Local_id.Map.add id ty results

    method! on_fun_ env f = if fun_contains_cursor f then super#on_fun_ env f

    method! on_method_ env m =
      if method_contains_cursor m then (
        if not m.Tast.m_static then
          self#add Typing_defs.this (Tast_env.get_self_ty_exn env);
        super#on_method_ env m
      )

    method! on_expr env e =
      let ((_, ty), e_) = e in
      match e_ with
      | Tast.Lvar (_, id) ->
        if matches_auto_complete_suffix (Local_id.get_name id) then
          after_cursor <- true
        else
          self#add id ty
      | Tast.Binop (Ast_defs.Eq _, e1, e2) ->
        (* Process the rvalue before the lvalue, since the lvalue is annotated
         with its type after the assignment. *)
        self#on_expr env e2;
        self#on_expr env e1
      | _ -> super#on_expr env e

    method! on_fun_param _ fp =
      let id = Local_id.make_unscoped fp.Tast.param_name in
      let (_, ty) = fp.Tast.param_annotation in
      self#add id ty
  end

let compute_complete_local ctx tast =
  (new local_types)#get_types ctx tast
  |> Local_id.Map.iter (fun x ty ->
         add_partial_result
           (Local_id.get_name x)
           (Phase.locl ty)
           SearchUtils.SI_LocalVariable
           None)

let reset () =
  auto_complete_for_global := "";
  argument_global_type := None;
  autocomplete_identifier := None;
  ac_env := None;
  autocomplete_results := [];
  autocomplete_is_complete := true

(* Find global autocomplete results *)
let find_global_results
    ~(kind_filter : SearchUtils.si_kind option ref)
    ~(max_results : int)
    ~(completion_type : SearchUtils.autocomplete_type option)
    ~(autocomplete_context : AutocompleteTypes.legacy_autocomplete_context)
    ~(sienv : SearchUtils.si_env)
    ~(tast_env : Tast_env.t) : unit =
  (* First step: Check obvious cases where autocomplete is not warranted.   *)
  (*                                                                        *)
  (* is_after_single_colon : XHP vs switch statements                       *)
  (* is_after_open_square_bracket : shape field names vs container keys     *)
  (* is_after_quote: shape field names vs arbitrary strings                 *)
  (*                                                                        *)
  (* We can recognize these cases by whether the prefix is empty.           *)
  (* We do this by checking the identifier length, as the string will       *)
  (* include the current namespace.                                         *)
  let have_user_prefix =
    match !autocomplete_identifier with
    | None -> failwith "No autocomplete position was set"
    | Some (pos, _) ->
      Pos.length pos > AutocompleteTypes.autocomplete_token_length
  in
  let ctx = autocomplete_context in
  if
    (not ctx.is_manually_invoked)
    && (not have_user_prefix)
    && ( ctx.is_after_single_colon
       || ctx.is_after_open_square_bracket
       || ctx.is_after_quote )
  then
    ()
  else if ctx.is_after_double_right_angle_bracket then
    (* <<__Override>>AUTO332 *)
    ()
  else if ctx.is_open_curly_without_equals then
    (* In the case that we trigger autocompletion with an open curly brace,
    we only want to perform autocompletion if it is preceded by an equal sign.

    i.e. if (true) {
      --> Do not autocomplete

    i.e. <foo:bar my-attribute={
      --> Allow autocompletion
    *)
    ()
  else (
    (kind_filter :=
       match completion_type with
       | Some Acnew -> Some SI_Class
       | Some Actrait_only -> Some SI_Trait
       | _ -> None);
    let replace_pos = get_replace_pos_exn () in
    (* Ensure that we do not have a leading backslash for Hack classes,
     * while ensuring that we have colons for XHP classes.  That's how we
     * differentiate between them. *)
    let query_text = strip_suffix !auto_complete_for_global in
    let query_text =
      if autocomplete_context.is_xhp_classname then
        Utils.add_xhp_ns query_text
      else
        Utils.strip_ns query_text
    in
    auto_complete_for_global := query_text;
    let (ns, _) = Utils.split_ns_from_name query_text in
    let absolute_none = Pos.none |> Pos.to_absolute in
    let results =
      SymbolIndex.find_matching_symbols
        ~sienv
        ~query_text
        ~max_results
        ~kind_filter:!kind_filter
        ~context:completion_type
    in
    (* Looking up a function signature using Tast_env.get_fun consumes ~67KB
     * and can cause complex typechecking which can take from 2-100 milliseconds
     * per result.  When tested in summer 2019 it was possible to load 1.4GB of data
     * if get_fun was called on every function in the WWW codebase.
     *
     * Therefore, this feature is only available via the option sie_resolve_signatures
     * - and please be careful not to turn it on unless you really want to consume
     * memory and performance.
     *)
    List.iter results ~f:(fun r ->
        (* If we are autocompleting XHP using "$x = <" then the suggestions we
         * return should omit the leading colon *)
        let (res_name, res_fullname) =
          if autocomplete_context.is_xhp_classname then
            (Utils.strip_xhp_ns r.si_name, Utils.add_xhp_ns r.si_fullname)
          else
            (r.si_name, r.si_fullname)
        in
        (* Only load func details if the flag sie_resolve_signatures is true *)
        let (func_details, res_ty) =
          if sienv.sie_resolve_signatures && equal_si_kind r.si_kind SI_Function
          then
            let fixed_name = ns ^ r.si_name in
            match Tast_env.get_fun tast_env fixed_name with
            | None -> (None, kind_to_string r.si_kind)
            | Some fe ->
              let ty = fe.fe_type in
              let details = get_func_details_for tast_env (DeclTy ty) in
              let res_ty = Tast_env.print_decl_ty tast_env ty in
              (details, res_ty)
          else
            (None, kind_to_string r.si_kind)
        in
        (* Only load exact positions if specially requested *)
        let res_pos =
          if sienv.sie_resolve_positions then
            let fixed_name = ns ^ r.si_name in
            match Tast_env.get_fun tast_env fixed_name with
            | None -> absolute_none
            | Some fe ->
              Typing_defs.get_pos fe.fe_type
              |> ServerPos.resolve tast_env
              |> Pos.to_absolute
          else
            absolute_none
        in
        (* Figure out how to display them *)
        let complete =
          {
            res_pos;
            res_replace_pos = replace_pos;
            res_base_class = None;
            res_ty;
            res_name;
            res_fullname;
            res_kind = r.si_kind;
            func_details;
            ranking_details = None;
          }
        in
        add_res (Complete complete));
    autocomplete_is_complete := List.length !autocomplete_results < max_results
  )

(* Main entry point for autocomplete *)
let go_ctx
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(autocomplete_context : AutocompleteTypes.legacy_autocomplete_context)
    ~(sienv : SearchUtils.si_env) =
  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_quarantined ~ctx ~entry
  in
  reset ();
  visitor#go ctx tast;
  Errors.ignore_ (fun () ->
      let start_time = Unix.gettimeofday () in
      let max_results = 100 in
      let kind_filter = ref None in
      let tast_env =
        match !ac_env with
        | Some e -> e
        | None -> Tast_env.empty ctx
      in
      let completion_type = !argument_global_type in
      let ( = ) = Option.equal equal_autocomplete_type in
      if
        completion_type = Some Acid
        || completion_type = Some Acnew
        || completion_type = Some Actype
        || completion_type = Some Actrait_only
      then
        find_global_results
          ~kind_filter
          ~max_results
          ~completion_type
          ~autocomplete_context
          ~sienv
          ~tast_env;

      if completion_type = Some Acprop then compute_complete_local ctx tast;
      let replace_pos =
        try get_replace_pos_exn ()
        with _ -> Pos.none |> Pos.to_absolute |> Ide_api_types.pos_to_range
      in
      let resolve (result : autocomplete_result) : complete_autocomplete_result
          =
        match result with
        | Partial res ->
          resolve_ty tast_env autocomplete_context res replace_pos
        | Complete res -> res
      in
      let complete_autocomplete_results =
        !autocomplete_results |> List.map ~f:resolve
      in
      let results =
        {
          With_complete_flag.is_complete = !autocomplete_is_complete;
          value =
            ( if sienv.use_ranked_autocomplete then (
              let ranking_start_time = Unix.gettimeofday () in
              let ranked_results =
                AutocompleteRankService.rank_autocomplete_result
                  ~ctx
                  ~entry
                  ~query_text:!auto_complete_for_global
                  ~results:complete_autocomplete_results
                  ~max_results:3
                  ~context:completion_type
                  ~kind_filter:!kind_filter
                  ~replace_pos
              in
              AutocompleteRankService.log_ranked_autocomplete
                ~sienv
                ~results:(List.length complete_autocomplete_results)
                ~context:completion_type
                ~start_time:ranking_start_time;
              HackEventLogger.ranked_autocomplete_duration
                ~start_time:ranking_start_time;
              ranked_results
            ) else
              complete_autocomplete_results );
        }
      in
      SymbolIndexCore.log_symbol_index_search
        ~sienv
        ~start_time
        ~query_text:!auto_complete_for_global
        ~max_results
        ~kind_filter:!kind_filter
        ~results:(List.length results.With_complete_flag.value)
        ~context:completion_type
        ~caller:"AutocompleteService.go";
      results)
