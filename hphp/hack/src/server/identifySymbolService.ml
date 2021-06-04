(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open SymbolOccurrence
open Typing_defs
module SN = Naming_special_names

module Result_set = Caml.Set.Make (struct
  type t = Relative_path.t SymbolOccurrence.t

  let compare : t -> t -> int = SymbolOccurrence.compare Relative_path.compare
end)

let is_target target_line target_char { pos; _ } =
  let (l, start, end_) = Pos.info_pos pos in
  l = target_line && start <= target_char && target_char - 1 <= end_

let process_class_id
    ?(is_declaration = false) ?(class_id_type = ClassId) (pos, cid) =
  Result_set.singleton
    { name = cid; type_ = Class class_id_type; is_declaration; pos }

let process_attribute (pos, name) class_name method_ =
  let type_ =
    match (name, class_name, method_) with
    | (name, Some (_, class_name), Some ((_, method_name), is_static))
      when String.equal name Naming_special_names.UserAttributes.uaOverride ->
      Attribute (Some { class_name; method_name; is_static })
    | _ -> Attribute None
  in
  Result_set.singleton { name; type_; is_declaration = false; pos }

let process_xml_attrs class_name attrs =
  List.fold attrs ~init:Result_set.empty ~f:(fun acc attr ->
      match attr with
      | Aast.Xhp_simple { Aast.xs_name = (pos, name); _ } ->
        Result_set.add
          {
            name;
            type_ = XhpLiteralAttr (class_name, Utils.add_xhp_ns name);
            is_declaration = false;
            pos;
          }
          acc
      | _ -> acc)

let clean_member_name name = String_utils.lstrip name "$"

let process_member ?(is_declaration = false) c_name id ~is_method ~is_const =
  let member_name = snd id in
  let type_ =
    if is_const then
      ClassConst (c_name, member_name)
    else if is_method then
      Method (c_name, member_name)
    else
      (*
        Per comment in symbolOcurrence.mli, XhpLiteralAttr
        is only used for attributes in XHP literals. Since
        process_member is not being used to handle XML attributes
        it is fine to define every symbol as Property.
      *)
      Property (c_name, member_name)
  in
  Result_set.singleton
    {
      name = c_name ^ "::" ^ clean_member_name member_name;
      type_;
      is_declaration;
      pos = fst id;
    }

let process_fun_id ?(is_declaration = false) id =
  Result_set.singleton
    { name = snd id; type_ = Function; is_declaration; pos = fst id }

let process_global_const ?(is_declaration = false) id =
  Result_set.singleton
    { name = snd id; type_ = GConst; is_declaration; pos = fst id }

let process_lvar_id ?(is_declaration = false) id =
  Result_set.singleton
    { name = snd id; type_ = LocalVar; is_declaration; pos = fst id }

let process_typeconst ?(is_declaration = false) (class_name, tconst_name, pos) =
  Result_set.singleton
    {
      name = class_name ^ "::" ^ tconst_name;
      type_ = Typeconst (class_name, tconst_name);
      is_declaration;
      pos;
    }

let process_class class_ =
  let acc = process_class_id ~is_declaration:true class_.Aast.c_name in
  let c_name = snd class_.Aast.c_name in
  let (constructor, static_methods, methods) = Aast.split_methods class_ in
  let all_methods = static_methods @ methods in
  let acc =
    List.fold all_methods ~init:acc ~f:(fun acc method_ ->
        Result_set.union acc
        @@ process_member
             c_name
             method_.Aast.m_name
             ~is_declaration:true
             ~is_method:true
             ~is_const:false)
  in
  let all_props = class_.Aast.c_vars in
  let acc =
    List.fold all_props ~init:acc ~f:(fun acc prop ->
        Result_set.union acc
        @@ process_member
             c_name
             prop.Aast.cv_id
             ~is_declaration:true
             ~is_method:false
             ~is_const:false)
  in
  let acc =
    List.fold class_.Aast.c_consts ~init:acc ~f:(fun acc const ->
        Result_set.union acc
        @@ process_member
             c_name
             const.Aast.cc_id
             ~is_declaration:true
             ~is_method:false
             ~is_const:true)
  in
  let acc =
    List.fold class_.Aast.c_typeconsts ~init:acc ~f:(fun acc typeconst ->
        let (pos, tconst_name) = typeconst.Aast.c_tconst_name in
        Result_set.union acc
        @@ process_typeconst ~is_declaration:true (c_name, tconst_name, pos))
  in
  (* We don't check anything about xhp attributes, so the hooks won't fire when
     typechecking the class. Need to look at them individually. *)
  let acc =
    List.fold class_.Aast.c_xhp_attr_uses ~init:acc ~f:(fun acc attr ->
        match attr with
        | (_, Aast.Happly (cid, _)) ->
          Result_set.union acc @@ process_class_id cid
        | _ -> acc)
  in
  match constructor with
  | Some method_ ->
    let id = (fst method_.Aast.m_name, SN.Members.__construct) in
    Result_set.union acc
    @@ process_member
         c_name
         id
         ~is_declaration:true
         ~is_method:true
         ~is_const:false
  | None -> acc

let typed_member_id env receiver_ty mid ~is_method ~is_const =
  Tast_env.get_class_ids env receiver_ty
  |> List.map ~f:(fun cid -> process_member cid mid ~is_method ~is_const)
  |> List.fold ~init:Result_set.empty ~f:Result_set.union

let typed_method = typed_member_id ~is_method:true ~is_const:false

let typed_const = typed_member_id ~is_method:false ~is_const:true

let typed_property = typed_member_id ~is_method:false ~is_const:false

let typed_constructor env ty pos =
  typed_method env ty (pos, SN.Members.__construct)

let typed_class_id ?(class_id_type = ClassId) env ty pos =
  Tast_env.get_class_ids env ty
  |> List.map ~f:(fun cid -> process_class_id ~class_id_type (pos, cid))
  |> List.fold ~init:Result_set.empty ~f:Result_set.union

(* When we detect a function reference encapsulated in a string,
 * we want to update the function reference without removing the apostrophes.
 *
 * Example: class_meth(myclass::class, 'myfunc');
 *
 * In this case, we only want to replace the text 'myfunc' - so we need
 * to shrink our positional data by the apostrophes. *)
let remove_apostrophes_from_function_eval (mid : Ast_defs.pstring) :
    Ast_defs.pstring =
  let (pos, member_name) = mid in
  let new_pos = Pos.shrink_by_one_char_both_sides pos in
  (new_pos, member_name)

let visitor =
  let class_name = ref None in
  let method_name = ref None in
  object (self)
    inherit [_] Tast_visitor.reduce as super

    method zero = Result_set.empty

    method plus = Result_set.union

    method! on_expr env expr =
      let pos = fst (fst expr) in
      let ( + ) = self#plus in
      let acc =
        match snd expr with
        | Aast.New (((p, ty), _), _, _, _, _) -> typed_constructor env ty p
        | Aast.Obj_get (((_, ty), _), (_, Aast.Id mid), _, _) ->
          typed_property env ty mid
        | Aast.Class_const (((_, ty), _), mid) -> typed_const env ty mid
        | Aast.Class_get (((_, ty), _), Aast.CGstring mid, _) ->
          typed_property env ty mid
        | Aast.Xml (cid, attrs, _) ->
          let class_id = process_class_id cid in
          let xhp_attributes = process_xml_attrs (snd cid) attrs in
          self#plus class_id xhp_attributes
        | Aast.Fun_id id ->
          process_fun_id (pos, SN.AutoimportedFunctions.fun_)
          + process_fun_id (remove_apostrophes_from_function_eval id)
        | Aast.FunctionPointer (Aast.FP_id id, _targs) -> process_fun_id id
        | Aast.FunctionPointer
            (Aast.FP_class_const (((_pos, ty), _cid), mid), _targs) ->
          typed_method env ty mid
        | Aast.Method_id (((_, ty), _), mid) ->
          process_fun_id (pos, SN.AutoimportedFunctions.inst_meth)
          + typed_method env ty (remove_apostrophes_from_function_eval mid)
        | Aast.Smethod_id (((_, ty), _), mid) ->
          process_fun_id (pos, SN.AutoimportedFunctions.class_meth)
          + typed_method env ty (remove_apostrophes_from_function_eval mid)
        | Aast.Method_caller (((_, cid) as pcid), mid) ->
          process_fun_id (pos, SN.AutoimportedFunctions.meth_caller)
          + process_class_id pcid
          + process_member
              cid
              (remove_apostrophes_from_function_eval mid)
              ~is_method:true
              ~is_const:false
        | Aast.EnumClassLabel (enum_name, label_name) ->
          (* We currently only support labels, not HH\Members using
           * __ViaLabel. TODO(T86724606)
           *)
          begin
            match enum_name with
            | None ->
              let ty = Typing_defs_core.get_node (snd (fst expr)) in
              (match ty with
              | Tnewtype (_, [ty_enum_class; _], _) ->
                (match get_node ty_enum_class with
                | Tclass ((_, enum_class_name), _, _)
                | Tgeneric (enum_class_name, _) ->
                  Result_set.singleton
                    {
                      (* TODO(T86724606) use "::" for __ViaLabel *)
                      name = Utils.strip_ns enum_class_name ^ "#" ^ label_name;
                      type_ = EnumClassLabel (enum_class_name, label_name);
                      is_declaration = false;
                      pos;
                    }
                | _ -> self#zero)
              | _ -> self#zero)
            | Some (_, enum_name) ->
              Result_set.singleton
                {
                  name = Utils.strip_ns enum_name ^ "#" ^ label_name;
                  type_ = EnumClassLabel (enum_name, label_name);
                  is_declaration = false;
                  pos;
                }
          end
        | _ -> self#zero
      in
      acc + super#on_expr env expr

    method! on_expression_tree env Aast.{ et_hint; et_virtualized_expr; _ } =
      let acc = self#on_hint env et_hint in
      self#plus acc (self#on_expr env et_virtualized_expr)

    method! on_class_id env ((p, ty), cid) =
      match cid with
      | Aast.CIexpr expr ->
        (* We want to special case this because we want to get the type of the
         inner expression, which will have a type like `classname<Foo>`, rather
         than the resolved type of the class ID, which will have a type like
         `Foo`. Since the class ID and the inner expression have the same span,
         it is not easy to distinguish them later. *)
        self#on_expr env expr
      | Aast.CIparent
      | Aast.CIself
      | Aast.CIstatic ->
        (* We want to special case these because we want to keep track of the
         original class id type. This information is useful in some cases, for
         instance when refactoring class names, because we want to avoid
         refactoring `self`, `static`, and `parent` class ids. *)
        typed_class_id ~class_id_type:Other env ty p
      | Aast.CI _ -> typed_class_id env ty p

    method! on_Call env e tal el unpacked_element =
      (* For Id, Obj_get (with an Id member), and Class_const, we don't want to
       * use the result of `self#on_expr env e`, since it would record a
       * property, class const, or global const access instead of a method call.
       * So instead of invoking super#on_Call, we reimplement it here, omitting
       * `self#on_expr env e` when necessary. *)
      let ( + ) = self#plus in
      let ea =
        match snd e with
        | Aast.Id id -> process_fun_id id
        | Aast.Obj_get ((((_, ty), _) as obj), (_, Aast.Id mid), _, _) ->
          self#on_expr env obj + typed_method env ty mid
        | Aast.Class_const ((((_, ty), _) as cid), mid) ->
          self#on_class_id env cid + typed_method env ty mid
        | _ -> self#on_expr env e
      in
      let tala = self#on_list self#on_targ env tal in
      let ela = self#on_list self#on_expr env el in
      let uea =
        Option.value_map
          ~default:Result_set.empty
          ~f:(self#on_expr env)
          unpacked_element
      in
      ea + tala + ela + uea

    method! on_Haccess env root ids =
      let acc =
        Tast_env.referenced_typeconsts env root ids
        |> List.map ~f:process_typeconst
        |> List.fold ~init:self#zero ~f:self#plus
      in
      self#plus acc (super#on_Haccess env root ids)

    method! on_Lvar env (pos, id) =
      let acc = process_lvar_id (pos, Local_id.get_name id) in
      self#plus acc (super#on_Lvar env (pos, id))

    method! on_fun_param env param =
      let acc = process_lvar_id (param.Aast.param_pos, param.Aast.param_name) in
      self#plus acc (super#on_fun_param env param)

    method! on_Happly env sid hl =
      let acc = process_class_id sid in
      self#plus acc (super#on_Happly env sid hl)

    method! on_catch env (sid, lid, block) =
      let acc = process_class_id sid in
      self#plus acc (super#on_catch env (sid, lid, block))

    method! on_class_ env class_ =
      Aast.(
        class_name := Some class_.c_name;
        let acc = process_class class_ in

        (*
      Enums implicitly extend BuiltinEnum. However, BuiltinEnums also extend
      the same Enum as a type parameter.

      Ex: enum Size extends BuiltinEnum<Size> { ... }

      This will return the definition of the enum twice when finding references
      on it. As a result, we set the extends property of an enum's tast to an empty list.
    *)
        let class_ =
          match class_.c_extends with
          | [(_, Happly ((_, builtin_enum), [(_, Happly (c_name, []))]))]
            when String.equal (snd c_name) (snd class_.c_name)
                 && String.equal
                      builtin_enum
                      Naming_special_names.Classes.cHH_BuiltinEnum ->
            { class_ with c_extends = [] }
          | _ -> class_
        in
        let acc = self#plus acc (super#on_class_ env class_) in
        class_name := None;
        acc)

    method! on_fun_ env fun_ =
      let acc = process_fun_id ~is_declaration:true fun_.Aast.f_name in
      self#plus acc (super#on_fun_ env { fun_ with Aast.f_unsafe_ctxs = None })

    method! on_typedef env typedef =
      let acc = process_class_id ~is_declaration:true typedef.Aast.t_name in
      self#plus acc (super#on_typedef env typedef)

    method! on_gconst env cst =
      let acc = process_global_const ~is_declaration:true cst.Aast.cst_name in
      self#plus acc (super#on_gconst env cst)

    method! on_Id env id =
      let acc = process_global_const id in
      self#plus acc (super#on_Id env id)

    method! on_Obj_get env obj member ognf in_parens =
      match snd member with
      | Aast.Id _ ->
        (* Don't visit this Id, since we would record it as a gconst access. *)
        let obja = self#on_expr env obj in
        let ognfa = self#on_og_null_flavor env ognf in
        self#plus obja ognfa
      | _ -> super#on_Obj_get env obj member ognf in_parens

    method! on_SFclass_const env cid mid =
      let ( + ) = Result_set.union in
      process_class_id cid
      + process_member (snd cid) mid ~is_method:false ~is_const:true
      + super#on_SFclass_const env cid mid

    method! on_method_ env m =
      method_name := Some (m.Aast.m_name, m.Aast.m_static);
      let acc = super#on_method_ env { m with Aast.m_unsafe_ctxs = None } in
      method_name := None;
      acc

    method! on_user_attribute env ua =
      let acc = process_attribute ua.Aast.ua_name !class_name !method_name in
      self#plus acc (super#on_user_attribute env ua)
  end

let all_symbols ctx tast =
  Errors.ignore_ (fun () -> visitor#go ctx tast |> Result_set.elements)

let all_symbols_ctx
    ~(ctx : Provider_context.t) ~(entry : Provider_context.entry) :
    Result_set.elt list =
  match entry.Provider_context.symbols with
  | None ->
    let { Tast_provider.Compute_tast.tast; _ } =
      Tast_provider.compute_tast_quarantined ~ctx ~entry
    in
    let symbols = all_symbols ctx tast in
    entry.Provider_context.symbols <- Some symbols;
    symbols
  | Some symbols -> symbols

let go_quarantined
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(line : int)
    ~(column : int) : Relative_path.t SymbolOccurrence.t list =
  all_symbols_ctx ~ctx ~entry |> List.filter ~f:(is_target line column)
