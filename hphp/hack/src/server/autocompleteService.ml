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
open Typing_defs_core
open Utils
open String_utils
open SearchUtils
open SearchTypes
include AutocompleteTypes
open Tast
module Phase = Typing_phase
module Cls = Decl_provider.Class
module Syntax = Full_fidelity_positioned_syntax
module Trivia = Full_fidelity_positioned_trivia

(* When autocompletion is called, we insert an autocomplete token
   "AUTO332" at the position of the cursor. For example, if the user
   has this in their IDE, with | showing their cursor:

       $x = new Fo|

   We see the following TAST:

       $x = new FoAUTO332

   This allows us to walk the syntax tree and identify the position
   of the cursor by looking for names containing "AUTO332". *)

let autocomplete_items : autocomplete_item list ref = ref []

let autocomplete_is_complete : bool ref = ref true

(*
 * Take the results, look them up, and add file position information.
 *)
let add_position_to_results
    (ctx : Provider_context.t) (raw_results : SearchTypes.si_item list) :
    SearchUtils.result =
  SearchUtils.(
    List.filter_map raw_results ~f:(fun r ->
        match SymbolIndexCore.get_pos_for_item_opt ctx r with
        | Some pos ->
          Some { name = r.si_fullname; pos; result_type = r.si_kind }
        | None -> None))

let max_results = 100

let auto_complete_for_global = ref ""

let strip_suffix s =
  String.sub
    s
    ~pos:0
    ~len:(String.length s - AutocompleteTypes.autocomplete_token_length)

let strip_supportdyn_decl ty =
  match Typing_defs.get_node ty with
  | Tapply ((_, n), [ty])
    when String.equal n Naming_special_names.Classes.cSupportDyn ->
    ty
  | _ -> ty

let strip_like_decl ty =
  match get_node ty with
  | Tlike ty -> ty
  | _ -> ty

(* We strip off ~ from types before matching on the underlying structure of the type.
 * Note that we only do this for matching, not pretty-printing of types, as we
 * want to retain ~ when showing types in suggestions
 *)
let expand_and_strip_dynamic env ty =
  let (_, ty) = Tast_env.expand_type env ty in
  let ty =
    Typing_utils.strip_dynamic (Tast_env.tast_env_as_typing_env env) ty
  in
  Tast_env.expand_type env ty

let expand_and_strip_supportdyn env ty =
  let (env, ty) = expand_and_strip_dynamic env ty in
  let (_, _, ty) =
    Typing_utils.strip_supportdyn (Tast_env.tast_env_as_typing_env env) ty
  in
  ty

let expand_and_strip_dynamic env ty =
  let (_, ty) = expand_and_strip_dynamic env ty in
  ty

let matches_auto_complete_suffix x =
  String.length x >= AutocompleteTypes.autocomplete_token_length
  &&
  let suffix =
    String.sub
      x
      ~pos:(String.length x - AutocompleteTypes.autocomplete_token_length)
      ~len:AutocompleteTypes.autocomplete_token_length
  in
  String.equal suffix AutocompleteTypes.autocomplete_token

(* Does [str] look like something we should offer code completion on? *)
let is_auto_complete str : bool =
  let results_without_keywords =
    List.filter !autocomplete_items ~f:(fun res ->
        match res.res_kind with
        | FileInfo.SI_Keyword -> false
        | _ -> true)
  in
  if List.is_empty results_without_keywords then
    matches_auto_complete_suffix str
  else
    false

let replace_pos_of_id ?(strip_xhp_colon = false) (pos, text) :
    Ide_api_types.range =
  let (_, name) = Utils.split_ns_from_name text in
  let name =
    if matches_auto_complete_suffix name then
      strip_suffix name
    else
      name
  in
  let name =
    if strip_xhp_colon then
      Utils.strip_xhp_ns name
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
  let name = res.res_label in
  let pos = res.res_decl_pos in
  let ty = res.res_detail in
  Hh_json.JSON_Object
    [
      ("name", Hh_json.JSON_String name);
      ("type", Hh_json.JSON_String ty);
      ("pos", Pos.json pos);
      ("expected_ty", Hh_json.JSON_Bool false);
      (* legacy field, left here in case clients need it *)
    ]

let add_res (res : autocomplete_item) : unit =
  autocomplete_items := res :: !autocomplete_items

let autocomplete_id (id : Aast.sid) : unit =
  if is_auto_complete (snd id) then auto_complete_for_global := snd id

let get_class_elt_types ~is_method env class_ cid elts =
  let is_visible (_, elt) =
    Tast_env.is_visible
      ~is_method
      env
      (elt.ce_visibility, get_ce_lsb elt)
      cid
      class_
  in
  elts
  |> List.filter ~f:is_visible
  |> List.map ~f:(fun (id, { ce_type = (lazy ty); _ }) -> (id, ty))

let get_class_req_attrs env pctx classname cid =
  let req_attrs cls =
    Cls.props cls
    |> List.filter ~f:(fun (_, ce) ->
           Tast_env.is_visible
             ~is_method:false
             env
             (ce.ce_visibility, get_ce_lsb ce)
             cid
             cls)
    |> List.filter ~f:(fun (_, ce) ->
           Xhp_attribute.opt_is_required (Typing_defs.get_ce_xhp_attr ce))
    |> List.map ~f:(fun (name, _) -> Utils.strip_xhp_ns name)
  in
  Decl_provider.get_class pctx classname
  |> Option.value_map ~default:[] ~f:req_attrs

let get_class_is_child_empty pctx classname =
  Decl_provider.get_class pctx classname
  |> Option.value_map ~default:false ~f:Cls.xhp_marked_empty

let get_pos_for (env : Tast_env.env) (ty : Typing_defs.phase_ty) : Pos.absolute
    =
  (match ty with
  | LoclTy loclt -> loclt |> Typing_defs.get_pos
  | DeclTy declt -> declt |> Typing_defs.get_pos)
  |> ServerPos.resolve env
  |> Pos.to_absolute

