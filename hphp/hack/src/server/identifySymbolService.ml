(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open SymbolOccurrence
open Typing_defs

module Result_set = Set.Make(struct
  type t = Relative_path.t SymbolOccurrence.t
  let compare = Pervasives.compare
end)

let is_target target_line target_char { pos; _ } =
  let l, start, end_ = Pos.info_pos pos in
  l = target_line && start <= target_char && target_char - 1 <= end_

let process_class_id ?(is_declaration=false) (pos, cid) =
  Result_set.singleton {
    name  = cid;
    type_ = Class;
    is_declaration;
    pos   = pos
  }

let clean_member_name name = String_utils.lstrip name "$"

let process_member ?(is_declaration=false) c_name id ~is_method ~is_const =
  let member_name = (snd id) in
  let type_ =
    if is_const then ClassConst (c_name, member_name)
    else if is_method then Method (c_name, member_name)
    else Property (c_name, member_name)
  in
  Result_set.singleton {
    name = (c_name ^ "::" ^ (clean_member_name member_name));
    type_;
    is_declaration;
    pos = fst id
  }

let process_fun_id ?(is_declaration=false) id =
  Result_set.singleton {
    name  = snd id;
    type_ = Function;
    is_declaration;
    pos   = fst id
  }

let process_global_const ?(is_declaration=false) id =
  Result_set.singleton {
    name  = snd id;
    type_ = GConst;
    is_declaration;
    pos   = fst id
  }

let process_lvar_id ?(is_declaration=false) id =
  Result_set.singleton {
    name  = snd id;
    type_ = LocalVar;
    is_declaration;
    pos   = fst id
  }

let process_typeconst ?(is_declaration=false) (class_name, tconst_name, pos) =
  Result_set.singleton {
    name = class_name ^ "::" ^ tconst_name;
    type_ = Typeconst (class_name, tconst_name);
    is_declaration;
    pos;
  }

let process_class class_ =
  let acc = process_class_id ~is_declaration:true class_.Tast.c_name in
  let c_name = snd class_.Tast.c_name in
  let all_methods = class_.Tast.c_methods @ class_.Tast.c_static_methods in
  let acc = List.fold all_methods ~init:acc ~f:begin fun acc method_ ->
    Result_set.union acc @@
      process_member c_name method_.Tast.m_name
        ~is_declaration:true ~is_method:true ~is_const:false
  end in
  let all_props = class_.Tast.c_vars @ class_.Tast.c_static_vars in
  let acc = List.fold all_props ~init:acc ~f:begin fun acc prop ->
    Result_set.union acc @@
      process_member c_name prop.Tast.cv_id
        ~is_declaration:true ~is_method:false ~is_const:false
  end in
  let acc = List.fold class_.Tast.c_consts ~init:acc ~f:begin fun acc (_, const_id, _) ->
    Result_set.union acc @@
      process_member c_name const_id
        ~is_declaration:true ~is_method:false ~is_const:true
  end in
  let acc = List.fold class_.Tast.c_typeconsts ~init:acc ~f:begin fun acc typeconst ->
    let pos, tconst_name = typeconst.Tast.c_tconst_name in
    Result_set.union acc @@
      process_typeconst ~is_declaration:true (c_name, tconst_name, pos)
  end in
  (* We don't check anything about xhp attributes, so the hooks won't fire when
     typechecking the class. Need to look at them individually. *)
  let acc = List.fold class_.Tast.c_xhp_attr_uses ~init:acc ~f:begin fun acc attr ->
    match attr with
    | _, Aast.Happly (cid, _) ->
      Result_set.union acc @@
        process_class_id cid
    | _ -> acc
  end in
  match class_.Tast.c_constructor with
    | Some method_ ->
      let id = fst method_.Tast.m_name, SN.Members.__construct in
      Result_set.union acc @@
        process_member c_name id
          ~is_declaration:true ~is_method:true ~is_const:false
    | None -> acc

let typed_member_id env receiver_ty mid ~is_method ~is_const =
  Tast_env.get_class_ids env receiver_ty
  |> List.map ~f:(fun cid -> process_member cid mid ~is_method ~is_const)
  |> List.fold ~init:Result_set.empty ~f:Result_set.union

let typed_method   = typed_member_id ~is_method:true  ~is_const:false
let typed_const    = typed_member_id ~is_method:false ~is_const:true
let typed_property = typed_member_id ~is_method:false ~is_const:false
let typed_constructor env ty pos =
  typed_method env ty (pos, SN.Members.__construct)

let typed_class_id env ty pos =
  Tast_env.get_class_ids env ty
  |> List.map ~f:(fun cid -> process_class_id (pos, cid))
  |> List.fold ~init:Result_set.empty ~f:Result_set.union

let visitor = object (self)
  inherit [_] Tast_visitor.reduce as super

  method zero = Result_set.empty
  method plus = Result_set.union

  method! on_expr env expr =
    let pos = fst (fst expr) in
    let (+) = self#plus in
    let acc =
      match snd expr with
      | Tast.New (((p, ty), _), _, _) ->
        typed_constructor env ty p
      | Tast.Obj_get (((_, ty), _), (_, Tast.Id mid), _) ->
        typed_property env ty mid
      | Tast.Class_const (((_, ty), _), mid) ->
        typed_const env ty mid
      | Tast.Class_get (((_, ty), _), mid) ->
        typed_property env ty mid
      | Tast.Xml (cid, _, _) ->
        process_class_id cid
      | Tast.Fun_id id ->
        process_fun_id (pos, "\\"^SN.SpecialFunctions.fun_) +
        process_fun_id id
      | Tast.Method_id (((_, ty), _), mid) ->
        process_fun_id (pos, "\\"^SN.SpecialFunctions.inst_meth) +
        typed_method env ty mid
      | Tast.Smethod_id ((_, cid) as pcid, mid) ->
        process_fun_id (pos, "\\"^SN.SpecialFunctions.class_meth) +
        process_class_id pcid +
        process_member cid mid ~is_method:true ~is_const:false
      | Tast.Method_caller ((_, cid) as pcid, mid) ->
        process_fun_id (pos, "\\"^SN.SpecialFunctions.meth_caller) +
        process_class_id pcid +
        process_member cid mid ~is_method:true ~is_const:false
      | _ -> self#zero
    in
    acc + super#on_expr env expr

  method! on_class_id env ((p, ty), cid) =
    match cid with
    | Tast.CIexpr expr ->
      (* We want to special case this because we want to get the type of the
         inner expression, which will have a type like `classname<Foo>`, rather
         than the resolved type of the class ID, which will have a type like
         `Foo`. Since the class ID and the inner expression have the same span,
         it is not easy to distinguish them later. *)
      self#on_expr env expr
    | _ -> typed_class_id env ty p

  method! on_Call env ct e hl el uel =
    (* For Id, Obj_get (with an Id member), and Class_const, we don't want to
     * use the result of `self#on_expr env e`, since it would record a
     * property, class const, or global const access instead of a method call.
     * So instead of invoking super#on_Call, we reimplement it here, omitting
     * `self#on_expr env e` when necessary. *)
    let (+) = self#plus in
    let cta = self#on_call_type env ct in
    let ea =
      match snd e with
      | Tast.Id id ->
        process_fun_id id
      | Tast.Obj_get (((_, ty), _) as obj, (_, Tast.Id mid), _) ->
        self#on_expr env obj + typed_method env ty mid
      | Tast.Class_const (((_, ty), _) as cid, mid) ->
        self#on_class_id env cid + typed_method env ty mid
      | _ -> self#on_expr env e
    in
    let hla  = self#on_list self#on_hint env hl in
    let ela  = self#on_list self#on_expr env el in
    let uela = self#on_list self#on_expr env uel in
    cta + ea + hla + ela + uela

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
    let acc = process_lvar_id (param.Tast.param_pos, param.Tast.param_name) in
    self#plus acc (super#on_fun_param env param)

  method! on_instantiated_sid env id =
    let acc = process_class_id (fst id) in
    self#plus acc (super#on_instantiated_sid env id)

  method! on_Happly env sid hl =
    let acc = process_class_id sid in
    self#plus acc (super#on_Happly env sid hl)

  method! on_catch env (sid, lid, block) =
    let acc = process_class_id sid in
    self#plus acc (super#on_catch env (sid, lid, block))

  method! on_class_ env class_ =
    let open Tast in
    let open Aast in
    let acc = process_class class_ in
    (*
      Enums implicitly extend BuiltinEnum. However, BuiltinEnums also extend
      the same Enum as a type parameter.

      Ex: enum Size extends BuiltinEnum<Size> { ... }

      This will return the definition of the enum twice when finding references
      on it. As a result, we set the extends property of an enum's tast to an empty list.
    *)
    let class_ = match class_.c_extends with
      | [(_,
          Happly ((_, builtin_enum), [(_,
            Happly (c_name, []))]
          )
        )]
        when c_name = class_.c_name && builtin_enum = Naming_special_names.Classes.cHH_BuiltinEnum
        -> { class_ with c_extends = [] }
      | _ -> class_
    in
    self#plus acc (super#on_class_ env class_)

  method! on_fun_ env fun_ =
    let acc = process_fun_id ~is_declaration:true fun_.Tast.f_name in
    self#plus acc (super#on_fun_ env fun_)

  method! on_typedef env typedef =
    let acc = process_class_id ~is_declaration:true typedef.Tast.t_name in
    self#plus acc (super#on_typedef env typedef)

  method! on_gconst env cst =
    let acc = process_global_const ~is_declaration:true cst.Tast.cst_name in
    self#plus acc (super#on_gconst env cst)

  method! on_Id env id =
    let acc = process_global_const id in
    self#plus acc (super#on_Id env id)

  method! on_Obj_get env obj member ognf =
    match snd member with
    | Tast.Id _ ->
      (* Don't visit this Id, since we would record it as a gconst access. *)
      let obja = self#on_expr env obj in
      let ognfa = self#on_og_null_flavor env ognf in
      self#plus obja ognfa
    | _ -> super#on_Obj_get env obj member ognf
end

let all_symbols tast =
  visitor#go tast
  |> Result_set.elements

let go tast line char =
  all_symbols tast
  |> List.filter ~f:(is_target line char)
