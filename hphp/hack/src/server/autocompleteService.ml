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
include AutocompleteTypes
open Tast
module Phase = Typing_phase
module Cls = Decl_provider.Class
module Syntax = Full_fidelity_positioned_syntax
module Trivia = Full_fidelity_positioned_trivia

let autocomplete_results : complete_autocomplete_result list ref = ref []

let autocomplete_is_complete : bool ref = ref true

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

(* When autocompletion is called, we insert an autocomplete token
   "AUTO332" at the position of the cursor. When we walk the TAST, we
   record the context that we saw a symbol containing this token.

   For example, if the user has this in their IDE, with | showing their
   cursor:

       $x = new Fo|

   We see the following TAST:

       $x = new FoAUTO332

   and we set argument_global_type to Acnew because we're completing in
   a class instantiation. *)
let (argument_global_type : autocomplete_type option ref) = ref None

let max_results = 100

let auto_complete_for_global = ref ""

let strip_suffix s =
  String.sub
    s
    ~pos:0
    ~len:(String.length s - AutocompleteTypes.autocomplete_token_length)

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
    List.filter !autocomplete_results ~f:(fun res ->
        match res.res_kind with
        | SI_Keyword -> false
        | _ -> true)
  in
  if List.is_empty results_without_keywords then
    matches_auto_complete_suffix str
  else
    false

let replace_pos_of_id (pos, text) : Ide_api_types.range =
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
          ( "params",
            Hh_json.JSON_Array (List.map fd.params ~f:func_param_to_json) );
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

let add_res (res : complete_autocomplete_result) : unit =
  autocomplete_results := res :: !autocomplete_results

let autocomplete_token ac_type x =
  if is_auto_complete (snd x) then (
    argument_global_type := Some ac_type;
    auto_complete_for_global := snd x
  )

let autocomplete_id id = autocomplete_token Acid id

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

let get_pos_for (env : Tast_env.env) (ty : Typing_defs.phase_ty) : Pos.absolute
    =
  (match ty with
  | LoclTy loclt -> loclt |> Typing_defs.get_pos
  | DeclTy declt -> declt |> Typing_defs.get_pos)
  |> ServerPos.resolve env
  |> Pos.to_absolute

(* Convert a `TFun` into a func details structure *)
let tfun_to_func_details (env : Tast_env.t) (ft : Typing_defs.locl_fun_type) :
    func_details_result =
  let n = List.length ft.ft_params in
  let param_to_record i param =
    {
      param_name =
        (match param.fp_name with
        | Some n -> n
        | None -> "");
      param_ty = Tast_env.print_ty env param.fp_type.et_type;
      param_variadic = i + 1 = n && get_ft_variadic ft;
    }
  in
  {
    return_ty = Tast_env.print_ty env ft.ft_ret.et_type;
    min_arity = arity_min ft;
    params = List.mapi ft.ft_params ~f:param_to_record;
  }

(* Convert a `ty` into a func details structure *)
let get_func_details_for env ty =
  let (env, ty) =
    match ty with
    | DeclTy ty -> Tast_env.localize_no_subst env ~ignore_errors:true ty
    | LoclTy ty -> (env, ty)
  in
  match Typing_defs.get_node ty with
  | Tfun ft -> Some (tfun_to_func_details env ft)
  | _ -> None

let autocomplete_shape_key autocomplete_context env fields id =
  if is_auto_complete (snd id) then (
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
        let ty = Phase.decl ty in
        let pos = get_pos_for env ty in
        let res_name =
          if autocomplete_context.is_before_apostrophe then
            rstrip code "'"
          else
            code
        in
        let complete =
          {
            res_pos = pos;
            res_replace_pos = replace_pos_of_id id;
            res_base_class = None;
            res_ty = kind_to_string kind;
            res_name;
            res_fullname = code;
            res_kind = kind;
            func_details = get_func_details_for env ty;
            ranking_details = None;
            res_documentation = None;
          }
        in
        add_res complete
    in

    List.iter (TShapeMap.keys fields) ~f:add
  )

