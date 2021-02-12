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
end

(* -- Decl_provider helpers ------------------------------------------------- *)
module Decl : sig
  val get_class_exn :
    Provider_context.t -> Decl_provider.class_key -> Decl_provider.class_decl

  val get_class_pos :
    Provider_context.t -> Decl_provider.class_key -> Pos.t option

  val get_class_pos_exn : Provider_context.t -> Decl_provider.class_key -> Pos.t

  val get_fun_pos : Provider_context.t -> Decl_provider.fun_key -> Pos.t option

  val get_fun_pos_exn : Provider_context.t -> Decl_provider.fun_key -> Pos.t

  val get_typedef_pos :
    Provider_context.t -> Decl_provider.typedef_key -> Pos.t option

  val get_gconst_pos :
    Provider_context.t -> Decl_provider.gconst_key -> Pos.t option

  val get_class_or_typedef_pos : Provider_context.t -> string -> Pos.t option
end = struct
  let get_class_exn ctx name =
    let not_found_msg = Printf.sprintf "Class %s" name in
    value_or_not_found not_found_msg @@ Decl_provider.get_class ctx name

  let get_fun_pos ctx name =
    Decl_provider.get_fun ctx name
    |> Option.map ~f:Typing_defs.((fun decl -> decl.fe_pos))

  let get_fun_pos_exn ctx name = value_or_not_found name (get_fun_pos ctx name)

  let get_class_pos ctx name =
    Decl_provider.get_class ctx name
    |> Option.map ~f:(fun decl -> Class.pos decl)

  let get_class_pos_exn ctx name =
    value_or_not_found name (get_class_pos ctx name)

  let get_typedef_pos ctx name =
    Decl_provider.get_typedef ctx name
    |> Option.map ~f:Typing_defs.((fun decl -> decl.td_pos))

  let get_gconst_pos ctx name =
    Decl_provider.get_gconst ctx name
    |> Option.map ~f:(fun ty -> Typing_defs.get_pos ty)

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
    (Pos.t, Nast.func_body_ann, unit, unit) Aast.class_typeconst option

  val get_prop :
    Provider_context.t ->
    string ->
    string ->
    (Pos.t, Nast.func_body_ann, unit, unit) Aast.class_var option

  val is_tydef : Provider_context.t -> string -> bool

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
    Decl_provider.class_key ->
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
      | Class cls -> Some cls
      | Cstr cls -> Some cls
      | AllMembers cls -> Some cls
      | Extends cls -> Some cls
      | Fun _
      | FunName _
      | GConst _
      | GConstName _ ->
        None
      | RecordDef _ -> raise Unsupported)

  let get_dep_pos ctx dep =
    let open Typing_deps.Dep in
    match dep with
    | Fun name
    | FunName name ->
      Decl.get_fun_pos ctx name
    | Class name
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
    | RecordDef _ -> raise Unsupported

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
    | Class name
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
    | RecordDef _ -> raise Unsupported

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
      | Typing_deps.Dep.Class h -> String.equal h SN.Typehints.wildcard
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
          let dep = Typing_deps.Dep.Class name in
          do_add_dep ctx env dep;

          (* If we have a constant of a generic type, it can only be an
           array type, e.g., vec<A>, for which don't need values of A
           to generate an initializer. *)
          List.iter tyl ~f:(add_dep ctx env ~this)

        method! on_tshape _ _ _ fdm =
          Nast.ShapeMap.iter
            (fun name Typing_defs.{ sft_ty; _ } ->
              (match name with
              | Ast_defs.SFlit_int _
              | Ast_defs.SFlit_str _ ->
                ()
              | Ast_defs.SFclass_const ((_, c), (_, s)) ->
                do_add_dep ctx env (Typing_deps.Dep.Class c);
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
              | Some Typing_defs.{ ttc_type; ttc_as_constraint; _ } ->
                Option.iter
                  ttc_type
                  ~f:(add_dep ctx ~this:(Some class_name) env);
                if not (List.is_empty tconsts) then (
                  match (ttc_type, ttc_as_constraint) with
                  | (Some tc_type, _)
                  | (None, Some tc_type) ->
                    (* What does 'this' refer to inside of T? *)
                    let this =
                      match Typing_defs.get_node tc_type with
                      | Typing_defs.Tapply ((_, name), _) -> Some name
                      | _ -> this
                    in
                    let taccess = make_taccess r tc_type tconsts in
                    add_dep ctx ~this env taccess
                  | (None, None) -> ()
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
      do_add_dep ctx env (Typing_deps.Dep.Class cls_name);
      (match Decl_provider.get_class ctx cls_name with
      | None ->
        let Typing_defs.{ td_type; td_constraint; _ } =
          value_or_not_found description
          @@ Decl_provider.get_typedef ctx cls_name
        in
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

            (* We need to initialize properties in the constructor, add a dependency on it *)
            do_add_dep ctx env (Cstr cls_name)
          | SProp (_, name) ->
            let Typing_defs.{ ce_type; _ } =
              value_or_not_found description @@ Class.get_sprop cls name
            in
            add_dep @@ Lazy.force ce_type
          | Method (_, name) ->
            let Typing_defs.{ ce_type; _ } =
              value_or_not_found description @@ Class.get_method cls name
            in
            add_dep @@ Lazy.force ce_type;
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
              add_dep @@ Lazy.force ce_type
            | _ -> ())
          | Class _ ->
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
          add_dep ctx ~this:None env @@ fe_type
        | GConst c
        | GConstName c ->
          let ty =
            value_or_not_found description @@ Decl_provider.get_gconst ctx c
          in
          add_dep ctx ~this:None env ty
        | _ -> raise UnexpectedDependency))

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
    if Dep.is_builtin ctx (Class ancestor_name) then
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
    let init = List.fold ~init:[] ~f @@ Class.all_ancestor_names cls in
    List.fold ~f ~init @@ Class.all_ancestor_req_names cls

  (** Add implementation depedencies of all class dependencies until we reach
    a fixed point *)
  let rec add_implementation_dependencies ctx env =
    let size = HashSet.length env.dependencies in
    let add_class dep acc =
      match dep with
      | Typing_deps.Dep.Class cls_name ->
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

  let rec pp_hint ppf (_, hint_) =
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
    | Aast.Hoption hint -> Fmt.(prefix (const string "?") pp_hint) ppf hint
    | Aast.Hlike hint -> Fmt.(prefix (const string "~") pp_hint) ppf hint
    | Aast.Hsoft hint -> Fmt.(prefix (const string "@") pp_hint) ppf hint
    | Aast.Htuple hints -> Fmt.(parens @@ list ~sep:comma pp_hint) ppf hints
    | Aast.Hunion hints -> Fmt.(parens @@ list ~sep:vbar pp_hint) ppf hints
    | Aast.Hintersection hints ->
      Fmt.(parens @@ list ~sep:amp pp_hint) ppf hints
    | Aast.Hprim prim -> pp_tprim ppf prim
    | Aast.Haccess (root, ids) ->
      Fmt.(pair ~sep:dbl_colon pp_hint @@ list ~sep:dbl_colon string)
        ppf
        (root, List.map ~f:snd ids)
    | Aast.Hvarray hint ->
      Fmt.(prefix (const string "varray") @@ angles pp_hint) ppf hint
    | Aast.Hvarray_or_darray (None, vhint) ->
      Fmt.(prefix (const string "varray_or_darray") @@ angles pp_hint) ppf vhint
    | Aast.Hvarray_or_darray (Some khint, vhint) ->
      Fmt.(
        prefix (const string "varray_or_darray")
        @@ angles
        @@ pair ~sep:comma pp_hint pp_hint)
        ppf
        (khint, vhint)
    | Aast.Hvec_or_dict (None, vhint) ->
      Fmt.(prefix (const string "vec_or_dict") @@ angles pp_hint) ppf vhint
    | Aast.Hvec_or_dict (Some khint, vhint) ->
      Fmt.(
        prefix (const string "vec_or_dict")
        @@ angles
        @@ pair ~sep:comma pp_hint pp_hint)
        ppf
        (khint, vhint)
    | Aast.Hdarray (khint, vhint) ->
      Fmt.(
        prefix (const string "darray")
        @@ angles
        @@ pair ~sep:comma pp_hint pp_hint)
        ppf
        (khint, vhint)
    | Aast.Habstr (name, [])
    | Aast.Happly ((_, name), []) ->
      Fmt.string ppf name
    | Aast.Habstr (name, hints)
    | Aast.Happly ((_, name), hints) ->
      Fmt.(prefix (const string name) @@ angles @@ list ~sep:comma pp_hint)
        ppf
        hints
    | Aast.(
        Hfun { hf_param_tys; hf_param_kinds; hf_variadic_ty; hf_return_ty; _ })
      ->
      (* TODO(vmladenov) support capability types here *)
      Fmt.(
        parens
        @@ pair
             ~sep:colon
             (pair
                ~sep:nop
                ( list ~sep:comma
                @@ pair ~sep:nop (option @@ suffix sp pp_paramkind) pp_hint )
                (option @@ surround ", " "..." @@ pp_hint))
             pp_hint)
        ppf
        ( (List.zip_exn hf_param_kinds hf_param_tys, hf_variadic_ty),
          hf_return_ty )
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
        pp_hint)
      ppf
      ((sfi_optional, sfi_name), sfi_hint)

  and pp_shape_field_name ppf = function
    | Ast_defs.SFlit_int (_, s) -> Fmt.string ppf s
    | Ast_defs.SFlit_str (_, s) -> Fmt.(surround "'" "'" string) ppf s
    | Ast_defs.SFclass_const ((_, c), (_, s)) ->
      Fmt.(pair ~sep:dbl_colon string string) ppf (c, s)

  let pp_user_attrs ppf attrs =
    match
      List.filter_map attrs ~f:(function
          | Aast.{ ua_name = (_, nm); ua_params = [] }
            when SMap.mem nm SN.UserAttributes.as_map ->
            Some nm
          | _ -> None)
    with
    | [] -> ()
    | rs -> Fmt.(angles @@ angles @@ list ~sep:comma string) ppf rs

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
    Format.(fprintf ppf {|%a %a|}) pp_constraint_kind kind pp_hint hint

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

  and pp_tparams ppf = function
    | [] -> ()
    | ps -> Fmt.(angles @@ list ~sep:comma pp_tparam) ppf ps

  let pp_type_hint ~is_ret_type ppf (_, hint) =
    if is_ret_type then
      Fmt.(option @@ prefix (const string ": ") pp_hint) ppf hint
    else
      Fmt.(option @@ suffix sp pp_hint) ppf hint

  let pp_fun_param_name ppf (param_is_variadic, param_name) =
    if param_is_variadic then
      Fmt.pf ppf {|...%s|} param_name
    else
      Fmt.string ppf param_name

  let pp_fun_param_default ppf _ = Fmt.pf ppf {| = \%s()|} __FN_MAKE_DEFAULT__

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
          _;
        } =
    Format.(
      fprintf
        ppf
        {|%a %a %a%a%a|}
        pp_user_attrs
        param_user_attributes
        Fmt.(option pp_paramkind)
        param_callconv
        (pp_type_hint ~is_ret_type:false)
        param_type_hint
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

  let pp_visibility ppf = function
    | Ast_defs.Private -> Fmt.string ppf "private"
    | Ast_defs.Public -> Fmt.string ppf "public"
    | Ast_defs.Protected -> Fmt.string ppf "protected"

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

    let mk_enum ctx Aast.{ c_name = (pos, name); c_enum; c_consts; _ } =
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
          Aast.{ f_user_attributes; f_tparams; f_variadic; f_params; f_ret; _ }
        ) =
      Fmt.(
        pf
          ppf
          "%a function %s%a%a%a {throw new \\Exception();}"
          pp_user_attrs
          f_user_attributes
          (strip_ns name)
          pp_tparams
          f_tparams
          pp_fun_params
          (f_params, f_variadic)
          (pp_type_hint ~is_ret_type:true)
          f_ret)

    (* -- Type definitions -------------------------------------------------- *)

    let pp_typedef_visiblity ppf = function
      | Aast_defs.Transparent -> Fmt.string ppf "type"
      | Aast_defs.Opaque -> Fmt.string ppf "newtype"

    let pp_fixme ppf code = Fmt.pf ppf "/* HH_FIXME[%d] */@." code

    let pp_tydef
        ppf (nm, fixmes, Aast.{ t_tparams; t_constraint; t_vis; t_kind; _ }) =
      Fmt.(
        pf
          ppf
          {|%a%a %s%a%a = %a;|}
          (list ~sep:cut pp_fixme)
          fixmes
          pp_typedef_visiblity
          t_vis
          (strip_ns nm)
          pp_tparams
          t_tparams
          (option @@ prefix (const string " as ") pp_hint)
          t_constraint
          pp_hint
          t_kind)

    (* -- Global constants -------------------------------------------------- *)

    let pp_gconst ppf (name, hint, init) =
      Fmt.pf ppf {|const %a %s = %s;|} pp_hint hint (strip_ns name) init

    (* -- Enums ------------------------------------------------------------- *)

    let pp_enum ppf (name, base, constr, consts) =
      Fmt.(
        pf
          ppf
          {|enum %s: %a%a {%a}|}
          (strip_ns name)
          pp_hint
          base
          (option @@ prefix (const string " as ") pp_hint)
          constr
          (list ~sep:sp @@ suffix semicolon @@ pair ~sep:equal_to string string)
          consts)

    let pp ppf = function
      | SEnum { name; base; constr; consts; _ } ->
        pp_enum ppf (name, base, constr, consts)
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
      (Pos.t, Nast.func_body_ann, unit, unit) Aast.class_typeconst -> t

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
          type_: Aast.hint option;
          constraint_: Aast.hint option;
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
        | (_, Some e) ->
          (match Decl_utils.infer_const e with
          | Some tprim ->
            let hint = (fst e, Aast.Hprim tprim) in
            (None, Some (init_value ctx hint))
          | None -> raise Unsupported)
        | (None, None) -> (None, None)
      in
      let line = Pos.line pos in
      EltConst { name; line; is_abstract; type_; init_val }

    let mk_tyconst
        Aast.
          {
            c_tconst_abstract;
            c_tconst_name = (pos, name);
            c_tconst_type;
            c_tconst_as_constraint;
            _;
          } =
      let is_abstract =
        Aast.(
          match c_tconst_abstract with
          | TCAbstract _ -> true
          | _ -> false)
      in
      EltTypeConst
        {
          name;
          line = Pos.line pos;
          is_abstract;
          type_ = c_tconst_type;
          constraint_ = c_tconst_as_constraint;
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
          "%a %a %a %a %a function %s%a%a%a%a"
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
          (option pp_hint)
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
          (option pp_hint)
          type_
          name
          (option @@ prefix equal_to string)
          init_val)

    (* -- Type constants ---------------------------------------------------- *)
    let pp_tyconst ppf (name, is_abstract, type_, constraint_) =
      Fmt.(
        pf
          ppf
          {|%a const type %s%a%a;|}
          (cond ~pp_t:(const string "abstract") ~pp_f:nop)
          is_abstract
          name
          (option @@ prefix (const string " as ") pp_hint)
          constraint_
          (option @@ prefix equal_to pp_hint)
          type_)

    (* -- Constants --------------------------------------------------------- *)

    let pp_const ppf (name, is_abstract, type_, init_val) =
      Fmt.(
        pf
          ppf
          {|%a const %a %s %a;|}
          (cond ~pp_t:(const string "abstract") ~pp_f:nop)
          is_abstract
          (option pp_hint)
          type_
          name
          (option @@ prefix equal_to string)
          init_val)

    (* -- Top level --------------------------------------------------------- *)
    let pp ppf = function
      | EltMethod (from_interface, _, method_) ->
        pp_method ppf (from_interface, method_)
      | EltTargetMethod (_, src) -> Fmt.string ppf @@ SourceText.text src
      | EltTypeConst { name; is_abstract; type_; constraint_; _ } ->
        pp_tyconst ppf (name, is_abstract, type_, constraint_)
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

    let pp_class_extends ppf = function
      | [] -> Fmt.nop ppf ()
      | hints ->
        Fmt.(prefix (const string "extends ") @@ list ~sep:comma pp_hint)
          ppf
          hints

    let pp_class_implements ppf = function
      | [] -> Fmt.nop ppf ()
      | hints ->
        Fmt.(prefix (const string "implements ") @@ list ~sep:comma pp_hint)
          ppf
          hints

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
          pp_class_extends
          extends
          pp_class_implements
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

    let pp_use ppf = Fmt.pf ppf "use %a;" pp_hint

    let pp_req_extends ppf = Fmt.pf ppf "require extends %a;" pp_hint

    let pp_req_implements ppf = Fmt.pf ppf "require implements %a;" pp_hint

    let pp ppf { uses; req_extends; req_implements; elements } =
      Fmt.(
        pair
          ~sep:sp
          (pair ~sep:sp (list ~sep:sp pp_use) (list ~sep:sp pp_req_extends))
          (pair
             ~sep:sp
             (list ~sep:sp pp_req_implements)
             (list ~sep:sp Class_elt.pp)))
        ppf
        ((uses, req_extends), (req_implements, elements))
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
      Fmt.(pair ~sep:sp Class_decl.pp @@ braces Class_body.pp) ppf (decl, body)
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
      (* -- Type defs and Enums -- *)
      | Class nm when Nast_helper.is_tydef ctx nm ->
        PartSingle (Single.mk_tydef @@ Nast_helper.get_typedef_exn ctx nm)
      | Class nm when Nast_helper.is_enum ctx nm ->
        PartSingle (Single.mk_enum ctx @@ Nast_helper.get_class_exn ctx nm)
      (* -- Classes -- *)
      | Class nm -> PartCls nm
      (* -- Ignore -- *)
      | Const _
      | Method _
      | SMethod _
      | Cstr _
      | Prop _
      | SProp _
      | AllMembers _
      | Extends _ ->
        PartIgnore
      | RecordDef _ -> raise Unsupported)

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

  let pp_namespace ppf nm = Fmt.pf ppf {|namespace %s|} nm

  let rec pp ppf { deps; children; _ } =
    Fmt.(pair ~sep:sp (list Grouped.pp) (list pp_children)) ppf (deps, children)

  and pp_children ppf (nm, t) =
    Fmt.(pair ~sep:sp pp_namespace @@ braces pp) ppf (nm, t)

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
      {|<<__Pure>>@.function %s(): nothing {@.  throw new \Exception();@.}|}
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
      Fmt.(
        pair ~sep:cut (list ~sep:sp @@ File.pp ~is_multifile:true) pp_helpers)
        ppf
        (files, (dep_default, dep_any))
    else
      let is_multifile =
        match files with
        | _ :: _ :: _ -> true
        | _ -> false
      in

      Fmt.(list ~sep:sp @@ File.pp ~is_multifile) ppf files

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
