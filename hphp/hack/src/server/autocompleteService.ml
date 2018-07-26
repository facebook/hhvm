(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open Reordered_argument_collections
open Typing_defs
open Utils
open String_utils
include AutocompleteTypes

module Phase = Typing_phase
module TUtils = Typing_utils

let ac_env = ref None
let autocomplete_results : autocomplete_result list ref = ref []
let autocomplete_is_complete : bool ref = ref true

(* The position we're autocompleting at. This is used when computing completions
 * for global identifiers. *)
let autocomplete_identifier: (Pos.t * string) option ref = ref None

type autocomplete_type =
  | Acid
  | Acnew
  | Actype
  | Acclass_get
  | Acprop
  | Acshape_key
  | Actrait_only

let (argument_global_type: autocomplete_type option ref) = ref None
let auto_complete_for_global = ref ""

let auto_complete_suffix = "AUTO332"
let suffix_len = String.length auto_complete_suffix
let strip_suffix s = String.sub s 0 (String.length s - suffix_len)

let matches_auto_complete_suffix x =
  String.length x >= suffix_len &&
  let suffix = String.sub x (String.length x - suffix_len) suffix_len in
  suffix = auto_complete_suffix

let is_auto_complete x =
  if !autocomplete_results = []
  then matches_auto_complete_suffix x
  else false

let get_replace_pos_exn ~delimit_on_namespaces =
  match !autocomplete_identifier with
  | None -> failwith "No autocomplete position was set."
  | Some (pos, text) ->
    if Pos.length pos < suffix_len
    then failwith "Matched position is shorter than autocomplete suffix."
    else
    let open Ide_api_types in
    let range = pos_to_range pos in
    let st = if delimit_on_namespaces
      then match String.rindex_opt text '\\' with
        | Some index ->
          { range.st with column = range.st.column + index }
        | None -> range.st
      else range.st in
    let ed = { range.ed with column = range.ed.column - suffix_len } in
    { st; ed }


let autocomplete_result_to_json res =
  let func_param_to_json param =
    Hh_json.JSON_Object [ "name", Hh_json.JSON_String param.param_name;
                     "type", Hh_json.JSON_String param.param_ty;
                     "variadic", Hh_json.JSON_Bool param.param_variadic;
                   ]
  in
  let func_details_to_json details =
    match details with
     | Some fd -> Hh_json.JSON_Object [
           "min_arity", Hh_json.int_ fd.min_arity;
           "return_type", Hh_json.JSON_String fd.return_ty;
           "params", Hh_json.JSON_Array (List.map fd.params func_param_to_json);
       ]
     | None -> Hh_json.JSON_Null
  in
  let name = res.res_name in
  let pos = res.res_pos in
  let ty = res.res_ty in
  Hh_json.JSON_Object [
      "name", Hh_json.JSON_String name;
      "type", Hh_json.JSON_String ty;
      "pos", Pos.json pos;
      "func_details", func_details_to_json res.func_details;
      "expected_ty", Hh_json.JSON_Bool false; (* legacy field, left here in case clients need it *)
  ]

let get_partial_result name ty kind class_opt =
  let base_class = Option.map ~f:(fun class_ -> class_.Typing_defs.tc_name) class_opt in
  Partial { ty; name; kind_=kind; base_class; }

let add_res (res: autocomplete_result) : unit =
  autocomplete_results := res :: !autocomplete_results

let add_partial_result name ty kind class_opt =
  add_res (get_partial_result name ty kind class_opt)

let autocomplete_token ac_type env x =
  if is_auto_complete (snd x)
  then begin
    ac_env := env;
    autocomplete_identifier := Some x;
    argument_global_type := Some ac_type;
    auto_complete_for_global := snd x
  end

let autocomplete_id id env = autocomplete_token Acid (Some env) id

let autocomplete_hint = autocomplete_token Actype None

let autocomplete_trait_only = autocomplete_token Actrait_only None

let autocomplete_new cid env =
  match cid with
  | Nast.CI (sid, _) -> autocomplete_token Acnew (Some env) sid
  | _ -> ()

let get_class_elt_types env class_ cid elts =
  let elts = SMap.filter elts begin fun _ x ->
    Tast_env.is_visible env x.ce_visibility cid class_
  end in
  SMap.map elts (fun { ce_type = lazy ty; _ } -> ty)

let autocomplete_shape_key env fields id =
  if is_auto_complete (snd id)
  then begin
    ac_env := Some env;
    autocomplete_identifier := Some id;
    argument_global_type := Some Acshape_key;
    (* not the same as `prefix == ""` in namespaces *)
    let have_prefix = (Pos.length (fst id)) > suffix_len in
    let prefix = strip_suffix (snd id) in
    let add (name: Ast.shape_field_name) =
      let code, kind, ty = match name with
        | Ast.SFlit_int (pos, str) ->
          let reason = Typing_reason.Rwitness pos in
          let ty = Typing_defs.Tprim Aast_defs.Tint in
          (str, Literal_kind, (reason, ty))
        | Ast.SFlit_str (pos, str) ->
          let reason = Typing_reason.Rwitness pos in
          let ty = Typing_defs.Tprim Aast_defs.Tstring in
          let quote = if have_prefix then Str.first_chars prefix 1 else "'" in
          (quote^str^quote, Literal_kind, (reason, ty))
        | Ast.SFclass_const ((pos, cid), (_, mid)) ->
          (Printf.sprintf "%s::%s" cid mid, Class_constant_kind, (Reason.Rwitness pos, Typing_defs.Tany))
      in
      if (not have_prefix) || string_starts_with code prefix
      then add_partial_result code (Phase.decl ty) kind None
    in
    List.iter (Ast.ShapeMap.keys fields) ~f:add
  end

let autocomplete_member ~is_static env class_ cid id =
  (* This is used for instance "$x->|" and static "Class1::|" members. *)
  (* It's also used for "<nt:fb:text |" XHP attributes, in which case  *)
  (* class_ is ":nt:fb:text" and its attributes are in tc_props.       *)
  if is_auto_complete (snd id)
  then begin
    ac_env := Some env;
    autocomplete_identifier := Some id;
    argument_global_type := Some Acclass_get;
    let add kind name ty = add_partial_result name (Phase.decl ty) kind (Some class_) in
    if is_static then begin
      SMap.iter (get_class_elt_types env class_ cid class_.tc_smethods) ~f:(add Method_kind);
      SMap.iter (get_class_elt_types env class_ cid class_.tc_sprops) ~f:(add Property_kind);
      SMap.iter (class_.tc_consts) ~f:(fun name cc -> add Class_constant_kind name cc.cc_type);
    end else begin
      SMap.iter (get_class_elt_types env class_ cid class_.tc_methods) ~f:(add Method_kind);
      SMap.iter (get_class_elt_types env class_ cid class_.tc_props) ~f:(add Property_kind);
    end
  end

let autocomplete_lvar id env =
  (* This is used for "$|" and "$x = $|" local variables. *)
  let text = Local_id.get_name (snd id) in
  if is_auto_complete text
  then begin
    argument_global_type := Some Acprop;
    ac_env := Some env;
    autocomplete_identifier := Some (fst id, text);
  end

let should_complete_class completion_type class_kind =
  match completion_type, class_kind with
  | Some Acid, Some Ast.Cnormal
  | Some Acid, Some Ast.Cabstract
  | Some Acnew, Some Ast.Cnormal
  | Some Actrait_only, Some Ast.Ctrait
  | Some Actype, Some _ -> true
  | _ -> false

let should_complete_fun completion_type =
  completion_type=Some Acid

let get_constructor_ty c =
  let pos = c.Typing_defs.tc_pos in
  let reason = Typing_reason.Rwitness pos in
  let return_ty = reason, Typing_defs.Tapply ((pos, c.Typing_defs.tc_name), []) in
  match (fst c.Typing_defs.tc_construct) with
    | Some elt ->
        begin match elt.ce_type with
          | lazy (_ as r, Tfun fun_) ->
              (* We have a constructor defined, but the return type is void
               * make it the object *)
              let fun_ = { fun_ with Typing_defs.ft_ret = return_ty } in
              r, Tfun fun_
          | _ -> (* how can a constructor not be a function? *) assert false
        end
    | None ->
        (* Nothing defined, so we need to fake the entire constructor *)
      reason,
      Typing_defs.Tfun
        (Typing_env.make_ft pos Nonreactive (*is_coroutine*)false [] return_ty)

(* Global identifier autocomplete uses search service to find matching names *)
let search_funs_and_classes input ~limit ~on_class ~on_function =
  HackSearchService.MasterApi.query_autocomplete input ~limit
    ~filter_map:begin fun _ _ res ->
      let name = res.SearchUtils.name in
      match res.SearchUtils.result_type with
      | HackSearchService.Class _-> on_class name
      | HackSearchService.Function -> on_function name
      | _ -> None
    end

(* compute_complete_global: given the sets content_funs and content_classes  *)
(* of function names and classes in the current file, returns a list of all  *)
(* possible identifier autocompletions at the autocomplete position (which   *)
(* is stored in a global mutable reference). The results are stored in the   *)
(* global mutable reference 'autocomplete_results'.                          *)
(* This function has two modes of dealing with namespaces...                 *)
(*     delimit_on_namespaces=true   delimit_on_namespaces=false              *)
(*      St| =>               Str                          Str\compare, ...   *)
(*  Str\\c| =>               compare                      Str\compare        *)
(* Essentially, 'delimit_on_namespaces=true' means that autocomplete treats  *)
(* namespaces as first class entities; 'false' means that it treats them     *)
(* purely as part of long identifier names where the symbol '\\' is not      *)
(* really any different from the symbol '_' for example.                     *)
(*                                                                           *)
(* XHP note:                                                                 *)
(* This function is also called for "<foo|", with gname="foo".               *)
(* This is a Hack shorthand for referring to the global classname ":foo".    *)
(* Our global dictionary of classnames stores the authoritative name ":foo", *)
(* and inside autocompleteService we do the job of inserting a leading ":"   *)
(* from the user's prefix, and stripping the leading ":" when we emit.       *)
let compute_complete_global
  ~(tcopt: TypecheckerOptions.t)
  ~(delimit_on_namespaces: bool)
  ~(autocomplete_context: AutocompleteTypes.legacy_autocomplete_context)
  ~(content_funs: Reordered_argument_collections.SSet.t)
  ~(content_classes: Reordered_argument_collections.SSet.t)
  : unit =
  let completion_type = !argument_global_type in
  let gname = Utils.strip_ns !auto_complete_for_global in
  let gname = strip_suffix gname in
  let gname = if autocomplete_context.is_xhp_classname then (":" ^ gname) else gname in
  (* is_after_single_colon : XHP vs switch statements                       *)
  (* is_after_open_square_bracket : shape field names vs container keys     *)
  (* is_after_quote: shape field names vs arbitrary strings                 *)
  (*                                                                        *)
  (* We can recognize these cases by whether the prefix is empty.           *)
  (* We do this by checking the identifier length, as the string will       *)
  (* include the current namespace.                                         *)
  let have_user_prefix = match !autocomplete_identifier with
    | None -> failwith "No autocomplete position was set"
    | Some (pos, _) -> Pos.length pos > suffix_len in
  let ctx = autocomplete_context in
  if (not ctx.is_manually_invoked) && (not have_user_prefix) &&
    (ctx.is_after_single_colon || ctx.is_after_open_square_bracket || ctx.is_after_quote)
  then ()
  else if ctx.is_after_double_right_angle_bracket then
    (* <<__Override>>AUTO332 *)
    ()
  else begin

    let does_fully_qualified_name_match_prefix name =
      let stripped_name = strip_ns name in
      if delimit_on_namespaces then
        (* name must match gname, and have no additional namespace slashes, e.g. *)
        (* name="Str\\co" gname="S" -> false *)
        (* name="Str\\co" gname="Str\\co" -> true *)
        string_starts_with stripped_name gname &&
          not (String.contains_from stripped_name (String.length gname) '\\')
      else
        string_starts_with stripped_name gname
    in

    let string_to_replace_prefix name =
      let stripped_name = strip_ns name in
      if delimit_on_namespaces then
        (* returns the part of 'name' after its rightmost slash *)
        try
          let len = String.length stripped_name in
          let i = (String.rindex stripped_name '\\') + 1 in
          String.sub stripped_name i (len - i)
        with _ ->
          stripped_name
      else
        stripped_name
    in

    let result_count = ref 0 in

    let on_class name ~seen =
      if SSet.mem seen name then None else
      if not (does_fully_qualified_name_match_prefix name) then None else
      let target = Typing_lazy_heap.get_class tcopt name in
      let target_kind = Option.map target ~f:(fun c -> c.Typing_defs.tc_kind) in
      if not (should_complete_class completion_type target_kind) then None else
      Option.map target ~f:(fun c ->
        incr result_count;
        if completion_type = Some Acnew then
          get_partial_result
            (string_to_replace_prefix name)
            (Phase.decl (get_constructor_ty c))
            Constructor_kind
            (* Only do doc block fallback on constructors if they're consistent. *)
            (if snd c.Typing_defs.tc_construct then Some c else None)
        else
          let kind = match c.Typing_defs.tc_kind with
            | Ast.Cabstract -> Abstract_class_kind
            | Ast.Cnormal -> Class_kind
            | Ast.Cinterface -> Interface_kind
            | Ast.Ctrait -> Trait_kind
            | Ast.Cenum -> Enum_kind
          in
          let ty =
            Typing_reason.Rwitness c.Typing_defs.tc_pos,
            Typing_defs.Tapply ((c.Typing_defs.tc_pos, name), []) in
          get_partial_result (string_to_replace_prefix name) (Phase.decl ty) kind None
      )
    in

    let on_function name ~seen =
      if autocomplete_context.is_xhp_classname then None else
      if SSet.mem seen name then None else
      if not (should_complete_fun completion_type) then None else
      if not (does_fully_qualified_name_match_prefix name) then None else
      Option.map (Typing_lazy_heap.get_fun tcopt name) ~f:(fun fun_ ->
        incr result_count;
        let ty = Typing_reason.Rwitness fun_.Typing_defs.ft_pos, Typing_defs.Tfun fun_ in
        get_partial_result (string_to_replace_prefix name) (Phase.decl ty) Function_kind None
      )
    in

    let on_namespace name : autocomplete_result option =
      (* name will have the form "Str" or "HH\\Lib\\Str" *)
      (* Our autocomplete will show up in the list as "Str". *)
      if autocomplete_context.is_xhp_classname then None else
      if not delimit_on_namespaces then None else
      if not (does_fully_qualified_name_match_prefix name) then None else
      Some (Complete {
        res_pos = Pos.none |> Pos.to_absolute;
        res_replace_pos = get_replace_pos_exn ~delimit_on_namespaces;
        res_base_class = None;
        res_ty = "namespace";
        res_name = string_to_replace_prefix name;
        res_kind = Namespace_kind;
        func_details = None;
      })
    in

    (* Try using the names in local content buffer first *)
    List.iter
      (List.filter_map (SSet.elements content_classes) (on_class ~seen:SSet.empty))
        add_res;
    List.iter
      (List.filter_map (SSet.elements content_funs) (on_function ~seen:SSet.empty))
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
    let standard_namespaces = [
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
    ] in
    let auto_namespace_map = GlobalOptions.po_auto_namespace_map tcopt in
    let all_standard_namespaces = List.concat_map standard_namespaces ~f:(
      fun ns -> [
         Printf.sprintf "%s" ns;
         Printf.sprintf "%s\\fb" ns;
         Printf.sprintf "HH\\Lib\\%s" ns;
         Printf.sprintf "HH\\Lib\\%s\\fb" ns;
      ]
    ) in
    let all_aliased_namespaces = List.concat_map auto_namespace_map ~f:(
      fun (alias, fully_qualified) -> [alias; fully_qualified]
    ) in
    let all_possible_namespaces =
      (all_standard_namespaces @ all_aliased_namespaces)
        |> SSet.of_list
        |> SSet.elements in
    List.iter all_possible_namespaces ~f:(fun ns ->
      let ns_results = search_funs_and_classes (ns ^ "\\") ~limit:(Some 1)
        ~on_class:(fun _className -> on_namespace ns)
        ~on_function:(fun _functionName -> on_namespace ns)
      in
      List.iter ns_results.With_complete_flag.value add_res
    );

    (* Use search results to look for matches, while excluding names we have
     * already seen in local content buffer *)
    let gname_results = search_funs_and_classes gname ~limit:(Some 100)
      ~on_class:(on_class ~seen:content_classes)
      ~on_function:(on_function ~seen:content_funs)
    in
    autocomplete_is_complete :=
      !autocomplete_is_complete && gname_results.With_complete_flag.is_complete;
    List.iter gname_results.With_complete_flag.value add_res;
  end

(* Here we turn partial_autocomplete_results into complete_autocomplete_results *)
(* by using typing environment to convert ty information into strings. *)
let resolve_ty
    (env: Tast_env.t)
    (autocomplete_context: legacy_autocomplete_context)
    (x: partial_autocomplete_result)
    ~(delimit_on_namespaces: bool)
  : complete_autocomplete_result =
  let env, ty = match x.ty with
    | DeclTy ty -> Tast_env.localize_with_self env ty
    | LoclTy ty -> env, ty
  in
  let desc_string = match x.kind_ with
    | Method_kind
    | Function_kind
    | Variable_kind
    | Property_kind
    | Class_constant_kind
    | Constructor_kind -> Tast_env.print_ty env ty
    | Abstract_class_kind -> "abstract class"
    | Class_kind -> "class"
    | Interface_kind -> "interface"
    | Trait_kind -> "trait"
    | Enum_kind -> "enum"
    | Namespace_kind -> "namespace"
    | Keyword_kind -> "keyword"
    | Literal_kind -> "literal"
  in
  let func_details = match ty with
    | (_, Tfun ft) ->
      let param_to_record ?(is_variadic=false) param =
        {
          param_name     = (match param.fp_name with
                             | Some n -> n
                             | None -> "");
          param_ty       = Tast_env.print_ty env param.fp_type;
          param_variadic = is_variadic;
        }
      in
      Some {
        return_ty = Tast_env.print_ty env ft.ft_ret;
        min_arity = arity_min ft.ft_arity;
        params    = List.map ft.ft_params param_to_record @
          (match ft.ft_arity with
             | Fellipsis _ ->
                 let empty = TUtils.default_fun_param (Reason.none, Tany) in
                 [param_to_record ~is_variadic:true empty]
             | Fvariadic (_, p) -> [param_to_record ~is_variadic:true p]
             | Fstandard _ -> [])
      }
    | _ -> None
  in
  (* XHP class+attribute names are stored internally with a leading colon.    *)
  (* We'll render them without it if and only if we're in an XHP context.     *)
  (*   $x = new :class1() -- do strip the colon in front of :class1           *)
  (*   $x = <:class1      -- don't strip it                                   *)
  (*   $x->:attr1         -- don't strip the colon in front of :attr1         *)
  (*   <class1 :attr="a"  -- do strip it                                      *)
  (* The logic is thorny here because we're relying upon regexes to figure    *)
  (* out the context. Once we switch autocomplete to FFP, it'll be cleaner.   *)
  let name = match x.kind_, autocomplete_context with
    | Property_kind, { AutocompleteTypes.is_instance_member = false; _ } -> lstrip x.name ":"
    | Abstract_class_kind, { AutocompleteTypes.is_xhp_classname = true; _ }
    | Class_kind, { AutocompleteTypes.is_xhp_classname = true; _ } -> lstrip x.name ":"
    | _ -> x.name
  in
  {
    res_pos         = (fst ty) |> Typing_reason.to_pos |> Pos.to_absolute;
    res_replace_pos = get_replace_pos_exn ~delimit_on_namespaces;
    res_base_class  = x.base_class;
    res_ty          = desc_string;
    res_name        = name;
    res_kind        = x.kind_;
    func_details    = func_details;
  }

let tast_cid_to_nast_cid env cid =
  let nmenv = Tast.nast_mapping_env (Tast_env.save env) in
  Tast.NastMapper.map_class_id_ nmenv cid

let autocomplete_typed_member ~is_static env class_ty cid mid =
  Tast_env.get_class_ids env class_ty
  |> List.iter ~f:begin fun cname ->
    Typing_lazy_heap.get_class (Tast_env.get_tcopt env) cname
    |> Option.iter ~f:begin fun class_ ->
      let cid = Option.map cid (tast_cid_to_nast_cid env) in
      autocomplete_member ~is_static env class_ cid mid
    end
  end

let autocomplete_static_member env ((_, ty), cid) mid =
  autocomplete_typed_member ~is_static:true env ty (Some cid) mid

let visitor = object (self)
  inherit Tast_visitor.iter as super

  method! on_Id env id =
    autocomplete_id id env;
    super#on_Id env id

  method! on_Fun_id env id =
    autocomplete_id id env;
    super#on_Fun_id env id

  method! on_New env cid el uel =
    autocomplete_new (tast_cid_to_nast_cid env (snd cid)) env;
    super#on_New env cid el uel

  method! on_Happly env sid hl =
    autocomplete_hint sid;
    super#on_Happly env sid hl

  method! on_Lvar env lid =
    autocomplete_lvar lid env;
    super#on_Lvar env lid

  method! on_Class_get env cid mid =
    autocomplete_static_member env cid mid;
    super#on_Class_get env cid mid

  method! on_Class_const env cid mid =
    autocomplete_static_member env cid mid;
    super#on_Class_const env cid mid

  method! on_Obj_get env obj mid ognf =
    (match mid with
    | _, Tast.Id mid ->
      autocomplete_typed_member ~is_static:false env (Tast.get_type obj) None mid
    | _ -> ()
    );
    super#on_Obj_get env obj mid ognf

  method! on_expr env expr =
    (match expr with
    | (_, Tast.Array_get (arr, Some ((pos, _), key))) ->
      let ty = Tast.get_type arr in
      let _, ty = Tast_env.expand_type env ty in
      begin match ty with
      | _, Typing_defs.Tshape (_, fields) ->
        (match key with
        | Tast.Id (_, mid) -> autocomplete_shape_key env fields (pos, mid);
        | Tast.String mid ->
          (* autocomplete generally assumes that there's a token ending with the suffix; *)
          (* This isn't the case for `$shape['a`, unless it's at the end of the file *)
          let offset = String_utils.substring_index auto_complete_suffix mid in
          if offset = -1
          then autocomplete_shape_key env fields (pos, mid)
          else begin
            let mid = Str.string_before mid (offset + suffix_len) in
            let (line, bol, cnum) = Pos.line_beg_offset pos in
            let pos = Pos.make_from_lnum_bol_cnum
              ~pos_file:(Pos.filename pos)
              ~pos_start:(line, bol, cnum)
              ~pos_end:(line, bol, cnum + offset + suffix_len)
            in
            autocomplete_shape_key env fields (pos, mid);
          end
        | _ -> ()
        )
      | _ -> ()
      end;
    | _ -> ()
    );
    super#on_expr env expr

  method! on_Xml env sid attrs el =
    (* In the case where we're autocompleting an open XHP bracket but haven't
       typed anything beyond that yet:

          $x = <

       we'll end up with the following AST:

          (Xml () () (:AUTO332))

       In order to handle this, we strip off the leading `:` if one exists and
       use that as the search term. *)
    let trimmed_sid =
      snd sid
      |> Utils.strip_ns
      |> (fun s -> (fst sid, lstrip s ":"))
    in
    autocomplete_id trimmed_sid env;
    let cid = Nast.CI (sid, []) in
    Typing_lazy_heap.get_class (Tast_env.get_tcopt env) (snd sid)
    |> Option.iter ~f:begin fun c ->
      List.iter attrs ~f:begin function
        | Tast.Xhp_simple (id, _) ->
          autocomplete_member ~is_static:false env c (Some cid) id
        | Tast.Xhp_spread _ -> ()
      end
    end;
    super#on_Xml env sid attrs el

  method! on_class_ env cls =
    List.iter cls.Tast.c_uses ~f:begin fun hint ->
      match snd hint with
      | Aast.Happly (sid, params) ->
        autocomplete_trait_only sid;
        List.iter params (self#on_hint env)
      | _ -> ()
    end;
    (* If we don't clear out c_uses we'll end up overwriting the trait
       completion as soon as we get to on_Happly. *)
    super#on_class_ env {cls with Tast.c_uses = []}
end

let auto_complete_suffix_finder = object
  inherit [_] Tast.reduce
  method zero = false
  method plus = (||)
  method! on_Lvar () (_, id) =
    matches_auto_complete_suffix (Local_id.get_name id)
end

let method_contains_cursor = auto_complete_suffix_finder#on_method_ ()
let fun_contains_cursor = auto_complete_suffix_finder#on_fun_ ()

class local_types = object (self)
  inherit Tast_visitor.iter as super

  val mutable results = Local_id.Map.empty;
  val mutable after_cursor = false;

  method get_types tast =
    self#go tast;
    results

  method add id ty =
    (* If we already have a type for this identifier, don't overwrite it with
       results from after the cursor position. *)
    if not (Local_id.Map.mem id results && after_cursor) then
      results <- Local_id.Map.add id ty results

  method! on_fun_ env f =
    if fun_contains_cursor f then
      super#on_fun_ env f

  method! on_method_ env m =
    if method_contains_cursor m then begin
      if not (Tast_env.is_static env) then
        self#add Typing_defs.this (Tast_env.get_self_exn env);
      super#on_method_ env m
    end

  method! on_expr env e =
    let (_, ty), e_ = e in
    match e_ with
    | Tast.Lvar (_, id) ->
      if matches_auto_complete_suffix (Local_id.get_name id)
      then after_cursor <- true
      else self#add id ty
    | Tast.Binop (Ast.Eq _, e1, e2) ->
      (* Process the rvalue before the lvalue, since the lvalue is annotated
         with its type after the assignment. *)
      self#on_expr env e2;
      self#on_expr env e1;
    | _ -> super#on_expr env e

  method! on_fun_param _ fp =
    let id = Local_id.get fp.Tast.param_name in
    let _, ty = fp.Tast.param_annotation in
    self#add id ty
end

let compute_complete_local tast =
  new local_types#get_types tast
  |> Local_id.Map.iter begin fun x ty ->
    add_partial_result (Local_id.get_name x) (Phase.locl ty) Variable_kind None
  end

let reset () =
  auto_complete_for_global := "";
  argument_global_type := None;
  autocomplete_identifier := None;
  ac_env := None;
  autocomplete_results := [];
  autocomplete_is_complete := true

let go
    ~tcopt
    ~delimit_on_namespaces
    ~content_funs
    ~content_classes
    ~autocomplete_context
    tast
  =
  reset ();
  visitor#go tast;
  Errors.ignore_ begin fun () ->
    let completion_type = !argument_global_type in
    if completion_type = Some Acid ||
       completion_type = Some Acnew ||
       completion_type = Some Actype ||
       completion_type = Some Actrait_only
    then compute_complete_global
      ~tcopt ~delimit_on_namespaces ~autocomplete_context ~content_funs ~content_classes;
    if completion_type = Some Acprop then compute_complete_local tast;
    let env = match !ac_env with
      | Some e -> e
      | None -> Tast_env.empty tcopt
    in
    let filter_results (result: autocomplete_result) : bool =
      let kind = match result with
        | Partial res -> res.kind_
        | Complete res -> res.res_kind
      in
      match completion_type, kind with
      | Some Actrait_only, Trait_kind -> true
      | Some Actrait_only, _ -> false
      | _ -> true
    in
    let resolve (result: autocomplete_result) : complete_autocomplete_result =
      match result with
      | Partial res -> resolve_ty env autocomplete_context res ~delimit_on_namespaces
      | Complete res -> res
    in
    {
      With_complete_flag.is_complete = !autocomplete_is_complete;
      value = !autocomplete_results |> List.filter ~f:filter_results |> List.map ~f:resolve;
    }
  end