let autocomplete_member ~is_static env class_ cid id =
  (* This is used for instance "$x->|" and static "Class1::|" members. *)
  if is_auto_complete (snd id) then (
    (* Detect usage of "parent::|" which can use both static and instance *)
    let parent_receiver =
      match cid with
      | Some Aast.CIparent -> true
      | _ -> false
    in
    argument_global_type := Some Acclass_get;
    let add kind (name, ty) =
      let res_ty = Tast_env.print_decl_ty env ty in
      let ty = Phase.decl ty in
      let complete =
        {
          res_pos = get_pos_for env ty;
          res_replace_pos = replace_pos_of_id id;
          res_base_class = Some (Cls.name class_);
          res_ty;
          res_name = name;
          res_fullname = name;
          res_kind = kind;
          func_details = get_func_details_for env ty;
          ranking_details = None;
          res_documentation = None;
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
        ~f:(add SearchUtils.SI_ClassMethod);
      List.iter
        (get_class_elt_types
           ~is_method:false
           env
           class_
           cid
           (Cls.sprops class_ |> sort))
        ~f:(add SearchUtils.SI_Property);
      List.iter
        (Cls.consts class_ |> sort)
        ~f:(fun (name, cc) ->
          add SearchUtils.SI_ClassConstant (name, cc.cc_type))
    );
    if (not is_static) || parent_receiver then (
      List.iter
        (get_class_elt_types
           ~is_method:true
           env
           class_
           cid
           (Cls.methods class_ |> sort))
        ~f:(add SearchUtils.SI_ClassMethod);
      List.iter
        (get_class_elt_types
           ~is_method:false
           env
           class_
           cid
           (Cls.props class_ |> sort))
        ~f:(add SearchUtils.SI_Property)
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
        ~f:(add SearchUtils.SI_ClassMethod)
  )

(*
  Autocompletion for XHP attribute names in an XHP literal.
*)
let autocomplete_xhp_attributes env class_ cid id attrs =
  (* This is used for "<nt:fb:text |" XHP attributes, in which case  *)
  (* class_ is ":nt:fb:text" and its attributes are in tc_props.     *)
  if is_auto_complete (snd id) && Cls.is_xhp class_ then (
    argument_global_type := Some Acprop;
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
          let kind = SearchUtils.SI_Property in
          let res_ty = Tast_env.print_decl_ty env ty in
          let ty = Phase.decl ty in
          let complete =
            {
              res_pos = get_pos_for env ty;
              res_replace_pos = replace_pos_of_id id;
              res_base_class = Some (Cls.name class_);
              res_ty;
              res_name = lstrip name ":";
              res_fullname = name;
              res_kind = kind;
              func_details = get_func_details_for env ty;
              ranking_details = None;
              res_documentation = None;
            }
          in
          add_res complete)
  )

let autocomplete_xhp_bool_value attr_ty id_id env =
  if is_auto_complete (snd id_id) then begin
    argument_global_type := Some Acprop;

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
      let kind = SearchUtils.SI_Literal in
      let ty = Phase.locl attr_ty in
      let complete =
        {
          res_pos = get_pos_for env ty;
          res_replace_pos = replace_pos_of_id id_id;
          res_base_class = None;
          res_ty = kind_to_string kind;
          res_name = "true";
          res_fullname = "true";
          res_kind = kind;
          func_details = None;
          ranking_details = None;
          res_documentation = None;
        }
      in

      add_res complete;
      add_res { complete with res_name = "false"; res_fullname = "false" }
    )
  end

