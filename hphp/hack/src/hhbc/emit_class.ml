(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Instruction_sequence
module H = Hhbc_ast
module SU = Hhbc_string_utils
module SN = Naming_special_names
module TV = Typed_value
module A = Aast

let hack_arr_dv_arrs () =
  Hhbc_options.hack_arr_dv_arrs !Hhbc_options.compiler_options

let add_symbol_refs class_base class_implements class_uses class_requirements =
  let add_from_ast_name c =
    Hhbc_id.Class.from_ast_name c |> Emit_symbol_refs.add_class
  in
  Option.iter class_base ~f:Emit_symbol_refs.add_class;
  List.iter class_implements ~f:Emit_symbol_refs.add_class;
  List.iter class_uses ~f:add_from_ast_name;
  List.iter class_requirements ~f:(function (_, c) -> add_from_ast_name c)

let make_86method ~name ~params ~is_static ~visibility ~is_abstract ~span instrs
    =
  let method_attributes = [] in
  (* TODO: move this. We just know that there are no iterators in 86methods *)
  Iterator.reset_iterator ();
  let method_is_final = false in
  let method_visibility = visibility in
  let method_return_type = None in
  let method_decl_vars = [] in
  let method_is_async = false in
  let method_is_generator = false in
  let method_is_pair_generator = false in
  let method_is_closure_body = false in
  let method_is_memoize_wrapper = false in
  let method_is_memoize_wrapper_lsb = false in
  let method_no_injection = true in
  let method_doc_comment = None in
  let method_is_interceptable = false in
  let method_is_memoize_impl = false in
  let method_env = None in
  let method_body =
    Emit_body.make_body
      instrs
      method_decl_vars
      method_is_memoize_wrapper
      method_is_memoize_wrapper_lsb
      []
      []
      params
      method_return_type
      method_doc_comment
      method_env
  in
  Hhas_method.make
    method_attributes
    method_visibility
    is_static
    method_is_final
    is_abstract
    method_no_injection
    (Hhbc_id.Method.from_ast_name name)
    method_body
    span
    method_is_async
    method_is_generator
    method_is_pair_generator
    method_is_closure_body
    method_is_interceptable
    method_is_memoize_impl
    Rx.NonRx
    (* method_rx_disabled *)
    false

let from_extends ~is_enum extends =
  if is_enum then
    Some (Hhbc_id.Class.from_raw_string "HH\\BuiltinEnum")
  else
    match extends with
    | [] -> None
    | h :: _ -> Some (Emit_type_hint.hint_to_class h)

let from_implements implements =
  List.map implements Emit_type_hint.hint_to_class

let from_type_constant tc =
  let type_constant_name = snd tc.A.c_tconst_name in
  match (tc.A.c_tconst_abstract, tc.A.c_tconst_type) with
  | (A.TCAbstract None, _)
  | ((A.TCPartiallyAbstract | A.TCConcrete), None) ->
    Hhas_type_constant.make type_constant_name None
  | (A.TCAbstract (Some init), _)
  | ((A.TCPartiallyAbstract | A.TCConcrete), Some init) ->
    (* TODO: Deal with the constraint *)
    let type_constant_initializer =
      (* Type constants do not take type vars hence tparams:[] *)
      Some
        (Emit_type_constant.hint_to_type_constant
           ~tparams:[]
           ~targ_map:SMap.empty
           init)
    in
    Hhas_type_constant.make type_constant_name type_constant_initializer

let from_class_elt_classvars ast_class class_is_const tparams =
  (* TODO: we need to emit doc comments for each property,
   * not one per all properties on the same line *)
  (* The doc comment is only for the first name in the list.
   * Currently this is organized in the ast_to_nast module*)
  let mapping_aux cv =
    let hint =
      if cv.A.cv_is_promoted_variadic then
        None
      else
        A.hint_of_type_hint cv.A.cv_type
    in
    Emit_property.from_ast
      ast_class
      cv.A.cv_user_attributes
      cv.A.cv_abstract
      cv.A.cv_is_static
      cv.A.cv_visibility (* This used to be cv_kinds *)
      class_is_const
      hint
      tparams
      ast_class.A.c_namespace
      cv.A.cv_doc_comment (* Doc comments are weird. T40098274 *)
      ((), cv.A.cv_id, cv.A.cv_expr)
  in
  List.map ~f:mapping_aux ast_class.A.c_vars

let from_class_elt_constants env class_ =
  let map_aux (c : Tast.class_const) =
    Hhas_constant.from_ast env c.A.cc_id c.A.cc_expr
  in
  List.map ~f:map_aux class_.A.c_consts

let from_class_elt_requirements class_ =
  List.map
    ~f:(fun (h, is_extends) ->
      let kind =
        if is_extends then
          Hhas_class.MustExtend
        else
          Hhas_class.MustImplement
      in
      (kind, Hhbc_id.Class.to_raw_string (Emit_type_hint.hint_to_class h)))
    class_.A.c_reqs

let from_class_elt_typeconsts class_ =
  List.map ~f:from_type_constant class_.A.c_typeconsts

let from_enum_type opt =
  match opt with
  | Some e ->
    let type_info_user_type =
      Some (Emit_type_hint.fmt_hint ~tparams:[] ~strip_tparams:true e.A.e_base)
    in
    let type_info_type_constraint =
      Hhas_type_constraint.make None [Hhas_type_constraint.ExtendedHint]
    in
    Some (Hhas_type_info.make type_info_user_type type_info_type_constraint)
  | _ -> None

let is_hh_namespace ns =
  Option.value_map ns.Namespace_env.ns_name ~default:false ~f:(fun v ->
      String.lowercase v = "hh")

let is_global_namespace ns = Option.is_none ns.Namespace_env.ns_name

let validate_class_name ns (p, class_name) =
  (* per Parser::checkClassDeclName:
     global names are always reserved in any namespace.
     hh_reserved names are checked either if
     - class is in global namespace
     - class is in HH namespace *)
  let is_special_class = String_utils.is_substring "$" class_name in
  let check_hh_name = is_global_namespace ns || is_hh_namespace ns in
  let name = SU.strip_ns class_name in
  let lower_name = String.lowercase name in
  let is_reserved_global_name =
    SN.Typehints.is_reserved_global_name lower_name
  in
  let name_is_reserved =
    (not is_special_class)
    && ( is_reserved_global_name
       || (check_hh_name && SN.Typehints.is_reserved_hh_name lower_name) )
  in
  if name_is_reserved then
    let message =
      Printf.sprintf
        "Cannot use '%s' as class name as it is reserved"
        ( if is_reserved_global_name then
          name
        else
          Utils.strip_ns class_name )
    in
    Emit_fatal.raise_fatal_parse p message

let emit_reified_extends_params env class_ =
  let type_params =
    match class_.A.c_extends with
    | (_, Aast.Happly (_, l)) :: _ -> l
    | _ -> []
  in
  if List.is_empty type_params then
    let tv =
      if hack_arr_dv_arrs () then
        TV.Vec ([], None)
      else
        TV.VArray ([], None)
    in
    instr (H.ILitConst (H.TypedValue tv))
  else
    gather
      [
        Emit_expression.emit_reified_targs env class_.A.c_span type_params;
        instr_record_reified_generic;
      ]

let emit_reified_init_body env num_reified class_ =
  let check_length =
    gather
      [
        instr_cgetl (Local.Named SU.Reified.reified_init_method_param_name);
        instr_check_reified_generic_mismatch;
      ]
  in
  let set_prop =
    if num_reified = 0 then
      empty
    else
      (* $this->86reified_prop = $__typestructures *)
      gather
        [
          check_length;
          instr_checkthis;
          instr_cgetl (Local.Named SU.Reified.reified_init_method_param_name);
          instr_baseh;
          instr_setm_pt
            0
            (Hhbc_id.Prop.from_raw_string SU.Reified.reified_prop_name);
          instr_popc;
        ]
  in
  let return = gather [instr_null; instr_retc] in
  if class_.A.c_extends = [] then
    gather [set_prop; return]
  else
    let generic_arr = emit_reified_extends_params env class_ in
    (* parent::86reifiedinit($generic_arr) *)
    let call_parent =
      gather
        [
          instr_nulluninit;
          instr_nulluninit;
          instr_nulluninit;
          generic_arr;
          instr_fcallclsmethodsd
            (make_fcall_args 1)
            Hhbc_ast.SpecialClsRef.Parent
            (Hhbc_id.Method.from_raw_string SU.Reified.reified_init_method_name);
          instr_popc;
        ]
    in
    gather [set_prop; call_parent; return]

let emit_reified_init_method env ast_class =
  let num_reified =
    List.count ast_class.A.c_tparams.A.c_tparam_list ~f:(fun t ->
        not (t.A.tp_reified = A.Erased))
  in
  let maybe_has_reified_parents =
    match ast_class.A.c_extends with
    | (_, Aast.Happly (_, l)) :: _ -> not @@ List.is_empty l
    | _ -> (* Hack classes can only extend a single parent *) false
  in
  if num_reified = 0 && not maybe_has_reified_parents then
    []
  else
    let tc = Hhas_type_constraint.make (Some "HH\\varray") [] in
    let params =
      [
        Hhas_param.make
          SU.Reified.reified_init_method_param_name
          false (* variadic *)
          false (* inout *)
          [] (* uattrs *)
          (Some (Hhas_type_info.make (Some "HH\\varray") tc))
          None;
        (* default value *)
      ]
    in
    let instrs = emit_reified_init_body env num_reified ast_class in
    [
      make_86method
        ~name:SU.Reified.reified_init_method_name
        ~params
        ~is_static:false
        ~visibility:Aast.Protected
        ~is_abstract:false
        ~span:(Hhas_pos.pos_to_span ast_class.A.c_span)
        instrs;
    ]

let emit_class (ast_class, hoisted) =
  let namespace = ast_class.A.c_namespace in
  validate_class_name namespace ast_class.A.c_name;
  let env = Emit_env.make_class_env ast_class in
  (* TODO: communicate this without looking at the name *)
  let is_closure_class =
    String.is_prefix ~prefix:"Closure$" (snd ast_class.A.c_name)
  in
  let class_attributes =
    Emit_attribute.from_asts namespace ast_class.A.c_user_attributes
  in
  let class_attributes =
    if is_closure_class then
      class_attributes
    else
      Emit_attribute.add_reified_attribute
        class_attributes
        ast_class.A.c_tparams.A.c_tparam_list
  in
  let class_attributes =
    if is_closure_class then
      class_attributes
    else
      Emit_attribute.add_reified_parent_attribute
        env
        class_attributes
        ast_class.A.c_extends
  in
  let class_is_const = Hhas_attribute.has_const class_attributes in
  (* In the future, we intend to set class_no_dynamic_props independently from
   * class_is_const, but for now class_is_const is the only thing that turns
   * it on. *)
  let class_no_dynamic_props = class_is_const in
  let class_id = Hhbc_id.Class.from_ast_name (snd ast_class.A.c_name) in
  let class_is_trait = ast_class.A.c_kind = Ast_defs.Ctrait in
  let class_is_interface = ast_class.A.c_kind = Ast_defs.Cinterface in
  let class_uses =
    List.filter_map ast_class.A.c_uses (fun (p, h) ->
        match h with
        | Aast.Happly ((_, name), _) ->
          if class_is_interface then
            Emit_fatal.raise_fatal_parse p "Interfaces cannot use traits"
          else
            Some name
        | _ -> None)
  in
  let elaborate_namespace_id (_, name) =
    Hhbc_id.Class.(from_ast_name name |> to_raw_string)
  in
  let class_use_aliases =
    List.map
      ~f:(fun (ido1, id, ido2, vis) ->
        let id1 = Option.map ido1 ~f:elaborate_namespace_id in
        let id2 = Option.map ido2 ~f:snd in
        (id1, snd id, id2, vis))
      ast_class.A.c_use_as_alias
  in
  let class_use_precedences =
    List.map
      ~f:(fun (id1, id2, ids) ->
        let id1 = elaborate_namespace_id id1 in
        let id2 = snd id2 in
        let ids = List.map ids ~f:elaborate_namespace_id in
        (id1, id2, ids))
      ast_class.A.c_insteadof_alias
  in
  let class_method_trait_resolutions =
    let string_of_trait trait =
      match snd trait with
      (* TODO: Currently, names are not elaborated.
       * Names should be elaborated if this feature is to be supported
       * T56629465
       *)
      | Aast.Happly ((_, trait), _) -> trait
      (* Happly converted from naming *)
      | Aast.Hprim p -> Emit_type_hint.prim_to_string p
      | Aast.Hany
      | Aast.Herr ->
        failwith "I'm convinced that this should be an error caught in naming"
      | Aast.Hmixed -> SN.Typehints.mixed
      | Aast.Hnonnull -> SN.Typehints.nonnull
      | Aast.Habstr s -> s
      | Aast.Harray _ -> SN.Typehints.array
      | Aast.Hdarray _ -> SN.Typehints.darray
      | Aast.Hvarray _ -> SN.Typehints.varray
      | Aast.Hvarray_or_darray _ -> SN.Typehints.varray_or_darray
      | Aast.Hthis -> SN.Typehints.this
      | Aast.Hdynamic -> SN.Typehints.dynamic
      | _ -> failwith "TODO Fail gracefully here"
    in
    List.map ast_class.A.c_method_redeclarations ~f:(fun mtr ->
        (mtr, string_of_trait mtr.A.mt_trait))
  in
  let class_enum_type =
    if ast_class.A.c_kind = Ast_defs.Cenum then
      from_enum_type ast_class.A.c_enum
    else
      None
  in
  let class_xhp_attributes =
    List.map
      ~f:(fun (ho, cv, b, eo) ->
        Hhas_xhp_attribute.make (A.hint_of_type_hint ho) cv b eo)
      ast_class.A.c_xhp_attrs
  in
  let class_xhp_children =
    match ast_class.A.c_xhp_children with
    | (p, sl) :: _ -> Some (p, [sl])
    | [] -> None
  in
  (* Find map instead of filter map. T40102763 *)
  let class_xhp_categories =
    match ast_class.A.c_xhp_category with
    | Some (p, c) -> Some (p, List.map c ~f:snd)
    | None -> None
  in
  let class_is_abstract = ast_class.A.c_kind = Ast_defs.Cabstract in
  let class_is_final =
    ast_class.A.c_final || class_is_trait || class_enum_type <> None
  in
  let class_is_sealed = Hhas_attribute.has_sealed class_attributes in
  let tparams =
    Emit_body.tparams_to_strings ast_class.A.c_tparams.A.c_tparam_list
  in
  let class_base =
    if class_is_interface then
      None
    else
      let base =
        from_extends ~is_enum:(class_enum_type <> None) ast_class.A.c_extends
      in
      match base with
      | Some cls
        when String.lowercase (Hhbc_id.Class.to_raw_string cls) = "closure"
             && not is_closure_class ->
        Emit_fatal.raise_fatal_runtime
          (fst ast_class.A.c_name)
          "Class cannot extend Closure"
      | _ -> base
  in
  let implements =
    if class_is_interface then
      ast_class.A.c_extends
    else
      ast_class.A.c_implements
  in
  let class_implements = from_implements implements in
  let class_span = Hhas_pos.pos_to_span ast_class.A.c_span in
  (* TODO: communicate this without looking at the name *)
  let additional_methods = [] in
  let additional_methods =
    match class_xhp_categories with
    | None -> additional_methods
    | Some cats ->
      additional_methods @ Emit_xhp.from_category_declaration ast_class cats
  in
  let additional_methods =
    match class_xhp_children with
    | None -> additional_methods
    | Some children ->
      additional_methods @ Emit_xhp.from_children_declaration ast_class children
  in
  let no_xhp_attributes =
    class_xhp_attributes = [] && ast_class.A.c_xhp_attr_uses = []
  in
  let additional_methods =
    if no_xhp_attributes then
      additional_methods
    else
      additional_methods
      @ Emit_xhp.from_attribute_declaration
          ast_class
          class_xhp_attributes
          ast_class.A.c_xhp_attr_uses
  in
  Label.reset_label ();
  let class_properties =
    from_class_elt_classvars ast_class class_is_const tparams
  in
  let env = Emit_env.make_class_env ast_class in
  let class_constants = from_class_elt_constants env ast_class in
  let class_requirements = from_class_elt_requirements ast_class in
  let make_init_methods filter ~name =
    if
      List.exists class_properties (fun p ->
          Option.is_some (Hhas_property.initializer_instrs p) && filter p)
    then
      let instrs =
        gather
        @@ List.filter_map class_properties (fun p ->
               if filter p then
                 Hhas_property.initializer_instrs p
               else
                 None)
      in
      let instrs = gather [instrs; instr_null; instr_retc] in
      [
        make_86method
          ~name
          ~params:[]
          ~is_static:true
          ~visibility:Aast.Private
          ~is_abstract:false
          ~span:class_span
          instrs;
      ]
    else
      []
  in
  let property_has_lsb p =
    Hhas_attribute.has_lsb (Hhas_property.attributes p)
  in
  let pinit_filter p = not (Hhas_property.is_static p) in
  let sinit_filter p = Hhas_property.is_static p && not (property_has_lsb p) in
  let linit_filter p = Hhas_property.is_static p && property_has_lsb p in
  let pinit_methods = make_init_methods pinit_filter ~name:"86pinit" in
  let sinit_methods = make_init_methods sinit_filter ~name:"86sinit" in
  let linit_methods = make_init_methods linit_filter ~name:"86linit" in
  let initialized_class_constants =
    List.filter_map class_constants (fun p ->
        match Hhas_constant.initializer_instrs p with
        | None -> None
        | Some instrs ->
          Some (Hhas_constant.name p, Label.next_regular (), instrs))
  in
  let cinit_methods =
    if List.is_empty initialized_class_constants then
      []
    else
      let return_label = Label.next_regular () in
      let rec make_cinit_instrs cs =
        match cs with
        | [] ->
          gather
            [
              instr_label return_label;
              Emit_pos.emit_pos ast_class.A.c_span;
              instr_retc;
            ]
        | (_, label, instrs) :: cs ->
          if List.is_empty cs then
            gather [instr_label label; instrs; make_cinit_instrs cs]
          else
            gather
              [
                instr_label label;
                instrs;
                Emit_pos.emit_pos ast_class.A.c_span;
                instr_jmp return_label;
                make_cinit_instrs cs;
              ]
      in
      let body_instrs =
        if List.length initialized_class_constants > 1 then
          let cases =
            List.map initialized_class_constants (fun (name, label, _) ->
                (name, label))
          in
          gather
            [
              instr_cgetl (Local.Named "$constName");
              instr_sswitch cases;
              make_cinit_instrs initialized_class_constants;
            ]
        else
          make_cinit_instrs initialized_class_constants
      in
      let instrs = Emit_pos.emit_pos_then ast_class.A.c_span body_instrs in
      let params = [Hhas_param.make "$constName" false false [] None None] in
      [
        make_86method
          ~name:"86cinit"
          ~params
          ~is_static:true
          ~visibility:Aast.Private
          ~is_abstract:class_is_interface
          ~span:class_span
          instrs;
      ]
  in
  let should_emit_reified_init =
    not
      ( Emit_env.is_systemlib ()
      || is_closure_class
      || class_is_interface
      || class_is_trait )
  in
  let reified_init_method =
    if not should_emit_reified_init then
      []
    else
      emit_reified_init_method env ast_class
  in
  let no_reifiedinit_needed =
    (not (List.is_empty reified_init_method))
    && List.is_empty ast_class.A.c_extends
  in
  let class_upper_bounds =
    if Hhbc_options.emit_generics_ub !Hhbc_options.compiler_options then
      Emit_body.emit_generics_upper_bounds
        ast_class.A.c_tparams.A.c_tparam_list
        []
        ~skipawaitable:false
    else
      []
  in
  let additional_methods =
    additional_methods
    @ reified_init_method
    @ pinit_methods
    @ sinit_methods
    @ linit_methods
    @ cinit_methods
  in
  let class_methods = Emit_method.from_asts ast_class ast_class.A.c_methods in
  let class_methods = class_methods @ additional_methods in
  let class_type_constants = from_class_elt_typeconsts ast_class in
  let info =
    Emit_memoize_method.make_info ast_class class_id ast_class.A.c_methods
  in
  let additional_properties =
    if no_xhp_attributes then
      []
    else
      Emit_xhp.properties_for_cache ~ns:namespace ast_class class_is_const
  in
  let additional_methods =
    Emit_memoize_method.emit_wrapper_methods
      env
      info
      ast_class
      ast_class.A.c_methods
  in
  add_symbol_refs class_base class_implements class_uses class_requirements;
  Hhas_class.make
    class_attributes
    class_base
    class_implements
    class_id
    class_span
    class_is_final
    class_is_sealed
    class_is_abstract
    class_is_interface
    class_is_trait
    (ast_class.A.c_is_xhp || ast_class.A.c_has_xhp_keyword)
    hoisted
    class_is_const
    class_no_dynamic_props
    no_reifiedinit_needed
    class_uses
    class_use_aliases (* Killing class_use_aliases T40098428 *)
    class_use_precedences (* Killing class_use_precedences as well T40098428 *)
    class_method_trait_resolutions
    class_enum_type
    (class_methods @ List.rev additional_methods)
    (class_properties @ additional_properties)
    class_constants
    class_type_constants
    class_requirements
    class_upper_bounds
    ast_class.A.c_doc_comment

let emit_classes_from_program
    (ast : (Closure_convert.hoist_kind * Tast.def) list) =
  let aux (is_top, d) =
    match d with
    | A.Class cd -> Some (emit_class (cd, is_top))
    | _ -> None
  in
  List.filter_map ~f:aux ast
