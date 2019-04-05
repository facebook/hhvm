(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Base
open Ast

let gen_fun_name field name =
  field ^ "##" ^ name

(* Gather information from a `ClassEnum` entry
   into a "expr_name -> (atom_name, expr_value) list" map

   Note: types are currently discarded. They might be used later on
   if we add `as` typing information in the branches
*)
let process_pumapping atom_name acc mappings =
  let f acc = function
    | PUMappingType _ -> acc
    | PUMappingID ((_, expr_name), expr_value) ->
      let lst = Option.value (SMap.find_opt expr_name acc) ~default:[] in
      let lst = (atom_name, expr_value) :: lst in
      SMap.add expr_name lst acc
  in List.fold_left ~f ~init:acc mappings

let process_class_enum fields =
  let f acc = function
    | PUCaseType _ | PUCaseTypeExpr _ -> acc
    | PUAtomDecl ((_, atom), mappings) ->
      process_pumapping atom acc mappings
  in
  let info = List.fold_left ~init:SMap.empty ~f fields in
  (* keep lists in the same order as their appear in the file *)
  SMap.map List.rev info

(* Creates a simple type hint from a string *)
let simple_typ pos name = (pos, Happly ((pos, name), []))

(* Error formatter *)
let error_msg cls field name =
  Printf.sprintf "%s::%s::%s unknown atom access: " cls field name

(* Generate a static accessor function from the pumapping information,
   something like

   static function Field_name##Expr_name(string $atom) : mixed {
     switch ($atom) {
       case "A":
        return valA;
       case "B":
        return valB;
       ...
       default:
           raise \Exception "illegal..."
     }
    }

   If the class is extending another, the raise statement is replaced with
   a call to parent::Field_name##Expr_name
*)
let gen_pu_accessor
    (fun_name: string)
    (pos: pos)
    (final: bool)
    (extends: bool)
    (info: (string * expr) list)
    (error: string) : class_elt =
  let var_name = "$atom" in
  let var_atom = (pos, Lvar (pos, var_name)) in
  let str pos name = (pos, String name) in
  let id pos name = (pos, Id (pos, name)) in
  let do_case entry init =
    Case (str pos entry, [
      (pos, Return (Some init))
    ]) in
  let cases =
    List.fold_right ~f:(fun (atom_name, value) acc ->
        (do_case atom_name value) :: acc) ~init:[] info in
  let default = if extends then
      let parent_call = Class_const (id pos "parent", (pos, fun_name)) in
      let call = Call ((pos, parent_call), [], [var_atom] , []) in
      Default [ pos, Return (Some (pos, call)) ]
    else
      let msg = Binop (Dot, str pos error, var_atom) in
      Default [
        pos, Throw (pos, New (id pos "\\Exception", [], [pos, msg], []))
      ] in
  let cases = cases @ [default] in
  let m_kind = [Public; Static] in
  let m_kind = if final then Final :: m_kind else m_kind in
  Method {
    m_kind = m_kind;
    m_tparams = [];
    m_constrs = [];
    m_name = (pos, fun_name);
    m_params = [ {
      param_hint = Some (simple_typ pos "string");
      param_is_reference = false;
      param_is_variadic = false;
      param_id = (pos, "$atom");
      param_expr = None;
      param_modifier = None;
      param_callconv = None;
      param_user_attributes = [];
    }];
    m_body = [
      (pos, Switch (var_atom, cases))
    ];
    m_user_attributes  = []; (* TODO: Memoize ? *)
    m_ret = Some (simple_typ pos "mixed");
    m_fun_kind = FSync;
    m_span = pos;
    m_doc_comment = None;
    m_external = false;
  }