(*
  Autocompletion for the value of an attribute in an XHP literals with the enum type,
  defined with the following syntax:
  attribute enum {"some value", "some other value"} my-attribute;
*)
let autocomplete_xhp_enum_attribute_value attr_name ty id_id env cls =
  if is_auto_complete (snd id_id) then begin
    argument_global_type := Some Acprop;

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
      let kind = SearchUtils.SI_Enum in
      let ty = Phase.locl ty in
      let complete =
        {
          res_pos = get_pos_for env ty;
          res_replace_pos = replace_pos_of_id id_id;
          res_base_class = None;
          res_ty = kind_to_string kind;
          res_name = name;
          res_fullname = name;
          res_kind = kind;
          func_details = get_func_details_for env ty;
          ranking_details = None;
          res_documentation = None;
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
    argument_global_type := Some Acprop;

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
                  let dty = Phase.decl ty.cc_type in
                  let name = Utils.strip_ns class_name ^ "::" ^ const_name in
                  let kind = SearchUtils.SI_Enum in
                  let res_base_class = Option.map ~f:Cls.name enum_class in

                  let complete =
                    {
                      res_pos = get_pos_for env dty;
                      res_replace_pos = replace_pos_of_id id_id;
                      res_base_class;
                      res_ty = kind_to_string kind;
                      res_name = name;
                      res_fullname = name;
                      res_kind = kind;
                      func_details = get_func_details_for env dty;
                      ranking_details = None;
                      res_documentation = None;
                    }
                  in
                  add_res complete))
  end

let autocomplete_lvar id =
  (* This is used for "$|" and "$x = $|" local variables. *)
  let text = Local_id.get_name (snd id) in
  if is_auto_complete text then argument_global_type := Some Acprop

let autocomplete_typed_member ~is_static env class_ty cid mid =
  Tast_env.get_class_ids env class_ty
  |> List.iter ~f:(fun cname ->
         Decl_provider.get_class (Tast_env.get_ctx env) cname
         |> Option.iter ~f:(fun class_ ->
                let cid = Option.map cid ~f:to_nast_class_id_ in
                autocomplete_member ~is_static env class_ cid mid))

let autocomplete_static_member env (ty, _, cid) mid =
  autocomplete_typed_member ~is_static:true env ty (Some cid) mid

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
        | Some expected_ty -> Tast_env.is_sub_type env cc_ty expected_ty
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
  let (_, ty_) = Typing_defs_core.deref ty in
  match ty_ with
  | Tapply ((_, name), [_; arg])
    when String.equal name Naming_special_names.Classes.cMemberOf ->
    arg
  | _ -> ty

