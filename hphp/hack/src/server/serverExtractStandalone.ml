(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Cmd = ServerCommandTypes.Extract_standalone
module SourceText = Full_fidelity_source_text
module Syntax = Full_fidelity_positioned_syntax
module SyntaxTree = Full_fidelity_syntax_tree.WithSyntax (Syntax)
module SyntaxError = Full_fidelity_syntax_error
module SN = Naming_special_names
module Class = Decl_provider.Class

(* -- Exceptions ------------------------------------------------------------ *)

(* Internal error: for example, we are generating code for a dependency on an
   enum, but the passed dependency is not an enum *)
exception UnexpectedDependency

exception DependencyNotFound of string

exception Unsupported

let value_exn ex opt =
  match opt with
  | Some s -> s
  | None -> raise ex

let value_or_not_found err_msg opt = value_exn (DependencyNotFound err_msg) opt

(* -- Pretty-printing constants --------------------------------------------- *)

let __FN_MAKE_DEFAULT__ = "extract_standalone_make_default"

let __ANY__ = "EXTRACT_STANDALONE_ANY"

let __FILE_PREFIX__ = "////"

let __DUMMY_FILE__ = "__extract_standalone__.php"

let __UNKNOWN_FILE__ = "__unknown__.php"

let __HH_HEADER_PREFIX__ = "<?hh"

let __HH_HEADER_SUFFIX_PARTIAL__ = "// partial"

(* -- Format helpers -------------------------------------------------------- *)
module Fmt = struct
  let pf fmt = Format.fprintf fmt

  let string = Format.pp_print_string

  let sp ppf _ = Format.pp_print_space ppf ()

  let cut ppf _ = Format.pp_print_cut ppf ()

  let to_to_string pp_v v = Format.asprintf "%a" pp_v v

  let of_to_string f ppf v = string ppf (f v)

  let cond ~pp_t ~pp_f ppf test =
    if test then
      pp_t ppf ()
    else
      pp_f ppf ()

  let pair ?sep:(pp_sep = sp) pp_a pp_b ppf (a, b) =
    pp_a ppf a;
    pp_sep ppf ();
    pp_b ppf b

  let prefix pp_pfx pp_v ppf v =
    pp_pfx ppf ();
    pp_v ppf v

  let suffix pp_sfx pp_v ppf v =
    pp_v ppf v;
    pp_sfx ppf ()

  let surround l r pp_v ppf v =
    string ppf l;
    pp_v ppf v;
    string ppf r

  let quote pp_v ppf v = surround "'" "'" pp_v ppf v

  let parens pp_v ppf v = surround "(" ")" pp_v ppf v

  let braces pp_v ppf v = surround "{" "}" pp_v ppf v

  let angles pp_v ppf v = surround "<" ">" pp_v ppf v

  let brackets pp_v ppf v = surround "[" "]" pp_v ppf v

  let comma ppf _ =
    string ppf ",";
    sp ppf ()

  let amp ppf _ =
    sp ppf ();
    string ppf "&";
    sp ppf ()

  let equal_to ppf _ =
    sp ppf ();
    string ppf "=";
    sp ppf ()

  let colon ppf _ =
    sp ppf ();
    string ppf ":";
    sp ppf ()

  let semicolon ppf _ = string ppf ";"

  let arrow ppf _ = string ppf "->"

  let fat_arrow ppf _ =
    sp ppf ();
    string ppf "=>";
    sp ppf ()

  let const str ppf _ = string ppf str

  let dbl_colon = const "::"

  let vbar ppf _ =
    sp ppf ();
    string ppf "|";
    sp ppf ()

  let nop _ _ = ()

  let const pp_v v ppf _ = pp_v ppf v

  let option ?none:(pp_none = nop) pp_v ppf = function
    | Some v -> pp_v ppf v
    | _ -> pp_none ppf ()

  let list ?sep:(pp_sep = sp) pp_elt ppf v =
    let is_first = ref true in
    let pp_elt v =
      if !is_first then
        is_first := false
      else
        pp_sep ppf ();
      pp_elt ppf v
    in
    List.iter ~f:pp_elt v

  let hbox pp_v ppf v =
    Format.(
      pp_open_hbox ppf ();
      pp_v ppf v;
      pp_close_box ppf ())

  let vbox ?(indent = 0) pp_v ppf v =
    Format.(
      pp_open_vbox ppf indent;
      pp_v ppf v;
      pp_close_box ppf ())
end

(* -- Decl_provider helpers ------------------------------------------------- *)
module Decl : sig
  val get_class_exn :
    Provider_context.t -> Decl_provider.type_key -> Decl_provider.class_decl

  val get_class_pos :
    Provider_context.t -> Decl_provider.type_key -> Pos.t option

  val get_class_pos_exn : Provider_context.t -> Decl_provider.type_key -> Pos.t

  val get_fun_pos : Provider_context.t -> Decl_provider.fun_key -> Pos.t option

  val get_fun_pos_exn : Provider_context.t -> Decl_provider.fun_key -> Pos.t

  val get_typedef_pos :
    Provider_context.t -> Decl_provider.type_key -> Pos.t option

  val get_gconst_pos :
    Provider_context.t -> Decl_provider.gconst_key -> Pos.t option

  val get_class_or_typedef_pos : Provider_context.t -> string -> Pos.t option
end = struct
  let get_class_exn ctx name =
    let not_found_msg = Printf.sprintf "Class %s" name in
    value_or_not_found not_found_msg @@ Decl_provider.get_class ctx name

  let get_fun_pos ctx name =
    Decl_provider.get_fun ctx name
    |> Option.map
         ~f:
           Typing_defs.(
             (fun decl -> decl.fe_pos |> Naming_provider.resolve_position ctx))

  let get_fun_pos_exn ctx name = value_or_not_found name (get_fun_pos ctx name)

  let get_class_pos ctx name =
    Decl_provider.get_class ctx name
    |> Option.map ~f:(fun decl ->
           Class.pos decl |> Naming_provider.resolve_position ctx)

  let get_class_pos_exn ctx name =
    value_or_not_found name (get_class_pos ctx name)

  let get_typedef_pos ctx name =
    Decl_provider.get_typedef ctx name
    |> Option.map
         ~f:
           Typing_defs.(
             (fun decl -> decl.td_pos |> Naming_provider.resolve_position ctx))

  let get_gconst_pos ctx name =
    Decl_provider.get_gconst ctx name
    |> Option.map ~f:(fun const ->
           const.Typing_defs.cd_pos |> Naming_provider.resolve_position ctx)

  let get_class_or_typedef_pos ctx name =
    Option.first_some (get_class_pos ctx name) (get_typedef_pos ctx name)
end

(* -- Nast helpers ---------------------------------------------------------- *)
module Nast_helper : sig
  val get_fun : Provider_context.t -> string -> Nast.fun_ option

  val get_fun_exn : Provider_context.t -> string -> Nast.fun_

  val get_class : Provider_context.t -> string -> Nast.class_ option

  val get_class_exn : Provider_context.t -> string -> Nast.class_

  val get_typedef : Provider_context.t -> string -> Nast.typedef option

  val get_typedef_exn : Provider_context.t -> string -> Nast.typedef

  val get_gconst : Provider_context.t -> string -> Nast.gconst option

  val get_gconst_exn : Provider_context.t -> string -> Nast.gconst

  val get_method : Provider_context.t -> string -> string -> Nast.method_ option

  val get_method_exn : Provider_context.t -> string -> string -> Nast.method_

  val get_const :
    Provider_context.t -> string -> string -> Nast.class_const option

  val get_typeconst :
    Provider_context.t ->
    string ->
    string ->
    (Pos.t, Nast.func_body_ann, unit, unit) Aast.class_typeconst_def option

  val get_prop :
    Provider_context.t ->
    string ->
    string ->
    (Pos.t, Nast.func_body_ann, unit, unit) Aast.class_var option

  val is_tydef : Provider_context.t -> string -> bool

  val is_class : Provider_context.t -> string -> bool

  val is_enum : Provider_context.t -> string -> bool

  val is_interface : Provider_context.t -> string -> bool
end = struct
  let make_nast_getter ~get_pos ~find_in_file ~naming ctx name =
    Option.(
      get_pos ctx name >>= fun pos ->
      find_in_file ctx (Pos.filename pos) name |> map ~f:(naming ctx))

  let get_fun =
    make_nast_getter
      ~get_pos:Decl.get_fun_pos
      ~find_in_file:Ast_provider.find_fun_in_file
      ~naming:Naming.fun_

  let get_fun_exn ctx name = value_or_not_found name (get_fun ctx name)

  let get_class =
    make_nast_getter
      ~get_pos:Decl.get_class_pos
      ~find_in_file:Ast_provider.find_class_in_file
      ~naming:Naming.class_

  let get_class_exn ctx name = value_or_not_found name (get_class ctx name)

  let get_typedef =
    make_nast_getter
      ~get_pos:Decl.get_typedef_pos
      ~find_in_file:Ast_provider.find_typedef_in_file
      ~naming:Naming.typedef

  let get_typedef_exn ctx name = value_or_not_found name (get_typedef ctx name)

  let get_gconst =
    make_nast_getter
      ~get_pos:Decl.get_gconst_pos
      ~find_in_file:Ast_provider.find_gconst_in_file
      ~naming:Naming.global_const

  let get_gconst_exn ctx name = value_or_not_found name (get_gconst ctx name)

  let make_class_element_nast_getter ~get_elements ~get_element_name =
    let elements_by_class_name = ref SMap.empty in
    fun ctx class_name element_name ->
      if SMap.mem class_name !elements_by_class_name then
        SMap.find_opt
          element_name
          (SMap.find class_name !elements_by_class_name)
      else
        let open Option in
        get_class ctx class_name >>= fun class_ ->
        let elements_by_element_name =
          List.fold_left
            (get_elements class_)
            ~f:(fun elements element ->
              SMap.add (get_element_name element) element elements)
            ~init:SMap.empty
        in
        elements_by_class_name :=
          SMap.add class_name elements_by_element_name !elements_by_class_name;
        SMap.find_opt element_name elements_by_element_name

  let get_method =
    Aast.(
      make_class_element_nast_getter
        ~get_elements:(fun class_ -> class_.c_methods)
        ~get_element_name:(fun method_ -> snd method_.m_name))

  let get_method_exn ctx class_name method_name =
    value_or_not_found
      (class_name ^ "::" ^ method_name)
      (get_method ctx class_name method_name)

  let get_const =
    Aast.(
      make_class_element_nast_getter
        ~get_elements:(fun class_ -> class_.c_consts)
        ~get_element_name:(fun const -> snd const.cc_id))

  let get_typeconst =
    Aast.(
      make_class_element_nast_getter
        ~get_elements:(fun class_ -> class_.c_typeconsts)
        ~get_element_name:(fun typeconst -> snd typeconst.c_tconst_name))

  let get_prop =
    Aast.(
      make_class_element_nast_getter
        ~get_elements:(fun class_ -> class_.c_vars)
        ~get_element_name:(fun class_var -> snd class_var.cv_id))

  let is_tydef ctx nm = Option.is_some @@ get_typedef ctx nm

  let is_class ctx nm = Option.is_some @@ get_class ctx nm

  let is_enum ctx nm =
    Option.value_map ~default:false ~f:(fun Aast.{ c_kind; _ } ->
        match c_kind with
        | Ast_defs.Cenum -> true
        | _ -> false)
    @@ get_class ctx nm

  let is_interface ctx nm =
    Option.value_map ~default:false ~f:(fun Aast.{ c_kind; _ } ->
        match c_kind with
        | Ast_defs.Cinterface -> true
        | _ -> false)
    @@ get_class ctx nm
end

(* -- Typing_deps helpers --------------------------------------------------- *)
module Dep : sig
  val get_class_name : 'a Typing_deps.Dep.variant -> string option

  val get_relative_path :
    Provider_context.t ->
    Typing_deps.Dep.dependency Typing_deps.Dep.variant ->
    Relative_path.t Hh_prelude.Option.t

  val get_mode :
    Provider_context.t ->
    Typing_deps.Dep.dependency Typing_deps.Dep.variant ->
    FileInfo.mode Hh_prelude.Option.t

  val get_origin :
    Provider_context.t ->
    Decl_provider.type_key ->
    Typing_deps.Dep.dependency Typing_deps.Dep.variant ->
    string

  val is_builtin :
    Provider_context.t ->
    Typing_deps.Dep.dependency Typing_deps.Dep.variant ->
    bool

  val is_relevant :
    Cmd.target -> Typing_deps.Dep.dependent Typing_deps.Dep.variant -> bool
end = struct
  let get_class_name : type a. a Typing_deps.Dep.variant -> string option =
   (* the OCaml compiler is not smart enough to let us use an or-pattern for all of these
    * because of how it's matching on a GADT *)
   fun dep ->
    Typing_deps.Dep.(
      match dep with
      | Const (cls, _) -> Some cls
      | Method (cls, _) -> Some cls
      | SMethod (cls, _) -> Some cls
      | Prop (cls, _) -> Some cls
      | SProp (cls, _) -> Some cls
      | Type cls -> Some cls
      | Cstr cls -> Some cls
      | AllMembers cls -> Some cls
      | Extends cls -> Some cls
      | Fun _
      | FunName _
      | GConst _
      | GConstName _ ->
        None)

  let get_dep_pos ctx dep =
    let open Typing_deps.Dep in
    match dep with
    | Fun name
    | FunName name ->
      Decl.get_fun_pos ctx name
    | Type name
    | Const (name, _)
    | Method (name, _)
    | SMethod (name, _)
    | Prop (name, _)
    | SProp (name, _)
    | Cstr name
    | AllMembers name
    | Extends name ->
      Decl.get_class_or_typedef_pos ctx name
    | GConst name
    | GConstName name ->
      Decl.get_gconst_pos ctx name

  let get_relative_path ctx dep =
    Option.map ~f:(fun pos -> Pos.filename pos) @@ get_dep_pos ctx dep

  let get_fun_mode ctx name =
    Nast_helper.get_fun ctx name |> Option.map ~f:(fun fun_ -> fun_.Aast.f_mode)

  let get_class_mode ctx name =
    Nast_helper.get_class ctx name
    |> Option.map ~f:(fun class_ -> class_.Aast.c_mode)

  let get_typedef_mode ctx name =
    Nast_helper.get_typedef ctx name
    |> Option.map ~f:(fun typedef -> typedef.Aast.t_mode)

  let get_gconst_mode ctx name =
    Nast_helper.get_gconst ctx name
    |> Option.map ~f:(fun gconst -> gconst.Aast.cst_mode)

  let get_class_or_typedef_mode ctx name =
    Option.first_some (get_class_mode ctx name) (get_typedef_mode ctx name)

  let get_mode ctx dep =
    let open Typing_deps.Dep in
    match dep with
    | Fun name
    | FunName name ->
      get_fun_mode ctx name
    | Type name
    | Const (name, _)
    | Method (name, _)
    | SMethod (name, _)
    | Prop (name, _)
    | SProp (name, _)
    | Cstr name
    | AllMembers name
    | Extends name ->
      get_class_or_typedef_mode ctx name
    | GConst name
    | GConstName name ->
      get_gconst_mode ctx name

  let get_origin ctx cls (dep : 'a Typing_deps.Dep.variant) =
    let open Typing_deps.Dep in
    let description = variant_to_string dep in
    let cls =
      value_or_not_found description @@ Decl_provider.get_class ctx cls
    in
    match dep with
    | Prop (_, name) ->
      let Typing_defs.{ ce_origin; _ } =
        value_or_not_found description @@ Class.get_prop cls name
      in
      ce_origin
    | SProp (_, name) ->
      let Typing_defs.{ ce_origin; _ } =
        value_or_not_found description @@ Class.get_sprop cls name
      in
      ce_origin
    | Method (_, name) ->
      let Typing_defs.{ ce_origin; _ } =
        value_or_not_found description @@ Class.get_method cls name
      in
      ce_origin
    | SMethod (_, name) ->
      let Typing_defs.{ ce_origin; _ } =
        value_or_not_found description @@ Class.get_smethod cls name
      in
      ce_origin
    | Const (_, name) ->
      let Typing_defs.{ cc_origin; _ } =
        value_or_not_found description @@ Class.get_const cls name
      in
      cc_origin
    | Cstr cls -> cls
    | _ -> raise UnexpectedDependency

  let is_builtin ctx dep =
    let msg = Typing_deps.Dep.variant_to_string dep in
    let pos = value_or_not_found msg @@ get_dep_pos ctx dep in
    Relative_path.(is_hhi @@ prefix @@ Pos.filename pos)

  let is_relevant
      (target : Cmd.target)
      (dep : Typing_deps.Dep.dependent Typing_deps.Dep.variant) =
    Cmd.(
      match target with
      | Function f ->
        (match dep with
        | Typing_deps.Dep.Fun g
        | Typing_deps.Dep.FunName g ->
          String.equal f g
        | _ -> false)
      (* We have to collect dependencies of the entire class because dependency collection is
      coarse-grained: if cls's member depends on D, we get a dependency edge cls --> D,
      not (cls, member) --> D *)
      | Method (cls, _) ->
        Option.equal String.equal (get_class_name dep) (Some cls))
end

(* -- Extraction target helpers --------------------------------------------- *)
module Target : sig
  val pos : Provider_context.t -> Cmd.target -> Pos.t

  val relative_path : Provider_context.t -> Cmd.target -> Relative_path.t

  val source_text : Provider_context.t -> Cmd.target -> SourceText.t
end = struct
  let pos ctx =
    Cmd.(
      function
      | Function name -> Decl.get_fun_pos_exn ctx name
      | Method (name, _) -> Decl.get_class_pos_exn ctx name)

  let relative_path ctx target = Pos.filename @@ pos ctx target

  let source_text ctx target =
    let filename = relative_path ctx target in
    let abs_filename = Relative_path.to_absolute filename in
    let file_content = In_channel.read_all abs_filename in
    let pos =
      Cmd.(
        match target with
        | Function name ->
          let fun_ = Nast_helper.get_fun_exn ctx name in
          fun_.Aast.f_span
        | Method (class_name, method_name) ->
          let method_ = Nast_helper.get_method_exn ctx class_name method_name in
          method_.Aast.m_span)
    in
    SourceText.make filename @@ Pos.get_text_from_pos file_content pos
end

(* -- Dependency extraction logic ------------------------------------------- *)
module Extract : sig
  val collect_dependencies :
    Provider_context.t ->
    Cmd.target ->
    bool * bool * Typing_deps.Dep.dependency Typing_deps.Dep.variant list
end = struct
  type extraction_env = {
    dependencies: Typing_deps.Dep.dependency Typing_deps.Dep.variant HashSet.t;
    depends_on_make_default: bool ref;
    depends_on_any: bool ref;
  }

  let rec do_add_dep ctx env dep =
    let is_wildcard =
      match dep with
      | Typing_deps.Dep.Type h -> String.equal h SN.Typehints.wildcard
      | _ -> false
    in
    if
      (not is_wildcard)
      && (not (HashSet.mem env.dependencies dep))
      && not (Dep.is_builtin ctx dep)
    then (
      HashSet.add env.dependencies dep;
      add_signature_dependencies ctx env dep
    )

  and add_dep ctx env ~this ty : unit =
    let visitor =
      object
        inherit [unit] Type_visitor.decl_type_visitor as super

        method! on_tany _ _ = env.depends_on_any := true

        method! on_tfun () r ft =
          if
            List.exists
              ~f:Typing_defs.get_fp_has_default
              ft.Typing_defs.ft_params
          then
            env.depends_on_make_default := true;
          super#on_tfun () r ft

        method! on_tapply _ _ (_, name) tyl =
          let dep = Typing_deps.Dep.Type name in
          do_add_dep ctx env dep;

          (* If we have a constant of a generic type, it can only be an
           array type, e.g., vec<A>, for which don't need values of A
           to generate an initializer. *)
          List.iter tyl ~f:(add_dep ctx env ~this)

        method! on_tshape _ _ _ fdm =
          Typing_defs.TShapeMap.iter
            (fun name Typing_defs.{ sft_ty; _ } ->
              (match name with
              | Typing_defs.TSFlit_int _
              | Typing_defs.TSFlit_str _ ->
                ()
              | Typing_defs.TSFclass_const ((_, c), (_, s)) ->
                do_add_dep ctx env (Typing_deps.Dep.Type c);
                do_add_dep ctx env (Typing_deps.Dep.Const (c, s)));
              add_dep ctx env ~this sft_ty)
            fdm

        (* We un-nest (((this::T1)::T2)::T3) into (this, [T1;T2;T3]) and then re-nest
         * because legacy representation of Taccess was using lists. TODO: implement
         * this more directly instead.
         *)
        method! on_taccess () r (root, tconst) =
          let rec split_taccess root ids =
            Typing_defs.(
              match get_node root with
              | Taccess (root, id) -> split_taccess root (id :: ids)
              | _ -> (root, ids))
          in
          let rec make_taccess r root ids =
            match ids with
            | [] -> root
            | id :: ids ->
              make_taccess
                Typing_reason.Rnone
                Typing_defs.(mk (r, Taccess (root, id)))
                ids
          in
          let (root, tconsts) = split_taccess root [tconst] in
          let expand_type_access class_name tconsts =
            match tconsts with
            | [] -> raise UnexpectedDependency
            (* Expand Class::TConst1::TConst2[::...]: get TConst1 in
             Class, get its type or upper bound T, continue adding
             dependencies of T::TConst2[::...] *)
            | (_, tconst) :: tconsts ->
              do_add_dep ctx env (Typing_deps.Dep.Const (class_name, tconst));
              let cls = Decl.get_class_exn ctx class_name in
              (match Decl_provider.Class.get_typeconst cls tconst with
              | Some
                  Typing_defs.
                    { ttc_type; ttc_as_constraint; ttc_super_constraint; _ } ->
                Option.iter
                  ttc_type
                  ~f:(add_dep ctx ~this:(Some class_name) env);
                let add_cstr_dep tc_type =
                  (* What does 'this' refer to inside of T? *)
                  let this =
                    match Typing_defs.get_node tc_type with
                    | Typing_defs.Tapply ((_, name), _) -> Some name
                    | _ -> this
                  in
                  let taccess = make_taccess r tc_type tconsts in
                  add_dep ctx ~this env taccess
                in
                if not (List.is_empty tconsts) then (
                  match (ttc_type, ttc_as_constraint, ttc_super_constraint) with
                  | (None, Some as_tc_type, Some super_tc_type) ->
                    add_cstr_dep as_tc_type;
                    add_cstr_dep super_tc_type
                  (* TODO(coeffects) double-check this *)
                  | (Some tc_type, _, _)
                  | (None, Some tc_type, _)
                  | (None, _, Some tc_type) ->
                    add_cstr_dep tc_type
                  | (None, None, None) -> ()
                )
              | None -> ())
          in
          match Typing_defs.get_node root with
          | Typing_defs.Taccess (root', tconst) ->
            add_dep ctx ~this env (make_taccess r root' (tconst :: tconsts))
          | Typing_defs.Tapply ((_, name), _) -> expand_type_access name tconsts
          | Typing_defs.Tthis ->
            expand_type_access (Option.value_exn this) tconsts
          | _ -> raise UnexpectedDependency
      end
    in
    visitor#on_type () ty

  and add_signature_dependencies ctx env obj =
    let description = Typing_deps.Dep.variant_to_string obj in
    match Dep.get_class_name obj with
    | Some cls_name ->
      do_add_dep ctx env (Typing_deps.Dep.Type cls_name);
      Option.iter ~f:(add_class_attr_deps ctx env)
      @@ Nast_helper.get_class ctx cls_name;
      (match Decl_provider.get_class ctx cls_name with
      | None ->
        let Typing_defs.{ td_type; td_constraint; _ } =
          value_or_not_found description
          @@ Decl_provider.get_typedef ctx cls_name
        in
        Option.iter ~f:(add_tydef_attr_deps ctx env)
        @@ Nast_helper.get_typedef ctx cls_name;
        add_dep ctx ~this:None env td_type;
        Option.iter td_constraint ~f:(add_dep ctx ~this:None env)
      | Some cls ->
        let add_dep = add_dep ctx env ~this:(Some cls_name) in
        Typing_deps.Dep.(
          (match obj with
          | Prop (_, name) ->
            let Typing_defs.{ ce_type; _ } =
              value_or_not_found description @@ Class.get_prop cls name
            in
            add_dep @@ Lazy.force ce_type;
            Option.iter ~f:(add_classvar_attr_deps ctx env)
            @@ Nast_helper.get_prop ctx cls_name name;
            (* We need to initialize properties in the constructor, add a dependency on it *)
            do_add_dep ctx env (Cstr cls_name)
          | SProp (_, name) ->
            let Typing_defs.{ ce_type; _ } =
              value_or_not_found description @@ Class.get_sprop cls name
            in
            add_dep @@ Lazy.force ce_type;
            Option.iter ~f:(add_classvar_attr_deps ctx env)
            @@ Nast_helper.get_prop ctx cls_name name
          | Method (_, name) ->
            let Typing_defs.{ ce_type; _ } =
              value_or_not_found description @@ Class.get_method cls name
            in
            add_dep @@ Lazy.force ce_type;
            Option.iter ~f:(add_method_attr_deps ctx env)
            @@ Nast_helper.get_method ctx cls_name name;
            Class.all_ancestor_names cls
            |> List.iter ~f:(fun ancestor_name ->
                   match Decl_provider.get_class ctx ancestor_name with
                   | Some ancestor when Class.has_method ancestor name ->
                     do_add_dep ctx env (Method (ancestor_name, name))
                   | _ -> ())
          | SMethod (_, name) ->
            (match Class.get_smethod cls name with
            | Some Typing_defs.{ ce_type; _ } ->
              add_dep @@ Lazy.force ce_type;
              Option.iter ~f:(add_method_attr_deps ctx env)
              @@ Nast_helper.get_method ctx cls_name name;
              Class.all_ancestor_names cls
              |> List.iter ~f:(fun ancestor_name ->
                     match Decl_provider.get_class ctx ancestor_name with
                     | Some ancestor when Class.has_smethod ancestor name ->
                       do_add_dep ctx env (SMethod (ancestor_name, name))
                     | _ -> ())
            | None ->
              (match Class.get_method cls name with
              | Some _ ->
                HashSet.remove env.dependencies obj;
                do_add_dep ctx env (Method (cls_name, name))
              | None -> raise (DependencyNotFound description)))
          | Const (_, name) ->
            (match Class.get_typeconst cls name with
            | Some Typing_defs.{ ttc_type; ttc_as_constraint; ttc_origin; _ } ->
              if not (String.equal cls_name ttc_origin) then
                do_add_dep ctx env (Const (ttc_origin, name));

              Option.iter ~f:(add_tyconst_attr_deps ctx env)
              @@ Nast_helper.get_typeconst ctx ttc_origin name;
              Option.iter ttc_type ~f:add_dep;
              Option.iter ttc_as_constraint ~f:add_dep
            | None ->
              let Typing_defs.{ cc_type; _ } =
                value_or_not_found description @@ Class.get_const cls name
              in
              add_dep cc_type)
          | Cstr _ ->
            (match Class.construct cls with
            | (Some Typing_defs.{ ce_type; _ }, _) ->
              add_dep @@ Lazy.force ce_type;
              Option.iter ~f:(add_method_attr_deps ctx env)
              @@ Nast_helper.get_method ctx cls_name "__construct"
            | _ -> ())
          | Type _ ->
            List.iter (Class.all_ancestors cls) (fun (_, ty) -> add_dep ty);
            List.iter (Class.all_ancestor_reqs cls) (fun (_, ty) -> add_dep ty);
            Option.iter
              (Class.enum_type cls)
              ~f:(fun Typing_defs.{ te_base; te_constraint; te_includes; _ } ->
                add_dep te_base;
                Option.iter te_constraint ~f:add_dep;
                List.iter te_includes ~f:add_dep)
          | AllMembers _ ->
            (* AllMembers is used for dependencies on enums, so we should depend on all constants *)
            List.iter
              (Class.consts cls)
              ~f:(fun (name, Typing_defs.{ cc_type; _ }) ->
                if not (String.equal name "class") then add_dep cc_type)
          (* Ignore, we fetch class hierarchy when we call add_signature_dependencies on a class dep *)
          | Extends _ -> ()
          | _ -> raise UnexpectedDependency)))
    | None ->
      Typing_deps.Dep.(
        (match obj with
        | Fun f
        | FunName f ->
          let Typing_defs.{ fe_type; _ } =
            value_or_not_found description @@ Decl_provider.get_fun ctx f
          in
          add_dep ctx ~this:None env @@ fe_type;
          Option.iter ~f:(add_fun_attr_deps ctx env)
          @@ Nast_helper.get_fun ctx f
        | GConst c
        | GConstName c ->
          let const =
            value_or_not_found description @@ Decl_provider.get_gconst ctx c
          in
          add_dep ctx ~this:None env const.Typing_defs.cd_type
        | _ -> raise UnexpectedDependency))

  and add_user_attr_deps ctx env user_attrs =
    List.iter user_attrs ~f:(fun Aast.{ ua_name = (_, cls); _ } ->
        if not @@ String.is_prefix ~prefix:"__" cls then (
          do_add_dep ctx env @@ Typing_deps.Dep.Type cls;
          do_add_dep ctx env @@ Typing_deps.Dep.Cstr cls
        ))

  and add_class_attr_deps
      ctx env Aast.{ c_user_attributes = attrs; c_tparams = tparams; _ } =
    add_user_attr_deps ctx env attrs;
    List.iter tparams ~f:(add_tparam_attr_deps ctx env)

  and add_classvar_attr_deps ctx env Aast.{ cv_user_attributes = attrs; _ } =
    add_user_attr_deps ctx env attrs

  and add_tyconst_attr_deps ctx env Aast.{ c_tconst_user_attributes = attrs; _ }
      =
    add_user_attr_deps ctx env attrs

  and add_arg_attr_deps ctx env (attrs, params, tparams) =
    add_user_attr_deps ctx env attrs;
    List.iter params ~f:(add_fun_param_attr_deps ctx env);
    List.iter tparams ~f:(add_tparam_attr_deps ctx env)

  and add_fun_attr_deps
      ctx
      env
      Aast.
        { f_user_attributes = attrs; f_params = params; f_tparams = tparams; _ }
      =
    add_arg_attr_deps ctx env (attrs, params, tparams)

  and add_method_attr_deps
      ctx
      env
      Aast.
        { m_user_attributes = attrs; m_tparams = tparams; m_params = params; _ }
      =
    add_arg_attr_deps ctx env (attrs, params, tparams)

  and add_fun_param_attr_deps ctx env Aast.{ param_user_attributes = attrs; _ }
      =
    add_user_attr_deps ctx env attrs

  and add_tydef_attr_deps ctx env Aast.{ t_user_attributes = attrs; _ } =
    add_user_attr_deps ctx env attrs

  and add_tparam_attr_deps
      ctx env Aast.{ tp_user_attributes = attrs; tp_parameters = tparams; _ } =
    add_user_attr_deps ctx env attrs;
    List.iter tparams ~f:(add_tparam_attr_deps ctx env)

  let add_impls ~ctx ~env ~cls acc ancestor_name =
    let open Typing_deps.Dep in
    let ancestor = Decl.get_class_exn ctx ancestor_name in
    let add_smethod_impl acc smethod_name =
      Option.value_map ~default:acc ~f:(fun Typing_defs.{ ce_origin; _ } ->
          Typing_deps.Dep.SMethod (ce_origin, smethod_name) :: acc)
      @@ Class.get_smethod cls smethod_name
    in
    let add_method_impl acc method_name =
      Option.value_map ~default:acc ~f:(fun Typing_defs.{ ce_origin; _ } ->
          Typing_deps.Dep.Method (ce_origin, method_name) :: acc)
      @@ Class.get_method cls method_name
    in
    let add_tyconst_impl acc typeconst_name =
      Option.value_map ~default:acc ~f:(fun Typing_defs.{ ttc_origin; _ } ->
          Typing_deps.Dep.Const (ttc_origin, typeconst_name) :: acc)
      @@ Class.get_typeconst cls typeconst_name
    in
    let add_const_impl acc const_name =
      Option.value_map ~default:acc ~f:(fun Typing_defs.{ cc_origin; _ } ->
          Typing_deps.Dep.Const (cc_origin, const_name) :: acc)
      @@ Class.get_const cls const_name
    in
    if Dep.is_builtin ctx (Type ancestor_name) then
      let with_smths =
        List.fold ~init:acc ~f:(fun acc (nm, _) -> add_smethod_impl acc nm)
        @@ Class.smethods ancestor
      in
      let with_mths =
        List.fold ~init:with_smths ~f:(fun acc (nm, _) ->
            add_method_impl acc nm)
        @@ Class.methods ancestor
      in
      let with_tyconsts =
        List.fold ~init:with_mths ~f:(fun acc (nm, _) ->
            add_tyconst_impl acc nm)
        @@ Class.typeconsts ancestor
      in
      List.fold ~init:with_tyconsts ~f:(fun acc (nm, _) ->
          add_const_impl acc nm)
      @@ Class.consts ancestor
    else
      let f dep acc =
        Typing_deps.Dep.(
          match dep with
          | SMethod (class_name, method_name)
            when String.equal class_name ancestor_name ->
            add_smethod_impl acc method_name
          | Method (class_name, method_name)
            when String.equal class_name ancestor_name ->
            add_method_impl acc method_name
          | Const (class_name, name)
            when String.equal class_name ancestor_name
                 && Option.is_some (Class.get_typeconst ancestor name) ->
            add_tyconst_impl acc name
          | Const (class_name, name)
            when String.equal class_name ancestor_name
                 && Option.is_some (Class.get_const ancestor name) ->
            add_const_impl acc name
          | _ -> acc)
      in
      HashSet.fold ~init:acc ~f env.dependencies

  (** Implementation dependencies of all ancestor classes and trait / interface
    requirements *)
  let get_implementation_dependencies ctx env cls =
    let f = add_impls ~ctx ~env ~cls in
    let init =
      Option.value_map ~default:[] ~f:(fun ss ->
          List.map ~f:(fun nm -> Typing_deps.Dep.Type nm) @@ SSet.elements ss)
      @@ Class.sealed_whitelist cls
    in
    let ancs = List.fold ~init ~f @@ Class.all_ancestor_names cls in
    List.fold ~f ~init:ancs @@ Class.all_ancestor_req_names cls

  (** Add implementation depedencies of all class dependencies until we reach
    a fixed point *)
  let rec add_implementation_dependencies ctx env =
    let size = HashSet.length env.dependencies in
    let add_class dep acc =
      match dep with
      | Typing_deps.Dep.Type cls_name ->
        Option.value_map ~default:acc ~f:(fun cls -> cls :: acc)
        @@ Decl_provider.get_class ctx cls_name
      | _ -> acc
    in
    env.dependencies
    |> HashSet.fold ~init:[] ~f:add_class
    |> List.concat_map ~f:(get_implementation_dependencies ctx env)
    |> List.iter ~f:(do_add_dep ctx env);
    if HashSet.length env.dependencies <> size then
      add_implementation_dependencies ctx env

  (** Collect dependencies from type decls and transitive closure of resulting
    implementation dependencies; determine if any of our dependencies
    depend on any or default parameter values *)
  let collect_dependencies ctx target =
    let filename = Target.relative_path ctx target in
    let env =
      {
        dependencies = HashSet.create ();
        depends_on_make_default = ref false;
        depends_on_any = ref false;
      }
    in
    let add_dependency
        (root : Typing_deps.Dep.dependent Typing_deps.Dep.variant)
        (obj : Typing_deps.Dep.dependency Typing_deps.Dep.variant) : unit =
      if Dep.is_relevant target root then do_add_dep ctx env obj
    in
    Typing_deps.add_dependency_callback "add_dependency" add_dependency;
    (* Collect dependencies through side effects of typechecking and remove
     * the target function/method from the set of dependencies to avoid
     * declaring it twice.
     *)
    let () =
      Typing_deps.Dep.(
        match target with
        | Cmd.Function func ->
          let (_ : (Tast.def * Typing_inference_env.t_global_with_pos) option) =
            Typing_check_service.type_fun ctx filename func
          in
          add_implementation_dependencies ctx env;
          HashSet.remove env.dependencies (Fun func);
          HashSet.remove env.dependencies (FunName func)
        | Cmd.Method (cls, m) ->
          let (_
                : (Tast.def * Typing_inference_env.t_global_with_pos list)
                  option) =
            Typing_check_service.type_class ctx filename cls
          in
          HashSet.add env.dependencies (Method (cls, m));
          HashSet.add env.dependencies (SMethod (cls, m));
          add_implementation_dependencies ctx env;
          HashSet.remove env.dependencies (Method (cls, m));
          HashSet.remove env.dependencies (SMethod (cls, m)))
    in
    ( !(env.depends_on_make_default),
      !(env.depends_on_any),
      HashSet.fold env.dependencies ~init:[] ~f:List.cons )
end

(* -- Grouped dependencies -------------------------------------------------  *)
module Grouped : sig
  type t

  val name : t -> string

  val line : t -> int

  val compare : t -> t -> int

  val pp : Format.formatter -> t -> unit

  val of_deps :
    Provider_context.t ->
    Cmd.target option ->
    Typing_deps.Dep.dependency Typing_deps.Dep.variant list ->
    t list
end = struct
  let strip_ns obj_name =
    match String.rsplit2 obj_name '\\' with
    | Some (_, name) -> name
    | None -> obj_name

  let is_contextual_param Aast.{ tp_name = (_, name); _ } =
    String.is_substring ~substring:"Tctx" name

  (** Generate an initial value based on type hint *)
  let init_value ctx hint =
    let unsupported_hint _ =
      Hh_logger.log
        "%s: get_init_from_hint: unsupported hint: %s"
        (Pos.string (Pos.to_absolute (fst hint)))
        (Aast_defs.show_hint hint);
      raise Unsupported
    in
    let rec pp tparams ppf hint =
      Aast.(
        match snd hint with
        | Hprim prim -> pp_prim ppf prim
        | Hoption _ -> Fmt.string ppf "null"
        | Hlike hint -> pp tparams ppf hint
        | Hdarray _ -> Fmt.string ppf "darray[]"
        | Hvec_or_dict _
        | Hvarray_or_darray _
        | Hvarray _ ->
          Fmt.string ppf "varray[]"
        | Htuple hs ->
          Fmt.(
            prefix (const string "tuple")
            @@ parens
            @@ list ~sep:comma (pp tparams))
            ppf
            hs
        | Hshape { nsi_field_map; _ } ->
          Fmt.(
            prefix (const string "shape")
            @@ parens
            @@ list ~sep:comma (pp_shape_field tparams))
            ppf
          @@ List.filter nsi_field_map ~f:(fun shape_field_info ->
                 not shape_field_info.sfi_optional)
        | Happly ((_, name), _)
          when String.(
                 name = SN.Collections.cVec
                 || name = SN.Collections.cKeyset
                 || name = SN.Collections.cDict) ->
          Fmt.(suffix (const string "[]") string) ppf @@ strip_ns name
        | Happly ((_, name), _)
          when String.(
                 name = SN.Collections.cVector
                 || name = SN.Collections.cImmVector
                 || name = SN.Collections.cMap
                 || name = SN.Collections.cImmMap
                 || name = SN.Collections.cSet
                 || name = SN.Collections.cImmSet) ->
          Fmt.(suffix (const string " {}") string) ppf @@ strip_ns name
        | Happly ((_, name), [fst; snd])
          when String.(name = SN.Collections.cPair) ->
          Fmt.(
            prefix (const string "Pair ")
            @@ braces
            @@ pair ~sep:comma (pp tparams) (pp tparams))
            ppf
            (fst, snd)
        | Happly ((_, name), [(_, Happly ((_, class_name), _))])
          when String.(name = SN.Classes.cClassname) ->
          Fmt.(suffix (const string "::class") string) ppf class_name
        | Happly ((_, name), hints) ->
          (match Nast_helper.get_class ctx name with
          | Some
              {
                c_kind = Ast_defs.Cenum;
                c_consts = Aast.{ cc_id = (_, const_name); _ } :: _;
                _;
              } ->
            Fmt.(pair ~sep:dbl_colon string string) ppf (name, const_name)
          | Some _ -> unsupported_hint ()
          | _ ->
            let typedef = Nast_helper.get_typedef_exn ctx name in
            let ts =
              List.fold2_exn
                typedef.t_tparams
                hints
                ~init:SMap.empty
                ~f:(fun tparams tparam hint ->
                  SMap.add (snd tparam.tp_name) hint tparams)
            in
            pp (ts :: tparams) ppf typedef.t_kind)
        | Habstr (name, []) ->
          (* FIXME: support non-empty type arguments of Habstr here? *)
          let rec loop tparams_stack =
            match tparams_stack with
            | tparams :: tparams_stack' ->
              (match SMap.find_opt name tparams with
              | Some hint -> pp tparams_stack' ppf hint
              | None -> loop tparams_stack')
            | [] -> unsupported_hint ()
          in
          loop tparams
        | _ -> unsupported_hint ())
    and pp_prim ppf =
      Aast_defs.(
        function
        | Tnull -> Fmt.string ppf "null"
        | Tint
        | Tnum ->
          Fmt.string ppf "0"
        | Tbool -> Fmt.string ppf "false"
        | Tfloat -> Fmt.string ppf "0.0"
        | Tstring
        | Tarraykey ->
          Fmt.string ppf "\"\""
        | Tvoid
        | Tresource
        | Tnoreturn ->
          raise Unsupported)
    and pp_shape_field tparams ppf Aast.{ sfi_hint; sfi_name; _ } =
      Fmt.(pair ~sep:fat_arrow pp_shape_field_name @@ pp tparams)
        ppf
        (sfi_name, sfi_hint)
    and pp_shape_field_name ppf =
      Ast_defs.(
        function
        | SFlit_int (_, s) -> Fmt.string ppf s
        | SFlit_str (_, s) -> Fmt.(quote string) ppf s
        | SFclass_const ((_, c), (_, s)) ->
          Fmt.(pair ~sep:dbl_colon string string) ppf (c, s))
    in

    Format.asprintf "%a" (pp []) hint

  (* -- Shared pretty printers ---------------------------------------------- *)
  let pp_visibility ppf = function
    | Ast_defs.Private -> Fmt.string ppf "private"
    | Ast_defs.Public -> Fmt.string ppf "public"
    | Ast_defs.Protected -> Fmt.string ppf "protected"

  let pp_paramkind ppf =
    Ast_defs.(
      function
      | Pinout -> Fmt.string ppf "inout")

  let pp_tprim ppf =
    Aast.(
      function
      | Tbool -> Fmt.string ppf "bool"
      | Tint -> Fmt.string ppf "int"
      | Tfloat -> Fmt.string ppf "float"
      | Tnum -> Fmt.string ppf "num"
      | Tstring -> Fmt.string ppf "string"
      | Tarraykey -> Fmt.string ppf "arraykey"
      | Tnull -> Fmt.string ppf "null"
      | Tvoid -> Fmt.string ppf "void"
      | Tresource -> Fmt.string ppf "resource"
      | Tnoreturn -> Fmt.string ppf "noreturn")

  let rec pp_hint ~is_ctx ppf (_, hint_) =
    match hint_ with
    | Aast.Hany
    | Aast.Herr ->
      Fmt.string ppf __ANY__
    | Aast.Hthis -> Fmt.string ppf "this"
    | Aast.Hdynamic -> Fmt.string ppf "dynamic"
    | Aast.Hnothing -> Fmt.string ppf "nothing"
    | Aast.Hmixed -> Fmt.string ppf "mixed"
    | Aast.Hnonnull -> Fmt.string ppf "nonnull"
    | Aast.Hvar name -> Fmt.string ppf name
    | Aast.Hfun_context name ->
      Fmt.(prefix (const string "ctx ") string) ppf name
    | Aast.Hoption hint ->
      Fmt.(prefix (const string "?") @@ pp_hint ~is_ctx:false) ppf hint
    | Aast.Hlike hint ->
      Fmt.(prefix (const string "~") @@ pp_hint ~is_ctx:false) ppf hint
    | Aast.Hsoft hint ->
      Fmt.(prefix (const string "@") @@ pp_hint ~is_ctx:false) ppf hint
    | Aast.Htuple hints ->
      Fmt.(parens @@ list ~sep:comma @@ pp_hint ~is_ctx:false) ppf hints
    | Aast.Hunion hints ->
      Fmt.(parens @@ list ~sep:vbar @@ pp_hint ~is_ctx:false) ppf hints
    | Aast.Hintersection hints when is_ctx ->
      Fmt.(brackets @@ list ~sep:comma @@ pp_hint ~is_ctx:false) ppf hints
    | Aast.Hintersection hints ->
      Fmt.(parens @@ list ~sep:amp @@ pp_hint ~is_ctx:false) ppf hints
    | Aast.Hprim prim -> pp_tprim ppf prim
    | Aast.Haccess (root, ids) ->
      Fmt.(
        pair ~sep:dbl_colon (pp_hint ~is_ctx:false)
        @@ list ~sep:dbl_colon string)
        ppf
        (root, List.map ~f:snd ids)
    | Aast.Hvarray hint ->
      Fmt.(prefix (const string "varray") @@ angles @@ pp_hint ~is_ctx:false)
        ppf
        hint
    | Aast.Hvarray_or_darray (None, vhint) ->
      Fmt.(
        prefix (const string "varray_or_darray")
        @@ angles
        @@ pp_hint ~is_ctx:false)
        ppf
        vhint
    | Aast.Hvarray_or_darray (Some khint, vhint) ->
      Fmt.(
        prefix (const string "varray_or_darray")
        @@ angles
        @@ pair ~sep:comma (pp_hint ~is_ctx:false) (pp_hint ~is_ctx:false))
        ppf
        (khint, vhint)
    | Aast.Hvec_or_dict (None, vhint) ->
      Fmt.(
        prefix (const string "vec_or_dict") @@ angles @@ pp_hint ~is_ctx:false)
        ppf
        vhint
    | Aast.Hvec_or_dict (Some khint, vhint) ->
      Fmt.(
        prefix (const string "vec_or_dict")
        @@ angles
        @@ pair ~sep:comma (pp_hint ~is_ctx:false) (pp_hint ~is_ctx:false))
        ppf
        (khint, vhint)
    | Aast.Hdarray (khint, vhint) ->
      Fmt.(
        prefix (const string "darray")
        @@ angles
        @@ pair ~sep:comma (pp_hint ~is_ctx:false) (pp_hint ~is_ctx:false))
        ppf
        (khint, vhint)
    | Aast.Habstr (name, [])
    | Aast.Happly ((_, name), []) ->
      Fmt.string ppf name
    | Aast.Habstr (name, hints)
    | Aast.Happly ((_, name), hints) ->
      Fmt.(
        prefix (const string name)
        @@ angles
        @@ list ~sep:comma
        @@ pp_hint ~is_ctx:false)
        ppf
        hints
    | Aast.(
        Hfun
          {
            hf_param_tys;
            hf_param_info;
            hf_variadic_ty;
            hf_return_ty;
            hf_ctxs;
            _;
          }) ->
      let hf_param_kinds =
        List.map hf_param_info ~f:(fun i ->
            Option.bind i (fun i -> i.Aast.hfparam_kind))
      in
      let pp_typed_param ppf kp =
        Fmt.(
          pair ~sep:nop (option @@ suffix sp pp_paramkind)
          @@ pp_hint ~is_ctx:false)
          ppf
          kp
      in
      let pp_fun_params ppf (ps, v) =
        Fmt.(
          parens
          @@ pair
               ~sep:nop
               (list ~sep:comma pp_typed_param)
               (option @@ surround ", " "..." @@ pp_hint ~is_ctx:false))
          ppf
          (ps, v)
      in
      let all_params =
        (List.zip_exn hf_param_kinds hf_param_tys, hf_variadic_ty)
      in
      Fmt.(
        parens
        @@ pair
             ~sep:colon
             ( prefix (const string "function")
             @@ pair ~sep:nop pp_fun_params (option pp_contexts) )
        @@ pp_hint ~is_ctx:false)
        ppf
        ((all_params, hf_ctxs), hf_return_ty)
    | Aast.(Hshape { nsi_allows_unknown_fields; nsi_field_map }) ->
      Fmt.(
        prefix (const string "shape")
        @@ parens
        @@ pair
             ~sep:nop
             (list ~sep:comma pp_shape_field)
             (cond ~pp_t:(const string ", ...") ~pp_f:nop))
        ppf
        (nsi_field_map, nsi_allows_unknown_fields)

  and pp_shape_field ppf Aast.{ sfi_optional; sfi_name; sfi_hint } =
    Fmt.(
      pair
        ~sep:fat_arrow
        (pair
           ~sep:nop
           (cond ~pp_t:(const string "?") ~pp_f:nop)
           pp_shape_field_name)
      @@ pp_hint ~is_ctx:false)
      ppf
      ((sfi_optional, sfi_name), sfi_hint)

  and pp_shape_field_name ppf = function
    | Ast_defs.SFlit_int (_, s) -> Fmt.string ppf s
    | Ast_defs.SFlit_str (_, s) -> Fmt.(quote string) ppf s
    | Ast_defs.SFclass_const ((_, c), (_, s)) ->
      Fmt.(pair ~sep:dbl_colon string string) ppf (c, s)

  and pp_contexts ppf (_, ctxts) =
    Fmt.(brackets @@ list ~sep:comma @@ pp_hint ~is_ctx:false) ppf ctxts

  let pp_lid ppf lid =
    Fmt.(prefix (const string "$") string) ppf @@ Local_id.get_name lid

  let rec pp_binop ppf = function
    | Ast_defs.Plus -> Fmt.string ppf "+"
    | Ast_defs.Minus -> Fmt.string ppf "-"
    | Ast_defs.Star -> Fmt.string ppf "*"
    | Ast_defs.Slash -> Fmt.string ppf "/"
    | Ast_defs.Eqeq -> Fmt.string ppf "=="
    | Ast_defs.Eqeqeq -> Fmt.string ppf "==="
    | Ast_defs.Starstar -> Fmt.string ppf "**"
    | Ast_defs.Diff -> Fmt.string ppf "diff"
    | Ast_defs.Diff2 -> Fmt.string ppf "diff2"
    | Ast_defs.Ampamp -> Fmt.string ppf "&&"
    | Ast_defs.Barbar -> Fmt.string ppf "||"
    | Ast_defs.Lt -> Fmt.string ppf "<"
    | Ast_defs.Lte -> Fmt.string ppf "<="
    | Ast_defs.Gt -> Fmt.string ppf ">"
    | Ast_defs.Gte -> Fmt.string ppf ">="
    | Ast_defs.Dot -> Fmt.string ppf "."
    | Ast_defs.Amp -> Fmt.string ppf "&"
    | Ast_defs.Bar -> Fmt.string ppf "|"
    | Ast_defs.Ltlt -> Fmt.string ppf "<<"
    | Ast_defs.Gtgt -> Fmt.string ppf ">>"
    | Ast_defs.Percent -> Fmt.string ppf "%"
    | Ast_defs.Xor -> Fmt.string ppf "^"
    | Ast_defs.Cmp -> Fmt.string ppf "<=>"
    | Ast_defs.QuestionQuestion -> Fmt.string ppf "??"
    | Ast_defs.Eq (Some op) -> Fmt.(suffix (const string "=") pp_binop) ppf op
    | Ast_defs.Eq _ -> Fmt.string ppf "="

  let pp_unop ppf op =
    match op with
    | Ast_defs.Utild -> Fmt.string ppf "~"
    | Ast_defs.Unot -> Fmt.string ppf "!"
    | Ast_defs.Uplus -> Fmt.string ppf "+"
    | Ast_defs.Uminus -> Fmt.string ppf "-"
    | Ast_defs.Uincr
    | Ast_defs.Upincr ->
      Fmt.string ppf "++"
    | Ast_defs.Udecr
    | Ast_defs.Updecr ->
      Fmt.string ppf "--"
    | Ast_defs.Usilence -> Fmt.string ppf "@"

  let is_postfix_unop = function
    | Ast_defs.Updecr
    | Ast_defs.Upincr ->
      true
    | _ -> false

  let pp_targ ppf (_, hint) = pp_hint ~is_ctx:false ppf hint

  let pp_targs ppf = function
    | [] -> ()
    | targs -> Fmt.(angles @@ list ~sep:comma pp_targ) ppf targs

  let pp_vc_kind ppf = function
    | Aast_defs.Vector -> Fmt.string ppf "Vector"
    | Aast_defs.ImmVector -> Fmt.string ppf "ImmVector"
    | Aast_defs.Vec -> Fmt.string ppf "vec"
    | Aast_defs.Set -> Fmt.string ppf "Set"
    | Aast_defs.ImmSet -> Fmt.string ppf "ImmSet"
    | Aast_defs.Keyset -> Fmt.string ppf "keyset"

  let pp_kvc_kind ppf = function
    | Aast_defs.Dict -> Fmt.string ppf "dict"
    | Aast_defs.Map -> Fmt.string ppf "Map"
    | Aast_defs.ImmMap -> Fmt.string ppf "ImmMap"

  let rec pp_expr ppf (_, expr_) = pp_expr_ ppf expr_

  and pp_expr_ ppf = function
    | Aast.Darray (kv_ty_opt, kvs) ->
      Fmt.(
        prefix (const string "darray")
        @@ pair
             ~sep:nop
             (option @@ angles @@ pair ~sep:comma pp_targ pp_targ)
             (brackets @@ list ~sep:comma @@ pair ~sep:fat_arrow pp_expr pp_expr))
        ppf
        (kv_ty_opt, kvs)
    | Aast.Varray (k_ty_opt, ks) ->
      Fmt.(
        prefix (const string "varray")
        @@ pair
             ~sep:nop
             (option @@ angles pp_targ)
             (brackets @@ list ~sep:comma pp_expr))
        ppf
        (k_ty_opt, ks)
    | Aast.Shape flds ->
      Fmt.(
        prefix (const string "shape")
        @@ parens
        @@ list ~sep:comma
        @@ pair ~sep:fat_arrow pp_shape_field_name pp_expr)
        ppf
        flds
    | Aast.ValCollection (kind, targ_opt, exprs) ->
      let delim =
        match kind with
        | Aast_defs.Keyset
        | Aast_defs.Vec ->
          Fmt.brackets
        | _ -> Fmt.braces
      in
      Fmt.(
        pair ~sep:nop pp_vc_kind
        @@ pair
             ~sep:nop
             (option @@ angles @@ pp_targ)
             (delim @@ list ~sep:comma pp_expr))
        ppf
        (kind, (targ_opt, exprs))
    | Aast.KeyValCollection (kind, targs_opt, flds) ->
      let delim =
        match kind with
        | Aast_defs.Dict -> Fmt.brackets
        | _ -> Fmt.braces
      in
      Fmt.(
        pair ~sep:nop pp_kvc_kind
        @@ pair
             ~sep:nop
             (option @@ angles @@ pair ~sep:comma pp_targ pp_targ)
             (delim @@ list ~sep:comma @@ pair ~sep:fat_arrow pp_expr pp_expr))
        ppf
        (kind, (targs_opt, flds))
    | Aast.Null -> Fmt.string ppf "null"
    | Aast.This -> Fmt.string ppf "this"
    | Aast.True -> Fmt.string ppf "true"
    | Aast.False -> Fmt.string ppf "false"
    | Aast.Id (_, id) -> Fmt.string ppf id
    | Aast.Lvar (_, lid) -> pp_lid ppf lid
    | Aast.Dollardollar _ -> Fmt.string ppf "$$"
    | Aast.Clone expr -> Fmt.(prefix (const string "clone") pp_expr) ppf expr
    | Aast.Array_get (arr_expr, idx_expr_opt) ->
      Fmt.(pair ~sep:nop pp_expr @@ brackets @@ option pp_expr)
        ppf
        (arr_expr, idx_expr_opt)
    | Aast.(Obj_get (obj_expr, get_expr, OG_nullsafe, false)) ->
      Fmt.(pair ~sep:arrow (suffix (const string "?") pp_expr) pp_expr)
        ppf
        (obj_expr, get_expr)
    | Aast.(Obj_get (obj_expr, get_expr, OG_nullsafe, _)) ->
      Fmt.(
        parens @@ pair ~sep:arrow (suffix (const string "?") pp_expr) pp_expr)
        ppf
        (obj_expr, get_expr)
    | Aast.(Obj_get (obj_expr, get_expr, _, false)) ->
      Fmt.(pair ~sep:arrow pp_expr pp_expr) ppf (obj_expr, get_expr)
    | Aast.(Obj_get (obj_expr, get_expr, _, _)) ->
      Fmt.(parens @@ pair ~sep:arrow pp_expr pp_expr) ppf (obj_expr, get_expr)
    | Aast.Class_get (class_id, class_get_expr, false) ->
      Fmt.(pair ~sep:dbl_colon pp_class_id pp_class_get_expr)
        ppf
        (class_id, class_get_expr)
    | Aast.Class_get (class_id, class_get_expr, _) ->
      Fmt.(parens @@ pair ~sep:dbl_colon pp_class_id pp_class_get_expr)
        ppf
        (class_id, class_get_expr)
    | Aast.Class_const (class_id, (_, cname)) ->
      Fmt.(pair ~sep:dbl_colon pp_class_id string) ppf (class_id, cname)
    | Aast.Call (fn, targs, exprs, expr_opt) ->
      Fmt.(pair ~sep:nop pp_expr @@ pair ~sep:nop pp_targs pp_arg_exprs)
        ppf
        (fn, (targs, (exprs, expr_opt)))
    | Aast.FunctionPointer (id, targs) ->
      Fmt.(pair ~sep:nop pp_function_ptr_id (angles @@ list ~sep:comma pp_targ))
        ppf
        (id, targs)
    | Aast.Int str
    | Aast.Float str ->
      Fmt.string ppf str
    | Aast.String str -> Fmt.(quote string) ppf str
    | Aast.String2 exprs -> Fmt.(quote @@ list ~sep:sp pp_expr) ppf exprs
    | Aast.PrefixedString (pfx, expr) ->
      Fmt.(pair ~sep:nop string @@ quote pp_expr) ppf (pfx, expr)
    | Aast.Yield afield ->
      Fmt.(prefix (const string "yield") pp_afield) ppf afield
    | Aast.Await expr -> Fmt.(prefix (const string "await") pp_expr) ppf expr
    | Aast.ReadonlyExpr expr ->
      Fmt.(prefix (const string "readonly") pp_expr) ppf expr
    | Aast.List exprs ->
      Fmt.(prefix (const string "list") @@ parens @@ list ~sep:comma pp_expr)
        ppf
        exprs
    | Aast.Tuple exprs ->
      Fmt.(prefix (const string "tuple") @@ parens @@ list ~sep:comma pp_expr)
        ppf
        exprs
    | Aast.Cast (hint, expr) ->
      Fmt.(pair ~sep:nop (parens @@ pp_hint ~is_ctx:false) pp_expr)
        ppf
        (hint, expr)
    | Aast.Unop (unop, expr) when is_postfix_unop unop ->
      Fmt.(pair ~sep:nop pp_expr pp_unop) ppf (expr, unop)
    | Aast.Unop (unop, expr) ->
      Fmt.(pair ~sep:nop pp_unop pp_expr) ppf (unop, expr)
    | Aast.Binop (op, e1, e2) ->
      Fmt.(pair ~sep:sp pp_expr @@ pair ~sep:sp pp_binop pp_expr)
        ppf
        (e1, (op, e2))
    | Aast.Pipe (_lid, e1, e2) ->
      Fmt.(pair ~sep:(const string " |> ") pp_expr pp_expr) ppf (e1, e2)
    | Aast.Eif (cond, Some texpr, fexpr) ->
      Fmt.(
        pair ~sep:(const string " ? ") pp_expr
        @@ pair ~sep:colon pp_expr pp_expr)
        ppf
        (cond, (texpr, fexpr))
    | Aast.Eif (cond, _, expr) ->
      Fmt.(pair ~sep:(const string " ?: ") pp_expr pp_expr) ppf (cond, expr)
    | Aast.Is (expr, hint) ->
      Fmt.(pair ~sep:(const string " is ") pp_expr @@ pp_hint ~is_ctx:false)
        ppf
        (expr, hint)
    | Aast.As (expr, hint, false) ->
      Fmt.(pair ~sep:(const string " as ") pp_expr @@ pp_hint ~is_ctx:false)
        ppf
        (expr, hint)
    | Aast.As (expr, hint, true) ->
      Fmt.(pair ~sep:(const string " ?as ") pp_expr @@ pp_hint ~is_ctx:false)
        ppf
        (expr, hint)
    | Aast.New (class_id, targs, exprs, expr_opt, _) ->
      Fmt.(
        prefix (const string "new")
        @@ pair ~sep:nop pp_class_id
        @@ pair ~sep:nop pp_targs pp_arg_exprs)
        ppf
        (class_id, (targs, (exprs, expr_opt)))
    | Aast.Record ((_, name), flds) ->
      Fmt.(
        pair ~sep:nop string
        @@ brackets
        @@ list ~sep:comma
        @@ pair ~sep:fat_arrow pp_expr pp_expr)
        ppf
        (name, flds)
    | Aast.Callconv (param_kind, expr) ->
      Fmt.(pair ~sep:sp pp_paramkind pp_expr) ppf (param_kind, expr)
    | Aast.Lplaceholder _ -> Fmt.string ppf "$_"
    | Aast.Fun_id (_, name) ->
      Fmt.(prefix (const string "fun") @@ quote string) ppf name
    | Aast.Method_id (expr, (_, meth)) ->
      Fmt.(
        prefix (const string "inst_meth")
        @@ parens
        @@ pair ~sep:comma pp_expr
        @@ quote string)
        ppf
        (expr, meth)
    | Aast.Pair (targs_opt, fst, snd) ->
      Fmt.(
        prefix (const string "Pair")
        @@ pair
             ~sep:nop
             (option @@ angles @@ pair ~sep:comma pp_targ pp_targ)
             (braces @@ pair ~sep:comma pp_expr pp_expr))
        ppf
        (targs_opt, (fst, snd))
    | Aast.Hole (expr, _, _, _) -> pp_expr ppf expr
    | Aast.EnumAtom name -> Fmt.(prefix (const string "#") string) ppf name
    | Aast.Efun _
    | Aast.Lfun _
    | Aast.Xml _
    | Aast.Import _
    | Aast.Collection _
    | Aast.ExpressionTree _
    | Aast.Method_caller _
    | Aast.Smethod_id _
    | Aast.ET_Splice _
    | Aast.Any
    | Aast.Omitted ->
      ()

  and pp_arg_exprs ppf (exprs, expr_opt) =
    match exprs with
    | [] ->
      Fmt.(parens @@ option @@ prefix (const string "...") pp_expr) ppf expr_opt
    | _ ->
      Fmt.(
        parens
        @@ pair
             ~sep:comma
             (list ~sep:comma pp_expr)
             (option @@ prefix (const string "...") pp_expr))
        ppf
        (exprs, expr_opt)

  and pp_afield ppf = function
    | Aast.AFvalue expr -> pp_expr ppf expr
    | Aast.AFkvalue (key_expr, val_expr) ->
      Fmt.(pair ~sep:fat_arrow pp_expr pp_expr) ppf (key_expr, val_expr)

  and pp_class_id ppf (_, class_id_) =
    match class_id_ with
    | Aast.CIparent -> Fmt.string ppf "parent"
    | Aast.CIstatic -> Fmt.string ppf "static"
    | Aast.CIself -> Fmt.string ppf "self"
    | Aast.CI (_, name) -> Fmt.string ppf name
    | Aast.CIexpr expr -> pp_expr ppf expr

  and pp_class_get_expr ppf = function
    | Aast.CGexpr expr -> pp_expr ppf expr
    | Aast.CGstring (_, name) -> Fmt.string ppf name

  and pp_function_ptr_id ppf = function
    | Aast.FP_id (_, name) -> Fmt.string ppf name
    | Aast.FP_class_const (class_id, (_, str)) ->
      Fmt.(pair ~sep:dbl_colon pp_class_id string) ppf (class_id, str)

  let pp_user_attr ppf Aast.{ ua_name = (_, nm); ua_params; _ } =
    match ua_params with
    | [] -> Fmt.string ppf nm
    | _ ->
      Fmt.(pair ~sep:nop string @@ parens @@ list ~sep:comma pp_expr)
        ppf
        (nm, ua_params)

  let pp_user_attrs ppf = function
    | [] -> ()
    | rs -> Fmt.(angles @@ angles @@ list ~sep:comma pp_user_attr) ppf rs

  let pp_variance ppf =
    Ast_defs.(
      function
      | Covariant -> Fmt.string ppf "+"
      | Contravariant -> Fmt.string ppf "-"
      | Invariant -> ())

  let pp_constraint_kind ppf =
    Ast_defs.(
      function
      | Constraint_as -> Fmt.string ppf "as"
      | Constraint_eq -> Fmt.string ppf "="
      | Constraint_super -> Fmt.string ppf "super")

  let pp_constraint ppf (kind, hint) =
    Format.(fprintf ppf {|%a %a|})
      pp_constraint_kind
      kind
      (pp_hint ~is_ctx:false)
      hint

  let pp_tp_reified ppf =
    Aast.(
      function
      | Erased -> ()
      | SoftReified
      | Reified ->
        Fmt.string ppf "reify")

  let rec pp_tparam
      ppf
      Aast.
        {
          tp_variance;
          tp_name = (_, name);
          tp_parameters;
          tp_constraints;
          tp_reified;
          tp_user_attributes;
        } =
    Format.(
      fprintf
        ppf
        {|%a %a %a%s %a %a |}
        pp_user_attrs
        tp_user_attributes
        pp_tp_reified
        tp_reified
        pp_variance
        tp_variance
        name
        pp_tparams
        tp_parameters
        Fmt.(list ~sep:sp pp_constraint)
        tp_constraints)

  and pp_tparams ppf ps =
    match List.filter ~f:Fn.(compose not is_contextual_param) ps with
    | [] -> ()
    | ps -> Fmt.(angles @@ list ~sep:comma pp_tparam) ppf ps

  let pp_type_hint ~is_ret_type ppf (_, hint) =
    if is_ret_type then
      Fmt.(option @@ prefix (const string ": ") @@ pp_hint ~is_ctx:false)
        ppf
        hint
    else
      Fmt.(option @@ suffix sp @@ pp_hint ~is_ctx:false) ppf hint

  let pp_fun_param_name ppf (param_is_variadic, param_name) =
    if param_is_variadic then
      Fmt.pf ppf {|...%s|} param_name
    else
      Fmt.string ppf param_name

  let pp_fun_param_default ppf _ = Fmt.pf ppf {| = \%s()|} __FN_MAKE_DEFAULT__

  let update_hfun_context (pos, hints) ~name =
    ( pos,
      List.map hints ~f:(function
          | (pos, Aast_defs.Hfun_context nm) when String.(nm = name) ->
            (pos, Aast_defs.Hvar "_")
          | hint -> hint) )

  let update_hint_fun_context hint_fun ~name =
    Aast_defs.
      {
        hint_fun with
        hf_ctxs =
          Option.map ~f:(update_hfun_context ~name) hint_fun.Aast_defs.hf_ctxs;
      }

  let update_fun_context (nm, hint_opt) ~name =
    let f ((pos, hint_) as hint) =
      Aast_defs.(
        match hint_ with
        | Hfun hint_fun -> (pos, Hfun (update_hint_fun_context hint_fun ~name))
        | Hoption (opt_pos, Hfun hint_fun) ->
          (pos, Hoption (opt_pos, Hfun (update_hint_fun_context hint_fun ~name)))
        (* Non-function option *)
        | Hoption _
        (* places where we _can't_ use wildcard contexts *)
        | Hlike _
        | Hsoft _
        | Hshape _
        | Htuple _
        (* places where function types shouldn't appear *)
        | Haccess _
        | Happly _
        (* non-denotable types *)
        | Hany
        | Herr
        | Hmixed
        | Hnonnull
        | Habstr _
        | Hdarray _
        | Hvarray _
        | Hvarray_or_darray _
        | Hvec_or_dict _
        | Hprim _
        | Hthis
        | Hdynamic
        | Hnothing
        | Hunion _
        | Hintersection _
        | Hfun_context _
        | Hvar _ ->
          hint)
    in
    (nm, Option.map hint_opt ~f)

  let pp_fun_param
      ppf
      Aast.
        {
          param_type_hint;
          param_is_variadic;
          param_name;
          param_expr;
          param_callconv;
          param_user_attributes;
          param_visibility;
          _;
        } =
    Format.(
      fprintf
        ppf
        {|%a %a %a %a%a%a|}
        pp_user_attrs
        param_user_attributes
        Fmt.(option pp_visibility)
        param_visibility
        Fmt.(option pp_paramkind)
        param_callconv
        (pp_type_hint ~is_ret_type:false)
        (* Type hint for parameter $f used for contextful function must be a
        function type hint whose context is exactly [_] *)
        (update_fun_context ~name:param_name param_type_hint)
        pp_fun_param_name
        (param_is_variadic, param_name)
        Fmt.(option pp_fun_param_default)
        param_expr)

  let pp_variadic_fun_param ppf = function
    | Aast.FVvariadicArg fp -> Fmt.pf ppf {|%a|} pp_fun_param fp
    | Aast.FVellipsis _ -> Fmt.string ppf "..."
    | Aast.FVnonVariadic -> ()

  let pp_fun_params ppf = function
    | ([], variadic) -> Fmt.(parens pp_variadic_fun_param) ppf variadic
    | tup ->
      Fmt.(
        parens
        @@ pair ~sep:comma (list ~sep:comma pp_fun_param) pp_variadic_fun_param)
        ppf
        tup

  (* -- Single element depdencies ------------------------------------------- *)

  module Single : sig
    type t

    val name : t -> string

    val line : t -> int

    val mk_enum : Provider_context.t -> Nast.class_ -> t

    val mk_gconst : Provider_context.t -> Nast.gconst -> t

    val mk_gfun : Nast.fun_ -> t

    val mk_tydef : Nast.typedef -> t

    val mk_target_fun : Provider_context.t -> string * Cmd.target -> t

    val pp : Format.formatter -> t -> unit
  end = struct
    type t =
      | SEnum of {
          name: string;
          line: int;
          base: Aast.hint;
          constr: Aast.hint option;
          consts: (string * string) list;
          user_attrs: Nast.user_attribute list;
        }
      | SGConst of {
          name: string;
          line: int;
          type_: Aast.hint;
          init_val: string;
        }
      | STydef of string * int * int list * Nast.typedef
      | SGFun of string * int * Nast.fun_
      | SGTargetFun of string * int * SourceText.t

    let mk_target_fun ctx (fun_name, target) =
      SGTargetFun
        ( fun_name,
          Pos.line @@ Target.pos ctx target,
          Target.source_text ctx target )

    let mk_enum
        ctx
        Aast.{ c_name = (pos, name); c_enum; c_consts; c_user_attributes; _ } =
      let Aast.{ e_base; e_constraint; _ } =
        value_or_not_found Format.(sprintf "expected an enum class: %s" name)
        @@ c_enum
      in
      let const_val = init_value ctx e_base in
      let consts =
        List.map c_consts ~f:(fun Aast.{ cc_id = (_, const_name); _ } ->
            (const_name, const_val))
      in
      SEnum
        {
          name;
          line = Pos.line pos;
          consts;
          base = e_base;
          constr = e_constraint;
          user_attrs = c_user_attributes;
        }

    let mk_gconst ctx Aast.{ cst_name = (pos, name); cst_type; _ } =
      let type_ = value_or_not_found ("type of " ^ name) cst_type in
      let init_val = init_value ctx type_ in
      SGConst { name; line = Pos.line pos; type_; init_val }

    let mk_gfun (Aast.{ f_name = (pos, name); _ } as ast) =
      SGFun (name, Pos.line pos, ast)

    let mk_tydef (Aast.{ t_name = (pos, name); _ } as ast) =
      let fixmes =
        ISet.elements @@ Fixme_provider.get_fixme_codes_for_pos pos
      in
      STydef (name, Pos.line pos, fixmes, ast)

    let name = function
      | SEnum { name; _ }
      | SGConst { name; _ }
      | STydef (name, _, _, _)
      | SGFun (name, _, _)
      | SGTargetFun (name, _, _) ->
        name

    let line = function
      | SEnum { line; _ }
      | SGConst { line; _ }
      | STydef (_, line, _, _)
      | SGFun (_, line, _)
      | SGTargetFun (_, line, _) ->
        line

    (* == Pretty print ====================================================== *)

    (* -- Global functions -------------------------------------------------- *)

    let pp_fun
        ppf
        ( name,
          Aast.
            {
              f_user_attributes;
              f_tparams;
              f_variadic;
              f_params;
              f_ret;
              f_ctxs;
              _;
            } ) =
      Fmt.(
        pf
          ppf
          "%a function %s%a%a%a%a {throw new \\Exception();}"
          pp_user_attrs
          f_user_attributes
          (strip_ns name)
          pp_tparams
          f_tparams
          pp_fun_params
          (f_params, f_variadic)
          (option pp_contexts)
          f_ctxs
          (pp_type_hint ~is_ret_type:true)
          f_ret)

    (* -- Type definitions -------------------------------------------------- *)

    let pp_typedef_visiblity ppf = function
      | Aast_defs.Transparent -> Fmt.string ppf "type"
      | Aast_defs.Opaque -> Fmt.string ppf "newtype"

    let pp_fixme ppf code = Fmt.pf ppf "/* HH_FIXME[%d] */@." code

    let pp_tydef
        ppf
        ( nm,
          fixmes,
          Aast.{ t_tparams; t_constraint; t_vis; t_kind; t_user_attributes; _ }
        ) =
      Fmt.(
        pf
          ppf
          {|%a%a%a %s%a%a = %a;|}
          (list ~sep:cut pp_fixme)
          fixmes
          pp_user_attrs
          t_user_attributes
          pp_typedef_visiblity
          t_vis
          (strip_ns nm)
          pp_tparams
          t_tparams
          (option @@ prefix (const string " as ") @@ pp_hint ~is_ctx:false)
          t_constraint
          (pp_hint ~is_ctx:false)
          t_kind)

    (* -- Global constants -------------------------------------------------- *)

    let pp_gconst ppf (name, hint, init) =
      Fmt.pf
        ppf
        {|const %a %s = %s;|}
        (pp_hint ~is_ctx:false)
        hint
        (strip_ns name)
        init

    (* -- Enums ------------------------------------------------------------- *)

    let pp_enum ppf (name, base, constr, consts, user_attrs) =
      Fmt.(
        pf
          ppf
          {|%a enum %s: %a%a {%a}|}
          pp_user_attrs
          user_attrs
          (strip_ns name)
          (pp_hint ~is_ctx:false)
          base
          (option @@ prefix (const string " as ") @@ pp_hint ~is_ctx:false)
          constr
          (list ~sep:sp @@ suffix semicolon @@ pair ~sep:equal_to string string)
          consts)

    let pp ppf = function
      | SEnum { name; base; constr; consts; user_attrs; _ } ->
        pp_enum ppf (name, base, constr, consts, user_attrs)
      | STydef (nm, _, fixmes, ast) -> pp_tydef ppf (nm, fixmes, ast)
      | SGConst { name; type_; init_val; _ } ->
        pp_gconst ppf (name, type_, init_val)
      | SGFun (nm, _, ast) -> pp_fun ppf (nm, ast)
      | SGTargetFun (_, _, source) -> Fmt.string ppf @@ SourceText.text source
  end

  (* -- Dependencies contained within a class dependency -------------------- *)

  module Class_elt : sig
    type t

    val mk_const : Provider_context.t -> Nast.class_const -> t

    val mk_tyconst :
      (Pos.t, Nast.func_body_ann, unit, unit) Aast.class_typeconst_def -> t

    val mk_method : bool -> Nast.method_ -> t

    val mk_target_method : Provider_context.t -> Cmd.target -> t

    val mk_prop :
      Provider_context.t ->
      (Pos.t, Nast.func_body_ann, unit, unit) Aast.class_var ->
      t

    val compare : t -> t -> int

    val pp : Format.formatter -> t -> unit
  end = struct
    type t =
      | EltConst of {
          name: string;
          line: int;
          is_abstract: bool;
          type_: Aast.hint option;
          init_val: string option;
        }
      | EltMethod of bool * int * Nast.method_
      | EltTypeConst of {
          name: string;
          line: int;
          is_abstract: bool;
          is_ctx: bool;
          type_: Aast.hint option;
          constraint_: Aast.hint option;
          user_attrs: Nast.user_attribute list;
        }
      | EltProp of {
          name: string;
          line: int;
          is_static: bool;
          visibility: Aast.visibility;
          user_attrs: Nast.user_attribute list;
          type_: Aast.hint option;
          init_val: string option;
        }
      | EltXHPAttr of {
          name: string;
          line: int;
          user_attrs: Nast.user_attribute list;
          type_: Aast.hint option;
          init_val: string option;
          xhp_attr_info: Aast.xhp_attr_info;
        }
      | EltTargetMethod of int * SourceText.t

    let line = function
      | EltConst { line; _ }
      | EltMethod (_, line, _)
      | EltTypeConst { line; _ }
      | EltProp { line; _ }
      | EltXHPAttr { line; _ }
      | EltTargetMethod (line, _) ->
        line

    let compare elt1 elt2 = Int.compare (line elt2) (line elt1)

    let mk_target_method ctx tgt =
      let pos = Target.pos ctx tgt in
      EltTargetMethod (Pos.line pos, Target.source_text ctx tgt)

    let mk_const ctx Aast.{ cc_id = (pos, name); cc_expr; cc_type; _ } =
      let is_abstract =
        Option.value_map ~default:true ~f:Fn.(const false) cc_expr
      in
      let (type_, init_val) =
        match (cc_type, cc_expr) with
        | (Some hint, _) -> (Some hint, Some (init_value ctx hint))
        | (_, Some (e_pos, e_)) ->
          (match Decl_utils.infer_const e_ with
          | Some tprim ->
            let hint = (e_pos, Aast.Hprim tprim) in
            (None, Some (init_value ctx hint))
          | None -> raise Unsupported)
        | (None, None) -> (None, None)
      in
      let line = Pos.line pos in
      EltConst { name; line; is_abstract; type_; init_val }

    let mk_tyconst
        Aast.
          {
            c_tconst_kind;
            c_tconst_name = (pos, name);
            c_tconst_is_ctx;
            c_tconst_user_attributes;
            _;
          } =
      let (is_abstract, type_, constraint_) =
        Aast.(
          match c_tconst_kind with
          | TCAbstract { c_atc_as_constraint = c; _ } -> (true, None, c)
          | TCPartiallyAbstract { c_patc_constraint = c; c_patc_type = t } ->
            (false, Some t, Some c)
          | TCConcrete { c_tc_type = t } -> (false, Some t, None))
      in
      EltTypeConst
        {
          name;
          line = Pos.line pos;
          is_abstract;
          type_;
          constraint_;
          is_ctx = c_tconst_is_ctx;
          user_attrs = c_tconst_user_attributes;
        }

    let mk_method from_interface (Aast.{ m_name = (pos, _); _ } as method_) =
      EltMethod (from_interface, Pos.line pos, method_)

    let mk_prop
        ctx
        Aast.
          {
            cv_id = (pos, name);
            cv_user_attributes;
            cv_type;
            cv_expr;
            cv_xhp_attr;
            cv_visibility;
            cv_is_static;
            _;
          } =
      let (type_, init_val) =
        match (Aast.hint_of_type_hint cv_type, cv_expr) with
        | (Some hint, Some _) -> (Some hint, Some (init_value ctx hint))
        | (Some hint, None) -> (Some hint, None)
        | (None, None) -> (None, None)
        (* Untyped prop, not supported for now *)
        | (None, Some _) -> raise Unsupported
      in
      match cv_xhp_attr with
      | Some xhp_attr_info ->
        EltXHPAttr
          {
            name;
            line = Pos.line pos;
            user_attrs = cv_user_attributes;
            type_;
            init_val;
            xhp_attr_info;
          }
      | _ ->
        EltProp
          {
            name;
            line = Pos.line pos;
            is_static = cv_is_static;
            visibility = cv_visibility;
            user_attrs = cv_user_attributes;
            type_;
            init_val;
          }

    (* == Pretty printers =================================================== *)

    (* -- Methods ----------------------------------------------------------- *)

    let pp_method
        ppf
        ( from_interface,
          Aast.
            {
              m_name = (_, name);
              m_tparams;
              m_params;
              m_variadic;
              m_ctxs;
              m_ret;
              m_static;
              m_abstract;
              m_final;
              m_visibility;
              m_user_attributes;
              _;
            } ) =
      Fmt.(
        pf
          ppf
          "%a %a %a %a %a function %s%a%a%a%a%a"
          pp_user_attrs
          m_user_attributes
          (cond ~pp_t:(const string "abstract") ~pp_f:nop)
          (m_abstract && not from_interface)
          (cond ~pp_t:(const string "final") ~pp_f:nop)
          m_final
          pp_visibility
          m_visibility
          (cond ~pp_t:(const string "static") ~pp_f:nop)
          m_static
          name
          pp_tparams
          m_tparams
          pp_fun_params
          (m_params, m_variadic)
          (option pp_contexts)
          m_ctxs
          (pp_type_hint ~is_ret_type:true)
          m_ret
          (cond
             ~pp_t:(const string ";")
             ~pp_f:(const string "{throw new \\Exception();}"))
          (m_abstract || from_interface))

    (* -- Properties -------------------------------------------------------- *)
    let pp_xhp_attr_tag ppf = function
      | Aast.Required -> Fmt.string ppf "@required"
      | Aast.LateInit -> Fmt.string ppf "@lateinit"

    let pp_xhp_attr_info ppf Aast.{ xai_tag; _ } =
      Fmt.(option pp_xhp_attr_tag) ppf xai_tag

    let pp_xhp_attr ppf (name, user_attrs, type_, init_val, xhp_attr_info) =
      Fmt.(
        pf
          ppf
          {|%a attribute %a %s %a %a;|}
          pp_user_attrs
          user_attrs
          (option @@ pp_hint ~is_ctx:false)
          type_
          (String.lstrip ~drop:(fun c -> Char.equal c ':') name)
          (option @@ prefix equal_to string)
          init_val
          pp_xhp_attr_info
          xhp_attr_info)

    let pp_prop ppf (name, is_static, visibility, user_attrs, type_, init_val) =
      Fmt.(
        pf
          ppf
          {|%a %a %a %a $%s%a;|}
          pp_user_attrs
          user_attrs
          pp_visibility
          visibility
          (cond ~pp_t:(const string "static") ~pp_f:nop)
          is_static
          (option @@ pp_hint ~is_ctx:false)
          type_
          name
          (option @@ prefix equal_to string)
          init_val)

    (* -- Type constants ---------------------------------------------------- *)
    let pp_tyconst
        ppf (name, is_abstract, is_ctx, type_, constraint_, user_attrs) =
      Fmt.(
        pf
          ppf
          {|%a %a const %a %s%a%a;|}
          pp_user_attrs
          user_attrs
          (cond ~pp_t:(const string "abstract") ~pp_f:nop)
          is_abstract
          (cond ~pp_t:(const string "ctx") ~pp_f:(const string "type"))
          is_ctx
          name
          (option @@ prefix (const string " as ") @@ pp_hint ~is_ctx:false)
          constraint_
          (option @@ prefix equal_to @@ pp_hint ~is_ctx)
          type_)

    (* -- Constants --------------------------------------------------------- *)

    let pp_const ppf (name, is_abstract, type_, init_val) =
      Fmt.(
        pf
          ppf
          {|%a const %a %s %a;|}
          (cond ~pp_t:(const string "abstract") ~pp_f:nop)
          is_abstract
          (option @@ pp_hint ~is_ctx:false)
          type_
          name
          (option @@ prefix equal_to string)
          init_val)

    (* -- Top level --------------------------------------------------------- *)
    let pp ppf = function
      | EltMethod (from_interface, _, method_) ->
        pp_method ppf (from_interface, method_)
      | EltTargetMethod (_, src) -> Fmt.string ppf @@ SourceText.text src
      | EltTypeConst
          { name; is_abstract; is_ctx; type_; constraint_; user_attrs; _ } ->
        pp_tyconst
          ppf
          (name, is_abstract, is_ctx, type_, constraint_, user_attrs)
      | EltConst { name; is_abstract; type_; init_val; _ } ->
        pp_const ppf (name, is_abstract, type_, init_val)
      | EltXHPAttr { name; user_attrs; type_; init_val; xhp_attr_info; _ } ->
        pp_xhp_attr ppf (name, user_attrs, type_, init_val, xhp_attr_info)
      | EltProp { name; is_static; visibility; user_attrs; type_; init_val; _ }
        ->
        pp_prop ppf (name, is_static, visibility, user_attrs, type_, init_val)
  end

  module Class_decl : sig
    type t

    val name : t -> string

    val line : t -> int

    val of_class : Nast.class_ -> t

    val pp : Format.formatter -> t -> unit
  end = struct
    type t = {
      name: string;
      line: int;
      user_attrs: Nast.user_attribute list;
      is_final: bool;
      is_xhp: bool;
      kind: Ast_defs.class_kind;
      tparams: Nast.tparam list;
      extends: Aast.class_hint list;
      implements: Aast.class_hint list;
    }

    let name { name; _ } = name

    let line { line; _ } = line

    let of_class
        Aast.
          {
            c_name = (pos, name);
            c_user_attributes;
            c_is_xhp;
            c_final;
            c_kind;
            c_tparams;
            c_extends;
            c_implements;
            _;
          } =
      {
        name;
        line = Pos.line pos;
        user_attrs = c_user_attributes;
        is_final = c_final;
        is_xhp = c_is_xhp;
        kind = c_kind;
        tparams = c_tparams;
        extends = c_extends;
        implements = c_implements;
      }

    let pp_class_kind ppf =
      Ast_defs.(
        function
        | Cabstract -> Fmt.string ppf "abstract class"
        | Cnormal -> Fmt.string ppf "class"
        | Cinterface -> Fmt.string ppf "interface"
        | Ctrait -> Fmt.string ppf "trait"
        | Cenum -> Fmt.string ppf "enum")

    let pp_class_ancestors class_or_intf ppf = function
      | [] ->
        (* because we are prefixing the list with a constant string, we
           match on the empty list here and noop to avoid priting "extends "
           ("implements ") with no class (interface) *)
        ()
      | hints ->
        let pfx =
          match class_or_intf with
          | `Class -> Fmt.(const string "extends ")
          | `Interface -> Fmt.(const string "implements ")
        in
        Fmt.(prefix pfx @@ list ~sep:comma @@ pp_hint ~is_ctx:false) ppf hints

    let pp
        ppf
        { name; user_attrs; is_final; kind; tparams; extends; implements; _ } =
      Fmt.(
        pf
          ppf
          {|%a %a %a %s%a %a %a|}
          pp_user_attrs
          user_attrs
          (cond ~pp_t:(const string "final") ~pp_f:nop)
          is_final
          pp_class_kind
          kind
          (strip_ns name)
          pp_tparams
          tparams
          (pp_class_ancestors `Class)
          extends
          (pp_class_ancestors `Interface)
          implements)
  end

  module Class_body : sig
    type t

    val of_class : Nast.class_ -> Class_elt.t list -> t

    val pp : Format.formatter -> t -> unit
  end = struct
    type t = {
      uses: Aast.trait_hint list;
      req_extends: Aast.class_hint list;
      req_implements: Aast.class_hint list;
      elements: Class_elt.t list;
    }

    let of_class Aast.{ c_uses; c_reqs; _ } class_elts =
      let (req_extends, req_implements) =
        List.partition_map c_reqs ~f:(fun (s, extends) ->
            if extends then
              `Fst s
            else
              `Snd s)
      in

      {
        uses = c_uses;
        req_extends;
        req_implements;
        elements = List.sort ~compare:Class_elt.compare class_elts;
      }

    let pp_use_extend ppf = function
      | ([], [], []) ->
        (* In the presence of any traits or trait/interface requirements
           we add a cut after them to match the Hack style given in the docs
           (line breaks are not added by HackFmt). We need to check that
           we have no traits or requirements here else we end up with
           an unnecessary line break in this case *)
        ()
      | (uses, req_extends, req_implements) ->
        let pp_elem pfx ppf hint =
          Fmt.(prefix pfx @@ suffix semicolon @@ pp_hint ~is_ctx:false) ppf hint
        in
        Fmt.(
          suffix cut
          @@ pair ~sep:nop (list @@ pp_elem @@ const string "use ")
          @@ pair
               ~sep:nop
               (list @@ pp_elem @@ const string "require extends ")
               (list @@ pp_elem @@ const string "require implements "))
          ppf
          (uses, (req_extends, req_implements))

    let pp_elts ppf = function
      | [] -> ()
      | elts -> Fmt.(list ~sep:cut @@ prefix cut Class_elt.pp) ppf elts

    let pp ppf { uses; req_extends; req_implements; elements } =
      Fmt.(pair ~sep:nop pp_use_extend pp_elts)
        ppf
        ((uses, req_extends, req_implements), elements)
  end

  type t =
    | ClsGroup of {
        decl: Class_decl.t;
        body: Class_body.t;
      }
    | Single of Single.t

  let name = function
    | ClsGroup { decl; _ } -> Class_decl.name decl
    | Single single -> Single.name single

  let line = function
    | ClsGroup { decl; _ } -> Class_decl.line decl
    | Single single -> Single.line single

  let compare g1 g2 = Int.compare (line g1) (line g2)

  let of_class (ast, class_elts) =
    ClsGroup
      {
        decl = Class_decl.of_class ast;
        body = Class_body.of_class ast class_elts;
      }

  let pp ppf = function
    | ClsGroup { decl; body; _ } ->
      Fmt.(pair Class_decl.pp @@ braces @@ vbox Class_body.pp) ppf (decl, body)
    | Single single -> Single.pp ppf single

  (* -- Classify and group extracted dependencies --------------------------- *)
  type dep_part =
    | PartClsElt of string * Class_elt.t
    | PartSingle of Single.t
    | PartCls of string
    | PartIgnore

  let of_method ctx cls_name method_name =
    let is_interface = Nast_helper.is_interface ctx cls_name in
    Option.value_map ~default:PartIgnore ~f:(fun ast ->
        PartClsElt (cls_name, Class_elt.(mk_method is_interface ast)))
    @@ Nast_helper.get_method ctx cls_name method_name

  let of_dep ctx dep =
    Typing_deps.Dep.(
      match dep with
      (* -- Class elements -- *)
      | Const (cls_nm, nm)
        when String.(cls_nm = Dep.get_origin ctx cls_nm dep && nm <> "class")
             && (not @@ Nast_helper.is_enum ctx cls_nm) ->
        PartClsElt
          ( cls_nm,
            value_or_not_found (cls_nm ^ "::" ^ nm)
            @@ Option.(
                 first_some
                   ( map ~f:Class_elt.mk_tyconst
                   @@ Nast_helper.get_typeconst ctx cls_nm nm )
                   ( map ~f:Class_elt.(mk_const ctx)
                   @@ Nast_helper.get_const ctx cls_nm nm )) )
      | Method (cls_nm, nm) when String.(cls_nm = Dep.get_origin ctx cls_nm dep)
        ->
        of_method ctx cls_nm nm
      | SMethod (cls_nm, nm)
        when String.(cls_nm = Dep.get_origin ctx cls_nm dep) ->
        of_method ctx cls_nm nm
      | Cstr cls_nm when String.(cls_nm = Dep.get_origin ctx cls_nm dep) ->
        let nm = "__construct" in
        of_method ctx cls_nm nm
      | Prop (cls_nm, nm) when String.(cls_nm = Dep.get_origin ctx cls_nm dep)
        ->
        PartClsElt
          ( cls_nm,
            value_or_not_found (cls_nm ^ "::" ^ nm)
            @@ Option.map ~f:Class_elt.(mk_prop ctx)
            @@ Nast_helper.get_prop ctx cls_nm nm )
      | SProp (cls_nm, snm) when String.(cls_nm = Dep.get_origin ctx cls_nm dep)
        ->
        let nm = String.lstrip ~drop:(fun c -> Char.equal c '$') snm in
        PartClsElt
          ( cls_nm,
            value_or_not_found (cls_nm ^ "::" ^ nm)
            @@ Option.map ~f:Class_elt.(mk_prop ctx)
            @@ Nast_helper.get_prop ctx cls_nm nm )
      (* -- Globals -- *)
      | Fun nm
      | FunName nm ->
        PartSingle (Single.mk_gfun @@ Nast_helper.get_fun_exn ctx nm)
      | GConst nm
      | GConstName nm ->
        PartSingle (Single.mk_gconst ctx @@ Nast_helper.get_gconst_exn ctx nm)
      (* -- Type defs, Enums, Classes, RecordDefs -- *)
      | Type nm when Nast_helper.is_tydef ctx nm ->
        PartSingle (Single.mk_tydef @@ Nast_helper.get_typedef_exn ctx nm)
      | Type nm when Nast_helper.is_enum ctx nm ->
        PartSingle (Single.mk_enum ctx @@ Nast_helper.get_class_exn ctx nm)
      | Type nm when Nast_helper.is_class ctx nm -> PartCls nm
      | Type _ -> raise Unsupported
      (* -- Ignore -- *)
      | Const _
      | Method _
      | SMethod _
      | Cstr _
      | Prop _
      | SProp _
      | AllMembers _
      | Extends _ ->
        PartIgnore)

  let of_deps ctx target_opt deps =
    let insert_elt (sgls, grps) (cls_nm, cls_elt) =
      ( sgls,
        SMap.update
          cls_nm
          (function
            | Some (cls_ast, cls_elts) -> Some (cls_ast, cls_elt :: cls_elts)
            | _ ->
              let cls_ast = Nast_helper.get_class_exn ctx cls_nm in
              Some (cls_ast, [cls_elt]))
          grps )
    and insert_cls (sgls, grps) cls_nm =
      ( sgls,
        SMap.update
          cls_nm
          (function
            | Some _ as data -> data
            | _ ->
              let cls_ast = Nast_helper.get_class_exn ctx cls_nm in
              Some (cls_ast, []))
          grps )
    and insert_single (sgls, grps) single = (Single single :: sgls, grps)
    and insert_target (sgls, grps) =
      match target_opt with
      | Some (Cmd.Function fun_name as tgt) ->
        let sgl = Single.mk_target_fun ctx (fun_name, tgt) in
        (Single sgl :: sgls, grps)
      | Some (Cmd.Method (cls_name, _) as tgt) ->
        ( sgls,
          SMap.update
            cls_name
            (function
              | Some (cls_ast, elts) ->
                let elt = Class_elt.mk_target_method ctx tgt in
                Some (cls_ast, elt :: elts)
              | _ ->
                let cls_ast = Nast_helper.get_class_exn ctx cls_name
                and elt = Class_elt.mk_target_method ctx tgt in
                Some (cls_ast, [elt]))
            grps )
      | None -> (sgls, grps)
    in

    let (sgls, clss) =
      insert_target
      @@ List.fold_left deps ~init:([], SMap.empty) ~f:(fun acc dep ->
             match of_dep ctx dep with
             | PartCls cls_nm -> insert_cls acc cls_nm
             | PartClsElt (cls_nm, elt) -> insert_elt acc (cls_nm, elt)
             | PartSingle single -> insert_single acc single
             | _ -> acc)
    in

    List.append sgls @@ List.map ~f:of_class @@ SMap.values clss
end

(* -- Dependencies nested into namespaces ----------------------------------- *)
module Namespaced : sig
  type t

  val unfold : Grouped.t list -> t

  val pp : Format.formatter -> t -> unit
end = struct
  type t = {
    line: int option;
    deps: Grouped.t list;
    children: (string * t) list;
  }

  let rec pp_ ppf { deps; children; _ } =
    (* We guard against empty list of depedencies or nested namespaces here
       since we should add a cut between them only when both are non-empty
    *)
    match (deps, children) with
    | (_, []) -> Fmt.(list ~sep:cut @@ suffix cut Grouped.pp) ppf deps
    | ([], _) -> Fmt.(list ~sep:cut pp_children) ppf children
    | _ ->
      Fmt.(
        pair
          ~sep:cut
          (list ~sep:cut @@ suffix cut Grouped.pp)
          (list ~sep:cut pp_children))
        ppf
        (deps, children)

  and pp_children ppf (nm, t) =
    Fmt.(pair (prefix (const string "namespace ") string) (braces pp_))
      ppf
      (nm, t)

  let pp ppf t = Fmt.(vbox @@ prefix cut pp_) ppf t

  let namespace_of dep =
    Grouped.name dep
    |> String.rsplit2_exn ~on:'\\'
    |> fst
    |> String.strip ~drop:(Char.equal '\\')
    |> String.split ~on:'\\'
    |> List.filter ~f:String.((fun s -> not @@ is_empty s))

  let group_by_prefix deps =
    Tuple2.map_snd ~f:SMap.elements
    @@ List.fold
         ~init:([], SMap.empty)
         ~f:(fun (toplvl, nested) -> function
           | ([], dep) -> (dep :: toplvl, nested)
           | (ns :: nss, dep) ->
             ( toplvl,
               SMap.update
                 ns
                 (function
                   | Some deps -> Some ((nss, dep) :: deps)
                   | _ -> Some [(nss, dep)])
                 nested ))
         deps

  let unfold deps =
    let compare_line (_, { line = ln1; _ }) (_, { line = ln2; _ }) =
      Option.compare Int.compare ln1 ln2
    in
    let rec aux ~k b =
      let (unsorted, dss) = group_by_prefix b in
      let deps = List.sort ~compare:Grouped.compare unsorted in
      let line = Option.map ~f:Grouped.line @@ List.hd deps in
      auxs dss ~k:(fun unsorted_cs ->
          let children = List.sort ~compare:compare_line unsorted_cs in
          k { deps; line; children })
    and auxs ~k = function
      | [] -> k []
      | (ns, next) :: rest ->
        auxs rest ~k:(fun rest' ->
            aux next ~k:(fun next' -> k @@ ((ns, next') :: rest')))
    in
    aux ~k:Fn.id @@ List.map ~f:(fun dep -> (namespace_of dep, dep)) deps
end

(* -- Hack format wrapper --------------------------------------------------- *)
module Hackfmt : sig
  val format : string -> string
end = struct
  let print_error source_text error =
    let text =
      SyntaxError.to_positioned_string
        error
        (SourceText.offset_to_position source_text)
    in
    Hh_logger.log "%s\n" text

  let tree_from_string s =
    let source_text = SourceText.make Relative_path.default s in
    let mode = Full_fidelity_parser.parse_mode source_text in
    let env = Full_fidelity_parser_env.make ?mode () in
    let tree = SyntaxTree.make ~env source_text in

    if List.is_empty (SyntaxTree.all_errors tree) then
      tree
    else (
      List.iter (SyntaxTree.all_errors tree) (print_error source_text);
      raise Hackfmt_error.InvalidSyntax
    )

  let tidy_xhp =
    let re = Str.regexp "\\\\:" in
    Str.global_replace re ":"

  let format text =
    try Libhackfmt.format_tree @@ tree_from_string @@ tidy_xhp text
    with Hackfmt_error.InvalidSyntax -> text
end

(* -- Per-file grouped dependencies ----------------------------------------- *)
module File : sig
  type t

  val compare : t -> t -> int

  val of_deps :
    Provider_context.t ->
    Relative_path.t * Cmd.target ->
    Relative_path.t option
    * Typing_deps.Dep.dependency Typing_deps.Dep.variant list ->
    t

  val pp : is_multifile:bool -> Format.formatter -> t -> unit
end = struct
  type t = {
    is_target: bool;
    name: string;
    mode: FileInfo.mode;
    path: Relative_path.t option;
    content: Namespaced.t;
  }

  let compare
      { is_target = tgt1; name = nm1; _ } { is_target = tgt2; name = nm2; _ } =
    if tgt1 && tgt2 then
      0
    else if tgt1 then
      -1
    else if tgt2 then
      1
    else
      String.compare nm2 nm1

  let of_deps ctx (target_path, target) (path, deps) =
    let is_target =
      Option.value_map ~default:false ~f:(Relative_path.equal target_path) path
    in
    {
      is_target;
      name =
        Option.value_map ~default:__UNKNOWN_FILE__ ~f:Relative_path.suffix path;
      mode =
        Option.(
          List.hd deps >>= Dep.get_mode ctx |> value ~default:FileInfo.Mstrict);
      path;
      content =
        Namespaced.unfold
        @@ Grouped.of_deps
             ctx
             ( if is_target then
               Some target
             else
               None )
             deps;
    }

  let pp_header ppf name =
    Fmt.(hbox @@ pair ~sep:sp string string) ppf (__FILE_PREFIX__, name)

  let pp_mode ppf = function
    | FileInfo.Mpartial ->
      Fmt.(hbox @@ pair ~sep:sp string string)
        ppf
        (__HH_HEADER_PREFIX__, __HH_HEADER_SUFFIX_PARTIAL__)
    | _ -> Fmt.string ppf __HH_HEADER_PREFIX__

  let to_string ~is_multifile { mode; content; name; _ } =
    let body = Hackfmt.format @@ Fmt.to_to_string Namespaced.pp content
    and mode = Fmt.to_to_string pp_mode mode in
    if is_multifile then
      let header = Fmt.to_to_string pp_header name in
      Format.sprintf "%s\n%s\n%s" header mode body
    else
      Format.sprintf "%s\n%s" mode body

  let pp ~is_multifile = Fmt.of_to_string (to_string ~is_multifile)
end

(* -- All releveant dependencies organized by file -------------------------- *)
module Standalone : sig
  type t

  val generate :
    Provider_context.t ->
    Cmd.target ->
    bool * bool * Typing_deps.Dep.dependency Typing_deps.Dep.variant list ->
    t

  val to_string : t -> string
end = struct
  type t = {
    dep_default: bool;
    dep_any: bool;
    files: File.t list;
  }

  let generate ctx target (dep_default, dep_any, deps) =
    let target_path = Target.relative_path ctx target in
    let files =
      List.sort ~compare:File.compare
      @@ List.map ~f:File.(of_deps ctx (target_path, target))
      @@ SMap.values
      @@ SMap.update (Relative_path.to_absolute target_path) (function
             | Some _ as data -> data
             | _ -> Some (Some target_path, []))
      @@ List.fold_left deps ~init:SMap.empty ~f:(fun acc dep ->
             let rel_path = Dep.get_relative_path ctx dep in
             let key =
               Option.value_map
                 ~default:"unknown"
                 ~f:Relative_path.to_absolute
                 rel_path
             in
             SMap.update
               key
               (function
                 | Some (path, deps) -> Some (path, dep :: deps)
                 | _ -> Some (rel_path, [dep]))
               acc)
    in
    { dep_default; dep_any; files }

  let __DEPS_ON_DEFAULT__ =
    Format.sprintf
      {|@.function %s()[]: nothing {@.  throw new \Exception();@.}|}
      __FN_MAKE_DEFAULT__

  let __DEPS_ON_ANY__ =
    Format.sprintf
      {|/* HH_FIXME[4101] */@.type %s = \%s_;@.type %s_<T> = T;|}
      __ANY__
      __ANY__
      __ANY__

  let pp_helpers ppf (deps_on_default, deps_on_any) =
    if deps_on_default && deps_on_any then
      Fmt.pf
        ppf
        {|%s %s@.%s@.%s@.%s@.|}
        __FILE_PREFIX__
        __DUMMY_FILE__
        __HH_HEADER_PREFIX__
        __DEPS_ON_DEFAULT__
        __DEPS_ON_ANY__
    else if deps_on_default then
      Fmt.pf
        ppf
        {|%s %s@.%s@.%s@.|}
        __FILE_PREFIX__
        __DUMMY_FILE__
        __HH_HEADER_PREFIX__
        __DEPS_ON_DEFAULT__
    else if deps_on_any then
      Fmt.pf
        ppf
        {|%s %s@.%s@.%s@.|}
        __FILE_PREFIX__
        __DUMMY_FILE__
        __HH_HEADER_PREFIX__
        __DEPS_ON_ANY__
    else
      ()

  let pp ppf { dep_default; dep_any; files } =
    if dep_default || dep_any then
      Fmt.(pair ~sep:cut (list @@ File.pp ~is_multifile:true) pp_helpers)
        ppf
        (files, (dep_default, dep_any))
    else
      let is_multifile =
        match files with
        | _ :: _ :: _ -> true
        | _ -> false
      in

      Fmt.(list @@ File.pp ~is_multifile) ppf files

  let to_string = Fmt.to_to_string pp
end

let go ctx target =
  try
    Standalone.to_string
    @@ Standalone.generate ctx target
    @@ Extract.collect_dependencies ctx target
  with
  | DependencyNotFound d -> Printf.sprintf "Dependency not found: %s" d
  | Unsupported
  | UnexpectedDependency ->
    Printexc.get_backtrace ()
