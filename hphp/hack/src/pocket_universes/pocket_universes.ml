(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Aast
module T = Tast

let annotation pos =
  (pos, Typing_defs.mk (Typing_reason.Rnone, Typing_defs.make_tany ()))

let gen_fun_name field name = field ^ "##" ^ name

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
  in
  List.fold_left ~f ~init:acc pum_exprs

let process_class_enum fields =
  let { pu_members; _ } = fields in
  let f acc member =
    let { pum_atom; _ } = member in
    process_pumapping (snd pum_atom) acc member
  in
  let info = List.fold_left ~init:SMap.empty ~f pu_members in
  (* keep lists in the same order as their appear in the file *)
  SMap.map List.rev info

(* Creates a simple type hint from a string *)
let simple_typ pos name = (pos, Happly ((pos, name), []))

let apply_to_typ pos name typ = (pos, Happly ((pos, name), [typ]))

let create_mixed_type_hint pos =
  ( Typing_make_type.mixed (Typing_defs.Reason.Rhint pos),
    Some (simple_typ pos "\\HH\\mixed") )

let create_vec_string_type_hint pos =
  ( Typing_make_type.mixed (Typing_defs.Reason.Rhint pos),
    Some (apply_to_typ pos "\\HH\\vec" (simple_typ pos "\\HH\\string")) )

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
    (fun_name : string)
    (pos : pos)
    (final : bool)
    (extends : bool)
    (info : (string * T.expr) list)
    (error : string) : T.method_ =
  let var_name = "$atom" in
  let var_atom =
    (annotation pos, Lvar (pos, Local_id.make_unscoped var_name))
  in
  let str pos name = (annotation pos, String name) in
  let id pos name = (annotation pos, Id (pos, name)) in
  let do_case entry init = Case (str pos entry, [(pos, Return (Some init))]) in
  let cases =
    List.fold_right
      ~f:(fun (atom_name, value) acc -> do_case atom_name value :: acc)
      ~init:[]
      info
  in
  let default =
    if extends then
      let class_id = (annotation pos, CIexpr (id pos "parent")) in
      let parent_call = Class_const (class_id, (pos, fun_name)) in
      let call =
        Call (Aast.Cnormal, (annotation pos, parent_call), [], [var_atom], None)
      in
      Default (pos, [(pos, Return (Some (annotation pos, call)))])
    else
      let open Ast_defs in
      let msg = Binop (Dot, str pos error, var_atom) in
      let class_id = (annotation pos, CIexpr (id pos "\\Exception")) in
      Default
        ( pos,
          [
            ( pos,
              Throw
                ( annotation pos,
                  New
                    (class_id, [], [(annotation pos, msg)], None, annotation pos)
                ) );
          ] )
  in
  let cases = cases @ [default] in
  let body =
    {
      fb_ast = [(pos, Switch (var_atom, cases))];
      fb_annotation = T.NoUnsafeBlocks;
    }
  in
  {
    m_tparams = [];
    m_name = (pos, fun_name);
    m_params =
      [
        {
          param_annotation = annotation pos;
          param_type_hint =
            (snd (annotation pos), Some (simple_typ pos "\\HH\\string"));
          param_is_variadic = false;
          param_pos = pos;
          param_name = "$atom";
          param_expr = None;
          param_callconv = None;
          param_user_attributes = [];
          param_visibility = None;
        };
      ];
    m_body = body;
    m_user_attributes = [];
    (* TODO: Memoize ? *)
    m_ret = create_mixed_type_hint pos;
    m_fun_kind = Ast_defs.FSync;
    m_span = pos;
    m_doc_comment = None;
    m_external = false;
    m_visibility = Public;
    m_static = true;
    m_final = final;
    m_variadic = FVnonVariadic;
    m_where_constraints = [];
    m_annotation = T.dummy_saved_env;
    m_abstract = false;
  }

(* Generate a helper/debug function called Members which is an immutable
   vector of all the atoms declared in a ClassEnum. Will be removed / boxed in
   the future
*)
let gen_Members field pos (fields : T.pu_enum) =
  let { pu_members; _ } = fields in
  let annot = annotation pos in
  let mems =
    List.map ~f:(fun x -> AFvalue (annot, String (snd x.pum_atom))) pu_members
  in
  let body =
    {
      fb_ast =
        [
          ( pos,
            Expr
              ( annot,
                Binop
                  ( Ast_defs.Eq None,
                    (annot, Lvar (pos, Local_id.make_unscoped "$mems")),
                    (annot, Collection ((pos, "vec"), None, mems)) ) ) );
          ( pos,
            Return (Some (annot, Lvar (pos, Local_id.make_unscoped "$mems"))) );
        ];
      fb_annotation = T.NoUnsafeBlocks;
    }
  in
  {
    m_visibility = Public;
    m_final = fields.pu_is_final;
    m_static = true;
    m_tparams = [];
    m_name = (pos, gen_fun_name field "Members");
    m_params = [];
    m_body = body;
    m_user_attributes = [];
    (* TODO: Memoize ? *)
    m_ret = create_vec_string_type_hint pos;
    m_fun_kind = Ast_defs.FSync;
    m_span = pos;
    m_doc_comment = None;
    m_external = false;
    m_where_constraints = [];
    m_variadic = FVnonVariadic;
    m_annotation = T.dummy_saved_env;
    m_abstract = false;
  }

let gen_pu_accessors (class_name : string) (extends : bool) (field : T.pu_enum)
    : T.method_ list =
  let (pos, field_name) = field.pu_name in
  let fun_members = gen_Members field_name pos field in
  let info = process_class_enum field in
  fun_members
  :: SMap.fold
       (fun expr_name info acc ->
         let fun_name = gen_fun_name field_name expr_name in
         let error = error_msg class_name field_name expr_name in
         let hd =
           gen_pu_accessor fun_name pos field.pu_is_final extends info error
         in
         hd :: acc)
       info
       []

(* Instance of an AST visitor which:
   - updates PU_atom and PU_identifier

  This only do erasure. The generated code is done in a second path
*)
class ['self] erase_body_visitor =
  object (_self : 'self)
    inherit [_] endo

    method on_'fb _env fb = fb

    method on_'ex _env ex = ex

    method on_'en _env en = en

    method on_'hi _env hi = hi

    method! on_PU_atom _ _ s = String s

    method! on_PU_identifier _ _ qual (pos, field) (_, name) =
      let fun_name = (pos, gen_fun_name field name) in
      Class_const (qual, fun_name)

    method! on_hint_ _ h =
      match h with
      | Hpu_access _ -> Hmixed
      | Hprim (Tatom _) -> Hprim Tstring
      | _ -> h
  end

(* Wrapper around the AST visitor *)
let visitor = new erase_body_visitor

let erase_stmt stmt = visitor#on_stmt () stmt

let erase_fun f = visitor#on_fun_ () f

let process_pufields class_name extends (pu_enums : T.pu_enum list) =
  List.concat_map ~f:(gen_pu_accessors class_name extends) pu_enums

let update_class c =
  let pu_methods =
    process_pufields
      (snd c.c_name)
      (not (List.is_empty c.c_extends))
      c.c_pu_enums
  in
  let c = visitor#on_class_ () c in
  { c with c_pu_enums = []; c_methods = c.c_methods @ pu_methods }

let update_def d =
  match d with
  | Class c -> Class (update_class c)
  | Stmt s -> Stmt (erase_stmt s)
  | Fun f -> Fun (erase_fun f)
  | RecordDef _
  | Typedef _
  | Constant _
  | Namespace _
  | NamespaceUse _
  | FileAttributes _
  | SetNamespaceEnv _ ->
    d

let translate (program : T.program) = List.map ~f:update_def program
