(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Typing code concerned the <<__PPL>> attribute. *)
open Hh_prelude
open Typing_defs
open Aast
module Cls = Decl_provider.Class
module Env = Tast_env

let has_ppl_attribute c =
  List.exists c.c_user_attributes (fun { ua_name; _ } ->
      String.equal SN.UserAttributes.uaProbabilisticModel (snd ua_name))

(* If an object's type is wrapped in a Tabstract, recurse until we've hit the base *)
let rec base_type env ty =
  let (env, ty) = Env.expand_type env ty in
  match get_node ty with
  | Tnewtype (_, _, ty)
  | Tdependent (_, ty) ->
    base_type env ty
  | _ -> (env, ty)

(**
 * Given a class, check the class's direct ancestors to verify that if
 * one member is annotated with the <<__PPL>> attribute, then all of them are.
 *)
let check_ppl_class env c =
  let is_ppl = has_ppl_attribute c in
  let child_class_string = Ast_defs.string_of_class_kind c.c_kind in
  let c_pos = fst c.c_name in
  let error = Errors.extend_ppl c_pos child_class_string is_ppl in
  let check verb parent_class_string = function
    | (_, Happly ((_, name), _)) ->
      begin
        match Decl_provider.get_class (Env.get_ctx env) name with
        | Some parent_type ->
          if Bool.( <> ) (Cls.ppl parent_type) is_ppl then
            error
              (Cls.pos parent_type)
              parent_class_string
              (Cls.name parent_type)
              verb
          else
            ()
        | None -> ()
      end
    | _ -> ()
  in
  List.iter c.c_extends (check "extend" "class");
  List.iter c.c_implements (check "implement" "interface");
  List.iter c.c_uses (check "use" "trait");
  let (c_extends, c_implements) = split_reqs c in
  List.iter c_extends (check "require" "class");
  List.iter c_implements (check "require" "interface")

(**
 * When we call a method on an object, if the object is a <<__PPL>> object,
 * then we can only call it via using the $this->method(...) syntax.
 *
 * This limits the ability to call it in a way that we are unable to rewrite.
 *)
let check_ppl_obj_get env ((p, ty), e) =
  let check_type ty =
    let (env, bty) = base_type env ty in
    match get_node bty with
    | Tclass ((_, name), _, _) ->
      begin
        match Decl_provider.get_class (Env.get_ctx env) name with
        | Some cls when Cls.ppl cls ->
          if not @@ Env.get_inside_ppl_class env then
            Errors.invalid_ppl_call p "from a different class";
          if Env.get_inside_constructor env then
            Errors.invalid_ppl_call
              p
              "from inside a <<__PPL>> class constructor";
          if not (phys_equal e This) then
            Errors.invalid_ppl_call
              p
              "inside a <<__PPL>> class unless using $this-> or $this:: syntax";
          ()
        | _ -> ()
      end
    | _ -> ()
  in
  let (_, type_list) = Env.get_concrete_supertypes env ty in
  List.iter type_list check_type

(**
 * If we are calling a parent method from within a ppl class, we cannot be in
 * the constructor of the child class.
 *
 * We will have already considered parent::__construct.
 *)
let check_ppl_parent_method env p =
  if Env.get_inside_ppl_class env && Env.get_inside_constructor env then
    Errors.invalid_ppl_static_call p "inside a <<__PPL>> class constructor"

(**
 * When we call a static method on a class, do not allow ClassName::method
 * because we are unable to detect whether the class being referred to
 * is a <<__PPL>> annotated class during codegen.
 *)
let check_ppl_class_const env p e =
  match e with
  | CIself
  | CIparent
  | CIstatic ->
    if Env.get_inside_ppl_class env && Env.get_inside_constructor env then
      Errors.invalid_ppl_static_call p "inside a <<__PPL>> class constructor"
    else
      ()
  | CI (_, name) ->
    begin
      match Decl_provider.get_class (Env.get_ctx env) name with
      | Some cls when Cls.ppl cls ->
        Errors.invalid_ppl_static_call
          p
          "by classname. Use self::, static::, or parent::"
      | _ -> ()
    end
  | CIexpr e -> check_ppl_obj_get env e

let check_ppl_meth_pointers env p classname special_name =
  match Decl_provider.get_class (Env.get_ctx env) classname with
  | Some cls when Cls.ppl cls -> Errors.ppl_meth_pointer p special_name
  | _ -> ()

let check_ppl_inst_meth env ((p, ty), _) =
  let check_type ty =
    let (_env, bty) = base_type env ty in
    match get_node bty with
    | Tclass ((_, name), _, _) ->
      begin
        match Decl_provider.get_class (Env.get_ctx env) name with
        | Some cls when Cls.ppl cls -> Errors.ppl_meth_pointer p "inst_meth"
        | _ -> ()
      end
    | _ -> ()
  in
  let (_, type_list) = Env.get_concrete_supertypes env ty in
  List.iter type_list check_type

let on_call_expr env ((p, _), x) =
  match x with
  | Obj_get (e, (_, _), _) -> check_ppl_obj_get env e
  | Class_const ((_, CIparent), (_, construct))
    when String.equal construct SN.Members.__construct ->
    ()
  | Class_const ((_, CIparent), _) -> check_ppl_parent_method env p
  | Class_const ((_, e), _) -> check_ppl_class_const env p e
  | _ -> ()

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env ((p, _), x) =
      match x with
      | Call (_, e, _, _, _) -> on_call_expr env e
      (* class_meth *)
      | Smethod_id ((_, classname), _) ->
        check_ppl_meth_pointers env p classname "class_meth"
      (* meth_caller *)
      | Method_caller ((_, classname), _) ->
        check_ppl_meth_pointers env p classname "meth_caller"
      (* inst_meth *)
      | Method_id (instance, _) -> check_ppl_inst_meth env instance
      | _ -> ()

    method! at_class_ env c = check_ppl_class env c
  end