(* Generate a helper/debug function called Members which is an immutable
   vector of all the atoms declared in a ClassEnum. Will be removed / boxed in
   the future
*)
let gen_Members field pos fields =
  let collect_member = function
    | PUAtomDecl ((_, id), _) -> Some id
    | _ -> None
  in
  let members = List.filter_map ~f:collect_member fields in
 let mems =
   List.map ~f:(fun x -> (AFvalue (pos, (String x)))) members
 in Method {
   m_kind = [Final; Public; Static ];
   m_tparams = [];
   m_constrs = [];
   m_name = (pos, gen_fun_name field "Members");
   m_params = [];
   m_body = [
     (pos,
      (Expr (pos,
             (Binop ((Eq None), (pos, (Lvar (pos, "$mems"))),
                     (pos, (Collection ((pos, "ImmVector"), None, mems)))
                    )))));
     (pos, (Return (Some (pos, (Lvar (pos, "$mems"))))))
   ];
   m_user_attributes  = []; (* TODO: Memoize ? *)
   m_ret = Some (simple_typ pos "mixed");
   m_fun_kind = FSync;
   m_span = pos;
   m_doc_comment = None;
   m_external = false;
 }

let gen_pu_accessors
    (class_name: string)
    (pos: pos)
    (final: bool)
    (extends: bool)
    (field_name: string)
    (fields: pufield list) : class_elt list =
  let fun_members = gen_Members field_name pos fields in
  let info = process_class_enum fields in
  fun_members ::
  SMap.fold (fun expr_name info acc ->
      let fun_name = gen_fun_name field_name expr_name in
      let error = error_msg class_name field_name expr_name in
      let hd = gen_pu_accessor fun_name pos final extends info error in
      hd :: acc
    ) info []

(* Gather all the 'from' constraints from a `constraint_kind * hint` list *)
let from_cstr l =
  let is_from_cstr = function
  | (Constraint_pu_from, _) -> true
  | _ -> false
  in
  let f { tp_name = (_, id); tp_constraints = l; _ } =
    if List.exists ~f:is_from_cstr l
    then Some id
    else None
  in
  List.filter_map ~f l

(* Remove instances of PU types in a `tparam` list. We don't use the
   regular visitor because we might end up removing some part of the list
   instead of just erase part of it *)
let erase_tparams clean_hint on_user_attribute (params: tparam list) =
  let erase_tparam_cst l =
    let f = function
      | (Constraint_pu_from, _) -> None
      | (cst, h) -> Some (cst, clean_hint h)
    in
    List.filter_map ~f l
  in
  let f tp = {
    tp with
    tp_constraints = erase_tparam_cst tp.tp_constraints;
    tp_user_attributes = List.map ~f:on_user_attribute tp.tp_user_attributes
  }
  in
  List.map ~f params

