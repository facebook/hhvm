(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Reordered_argument_collections
open Typing_defs
open Utils
open String_utils
open SearchUtils
include AutocompleteTypes
open Tast
module Nast = Aast
module Tast = Aast
module Phase = Typing_phase
module TUtils = Typing_utils
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
let add_position_to_results (raw_results : SearchUtils.si_results) :
    SearchUtils.result =
  SearchUtils.(
    List.filter_map raw_results ~f:(fun r ->
        match SymbolIndex.get_pos_for_item_opt r with
        | Some pos -> Some { name = r.si_name; pos; result_type = r.si_kind }
        | None -> None))

let (argument_global_type : autocomplete_type option ref) = ref None

let auto_complete_for_global = ref ""

let auto_complete_suffix = "AUTO332"

let suffix_len = String.length auto_complete_suffix

let strip_suffix s = String.sub s 0 (String.length s - suffix_len)

let matches_auto_complete_suffix x =
  String.length x >= suffix_len
  &&
  let suffix = String.sub x (String.length x - suffix_len) suffix_len in
  suffix = auto_complete_suffix

let is_auto_complete x =
  if !autocomplete_results = [] then
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
      let ed = { range.ed with column = range.ed.column - suffix_len } in
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
  | Nast.CI sid -> autocomplete_token Acnew (Some env) sid
  | _ -> ()

let get_class_elt_types env class_ cid elts =
  let is_visible (_, elt) =
    Tast_env.is_visible env (elt.ce_visibility, elt.ce_lsb) cid class_
  in
  elts
  |> Sequence.filter ~f:is_visible
  |> Sequence.map ~f:(fun (id, { ce_type = (lazy ty); _ }) -> (id, ty))

let autocomplete_shape_key env fields id =
  if is_auto_complete (snd id) then (
    ac_env := Some env;
    autocomplete_identifier := Some id;
    argument_global_type := Some Acshape_key;

    (* not the same as `prefix == ""` in namespaces *)
    let have_prefix = Pos.length (fst id) > suffix_len in
    let prefix = strip_suffix (snd id) in
    let add (name : Ast_defs.shape_field_name) =
      let (code, kind, ty) =
        match name with
        | Ast_defs.SFlit_int (pos, str) ->
          let reason = Typing_reason.Rwitness pos in
          let ty = Typing_defs.Tprim Aast_defs.Tint in
          (str, SI_Literal, (reason, ty))
        | Ast_defs.SFlit_str (pos, str) ->
          let reason = Typing_reason.Rwitness pos in
          let ty = Typing_defs.Tprim Aast_defs.Tstring in
          let quote =
            if have_prefix then
              Str.first_chars prefix 1
            else
              "'"
          in
          (quote ^ str ^ quote, SI_Literal, (reason, ty))
        | Ast_defs.SFclass_const ((pos, cid), (_, mid)) ->
          ( Printf.sprintf "%s::%s" cid mid,
            SI_ClassConstant,
            (Reason.Rwitness pos, Typing_defs.make_tany ()) )
      in
      if (not have_prefix) || string_starts_with code prefix then
        add_partial_result code (Phase.decl ty) kind None
    in
    List.iter (Ast_defs.ShapeMap.keys fields) ~f:add
  )

let autocomplete_member ~is_static env class_ cid id =
  (* This is used for instance "$x->|" and static "Class1::|" members. *)
  (* It's also used for "<nt:fb:text |" XHP attributes, in which case  *)
  (* class_ is ":nt:fb:text" and its attributes are in tc_props.       *)
  if is_auto_complete (snd id) then (
    (* Detect usage of "parent::|" which can use both static and instance *)
    let match_both_static_and_instance =
      match cid with
      | Some Nast.CIparent -> true
      | _ -> false
    in
    ac_env := Some env;
    autocomplete_identifier := Some id;
    argument_global_type := Some Acclass_get;
    let add kind (name, ty) =
      add_partial_result name (Phase.decl ty) kind (Some class_)
    in
    if is_static || match_both_static_and_instance then (
      Sequence.iter
        (get_class_elt_types env class_ cid (Cls.smethods class_))
        ~f:(add SearchUtils.SI_ClassMethod);
      Sequence.iter
        (get_class_elt_types env class_ cid (Cls.sprops class_))
        ~f:(add SearchUtils.SI_Property);
      Sequence.iter (Cls.consts class_) ~f:(fun (name, cc) ->
          add SearchUtils.SI_ClassConstant (name, cc.cc_type))
    );
    if (not is_static) || match_both_static_and_instance then (
      Sequence.iter
        (get_class_elt_types env class_ cid (Cls.methods class_))
        ~f:(add SearchUtils.SI_ClassMethod);
      Sequence.iter
        (get_class_elt_types env class_ cid (Cls.props class_))
        ~f:(add SearchUtils.SI_Property)
    )
  )

let autocomplete_lvar id env =
  (* This is used for "$|" and "$x = $|" local variables. *)
  let text = Local_id.get_name (snd id) in
  if is_auto_complete text then (
    argument_global_type := Some Acprop;
    ac_env := Some env;
    autocomplete_identifier := Some (fst id, text)
  )

let should_complete_class completion_type class_kind =
  match (completion_type, class_kind) with
  | (Some Acid, Some Ast_defs.Cnormal)
  | (Some Acid, Some Ast_defs.Cabstract)
  | (Some Acnew, Some Ast_defs.Cnormal)
  | (Some Actrait_only, Some Ast_defs.Ctrait)
  | (Some Actype, Some _) ->
    true
  | _ -> false

let should_complete_fun completion_type = completion_type = Some Acid

let get_constructor_ty c =
  let pos = Cls.pos c in
  let reason = Typing_reason.Rwitness pos in
  let return_ty = (reason, Typing_defs.Tapply ((pos, Cls.name c), [])) in
  match fst (Cls.construct c) with
  | Some elt ->
    begin
      match elt.ce_type with
      | (lazy ((_ as r), Tfun fun_)) ->
        (* We have a constructor defined, but the return type is void
         * make it the object *)
        let fun_ =
          {
            fun_ with
            Typing_defs.ft_ret = { et_type = return_ty; et_enforced = false };
          }
        in
        (r, Tfun fun_)
      | _ -> (* how can a constructor not be a function? *) assert false
    end
  | None ->
    (* Nothing defined, so we need to fake the entire constructor *)
    ( reason,
      Typing_defs.Tfun
        {
          ft_pos = pos;
          ft_deprecated = None;
          ft_abstract = false;
          ft_is_coroutine = false;
          ft_arity = Fstandard (0, 0);
          ft_tparams = ([], FTKtparams);
          ft_where_constraints = [];
          ft_params = [];
          ft_ret = { et_type = return_ty; et_enforced = false };
          ft_fun_kind = Ast_defs.FSync;
          ft_reactive = Nonreactive;
          ft_return_disposable = false;
          ft_returns_mutable = false;
          ft_mutability = None;
          ft_decl_errors = None;
          ft_returns_void_to_rx = false;
        } )

(* Global identifier autocomplete uses search service to find matching names *)
let search_funs_and_classes input ~limit ~on_class ~on_function =
  SymbolIndex.query_for_autocomplete input ~limit ~filter_map:(fun _ _ res ->
      let name = res.SearchUtils.name in
      match res.SearchUtils.result_type with
      | SearchUtils.SI_Interface
      | SearchUtils.SI_Trait
      | SearchUtils.SI_Enum
      | SearchUtils.SI_Class ->
        on_class name
      | SearchUtils.SI_Function -> on_function name
      | _ -> None)

(* compute_complete_global: given the sets content_funs and content_classes  *)
(* of function names and classes in the current file, returns a list of all  *)
(* possible identifier autocompletions at the autocomplete position (which   *)
(* is stored in a global mutable reference). The results are stored in the   *)
(* global mutable reference 'autocomplete_results'.                          *)
(*                                                                           *)
(* We treat namespaces as first-class entities in all cases.                 *)
(*                                                                           *)
(* XHP note:                                                                 *)
(* This function is also called for "<foo|", with gname="foo".               *)
(* This is a Hack shorthand for referring to the global classname ":foo".    *)
(* Our global dictionary of classnames stores the authoritative name ":foo", *)
(* and inside autocompleteService we do the job of inserting a leading ":"   *)
(* from the user's prefix, and stripping the leading ":" when we emit.       *)
let compute_complete_global
    ~(tcopt : TypecheckerOptions.t)
    ~(autocomplete_context : AutocompleteTypes.legacy_autocomplete_context)
    ~(content_funs : Reordered_argument_collections.SSet.t)
    ~(content_classes : Reordered_argument_collections.SSet.t) : unit =
  let completion_type = !argument_global_type in
  let gname = Utils.strip_ns !auto_complete_for_global in
  let gname = strip_suffix gname in
  let gname =
    if autocomplete_context.is_xhp_classname then
      ":" ^ gname
    else
      gname
  in
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
    | Some (pos, _) -> Pos.length pos > suffix_len
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
  else
    let does_fully_qualified_name_match_prefix name =
      let stripped_name = strip_ns name in
      (* name must match gname, and have no additional namespace slashes, e.g. *)
      (* name="Str\\co" gname="S" -> false *)
      (* name="Str\\co" gname="Str\\co" -> true *)
      string_starts_with stripped_name gname
      && not (String.contains stripped_name ~pos:(String.length gname) '\\')
    in
    let string_to_replace_prefix name = Utils.strip_ns name in
    let result_count = ref 0 in
    let on_class name ~seen =
      if SSet.mem seen name then
        None
      else if not (does_fully_qualified_name_match_prefix name) then
        None
      else
        let target = Decl_provider.get_class name in
        let target_kind = Option.map target ~f:(fun c -> Cls.kind c) in
        if not (should_complete_class completion_type target_kind) then
          None
        else
          Option.map target ~f:(fun c ->
              incr result_count;
              if completion_type = Some Acnew then
                get_partial_result
                  (string_to_replace_prefix name)
                  (Phase.decl (get_constructor_ty c))
                  SearchUtils.SI_Constructor
                  (* Only do doc block fallback on constructors if they're consistent. *)
                  ( if snd (Cls.construct c) <> Inconsistent then
                    Some c
                  else
                    None )
              else
                let kind =
                  match Cls.kind c with
                  | Ast_defs.Cabstract
                  | Ast_defs.Cnormal ->
                    SearchUtils.SI_Class
                  | Ast_defs.Cinterface -> SearchUtils.SI_Interface
                  | Ast_defs.Ctrait -> SearchUtils.SI_Trait
                  | Ast_defs.Cenum
                  | Ast_defs.Crecord ->
                    SearchUtils.SI_Enum
                  (* TODO(T36697624): Add Record_kind *)
                in
                let ty =
                  ( Typing_reason.Rwitness (Cls.pos c),
                    Typing_defs.Tapply ((Cls.pos c, name), []) )
                in
                get_partial_result
                  (string_to_replace_prefix name)
                  (Phase.decl ty)
                  kind
                  None)
    in
    let on_function name ~seen =
      if autocomplete_context.is_xhp_classname then
        None
      else if SSet.mem seen name then
        None
      else if not (should_complete_fun completion_type) then
        None
      else if not (does_fully_qualified_name_match_prefix name) then
        None
      else
        Option.map (Decl_provider.get_fun name) ~f:(fun fun_ ->
            incr result_count;
            let ty =
              ( Typing_reason.Rwitness fun_.Typing_defs.ft_pos,
                Typing_defs.Tfun fun_ )
            in
            get_partial_result
              (string_to_replace_prefix name)
              (Phase.decl ty)
              SearchUtils.SI_Function
              None)
    in
    let on_namespace name : autocomplete_result option =
      (* name will have the form "Str" or "HH\\Lib\\Str" *)
      (* Our autocomplete will show up in the list as "Str". *)
      if autocomplete_context.is_xhp_classname then
        None
      else if not (does_fully_qualified_name_match_prefix name) then
        None
      else
        Some
          (Complete
             {
               res_pos = Pos.none |> Pos.to_absolute;
               res_replace_pos = get_replace_pos_exn ();
               res_base_class = None;
               res_ty = "namespace";
               res_name = string_to_replace_prefix name;
               res_fullname = string_to_replace_prefix name;
               res_kind = SearchUtils.SI_Namespace;
               func_details = None;
             })
    in
    (* Try using the names in local content buffer first *)
    List.iter
      (List.filter_map
         (SSet.elements content_classes)
         (on_class ~seen:SSet.empty))
      add_res;
    List.iter
      (List.filter_map
         (SSet.elements content_funs)
         (on_function ~seen:SSet.empty))
      add_res;

    (* Add namespaces. The hack server doesn't index namespaces themselves; it  *)
    (* only stores names of functions and classes in fully qualified form, e.g. *)
    (*   \\HH\\Lib\\Str\\length                                                 *)
    (* If the project's .hhconfig has auto_namesspace_map "Str": "HH\Lib\\Str"  *)
    (* then the hack server will index the function just as                     *)
    (*   \\Str\\length                                                          *)
    (* The main index, having only a global list if functions/classes, doesn't  *)
    (* actually offer any way for us to iterate over namespaces. And changing   *)
    (* its trie-indexer to do so is kind of ugly. So as a temporary workaround, *)
    (* to give an okay user-experience at least for the Hack standard library,  *)
    (* we're just going to list all the possible standard namespaces right here *)
    (* and see if any of them really exist in the current codebase/hhconfig!    *)
    (* This will give a good experience only for codebases where users rarely   *)
    (* define their own namespaces...                                           *)
    let standard_namespaces =
      [
        "C";
        "Vec";
        "Dict";
        "Str";
        "Keyset";
        "Math";
        "PseudoRandom";
        "SecureRandom";
        "PHP";
        "JS";
      ]
    in
    let auto_namespace_map = GlobalOptions.po_auto_namespace_map tcopt in
    let all_standard_namespaces =
      List.concat_map standard_namespaces ~f:(fun ns ->
          [
            Printf.sprintf "%s" ns;
            Printf.sprintf "%s\\fb" ns;
            Printf.sprintf "HH\\Lib\\%s" ns;
            Printf.sprintf "HH\\Lib\\%s\\fb" ns;
          ])
    in
    let all_aliased_namespaces =
      List.concat_map auto_namespace_map ~f:(fun (alias, fully_qualified) ->
          [alias; fully_qualified])
    in
    let all_possible_namespaces =
      all_standard_namespaces @ all_aliased_namespaces
      |> SSet.of_list
      |> SSet.elements
    in
    List.iter all_possible_namespaces ~f:(fun ns ->
        let ns_results =
          search_funs_and_classes
            (ns ^ "\\")
            ~limit:(Some 1)
            ~on_class:(fun _className -> on_namespace ns)
            ~on_function:(fun _functionName -> on_namespace ns)
        in
        List.iter ns_results.With_complete_flag.value add_res);

    (* Use search results to look for matches, while excluding names we have
     * already seen in local content buffer *)
    let gname_results =
      search_funs_and_classes
        gname
        ~limit:(Some 100)
        ~on_class:(on_class ~seen:content_classes)
        ~on_function:(on_function ~seen:content_funs)
    in
    autocomplete_is_complete :=
      !autocomplete_is_complete && gname_results.With_complete_flag.is_complete;
    List.iter gname_results.With_complete_flag.value add_res

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
    min_arity = arity_min ft.ft_arity;
    params =
      ( List.map ft.ft_params param_to_record
      @
      match ft.ft_arity with
      | Fellipsis _ ->
        let empty =
          TUtils.default_fun_param (Reason.none, Tany TanySentinel.value)
        in
        [param_to_record ~is_variadic:true empty]
      | Fvariadic (_, p) -> [param_to_record ~is_variadic:true p]
      | Fstandard _ -> [] );
  }

(* Convert a `ty` into a func details structure *)
let get_func_details_for env ty =
  let (env, ty) =
    match ty with
    | DeclTy ty -> Tast_env.localize_with_self env ty
    | LoclTy ty -> (env, ty)
  in
  match ty with
  | (_, Tfun ft) -> Some (tfun_to_func_details env ft)
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
  let name =
    match (x.kind_, autocomplete_context) with
    | ( SearchUtils.SI_Property,
        { AutocompleteTypes.is_instance_member = false; _ } ) ->
      lstrip x.name ":"
    | (SearchUtils.SI_XHP, { AutocompleteTypes.is_xhp_classname = true; _ })
    | (SearchUtils.SI_Class, { AutocompleteTypes.is_xhp_classname = true; _ })
      ->
      lstrip x.name ":"
    | _ -> x.name
  in
  let pos =
    match x.ty with
    | LoclTy loclt -> fst loclt |> Typing_reason.to_pos |> Pos.to_absolute
    | DeclTy declt -> fst declt |> Typing_reason.to_pos |> Pos.to_absolute
  in
  {
    res_pos = pos;
    res_replace_pos = replace_pos;
    res_base_class = x.base_class;
    res_ty = get_desc_string_for env x.ty x.kind_;
    res_name = name;
    res_fullname = name;
    res_kind = x.kind_;
    func_details = get_func_details_for env x.ty;
  }

let autocomplete_typed_member ~is_static env class_ty cid mid =
  Tast_env.get_class_ids env class_ty
  |> List.iter ~f:(fun cname ->
         Decl_provider.get_class cname
         |> Option.iter ~f:(fun class_ ->
                let cid = Option.map cid to_nast_class_id_ in
                autocomplete_member ~is_static env class_ cid mid))

let autocomplete_static_member env ((_, ty), cid) mid =
  autocomplete_typed_member ~is_static:true env ty (Some cid) mid

let visitor =
  object (self)
    inherit Tast_visitor.iter as super

    method! on_Id env id =
      autocomplete_id id env;
      super#on_Id env id

    method! on_Fun_id env id =
      autocomplete_id id env;
      super#on_Fun_id env id

    method! on_New env cid el uel =
      autocomplete_new (to_nast_class_id_ (snd cid)) env;
      super#on_New env cid el uel

    method! on_Happly env sid hl =
      autocomplete_hint sid;
      super#on_Happly env sid hl

    method! on_Lvar env lid =
      autocomplete_lvar lid env;
      super#on_Lvar env lid

    method! on_Class_get env cid mid =
      match mid with
      | Tast.CGstring p -> autocomplete_static_member env cid p
      | Tast.CGexpr _ ->
        ();
        super#on_Class_get env cid mid

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
          match ty with
          | (_, Typing_defs.Tshape (_, fields)) ->
            (match key with
            | Tast.Id (_, mid) -> autocomplete_shape_key env fields (pos, mid)
            | Tast.String mid ->
              (* autocomplete generally assumes that there's a token ending with the suffix; *)
              (* This isn't the case for `$shape['a`, unless it's at the end of the file *)
              let offset =
                String_utils.substring_index auto_complete_suffix mid
              in
              if offset = -1 then
                autocomplete_shape_key env fields (pos, mid)
              else
                let mid = Str.string_before mid (offset + suffix_len) in
                let (line, bol, cnum) = Pos.line_beg_offset pos in
                let pos =
                  Pos.make_from_lnum_bol_cnum
                    ~pos_file:(Pos.filename pos)
                    ~pos_start:(line, bol, cnum)
                    ~pos_end:(line, bol, cnum + offset + suffix_len)
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
      let cid = Nast.CI sid in
      Decl_provider.get_class (snd sid)
      |> Option.iter ~f:(fun c ->
             List.iter attrs ~f:(function
                 | Tast.Xhp_simple (id, _) ->
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

    method get_types tast =
      self#go tast;
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
          self#add Typing_defs.this (Tast_env.get_self_exn env);
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

let compute_complete_local tast =
  (new local_types)#get_types tast
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
    ~(tcopt : TypecheckerOptions.t)
    ~(content_funs : Reordered_argument_collections.SSet.t)
    ~(content_classes : Reordered_argument_collections.SSet.t)
    ~(autocomplete_context : AutocompleteTypes.legacy_autocomplete_context)
    ~(sienv : SearchUtils.si_env)
    ~(tast_env : Tast_env.t) : unit =
  (* Select the provider to use for symbol autocomplete *)
  match sienv.sie_provider with
  (* Legacy provider should match previous behavior *)
  | TrieIndex ->
    compute_complete_global
      ~tcopt
      ~autocomplete_context
      ~content_funs
      ~content_classes;

    (* If only traits are valid, filter to that *)
    if completion_type = Some Actrait_only then
      autocomplete_results :=
        List.filter !autocomplete_results ~f:(fun r ->
            match r with
            | Complete c -> c.res_kind = SI_Trait
            | Partial p -> p.kind_ = SI_Trait)
  (* The new simpler providers *)
  | _ ->
    (kind_filter :=
       match completion_type with
       | Some Acnew -> Some SI_Class
       | Some Actrait_only -> Some SI_Trait
       | _ -> None);
    let query_text = strip_suffix !auto_complete_for_global in
    let replace_pos = get_replace_pos_exn () in
    let (ns, _) = Utils.split_ns_from_name query_text in
    (* This ensures that we do not have a leading backslash *)
    let query_text = Utils.strip_ns query_text in
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
        (* Only load func details if the flag sie_resolve_signatures is true *)
        let (func_details, res_ty) =
          if sienv.sie_resolve_signatures && r.si_kind = SI_Function then
            let fixed_name = ns ^ r.si_name in
            match Tast_env.get_fun tast_env fixed_name with
            | None -> (None, kind_to_string r.si_kind)
            | Some ft ->
              let ty = (Typing_reason.Rnone, Tfun ft) in
              let details = get_func_details_for tast_env (DeclTy ty) in
              let res_ty = Tast_env.print_decl_ty tast_env ty in
              (details, res_ty)
          else
            (None, kind_to_string r.si_kind)
        in
        (* Figure out how to display them *)
        let complete =
          {
            res_pos = absolute_none;
            (* This is okay - resolve will fill it in *)
            res_replace_pos = replace_pos;
            res_base_class = None;
            res_ty;
            res_name = r.si_name;
            res_fullname = r.si_fullname;
            res_kind = r.si_kind;
            func_details;
          }
        in
        add_res (Complete complete));
    autocomplete_is_complete := List.length results < max_results

(* Main entry point for autocomplete *)
let go
    ~(tcopt : TypecheckerOptions.t)
    ~(content_funs : Reordered_argument_collections.SSet.t)
    ~(content_classes : Reordered_argument_collections.SSet.t)
    ~(autocomplete_context : AutocompleteTypes.legacy_autocomplete_context)
    ~(sienv : SearchUtils.si_env)
    tast =
  reset ();
  visitor#go tast;
  Errors.ignore_ (fun () ->
      let start_time = Unix.gettimeofday () in
      let max_results = 100 in
      let kind_filter = ref None in
      let tast_env =
        match !ac_env with
        | Some e -> e
        | None -> Tast_env.empty tcopt
      in
      let completion_type = !argument_global_type in
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
          ~tcopt
          ~autocomplete_context
          ~content_funs
          ~content_classes
          ~sienv
          ~tast_env;

      if completion_type = Some Acprop then compute_complete_local tast;
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
      let results =
        {
          With_complete_flag.is_complete = !autocomplete_is_complete;
          value = !autocomplete_results |> List.map ~f:resolve;
        }
      in
      SymbolIndex.log_symbol_index_search
        ~sienv
        ~start_time
        ~query_text:!auto_complete_for_global
        ~max_results
        ~kind_filter:!kind_filter
        ~results:(List.length results.With_complete_flag.value)
        ~context:completion_type
        ~caller:"AutocompleteService.go";
      results)
