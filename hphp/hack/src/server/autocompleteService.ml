(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core
open Reordered_argument_collections
open Typing_defs
open Utils
open String_utils

module Phase = Typing_phase

(* Details about functions to be added in json output *)
type func_param_result = {
    param_name     : string;
    param_ty       : string;
    param_variadic : bool;
  }

type func_details_result = {
    params    : func_param_result list;
    return_ty : string;
    min_arity : int;
  }

type autocomplete_kind =
  | Abstract_class_kind
  | Class_kind
  | Method_kind
  | Variable_kind
  | Interface_kind
  | Trait_kind
  | Enum_kind
  | Namespace_kind
  | Constructor_kind

(* Results ready to be displayed to the user *)
type complete_autocomplete_result = {
    res_pos      : Pos.absolute;
    res_ty       : string;
    res_name     : string;
    res_kind     : autocomplete_kind;
    expected_ty  : bool;
    func_details : func_details_result option;
  }

(* Results that still need a typing environment to convert ty information
   into strings *)
type partial_autocomplete_result = {
    ty   : Typing_defs.phase_ty;
    name : string;
    kind_: autocomplete_kind;
  }

type autocomplete_result =
  | Partial of partial_autocomplete_result
  | Complete of complete_autocomplete_result

(* The type returned to the client *)
type ide_result = {
  completions : complete_autocomplete_result list;
  char_at_pos : char;
}

type result = complete_autocomplete_result list

let ac_env = ref None
let ac_type = ref None
let autocomplete_results : autocomplete_result list ref = ref []

let auto_complete_suffix = "AUTO332"
let suffix_len = String.length auto_complete_suffix
let strip_suffix s = String.sub s 0 (String.length s - suffix_len)
let is_auto_complete x =
  if !autocomplete_results = []
  then begin
    String.length x >= suffix_len &&
    let suffix = String.sub x (String.length x - suffix_len) suffix_len in
    suffix = auto_complete_suffix
  end else
    false

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
  let expected_ty = res.expected_ty in
  Hh_json.JSON_Object [
      "name", Hh_json.JSON_String name;
      "type", Hh_json.JSON_String ty;
      "pos", Pos.json pos;
      "func_details", func_details_to_json res.func_details;
      "expected_ty", Hh_json.JSON_Bool expected_ty;
  ]

let autocomplete_result_to_ide_response res =
  let param_to_ide_response x = {
    Ide_message.callable_param_name = x.param_name;
    callable_param_type = x.param_ty;
  } in

  let callable_details_to_ide_response res = {
    Ide_message.return_type = res.return_ty;
    callable_params = List.map res.params param_to_ide_response;
  } in

  let callable_details_to_ide_response =
    Option.map ~f:callable_details_to_ide_response in

  let autocomplete_result_to_ide_response res = {
    Ide_message.autocomplete_item_text = res.res_name;
    autocomplete_item_type = res.res_ty;
    callable_details = callable_details_to_ide_response res.func_details;
  } in
  Ide_message.Autocomplete_response
    (List.map res.completions ~f:autocomplete_result_to_ide_response)

let get_partial_result name ty kind =
  Partial { ty; name; kind_=kind; }

let add_res (res: autocomplete_result) : unit =
  autocomplete_results := res :: !autocomplete_results

let add_partial_result name ty kind =
  add_res (get_partial_result name ty kind)

let autocomplete_token ac_type env x =
  if is_auto_complete (snd x)
  then begin
    ac_env := env;
    Autocomplete.auto_complete_pos := Some (fst x);
    Autocomplete.argument_global_type := Some ac_type;
    Autocomplete.auto_complete_for_global := snd x
  end

let autocomplete_id id env = autocomplete_token Autocomplete.Acid (Some env) id

let autocomplete_hint = autocomplete_token Autocomplete.Actype None

let autocomplete_new cid env _ =
  match cid with
  | Nast.CI (sid, _) -> autocomplete_token Autocomplete.Acnew (Some env) sid
  | _ -> ()

let get_class_elt_types env class_ cid elts =
  let elts = SMap.filter elts begin fun _ x ->
    Typing_visibility.is_visible env x.ce_visibility cid class_
  end in
  SMap.map elts (fun { ce_type = lazy ty; _ } -> ty)

let autocomplete_method is_static class_ ~targs:_ id env cid
    ~is_method:_ ~is_const:_ =
  if is_auto_complete (snd id)
  then begin
    ac_env := Some env;
    Autocomplete.auto_complete_pos := Some (fst id);
    Autocomplete.argument_global_type := Some Autocomplete.Acclass_get;
    let results =
      if is_static
      then
        let elts = SMap.union class_.tc_smethods class_.tc_sprops in
        let elt_types = get_class_elt_types env class_ cid elts in
        SMap.fold class_.tc_consts ~f:begin fun x class_const acc ->
          SMap.add acc x class_const.cc_type
        end ~init:elt_types
      else
        let elts = SMap.union class_.tc_methods class_.tc_props in
        get_class_elt_types env class_ cid elts
    in
    SMap.iter results begin fun x ty ->
      add_partial_result x (Phase.decl ty) Method_kind
    end;
  end

let autocomplete_smethod = autocomplete_method true

let autocomplete_cmethod = autocomplete_method false

let autocomplete_lvar_naming _ id locals =
  if is_auto_complete (snd id)
  then begin
    Autocomplete.argument_global_type := Some Autocomplete.Acprop;
    (* Store the position and a map of name to ident so we can add
     * types at this point later *)
    Autocomplete.auto_complete_pos := Some (fst id);
    Autocomplete.auto_complete_vars := SMap.map locals snd
  end

let autocomplete_lvar_typing id env =
  if Some (fst id)= !(Autocomplete.auto_complete_pos)
  then begin
    (* The typechecker might call this hook more than once (loops) so we
     * need to clear the list of results first or we could have repeat locals *)
    autocomplete_results := [];
    ac_env := Some env;
    Autocomplete.auto_complete_pos := Some (fst id);
    (* Get the types of all the variables in scope at this point *)
    SMap.iter !Autocomplete.auto_complete_vars begin fun x ident ->
      let _, ty = Typing_env.get_local env ident in
      add_partial_result x (Phase.locl ty) Variable_kind
    end;
    (* Add $this if we're in a instance method *)
    let ty = Typing_env.get_self env in
    if not (Typing_env.is_static env) && (fst ty) <> Reason.Rnone
    then add_partial_result
      Naming_special_names.SpecialIdents.this (Phase.locl ty) Variable_kind
  end

let should_complete_class completion_type class_kind =
  match completion_type, class_kind with
  | Some Autocomplete.Acid, Some Ast.Cnormal
  | Some Autocomplete.Acid, Some Ast.Cabstract
  | Some Autocomplete.Acnew, Some Ast.Cnormal
  | Some Autocomplete.Actype, Some _ -> true
  | _ -> false

let should_complete_fun completion_type =
  completion_type=Some Autocomplete.Acid

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
      reason, Typing_defs.Tfun (Typing_env.make_ft pos [] return_ty)

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
let compute_complete_global
  ~(tcopt: TypecheckerOptions.t)
  ~(delimit_on_namespaces: bool)
  ~(content_funs: Reordered_argument_collections.SSet.t)
  ~(content_classes: Reordered_argument_collections.SSet.t)
  : unit =
  let completion_type = !Autocomplete.argument_global_type in
  let gname = Utils.strip_ns !Autocomplete.auto_complete_for_global in
  let gname = strip_suffix gname in

  (* Objective: given "fo|", we should suggest functions in both the current *)
  (* namespace and also in the global namespace. We'll use gname_gns to      *)
  (* indicate what prefix to look for (if any) in the global namespace...    *)

  (* "$z = S|"      => gname = "S",      gname_gns = Some "S"   *)
  (* "$z = Str|"    => gname = "Str",    gname_gns = Some "Str" *)
  (* "$z = Str\\|"  => gname = "Str\\",  gname_gns = None       *)
  (* "$z = Str\\s|" => gname = "Str\\s", gname_gns = None       *)
  (* "$z = \\s|"    => gname = "\\s",    gname_gns = None       *)
  (* "...; |"       => gname = "",       gname_gns = Some ""    *)
  let gname_gns = if should_complete_fun completion_type then
    (* Disgusting hack alert!
     *
     * In PHP/Hack, namespaced function lookup falls back into the global
     * namespace if no function in the current namespace exists. The
     * typechecker knows everything that exists, and resolves all of this
     * during naming -- meaning that by the time that we get to typing, not
     * only has "gname" been fully qualified, but we've lost whatever it
     * might have looked like originally. This makes it tough to do the full
     * namespace fallback behavior here -- we'd like to know if whatever
     * "gname" corresponds to in the source code has a '\' to qualify it, but
     * since it's already fully qualified here, we can't know.
     *
     *   [NOTE(ljw): is this really even true? if user wrote "foo|" then name
     *   lookup of "fooAUTO332" is guaranteed to fail in the current namespace,
     *   and guaranteed to fail in the global namespace, so how would the
     *   typechecker fully qualify it? to what? Experimentally I've only
     *   ever seen gname to be the exact string that the user typed.]
     *
     * Except, we can kinda reverse engineer and figure it out. We have the
     * positional information, which we can use to figure out how long the
     * original source code token was, and then figure out what portion of
     * "gname" that corresponds to, and see if it has a '\'. Since fully
     * qualifying a name will always prepend, this all works.
     *)
    match !Autocomplete.auto_complete_pos with
      | None -> None
      | Some p ->
          let len = (Pos.length p) - suffix_len in
          let start = String.length gname - len in
          if start < 0 || String.contains_from gname start '\\'
          then None else Some (strip_all_ns gname)
    else None in

  let does_fully_qualified_name_match_prefix ?(funky_gns_rules=false) name =
    let stripped_name = strip_ns name in
    if delimit_on_namespaces then
      (* name must match gname, and have no additional namespace slashes, e.g. *)
      (* name="Str\\co" gname="S" -> false *)
      (* name="Str\\co" gname="Str\\co" -> true *)
      string_starts_with stripped_name gname &&
        not (String.contains_from stripped_name (String.length gname) '\\')
    else
      match gname_gns with
      | _ when string_starts_with stripped_name gname -> true
      | Some gns when funky_gns_rules -> string_starts_with stripped_name gns
      | _ -> false
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
      if completion_type = Some Autocomplete.Acnew then
        get_partial_result
          (string_to_replace_prefix name) (Phase.decl (get_constructor_ty c)) Constructor_kind
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
        get_partial_result (string_to_replace_prefix name) (Phase.decl ty) kind
    )
  in

  let on_function name ~seen =
    if SSet.mem seen name then None else
    if not (should_complete_fun completion_type) then None else
    if not (does_fully_qualified_name_match_prefix ~funky_gns_rules:true name) then None else
    Option.map (Typing_lazy_heap.get_fun tcopt name) ~f:(fun fun_ ->
      incr result_count;
      let ty = Typing_reason.Rwitness fun_.Typing_defs.ft_pos, Typing_defs.Tfun fun_ in
      get_partial_result (string_to_replace_prefix name) (Phase.decl ty) Method_kind
    )
  in

  (* Try using the names in local content buffer first *)
  List.iter
    (List.filter_map (SSet.elements content_classes) (on_class ~seen:SSet.empty))
      add_res;
  List.iter
    (List.filter_map (SSet.elements content_funs) (on_function ~seen:SSet.empty))
      add_res;

  (* Use search results to look for matches, while excluding names we have
   * already seen in local content buffer *)
  let gname_results = search_funs_and_classes gname ~limit:(Some 100)
    ~on_class:(on_class ~seen:content_classes)
    ~on_function:(on_function ~seen:content_funs)
  in
  List.iter gname_results add_res;

  (* Compute global namespace fallback results for functions, if applicable *)
  match gname_gns with
  | Some gname_gns when gname <> gname_gns ->
    let gname_gns_results = search_funs_and_classes gname_gns ~limit:(Some 100)
      ~on_class:(fun _ -> None)
      ~on_function:(on_function ~seen:content_funs)
    in
    List.iter gname_gns_results add_res;
  | _ -> ()

let process_fun_call fun_args used_args _env =
  let is_target target_pos p =
    let line, char_pos, _ = Pos.info_pos target_pos in
    let start_line, start_col, end_col = Pos.info_pos p in
    start_line = line && start_col <= char_pos && char_pos - 1 <= end_col
  in
  match !Autocomplete.auto_complete_pos with
    | Some pos when !ac_type = None ->
        (* This function gets called on the 'way up' of the recursion that
         * processes args. Therefore, inner arguments will hit this function
         * first, so we only care when we don't have a result yet. This has to
         * happen on the way up because autocomplete pos needs to get set
         * before this is called *)
        let argument_index = ref (-1) in
        List.iteri used_args begin fun index arg ->
          if is_target pos arg then argument_index := index;
        end;
        begin try
          let _, arg_ty = List.nth_exn fun_args !argument_index in
          ac_type := Some arg_ty
        with
          | Failure _ ->
              (* They're specifying too many args, so we'll accept anything *)
              ac_type := Some (Typing_reason.Rnone, Typing_defs.Tany)
          | Invalid_argument _ ->
              (* Never matched at all*)
              ()
        end
    | _ -> ()

let rec result_matches_expected_ty ty =
  match !ac_type, !ac_env with
    | Some goal_type, Some env ->
        (match goal_type, ty with
          | (_, Tany), _ | _, (_, Tany) ->
              (* Everything will just be a match so this is pointless *)
              false
          | _, (_, Tfun fun_) ->
              (* if this is a function, we'll check if the return type
               * is a good result as well TODO: stop after enough levels
               * and explore methods on the objects as well *)
              if Typing_subtype.is_sub_type env ty goal_type then true
              else result_matches_expected_ty fun_.Typing_defs.ft_ret
          | _ -> Typing_subtype.is_sub_type env ty goal_type)
    | _ -> false

(* Here we turn partial_autocomplete_results into complete_autocomplete_results *)
(* by using typing environment to convert ty information into strings. *)
let resolve_ty (env: Typing_env.env) (x: partial_autocomplete_result)
    : complete_autocomplete_result =
  let env, ty = match x.ty with
    | DeclTy ty -> Phase.localize_with_self env ty
    | LoclTy ty -> env, ty
  in
  let desc_string = match x.kind_ with
    | Method_kind
    | Variable_kind
    | Constructor_kind -> Typing_print.full_strip_ns env ty
    | Abstract_class_kind -> "abstract class"
    | Class_kind -> "class"
    | Interface_kind -> "interface"
    | Trait_kind -> "trait"
    | Enum_kind -> "enum"
    | Namespace_kind -> "namespace"
  in
  let func_details = match ty with
    | (_, Tfun ft) ->
      let param_to_record ?(is_variadic=false) (name, pty) =
        {
          param_name     = (match name with
                             | Some n -> n
                             | None -> "");
          param_ty       = Typing_print.full_strip_ns env pty;
          param_variadic = is_variadic;
        }
      in
      Some {
        return_ty = Typing_print.full_strip_ns env ft.ft_ret;
        min_arity = arity_min ft.ft_arity;
        params    = List.map ft.ft_params param_to_record @
          (match ft.ft_arity with
             | Fellipsis _ -> let empty = (None, (Reason.none, Tany)) in
                              [param_to_record ~is_variadic:true empty]
             | Fvariadic (_, p) -> [param_to_record ~is_variadic:true p]
             | Fstandard _ -> [])
      }
    | _ -> None
  in
  let expected_ty = result_matches_expected_ty ty in
  let pos = Typing_reason.to_pos (fst ty)
  in
  {
    res_pos      = Pos.to_absolute pos;
    res_ty       = desc_string;
    res_name     = x.name;
    res_kind     = x.kind_;
    expected_ty  = expected_ty;
    func_details = func_details;
  }

let result_compare a b =
  if a.expected_ty = b.expected_ty then
    String.compare a.res_name b.res_name
  else if a.expected_ty then -1
  else 1

let get_results ~tcopt ~delimit_on_namespaces ~content_funs ~content_classes =
  Errors.ignore_ begin fun () ->
    let completion_type = !Autocomplete.argument_global_type in
    if completion_type = Some Autocomplete.Acid ||
       completion_type = Some Autocomplete.Acnew ||
       completion_type = Some Autocomplete.Actype
    then compute_complete_global ~tcopt ~delimit_on_namespaces ~content_funs ~content_classes;
    let results = !autocomplete_results in
    let env = match !ac_env with
      | Some e -> e
      | None -> Typing_env.empty tcopt Relative_path.default ~droot:None
    in
    let resolve (result: autocomplete_result) : complete_autocomplete_result =
      match result with
      | Partial res -> resolve_ty env res
      | Complete res -> res
    in
    results |> List.map ~f:resolve |> List.sort ~cmp:result_compare
end

let reset () =
  Autocomplete.auto_complete_for_global := "";
  Autocomplete.argument_global_type := None;
  Autocomplete.auto_complete_pos := None;
  Autocomplete.auto_complete_vars := SMap.empty;
  ac_env := None;
  ac_type := None;
  autocomplete_results := []

let attach_hooks () =
  reset();
  Autocomplete.auto_complete := true;
  Typing_hooks.attach_id_hook autocomplete_id;
  Typing_hooks.attach_smethod_hook autocomplete_smethod;
  Typing_hooks.attach_cmethod_hook autocomplete_cmethod;
  Typing_hooks.attach_lvar_hook autocomplete_lvar_typing;
  Typing_hooks.attach_fun_call_hook process_fun_call;
  Typing_hooks.attach_new_id_hook autocomplete_new;
  Naming_hooks.attach_hint_hook autocomplete_hint;
  Naming_hooks.attach_lvar_hook autocomplete_lvar_naming

let detach_hooks () =
  reset();
  Autocomplete.auto_complete := false;
  Typing_hooks.remove_all_hooks();
  Naming_hooks.remove_all_hooks()