let snippet_for_params (params : 'a Typing_defs.fun_param list) : string =
  (* A function call snippet looks like this:

     fun_name(${1:\$first_arg}, ${2:\$second_arg})

     The syntax for snippets is described here:
     https://code.visualstudio.com/docs/editor/userdefinedsnippets#_variables *)
  let param_templates =
    List.mapi params ~f:(fun i param ->
        match param.fp_name with
        | Some param_name -> Printf.sprintf "${%d:\\%s}" (i + 1) param_name
        | None -> Printf.sprintf "${%d}" (i + 1))
  in
  String.concat ~sep:", " param_templates

let get_snippet_for_xhp_req_attrs tag attrs has_children =
  let content =
    if not (List.is_empty attrs) then
      let attr_content =
        List.mapi attrs ~f:(fun i name ->
            Format.sprintf "%s={${%d}}" name (i + 1))
        |> String.concat ~sep:" "
      in
      tag ^ " " ^ attr_content
    else
      tag
  in
  if has_children then
    Format.sprintf "%s>$0</%s>" content tag
  else
    content ^ " />"

let insert_text_for_xhp_req_attrs tag attrs has_children =
  let content = get_snippet_for_xhp_req_attrs tag attrs has_children in
  let insert_type = (not (List.is_empty attrs)) && has_children in
  if insert_type then
    InsertAsSnippet { snippet = content; fallback = tag }
  else
    InsertLiterally content

let get_snippet_for_xhp_classname cls ctx env =
  (* This is used to check if the class exists or not *)
  let class_ = Decl_provider.get_class ctx cls in
  match class_ with
  | None -> None
  | Some class_ ->
    if Cls.is_xhp class_ then
      let cls = Utils.add_ns cls in
      let attrs = get_class_req_attrs env ctx cls None in
      let has_children = not (get_class_is_child_empty ctx cls) in
      Option.some
        (get_snippet_for_xhp_req_attrs
           (Utils.strip_both_ns cls)
           attrs
           has_children)
    else
      None

(* If we're autocompleting a call (function or method), insert a
   template for the arguments as well as function name. *)
let insert_text_for_fun_call
    env
    (autocomplete_context : legacy_autocomplete_context)
    (fun_name : string)
    (ft : Typing_defs.locl_fun_type) : insert_text =
  if Char.equal autocomplete_context.char_at_pos '(' then
    (* Arguments are present already, e.g. the user has ->AUTO332(1, 2).
       We don't want to insert params when we complete the name, or we
       end up with ->methName()(1, 2). *)
    InsertLiterally fun_name
  else
    let arity =
      if Tast_env.is_in_expr_tree env then
        (* Helper functions in expression trees don't have relevant
           parameter names on their decl, so don't autofill
           parameters.*)
        0
      else
        arity_min ft
    in
    let fallback = Printf.sprintf "%s()" fun_name in
    if arity = 0 then
      InsertLiterally fallback
    else
      let params = List.take ft.ft_params arity in
      let snippet =
        Printf.sprintf "%s(%s)" fun_name (snippet_for_params params)
      in
      InsertAsSnippet { snippet; fallback }

let insert_text_for_ty
    env
    (autocomplete_context : legacy_autocomplete_context)
    (label : string)
    (ty : phase_ty) : insert_text =
  let (env, ty) =
    match ty with
    | LoclTy locl_ty -> (env, locl_ty)
    | DeclTy decl_ty ->
      Tast_env.localize_no_subst env ~ignore_errors:true decl_ty
  in
  (* Functions that support dynamic will be wrapped by supportdyn<_> *)
  let ty = expand_and_strip_supportdyn env ty in
  match Typing_defs.get_node ty with
  | Tfun ft -> insert_text_for_fun_call env autocomplete_context label ft
  | _ -> InsertLiterally label

let autocomplete_shape_key autocomplete_context env fields id =
  if is_auto_complete (snd id) then
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
          (str, FileInfo.SI_Literal, Typing_defs.mk (reason, ty))
        | Typing_defs.TSFlit_str (pos, str) ->
          let reason = Typing_reason.Rwitness_from_decl pos in
          let ty = Typing_defs.Tprim Aast_defs.Tstring in
          let quote =
            if have_prefix then
              Str.first_chars prefix 1
            else
              "'"
          in
          (quote ^ str ^ quote, FileInfo.SI_Literal, Typing_defs.mk (reason, ty))
        | Typing_defs.TSFclass_const ((pos, cid), (_, mid)) ->
          ( Printf.sprintf "%s::%s" cid mid,
            FileInfo.SI_ClassConstant,
            Typing_defs.mk
              (Reason.Rwitness_from_decl pos, Typing_defs.make_tany ()) )
      in
      if (not have_prefix) || String.is_prefix code ~prefix then
        let ty = Phase.decl ty in
        let pos = get_pos_for env ty in
        let res_insert_text =
          if autocomplete_context.is_before_apostrophe then
            rstrip code "'"
          else
            code
        in
        let complete =
          {
            res_decl_pos = pos;
            res_replace_pos = replace_pos_of_id id;
            res_base_class = None;
            res_detail = kind_to_string kind;
            res_insert_text = InsertLiterally res_insert_text;
            res_label = res_insert_text;
            res_fullname = code;
            res_kind = kind;
            res_documentation = None;
            res_filter_text = None;
            res_additional_edits = [];
            res_sortText = None;
          }
        in
        add_res complete
    in

    List.iter (TShapeMap.keys fields) ~f:add

let autocomplete_member ~is_static autocomplete_context env class_ cid id =
  (* This is used for instance "$x->|" and static "Class1::|" members. *)
  if is_auto_complete (snd id) then (
    (* Detect usage of "parent::|" which can use both static and instance *)
    let parent_receiver =
      match cid with
      | Some Aast.CIparent -> true
      | _ -> false
    in

    let add kind (name, ty) =
      (* Functions that support dynamic will be wrapped by supportdyn<_> *)
      let ty = strip_supportdyn_decl ty in
      let res_detail =
        match Typing_defs.get_node ty with
        | Tfun _ ->
          String_utils.rstrip
            (String_utils.lstrip (Tast_env.print_decl_ty env ty) "(")
            ")"
        | _ -> Tast_env.print_decl_ty env ty
      in
      let ty = Phase.decl ty in
      let complete =
        {
          res_decl_pos = get_pos_for env ty;
          res_replace_pos = replace_pos_of_id id;
          res_base_class = Some (Cls.name class_);
          res_detail;
          res_label = name;
          res_insert_text = insert_text_for_ty env autocomplete_context name ty;
          res_fullname = name;
          res_kind = kind;
          res_documentation = None;
          res_filter_text = None;
          res_additional_edits = [];
          res_sortText = None;
        }
      in
      add_res complete
    in

    let sort : 'a. (string * 'a) list -> (string * 'a) list =
     fun list ->
      List.sort ~compare:(fun (a, _) (b, _) -> String.compare a b) list
    in
    (* There's no reason for us to sort -- we can expect our client to do its
       own sorting of our results -- but having a sorted list here makes our tests
       more stable. *)
    if is_static || parent_receiver then (
      List.iter
        (get_class_elt_types
           ~is_method:true
           env
           class_
           cid
           (Cls.smethods class_ |> sort))
        ~f:(add FileInfo.SI_ClassMethod);
      List.iter
        (get_class_elt_types
           ~is_method:false
           env
           class_
           cid
           (Cls.sprops class_ |> sort))
        ~f:(add FileInfo.SI_Property);
      List.iter
        (Cls.consts class_ |> sort)
        ~f:(fun (name, cc) -> add FileInfo.SI_ClassConstant (name, cc.cc_type))
    );
    if (not is_static) || parent_receiver then (
      List.iter
        (get_class_elt_types
           ~is_method:true
           env
           class_
           cid
           (Cls.methods class_ |> sort))
        ~f:(add FileInfo.SI_ClassMethod);
      List.iter
        (get_class_elt_types
           ~is_method:false
           env
           class_
           cid
           (Cls.props class_ |> sort))
        ~f:(add FileInfo.SI_Property)
    );
    (* Only complete __construct() when we see parent::, as we don't
       allow __construct to be called as e.g. $foo->__construct(). *)
    if parent_receiver then
      let (constructor, _) = Cls.construct class_ in
      let constructor =
        Option.map constructor ~f:(fun elt ->
            (Naming_special_names.Members.__construct, elt))
      in
      List.iter
        (get_class_elt_types
           ~is_method:true
           env
           class_
           cid
           (Option.to_list constructor))
        ~f:(add FileInfo.SI_ClassMethod)
  )

(*
  Autocompletion for XHP attribute names in an XHP literal.
*)
let autocomplete_xhp_attributes env class_ cid id attrs =
  (* This is used for "<nt:fb:text |" XHP attributes, in which case  *)
  (* class_ is ":nt:fb:text" and its attributes are in tc_props.     *)
  if is_auto_complete (snd id) && Cls.is_xhp class_ then
    let existing_attr_names : SSet.t =
      attrs
      |> List.filter_map ~f:(fun attr ->
             match attr with
             | Aast.Xhp_simple { Aast.xs_name = id; _ } -> Some (snd id)
             | Aast.Xhp_spread _ -> None)
      |> List.filter ~f:(fun name -> not (matches_auto_complete_suffix name))
      |> SSet.of_list
    in
    List.iter
      (get_class_elt_types ~is_method:false env class_ cid (Cls.props class_))
      ~f:(fun (name, ty) ->
        if
          not
            (SSet.exists
               (fun key -> String.equal (":" ^ key) name)
               existing_attr_names)
        then
          let kind = FileInfo.SI_Property in
          let res_detail = Tast_env.print_decl_ty env ty in
          let ty = Phase.decl ty in
          let complete =
            {
              res_decl_pos = get_pos_for env ty;
              res_replace_pos = replace_pos_of_id id;
              res_base_class = Some (Cls.name class_);
              res_detail;
              res_label = lstrip name ":";
              res_insert_text = InsertLiterally (lstrip name ":");
              res_fullname = name;
              res_kind = kind;
              res_documentation = None;
              res_filter_text = None;
              res_additional_edits = [];
              res_sortText = None;
            }
          in
          add_res complete)

let autocomplete_xhp_bool_value attr_ty id_id env =
  if is_auto_complete (snd id_id) then begin
    let is_bool_or_bool_option ty : bool =
      let is_bool = function
        | Tprim Tbool -> true
        | _ -> false
      in
      let ty = expand_and_strip_dynamic env ty in
      let (_, ty_) = Typing_defs_core.deref ty in
      match ty_ with
      | Toption ty -> is_bool (get_node (expand_and_strip_dynamic env ty))
      | _ -> is_bool ty_
    in

    if is_bool_or_bool_option attr_ty then (
      let kind = FileInfo.SI_Literal in
      let ty = Phase.locl attr_ty in
      let complete =
        {
          res_decl_pos = get_pos_for env ty;
          res_replace_pos = replace_pos_of_id id_id;
          res_base_class = None;
          res_detail = kind_to_string kind;
          res_label = "true";
          res_insert_text = InsertLiterally "true";
          res_fullname = "true";
          res_kind = kind;
          res_documentation = None;
          res_filter_text = None;
          res_additional_edits = [];
          res_sortText = None;
        }
      in

      add_res complete;
      add_res
        {
          complete with
          res_label = "false";
          res_insert_text = InsertLiterally "false";
          res_fullname = "false";
        }
    )
  end

(*
  Autocompletion for the value of an attribute in an XHP literals with the enum type,
  defined with the following syntax:
  attribute enum {"some value", "some other value"} my-attribute;
*)
let autocomplete_xhp_enum_attribute_value attr_name ty id_id env cls =
  if is_auto_complete (snd id_id) then begin
    let attr_origin =
      Cls.props cls
      |> List.find ~f:(fun (name, _) -> String.equal (":" ^ attr_name) name)
      |> Option.map ~f:(fun (_, { ce_origin = n; _ }) -> n)
      |> Option.bind ~f:(fun cls_name ->
             Decl_provider.get_class (Tast_env.get_ctx env) cls_name)
    in

    let enum_values =
      match attr_origin with
      | Some cls -> Cls.xhp_enum_values cls
      | None -> SMap.empty
    in

    let add_enum_value_result xev =
      let suggestion = function
        | Ast_defs.XEV_Int value -> string_of_int value
        | Ast_defs.XEV_String value -> "\"" ^ value ^ "\""
      in
      let name = suggestion xev in
      let kind = FileInfo.SI_Enum in
      let ty = Phase.locl ty in
      let complete =
        {
          res_decl_pos = get_pos_for env ty;
          res_replace_pos = replace_pos_of_id id_id;
          res_base_class = None;
          res_detail = kind_to_string kind;
          res_label = name;
          res_insert_text = InsertLiterally name;
          res_fullname = name;
          res_kind = kind;
          res_documentation = None;
          res_filter_text = None;
          res_additional_edits = [];
          res_sortText = None;
        }
      in
      add_res complete
    in

    match SMap.find_opt (":" ^ attr_name) enum_values with
    | Some enum_values -> List.iter enum_values ~f:add_enum_value_result
    | None -> ()
  end

(*
  Autocompletion for the value of an attribute in an XHP literals
  with type that is an enum class. i.e.

    enum MyEnum {}
    class :foo {
      attribute MyEnum my-attribute;
    }
*)
let autocomplete_xhp_enum_class_value attr_ty id_id env =
  if is_auto_complete (snd id_id) then begin
    let rec get_class_name_decl ty : string option =
      let ty = strip_like_decl ty in
      match get_node ty with
      | Toption ty -> get_class_name_decl ty
      | Tapply ((_, name), _) -> Some name
      | _ -> None
    in
    let rec get_class_name ty : string option =
      let ty = expand_and_strip_dynamic env ty in
      match get_node ty with
      | Toption ty -> get_class_name ty
      | Tnewtype (name, _, _) -> Some name
      | _ -> None
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
             is_correct_class (get_class_name_decl class_const.cc_type))
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
                  let dty = Phase.decl ty.cc_type in
                  let name = Utils.strip_ns class_name ^ "::" ^ const_name in
                  let kind = FileInfo.SI_Enum in
                  let res_base_class = Option.map ~f:Cls.name enum_class in

                  let complete =
                    {
                      res_decl_pos = get_pos_for env dty;
                      res_replace_pos = replace_pos_of_id id_id;
                      res_base_class;
                      res_detail = kind_to_string kind;
                      res_label = name;
                      res_insert_text = InsertLiterally name;
                      res_fullname = name;
                      res_kind = kind;
                      res_documentation = None;
                      res_filter_text = None;
                      res_additional_edits = [];
                      res_sortText = None;
                    }
                  in
                  add_res complete))
  end