let autocomplete_enum_class_label env opt_cname pos_labelname expected_ty =
  argument_global_type := Some Acclass_get;
  let suggest_members cls =
    List.iter
      (compatible_enum_class_consts env cls expected_ty)
      ~f:(fun (name, cc) ->
        let res_ty =
          Tast_env.print_decl_ty env (unwrap_enum_memberof cc.cc_type)
        in
        let ty = Phase.decl cc.cc_type in
        let kind = SearchUtils.SI_ClassConstant in
        let complete =
          {
            res_pos = get_pos_for env ty;
            res_replace_pos = replace_pos_of_id pos_labelname;
            res_base_class = Some (Cls.name cls);
            res_ty;
            res_name = name;
            res_fullname = name;
            res_kind = kind;
            func_details = get_func_details_for env ty;
            ranking_details = None;
            res_documentation = None;
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
  argument_global_type := Some Acclass_get;
  let suggest_members_from_ty env ty pos_labelname expected_ty =
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
  match get_node fty with
  | Tfun { ft_params; _ } ->
    let ty_args = zip_truncate args ft_params in
    List.iter
      ~f:(fun (arg, arg_ty) ->
        match (arg, get_node arg_ty.fp_type.et_type) with
        | ( (_, (_, p, Aast.EnumClassLabel (None, n))),
            Typing_defs.Tnewtype (ty_name, [enum_ty; member_ty], _) )
          when is_enum_class_label_ty_name ty_name && is_auto_complete n ->
          suggest_members_from_ty env enum_ty (p, n) (Some member_ty)
        | (_, _) -> ())
      ty_args
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
  | Aast.Hole (e, _, _, _) -> e
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
    argument_global_type := Some Acshape_key;

    let ty = Tprim Aast_defs.Tstring in
    let reason = Typing_reason.Rwitness pos in
    let ty = mk (reason, ty) in

    let kind = SI_Literal in
    let lty = Phase.locl ty in
    let complete =
      {
        res_pos = get_pos_for env lty;
        res_replace_pos = replace_pos_of_id (pos, key);
        res_base_class = None;
        res_ty = kind_to_string kind;
        res_name = key;
        res_fullname = key;
        res_kind = kind;
        func_details = get_func_details_for env lty;
        ranking_details = None;
        res_documentation = None;
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
              | Tshape (_, fields) ->
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
  argument_global_type := Some Acid;

  let complete =
    {
      res_pos = Pos.to_absolute Pos.none;
      res_replace_pos = replace_pos;
      res_base_class = None;
      res_ty = "built-in attribute";
      res_name = name;
      res_fullname = name;
      res_kind = SI_Class;
      func_details = None;
      ranking_details = None;
      res_documentation = Some doc;
    }
  in
  add_res complete

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
  argument_global_type := Some Acid;

  let ty = Tprim Aast_defs.Tstring in
  let reason = Typing_reason.Rwitness pos in
  let ty = mk (reason, ty) in

  let kind = SI_ClassConstant in
  let lty = Phase.locl ty in
  let key = prefix ^ const_name in
  let complete =
    {
      res_pos = get_pos_for env lty;
      res_replace_pos = replace_pos;
      res_base_class = None;
      res_ty = kind_to_string kind;
      res_name = key;
      res_fullname = key;
      res_kind = kind;
      func_details = get_func_details_for env lty;
      ranking_details = None;
      res_documentation = None;
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
        let (_, ty) = Tast_env.expand_type env recv_ty in
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
        let (_, ty) = Tast_env.expand_type env expected_ty.fp_type.et_type in
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
        let (_, ty) = Tast_env.expand_type env expected_ty.fp_type.et_type in
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
    ~(id : Pos.t * string)
    ~(completion_type : SearchUtils.autocomplete_type option)
    ~(autocomplete_context : AutocompleteTypes.legacy_autocomplete_context)
    ~(sienv : SearchUtils.si_env)
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
    (* Ensure that we do not have a leading backslash for Hack classes,
     * while ensuring that we have colons for XHP classes.  That's how we
     * differentiate between them. *)
    let query_text = strip_suffix (snd id) in
    let query_text =
      if autocomplete_context.is_xhp_classname then
        Utils.add_xhp_ns query_text
      else
        Utils.strip_ns query_text
    in
    auto_complete_for_global := query_text;
    let (ns, _) = Utils.split_ns_from_name query_text in
    let absolute_none = Pos.none |> Pos.to_absolute in
    let kind_filter =
      match completion_type with
      | Some Acnew -> Some SI_Class
      | Some Actrait_only -> Some SI_Trait
      | _ -> None
    in

    let results =
      SymbolIndex.find_matching_symbols
        ~sienv
        ~query_text
        ~max_results
        ~kind_filter
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
            res_replace_pos = replace_pos_of_id id;
            res_base_class = None;
            res_ty;
            res_name;
            res_fullname;
            res_kind = r.si_kind;
            func_details;
            ranking_details = None;
            res_documentation = None;
          }
        in
        add_res complete);
    autocomplete_is_complete := List.length !autocomplete_results < max_results;

    (* Add any builtins that match *)
    match completion_type with
    | Some Actype
    | None ->
      builtin_type_hints
      |> List.filter ~f:(fun (_, name) ->
             String_utils.string_starts_with name query_text)
      |> List.iter ~f:(fun (hint, name) ->
             let kind = SI_Typedef in
             let documentation = SymbolOccurrence.built_in_type_hover hint in
             add_res
               {
                 res_pos = absolute_none;
                 res_replace_pos = replace_pos_of_id id;
                 res_base_class = None;
                 res_ty = "builtin";
                 res_name = name;
                 res_fullname = name;
                 res_kind = kind;
                 func_details = None;
                 ranking_details = None;
                 res_documentation = Some documentation;
               })
    | _ -> ()

let visitor ctx autocomplete_context sienv =
  object (self)
    inherit Tast_visitor.iter as super

    method complete_global id ac_type : unit =
      if is_auto_complete (snd id) then
        let completion_type = Some ac_type in
        find_global_results
          ~id
          ~completion_type
          ~autocomplete_context
          ~sienv
          ~pctx:ctx

    method complete_id (id : Ast_defs.id) : unit = self#complete_global id Acid

    method! on_def env d =
      match d with
      | Aast.Stmt _ ->
        (* Don't try to complete top-level statements. If we see
           'fAUTO332' the user will be expecting us to complete
           'function', not a top-level expression
           'foo_type::method()`. *)
        ()
      | _ -> super#on_def env d

    method! on_Id env id =
      autocomplete_id id;
      self#complete_id id;
      super#on_Id env id

    method! on_Call env f targs args unpack_arg =
      autocomplete_enum_class_label_call env f args;
      super#on_Call env f targs args unpack_arg

    method! on_Fun_id env id =
      autocomplete_id id;
      self#complete_id id;
      super#on_Fun_id env id

    method! on_New env ((_, _, cid_) as cid) el unpacked_element =
      (match cid_ with
      | Aast.CI id -> self#complete_global id Acnew
      | _ -> ());
      super#on_New env cid el unpacked_element

    method! on_Happly env sid hl =
      self#complete_global sid Actype;
      super#on_Happly env sid hl

    method! on_Lvar env lid =
      autocomplete_lvar lid;
      super#on_Lvar env lid

    method! on_Class_get env cid mid prop_or_method =
      match mid with
      | Aast.CGstring p -> autocomplete_static_member env cid p
      | Aast.CGexpr _ -> super#on_Class_get env cid mid prop_or_method

    method! on_Class_const env cid mid =
      autocomplete_static_member env cid mid;
      super#on_Class_const env cid mid

    method! on_Obj_get env obj mid ognf =
      (match mid with
      | (_, _, Aast.Id mid) ->
        autocomplete_typed_member ~is_static:false env (get_type obj) None mid
      | _ -> ());
      super#on_Obj_get env obj mid ognf

    method! on_expr env expr =
      (match expr with
      | (_, _, Aast.Array_get (arr, Some (_, pos, key))) ->
        let ty = get_type arr in
        let (_, ty) = Tast_env.expand_type env ty in
        begin
          match get_node ty with
          | Tshape (_, fields) ->
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
      | (_, _, Aast.Call ((recv_ty, _, _), _, args, _)) ->
        (match deref recv_ty with
        | (_r, Tfun ft) ->
          autocomplete_shape_literal_in_call env ft args;
          autocomplete_enum_value_in_call env ft args
        | _ -> ())
      | (_, p, Aast.EnumClassLabel (opt_cname, n)) when is_auto_complete n ->
        autocomplete_enum_class_label env opt_cname (p, n) None
      | (_, _, Aast.Efun (f, _))
      | (_, _, Aast.Lfun (f, _)) ->
        List.iter f.Aast.f_user_attributes ~f:(fun ua ->
            autocomplete_builtin_attribute
              ua.Aast.ua_name
              Naming_special_names.AttributeKinds.lambda)
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
      autocomplete_id trimmed_sid;
      self#complete_id trimmed_sid;

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
                     autocomplete_member ~is_static:false env c (Some cid) id
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
            self#complete_global id Actrait_only;
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
class local_types =
  object (self)
    inherit Tast_visitor.iter as super

    val mutable results = Local_id.Map.empty

    val mutable id_at_cursor = None

    method get_types ctx tast =
      self#go ctx tast;
      (results, id_at_cursor)

    method add env id ty =
      (* If we already have a type for this identifier, don't overwrite it with
         results from after the cursor position. *)
      if not (Local_id.Map.mem id results && Option.is_some id_at_cursor) then
        let ty = LoclTy ty in
        results <-
          Local_id.Map.add
            id
            (get_func_details_for env ty, get_pos_for env ty)
            results

    method! on_fun_ env f = if fun_contains_cursor f then super#on_fun_ env f

    method! on_method_ env m =
      if method_contains_cursor m then (
        if not m.Aast.m_static then
          self#add env Typing_defs.this (Tast_env.get_self_ty_exn env);
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
          self#add env id ty
      | Aast.Binop (Ast_defs.Eq _, e1, e2) ->
        (* Process the rvalue before the lvalue, since the lvalue is annotated
           with its type after the assignment. *)
        self#on_expr env e2;
        self#on_expr env e1
      | _ -> super#on_expr env e

    method! on_fun_param env fp =
      let id = Local_id.make_unscoped fp.Aast.param_name in
      let ty = fp.Aast.param_annotation in
      self#add env id ty
  end

let compute_complete_local ctx tast =
  let (locals, id_at_cursor) = (new local_types)#get_types ctx tast in
  let replace_pos =
    replace_pos_of_id (Option.value id_at_cursor ~default:(Pos.none, ""))
  in
  Local_id.Map.iter
    (fun id (func_details, pos) ->
      let kind = SearchUtils.SI_LocalVariable in
      let name = Local_id.get_name id in
      let complete =
        {
          res_pos = pos;
          res_replace_pos = replace_pos;
          res_base_class = None;
          res_ty = kind_to_string kind;
          res_name = name;
          res_fullname = name;
          res_kind = kind;
          func_details;
          ranking_details = None;
          res_documentation = None;
        }
      in
      add_res complete)
    locals

let reset () =
  auto_complete_for_global := "";
  argument_global_type := None;
  autocomplete_results := [];
  autocomplete_is_complete := true

let complete_keywords_at possible_keywords text pos : unit =
  if is_auto_complete text then
    let prefix = strip_suffix text in
    possible_keywords
    |> List.filter ~f:(fun possible_keyword ->
           String.is_prefix possible_keyword ~prefix)
    |> List.iter ~f:(fun keyword ->
           let kind = SI_Keyword in
           let complete =
             {
               res_pos = Pos.none |> Pos.to_absolute;
               res_replace_pos = replace_pos_of_id (pos, text);
               res_base_class = None;
               res_ty = kind_to_string kind;
               res_name = keyword;
               res_fullname = keyword;
               res_kind = kind;
               func_details = None;
               ranking_details = None;
               res_documentation = None;
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
  let trait_member_keywords = ["require extends"; "require implements"] in
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
          | ErrorSyntax es ->
            class_member_start_keywords filename es.error_error !inner_ctx
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
    ~(sienv : SearchUtils.si_env) =
  reset ();

  let cst = Ast_provider.compute_cst ~ctx ~entry in
  let tree = Provider_context.PositionedSyntaxTree.root cst in
  keywords entry.Provider_context.path tree;

  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_quarantined ~ctx ~entry
  in
  (visitor ctx autocomplete_context sienv)#go ctx tast;

  Errors.ignore_ (fun () ->
      let start_time = Unix.gettimeofday () in
      let completion_type = !argument_global_type in
      let ( = ) = Option.equal equal_autocomplete_type in

      if completion_type = Some Acprop then compute_complete_local ctx tast;

      let complete_autocomplete_results = !autocomplete_results in
      let replace_pos =
        match List.hd complete_autocomplete_results with
        | Some r -> r.res_replace_pos
        | None -> Pos.none |> Pos.to_absolute |> Ide_api_types.pos_to_range
      in

      let kind_filter =
        match completion_type with
        | Some Acnew -> Some SI_Class
        | Some Actrait_only -> Some SI_Trait
        | _ -> None
      in

      let results =
        {
          With_complete_flag.is_complete = !autocomplete_is_complete;
          value =
            (if sienv.use_ranked_autocomplete then (
              let ranking_start_time = Unix.gettimeofday () in
              let ranked_results =
                AutocompleteRankService.rank_autocomplete_result
                  ~ctx
                  ~entry
                  ~query_text:!auto_complete_for_global
                  ~results:complete_autocomplete_results
                  ~max_results:3
                  ~context:completion_type
                  ~kind_filter
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
              complete_autocomplete_results);
        }
      in
      SymbolIndexCore.log_symbol_index_search
        ~sienv
        ~start_time
        ~query_text:!auto_complete_for_global
        ~max_results
        ~kind_filter
        ~results:(List.length results.With_complete_flag.value)
        ~context:completion_type
        ~caller:"AutocompleteService.go";
      results)
