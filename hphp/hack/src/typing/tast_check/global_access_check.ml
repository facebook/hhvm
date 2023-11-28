(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * This global access checker raises an error when:
 * - a global variable is written:
 *   - a global variable is directly written (e.g. (Foo::$bar)->prop = 1;)
 *   - a global variable is written via references (e.g. $a = Foo::$bar; $a->prop = 1;)
 *   - a global variable is passed to a function call, in which it may be written.
 * - or a global variable is read.
 *
 * Notice that the return value of a memoized function (if it is mutable)
 * is treated as a global variable as well.
 *
 * By default, this checker is turned off.
 * To turn on this checker, use the argument --enable-global-access-check.
 *
 * A trick to run this checker on a specific list of files is to
 *   use "--config enable_type_check_filter_files=true" togehter with above arguments,
 *   which runs typechecker only on files listed in ~/.hack_type_check_files_filter.
 *)

open Hh_prelude
open Aast
module MakeType = Typing_make_type
module Reason = Typing_reason
module Cls = Decl_provider.Class
module Hashtbl = Stdlib.Hashtbl
module Option = Stdlib.Option
module GlobalAccessCheck = Error_codes.GlobalAccessCheck
module SN = Naming_special_names

(* Recognize common patterns for global access (only writes for now). *)
type global_access_pattern =
  | Singleton (* Write to a global variable whose value is null *)
  | Caching
    (* Write to a global collection when the element is null or does not exist *)
  | CounterIncrement (* Increase a global variable by 1 or -1 *)
  | WriteEmptyOrNull (* Assign null or empty string to a global variable *)
  | WriteLiteral (* Assign a literal to a global variable *)
  | WriteGlobalToGlobal (* Assign a global variable to another one *)
  | WriteNonSensitive (* Write non-sensitive data to a global variable *)
  | SuperGlobalWrite (* Global write via super global functions *)
  | SuperGlobalRead (* Global read via super global functions *)
  | NoPattern (* No pattern above is recognized *)
[@@deriving ord, show { with_path = false }]

module GlobalAccessPatternSet = Stdlib.Set.Make (struct
  type t = global_access_pattern

  let compare = compare_global_access_pattern
end)

(* Raise a global access error if the corresponding write/read mode is enabled. *)
let raise_global_access_error
    pos fun_name data_type global_set patterns error_code =
  let error_message =
    match error_code with
    | GlobalAccessCheck.DefiniteGlobalWrite -> "definitely written."
    | GlobalAccessCheck.PossibleGlobalWriteViaReference ->
      "possibly written via reference."
    | GlobalAccessCheck.PossibleGlobalWriteViaFunctionCall ->
      "possibly written via function call."
    | GlobalAccessCheck.DefiniteGlobalRead -> "definitely read."
  in
  let global_vars_str = String.concat ~sep:";" (SSet.elements global_set) in
  let print_pattern p =
    match p with
    | NoPattern -> None
    | _ as p -> Some (show_global_access_pattern p)
  in
  let patterns_str =
    String.concat
      ~sep:","
      (Stdlib.List.filter_map
         print_pattern
         (GlobalAccessPatternSet.elements patterns))
  in
  let message =
    "["
    ^ fun_name
    ^ "]{"
    ^ global_vars_str
    ^ "}("
    ^ data_type
    ^ ")"
    ^ (if String.length patterns_str > 0 then
        "<" ^ patterns_str ^ ">"
      else
        "")
    ^ " A global variable is "
    ^ error_message
  in
  Errors.global_access_error error_code pos message

(* Various types of data sources for a local variable. *)
type data_source =
  | GlobalReference of string (* A reference to a mutable global var *)
  | GlobalValue of string (* The value of a immutable global var *)
  | NullOrEmpty (* null or empty string *)
  | Literal (* Boolean, integer, floating-point, or string literals *)
  | NonSensitive (* E.g. timer, site var *)
  | Unknown (* Other sources that have not been identified yet *)
[@@deriving ord, show]

module DataSourceSet = Stdlib.Set.Make (struct
  type t = data_source

  let compare = compare_data_source
end)

(* Given the data sources of written data (i.e. RHS of assignment),
   return the recognized patterns. Specially, if there is unknown
   data sources, no pattern is recognized. *)
let get_patterns_from_written_data_srcs srcs =
  let is_unknown s =
    match s with
    | Unknown -> true
    | _ -> false
  in
  if not (DataSourceSet.exists is_unknown srcs) then
    let add_src_to_patterns s acc =
      let p =
        match s with
        | GlobalReference _
        | GlobalValue _ ->
          WriteGlobalToGlobal
        | NullOrEmpty -> WriteEmptyOrNull
        | Literal -> WriteLiteral
        | NonSensitive -> WriteNonSensitive
        | Unknown -> NoPattern
      in
      GlobalAccessPatternSet.add p acc
    in
    DataSourceSet.fold add_src_to_patterns srcs GlobalAccessPatternSet.empty
  else
    GlobalAccessPatternSet.empty

(* The set of safe/non-sensitive functions or classes *)
let safe_func_ids = SSet.of_list ["\\microtime"; "\\FlibSL\\PHP\\microtime"]

let safe_class_ids = SSet.of_list ["\\Time"]

(* The set of functions whose return value's data sources are
   the union of the data sources of all its parameters. *)
let src_from_para_func_ids =
  SSet.of_list
    [
      "\\FlibSL\\Dict\\merge";
      "\\HH\\Lib\\Dict\\merge";
      "\\FlibSL\\Keyset\\union";
      "\\HH\\Lib\\Keyset\\union";
    ]

(* Functions whose return value is from only its first parameter. *)
let src_from_first_para_func_ids =
  SSet.of_list
    ["\\HH\\idx"; "\\FlibSL\\Dict\\filter_keys"; "\\HH\\Lib\\Dict\\filter_keys"]

(* Functions that check if its first parameter is not null. *)
let check_non_null_func_ids =
  SSet.of_list ["\\FlibSL\\C\\contains_key"; "\\HH\\Lib\\C\\contains_key"]