(* Instance of an AST visitor which:
   - updates PU_atom and PU_identifier
   - remove 'from' constraints
   - replace PU "dependent types" like `TF::T` with `mixed`

  This only do erasure. The generated code is done in a second path
*)
class ['self] erase_body_visitor = object (_self: 'self)
  inherit [_] endo as super
  method! on_PU_atom _ _ (_, id) = String id
  method! on_PU_identifier _ _ qual (pos, field) (_, name) =
    let fun_name = (pos, gen_fun_name field name) in
    Class_const (qual, fun_name)

  method! on_hint_ from_cstrs = function
    | Hoption h -> Hoption (super#on_hint from_cstrs h)
    | Hlike h -> Hlike (super#on_hint from_cstrs h)
    | Hfun (ic, hlist, plist, vhint, h) ->
      Hfun (
        ic, List.map ~f:(super#on_hint from_cstrs) hlist, plist,
        super#on_variadic_hint from_cstrs vhint,
        super#on_hint from_cstrs h)
    | Htuple hlist ->
      Htuple (List.map ~f:(super#on_hint from_cstrs) hlist)
    | Happly ((pos, id), hlist) ->
      let hlist = List.map ~f:(super#on_hint from_cstrs) hlist in
      if List.mem ~equal:String.equal from_cstrs id
      then Happly ((pos, "mixed"), hlist)
      else Happly ((pos, id), hlist)
    | Hshape si -> Hshape (super#on_shape_info from_cstrs si)
    | Haccess ((pos, id), _, _) as h ->
      if List.mem ~equal:String.equal from_cstrs id
      then Happly ((pos, "mixed"), [])
      else h
    | Hsoft h -> Hsoft (super#on_hint from_cstrs h)

  method! on_fun_ _ f =
    let from_cstrs = from_cstr f.f_tparams in
    (* erase f_tparams. Since we'll need to remove some of them, we don't
       recurse with the visitor, but by hand *)
    let f_tparams =
      erase_tparams (super#on_hint from_cstrs)
        (super#on_user_attribute from_cstrs) f.f_tparams in
    (* erase f_constrs. Since pattern *)
    let erase_constrs = function
      | (_, Constraint_pu_from, _) -> None
      | (h1, c, h2) ->
        Some (super#on_hint from_cstrs h1, c, super#on_hint from_cstrs h2)
    in
    let f_constrs = List.filter_map ~f:erase_constrs f.f_constrs in
    let f_ret = Option.map ~f:(super#on_hint from_cstrs) f.f_ret in
    let f_params =
      List.map ~f:(super#on_fun_param from_cstrs) f.f_params in
    let f_body = super#on_block from_cstrs f.f_body in
    let f_user_attributes =
      List.map ~f:(super#on_user_attribute from_cstrs)
        f.f_user_attributes in
    let f_file_attributes =
      List.map ~f:(super#on_file_attributes from_cstrs)
        f.f_file_attributes in
    { f with f_tparams; f_constrs; f_ret; f_params; f_body;
                 f_user_attributes; f_file_attributes }

  method! on_class_ _ c =
    let from_cstrs = from_cstr c.c_tparams in
    let c_user_attributes =
      List.map ~f:(super#on_user_attribute from_cstrs)
        c.c_user_attributes in
    let c_file_attributes =
      List.map ~f:(super#on_file_attributes from_cstrs)
        c.c_file_attributes in
    let c_tparams = erase_tparams (super#on_hint from_cstrs)
        (super#on_user_attribute from_cstrs) c.c_tparams in
    let c_extends = List.map ~f:(super#on_hint from_cstrs) c.c_extends in
    let c_implements =
      List.map ~f:(super#on_hint from_cstrs) c.c_implements in
    let c_body = List.map ~f:(super#on_class_elt from_cstrs) c.c_body in
    let c_enum = Option.map ~f:(super#on_enum_ from_cstrs) c.c_enum in
    { c with c_user_attributes; c_file_attributes; c_tparams;
                 c_extends; c_implements; c_body; c_enum }
end

(* Wrapper around the AST visitor *)
let visitor = new erase_body_visitor

let erase_stmt stmt = visitor#on_stmt [] stmt
let erase_fun f = visitor#on_fun_ [] f

(* Remove the PU entries in the class declaration *)
let strip_class_enum l =
  let is_class_enum = function
    | Const _
    | AbsConst _
    | Attributes _
    | TypeConst _
    | ClassUse _
    | ClassUseAlias _
    | ClassUsePrecedence _
    | XhpAttrUse _
    | ClassTraitRequire _
    | ClassVars _
    | XhpAttr _
    | Method _
    | MethodTraitResolution _
    | XhpCategory _
    | XhpChild _ -> true
    | ClassEnum _ -> false in
  List.filter ~f:is_class_enum l

let process_pufields class_name extends body =
  let f = function
    | ClassEnum (final, (pos, field_name), fields) ->
      gen_pu_accessors class_name pos final extends field_name fields
    | _ -> []
  in
  List.bind ~f body

let update_class c =
  let new_class_elt =
    process_pufields (snd c.c_name) (not (List.is_empty c.c_extends))
      c.c_body in
  let c = visitor#on_class_ [] c in
  let c_body = strip_class_enum c.c_body in
  { c with c_body = c_body @ new_class_elt }

let update_def d = match d with
  | Class c -> Class (update_class c)
  | Stmt s -> Stmt (erase_stmt s)
  | Fun f -> Fun (erase_fun f)
  | Typedef _
  | Constant _
  | Namespace _
  | NamespaceUse _
  | FileAttributes _
  | SetNamespaceEnv _ -> d

let translate (program : program) =
  List.map ~f:update_def program