(** Return all the paths of .hhi files. *)
let hhi_paths () : Relative_path.t list =
  let hhi_root = Path.make (Relative_path.path_of_prefix Relative_path.Hhi) in
  let as_abs_path hhi_path : Path.t = Path.concat hhi_root hhi_path in

  Hhi.get_raw_hhi_contents ()
  |> Array.to_list
  |> List.map ~f:(fun (fn, _) ->
         Relative_path.create
           Relative_path.Hhi
           (Path.to_string (as_abs_path fn)))

(** Return the fully qualified names of all functions in .hhi files, sorted
    alphabetically. *)
let hhi_funs (naming_table : Naming_table.t) : string list =
  hhi_paths ()
  |> List.map ~f:(fun fn ->
         match Naming_table.get_file_info naming_table fn with
         | Some info ->
           List.map info.FileInfo.funs ~f:(fun (_, name, _) -> name)
         | None -> [])
  |> List.concat
  |> List.sort ~compare:String.compare
  |> List.rev

(** Filter function names to those in namespaces can be used with value types. *)
let filter_fake_arrow_namespaces (fun_names : string list) : string list =
  let in_relevant_ns name : bool =
    let name = Utils.strip_hh_lib_ns name in
    String.is_prefix ~prefix:"C\\" name
    || String.is_prefix ~prefix:"Vec\\" name
    || String.is_prefix ~prefix:"Dict\\" name
    || String.is_prefix ~prefix:"Keyset\\" name
    || String.is_prefix ~prefix:"Str\\" name
  in
  List.filter fun_names ~f:in_relevant_ns