(* The type to represent the super global classes under www/flib/core/superglobals. *)
type super_global_class_detail = {
  (* global_variable is None if the corresponding super global variable is not fixed,
     but is given as the parameter of get/set methods, e.g. in GlobalVARIABLES. *)
  global_variable: string option;
  global_write_methods: SSet.t;
  global_read_methods: SSet.t;
}

(* Map from super global class names to the corresponding details. *)
let super_global_class_map : super_global_class_detail SMap.t =
  SMap.of_list
    [
      ( "\\GlobalCOOKIE",
        {
          global_variable = Some "$_COOKIE";
          global_write_methods =
            SSet.of_list
              [
                "redactCookieSuperglobal";
                "overrideCookieSuperglobalForEmulation";
                "setForCookieMonsterInternalTest";
              ];
          global_read_methods =
            SSet.of_list
              [
                "getInitialKeys";
                "getCopy_COOKIE_MONSTER_INTERNAL_ONLY";
                "getCopyForEmulation";
              ];
        } );
      ( "\\GlobalENV",
        {
          global_variable = Some "$_ENV";
          global_write_methods = SSet.of_list ["set_ENV_DO_NOT_USE"; "set"];
          global_read_methods =
            SSet.of_list
              ["getCopy_DO_NOT_USE"; "get"; "getReadonly"; "idx"; "idxReadonly"];
        } );
      ( "\\GlobalFILES",
        {
          global_variable = Some "$_FILES";
          global_write_methods = SSet.of_list ["set_FILES_DO_NOT_USE"; "set"];
          global_read_methods =
            SSet.of_list ["getCopy_DO_NOT_USE"; "get"; "getReadonly"; "idx"];
        } );
      ( "\\GlobalGET",
        {
          global_variable = Some "$_GET";
          global_write_methods = SSet.of_list ["set_GET_DO_NOT_USE"; "set"];
          global_read_methods =
            SSet.of_list
              ["getCopy_DO_NOT_USE"; "get"; "getReadonly"; "idx"; "idxReadonly"];
        } );
      ( "\\GlobalPOST",
        {
          global_variable = Some "$_POST";
          global_write_methods = SSet.of_list ["set_POST_DO_NOT_USE"; "set"];
          global_read_methods =
            SSet.of_list
              ["getCopy_DO_NOT_USE"; "get"; "getReadonly"; "idx"; "idxReadonly"];
        } );
      ( "\\GlobalREQUEST",
        {
          global_variable = Some "$_REQUEST";
          global_write_methods =
            SSet.of_list
              [
                "set_REQUEST_DO_NOT_USE";
                "set";
                "redactCookiesFromRequestSuperglobal";
              ];
          global_read_methods =
            SSet.of_list ["getCopy_DO_NOT_USE"; "get"; "getReadonly"; "idx"];
        } );
      ( "\\GlobalSERVER",
        {
          global_variable = Some "$_SERVER";
          global_write_methods = SSet.of_list ["set_SERVER_DO_NOT_USE"; "set"];
          global_read_methods =
            SSet.of_list
              [
                "getCopy_DO_NOT_USE";
                "getDocumentRoot";
                "getPHPRoot";
                "getPHPSelf";
                "getRequestTime";
                "getParentRequestTime";
                "getRequestTimeFloat";
                "getParentRequestTimeFloat";
                "getScriptName";
                "getServerAddress";
                "getArgv";
                "getArgc";
                "getHTTPAuthorization";
                "getHTTPHost";
                "getHTTPReferer_DO_NOT_USE";
                "getHTTPUserAgent";
                "getRequestMethod";
                "getSecFetchDest";
                "getSecFetchMode";
                "getSecFetchSite";
                "getSecFetchUser";
                "getXFBLSD";
                "getTFBENV";
                "getRequestURIString";
                "getRequestURI_UNSAFE_TO_LOG";
                "getServerName";
                "getUserName";
                "getLocalCanaryID";
                "getInterestingIniKeys";
                "idx";
                "idxReadonly";
                "get";
                "getReadonly";
                "idxString_UNSAFE";
                "idxArgv";
              ];
        } );
      (* GlobalSESSION is not implemented and hence omitted. *)
      ( "\\GlobalVARIABLES",
        {
          (* The global variable is not fixed but given as the methods's parameter. *)
          global_variable = None;
          global_write_methods = SSet.of_list ["set"];
          global_read_methods =
            SSet.of_list ["idxReadonly"; "idx"; "get"; "getReadonly"];
        } );
    ]

(* Given an expression, check if it's a call of static method from some super global class.
   If so, raise the global read/write error, and return true; otherwise, return false. *)
let check_super_global_method expr env external_fun_name =
  match expr with
  | Call
      {
        func =
          ( func_ty,
            pos,
            Class_const ((_, _, CI (_, class_name)), (_, method_name)) );
        args;
        _;
      } ->
    (* Check if the class is a super global class *)
    (match SMap.find_opt class_name super_global_class_map with
    | Some class_detail ->
      (* Check if the method is write or read or not recognized. *)
      let error_code_pattern_opt =
        if SSet.mem method_name class_detail.global_write_methods then
          Some (GlobalAccessCheck.DefiniteGlobalWrite, SuperGlobalWrite)
        else if SSet.mem method_name class_detail.global_read_methods then
          Some (GlobalAccessCheck.DefiniteGlobalRead, SuperGlobalRead)
        else
          None
      in
      if Option.is_some error_code_pattern_opt then (
        let func_ty_str = Tast_env.print_ty env func_ty in
        let (error_code, pattern) = Option.get error_code_pattern_opt in
        let default_global_var_name = "$_" in
        let global_var_name =
          match class_detail.global_variable with
          | Some var_name -> var_name
          | None ->
            (* If the super global class does not have fixed global var name,
               then the 1st parameter of those get/set methods is assumed to
               be the global var; and if it's a string literal, we simply use
               it, otherwise we don't know and use default name. *)
            (match args with
            | (_, (_, _, para_expr)) :: _ ->
              (match para_expr with
              | String s -> "$" ^ s
              | _ -> default_global_var_name)
            | [] -> default_global_var_name)
        in
        raise_global_access_error
          pos
          external_fun_name
          ("superglobal " ^ func_ty_str)
          (SSet.singleton global_var_name)
          (GlobalAccessPatternSet.singleton pattern)
          error_code;
        true
      ) else
        false
    | _ -> false)
  | _ -> false

(* The context maintains:
   (1) A hash table from local variables to the corresponding data sources.
   For any local variable, if its data sources contain GlobalReference(s), then
   it is potentially a reference to global variable. Notice that a local variable
   may refers to multiple global variables, for example:
   "if (condition) { $a = Foo::$bar; } else { $a = memoized_func(); }"
   after the above code, $a has a reference to either Foo::$bar or memoized_func,
   thus var_data_src_tbl is like {"a" => {GlobalReference of "Foo::$bar",
   GlobalReference of "\memoized_func"}}.
   (2) A set of global variables whose values are null, which can be used to
   identify singletons. For example, consider the following program:
   "if (Foo::$bar is null) { Foo::$bar = new Bar(); }"
   inside the true branch, null_global_var_set is {"Foo::$bar"}, and
   the assignment to Foo::$bar shall be identified as a singleton. *)
type ctx = {
  var_data_src_tbl: (string, DataSourceSet.t) Hashtbl.t ref;
  null_global_var_set: SSet.t ref;
}

let current_ctx =
  {
    var_data_src_tbl = ref (Hashtbl.create 0);
    null_global_var_set = ref SSet.empty;
  }

(* Add the key (a variable name) and the value (a set of data srcs) to the table. *)
let add_var_data_srcs_to_tbl tbl var srcs =
  let pre_data_src_set =
    if Hashtbl.mem tbl var then
      Hashtbl.find tbl var
    else
      DataSourceSet.empty
  in
  Hashtbl.replace tbl var (DataSourceSet.union pre_data_src_set srcs)

let replace_var_data_srcs_in_tbl tbl var srcs = Hashtbl.replace tbl var srcs

(* Given two hash tables of type (string, DataSourceSet.t) Hashtbl.t, merge the second
   table into the first one. *)
let merge_var_data_srcs_tbls tbl1 tbl2 =
  Hashtbl.iter (add_var_data_srcs_to_tbl tbl1) tbl2

(* For a hash table whose value is DataSourceSet, get its total cardinal. *)
let get_tbl_total_cardinal tbl =
  Hashtbl.fold
    (fun _ v cur_cardinal -> cur_cardinal + DataSourceSet.cardinal v)
    tbl
    0

let rec grab_class_elts_from_ty ~static ?(seen = SSet.empty) env ty prop_id =
  let open Typing_defs in
  (* Given a list of types, find recurse on the first type that
     has the property and return the result *)
  let find_first_in_list ~seen tyl =
    List.find_map
      ~f:(fun ty ->
        match grab_class_elts_from_ty ~static ~seen env ty prop_id with
        | [] -> None
        | tyl -> Some tyl)
      tyl
  in
  match get_node ty with
  | Tclass (id, _exact, _args) ->
    let provider_ctx = Tast_env.get_ctx env in
    let class_decl = Decl_provider.get_class provider_ctx (snd id) in
    (match class_decl with
    | Decl_entry.Found class_decl ->
      let prop =
        if static then
          Cls.get_sprop class_decl (snd prop_id)
        else
          Cls.get_prop class_decl (snd prop_id)
      in
      Option.to_list prop
    | Decl_entry.DoesNotExist
    | Decl_entry.NotYetAvailable ->
      [])
  (* Accessing a property off of an intersection type
     should involve exactly one kind of readonlyness, since for
     the intersection type to exist, the property must be related
     by some subtyping relationship anyways, and property readonlyness
     is invariant. Thus we just grab the first one from the list where the prop exists. *)
  | Tintersection [] -> []
  | Tintersection tyl ->
    find_first_in_list ~seen tyl |> Option.value ~default:[]
  (* A union type is more interesting, where we must return all possible cases
     and be conservative in our use case. *)
  | Tunion tyl ->
    List.concat_map
      ~f:(fun ty -> grab_class_elts_from_ty ~static ~seen env ty prop_id)
      tyl
  (* Generic types can be treated similarly to an intersection type
     where we find the first prop that works from the upper bounds *)
  | Tgeneric (name, tyargs) ->
    (* Avoid circular generics with a set *)
    if SSet.mem name seen then
      []
    else
      let new_seen = SSet.add name seen in
      let upper_bounds = Tast_env.get_upper_bounds env name tyargs in
      find_first_in_list ~seen:new_seen (Typing_set.elements upper_bounds)
      |> Option.value ~default:[]
  | Tdependent (_, ty) ->
    (* Dependent types have an upper bound that's a class or generic *)
    grab_class_elts_from_ty ~static ~seen env ty prop_id
  | Toption ty ->
    (* If it's nullable, take the *)
    grab_class_elts_from_ty ~static ~seen env ty prop_id
  | _ -> []

(* Return a list of possible static prop elts given a class_get expression *)
let get_static_prop_elts env class_id get =
  let (ty, _, _) = class_id in
  match get with
  | CGstring prop_id -> grab_class_elts_from_ty ~static:true env ty prop_id
  (* An expression is dynamic, so there's no way to tell the type generally *)
  | CGexpr _ -> []

(* Check if an expression is directly from a static variable or not,
   e.g. it returns true for Foo::$bar or (Foo::$bar)->prop. *)
let rec is_expr_static env (_, _, te) =
  match te with
  | Class_get (class_id, expr, Is_prop) ->
    (* Ignore static variables annotated with <<__SafeForGlobalAccessCheck>> *)
    let class_elts = get_static_prop_elts env class_id expr in
    not (List.exists class_elts ~f:Typing_defs.get_ce_safe_global_variable)
  | Obj_get (e, _, _, Is_prop) -> is_expr_static env e
  | Array_get (e, _) -> is_expr_static env e
  | _ -> false

(* Print out global variables, e.g. Foo::$bar => "Foo::$bar", self::$bar => "Foo::$bar",
   static::$bar => "this::$bar", memoized_func => "\memoized_func", $baz->memoized_method
   => "Baz->memoized_method", Baz::memoized_method => "Baz::memoized_method",
   $this->memoized_method => "this->memoized_method".
   Notice that this does not handle arbitrary expressions. *)
let rec print_global_expr env expr =
  match expr with
  | Call { func = (_, _, caller_expr); _ } ->
    (* For function/method calls, we print the caller expression, which could be
       Id (e.g. memoized_func()) or Obj_get (e.g. $baz->memoized_method()). *)
    print_global_expr env caller_expr
  | Class_get ((c_ty, _, _), expr, _) ->
    (* For static properties, we concatenate the class type (instead of class_id_, which
       could be self, parent, static) and the property name. *)
    let class_ty_str = Tast_env.print_ty env c_ty in
    (match expr with
    | CGstring (_, expr_str) -> class_ty_str ^ "::" ^ expr_str
    | CGexpr _ -> class_ty_str ^ "::Unknown")
  | Class_const ((c_ty, _, _), (_, const_str)) ->
    (* For static method calls, we concatenate the class type and the method name. *)
    Tast_env.print_ty env c_ty ^ "::" ^ const_str
  | Id (_, name) -> name
  | Obj_get (obj, m, _, _) ->
    (* For Obj_get (e.g. $baz->memoized_method()), we concatenate the class type and the method id. *)
    let class_ty_str = Tast_env.print_ty env (Tast.get_type obj) in
    let (_, _, m_id) = m in
    (* For the case $obj?->method(), the question mark is removed from the class type,
       since we are not interested in the case where $obj is null. *)
    let remove_question_mark_prefix str =
      if String.is_prefix ~prefix:"?" str then
        String.sub str ~pos:1 ~len:(String.length str - 1)
      else
        str
    in
    remove_question_mark_prefix class_ty_str ^ "->" ^ print_global_expr env m_id
  | _ -> "Unknown"

(* Given a function call of type Aast.expr_, if it's memoized, return its function
   name, otherwise return None. *)
let check_func_is_memoized func_expr env =
  let open Typing_defs in
  let rec find_fty ty =
    match get_node ty with
    | Tnewtype (name, _, ty) when String.equal name SN.Classes.cSupportDyn ->
      find_fty ty
    | Tfun fty -> Some fty
    | _ -> None
  in
  let (func_ty, _, te) = func_expr in
  match find_fty func_ty with
  | Some fty when get_ft_is_memoized fty -> Some (print_global_expr env te)
  | _ -> None

(* Check if type is a collection. *)
let is_value_collection_ty env ty =
  let mixed = MakeType.mixed Reason.none in
  let env = Tast_env.tast_env_as_typing_env env in
  let hackarray = MakeType.any_array Reason.none mixed mixed in
  (* Subtype against an empty open shape (shape(...)) *)
  let shape = MakeType.open_shape Reason.none Typing_defs.TShapeMap.empty in
  Typing_utils.is_sub_type env ty hackarray
  || Typing_utils.is_sub_type env ty shape

(* Check if the variable type does NOT has a reference to any object:
   if so, then it is OK to write to this variable.
   Copied from is_safe_mut_ty in readonly_check.ml.
   To do: check if any change is needed for the global write checker. *)
let rec has_no_object_ref_ty env (seen : SSet.t) ty =
  let open Typing_defs_core in
  let (env, ty) = Tast_env.expand_type env ty in
  let tenv = Tast_env.tast_env_as_typing_env env in
  let ty = Typing_utils.strip_dynamic tenv ty in
  match get_node ty with
  (* Allow all primitive types *)
  | Tprim _ -> true
  (* Open shapes can technically have objects in them, but as long as the current fields don't have objects in them
     we will allow you to call the function. Note that the function fails at runtime if any shape fields are objects. *)
  | Tshape { s_fields = fields; _ } ->
    TShapeMap.for_all
      (fun _k v -> has_no_object_ref_ty env seen v.sft_ty)
      fields
  (* If it's a Tclass it's an array type by is_value_collection *)
  | Tintersection tyl ->
    List.exists tyl ~f:(fun l -> has_no_object_ref_ty env seen l)
  (* Only error if there isn't a type that it could be that's primitive *)
  | Tunion tyl -> List.exists tyl ~f:(fun l -> has_no_object_ref_ty env seen l)
  | Ttuple tyl -> List.for_all tyl ~f:(fun l -> has_no_object_ref_ty env seen l)
  | Tdependent (_, upper) ->
    (* check upper bounds *)
    has_no_object_ref_ty env seen upper
  | Tclass ((_, id), _, tyl)
    when is_value_collection_ty env ty || String.equal id "\\HH\\Awaitable" ->
    List.for_all tyl ~f:(fun l -> has_no_object_ref_ty env seen l)
  | Tgeneric (name, tyargs) ->
    (* Avoid circular generics with a set *)
    if SSet.mem name seen then
      false
    else
      let new_seen = SSet.add name seen in
      let upper_bounds = Tast_env.get_upper_bounds env name tyargs in
      Typing_set.exists
        (fun l -> has_no_object_ref_ty env new_seen l)
        upper_bounds
  | _ ->
    (* Otherwise, check if there's any primitive type it could be *)
    let env = Tast_env.tast_env_as_typing_env env in
    let primitive_types =
      [
        MakeType.bool Reason.none;
        MakeType.int Reason.none;
        MakeType.arraykey Reason.none;
        MakeType.string Reason.none;
        MakeType.float Reason.none;
        MakeType.num Reason.none;
        (* Keysets only contain arraykeys so if they're readonly its safe to remove *)
        MakeType.keyset Reason.none (MakeType.arraykey Reason.none);
        (* We don't put null here because we want to exclude ?Foo.
           as_mut(null) itself is allowed by the Tprim above*)
      ]
    in
    (* Make sure that a primitive *could* be this type by intersecting all primitives and subtyping. *)
    let union = MakeType.union Reason.none primitive_types in
    not (Typing_subtype.is_type_disjoint env ty union)

(* Get all possible data sources for the given expression. *)
let rec get_data_srcs_from_expr env ctx (tp, _, te) =
  let is_immutable_ty = has_no_object_ref_ty env SSet.empty tp in
  let convert_ref_to_val srcs =
    DataSourceSet.map
      (fun src ->
        match src with
        | GlobalReference s -> GlobalValue s
        | _ as s -> s)
      srcs
  in
  (* For a collection (Varray, ValCollection and List) that uses a list of expressions
     (e.g. vec[$a, $b] uses the list [$a; $b]), the function get_data_srcs_of_expr_list
     gets the data source for each expression in the list and do the union; specially,
     if the list is empty (e.g. vec[]), the data source is NullOrEmpty. *)
  let get_data_srcs_of_expr_list list =
    if List.is_empty list then
      DataSourceSet.singleton NullOrEmpty
    else
      List.fold list ~init:DataSourceSet.empty ~f:(fun cur_src_set e ->
          DataSourceSet.union cur_src_set (get_data_srcs_from_expr env ctx e))
  in
  (* For a collection (Darray, Shape and KeyValCollection) that uses a list of expression
     pairs (e.g. dict[0 => $a, 1 => $b] uses the list [(0, $a); (1, $b)]), the function
     get_data_srcs_of_pair_expr_list gets the data source for the 2nd expression in each
     pair and do the union; specially, return NullOrEmpty if the list is empty (e.g. dict[]). *)
  let get_data_srcs_of_pair_expr_list list =
    if List.is_empty list then
      DataSourceSet.singleton NullOrEmpty
    else
      List.fold list ~init:DataSourceSet.empty ~f:(fun cur_src_set (_, e) ->
          DataSourceSet.union cur_src_set (get_data_srcs_from_expr env ctx e))
  in
  match te with
  | Class_get (class_id, expr, Is_prop) ->
    (* Static variables annotated with <<__SafeForGlobalAccessCheck>> are
       treated as non-sensitive. *)
    let class_elts = get_static_prop_elts env class_id expr in
    if List.exists class_elts ~f:Typing_defs.get_ce_safe_global_variable then
      DataSourceSet.singleton NonSensitive
    else
      let expr_str = print_global_expr env te in
      if is_immutable_ty then
        DataSourceSet.singleton (GlobalValue expr_str)
      else
        DataSourceSet.singleton (GlobalReference expr_str)
  | Lvar (_, id) ->
    if Hashtbl.mem !(ctx.var_data_src_tbl) (Local_id.to_string id) then
      Hashtbl.find !(ctx.var_data_src_tbl) (Local_id.to_string id)
    else
      DataSourceSet.singleton Unknown
  | Call { func = (_, _, func_expr) as caller; args; _ } ->
    (match check_func_is_memoized caller env with
    | Some func_name ->
      (* If the called function is memoized, then its return is global. *)
      if is_immutable_ty then
        DataSourceSet.singleton (GlobalValue func_name)
      else
        DataSourceSet.singleton (GlobalReference func_name)
    | None ->
      (* Otherwise, the function call is treated as a black boxes, except
         for some special function (i.e. idx), the data source of its return
         is assumed to be the same as its first parameter. *)
      (match func_expr with
      | Id (_, func_id) when SSet.mem func_id src_from_first_para_func_ids ->
        (match args with
        | (_, para_expr) :: _ -> get_data_srcs_from_expr env ctx para_expr
        | [] -> DataSourceSet.singleton Unknown)
      | Id (_, func_id) ->
        if SSet.mem func_id safe_func_ids then
          DataSourceSet.singleton NonSensitive
        else if SSet.mem func_id src_from_para_func_ids then
          List.fold
            args
            ~init:DataSourceSet.empty
            ~f:(fun cur_src_set (_, para_expr) ->
              DataSourceSet.union
                cur_src_set
                (get_data_srcs_from_expr env ctx para_expr))
        else
          DataSourceSet.singleton Unknown
      | Class_const ((_, _, CI (_, class_name)), _) ->
        (* A static method call is safe if the class name is in "safe_class_ids",
           or it starts with "SV_" (i.e. it's a site var). *)
        let is_class_safe =
          SSet.mem class_name safe_class_ids
          || Stdlib.String.starts_with ~prefix:"\\SV_" class_name
        in
        if is_class_safe then
          DataSourceSet.singleton NonSensitive
        else
          DataSourceSet.singleton Unknown
      | _ -> DataSourceSet.singleton Unknown))
  | Darray (_, tpl) -> get_data_srcs_of_pair_expr_list tpl
  | Varray (_, el) -> get_data_srcs_of_expr_list el
  | Shape tpl -> get_data_srcs_of_pair_expr_list tpl
  | ValCollection (_, _, el) -> get_data_srcs_of_expr_list el
  | KeyValCollection (_, _, fl) -> get_data_srcs_of_pair_expr_list fl
  | Null -> DataSourceSet.singleton NullOrEmpty
  | Clone e -> get_data_srcs_from_expr env ctx e
  | Array_get (e, _) -> get_data_srcs_from_expr env ctx e
  | Obj_get (obj_expr, _, _, Is_prop) ->
    let obj_data_srcs = get_data_srcs_from_expr env ctx obj_expr in
    if is_immutable_ty then
      convert_ref_to_val obj_data_srcs
    else
      obj_data_srcs
  | Class_const _ -> DataSourceSet.singleton NonSensitive
  | Await e -> get_data_srcs_from_expr env ctx e
  | ReadonlyExpr e -> get_data_srcs_from_expr env ctx e
  | Tuple el
  | List el ->
    get_data_srcs_of_expr_list el
  | Cast (_, e) -> get_data_srcs_from_expr env ctx e
  | Unop (_, e) -> get_data_srcs_from_expr env ctx e
  | Binop { lhs; rhs; _ } ->
    DataSourceSet.union
      (get_data_srcs_from_expr env ctx lhs)
      (get_data_srcs_from_expr env ctx rhs)
  | Pipe (_, _, e) -> get_data_srcs_from_expr env ctx e
  | Eif (_, e1, e2) ->
    DataSourceSet.union
      (match e1 with
      | Some e -> get_data_srcs_from_expr env ctx e
      | None -> DataSourceSet.empty)
      (get_data_srcs_from_expr env ctx e2)
  | As { expr = e; _ } -> get_data_srcs_from_expr env ctx e
  | Upcast (e, _) -> get_data_srcs_from_expr env ctx e
  | Pair (_, e1, e2) ->
    DataSourceSet.union
      (get_data_srcs_from_expr env ctx e1)
      (get_data_srcs_from_expr env ctx e2)
  | True
  | False
  | Int _
  | Float _
  | String _
  | String2 _
  | PrefixedString _
  | Nameof _
  | ExpressionTree _ ->
    DataSourceSet.singleton Literal
  | This
  | Omitted
  | Id _
  | Dollardollar _
  | Obj_get (_, _, _, Is_method)
  | Class_get (_, _, Is_method)
  | FunctionPointer _
  | Yield _
  | Is _
  | New _
  | Efun _
  | Lfun _
  | Xml _
  | Import _
  | Collection _
  | Lplaceholder _
  | Method_caller _
  | ET_Splice _
  | EnumClassLabel _
  | Hole _
  | Invalid _
  | Package _ ->
    DataSourceSet.singleton Unknown

(* Get the global variable names from the given expression's data sources.
   By default, include_immutable is true, and we return both GlobalReference
   and GlobalValue; otherwise, we return only GlobalReference. If no such
   global variable exists, None is returned. *)
let get_global_vars_from_expr ?(include_immutable = true) env ctx expr =
  let is_src_global src =
    match src with
    | GlobalReference _ -> true
    | GlobalValue _ -> include_immutable
    | _ -> false
  in
  let print_src src =
    match src with
    | GlobalReference str
    | GlobalValue str ->
      str
    | _ as s -> show_data_source s
  in
  let data_srcs = get_data_srcs_from_expr env ctx expr in
  let global_srcs = DataSourceSet.filter is_src_global data_srcs in
  if DataSourceSet.is_empty global_srcs then
    None
  else
    Some
      (DataSourceSet.fold
         (fun src s -> SSet.add (print_src src) s)
         global_srcs
         SSet.empty)

(* Given an expression that appears on LHS of an assignment,
   this method gets the set of variables whose value may be assigned. *)
let rec get_vars_in_expr vars (_, _, te) =
  match te with
  | Lvar (_, id) -> vars := SSet.add (Local_id.to_string id) !vars
  | Obj_get (e, _, _, Is_prop) -> get_vars_in_expr vars e
  | Array_get (e, _) -> get_vars_in_expr vars e
  | ReadonlyExpr e -> get_vars_in_expr vars e
  | List el -> List.iter el ~f:(get_vars_in_expr vars)
  | _ -> ()

(* Suppose te is on LHS of an assignment, check if we can write to global variables
   by accessing either directly static variables or an object's properties. *)
let rec has_global_write_access (_, _, te) =
  match te with
  | Class_get (_, _, Is_prop)
  | Obj_get (_, _, _, Is_prop) ->
    true
  | List el -> List.exists el ~f:has_global_write_access
  | Lvar _
  | ReadonlyExpr _
  | Array_get _
  | _ ->
    false

let visitor =
  object (self)
    inherit [_] Tast_visitor.iter_with_state as super

    method! on_method_ (env, (ctx, fun_name)) m =
      Hashtbl.clear !(ctx.var_data_src_tbl);
      ctx.null_global_var_set := SSet.empty;
      super#on_method_ (env, (ctx, fun_name)) m

    method! on_fun_def (env, (ctx, fun_name)) f =
      Hashtbl.clear !(ctx.var_data_src_tbl);
      ctx.null_global_var_set := SSet.empty;
      super#on_fun_def (env, (ctx, fun_name)) f

    method! on_fun_ (env, (ctx, fun_name)) f =
      let var_data_src_tbl_cpy = Hashtbl.copy !(ctx.var_data_src_tbl) in
      let null_global_var_set_cpy = !(ctx.null_global_var_set) in
      super#on_fun_ (env, (ctx, fun_name)) f;
      ctx.var_data_src_tbl := var_data_src_tbl_cpy;
      ctx.null_global_var_set := null_global_var_set_cpy

    method! on_stmt_ (env, (ctx, fun_name)) s =
      match s with
      | If (((_, _, cond) as c), b1, b2) ->
        (* Make a copy of null_global_var_set, and reset afterwards. *)
        let null_global_var_set_cpy = !(ctx.null_global_var_set) in
        let ctx_else_branch =
          {
            var_data_src_tbl = ref (Hashtbl.copy !(ctx.var_data_src_tbl));
            null_global_var_set = ref !(ctx.null_global_var_set);
          }
        in
        (* From the condition, we get the expression whose value is null in one brach
           (true represents the if branch, while false represents the else branch). *)
        let nullable_expr_in_cond_opt =
          match cond with
          (* For the condition of format "expr is null", "expr === null" or "expr == null",
             return expr and true (i.e. if branch). *)
          | Is (cond_expr, (_, Hprim Tnull))
          | Binop
              {
                bop = Ast_defs.(Eqeqeq | Eqeq);
                lhs = cond_expr;
                rhs = (_, _, Null);
              } ->
            Some (cond_expr, true)
          (* For the condition of format "!C\contains_key(expr, $key)" where expr shall be a
             dictionary, return expr and true (i.e. if branch). *)
          | Unop
              ( Ast_defs.Unot,
                (_, _, Call { func = (_, _, Id (_, func_id)); args; _ }) )
            when SSet.mem func_id check_non_null_func_ids ->
            (match args with
            | [] -> None
            | (_, para_expr) :: _ -> Some (para_expr, true))
          (* For the condition of format "expr is nonnull", "expr !== null" or "expr != null",
             return expr and false (i.e. else branch). *)
          | Is (cond_expr, (_, Hnonnull))
          | Binop
              {
                bop = Ast_defs.(Diff | Diff2);
                lhs = cond_expr;
                rhs = (_, _, Null);
              } ->
            Some (cond_expr, false)
          (* For the condition of format "C\contains_key(expr, $key)" where expr shall be a
             dictionary, return expr and false (i.e. else branch). *)
          | Call { func = (_, _, Id (_, func_id)); args; _ }
            when SSet.mem func_id check_non_null_func_ids ->
            (match args with
            | [] -> None
            | (_, para_expr) :: _ -> Some (para_expr, false))
          | _ -> None
        in
        let () =
          match nullable_expr_in_cond_opt with
          | None -> ()
          | Some (nullable_expr, if_branch) ->
            let nullable_global_vars_opt =
              get_global_vars_from_expr env ctx nullable_expr
            in
            (* Add nullable global variables to null_global_var_set of the right branch. *)
            (match (nullable_global_vars_opt, if_branch) with
            | (None, _) -> ()
            | (Some vars, true) ->
              ctx.null_global_var_set :=
                SSet.union !(ctx.null_global_var_set) vars
            | (Some vars, false) ->
              ctx_else_branch.null_global_var_set :=
                SSet.union !(ctx_else_branch.null_global_var_set) vars)
        in
        (* Check the condition expression and report global reads/writes if any.
           For example, a global read is reported if a global varialbe is directly used;
           a global write is reported if a global variable is passed to a function call
           (which is very likely to happen) or it is directly mutated inside the condition
           (which is uncommon but possible). *)
        super#on_expr (env, (ctx, fun_name)) c;
        (* Check two branches separately, then merge the table of global variables
           and reset the set of nullable variables. *)
        super#on_block (env, (ctx, fun_name)) b1;
        super#on_block (env, (ctx_else_branch, fun_name)) b2;
        merge_var_data_srcs_tbls
          !(ctx.var_data_src_tbl)
          !(ctx_else_branch.var_data_src_tbl);
        ctx.null_global_var_set := null_global_var_set_cpy
      | Do (b, _)
      | While (_, b)
      | For (_, _, _, b)
      | Foreach (_, _, b) ->
        super#on_stmt_ (env, (ctx, fun_name)) s;
        (* Iterate the block and update the set of global varialbes until
           no new global variable is found *)
        let ctx_cpy =
          {
            var_data_src_tbl = ref (Hashtbl.copy !(ctx.var_data_src_tbl));
            null_global_var_set = ref !(ctx.null_global_var_set);
          }
        in
        let ctx_len = ref (get_tbl_total_cardinal !(ctx.var_data_src_tbl)) in
        let has_context_change = ref true in
        (* Continue the loop until no more data sources are found. *)
        while !has_context_change do
          super#on_block (env, (ctx_cpy, fun_name)) b;
          merge_var_data_srcs_tbls
            !(ctx.var_data_src_tbl)
            !(ctx_cpy.var_data_src_tbl);
          let new_ctx_tbl_cardinal =
            get_tbl_total_cardinal !(ctx.var_data_src_tbl)
          in
          if new_ctx_tbl_cardinal <> !ctx_len then
            ctx_len := new_ctx_tbl_cardinal
          else
            has_context_change := false
        done
      | Return r ->
        (match r with
        | Some ((ty, p, _) as e) ->
          (match
             get_global_vars_from_expr ~include_immutable:false env ctx e
           with
          | Some global_set ->
            raise_global_access_error
              p
              fun_name
              (Tast_env.print_ty env ty)
              global_set
              (GlobalAccessPatternSet.singleton NoPattern)
              GlobalAccessCheck.PossibleGlobalWriteViaFunctionCall
          | None -> ())
        | None -> ());
        super#on_stmt_ (env, (ctx, fun_name)) s
      | _ -> super#on_stmt_ (env, (ctx, fun_name)) s

    method! on_expr (env, (ctx, fun_name)) ((ty, p, e) as te) =
      match e with
      (* For the case where the expression is directly a static variable or a memoized
         function, and we report a global read. Notice that if the expression is on
         the LHS of an assignment, we will not run on_expr on it, hence no global
         reads would be reported; similarly, for the unary operation like "Foo::$prop++",
         we don't report a global read. *)
      | Class_get (class_id, expr, Is_prop) ->
        (* Ignore static variables annotated with <<__SafeForGlobalAccessCheck>> *)
        let class_elts = get_static_prop_elts env class_id expr in
        let is_annotated_safe =
          List.exists class_elts ~f:Typing_defs.get_ce_safe_global_variable
        in
        if not is_annotated_safe then
          let expr_str = print_global_expr env e in
          let ty_str = Tast_env.print_ty env ty in
          raise_global_access_error
            p
            fun_name
            ty_str
            (SSet.singleton expr_str)
            (GlobalAccessPatternSet.singleton NoPattern)
            GlobalAccessCheck.DefiniteGlobalRead
      | Call { func = (func_ty, _, _) as func_expr; args; _ } ->
        (* First check if the called functions is from super globals. *)
        let is_super_global_call = check_super_global_method e env fun_name in
        (if not is_super_global_call then
          (* If not super global, then check if the function is memoized. *)
          let func_ty_str = Tast_env.print_ty env func_ty in
          match check_func_is_memoized func_expr env with
          | Some memoized_func_name ->
            raise_global_access_error
              p
              fun_name
              func_ty_str
              (SSet.singleton memoized_func_name)
              (GlobalAccessPatternSet.singleton NoPattern)
              GlobalAccessCheck.DefiniteGlobalRead
          | None -> ());
        (* Lastly, check if a global variable is used as the parameter. *)
        List.iter args ~f:(fun (pk, ((ty, pos, _) as expr)) ->
            let e_global_opt =
              match pk with
              | Ast_defs.Pinout _ -> get_global_vars_from_expr env ctx expr
              | Ast_defs.Pnormal ->
                get_global_vars_from_expr ~include_immutable:false env ctx expr
            in
            if Option.is_some e_global_opt then
              raise_global_access_error
                pos
                fun_name
                (Tast_env.print_ty env ty)
                (Option.get e_global_opt)
                (GlobalAccessPatternSet.singleton NoPattern)
                GlobalAccessCheck.PossibleGlobalWriteViaFunctionCall);
        super#on_expr (env, (ctx, fun_name)) te
      | Binop { bop = Ast_defs.Eq bop_opt; lhs; rhs } ->
        let () = self#on_expr (env, (ctx, fun_name)) rhs in
        let re_ty = Tast_env.print_ty env (Tast.get_type rhs) in
        let le_global_opt = get_global_vars_from_expr env ctx lhs in
        (* When write to a global variable whose value is null or does not exist:
           if the written variable is in a collection (e.g. $global[$key] = $val),
           then it's asumed to be caching; otherwise, it's a singleton. *)
        let singleton_or_caching =
          match lhs with
          | (_, _, Array_get _) -> Caching
          | _ -> Singleton
        in
        let le_pattern =
          match (le_global_opt, bop_opt) with
          | (None, _) -> NoPattern
          | (Some _, Some Ast_defs.QuestionQuestion) -> singleton_or_caching
          | (Some le_global, _) ->
            if
              SSet.exists
                (fun v -> SSet.mem v !(ctx.null_global_var_set))
                le_global
            then
              singleton_or_caching
            else
              NoPattern
        in
        let re_data_srcs = get_data_srcs_from_expr env ctx rhs in
        let re_patterns = get_patterns_from_written_data_srcs re_data_srcs in
        (* Distinguish directly writing to static variables from writing to a variable
           that has references to static variables. *)
        (if is_expr_static env lhs && Option.is_some le_global_opt then
          raise_global_access_error
            p
            fun_name
            re_ty
            (Option.get le_global_opt)
            (GlobalAccessPatternSet.add le_pattern re_patterns)
            GlobalAccessCheck.DefiniteGlobalWrite
        else
          let vars_in_le = ref SSet.empty in
          let () = get_vars_in_expr vars_in_le lhs in
          if has_global_write_access lhs then (
            if Option.is_some le_global_opt then
              raise_global_access_error
                p
                fun_name
                re_ty
                (Option.get le_global_opt)
                (GlobalAccessPatternSet.add le_pattern re_patterns)
                GlobalAccessCheck.PossibleGlobalWriteViaReference;
            SSet.iter
              (fun v ->
                add_var_data_srcs_to_tbl !(ctx.var_data_src_tbl) v re_data_srcs)
              !vars_in_le
          ) else
            SSet.iter
              (fun v ->
                replace_var_data_srcs_in_tbl
                  !(ctx.var_data_src_tbl)
                  v
                  re_data_srcs)
              !vars_in_le);
        super#on_expr (env, (ctx, fun_name)) rhs
        (* add_var_refs_to_tbl !(ctx.global_var_refs_tbl) !vars_in_le *)
      | Unop (op, e) ->
        let e_global_opt = get_global_vars_from_expr env ctx e in
        if Option.is_some e_global_opt then
          let e_global = Option.get e_global_opt in
          let e_ty = Tast_env.print_ty env (Tast.get_type e) in
          (match op with
          | Ast_defs.Uincr
          | Ast_defs.Udecr
          | Ast_defs.Upincr
          | Ast_defs.Updecr ->
            (* Distinguish directly writing to static variables from writing to a variable that has references to static variables. *)
            if is_expr_static env e then
              raise_global_access_error
                p
                fun_name
                e_ty
                e_global
                (GlobalAccessPatternSet.singleton CounterIncrement)
                GlobalAccessCheck.DefiniteGlobalWrite
            else if has_global_write_access e then
              raise_global_access_error
                p
                fun_name
                e_ty
                e_global
                (GlobalAccessPatternSet.singleton CounterIncrement)
                GlobalAccessCheck.PossibleGlobalWriteViaReference
          | _ -> super#on_expr (env, (ctx, fun_name)) e)
      | New (_, _, el, _, _) ->
        List.iter el ~f:(fun ((ty, pos, _) as expr) ->
            let e_global_opt =
              get_global_vars_from_expr ~include_immutable:false env ctx expr
            in
            if Option.is_some e_global_opt then
              raise_global_access_error
                pos
                fun_name
                (Tast_env.print_ty env ty)
                (Option.get e_global_opt)
                (GlobalAccessPatternSet.singleton NoPattern)
                GlobalAccessCheck.PossibleGlobalWriteViaFunctionCall);
        super#on_expr (env, (ctx, fun_name)) te
      | _ -> super#on_expr (env, (ctx, fun_name)) te
  end

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_method_ env m =
      let class_name = Tast_env.get_self_id env in
      let (_, method_name) = m.m_name in
      let full_name =
        match class_name with
        | Some s -> s ^ "::" ^ method_name
        | _ -> method_name
      in
      (* Class name starts with '\' or ';' *)
      let full_name =
        String.sub full_name ~pos:1 ~len:(String.length full_name - 1)
      in
      visitor#on_method_ (env, (current_ctx, full_name)) m

    method! at_fun_def env f =
      let (_, function_name) = f.fd_name in
      (* Function name starts with '\'*)
      let function_name =
        String.sub function_name ~pos:1 ~len:(String.length function_name - 1)
      in
      visitor#on_fun_def (env, (current_ctx, function_name)) f
  end
