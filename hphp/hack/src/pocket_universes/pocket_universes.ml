(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Base
open Aast
open Tast
open Ast_defs

let annotation pos = pos, (Typing_reason.Rnone, Typing_defs.Tany)

let gen_fun_name field name =
  field ^ "##" ^ name

(* Gather information from a `ClassEnum` entry
   into a "expr_name -> (atom_name, expr_value) list" map

   Note: types are currently discarded. They might be used later on
   if we add `as` typing information in the branches
*)
let process_pumapping atom_name acc pu_member =
  let { pum_exprs; _ } = pu_member in
  let f acc ((_, expr_name), expr_value) =
    let lst = Option.value (SMap.find_opt expr_name acc) ~default:[] in
    let lst = (atom_name, expr_value) :: lst in
    SMap.add expr_name lst acc
  in List.fold_left ~f ~init:acc pum_exprs

let process_class_enum fields =
  let { pu_members; _ } = fields in
  let f acc member =
    let {pum_atom; _ } = member in
    process_pumapping (snd pum_atom) acc member
  in
  let info = List.fold_left ~init:SMap.empty ~f pu_members in
  (* keep lists in the same order as their appear in the file *)
  SMap.map List.rev info

(* Creates a simple type hint from a string *)
let simple_typ pos name = (pos, Happly ((pos, name), []))

let create_mixed_type_hint pos =
  (* The Pos.none on the first position is voluntary, see (T47713369) *)
  (Pos.none, Typing_make_type.nothing (Typing_defs.Reason.Rhint pos)),
  Some (simple_typ pos "mixed")

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
    (error: string) : method_ =
  let var_name = "$atom" in
  let var_atom = ((annotation pos), Lvar (pos, Local_id.make_unscoped var_name)) in
  let str pos name = ((annotation pos), String name) in
  let id pos name = ((annotation pos), Id (pos, name)) in
  let do_case entry init =
    Case (str pos entry, [
      (pos, Return (Some init))
    ]) in
  let cases =
    List.fold_right ~f:(fun (atom_name, value) acc ->
        (do_case atom_name value) :: acc) ~init:[] info in
  let default = if extends then
      let class_id = (annotation pos), CIexpr (id pos "parent") in
      let parent_call = Class_const (class_id, (pos, fun_name)) in
      let call = Call (Aast.Cnormal, ((annotation pos), parent_call), [], [var_atom] , []) in
      Default [ pos, Return (Some ((annotation pos), call)) ]
    else
      let msg = Binop (Dot, str pos error, var_atom) in
      let class_id = (annotation pos), CIexpr (id pos "\\Exception") in
      Default [
        pos,
        Throw ((annotation pos), New (class_id, [], [(annotation pos), msg], [], (annotation pos)))
      ] in
  let cases = cases @ [default] in
  let body = {
    fb_ast = [(pos, Switch (var_atom, cases))];
    fb_annotation = NoUnsafeBlocks
  }
  in
  {
    m_tparams = [];
    m_name = (pos, fun_name);
    m_params = [ {
      param_annotation = annotation pos;
      param_hint = Some (simple_typ pos "string");
      param_is_reference = false;
      param_is_variadic = false;
      param_pos = pos;
      param_name = "$atom";
      param_expr = None;
      param_callconv = None;
      param_user_attributes = [];
    }];
    m_body = body;
    m_user_attributes  = []; (* TODO: Memoize ? *)
    m_ret = create_mixed_type_hint pos;
    m_fun_kind = FSync;
    m_span = pos;
    m_doc_comment = None;
    m_external = false;
    m_visibility = Public;
    m_static = true;
    m_final = final;
    m_variadic = FVnonVariadic;
    m_where_constraints = [];
    m_annotation = dummy_saved_env;
    m_abstract = false;
  }

(* Generate a helper/debug function called Members which is an immutable
   vector of all the atoms declared in a ClassEnum. Will be removed / boxed in
   the future
*)
let gen_Members field pos (fields: pu_enum) =
 let { pu_members; _ } = fields in
 let annot = annotation pos in
 let mems =
   List.map ~f:(fun x -> (AFvalue (annot, (String (snd x.pum_atom))))) pu_members
 in
 let body = {
   fb_ast = [
     (pos,
      (Expr (annot,
             (Binop ((Eq None), (annot, (Lvar (pos, Local_id.make_unscoped "$mems"))),
                     (annot, (Collection ((pos, "ImmVector"), None, mems)))
                    )))));
     (pos, (Return (Some (annot, (Lvar (pos, Local_id.make_unscoped "$mems"))))))
   ];
   fb_annotation = NoUnsafeBlocks
 }
 in {
   m_visibility = Public;
   m_final = fields.pu_is_final;
   m_static = true;
   m_tparams = [];
   m_name = (pos, gen_fun_name field "Members");
   m_params = [];
   m_body = body;
   m_user_attributes  = []; (* TODO: Memoize ? *)
   m_ret = create_mixed_type_hint pos;
   m_fun_kind = FSync;
   m_span = pos;
   m_doc_comment = None;
   m_external = false;
   m_where_constraints = [];
   m_variadic = FVnonVariadic;
   m_annotation = dummy_saved_env;
   m_abstract = false;
 }

let gen_pu_accessors
    (class_name: string)
    (extends: bool)
    (field: pu_enum) : method_ list =
  let (pos, field_name) = field.pu_name in
  let fun_members = gen_Members field_name pos field in
  let info = process_class_enum field in
  fun_members ::
  SMap.fold (fun expr_name info acc ->
      let fun_name = gen_fun_name field_name expr_name in
      let error = error_msg class_name field_name expr_name in
      let hd = gen_pu_accessor fun_name pos field.pu_is_final extends info error in
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
  method on_'fb _env fb = fb
  method on_'ex _env ex = ex
  method on_'en _env en = en

  method! on_PU_atom _ _ s = String s
  method! on_PU_identifier _ _ qual (pos, field) (_, name) =
    let fun_name = (pos, gen_fun_name field name) in
    Class_const (qual, fun_name)

  method! on_hint_ from_cstrs = function
    | Hoption h -> Hoption (super#on_hint from_cstrs h)
    | Hlike h -> Hlike (super#on_hint from_cstrs h)
    | Hfun (fun_reactive, ic, hlist, plist, param_mutability_list, vhint, h, mut_return) ->
      Hfun (
        fun_reactive, ic, List.map ~f:(super#on_hint from_cstrs) hlist, plist,
        param_mutability_list,
        super#on_variadic_hint from_cstrs vhint,
        super#on_hint from_cstrs h,
        mut_return)
    | Htuple hlist ->
      Htuple (List.map ~f:(super#on_hint from_cstrs) hlist)
    | Happly ((pos, id), hlist) ->
      let hlist = List.map ~f:(super#on_hint from_cstrs) hlist in
      if List.mem ~equal:String.equal from_cstrs id
      then Happly ((pos, "mixed"), hlist)
      else Happly ((pos, id), hlist)
    | Hshape si -> Hshape (super#on_nast_shape_info from_cstrs si)
    | Haccess (_, (pos, id) :: _) as h ->
      if List.mem ~equal:String.equal from_cstrs id
      then Happly ((pos, "mixed"), [])
      else h
    | Hsoft h -> Hsoft (super#on_hint from_cstrs h)
    | Haccess (_, []) -> failwith "PocketUniverses: Encountered unexpected use of Haccess"
    (* The following hints do not exist on the legacy AST *)
    | Hany -> failwith "PocketUniverses: Encountered unexpected Hany"
    | Hmixed -> failwith "PocketUniverses: Encountered unexpected Hmixed"
    | Hnonnull -> failwith "PocketUniverses: Encountered unexpected Hnonnull"
    | Habstr _ -> failwith "PocketUniverses: Encountered unexpected Habstr"
    | Harray _ -> failwith "PocketUniverses: Encountered unexpected Harray"
    | Hdarray _ -> failwith "PocketUniverses: Encountered unexpected Hdarray"
    | Hvarray _ -> failwith "PocketUniverses: Encountered unexpected Hvarray"
    | Hvarray_or_darray _ ->
      failwith "PocketUniverses: Encountered unexpected Hvarray_or_darray"
    | Hprim _ -> failwith "PocketUniverses: Encountered unexpected Hprim"
    | Hthis -> failwith "PocketUniverses: Encountered unexpected Hthis"
    | Hdynamic -> failwith "PocketUniverses: Encountered unexpected Hdynamic"
    | Hnothing -> failwith "PocketUniverses: Encountered unexpected Hnothing"

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
    let f_where_constraints = List.filter_map ~f:erase_constrs f.f_where_constraints in
    let f_ret = type_hint_option_map ~f:(super#on_hint from_cstrs) f.f_ret in
    let f_params =
      List.map ~f:(super#on_fun_param from_cstrs) f.f_params in
    let f_body = {
      fb_ast = super#on_block from_cstrs f.f_body.fb_ast;
      fb_annotation = f.f_body.fb_annotation
    }
    in
    let f_user_attributes =
      List.map ~f:(super#on_user_attribute from_cstrs)
        f.f_user_attributes in
    let f_file_attributes =
      List.map ~f:(super#on_file_attribute from_cstrs)
        f.f_file_attributes in
    { f with f_tparams; f_where_constraints; f_ret; f_params; f_body;
                 f_user_attributes; f_file_attributes }

  method! on_class_ _ c =
    let from_cstrs = from_cstr c.c_tparams.c_tparam_list in
    let c_user_attributes =
      List.map ~f:(super#on_user_attribute from_cstrs)
        c.c_user_attributes in
    let c_file_attributes =
      List.map ~f:(super#on_file_attribute from_cstrs)
        c.c_file_attributes in
    let c_tparam_list = erase_tparams (super#on_hint from_cstrs)
        (super#on_user_attribute from_cstrs) c.c_tparams.c_tparam_list in
    let c_tparams = { c.c_tparams with c_tparam_list } in
    let c_extends = List.map ~f:(super#on_hint from_cstrs) c.c_extends in
    let c_implements =
      List.map ~f:(super#on_hint from_cstrs) c.c_implements in
    let c_methods = List.map ~f:(super#on_method_ from_cstrs) c.c_methods in
    let c_enum = Option.map ~f:(super#on_enum_ from_cstrs) c.c_enum in
    { c with c_user_attributes; c_file_attributes; c_tparams;
                 c_extends; c_implements; c_methods; c_enum }
end

(* Wrapper around the AST visitor *)
let visitor = new erase_body_visitor

let erase_stmt stmt = visitor#on_stmt [] stmt
let erase_fun f = visitor#on_fun_ [] f

let process_pufields class_name extends (pu_enums: pu_enum list) =
  List.concat_map ~f:(gen_pu_accessors class_name extends) pu_enums

let update_class c =
  let pu_methods =
    process_pufields (snd c.c_name) (not (List.is_empty c.c_extends))
      c.c_pu_enums in
  let c = visitor#on_class_ [] c in
  { c with c_pu_enums = []; c_methods = c.c_methods @ pu_methods }

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