(** Create a fresh type variable for every item in [tparams]. *)
let fresh_tyvars (env : Tast_env.env) (tparams : 'a list) :
    Tast_env.env * locl_ty list =
  List.fold tparams ~init:(env, []) ~f:(fun (env, tvars) _ ->
      let (env, tvar) = Tast_env.fresh_type env Pos.none in
      (env, tvar :: tvars))

let fresh_expand_env
    (env : Tast_env.env) (tparams : decl_ty Typing_defs.tparam list) :
    Tast_env.env * expand_env =
  let (env, tyvars) = fresh_tyvars env tparams in
  let substs = Decl_subst.make_locl tparams tyvars in
  (env, { Typing_defs.empty_expand_env with substs })

(** Does Hack function [f] accept [arg_ty] as its first argument? *)
let fun_accepts_first_arg (env : Tast_env.env) (f : fun_elt) (arg_ty : locl_ty)
    : bool =
  (* Functions that support dynamic will be wrapped by supportdyn<_> *)
  let ty = strip_supportdyn_decl f.fe_type in
  match Typing_defs.get_node ty with
  | Tfun ft ->
    let params = ft.ft_params in
    (match List.hd params with
    | Some first_param ->
      let (env, ety_env) = fresh_expand_env env ft.ft_tparams in
      let (env, first_param) =
        Tast_env.localize env ety_env first_param.fp_type.et_type
      in
      (match get_node first_param with
      | Tvar _ ->
        (* Kludge: If inference found a type variable that it couldn't solve,
           we've probably violated a where constraint and shouldn't consider
           this function to be compatible. *)
        false
      | _ ->
        let arg_ty = expand_and_strip_dynamic env arg_ty in
        let first_param = expand_and_strip_dynamic env first_param in
        Tast_env.can_subtype env arg_ty first_param)
    | _ -> false)
  | _ -> false

(** Fetch decls for all the [fun_names] and filter to those where the first argument
    is compatible with [arg_ty]. *)
let compatible_fun_decls
    (env : Tast_env.env) (arg_ty : locl_ty) (fun_names : string list) :
    (string * fun_elt) list =
  List.filter_map fun_names ~f:(fun fun_name ->
      match Decl_provider.get_fun (Tast_env.get_ctx env) fun_name with
      | Some f when fun_accepts_first_arg env f arg_ty -> Some (fun_name, f)
      | _ -> None)

(** Is [ty] a value type where we want to allow -> to complete to a HSL function? *)
let is_fake_arrow_ty (env : Tast_env.env) (ty : locl_ty) : bool =
  let ty = expand_and_strip_dynamic env ty in
  let (_, ty_) = Typing_defs_core.deref ty in
  match ty_ with
  | Tclass ((_, name), _, _) ->
    String.equal Naming_special_names.Collections.cVec name
    || String.equal Naming_special_names.Collections.cDict name
    || String.equal Naming_special_names.Collections.cKeyset name
  | Tprim Tstring -> true
  | _ -> false

(** If the user is completing $v-> and $v is a vec/keyset/dict/string,
    offer HSL functions like C\count(). *)
let autocomplete_hack_fake_arrow
    (env : Tast_env.env)
    (recv : expr)
    (prop_name : sid)
    (naming_table : Naming_table.t) : unit =
  let prop_replace_pos = replace_pos_of_id prop_name in
  let prop_start = prop_replace_pos.Ide_api_types.st in
  let replace_pos =
    Ide_api_types.
      {
        prop_replace_pos with
        st = { prop_start with column = prop_start.column - 2 };
      }
  in

  let (recv_ty, recv_pos, _) = recv in
  if is_fake_arrow_ty env recv_ty && is_auto_complete (snd prop_name) then
    let recv_start_pos = Pos.shrink_to_start recv_pos in

    let fake_arrow_funs =
      filter_fake_arrow_namespaces (hhi_funs naming_table)
    in
    let compatible_funs = compatible_fun_decls env recv_ty fake_arrow_funs in

    List.iter compatible_funs ~f:(fun (fun_name, fun_decl) ->
        let name = Utils.strip_hh_lib_ns fun_name in
        let kind = FileInfo.SI_Function in

        (* We want to transform $some_vec-> to e.g.

           C\contains(some_vec, ${1:\$value})

           This requires an additional insertion "C\contains(" before
           the expression. *)
        let res_additional_edits =
          [
            ( Printf.sprintf "%s(" name,
              Ide_api_types.pos_to_range recv_start_pos );
          ]
        in

        (* Construct a snippet with placeholders for any additional
           required arguments for this function. For the C\contains()
           example, this is ", ${1:\$value})". *)
        let required_params =
          match Typing_defs.get_node fun_decl.fe_type with
          | Tfun ft -> List.take ft.ft_params (arity_min ft)
          | _ -> []
        in
        let params = List.drop required_params 1 in
        let res_insert_text =
          match params with
          | [] -> InsertLiterally ")"
          | params ->
            let snippet = Printf.sprintf ", %s)" (snippet_for_params params) in
            InsertAsSnippet { snippet; fallback = ")" }
        in

        let complete =
          {
            res_decl_pos =
              Pos.to_absolute (ServerPos.resolve env fun_decl.fe_pos);
            res_replace_pos = replace_pos;
            res_base_class = None;
            res_detail = Tast_env.print_decl_ty env fun_decl.fe_type;
            res_insert_text;
            (* VS Code uses filter text to decide which items match the current
               prefix. However, "C\contains" does not start with "->", so VS
               Code would normally ignore this completion item.

               We set the filter text to "->contains" so we offer C\contains if
               the user types e.g. "->" or "->co". *)
            res_filter_text =
              Some (Printf.sprintf "->%s" (Utils.strip_all_ns fun_name));
            res_label = name;
            res_fullname = name;
            res_kind = kind;
            res_documentation = None;
            res_additional_edits;
            res_sortText = None;
          }
        in
        add_res complete)

let autocomplete_typed_member
    ~is_static autocomplete_context env class_ty cid mid =
  Tast_env.get_class_ids env class_ty
  |> List.iter ~f:(fun cname ->
         Decl_provider.get_class (Tast_env.get_ctx env) cname
         |> Option.iter ~f:(fun class_ ->
                let cid = Option.map cid ~f:to_nast_class_id_ in
                autocomplete_member
                  ~is_static
                  autocomplete_context
                  env
                  class_
                  cid
                  mid))

let autocomplete_static_member autocomplete_context env (ty, _, cid) mid =
  autocomplete_typed_member
    ~is_static:true
    autocomplete_context
    env
    ty
    (Some cid)
    mid

let compatible_enum_class_consts env cls (expected_ty : locl_ty option) =
  (* Ignore ::class, as it's not a normal enum constant. *)
  let consts =
    List.filter (Cls.consts cls) ~f:(fun (name, _) ->
        String.(name <> Naming_special_names.Members.mClass))
  in

  (* All the constants that are compatible with [expected_ty]. *)
  let compatible_consts =
    List.filter consts ~f:(fun (_, cc) ->
        let (env, cc_ty) =
          Tast_env.localize_no_subst env ~ignore_errors:true cc.cc_type
        in
        match expected_ty with
        | Some expected_ty ->
          Tast_env.is_sub_type
            env
            cc_ty
            (Typing_make_type.locl_like Reason.Rnone expected_ty)
        | None -> true)
  in

  match compatible_consts with
  | [] ->
    (* If we couldn't find any constants that match the expected
       type, show all constants. We sometimes infer [nothing] for
       generics when a user hasn't passed all the arguments to a
       function yet. *)
    consts
  | _ -> compatible_consts

(* If [ty] is a MemberOf, unwrap the inner type, otherwise return the
   argument unchanged.

   MemberOf<SomeEnum, X> -> X
   Y -> Y *)
let unwrap_enum_memberof (ty : Typing_defs.decl_ty) : Typing_defs.decl_ty =
  let ty = strip_like_decl ty in
  match get_node ty with
  | Tapply ((_, name), [_; arg])
    when String.equal name Naming_special_names.Classes.cMemberOf ->
    arg
  | _ -> ty

let autocomplete_enum_class_label env opt_cname pos_labelname expected_ty =
  let suggest_members cls =
    List.iter
      (compatible_enum_class_consts env cls expected_ty)
      ~f:(fun (name, cc) ->
        let res_detail =
          Tast_env.print_decl_ty env (unwrap_enum_memberof cc.cc_type)
        in
        let ty = Phase.decl cc.cc_type in
        let kind = FileInfo.SI_ClassConstant in
        let complete =
          {
            res_decl_pos = get_pos_for env ty;
            res_replace_pos = replace_pos_of_id pos_labelname;
            res_base_class = Some (Cls.name cls);
            res_detail;
            res_label = name;
            res_insert_text = InsertLiterally name;
            res_fullname = name;
            res_kind = kind;
            res_documentation = None;
            res_filter_text = None;
            res_additional_edits = [];
            res_sortText = None;
          }
        in
        add_res complete)
  in
  let open Option in
  Option.iter
    ~f:suggest_members
    (opt_cname >>= fun (_, id) -> Tast_env.get_class env id)

(* Zip two lists together. If the two lists have different lengths,
   take the shortest. *)
let rec zip_truncate (xs : 'a list) (ys : 'b list) : ('a * 'b) list =
  match (xs, ys) with
  | (x :: xs, y :: ys) -> (x, y) :: zip_truncate xs ys
  | _ -> []

(* Auto complete for short labels `#Foo` inside a function call.
 * Leverage function type information to infer the right enum class.
 *)
let autocomplete_enum_class_label_call env f args =
  let suggest_members_from_ty env ty pos_labelname expected_ty =
    let ty = expand_and_strip_dynamic env ty in
    match get_node ty with
    | Tclass ((p, enum_name), _, _) when Tast_env.is_enum_class env enum_name ->
      autocomplete_enum_class_label
        env
        (Some (p, enum_name))
        pos_labelname
        expected_ty
    | _ -> ()
  in
  let is_enum_class_label_ty_name name =
    String.equal Naming_special_names.Classes.cEnumClassLabel name
  in
  let (fty, _, _) = f in
  (* Functions that support dynamic will be wrapped by supportdyn<_> *)
  let fty = expand_and_strip_supportdyn env fty in
  match get_node fty with
  | Tfun { ft_params; _ } ->
    let ty_args = zip_truncate args ft_params in
    List.iter
      ~f:(fun (arg, arg_ty) ->
        match
          (arg, get_node (expand_and_strip_dynamic env arg_ty.fp_type.et_type))
        with
        | ( (_, (_, p, Aast.EnumClassLabel (None, n))),
            Typing_defs.Tnewtype (ty_name, [enum_ty; member_ty], _) )
          when is_enum_class_label_ty_name ty_name && is_auto_complete n ->
          suggest_members_from_ty env enum_ty (p, n) (Some member_ty)
        | (_, _) -> ())
      ty_args
  | _ -> ()

(* Best-effort to find a class associated with this type constant. *)
let typeconst_class_name (typeconst : typeconst_type) : string option =
  let decl_ty =
    match typeconst.ttc_kind with
    | TCAbstract { atc_as_constraint; _ } -> atc_as_constraint
    | TCConcrete { tc_type } -> Some tc_type
  in
  match decl_ty with
  | Some decl_ty ->
    let decl_ty = strip_like_decl decl_ty in
    let (_, ty_) = Typing_defs_core.deref decl_ty in
    (match ty_ with
    | Tapply ((_, name), _) -> Some name
    | _ -> None)
  | None -> None

(* Given a typeconstant `Foo::TBar::TBaz`, if TBaz is a classish,
   return its decl. *)
let rec typeconst_decl env (ids : sid list) (cls_name : string) : Cls.t option =
  let open Option in
  match Tast_env.get_class env cls_name with
  | Some cls_decl ->
    (match ids with
    | [] -> Some cls_decl
    | (_, id) :: ids ->
      Cls.get_typeconst cls_decl id
      >>= typeconst_class_name
      >>= typeconst_decl env ids)
  | None -> None

(* Autocomplete type constants, which look like `MyClass::AUTO332`. *)
let autocomplete_class_type_const env ((_, h) : Aast.hint) (ids : sid list) :
    unit =
  match List.last ids with
  | Some ((_, id) as sid) when is_auto_complete id ->
    let prefix = strip_suffix id in
    let class_name =
      match h with
      | Happly ((_, name), _) -> Some name
      | Hthis -> Tast_env.get_self_id env
      | _ -> None
    in
    let class_decl =
      match class_name with
      | Some class_name ->
        typeconst_decl env (List.drop_last_exn ids) class_name
      | None -> None
    in
    (match class_decl with
    | Some class_decl ->
      let consts = Cls.consts class_decl in
      List.iter consts ~f:(fun (name, cc) ->
          if String.is_prefix ~prefix name then
            let complete =
              {
                res_decl_pos = Pos.to_absolute Pos.none;
                res_replace_pos = replace_pos_of_id sid;
                res_base_class = Some cc.cc_origin;
                res_detail = "const type";
                res_label = name;
                res_insert_text = InsertLiterally name;
                res_fullname = name;
                res_kind = FileInfo.SI_ClassConstant;
                res_documentation = None;
                res_filter_text = None;
                res_additional_edits = [];
                res_sortText = None;
              }
            in
            add_res complete)
    | None -> ())
  | _ -> ()

(* Get the names of string literal keys in this shape type. *)
let shape_string_keys (sm : 'a TShapeMap.t) : string list =
  let fields = TShapeMap.keys sm in
  List.filter_map fields ~f:(fun field ->
      match field with
      | TSFlit_str (_, s) -> Some s
      | _ -> None)

let unwrap_holes ((_, _, e_) as e : Tast.expr) : Tast.expr =
  match e_ with
  | Aast.Hole (e, _, _, _)
  | Aast.Invalid (Some e) ->
    e
  | _ -> e

(* If we see a call to a function that takes a shape argument, offer
   completions for field names in shape literals.

   takes_shape(shape('x' => 123, '|'));
*)
let autocomplete_shape_literal_in_call
    env
    (ft : Typing_defs.locl_fun_type)
    (args : (Ast_defs.param_kind * Tast.expr) list) : unit =
  let add_shape_key_result pos key =
    let ty = Tprim Aast_defs.Tstring in
    let reason = Typing_reason.Rwitness pos in
    let ty = mk (reason, ty) in

    let kind = FileInfo.SI_Literal in
    let lty = Phase.locl ty in
    let complete =
      {
        res_decl_pos = get_pos_for env lty;
        res_replace_pos = replace_pos_of_id (pos, key);
        res_base_class = None;
        res_detail = kind_to_string kind;
        res_label = key;
        res_insert_text = InsertLiterally key;
        res_fullname = key;
        res_kind = kind;
        res_documentation = None;
        res_filter_text = None;
        res_additional_edits = [];
        res_sortText = None;
      }
    in
    add_res complete
  in

  (* If this shape field name is of the form "fooAUTO332", return "foo". *)
  let shape_field_autocomplete_prefix (sfn : Ast_defs.shape_field_name) :
      string option =
    match sfn with
    | Ast_defs.SFlit_str (_, name) when is_auto_complete name ->
      Some (strip_suffix name)
    | _ -> None
  in

  let args = List.map args ~f:(fun (_, e) -> unwrap_holes e) in

  List.iter
    ~f:(fun (arg, expected_ty) ->
      match arg with
      | (_, pos, Aast.Shape kvs) ->
        (* We're passing a shape literal as this function argument. *)
        List.iter kvs ~f:(fun (name, _val) ->
            match shape_field_autocomplete_prefix name with
            | Some prefix ->
              (* This shape key is being autocompleted. *)
              let (_, ty_) =
                Typing_defs_core.deref expected_ty.fp_type.et_type
              in
              (match ty_ with
              | Tshape { s_fields = fields; _ } ->
                (* This parameter is known to be a concrete shape type. *)
                let keys = shape_string_keys fields in
                let matching_keys =
                  List.filter keys ~f:(String.is_prefix ~prefix)
                in
                List.iter matching_keys ~f:(add_shape_key_result pos)
              | _ -> ())
            | _ -> ())
      | _ -> ())
    (zip_truncate args ft.ft_params)

let add_builtin_attribute_result replace_pos ~doc ~name : unit =
  let complete =
    {
      res_decl_pos = Pos.to_absolute Pos.none;
      res_replace_pos = replace_pos;
      res_base_class = None;
      res_detail = "built-in attribute";
      res_label = name;
      res_insert_text = InsertLiterally name;
      res_fullname = name;
      res_kind = FileInfo.SI_Class;
      res_documentation = Some doc;
      res_filter_text = None;
      res_additional_edits = [];
      res_sortText = None;
    }
  in
  add_res complete

let enclosing_class_decl (env : Tast_env.env) : Cls.t option =
  match Tast_env.get_self_id env with
  | Some id -> Tast_env.get_class env id
  | None -> None

(** Return the inherited class methods whose name starts with [prefix]
    and haven't already been overridden. *)
let methods_could_override (cls : Cls.t) ~(is_static : bool) (prefix : string) :
    (string * class_elt) list =
  let c_name = Cls.name cls in
  let methods =
    if is_static then
      Cls.smethods cls
    else
      Cls.methods cls
  in
  List.filter methods ~f:(fun (n, ce) ->
      (* We're only interested in methods that start with this prefix,
         and aren't already defined in the current class. *)
      String.is_prefix n ~prefix && not (String.equal ce.ce_origin c_name))

(** Autocomplete a partially written override method definition.

   <<__Override>>
   public static function xAUTO332
*)
let autocomplete_overriding_method env m : unit =
  let open Aast in
  let name = snd m.m_name in
  if is_auto_complete name then
    let prefix = strip_suffix name in
    let has_override =
      List.exists m.m_user_attributes ~f:(fun { ua_name = (_, attr_name); _ } ->
          String.equal attr_name Naming_special_names.UserAttributes.uaOverride)
    in
    match enclosing_class_decl env with
    | Some cls when has_override ->
      List.iter
        (methods_could_override cls ~is_static:m.m_static prefix)
        ~f:(fun (name, ce) ->
          let complete =
            {
              res_decl_pos = Pos.to_absolute Pos.none;
              res_replace_pos = replace_pos_of_id m.m_name;
              res_base_class = Some ce.ce_origin;
              res_detail = "method name";
              res_label = name;
              res_insert_text = InsertLiterally name;
              res_fullname = name;
              res_kind = FileInfo.SI_ClassMethod;
              res_documentation = None;
              res_filter_text = None;
              res_additional_edits = [];
              res_sortText = None;
            }
          in
          add_res complete)
    | _ -> ()

let autocomplete_builtin_attribute
    ((pos, name) : Pos.t * string) (attr_kind : string) : unit =
  let module UA = Naming_special_names.UserAttributes in
  let stripped_name = Utils.strip_ns name in
  if is_auto_complete name && String.is_prefix stripped_name ~prefix:"_" then
    let replace_pos = replace_pos_of_id (pos, name) in
    let prefix = strip_suffix stripped_name in
    (* Built-in attributes that match the prefix the user has typed. *)
    let possible_attrs =
      SMap.filter
        (fun name attr_info ->
          String.is_prefix name ~prefix
          && attr_info.UA.autocomplete
          && List.mem attr_info.UA.contexts attr_kind ~equal:String.equal)
        UA.as_map
    in
    (* Sort by attribute name. This isn't necessary in the IDE, which
       does its own sorting, but helps tests. *)
    let sorted_attrs =
      List.sort (SMap.elements possible_attrs) ~compare:(fun (x, _) (y, _) ->
          String.compare x y)
      |> List.rev
    in
    List.iter
      ~f:(fun (name, attr_info) ->
        let doc = attr_info.UA.doc in
        add_builtin_attribute_result replace_pos ~name ~doc)
      sorted_attrs

(* If [name] is an enum, return the list of the constants it defines. *)
let enum_consts env name : string list option =
  match Decl_provider.get_class (Tast_env.get_ctx env) name with
  | Some cls ->
    (match Cls.kind cls with
    | Ast_defs.Cenum ->
      let consts =
        Cls.consts cls
        |> List.map ~f:fst
        |> List.filter ~f:(fun name ->
               String.(name <> Naming_special_names.Members.mClass))
      in
      Some consts
    | _ -> None)
  | None -> None

let add_enum_const_result env pos replace_pos prefix const_name =
  let ty = Tprim Aast_defs.Tstring in
  let reason = Typing_reason.Rwitness pos in
  let ty = mk (reason, ty) in

  let kind = FileInfo.SI_ClassConstant in
  let lty = Phase.locl ty in
  let key = prefix ^ const_name in
  let complete =
    {
      res_decl_pos = get_pos_for env lty;
      res_replace_pos = replace_pos;
      res_base_class = None;
      res_detail = kind_to_string kind;
      res_label = key;
      res_insert_text = InsertLiterally key;
      res_fullname = key;
      res_kind = kind;
      res_documentation = None;
      res_filter_text = None;
      res_additional_edits = [];
      res_sortText = None;
    }
  in
  add_res complete

let case_names (expected_enum : string) (cases : Tast.case list) : SSet.t =
  let case_name case =
    match fst case with
    | ( _,
        _,
        Aast.Class_const ((_, _, Aast.CI (_, enum_name)), (_, variant_name)) )
      when String.equal expected_enum enum_name ->
      Some variant_name
    | _ -> None
  in
  SSet.of_list (List.filter_map cases ~f:case_name)

(* Autocomplete enum values in case statements.

   switch ($enum_value) {
     case AUTO332
   } *)
let autocomplete_enum_case env (expr : Tast.expr) (cases : Tast.case list) =
  List.iter cases ~f:(fun (e, _) ->
      match e with
      | (_, _, Aast.Id (pos, id)) when is_auto_complete id ->
        let (recv_ty, _, _) = expr in
        let ty = expand_and_strip_dynamic env recv_ty in
        let (_, ty_) = Typing_defs_core.deref ty in
        let replace_pos = replace_pos_of_id (pos, id) in
        (match ty_ with
        | Tnewtype (name, _, _) ->
          (match enum_consts env name with
          | Some consts ->
            let used_consts = case_names name cases in
            let unused_consts =
              List.filter consts ~f:(fun const ->
                  not (SSet.mem const used_consts))
            in

            let prefix = Utils.strip_ns name ^ "::" in
            List.iter
              unused_consts
              ~f:(add_enum_const_result env pos replace_pos prefix)
          | None -> ())
        | _ -> ())
      | _ -> ())

(* Autocomplete enum values in function arguments that are known to be enums.

   takes_enum(AUTO332); *)
let autocomplete_enum_value_in_call env (ft : Typing_defs.locl_fun_type) args :
    unit =
  let args = List.map args ~f:(fun (_, e) -> unwrap_holes e) in

  List.iter
    ~f:(fun (arg, expected_ty) ->
      match arg with
      | (_, _, Aast.Id (pos, id)) when matches_auto_complete_suffix id ->
        let ty = expand_and_strip_dynamic env expected_ty.fp_type.et_type in
        let (_, ty_) = Typing_defs_core.deref ty in
        let replace_pos = replace_pos_of_id (pos, id) in

        (match ty_ with
        | Tnewtype (name, _, _) ->
          (match enum_consts env name with
          | Some consts ->
            let prefix = Utils.strip_ns name ^ "::" in
            List.iter
              consts
              ~f:(add_enum_const_result env pos replace_pos prefix)
          | None -> ())
        | _ -> ())
      | (_, _, Aast.Class_const ((_, _, Aast.CI _name), (pos, id)))
        when matches_auto_complete_suffix id ->
        let ty = expand_and_strip_dynamic env expected_ty.fp_type.et_type in
        let (_, ty_) = Typing_defs_core.deref ty in
        let replace_pos = replace_pos_of_id (pos, id) in

        (match ty_ with
        | Tnewtype (name, _, _) ->
          (match enum_consts env name with
          | Some consts ->
            List.iter consts ~f:(add_enum_const_result env pos replace_pos "")
          | None -> ())
        | _ -> ())
      | _ -> ())
    (zip_truncate args ft.ft_params)

let builtin_type_hints =
  [
    (SymbolOccurrence.BImixed, "mixed");
    (SymbolOccurrence.BIdynamic, "dynamic");
    (SymbolOccurrence.BInothing, "nothing");
    (SymbolOccurrence.BInonnull, "nonnull");
    (SymbolOccurrence.BIshape, "shape");
    (SymbolOccurrence.BIprimitive Aast_defs.Tnull, "null");
    (* TODO: only offer void in return positions. *)
    (SymbolOccurrence.BIprimitive Aast_defs.Tvoid, "void");
    (SymbolOccurrence.BIprimitive Aast_defs.Tint, "int");
    (SymbolOccurrence.BIprimitive Aast_defs.Tbool, "bool");
    (SymbolOccurrence.BIprimitive Aast_defs.Tfloat, "float");
    (SymbolOccurrence.BIprimitive Aast_defs.Tstring, "string");
    (SymbolOccurrence.BIprimitive Aast_defs.Tresource, "resource");
    (SymbolOccurrence.BIprimitive Aast_defs.Tnum, "num");
    (SymbolOccurrence.BIprimitive Aast_defs.Tarraykey, "arraykey");
    (SymbolOccurrence.BIprimitive Aast_defs.Tnoreturn, "noreturn");
  ]

(* Find global autocomplete results *)
let find_global_results
    ~(env : Tast_env.env)
    ~(id : Pos.t * string)
    ~(completion_type : SearchTypes.autocomplete_type)
    ~(autocomplete_context : AutocompleteTypes.legacy_autocomplete_context)
    ~(sienv_ref : SearchUtils.si_env ref)
    ~(pctx : Provider_context.t) : unit =
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
    Pos.length (fst id) > AutocompleteTypes.autocomplete_token_length
  in

  let tast_env = Tast_env.empty pctx in
  let ctx = autocomplete_context in
  if
    (not ctx.is_manually_invoked)
    && (not have_user_prefix)
    && (ctx.is_after_single_colon
       || ctx.is_after_open_square_bracket
       || ctx.is_after_quote)
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
  else
    let query_text = strip_suffix (snd id) in
    let query_text = Utils.strip_ns query_text in
    auto_complete_for_global := query_text;
    let (ns, _) = Utils.split_ns_from_name query_text in
    let absolute_none = Pos.none |> Pos.to_absolute in
    let kind_filter =
      match completion_type with
      | Acnew -> Some FileInfo.SI_Class
      | Actrait_only -> Some FileInfo.SI_Trait
      | Acclassish
      | Acid
      | Actype
      | Ac_workspace_symbol ->
        None
    in

    (* Note that we are called by a visitor on the TAST, and the TAST has already done namespace-aliasing elaboration.
       For instance given source code "Vec\chAUTO332" and .hhconfig auto_namespace_map={"Vec":"FlibSL\\Vec"},
       then the TAST will have a node with id "FLibSL\Vec\chAUTO332".
       This then will be the [query_text] that we send to [find_matching_symbols]. *)
    let (results, is_complete) =
      SymbolIndex.find_matching_symbols
        ~sienv_ref
        ~query_text
        ~max_results
        ~kind_filter
        ~context:completion_type
    in
    autocomplete_is_complete :=
      SearchTypes.equal_si_complete is_complete SearchTypes.Complete;
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
        let (res_label, res_fullname) = (r.si_name, r.si_fullname) in
        (* Only load func details if the flag sie_resolve_signatures is true *)
        let (res_detail, res_insert_text) =
          if
            !sienv_ref.sie_resolve_signatures
            && FileInfo.equal_si_kind r.si_kind FileInfo.SI_Function
          then
            let fixed_name = ns ^ r.si_name in
            match Tast_env.get_fun tast_env fixed_name with
            | None -> (kind_to_string r.si_kind, InsertLiterally res_label)
            | Some fe ->
              let ty = fe.fe_type in
              let res_detail = Tast_env.print_decl_ty tast_env ty in
              ( res_detail,
                insert_text_for_ty
                  env
                  autocomplete_context
                  res_label
                  (Phase.decl ty) )
          else
            (kind_to_string r.si_kind, InsertLiterally res_label)
        in
        (* Only load exact positions if specially requested *)
        let res_decl_pos =
          if !sienv_ref.sie_resolve_positions then
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
            res_decl_pos;
            res_replace_pos = replace_pos_of_id id;
            res_base_class = None;
            res_detail;
            res_label;
            res_insert_text;
            res_fullname;
            res_kind = r.si_kind;
            res_documentation = None;
            res_filter_text = None;
            res_additional_edits = [];
            res_sortText = None;
          }
        in
        add_res complete);

    (* Add any builtins that match *)
    match completion_type with
    | Acid
    | Actrait_only
    | Acclassish
    | Ac_workspace_symbol
    | Acnew ->
      ()
    | Actype ->
      builtin_type_hints
      |> List.filter ~f:(fun (_, name) ->
             String.is_prefix name ~prefix:query_text)
      |> List.iter ~f:(fun (hint, name) ->
             let kind = FileInfo.SI_Typedef in
             let documentation = SymbolOccurrence.built_in_type_hover hint in
             add_res
               {
                 res_decl_pos = absolute_none;
                 res_replace_pos = replace_pos_of_id id;
                 res_base_class = None;
                 res_detail = "builtin";
                 res_label = name;
                 res_insert_text = InsertLiterally name;
                 res_fullname = name;
                 res_kind = kind;
                 res_documentation = Some documentation;
                 res_filter_text = None;
                 res_additional_edits = [];
                 res_sortText = None;
               })

let complete_xhp_tag
    ~(id : Pos.t * string)
    ~(does_autocomplete_snippet : bool)
    ~(sienv_ref : SearchUtils.si_env ref)
    ~(pctx : Provider_context.t) : unit =
  let tast_env = Tast_env.empty pctx in
  let query_text = strip_suffix (snd id) in
  let query_text = Utils.strip_ns query_text in
  auto_complete_for_global := query_text;
  let absolute_none = Pos.none |> Pos.to_absolute in
  let (results, _is_complete) =
    SymbolIndex.find_matching_symbols
      ~sienv_ref
      ~query_text
      ~max_results
      ~kind_filter:None
      ~context:Acid
  in
  List.iter results ~f:(fun r ->
      let (res_label, res_fullname) =
        (Utils.strip_xhp_ns r.si_name, Utils.add_xhp_ns r.si_fullname)
      in
      let (res_detail, res_insert_text) =
        (kind_to_string r.si_kind, InsertLiterally res_label)
      in
      let res_insert_text =
        match r.si_kind with
        | FileInfo.SI_XHP
        | FileInfo.SI_Class
          when does_autocomplete_snippet ->
          let classname = Utils.add_ns res_fullname in
          let attrs =
            get_class_req_attrs tast_env pctx classname (Some (Aast.CI id))
          in
          let has_children = not (get_class_is_child_empty pctx classname) in
          insert_text_for_xhp_req_attrs res_label attrs has_children
        | _ -> res_insert_text
      in
      (* Figure out how to display them *)
      let complete =
        {
          res_decl_pos = absolute_none;
          res_replace_pos = replace_pos_of_id ~strip_xhp_colon:true id;
          res_base_class = None;
          res_detail;
          res_label;
          res_insert_text;
          res_fullname;
          res_kind = r.si_kind;
          res_documentation = None;
          res_filter_text = None;
          res_additional_edits = [];
          res_sortText = None;
        }
      in
      add_res complete);
  autocomplete_is_complete := List.length !autocomplete_items < max_results

let auto_complete_suffix_finder =
  object
    inherit [_] Aast.reduce

    method zero = false

    method plus = ( || )

    method! on_Lvar () (_, id) =
      matches_auto_complete_suffix (Local_id.get_name id)
  end

let method_contains_cursor = auto_complete_suffix_finder#on_method_ ()

let fun_contains_cursor = auto_complete_suffix_finder#on_fun_ ()

(* Find all the local variables before the cursor, so we can offer them as completion candidates. *)
class local_vars =
  object (self)
    inherit Tast_visitor.iter as super

    val mutable results = Local_id.Map.empty

    val mutable id_at_cursor = None

    method get_locals ctx tast =
      self#go ctx tast;
      (results, id_at_cursor)

    method add id ty =
      (* If we already have a position for this identifier, don't overwrite it with
         results from after the cursor position. *)
      if not (Local_id.Map.mem id results && Option.is_some id_at_cursor) then
        results <- Local_id.Map.add id ty results

    method! on_fun_ env f = if fun_contains_cursor f then super#on_fun_ env f

    method! on_method_ env m =
      if method_contains_cursor m then (
        if not m.Aast.m_static then
          self#add Typing_defs.this (Tast_env.get_self_ty_exn env);
        super#on_method_ env m
      )

    method! on_expr env e =
      let (ty, _, e_) = e in
      match e_ with
      | Aast.Lvar (pos, id) ->
        let name = Local_id.get_name id in
        if matches_auto_complete_suffix name then
          id_at_cursor <- Some (pos, name)
        else
          self#add id ty
      | Aast.(Binop { bop = Ast_defs.Eq _; lhs = e1; rhs = e2 }) ->
        (* Process the rvalue before the lvalue, since the lvalue is annotated
           with its type after the assignment. *)
        self#on_expr env e2;
        self#on_expr env e1
      | _ -> super#on_expr env e

    method! on_fun_param _env fp =
      let id = Local_id.make_unscoped fp.Aast.param_name in
      let ty = fp.Aast.param_annotation in
      self#add id ty
  end

let compute_complete_local env ctx tast =
  let (locals, id_at_cursor) = (new local_vars)#get_locals ctx tast in
  let id_at_cursor = Option.value id_at_cursor ~default:(Pos.none, "") in
  let replace_pos = replace_pos_of_id id_at_cursor in

  let id_prefix =
    if is_auto_complete (snd id_at_cursor) then
      strip_suffix (snd id_at_cursor)
    else
      ""
  in

  Local_id.Map.iter
    (fun id ty ->
      let kind = FileInfo.SI_LocalVariable in
      let name = Local_id.get_name id in
      if String.is_prefix name ~prefix:id_prefix then
        let complete =
          {
            res_decl_pos = get_pos_for env (LoclTy ty);
            res_replace_pos = replace_pos;
            res_base_class = None;
            res_detail = Tast_env.print_ty env ty;
            res_label = name;
            res_insert_text = InsertLiterally name;
            res_fullname = name;
            res_kind = kind;
            res_documentation = None;
            res_filter_text = None;
            res_additional_edits = [];
            res_sortText = None;
          }
        in
        add_res complete)
    locals

(* Walk the TAST and append autocomplete results for
   any syntax that contains AUTO332 in its name. *)
let visitor
    (ctx : Provider_context.t)
    (autocomplete_context : legacy_autocomplete_context)
    (sienv_ref : si_env ref)
    (naming_table : Naming_table.t)
    (toplevel_tast : Tast.def list) =
  object (self)
    inherit Tast_visitor.iter as super

    method complete_global
        (env : Tast_env.env) (id : sid) (completion_type : autocomplete_type)
        : unit =
      if is_auto_complete (snd id) then
        find_global_results
          ~env
          ~id
          ~completion_type
          ~autocomplete_context
          ~sienv_ref
          ~pctx:ctx

    method complete_id env (id : Ast_defs.id) : unit =
      self#complete_global env id Acid

    method! on_Id env id =
      autocomplete_id id;
      self#complete_id env id;
      super#on_Id env id

    method! on_Call env call =
      let Aast.{ func; args; _ } = call in
      autocomplete_enum_class_label_call env func args;
      super#on_Call env call

    method! on_New env ((_, _, cid_) as cid) el unpacked_element =
      (match cid_ with
      | Aast.CI id -> self#complete_global env id Acnew
      | _ -> ());
      super#on_New env cid el unpacked_element

    method! on_Nameof env ((_, _, cid_) as cid) =
      (match cid_ with
      | Aast.CI id -> self#complete_global env id Acclassish
      | _ -> ());
      super#on_Nameof env cid

    method! on_Happly env sid hl =
      self#complete_global env sid Actype;
      super#on_Happly env sid hl

    method! on_Haccess env h ids =
      autocomplete_class_type_const env h ids;
      super#on_Haccess env h ids

    method! on_Lvar env ((_, name) as lid) =
      if is_auto_complete (Local_id.get_name name) then
        compute_complete_local env ctx toplevel_tast;
      super#on_Lvar env lid

    method! on_Class_get env cid mid prop_or_method =
      match mid with
      | Aast.CGstring p ->
        autocomplete_static_member autocomplete_context env cid p
      | Aast.CGexpr _ -> super#on_Class_get env cid mid prop_or_method

    method! on_Class_const env cid mid =
      autocomplete_static_member autocomplete_context env cid mid;
      super#on_Class_const env cid mid

    method! on_Obj_get env obj mid ognf =
      (match mid with
      | (_, _, Aast.Id mid) ->
        autocomplete_hack_fake_arrow env obj mid naming_table;
        autocomplete_typed_member
          ~is_static:false
          autocomplete_context
          env
          (get_type obj)
          None
          mid
      | _ -> ());
      super#on_Obj_get env obj mid ognf

    method! on_expr env expr =
      (match expr with
      | (_, _, Aast.Array_get (arr, Some (_, pos, key))) ->
        let ty = get_type arr in
        let ty = expand_and_strip_dynamic env ty in
        begin
          match get_node ty with
          | Tshape { s_fields = fields; _ } ->
            (match key with
            | Aast.Id (_, mid) ->
              autocomplete_shape_key autocomplete_context env fields (pos, mid)
            | Aast.String mid ->
              (* autocomplete generally assumes that there's a token ending with the suffix; *)
              (* This isn't the case for `$shape['a`, unless it's at the end of the file *)
              let offset =
                String_utils.substring_index
                  AutocompleteTypes.autocomplete_token
                  mid
              in
              if Int.equal offset (-1) then
                autocomplete_shape_key autocomplete_context env fields (pos, mid)
              else
                let mid =
                  Str.string_before
                    mid
                    (offset + AutocompleteTypes.autocomplete_token_length)
                in
                let (line, bol, start_offset) = Pos.line_beg_offset pos in
                let pos =
                  Pos.make_from_lnum_bol_offset
                    ~pos_file:(Pos.filename pos)
                    ~pos_start:(line, bol, start_offset)
                    ~pos_end:
                      ( line,
                        bol,
                        start_offset
                        + offset
                        + AutocompleteTypes.autocomplete_token_length )
                in
                autocomplete_shape_key autocomplete_context env fields (pos, mid)
            | _ -> ())
          | _ -> ()
        end
      | (_, _, Aast.(Call { func = (recv_ty, _, _); args; _ })) ->
        (* Functions that support dynamic will be wrapped by supportdyn<_> *)
        let recv_ty = expand_and_strip_supportdyn env recv_ty in
        (match deref recv_ty with
        | (_r, Tfun ft) ->
          autocomplete_shape_literal_in_call env ft args;
          autocomplete_enum_value_in_call env ft args
        | _ -> ())
      | (_, p, Aast.EnumClassLabel (opt_cname, n)) when is_auto_complete n ->
        autocomplete_enum_class_label env opt_cname (p, n) None
      | (_, _, Aast.Efun { Aast.ef_fun = f; _ })
      | (_, _, Aast.Lfun (f, _)) ->
        List.iter f.Aast.f_user_attributes ~f:(fun ua ->
            autocomplete_builtin_attribute
              ua.Aast.ua_name
              Naming_special_names.AttributeKinds.lambda)
      | _ -> ());
      super#on_expr env expr

    method! on_Xml env sid attrs el =
      autocomplete_id sid;
      if is_auto_complete (snd sid) then
        complete_xhp_tag
          ~id:sid
          ~does_autocomplete_snippet:(List.is_empty attrs)
          ~sienv_ref
          ~pctx:ctx;

      let cid = Aast.CI sid in
      Decl_provider.get_class (Tast_env.get_ctx env) (snd sid)
      |> Option.iter ~f:(fun (c : Cls.t) ->
             List.iter attrs ~f:(function
                 | Aast.Xhp_simple
                     { Aast.xs_name = id; xs_expr = value; xs_type = ty } ->
                   (match value with
                   | (_, _, Aast.Id id_id) ->
                     (* This handles the situation
                          <foo:bar my-attribute={AUTO332}
                     *)
                     autocomplete_xhp_enum_attribute_value
                       (snd id)
                       ty
                       id_id
                       env
                       c;
                     autocomplete_xhp_enum_class_value ty id_id env;
                     autocomplete_xhp_bool_value ty id_id env
                   | _ -> ());
                   if Cls.is_xhp c then
                     autocomplete_xhp_attributes env c (Some cid) id attrs
                   else
                     autocomplete_member
                       ~is_static:false
                       autocomplete_context
                       env
                       c
                       (Some cid)
                       id
                 | Aast.Xhp_spread _ -> ()));
      super#on_Xml env sid attrs el

    method! on_typedef env t =
      List.iter t.Aast.t_user_attributes ~f:(fun ua ->
          autocomplete_builtin_attribute
            ua.Aast.ua_name
            Naming_special_names.AttributeKinds.typealias);
      super#on_typedef env t

    method! on_fun_def env fd =
      List.iter fd.Aast.fd_fun.Aast.f_user_attributes ~f:(fun ua ->
          autocomplete_builtin_attribute
            ua.Aast.ua_name
            Naming_special_names.AttributeKinds.fn);
      super#on_fun_def env fd

    method! on_fun_param env fp =
      List.iter fp.Aast.param_user_attributes ~f:(fun ua ->
          autocomplete_builtin_attribute
            ua.Aast.ua_name
            Naming_special_names.AttributeKinds.parameter);
      super#on_fun_param env fp

    method! on_tparam env tp =
      List.iter tp.Aast.tp_user_attributes ~f:(fun ua ->
          autocomplete_builtin_attribute
            ua.Aast.ua_name
            Naming_special_names.AttributeKinds.typeparam);
      super#on_tparam env tp

    method! on_method_ env m =
      autocomplete_overriding_method env m;
      List.iter m.Aast.m_user_attributes ~f:(fun ua ->
          autocomplete_builtin_attribute
            ua.Aast.ua_name
            Naming_special_names.AttributeKinds.mthd);
      super#on_method_ env m

    method! on_class_var env cv =
      List.iter cv.Aast.cv_user_attributes ~f:(fun ua ->
          autocomplete_builtin_attribute
            ua.Aast.ua_name
            (if cv.Aast.cv_is_static then
              Naming_special_names.AttributeKinds.staticProperty
            else
              Naming_special_names.AttributeKinds.instProperty));
      super#on_class_var env cv

    method! on_class_typeconst_def env ctd =
      List.iter ctd.Aast.c_tconst_user_attributes ~f:(fun ua ->
          autocomplete_builtin_attribute
            ua.Aast.ua_name
            Naming_special_names.AttributeKinds.typeconst);
      super#on_class_typeconst_def env ctd

    method! on_class_ env cls =
      List.iter cls.Aast.c_user_attributes ~f:(fun ua ->
          autocomplete_builtin_attribute
            ua.Aast.ua_name
            Naming_special_names.AttributeKinds.cls);

      List.iter cls.Aast.c_uses ~f:(fun hint ->
          match snd hint with
          | Aast.Happly (id, params) ->
            self#complete_global env id Actrait_only;
            List.iter params ~f:(self#on_hint env)
          | _ -> ());

      (* Ignore properties on interfaces. They're not legal syntax, but
         they can occur when we parse partial method definitions. *)
      let cls =
        match cls.Aast.c_kind with
        | Ast_defs.Cinterface -> { cls with Aast.c_vars = [] }
        | _ -> cls
      in

      (* If we don't clear out c_uses we'll end up overwriting the trait
         completion as soon as we get to on_Happly. *)
      super#on_class_ env { cls with Aast.c_uses = [] }

    method! on_Switch env expr cases default_case =
      autocomplete_enum_case env expr cases;
      super#on_Switch env expr cases default_case
  end

let reset () =
  auto_complete_for_global := "";
  autocomplete_items := [];
  autocomplete_is_complete := true

let complete_keywords_at possible_keywords text pos : unit =
  if is_auto_complete text then
    let prefix = strip_suffix text in
    possible_keywords
    |> List.filter ~f:(fun possible_keyword ->
           String.is_prefix possible_keyword ~prefix)
    |> List.iter ~f:(fun keyword ->
           let kind = FileInfo.SI_Keyword in
           let complete =
             {
               res_decl_pos = Pos.none |> Pos.to_absolute;
               res_replace_pos = replace_pos_of_id (pos, text);
               res_base_class = None;
               res_detail = kind_to_string kind;
               res_label = keyword;
               res_insert_text = InsertLiterally keyword;
               res_fullname = keyword;
               res_kind = kind;
               res_documentation = None;
               res_filter_text = None;
               res_additional_edits = [];
               res_sortText = None;
             }
           in
           add_res complete)

let complete_keywords_at_token possible_keywords filename (s : Syntax.t) : unit
    =
  let token_str = Syntax.text s in
  match s.Syntax.syntax with
  | Syntax.Token t ->
    let start_offset = t.Syntax.Token.offset + t.Syntax.Token.leading_width in
    let end_offset = start_offset + t.Syntax.Token.width in
    let source_text = t.Syntax.Token.source_text in

    let pos =
      Full_fidelity_source_text.relative_pos
        filename
        source_text
        start_offset
        end_offset
    in
    complete_keywords_at possible_keywords token_str pos
  | _ -> ()

let complete_keywords_at_trivia possible_keywords filename (t : Trivia.t) : unit
    =
  let trivia_str = Trivia.text t in
  match Trivia.kind t with
  | Trivia.TriviaKind.ExtraTokenError when is_auto_complete trivia_str ->
    let start_offset = t.Trivia.offset in
    let end_offset = start_offset + t.Trivia.width in
    let source_text = t.Trivia.source_text in

    let pos =
      Full_fidelity_source_text.relative_pos
        filename
        source_text
        start_offset
        end_offset
    in
    complete_keywords_at possible_keywords trivia_str pos
  | _ -> ()

let def_start_keywords filename s : unit =
  let possible_keywords =
    [
      "function";
      (* `async` at the top level can occur for functions. *)
      "async";
      "class";
      "trait";
      "interface";
      "type";
      "newtype";
      "enum";
    ]
  in
  complete_keywords_at_token possible_keywords filename s

let class_member_start_keywords filename s (ctx : Ast_defs.classish_kind option)
    : unit =
  let visibility_keywords = ["public"; "protected"] in
  let const_type_keywords = ["const"; "abstract"] in
  (* Keywords allowed on class and trait methods, but not allowed on interface methods. *)
  let concrete_method_keywords = ["private"; "final"] in
  let trait_member_keywords =
    ["require extends"; "require implements"; "require class"]
  in
  let inclusion_keywords = ["use"] in

  let possible_keywords =
    match ctx with
    | Some (Ast_defs.Cclass _) ->
      visibility_keywords
      @ const_type_keywords
      @ concrete_method_keywords
      @ inclusion_keywords
    | Some Ast_defs.Cinterface -> visibility_keywords @ const_type_keywords
    | Some Ast_defs.Ctrait ->
      visibility_keywords
      @ const_type_keywords
      @ concrete_method_keywords
      @ trait_member_keywords
      @ inclusion_keywords
    | _ -> []
  in

  complete_keywords_at_token possible_keywords filename s

(* Drop the items from [possible_keywords] if they're already in
   [existing_modifiers], and drop visibility keywords if we already
   have any visibility keyword. *)
let available_keywords existing_modifiers possible_keywords : string list =
  let current_modifiers =
    Syntax.syntax_node_to_list existing_modifiers
    |> List.filter_map ~f:(fun s ->
           match s.Syntax.syntax with
           | Syntax.Token _ -> Some (Syntax.text s)
           | _ -> None)
  in
  let visibility_modifiers = SSet.of_list ["public"; "protected"; "private"] in
  let has_visibility =
    List.exists current_modifiers ~f:(fun kw ->
        SSet.mem kw visibility_modifiers)
  in
  List.filter possible_keywords ~f:(fun kw ->
      (not (List.mem current_modifiers kw ~equal:String.equal))
      && not (SSet.mem kw visibility_modifiers && has_visibility))

let class_keywords filename existing_modifiers s : unit =
  let possible_keywords =
    available_keywords existing_modifiers ["final"; "abstract"; "class"]
  in
  complete_keywords_at_token possible_keywords filename s

let method_keywords filename trivia =
  complete_keywords_at_trivia
    ["public"; "protected"; "private"; "static"; "abstract"; "final"; "async"]
    filename
    trivia

let classish_after_name_keywords filename ctx ~has_extends trivia =
  let possible_keywords =
    match ctx with
    | Some (Ast_defs.Cclass _) ->
      if has_extends then
        ["implements"]
      else
        ["implements"; "extends"]
    | Some Ast_defs.Cinterface -> ["extends"]
    | Some Ast_defs.Ctrait -> ["implements"]
    | _ -> []
  in
  complete_keywords_at_trivia possible_keywords filename trivia

let interface_method_keywords filename existing_modifiers s : unit =
  let possible_keywords =
    available_keywords
      existing_modifiers
      ["public"; "protected"; "private"; "static"; "function"]
  in
  complete_keywords_at_token possible_keywords filename s

let property_or_method_keywords filename existing_modifiers s : unit =
  let possible_keywords =
    available_keywords
      existing_modifiers
      [
        "public";
        "protected";
        "private";
        "static";
        "abstract";
        "final";
        "async";
        "function";
      ]
  in
  complete_keywords_at_token possible_keywords filename s

let keywords filename tree : unit =
  let open Syntax in
  let rec aux (ctx : Ast_defs.classish_kind option) s =
    let inner_ctx = ref ctx in
    (match s.syntax with
    | Script sd ->
      List.iter (syntax_node_to_list sd.script_declarations) ~f:(fun d ->
          (* If we see AUTO332 at the top-level it's parsed as an
             expression, but the user is about to write a top-level
             definition (function, class etc). *)
          match d.syntax with
          | ExpressionStatement es ->
            def_start_keywords filename es.expression_statement_expression
          | _ -> ())
    | NamespaceBody nb ->
      List.iter (syntax_node_to_list nb.namespace_declarations) ~f:(fun d ->
          match d.syntax with
          (* Treat expressions in namespaces consistently with
             top-level expressions. *)
          | ExpressionStatement es ->
            def_start_keywords filename es.expression_statement_expression
          | _ -> ())
    | FunctionDeclarationHeader fdh ->
      (match fdh.function_keyword.syntax with
      | Missing ->
        (* The user has written `async AUTO332`, so we're expecting `function`. *)
        complete_keywords_at_token ["function"] filename fdh.function_name
      | _ -> ())
    | ClassishDeclaration cd ->
      (match cd.classish_keyword.syntax with
      | Missing ->
        (* The user has written `final AUTO332` or `abstract AUTO332`,
           so we're expecting a class, not an interface or trait. *)
        class_keywords filename cd.classish_modifiers cd.classish_name
      | Token _ ->
        (match Syntax.text cd.classish_keyword with
        | "interface" -> inner_ctx := Some Ast_defs.Cinterface
        | "class" ->
          (* We're only interested if the context is a class or not,
             so arbitrarily consider this a concrete class. *)
          inner_ctx := Some (Ast_defs.Cclass Ast_defs.Concrete)
        | "trait" -> inner_ctx := Some Ast_defs.Ctrait
        | "enum" -> inner_ctx := Some Ast_defs.Cenum
        | _ -> ())
      | _ -> ());
      (match cd.classish_body.syntax with
      | ClassishBody cb ->
        let has_extends =
          match cd.classish_extends_keyword.syntax with
          | Token _ -> true
          | _ -> false
        in
        (match cb.classish_body_left_brace.syntax with
        | Token t ->
          (* The user has written `class Foo AUTO332`, so we're
             expecting `extends` or `implements`.*)
          List.iter
            (Syntax.Token.leading t)
            ~f:(classish_after_name_keywords filename !inner_ctx ~has_extends)
        | _ -> ())
      | _ -> ())
    | ClassishBody cb ->
      List.iter (syntax_node_to_list cb.classish_body_elements) ~f:(fun d ->
          match d.syntax with
          | ErrorSyntax { error_error } ->
            (match
               ( Syntax.leading_width error_error,
                 Syntax.trailing_width error_error )
             with
            | (0, 0) ->
              (* If there's no leading or trailing whitespace, the
                 user has their cursor between the curly braces.

                 class Foo {AUTO332}

                 We don't want to offer completion here, because it
                 interferes with pressing enter to insert a
                 newline. *)
              ()
            | _ -> class_member_start_keywords filename error_error !inner_ctx)
          | _ -> ())
    | MethodishDeclaration md ->
      let header = md.methodish_function_decl_header in
      (match header.syntax with
      | FunctionDeclarationHeader fdh ->
        (match fdh.function_keyword.syntax with
        | Token t ->
          (* The user has written `public AUTO332 function`, so we're expecting
             method modifiers like `static`. *)
          List.iter (Syntax.Token.leading t) ~f:(method_keywords filename)
        | _ -> ())
      | _ -> ())
    | PropertyDeclaration pd ->
      (match (pd.property_type.syntax, ctx) with
      | (SimpleTypeSpecifier sts, Some Ast_defs.Cinterface) ->
        (* Interfaces cannot contain properties, and have fewer
           modifiers available on methods. *)
        interface_method_keywords
          filename
          pd.property_modifiers
          sts.simple_type_specifier
      | (SimpleTypeSpecifier sts, _) ->
        (* The user has written `public AUTO332`. This could be a method
           `public function foo(): void {}` or a property
           `public int $x = 1;`. *)
        property_or_method_keywords
          filename
          pd.property_modifiers
          sts.simple_type_specifier
      | _ -> ())
    | _ -> ());
    List.iter (children s) ~f:(aux !inner_ctx)
  in

  aux None tree

(* Main entry point for autocomplete *)
let go_ctx
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(autocomplete_context : AutocompleteTypes.legacy_autocomplete_context)
    ~(sienv_ref : SearchUtils.si_env ref)
    ~(naming_table : Naming_table.t) =
  reset ();

  let cst = Ast_provider.compute_cst ~ctx ~entry in
  let tree = Provider_context.PositionedSyntaxTree.root cst in
  keywords entry.Provider_context.path tree;

  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_quarantined ~ctx ~entry
  in
  let tast = tast.Tast_with_dynamic.under_normal_assumptions in
  (visitor ctx autocomplete_context sienv_ref naming_table tast)#go ctx tast;

  Errors.ignore_ (fun () ->
      let start_time = Unix.gettimeofday () in
      let autocomplete_items = !autocomplete_items in
      let results =
        {
          With_complete_flag.is_complete = !autocomplete_is_complete;
          value = autocomplete_items;
        }
      in
      SymbolIndexCore.log_symbol_index_search
        ~sienv:!sienv_ref
        ~start_time
        ~query_text:!auto_complete_for_global
        ~max_results
        ~kind_filter:None
        ~results:(List.length results.With_complete_flag.value)
        ~caller:"AutocompleteService.go";
      results)
