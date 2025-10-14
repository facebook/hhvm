(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
open Hh_prelude
open Src
open Hack
open Gencode
module Fact_acc = Predicate.Fact_acc

let is_async = function
  | Ast_defs.FAsync -> true
  | Ast_defs.FAsyncGenerator -> true
  | _ -> false

let namespace_decl name fa =
  let json =
    NamespaceDeclaration.(
      { name = Util.make_namespaceqname name } |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack NamespaceDeclaration) json fa

(* create a namespace fact from a namespace attribute in node. Name can be empty
   in that case we simply ignore it *)
let namespace_decl_opt name fa =
  match name.Namespace_env.ns_name with
  | None -> fa (* global name space *)
  | Some "" -> fa
  | Some name -> namespace_decl name fa |> snd

let container_decl_qname decl_pred qname fa =
  let json =
    match decl_pred with
    | Predicate.(Hack ClassDeclaration) ->
      ClassDeclaration.(to_json_key { name = qname })
    | Predicate.(Hack EnumDeclaration) ->
      EnumDeclaration.(to_json_key { name = qname })
    | Predicate.(Hack InterfaceDeclaration) ->
      InterfaceDeclaration.(to_json_key { name = qname })
    | Predicate.(Hack TraitDeclaration) ->
      TraitDeclaration.(to_json_key { name = qname })
    | _ -> failwith "Impossible has happened"
  in
  Fact_acc.add_fact decl_pred json fa

let parent_decls decls pred fa =
  List.fold decls ~init:([], fa) ~f:(fun (decl_refs, fa) decl ->
      match Util.class_hint_to_qname decl with
      | Some qname ->
        let (decl_id, fa) = container_decl_qname pred qname fa in
        (decl_id :: decl_refs, fa)
      | None -> (decl_refs, fa))

let parent_decls_enum decls fa =
  let (fact_ids, fa) = parent_decls decls Predicate.(Hack EnumDeclaration) fa in
  (List.map ~f:(fun x -> EnumDeclaration.Id x) fact_ids, fa)

let parent_decls_class decls fa =
  let (fact_ids, fa) =
    parent_decls decls Predicate.(Hack ClassDeclaration) fa
  in

  (List.map ~f:(fun x -> ClassDeclaration.Id x) fact_ids, fa)

let parent_decls_trait decls fa =
  let (fact_ids, fa) =
    parent_decls decls Predicate.(Hack TraitDeclaration) fa
  in
  (List.map ~f:(fun x -> TraitDeclaration.Id x) fact_ids, fa)

let parent_decls_interface decls fa =
  let (fact_ids, fa) =
    parent_decls decls Predicate.(Hack InterfaceDeclaration) fa
  in
  (List.map ~f:(fun x -> InterfaceDeclaration.Id x) fact_ids, fa)

let module_decl name fa =
  let json =
    ModuleDeclaration.({ name = Util.make_name name } |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack ModuleDeclaration) json fa

let module_field module_ internal fa =
  match module_ with
  | None -> (None, fa)
  | Some (_pos, module_name) ->
    let (decl_id, fa) = module_decl module_name fa in
    (Some (Build_fact.module_membership decl_id ~internal), fa)

let member_cluster ~members fa =
  let json = MemberCluster.({ members } |> to_json_key) in
  Fact_acc.add_fact Predicate.(Hack MemberCluster) json fa

let inherited_members ~container_type ~container_id ~member_clusters fa =
  let json =
    InheritedMembers.(
      {
        container = Predicate.container_decl container_type container_id;
        inherited_members =
          List.map ~f:(fun x -> MemberCluster.Id x) member_clusters;
      }
      |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack InheritedMembers) json fa

let property_decl con_type decl_id name fa =
  let json =
    PropertyDeclaration.(
      {
        name = Util.make_name name;
        container = Predicate.container_decl con_type decl_id;
      }
      |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack PropertyDeclaration) json fa

let class_const_decl con_type decl_id name fa =
  let json =
    ClassConstDeclaration.(
      {
        name = Util.make_name name;
        container = Predicate.container_decl con_type decl_id;
      }
      |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack ClassConstDeclaration) json fa

let type_const_decl con_type decl_id name fa =
  let json =
    TypeConstDeclaration.(
      {
        name = Util.make_name name;
        container = Predicate.container_decl con_type decl_id;
      }
      |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack TypeConstDeclaration) json fa

let method_decl con_type decl_id name fa =
  let json =
    MethodDeclaration.(
      {
        name = Util.make_name name;
        container = Predicate.container_decl con_type decl_id;
      }
      |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack MethodDeclaration) json fa

let hint_to_type_info (hint : Aast.hint) fa : Type.t * TypeInfo.t * Fact_acc.t =
  let aggregate_pos (json_pos_list : (XRefTarget.t * Pretty.pos) list) :
      (XRefTarget.t * Pretty.pos list) list =
    let jmap =
      List.fold json_pos_list ~init:Map.Poly.empty ~f:(fun acc (json, pos) ->
          let f = function
            | None -> [pos]
            | Some prev -> pos :: prev
          in
          Map.Poly.update acc json ~f)
    in
    Map.Poly.to_alist jmap
  in
  let type_info ~ty sym_pos hint fa =
    let json =
      TypeInfo.(
        {
          display_type = Type.Key (ty |> Utils.strip_ns);
          xrefs = Build_fact.hint_xrefs sym_pos;
          hint = Some hint;
        }
        |> to_json_key)
    in
    Fact_acc.add_fact Predicate.(Hack TypeInfo) json fa
  in
  let (ty, sym_pos) = Pretty.hint_to_string_and_symbols ~is_ctx:false hint in
  let hint = Pretty.hint_to_angle hint in
  let pos_map =
    match Fact_acc.get_pos_map fa with
    | Some pos_map -> pos_map
    | None -> failwith "Internal error: pos_map should be set in previous phase"
  in
  let decl_json_pos =
    List.filter_map sym_pos ~f:(fun (source_pos, pos) ->
        match Xrefs.PosMap.find_opt source_pos pos_map with
        | Some (Xrefs.{ target; _ } :: _) -> Some (target, pos)
        | _ -> None)
  in
  let decl_json_aggr_pos = aggregate_pos decl_json_pos in
  let (fact_id, fa) = type_info ~ty decl_json_aggr_pos hint fa in
  (Type.Key ty, TypeInfo.Id fact_id, fa)

let hint_opt_to_type_info fa = function
  | None -> (None, None, fa)
  | Some type_ ->
    let (ty, ty_info, fa) = hint_to_type_info type_ fa in
    (Some ty, Some ty_info, fa)

let type_param source_text tp fa =
  let constraint_ (kind, hint) fa =
    let (type_, type_info, fa) = hint_to_type_info hint fa in
    let cst =
      Constraint.
        {
          constraint_kind = Util.make_constraint_kind kind;
          type_;
          type_info = Some type_info;
        }
    in
    (cst, fa)
  in
  let (_, name) = tp.tp_name in
  let name = Util.make_name name in
  let (constraints, fa) =
    List.fold tp.tp_constraints ~init:([], fa) ~f:(fun (fact_ids, fa) cst ->
        let (fact_id, fa) = constraint_ cst fa in
        (fact_id :: fact_ids, fa))
  in
  let tparam =
    TypeParameter.
      {
        name;
        variance = Util.make_variance tp.tp_variance;
        reify_kind = Util.make_reify_kind tp.tp_reified;
        constraints = List.rev constraints;
        attributes = Build_fact.attributes source_text tp.tp_user_attributes;
      }
  in
  (tparam, fa)

let type_params source_text params fa =
  let (type_params, fa) =
    List.fold params ~init:([], fa) ~f:(fun (fact_ids, fa) param ->
        let (fact_id, fa) = type_param source_text param fa in
        (fact_id :: fact_ids, fa))
  in
  (List.rev type_params, fa)

let container_defn source_text clss decl_id members fa =
  let fa = namespace_decl_opt clss.c_namespace fa in
  let (type_params, fa) = type_params source_text clss.c_tparams fa in
  let (module_, fa) = module_field clss.c_module clss.c_internal fa in
  let attributes = Build_fact.attributes source_text clss.c_user_attributes in
  (* T207902686: add Glean support for experimental require this as C constraints *)
  let (req_extends_hints, req_implements_hints, req_class_hints, _) =
    Aast.partition_map_require_kind ~f:(fun x -> x) clss.c_reqs
  in
  let (require_extends, fa) =
    parent_decls_class (List.map req_extends_hints ~f:fst) fa
  in
  let (require_implements, fa) =
    parent_decls_interface (List.map req_implements_hints ~f:fst) fa
  in
  let (require_class, fa) =
    parent_decls_class (List.map req_class_hints ~f:fst) fa
  in
  match Predicate.get_parent_kind clss.c_kind with
  | Predicate.InterfaceContainer ->
    let (extends_, fa) = parent_decls_interface clss.c_extends fa in
    let json =
      InterfaceDefinition.(
        {
          declaration = InterfaceDeclaration.Id decl_id;
          type_params;
          members;
          attributes;
          extends_;
          require_extends;
          module_;
        }
        |> to_json_key)
    in
    Fact_acc.add_fact Predicate.(Hack InterfaceDefinition) json fa
  | Predicate.TraitContainer ->
    let (implements_, fa) = parent_decls_interface clss.c_implements fa in
    let (uses, fa) = parent_decls_trait clss.c_uses fa in
    let json =
      TraitDefinition.(
        {
          declaration = TraitDeclaration.Id decl_id;
          members;
          implements_;
          uses;
          attributes;
          type_params;
          require_extends;
          require_implements;
          require_class = Some require_class;
          module_;
        }
        |> to_json_key)
    in
    Fact_acc.add_fact Predicate.(Hack TraitDefinition) json fa
  | Predicate.ClassContainer ->
    let (implements_, fa) = parent_decls_interface clss.c_implements fa in
    let (uses, fa) = parent_decls_trait clss.c_uses fa in
    let (extends_, fa) =
      match clss.c_extends with
      | [] -> (None, fa)
      | [parent] ->
        (match Util.class_hint_to_qname parent with
        | Some qname ->
          let (decl_id, fa) =
            let json = ClassDeclaration.(to_json_key { name = qname }) in
            Fact_acc.add_fact Predicate.(Hack ClassDeclaration) json fa
          in
          (Some (ClassDeclaration.Id decl_id), fa)
        | None -> (None, fa))
      | _ ->
        Hh_logger.log
          "WARNING: skipping extends field for class with multiple parents %s"
          (snd clss.c_name);
        (None, fa)
    in
    let json =
      ClassDefinition.(
        {
          declaration = ClassDeclaration.Id decl_id;
          is_abstract = Ast_defs.is_classish_abstract clss.c_kind;
          is_final = clss.c_final;
          members;
          extends_;
          implements_;
          uses;
          attributes;
          type_params;
          module_;
        }
        |> to_json_key)
    in
    Fact_acc.add_fact Predicate.(Hack ClassDefinition) json fa

let build_signature source_text params ctxs ret fa =
  let (params, fa) =
    List.fold params ~init:([], fa) ~f:(fun (t_params, fa) p ->
        let (type_, type_info, fa) =
          Aast.hint_of_type_hint p.param_type_hint |> hint_opt_to_type_info fa
        in
        ((p, type_info, type_) :: t_params, fa))
  in
  let params = List.rev params in
  let (returns, returns_type_info, fa) =
    Aast.hint_of_type_hint ret |> hint_opt_to_type_info fa
  in
  let signature =
    Build_fact.signature source_text params ctxs ~returns ~returns_type_info
  in
  (signature, fa)

let method_defn source_text meth decl_id fa =
  let m_tparams = Util.remove_generated_tparams meth.m_tparams in
  let (type_params, fa) = type_params source_text m_tparams fa in
  let (signature, fa) =
    build_signature source_text meth.m_params meth.m_ctxs meth.m_ret fa
  in
  let readonly_ret =
    Option.map meth.m_readonly_ret ~f:(fun _ -> ReadonlyKind.Readonly)
  in
  let is_readonly_this =
    if meth.m_readonly_this then
      Some meth.m_readonly_this
    else
      None
  in
  let json =
    MethodDefinition.(
      {
        declaration = MethodDeclaration.Id decl_id;
        signature;
        visibility = Util.(make_visibility meth.m_visibility);
        is_abstract = meth.m_abstract;
        is_async = is_async meth.m_fun_kind;
        is_final = meth.m_final;
        is_static = meth.m_static;
        attributes = Build_fact.attributes source_text meth.m_user_attributes;
        type_params;
        readonly_ret;
        is_readonly_this;
      }
      |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack MethodDefinition) json fa

let method_overrides
    ~annotation
    meth_name
    base_cont_name
    base_cont_type
    der_cont_name
    der_cont_type
    fa =
  let json =
    MethodOverrides.(
      {
        annotation = Some annotation;
        derived = Build_fact.method_decl meth_name der_cont_name der_cont_type;
        base = Build_fact.method_decl meth_name base_cont_name base_cont_type;
      }
      |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack MethodOverrides) json fa

let property_defn source_text prop decl_id fa =
  let (type_, type_info, fa) =
    Aast.hint_of_type_hint prop.cv_type |> hint_opt_to_type_info fa
  in
  let json =
    PropertyDefinition.(
      {
        declaration = PropertyDeclaration.Id decl_id;
        type_ : Type.t option;
        visibility = Util.(make_visibility prop.cv_visibility);
        is_final = prop.cv_final;
        is_abstract = prop.cv_abstract;
        is_static = prop.cv_is_static;
        attributes = Build_fact.attributes source_text prop.cv_user_attributes;
        type_info;
      }
      |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack PropertyDefinition) json fa

let class_const_defn source_text const decl_id fa =
  let value =
    match const.cc_kind with
    | CCAbstract None -> None
    | CCAbstract (Some expr)
    | CCConcrete expr ->
      Some (Pretty.expr_to_string source_text expr)
  in
  let (type_, type_info, fa) = hint_opt_to_type_info fa const.cc_type in
  let json =
    ClassConstDefinition.(
      {
        declaration = ClassConstDeclaration.Id decl_id;
        type_;
        value;
        type_info;
      }
      |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack ClassConstDefinition) json fa

let type_const_defn source_text tc decl_id fa =
  (* TODO(T88552052) should the default of an abstract type constant be used
     * as a value here *)
  let (type_, type_info, fa) =
    match tc.c_tconst_kind with
    | TCConcrete { c_tc_type = h }
    | TCAbstract { c_atc_default = Some h; _ } ->
      let (x, y, fa) = hint_to_type_info h fa in
      (Some x, Some y, fa)
    | TCAbstract { c_atc_default = None; _ } -> (None, None, fa)
  in
  let json =
    TypeConstDefinition.(
      {
        declaration = TypeConstDeclaration.Id decl_id;
        kind = Util.(make_type_const_kind tc.c_tconst_kind);
        type_;
        attributes =
          Build_fact.attributes source_text tc.c_tconst_user_attributes;
        type_info;
      }
      |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack TypeConstDefinition) json fa

let enum_decl name fa =
  let json = EnumDeclaration.({ name = Util.make_qname name } |> to_json_key) in
  Fact_acc.add_fact Predicate.(Hack EnumDeclaration) json fa

let enum_defn source_text enm enum_id enum_data enumerators fa =
  let enumerators = List.map enumerators ~f:(fun x -> Enumerator.Id x) in
  let fa = namespace_decl_opt enm.c_namespace fa in
  let (includes, fa) = parent_decls_enum enum_data.e_includes fa in
  let (module_, fa) = module_field enm.c_module enm.c_internal fa in
  let (enum_base, enum_base_type_info, fa) =
    hint_to_type_info enum_data.e_base fa
  in
  let (enum_constraint, enum_constraint_type_info, fa) =
    hint_opt_to_type_info fa enum_data.e_constraint
  in
  let json =
    EnumDefinition.(
      {
        declaration = EnumDeclaration.Id enum_id;
        enum_base;
        enumerators;
        attributes = Build_fact.attributes source_text enm.c_user_attributes;
        includes;
        is_enum_class = Aast.is_enum_class enm;
        module_;
        enum_constraint;
        enum_constraint_type_info;
        enum_base_type_info = Some enum_base_type_info;
      }
      |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack EnumDefinition) json fa

let enumerator decl_id const_name fa =
  let json =
    Enumerator.(
      {
        name = Util.make_name const_name;
        enumeration = EnumDeclaration.Id decl_id;
      }
      |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack Enumerator) json fa

let func_decl name fa =
  let json =
    FunctionDeclaration.({ name = Util.make_qname name } |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack FunctionDeclaration) json fa

let func_defn source_text fd decl_id fa =
  let elem = fd.fd_fun in
  let fa = namespace_decl_opt fd.fd_namespace fa in
  let fd_tparams = Util.remove_generated_tparams fd.fd_tparams in
  let (type_params, fa) = type_params source_text fd_tparams fa in
  let (module_, fa) = module_field fd.fd_module fd.fd_internal fa in
  let (signature, fa) =
    build_signature source_text elem.f_params elem.f_ctxs elem.f_ret fa
  in
  let readonly_ret =
    Option.map elem.f_readonly_ret ~f:(fun _ -> ReadonlyKind.Readonly)
  in
  let json =
    FunctionDefinition.(
      {
        declaration = FunctionDeclaration.Id decl_id;
        signature;
        is_async = is_async elem.f_fun_kind;
        attributes = Build_fact.attributes source_text elem.f_user_attributes;
        type_params;
        readonly_ret;
        module_;
      }
      |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack FunctionDefinition) json fa

let module_defn source_text elem decl_id fa =
  let json =
    ModuleDefinition.(
      {
        declaration = ModuleDeclaration.Id decl_id;
        attributes = Build_fact.attributes source_text elem.md_user_attributes;
      }
      |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack ModuleDefinition) json fa

let typedef_decl name fa =
  let json =
    TypedefDeclaration.({ name = Util.make_qname name } |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack TypedefDeclaration) json fa

let typedef_defn source_text elem decl_id fa =
  let fa = namespace_decl_opt elem.t_namespace fa in
  let is_transparent =
    match elem.t_assignment with
    | SimpleTypeDef { tvh_vis = Transparent; tvh_hint = _ } -> true
    | SimpleTypeDef { tvh_vis = Opaque; tvh_hint = _ }
    | SimpleTypeDef { tvh_vis = OpaqueModule; tvh_hint = _ }
    | CaseType _ ->
      false
  in
  let (type_params, fa) = type_params source_text elem.t_tparams fa in
  let (module_, fa) = module_field elem.t_module elem.t_internal fa in
  let json =
    TypedefDefinition.(
      {
        declaration = TypedefDeclaration.Id decl_id;
        is_transparent;
        attributes = Build_fact.attributes source_text elem.t_user_attributes;
        type_params;
        module_;
      }
      |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack TypedefDefinition) json fa

let gconst_decl name fa =
  let json =
    GlobalConstDeclaration.({ name = Util.make_qname name } |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack GlobalConstDeclaration) json fa

let gconst_defn source_text elem decl_id fa =
  let fa = namespace_decl_opt elem.cst_namespace fa in
  let value = Pretty.expr_to_string source_text elem.cst_value in
  let (type_, type_info, fa) = hint_opt_to_type_info fa elem.cst_type in
  let json =
    GlobalConstDefinition.(
      {
        declaration = GlobalConstDeclaration.Id decl_id;
        type_;
        value;
        type_info;
      }
      |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack GlobalConstDefinition) json fa

let decl_loc ~path pos declaration fa =
  let json =
    DeclarationLocation.(
      { declaration; file = File.(Key path); span = Util.(make_byte_span pos) }
      |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack DeclarationLocation) json fa

let decl_comment ~path pos declaration fa =
  let json =
    DeclarationComment.(
      { declaration; file = File.Key path; span = Util.make_byte_span pos }
      |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack DeclarationComment) json fa

let decl_span ~path pos declaration fa =
  let json =
    DeclarationSpan.(
      { declaration; file = File.Key path; span = Util.make_byte_span pos }
      |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack DeclarationSpan) json fa

let file_lines ~path sourceText fa =
  let lengths =
    Line_break_map.offsets_to_line_lengths
      sourceText.Full_fidelity_source_text.offset_map
  in
  let ends_in_newline = Util.ends_in_newline sourceText in
  let has_unicode_or_tabs = Util.has_tabs_or_multibyte_codepoints sourceText in
  let json =
    FileLines.(
      to_json_key
        { file = File.Key path; lengths; ends_in_newline; has_unicode_or_tabs })
  in
  Fact_acc.add_fact Predicate.(Src FileLines) json fa

let file_xrefs ~path fact_map fa =
  let json =
    FileXRefs.(
      { file = File.Key path; xrefs = Build_fact.xrefs fact_map } |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack FileXRefs) json fa

let file_decls ~path declarations fa =
  let json =
    FileDeclarations.({ file = File.Key path; declarations } |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack FileDeclarations) json fa

let method_occ receiver_class name fa =
  let json =
    MethodOccurrence.(
      {
        name = Util.make_name name;
        class_name =
          (match receiver_class with
          | SymbolOccurrence.UnknownClass -> None
          | SymbolOccurrence.ClassName class_name ->
            Some (Util.make_name class_name));
      }
      |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack MethodOccurrence) json fa

let file_call ~path pos ~callee_infos ~call_args ~dispatch_arg fa =
  let callee_xrefs = List.map callee_infos ~f:(fun ti -> ti.Xrefs.target) in
  let callee_xref =
    match List.sort ~compare:XRefTarget.compare callee_xrefs with
    | [] -> None
    | hd :: _ -> Some hd
  in
  let receiver_type =
    List.find_map callee_infos ~f:(fun ti -> ti.Xrefs.receiver_type)
  in
  let json =
    FileCall.(
      {
        file = File.Key path;
        callee_span = Util.(make_byte_span pos);
        callee_xref;
        dispatch_arg;
        receiver_type;
        call_args;
        callee_xrefs;
      }
      |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack FileCall) json fa

let global_namespace_alias ~from ~to_ fa =
  let json =
    GlobalNamespaceAlias.(
      { from = Name.(Key from); to_ = Util.make_namespaceqname to_ }
      |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack GlobalNamespaceAlias) json fa

let indexerInputsHash key hashes fa =
  let key = IndexerInputsHash.(key |> to_json_key) in
  let value =
    IndexerInputsHash.(List.map hashes ~f:Md5.to_binary |> to_json_value)
  in
  Fact_acc.add_fact Predicate.(Hack IndexerInputsHash) key ~value fa

let gen_code ~path ~fully_generated ~signature ~source ~command ~class_ fa =
  let json =
    GenCode.(
      {
        file = File.Key path;
        variant =
          (if fully_generated then
            GenCodeVariant.Full
          else
            GenCodeVariant.Partial);
        source = Option.map ~f:(fun x -> File.Key x) source;
        signature = Option.map ~f:(fun x -> GenCodeSignature.Key x) signature;
        command = Option.map ~f:(fun x -> GenCodeCommand.Key x) command;
        class_ = Option.map ~f:(fun x -> GenCodeClass.Key x) class_;
      }
      |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Gencode GenCode) json fa

let hack_to_thrift hack thrift fa =
  let json = HackToThrift.({ from = hack; to_ = thrift } |> to_json_key) in
  Fact_acc.add_fact Predicate.(Hack HackToThrift) json fa |> snd

let container_decl decl_pred name fa =
  let qname = Util.make_qname name in
  container_decl_qname decl_pred qname fa

let file_package ~path package_ has_package_override fa =
  let json =
    FilePackage.(
      {
        file = File.Key path;
        package_;
        hasPackageOverride = has_package_override;
      }
      |> to_json_key)
  in
  Fact_acc.add_fact Predicate.(Hack FilePackage) json fa
