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
module FFP = Full_fidelity_positioned_syntax

module Result_set = Stdlib.Set.Make (struct
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

let process_member ?(is_declaration = false) recv_class id ~is_method ~is_const
    =
  let member_name = snd id in
  let type_ =
    if is_const then
      ClassConst (recv_class, member_name)
    else if is_method then
      Method (recv_class, member_name)
    else
      (*
        Per comment in symbolOcurrence.mli, XhpLiteralAttr
        is only used for attributes in XHP literals. Since
        process_member is not being used to handle XML attributes
        it is fine to define every symbol as Property.
      *)
      Property (recv_class, member_name)
  in
  let c_name =
    match recv_class with
    | ClassName name -> name
    | UnknownClass -> "_"
  in
  Result_set.singleton
    {
      name = c_name ^ "::" ^ clean_member_name member_name;
      type_;
      is_declaration;
      pos = fst id;
    }

(* If there's an exact class name can find for this type, return its name. *)
let concrete_cls_name_from_ty enclosing_class_name ty : string option =
  let ty =
    match Typing_defs_core.get_node ty with
    | Tnewtype (n, _, ty) when String.equal n SN.Classes.cSupportDyn -> ty
    | _ -> ty
  in
  match Typing_defs_core.get_node ty with
  | Tclass ((_, cls_name), _, _) -> Some cls_name
  | Tgeneric ("this", _) -> enclosing_class_name
  | _ -> None

let get_callee enclosing_class_name (recv : Tast.expr) : receiver option =
  let (_, _, recv_expr_) = recv in
  match recv_expr_ with
  | Aast.Id (_, id) -> Some (FunctionReceiver id)
  | Aast.Class_const ((ty, _, _), (_, meth_name)) ->
    (match concrete_cls_name_from_ty enclosing_class_name ty with
    | Some cls_name ->
      Some (MethodReceiver { cls_name; meth_name; is_static = true })
    | None -> None)
  | Aast.Obj_get ((ty, _, _), (_, _, Aast.Id (_, meth_name)), _, _) ->
    (match concrete_cls_name_from_ty enclosing_class_name ty with
    | Some cls_name ->
      Some (MethodReceiver { cls_name; meth_name; is_static = false })
    | None -> None)
  | _ -> None

let process_arg_names recv (args : Tast.expr list) : Result_set.t =
  match recv with
  | Some recv_name ->
    List.mapi args ~f:(fun i (_, pos, _) ->
        {
          name = "(unused)";
          type_ = BestEffortArgument (recv_name, i);
          is_declaration = false;
          pos;
        })
    |> Result_set.of_list
  | None -> Result_set.empty

(* Add parameter names for all arguments at a call site. This enables
   us to show hover information on arguments.

       greet_user("John Doe");
                //^ Hover shows: Parameter: $name
*)
let process_callee_arg_names
    enclosing_class
    (recv : Tast.expr)
    (args : (Ast_defs.param_kind * Tast.expr) list) : Result_set.t =
  let enclosing_class_name = Option.map ~f:snd enclosing_class in
  let recv = get_callee enclosing_class_name recv in
  process_arg_names recv (List.map ~f:snd args)

(* Add parameter names for all arguments at an instantiation
   site. This enables us to show hover information on arguments.

       new User("John Doe");
              //^ Hover shows: Parameter: $name
*)
let process_constructor_arg_names
    enclosing_class
    (parent_class_hint : Aast.hint option)
    ((_, _, cid) : Tast.class_id)
    (args : Tast.expr list) : Result_set.t =
  let enclosing_class_name = Option.map ~f:snd enclosing_class in
  let cls_name =
    match cid with
    | Aast.CIparent ->
      (match parent_class_hint with
      | Some (_, Aast.Happly ((_, cls_name), _)) -> Some cls_name
      | _ -> None)
    | Aast.CIself -> enclosing_class_name
    | Aast.CIstatic -> enclosing_class_name
    | Aast.CIexpr (ty, _, _) ->
      concrete_cls_name_from_ty enclosing_class_name ty
    | Aast.CI (_, id) -> Some id
  in
  let recv =
    match cls_name with
    | Some cls_name ->
      Some
        (MethodReceiver
           { cls_name; meth_name = SN.Members.__construct; is_static = false })
    | None -> None
  in
  process_arg_names recv args

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
  let (constructor, static_methods, methods) =
    Aast.split_methods class_.Aast.c_methods
  in
  let all_methods = static_methods @ methods in
  let acc =
    List.fold all_methods ~init:acc ~f:(fun acc method_ ->
        Result_set.union acc
        @@ process_member
             (ClassName c_name)
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
             (ClassName c_name)
             prop.Aast.cv_id
             ~is_declaration:true
             ~is_method:false
             ~is_const:false)
  in
  let acc =
    List.fold class_.Aast.c_consts ~init:acc ~f:(fun acc const ->
        Result_set.union acc
        @@ process_member
             (ClassName c_name)
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
         (ClassName c_name)
         id
         ~is_declaration:true
         ~is_method:true
         ~is_const:false
  | None -> acc

let typed_member_id env receiver_ty mid ~is_method ~is_const =
  Tast_env.get_receiver_ids env receiver_ty
  |> List.map ~f:(function
         | Tast_env.RIclass cid -> ClassName cid
         | Tast_env.RIdynamic
         | Tast_env.RIerr
         | Tast_env.RIany ->
           UnknownClass)
  |> List.map ~f:(fun rid -> process_member rid mid ~is_method ~is_const)
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
  let parent_class_hint = ref None in
  let method_name = ref None in
  object (self)
    inherit [_] Tast_visitor.reduce as super

    method zero = Result_set.empty

    method plus = Result_set.union

    method! on_stmt env stmt =
      let (_, stmt_) = stmt in
      let ( + ) = self#plus in
      let acc =
        match stmt_ with
        | Aast.Declare_local (lid, _, _) -> self#on_Lvar env lid
        | _ -> self#zero
      in
      acc + super#on_stmt env stmt

    method! on_expr env expr =
      let (_, pos, expr_) = expr in
      let ( + ) = self#plus in
      let acc =
        match expr_ with
        | Aast.Hole (_e, _ty, _ty2, Aast.UnsafeCast _) ->
          process_fun_id (pos, SN.PseudoFunctions.unsafe_cast)
        | Aast.Hole (_e, _ty, _ty2, Aast.UnsafeNonnullCast) ->
          process_fun_id (pos, SN.PseudoFunctions.unsafe_nonnull_cast)
        | Aast.New ((ty, p, _), _, _, _, _) -> typed_constructor env ty p
        | Aast.Obj_get ((ty, _, _), (_, _, Aast.Id mid), _, _) ->
          typed_property env ty mid
        | Aast.Class_const ((ty, _, _), mid) -> typed_const env ty mid
        | Aast.Class_get ((ty, _, _), Aast.CGstring mid, _) ->
          typed_property env ty mid
        | Aast.Xml (cid, attrs, _) ->
          let class_id = process_class_id cid in
          let xhp_attributes = process_xml_attrs (snd cid) attrs in
          self#plus class_id xhp_attributes
        | Aast.FunctionPointer (Aast.FP_id id, _targs) -> process_fun_id id
        | Aast.FunctionPointer (Aast.FP_class_const ((ty, _, _cid), mid), _targs)
          ->
          typed_method env ty mid
        | Aast.Method_caller (((_, cid) as pcid), mid) ->
          process_fun_id (pos, SN.AutoimportedFunctions.meth_caller)
          + process_class_id pcid
          + process_member
              (ClassName cid)
              (remove_apostrophes_from_function_eval mid)
              ~is_method:true
              ~is_const:false
        | Aast.ValCollection ((_, kind), _, _) ->
          let type_name =
            match kind with
            | Aast_defs.Vector -> "Vector"
            | Aast_defs.ImmVector -> "ImmVector"
            | Aast_defs.Set -> "Set"
            | Aast_defs.ImmSet -> "ImmSet"
            | Aast_defs.Keyset -> "keyset"
            | Aast_defs.Vec -> "vec"
          in
          process_class_id (pos, "\\HH\\" ^ type_name)
        | Aast.KeyValCollection ((_, kind), _, _) ->
          let type_name =
            match kind with
            | Aast_defs.Map -> "Map"
            | Aast_defs.ImmMap -> "ImmMap"
            | Aast_defs.Dict -> "dict"
          in
          process_class_id (pos, "\\HH\\" ^ type_name)
        | Aast.EnumClassLabel (enum_name, label_name) -> begin
          match enum_name with
          | None ->
            let (ety, _, _) = expr in
            let ty = Typing_defs_core.get_node ety in
            (match ty with
            | Tnewtype (_, [ty_enum_class; _], _) ->
              (match get_node ty_enum_class with
              | Tclass ((_, enum_class_name), _, _)
              | Tgeneric (enum_class_name, _) ->
                Result_set.singleton
                  {
                    name = Utils.strip_ns enum_class_name ^ "#" ^ label_name;
                    type_ = EnumClassLabel (enum_class_name, label_name);
                    is_declaration = false;
                    pos;
                  }
              | _ -> self#zero)
            | _ -> self#zero)
          | Some ((_, enum_name) as enum_id) ->
            process_class_id enum_id
            + Result_set.singleton
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

    method! on_expression_tree
        env
        Aast.
          {
            et_class;
            et_virtualized_expr;
            et_splices;
            et_function_pointers;
            et_runtime_expr = _;
            et_dollardollar_pos = _;
          } =
      (* We only want to consider completion from the hint and the
         virtualized expression, not the visitor expression. The
         visitor expression is unityped, so we can't do much.*)
      let acc =
        process_class_id ~class_id_type:ExpressionTreeVisitor et_class
      in
      let acc = self#plus acc (self#on_Block env None et_splices) in

      (* We're overriding super#on_expression_tree, so we need to
         update the environment. *)
      let env = Tast_env.inside_expr_tree env et_class in
      let acc = self#plus acc (self#on_Block env None et_function_pointers) in

      let (_, _, virtualized_expr_) = et_virtualized_expr in
      let e =
        match virtualized_expr_ with
        | Aast.(
            Efun
              {
                ef_fun = { f_body = { fb_ast = [(_, Return (Some e))]; _ }; _ };
                _;
              }) ->
          (* The virtualized expression is wrapped in a
             lambda to help check unbound variables, which leads to
             unwanted closure info in hovers. Use the inner
             expression directly. *)
          e
        | _ -> et_virtualized_expr
      in
      self#plus acc (self#on_expr env e)

    method! on_class_id env (ty, p, cid) =
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

    method! on_If env cond then_block else_block : Result_set.t =
      match ServerUtils.resugar_invariant_call env cond then_block with
      | Some e -> self#on_expr env e
      | None -> super#on_If env cond then_block else_block

    method! on_Call
        env
        Aast.{ func = (_, _, expr_) as e; targs = tal; args = el; unpacked_arg }
        =
      (* For Id, Obj_get (with an Id member), and Class_const, we don't want to
       * use the result of `self#on_expr env e`, since it would record a
       * property, class const, or global const access instead of a method call.
       * So instead of invoking super#on_Call, we reimplement it here, omitting
       * `self#on_expr env e` when necessary. *)
      let ( + ) = self#plus in

      let ea =
        if Tast_env.is_in_expr_tree env then
          (* In an expression tree, we desugar function calls and
             method calls to calls to ::symbolType(). Any other
             function call is just calling typing helpers, and isn't
             useful in hover or go-to-definition.

             This ensures that when a user hovers over a literal "abc" we only
             show the inferred type, and don't show
             MyVisitor::stringType() information. *)
          match expr_ with
          | Aast.(
              Call
                {
                  func = (_, _, Class_const (_, (_, methName)));
                  args = [(_, arg)];
                  _;
                })
            when String.equal methName SN.ExpressionTrees.symbolType ->
            (* Treat MyVisitor::symbolType(foo<>) as just foo(). *)
            self#on_expr env arg
          | _ -> self#zero
        else
          let expr_ =
            match expr_ with
            | Aast.ReadonlyExpr (_, _, e) -> e
            | _ -> expr_
          in
          match expr_ with
          | Aast.Id id ->
            (* E.g. foo() *)
            process_fun_id id
          | Aast.Obj_get
              (((ty, _, _) as obj), (_, _, Aast.Id mid), _, Aast.Is_method) ->
            (* E.g. $bar->foo() *)
            self#on_expr env obj + typed_method env ty mid
          | Aast.Obj_get
              (((ty, _, _) as obj), (_, _, Aast.Id mid), _, Aast.Is_prop) ->
            (* E.g. ($bar->foo)() *)
            self#on_expr env obj + typed_property env ty mid
          | Aast.Class_const (((ty, _, _) as cid), mid) ->
            (* E.g. Bar::foo() *)
            self#on_class_id env cid + typed_method env ty mid
          | _ -> self#on_expr env e
      in

      let tala = self#on_list self#on_targ env tal in
      let ela = self#on_list self#on_expr env (List.map ~f:snd el) in
      let arg_names = process_callee_arg_names !class_name e el in
      let uea =
        Option.value_map
          ~default:Result_set.empty
          ~f:(self#on_expr env)
          unpacked_arg
      in
      ea + tala + ela + arg_names + uea

    method! on_New env cid targs args var_arg constr_ty =
      let named_params =
        process_constructor_arg_names !class_name !parent_class_hint cid args
      in
      self#plus named_params (super#on_New env cid targs args var_arg constr_ty)

    method! on_Haccess env root ids =
      let acc =
        Tast_env.referenced_typeconsts env root ids
        |> List.map ~f:process_typeconst
        |> List.fold ~init:self#zero ~f:self#plus
      in
      self#plus acc (super#on_Haccess env root ids)

    method! on_Hrefinement env root refs =
      let process_refinement acc r =
        let id =
          match r with
          | Aast.Rctx (id, _)
          | Aast.Rtype (id, _) ->
            id
        in
        Tast_env.referenced_typeconsts env root [id]
        |> List.map ~f:process_typeconst
        |> List.fold ~init:acc ~f:self#plus
      in
      let acc = List.fold ~init:self#zero ~f:process_refinement refs in
      self#plus acc (super#on_Hrefinement env root refs)

    method! on_tparam env tp =
      let (pos, name) = tp.Aast.tp_name in
      let acc =
        Result_set.singleton
          { name; type_ = TypeVar; is_declaration = true; pos }
      in
      self#plus acc (super#on_tparam env tp)

    method! on_Lvar env (pos, id) =
      let acc =
        if Local_id.is_user_denotable id then
          process_lvar_id (pos, Local_id.get_name id)
        else
          Result_set.empty
      in
      self#plus acc (super#on_Lvar env (pos, id))

    method! on_capture_lid _env (_, (pos, id)) =
      process_lvar_id (pos, Local_id.get_name id)

    method! on_hint env h =
      let acc =
        match h with
        | (pos, Aast.Habstr (name, _)) ->
          Result_set.singleton
            { name; type_ = TypeVar; is_declaration = false; pos }
        | (pos, Aast.Hprim prim) ->
          let name = Aast_defs.string_of_tprim prim in
          Result_set.singleton
            {
              name;
              type_ = BuiltInType (BIprimitive prim);
              is_declaration = false;
              pos;
            }
        | (pos, Aast.Hnothing) ->
          Result_set.singleton
            {
              name = "nothing";
              type_ = BuiltInType BInothing;
              is_declaration = false;
              pos;
            }
        | (pos, Aast.Hmixed) ->
          Result_set.singleton
            {
              name = "mixed";
              type_ = BuiltInType BImixed;
              is_declaration = false;
              pos;
            }
        | (pos, Aast.Hnonnull) ->
          Result_set.singleton
            {
              name = "nonnull";
              type_ = BuiltInType BInonnull;
              is_declaration = false;
              pos;
            }
        | (pos, Aast.Hdynamic) ->
          Result_set.singleton
            {
              name = "dynamic";
              type_ = BuiltInType BIdynamic;
              is_declaration = false;
              pos;
            }
        | (pos, Aast.Hshape _) ->
          Result_set.singleton
            {
              name = "shape";
              type_ = BuiltInType BIshape;
              is_declaration = false;
              pos;
            }
        | (pos, Aast.Hthis) ->
          Result_set.singleton
            {
              name = "this";
              type_ = BuiltInType BIthis;
              is_declaration = false;
              pos;
            }
        | (pos, Aast.Hoption _) ->
          (* Narrow the position to just the '?', not the whole ?Foo<Complicated<Bar>>. *)
          let (_start_line, start_column) = Pos.line_column pos in
          let qmark_pos = Pos.set_col_end (start_column + 1) pos in
          Result_set.singleton
            {
              name = "?";
              type_ = BuiltInType BIoption;
              is_declaration = false;
              pos = qmark_pos;
            }
        | _ -> Result_set.empty
      in
      self#plus acc (super#on_hint env h)

    method! on_fun_param env param =
      let acc = process_lvar_id (param.Aast.param_pos, param.Aast.param_name) in
      self#plus acc (super#on_fun_param env param)

    method! on_Happly env sid hl =
      match hl with
      | [h] when String.equal (snd sid) SN.Classes.cSupportDyn ->
        self#on_hint env h
      | _ ->
        let acc = process_class_id sid in
        self#plus acc (super#on_Happly env sid hl)

    method! on_catch env (sid, lid, block) =
      let acc = process_class_id sid in
      self#plus acc (super#on_catch env (sid, lid, block))

    method! on_class_ env class_ =
      class_name := Some class_.Aast.c_name;
      parent_class_hint := List.hd class_.Aast.c_extends;
      let acc = process_class class_ in

      (*
      Enums implicitly extend BuiltinEnum. However, BuiltinEnums also extend
      the same Enum as a type parameter.

      Ex: enum Size extends BuiltinEnum<Size> { ... }

      This will return the definition of the enum twice when finding references
      on it. As a result, we set the extends property of an enum's tast to an empty list.

      The same situation applies to Enum classes that extends
      BuiltinEnumClass. However in this case we just want to filter out
      this one extends, and keep the other unchanged.
    *)
      let class_ =
        let open Aast in
        let c_name = snd class_.c_name in
        (* Checks if the hint is matching the pattern
         * `HH\BuiltinEnumClass<HH\MemberOf<c_name, _>>`
         *)
        let is_generated_builtin_enum_class = function
          | ( _,
              Happly
                ( (_, builtin_enum_class),
                  [
                    ( _,
                      Happly
                        ( (_, memberof),
                          [(_, Happly ((_, name), [])); _interface] ) );
                  ] ) ) ->
            String.equal builtin_enum_class SN.Classes.cHH_BuiltinEnumClass
            && String.equal memberof SN.Classes.cMemberOf
            && String.equal name c_name
          | (_, Happly ((_, builtin_abstract_enum_class), [])) ->
            String.equal
              builtin_abstract_enum_class
              SN.Classes.cHH_BuiltinAbstractEnumClass
          | _ -> false
        in
        (* Checks if the hint is matching the pattern
         * `HH\BuiltinEnum<c_name>`
         *)
        let is_generated_builtin_enum = function
          | (_, Happly ((_, builtin_enum), [(_, Happly ((_, name), []))])) ->
            String.equal builtin_enum SN.Classes.cHH_BuiltinEnum
            && String.equal name c_name
          | _ -> false
        in
        (* If the class is an enum or enum class, remove the generated
               * occurrences.
        *)
        if Ast_defs.is_c_enum_class class_.c_kind then
          (* Enum classes might extend other classes, so we filter
           * the list and we don't depend on their order.
           *)
          let c_extends =
            List.filter_map
              ~f:(fun h ->
                if is_generated_builtin_enum_class h then
                  (* don't take this occurrence into account *)
                  None
                else
                  Some h)
              class_.c_extends
          in
          (* We also have to take care of the type of constants that
           * are rewritten from Foo to MemberOf<EnumName, Foo>
           *)
          let c_consts =
            List.map
              ~f:(fun cc ->
                let cc_type =
                  Option.map
                    ~f:(fun h ->
                      match snd h with
                      | Happly ((_, name), [_; h])
                        when String.equal name SN.Classes.cMemberOf ->
                        h
                      | _ -> h)
                    cc.cc_type
                in
                { cc with cc_type })
              class_.c_consts
          in
          { class_ with c_extends; c_consts }
        else if Ast_defs.is_c_enum class_.c_kind then
          (* For enums, we could remove everything as they don't extends
           * other classes, but let's filter anyway, just to be resilient
           * to future evolutions
           *)
          let c_extends =
            List.filter_map
              ~f:(fun h ->
                if is_generated_builtin_enum h then
                  (* don't take this occurrence into account *)
                  None
                else
                  Some h)
              class_.c_extends
          in
          { class_ with c_extends }
        else
          class_
      in
      let acc = self#plus acc (super#on_class_ env class_) in
      class_name := None;
      parent_class_hint := None;
      acc

    method! on_fun_def env fd =
      let acc = process_fun_id ~is_declaration:true fd.Aast.fd_name in
      self#plus acc (super#on_fun_def env fd)

    method! on_fun_ env fun_ =
      super#on_fun_ env { fun_ with Aast.f_unsafe_ctxs = None }

    method! on_typedef env typedef =
      let acc = process_class_id ~is_declaration:true typedef.Aast.t_name in
      self#plus acc (super#on_typedef env typedef)

    method! on_gconst env cst =
      let acc = process_global_const ~is_declaration:true cst.Aast.cst_name in
      self#plus acc (super#on_gconst env cst)

    method! on_Id env id =
      let acc = process_global_const id in
      self#plus acc (super#on_Id env id)

    method! on_Obj_get env obj ((_, _, expr_) as member) ognf prop_or_method =
      match expr_ with
      | Aast.Id _ ->
        (* Don't visit this Id, since we would record it as a gconst access. *)
        let obja = self#on_expr env obj in
        let ognfa = self#on_og_null_flavor env ognf in
        self#plus obja ognfa
      | _ -> super#on_Obj_get env obj member ognf prop_or_method

    method! on_SFclass_const env cid mid =
      let ( + ) = Result_set.union in
      process_class_id cid
      + process_member (ClassName (snd cid)) mid ~is_method:false ~is_const:true
      + super#on_SFclass_const env cid mid

    method! on_method_ env m =
      method_name := Some (m.Aast.m_name, m.Aast.m_static);
      let acc = super#on_method_ env { m with Aast.m_unsafe_ctxs = None } in
      method_name := None;
      acc

    method! on_user_attribute env ua =
      let tcopt = Tast_env.get_tcopt env in
      (* Don't show __SupportDynamicType if it's implicit everywhere *)
      if
        String.equal
          (snd ua.Aast.ua_name)
          SN.UserAttributes.uaSupportDynamicType
        && TypecheckerOptions.everything_sdt tcopt
        && not (TypecheckerOptions.enable_no_auto_dynamic tcopt)
      then
        Result_set.empty
      else
        let acc = process_attribute ua.Aast.ua_name !class_name !method_name in
        self#plus acc (super#on_user_attribute env ua)

    method! on_SetModule env sm =
      let (pos, id) = sm in
      let acc =
        Result_set.singleton
          { name = id; type_ = Module; is_declaration = false; pos }
      in
      self#plus acc (super#on_SetModule env sm)

    method! on_module_def env md =
      let (pos, id) = md.Aast.md_name in
      let acc =
        Result_set.singleton
          { name = id; type_ = Module; is_declaration = false; pos }
      in
      self#plus acc (super#on_module_def env md)
  end

(* Types of decls used in keyword extraction.*)
type classish_decl_kind =
  | DKclass
  | DKinterface
  | DKenumclass

type module_decl_kind =
  | DKModuleDeclaration
  | DKModuleMembershipDeclaration

type keyword_context =
  | ClassishDecl of classish_decl_kind
  | Method
  | Parameter
  | ReturnType
  | TypeConst
  | AsyncBlockHeader
  | ModuleDecl of module_decl_kind

let trivia_pos (t : Full_fidelity_positioned_trivia.t) : Pos.t =
  let open Full_fidelity_positioned_trivia in
  let open Full_fidelity_source_text in
  relative_pos
    t.source_text.file_path
    t.source_text
    t.offset
    (t.offset + t.width)

let fixme_elt (t : Full_fidelity_positioned_trivia.t) : Result_set.elt option =
  match t.Full_fidelity_positioned_trivia.kind with
  | Full_fidelity_trivia_kind.FixMe ->
    let pos = trivia_pos t in
    Some { name = "HH_FIXME"; type_ = HhFixme; is_declaration = false; pos }
  | _ -> None

let fixmes (tree : Full_fidelity_positioned_syntax.t) : Result_set.elt list =
  let open Full_fidelity_positioned_syntax in
  let rec aux acc s =
    let trivia_list = leading_trivia s in
    let fixme_elts = List.map trivia_list ~f:fixme_elt |> List.filter_opt in
    List.fold (children s) ~init:(fixme_elts @ acc) ~f:aux
  in

  aux [] tree

let token_pos (t : FFP.Token.t) : Pos.t =
  let offset = t.FFP.Token.offset + t.FFP.Token.leading_width in
  Full_fidelity_source_text.relative_pos
    t.FFP.Token.source_text.Full_fidelity_source_text.file_path
    t.FFP.Token.source_text
    offset
    (offset + t.FFP.Token.width)

let syntax_pos (s : FFP.t) : Pos.t =
  let open Full_fidelity_positioned_syntax in
  let offset = start_offset s + leading_width s in
  let source = source_text s in
  Full_fidelity_source_text.relative_pos
    source.Full_fidelity_source_text.file_path
    source
    offset
    (offset + width s)

(** Get keyword positions from the FFP for every keyword that has hover
    documentation. **)
let keywords (tree : FFP.t) : Result_set.elt list =
  let open Full_fidelity_positioned_syntax in
  let elt_of_token (ctx : keyword_context option) (t : FFP.Token.t) :
      Result_set.elt option =
    match t.Token.kind with
    | Token.TokenKind.Class ->
      (match ctx with
      | Some (ClassishDecl DKenumclass) ->
        Some
          {
            name = "enum class";
            type_ = Keyword EnumClass;
            is_declaration = false;
            pos = token_pos t;
          }
      | _ ->
        Some
          {
            name = "class";
            type_ = Keyword Class;
            is_declaration = false;
            pos = token_pos t;
          })
    | Token.TokenKind.Interface ->
      Some
        {
          name = "interface";
          type_ = Keyword Interface;
          is_declaration = false;
          pos = token_pos t;
        }
    | Token.TokenKind.Trait ->
      Some
        {
          name = "trait";
          type_ = Keyword Trait;
          is_declaration = false;
          pos = token_pos t;
        }
    | Token.TokenKind.Enum ->
      (match ctx with
      | Some (ClassishDecl DKenumclass) ->
        Some
          {
            name = "enum class";
            type_ = Keyword EnumClass;
            is_declaration = false;
            pos = token_pos t;
          }
      | _ ->
        Some
          {
            name = "enum";
            type_ = Keyword Enum;
            is_declaration = false;
            pos = token_pos t;
          })
    | Token.TokenKind.Type ->
      Some
        {
          name = "type";
          type_ =
            Keyword
              (match ctx with
              | Some TypeConst -> ConstType
              | _ -> Type);
          is_declaration = false;
          pos = token_pos t;
        }
    | Token.TokenKind.Newtype ->
      Some
        {
          name = "newtype";
          type_ = Keyword Newtype;
          is_declaration = false;
          pos = token_pos t;
        }
    | Token.TokenKind.Attribute ->
      Some
        {
          name = "attribute";
          type_ = Keyword XhpAttribute;
          is_declaration = false;
          pos = token_pos t;
        }
    | Token.TokenKind.Children ->
      Some
        {
          name = "children";
          type_ = Keyword XhpChildren;
          is_declaration = false;
          pos = token_pos t;
        }
    | Token.TokenKind.Const ->
      Some
        {
          name = "const";
          type_ =
            Keyword
              (match ctx with
              | None -> ConstGlobal
              | Some TypeConst -> ConstType
              | _ -> ConstOnClass);
          is_declaration = false;
          pos = token_pos t;
        }
    | Token.TokenKind.Static ->
      Some
        {
          name = "static";
          type_ =
            Keyword
              (match ctx with
              | Some Method -> StaticOnMethod
              | _ -> StaticOnProperty);
          is_declaration = false;
          pos = token_pos t;
        }
    | Token.TokenKind.Use ->
      Some
        {
          name = "use";
          type_ = Keyword Use;
          is_declaration = false;
          pos = token_pos t;
        }
    | Token.TokenKind.Function ->
      Some
        {
          name = "function";
          type_ =
            Keyword
              (match ctx with
              | Some Method -> FunctionOnMethod
              | _ -> FunctionGlobal);
          is_declaration = false;
          pos = token_pos t;
        }
    | Token.TokenKind.Extends ->
      Some
        {
          name = "extends";
          type_ =
            Keyword
              (match ctx with
              | Some (ClassishDecl DKclass) -> ExtendsOnClass
              | Some (ClassishDecl DKinterface) -> ExtendsOnInterface
              | _ -> ExtendsOnClass);
          is_declaration = false;
          pos = token_pos t;
        }
    | Token.TokenKind.Abstract ->
      Some
        {
          name = "abstract";
          type_ =
            Keyword
              (match ctx with
              | Some Method -> AbstractOnMethod
              | _ -> AbstractOnClass);
          is_declaration = false;
          pos = token_pos t;
        }
    | Token.TokenKind.Final ->
      Some
        {
          name = "final";
          type_ =
            Keyword
              (match ctx with
              | Some Method -> FinalOnMethod
              | _ -> FinalOnClass);
          is_declaration = false;
          pos = token_pos t;
        }
    | Token.TokenKind.Public ->
      Some
        {
          name = "public";
          type_ = Keyword Public;
          is_declaration = false;
          pos = token_pos t;
        }
    | Token.TokenKind.Protected ->
      Some
        {
          name = "protected";
          type_ = Keyword Protected;
          is_declaration = false;
          pos = token_pos t;
        }
    | Token.TokenKind.Private ->
      Some
        {
          name = "private";
          type_ = Keyword Private;
          is_declaration = false;
          pos = token_pos t;
        }
    | Token.TokenKind.Async ->
      Some
        {
          name = "async";
          type_ =
            Keyword
              (match ctx with
              | Some AsyncBlockHeader -> AsyncBlock
              | _ -> Async);
          is_declaration = false;
          pos = token_pos t;
        }
    | Token.TokenKind.Await ->
      Some
        {
          name = "await";
          type_ = Keyword Await;
          is_declaration = false;
          pos = token_pos t;
        }
    | Token.TokenKind.Concurrent ->
      Some
        {
          name = "concurrent";
          type_ = Keyword Concurrent;
          is_declaration = false;
          pos = token_pos t;
        }
    | Token.TokenKind.Readonly ->
      Some
        {
          name = "readonly";
          type_ =
            Keyword
              (match ctx with
              | Some Method -> ReadonlyOnMethod
              | Some Parameter -> ReadonlyOnParameter
              | Some ReturnType -> ReadonlyOnReturnType
              | _ -> ReadonlyOnExpression);
          is_declaration = false;
          pos = token_pos t;
        }
    | Token.TokenKind.Internal ->
      Some
        {
          name = "internal";
          type_ = Keyword Internal;
          is_declaration = false;
          pos = token_pos t;
        }
    | Token.TokenKind.Module ->
      Some
        {
          name = "module";
          type_ =
            Keyword
              (match ctx with
              | Some (ModuleDecl DKModuleDeclaration) ->
                ModuleInModuleDeclaration
              | Some (ModuleDecl DKModuleMembershipDeclaration) ->
                ModuleInModuleMembershipDeclaration
              | _ -> ModuleInModuleDeclaration);
          is_declaration = false;
          pos = token_pos t;
        }
    | _ -> None
  in

  (* Walk FFP syntax node [s], tracking the current context [ctx]
     (e.g. are we in an interface?) and accumulate hover items. *)
  let rec aux
      (ctx : keyword_context option) (acc : Result_set.elt list) (s : FFP.t) :
      Result_set.elt list =
    match s.syntax with
    | ClassishDeclaration cd ->
      let classish_decl_kind =
        match cd.classish_keyword.syntax with
        | Token t ->
          (match t.Token.kind with
          | Token.TokenKind.Class -> DKclass
          | Token.TokenKind.Interface -> DKinterface
          | _ -> DKclass)
        | _ -> DKclass
      in
      let ctx = Some (ClassishDecl classish_decl_kind) in
      List.fold (children s) ~init:acc ~f:(aux ctx)
    | EnumClassDeclaration _ ->
      List.fold
        (children s)
        ~init:acc
        ~f:(aux (Some (ClassishDecl DKenumclass)))
    | MethodishDeclaration _ ->
      List.fold (children s) ~init:acc ~f:(aux (Some Method))
    | FunctionDeclarationHeader
        {
          function_modifiers;
          function_keyword;
          function_name;
          function_type_parameter_list;
          function_left_paren;
          function_parameter_list;
          function_right_paren;
          function_contexts;
          function_colon;
          function_readonly_return;
          function_type;
          function_where_clause;
        } ->
      let acc = aux ctx acc function_modifiers in
      let acc = aux ctx acc function_keyword in
      let acc = aux ctx acc function_name in
      let acc = aux ctx acc function_type_parameter_list in
      let acc = aux ctx acc function_left_paren in
      let acc = aux ctx acc function_parameter_list in
      let acc = aux ctx acc function_right_paren in
      let acc = aux ctx acc function_contexts in
      let acc = aux ctx acc function_colon in
      let acc = aux (Some ReturnType) acc function_readonly_return in
      let acc = aux ctx acc function_type in
      let acc = aux ctx acc function_where_clause in
      acc
    | ParameterDeclaration _ ->
      List.fold (children s) ~init:acc ~f:(aux (Some Parameter))
    | AwaitableCreationExpression
        {
          awaitable_attribute_spec;
          awaitable_async;
          awaitable_compound_statement;
        } ->
      let acc = aux ctx acc awaitable_attribute_spec in
      let acc = aux (Some AsyncBlockHeader) acc awaitable_async in
      aux ctx acc awaitable_compound_statement
    | ModuleDeclaration _ ->
      List.fold
        (children s)
        ~init:acc
        ~f:(aux (Some (ModuleDecl DKModuleDeclaration)))
    | ModuleMembershipDeclaration _ ->
      List.fold
        (children s)
        ~init:acc
        ~f:(aux (Some (ModuleDecl DKModuleMembershipDeclaration)))
    | TypeConstDeclaration _ ->
      List.fold (children s) ~init:acc ~f:(aux (Some TypeConst))
    | Contexts c ->
      let acc = List.fold (children s) ~init:acc ~f:(aux ctx) in
      let is_empty =
        match c.contexts_types.syntax with
        | Missing -> true
        | _ -> false
      in
      if is_empty then
        {
          name = "pure function";
          type_ = PureFunctionContext;
          is_declaration = false;
          pos = syntax_pos s;
        }
        :: acc
      else
        acc
    | Token t ->
      (match elt_of_token ctx t with
      | Some elt -> elt :: acc
      | None -> acc)
    | _ -> List.fold (children s) ~init:acc ~f:(aux ctx)
  in

  aux None [] tree

let all_symbols ctx tast : Result_set.elt list =
  Errors.ignore_ (fun () -> visitor#go ctx tast |> Result_set.elements)

let all_symbols_ctx
    ~(ctx : Provider_context.t) ~(entry : Provider_context.entry) :
    Result_set.elt list =
  match entry.Provider_context.symbols with
  | None ->
    let cst = Ast_provider.compute_cst ~ctx ~entry in
    let tree = Provider_context.PositionedSyntaxTree.root cst in

    let fixme_symbols = fixmes tree in
    let keyword_symbols = keywords tree in
    let { Tast_provider.Compute_tast.tast; _ } =
      Tast_provider.compute_tast_quarantined ~ctx ~entry
    in

    let symbols =
      fixme_symbols
      @ keyword_symbols
      @ all_symbols ctx tast.Tast_with_dynamic.under_normal_assumptions
    in
    entry.Provider_context.symbols <- Some symbols;
    symbols
  | Some symbols -> symbols

let go_quarantined
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(line : int)
    ~(column : int) : Relative_path.t SymbolOccurrence.t list =
  all_symbols_ctx ~ctx ~entry |> List.filter ~f:(is_target line column)
