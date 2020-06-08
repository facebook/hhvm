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

(* Remark: the compilation of PU generates code that uses Memoization on
   non-final static methods.  Although in the general case this is unsafe,
   in our case either the receiver of the parent:: call is well-defined,
   or the call is dead-code because it is prevented by the PU type-checker,
   or the call is encapsulated by reflection.  To the best of our knowledge
   our use of Memoization is thus safe.
*)

let tany = Typing_defs.mk (Typing_reason.Rnone, Typing_defs.make_tany ())

module T = struct
  include Tast

  let tany pos = (pos, tany)
end

let pu_name_mangle instance_name name = "pu$" ^ instance_name ^ "$" ^ name

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

let process_class_enum pu_members =
  let f acc member =
    let { pum_atom; _ } = member in
    process_pumapping (snd pum_atom) acc member
  in
  let info = List.fold_left ~init:SMap.empty ~f pu_members in
  (* keep lists in the same order as their appear in the file *)
  SMap.map List.rev info

(* Helper functions to generate Hint nodes *)
let simple_hint pos name : T.hint = (pos, Happly ((pos, name), []))

let apply_to_hint pos name typ : T.hint = (pos, Happly ((pos, name), [typ]))

let type_hint pos name : T.type_hint = (tany, Some (simple_hint pos name))

let create_mixed_type_hint pos : T.type_hint = type_hint pos "\\HH\\mixed"

(* let create_void_type_hint pos : T.type_hint = type_hint pos "\\HH\\void" *)

let create_string_type_hint pos : T.type_hint = type_hint pos "\\HH\\string"

let create_key_set_string_type_hint pos : T.type_hint =
  let hstring = simple_hint pos "\\HH\\string" in
  let keyset_string = apply_to_hint pos "\\HH\\keyset" hstring in
  (tany, Some keyset_string)

(* Helper functions to generate Ast nodes *)
let id pos name : T.expr = (T.tany pos, Id (pos, name))

let class_id pos name : T.class_id = (T.tany pos, CIexpr (id pos name))

let class_const pos cls name : T.expr =
  (T.tany pos, Class_const (class_id pos cls, (pos, name)))

let call pos (caller : T.expr) (args : T.expr list) : T.expr =
  let tany = T.tany pos in
  (tany, Call (Aast.Cnormal, caller, [], args, None))

let lvar pos name : T.expr =
  (T.tany pos, Lvar (pos, Local_id.make_unscoped name))

let str pos name : T.expr = (T.tany pos, String name)

(* Get property from class *)
(* let class_get pos cls name : T.expr =
  (T.tany pos, Class_get (class_id pos cls, CGstring (pos, "$" ^ name))) *)

let new_ pos cls args : T.expr =
  let tany = T.tany pos in
  (tany, New (class_id pos cls, [], args, None, tany))

(* Get method from class *)
let obj_get pos var_name method_name : T.expr =
  (T.tany pos, Obj_get (lvar pos var_name, id pos method_name, OG_nullthrows))

let return pos expr : T.stmt = (pos, Return (Some expr))

let user_attribute pos name params : T.user_attribute =
  { ua_name = (pos, name); ua_params = params }

let memoize pos = user_attribute pos "__Memoize" []

let override pos = user_attribute pos "__Override" []

(* Error formatter *)
let error_msg cls instance_name name =
  Printf.sprintf "%s:@%s::%s unknown atom access: " cls instance_name name

(* Generate a static accessor function from the pumapping information, eg:

   <<__Memoize>>
   static function pu$Field_name$Expr_name(string $atom) : mixed {
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

   If the class extends a superclass, the raise statement is replaced with
   a call to parent::pu$Field_name$Expr_name.

   Remark: at compile time we cannot reliably detect if, whenever a class D
   extends a class C, a PU defined in the subclass D extends a PU defined in C
   or is self-contained.  In this case the accessor functions include the
   default call to parent.  Type checking ensures that the default case is
   not reachable if the PU was not inherited from the super-class.
*)
let gen_pu_accessor
    (fun_name : string)
    (pos : pos)
    (final : bool)
    (extends : bool)
    (info : (string * T.expr) list)
    (error : string) : T.method_ =
  let var_name = "$atom" in
  let var_atom = lvar pos var_name in
  let do_case entry init = Case (str pos entry, [return pos init]) in
  let cases =
    List.fold_right
      ~f:(fun (atom_name, value) acc -> do_case atom_name value :: acc)
      ~init:[]
      info
  in
  let default =
    if extends then
      let parent_call = class_const pos "parent" fun_name in
      let call = call pos parent_call [var_atom] in
      Default (pos, [return pos call])
    else
      let open Ast_defs in
      let msg = (T.tany pos, Binop (Dot, str pos error, var_atom)) in
      Default (pos, [(pos, Throw (new_ pos "\\Exception" [msg]))])
  in
  let cases = cases @ [default] in
  let body =
    { fb_ast = [(pos, Switch (var_atom, cases))]; fb_annotation = () }
  in
  {
    m_tparams = [];
    m_name = (pos, fun_name);
    m_params =
      [
        {
          param_annotation = T.tany pos;
          param_type_hint = create_string_type_hint pos;
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
    m_user_attributes = [memoize pos];
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

(* Generate a helper/debug function called Members which is a keyset
   of all the atoms declared in a ClassEnum.

   All PU classes get a new private static method called `Members` which
   returns a keyset<string> of all the instance names of a universe.

   If a class doesn't extend any other, the method directly
   returns the list of the instance names, available locally.

   If a class extends another one, we might need to look into it, so
   Members will do the recursive call, to correctly get them all.
   Since this can be expensive, we Memoize the function so it is done only
   once.
*)

(* As for accessors, we do not have a reliable way to detect, whenever a
   PU is defined in a subclass, if the PU is inherited from the superclass.
   We thus systematically perform a call to Members() in the parent class to
   check if there are instances of the PU defined in the superclass.
   As the Members method might not exist in the superclass, we encapsulate
   the call using reflection and catch the eventual exception.
   Reflection can be slow, but Memoization ensures that the class hierarchy
   is explored only once.

  <<__Memoize, __Override>>
  public static function pu$E$Members() : keyset<string> {
    $result = keyset[ .. strings based on local PU instances ...];
    $class = new ReflectionClass(parent::class);
    try {
      // might throw if the method is not in the parent class
      $method = $class->getMethod('pu$E$Members');
      // method is here, call it
      $parent_members = $method->invoke(null);
      foreach ($parent_members as $p) {
        $result[] = $p;
      }
    } catch (ReflectionException $_) {
      // not the right method: just use local info
    }
   return $result;
  }
*)
let gen_Members_extends instance_name pos (pu_members : T.pu_member list) =
  let tany = T.tany pos in
  let m_Members = pu_name_mangle instance_name "Members" in
  let mems =
    List.map ~f:(fun x -> AFvalue (str pos (snd x.pum_atom))) pu_members
  in
  let assign target expr =
    (pos, Expr (tany, Binop (Ast_defs.Eq None, target, expr)))
  in
  let assign_lvar target expr = assign (lvar pos target) expr in
  let body =
    {
      fb_ast =
        [
          (* $result = keyset[ ... names of local instances ... ] *)
          assign_lvar "$result" (tany, Collection ((pos, "keyset"), None, mems));
          (* $class = new ReflectionClass(parent::class) *)
          assign_lvar
            "$class"
            (new_ pos "ReflectionClass" [class_const pos "parent" "class"]);
          ( pos,
            (* try { *)
            Try
              ( [
                  (* $method = $class->getMethod('pu$E$Members'); *)
                  assign_lvar
                    "$method"
                    (call
                       pos
                       (obj_get pos "$class" "getMethod")
                       [str pos m_Members]);
                  (* $parent_members = $method->invoke(null); *)
                  assign_lvar
                    "$parent_members"
                    (call pos (obj_get pos "$method" "invoke") [(tany, Null)]);
                  (* foreach ($parent_members as $p) { $result[] = $p; } *)
                  ( pos,
                    Foreach
                      ( lvar pos "$parent_members",
                        As_v (lvar pos "$p"),
                        [
                          assign
                            (tany, Array_get (lvar pos "$result", None))
                            (lvar pos "$p");
                        ] ) );
                ],
                (* } catch (ReflectionException $_) { } *)
                [
                  ( (pos, "ReflectionException"),
                    (pos, Local_id.make_unscoped "$_"),
                    [] );
                ],
                [] ) );
          (* return $result; *)
          return pos (lvar pos "$result");
        ];
      fb_annotation = ();
    }
  in
  {
    m_visibility = Public;
    m_final = false;
    m_static = true;
    m_tparams = [];
    m_name = (pos, m_Members);
    m_params = [];
    m_body = body;
    m_user_attributes = [memoize pos; override pos];
    m_ret = create_key_set_string_type_hint pos;
    m_fun_kind = Ast_defs.FSync;
    m_span = pos;
    m_doc_comment = None;
    m_external = false;
    m_where_constraints = [];
    m_variadic = FVnonVariadic;
    m_annotation = T.dummy_saved_env;
    m_abstract = false;
  }

(*
 If the class doesn't extends another one, everything is available locally
  <<__Memoize>>
  public static function pu$E$Members() : keyset<string> {
    return keyset[ .. strings based on local PU instances ...];
   }
 *)
let gen_Members_no_extends instance_name pos (pu_members : T.pu_member list) =
  let tany = T.tany pos in
  let m_Members = pu_name_mangle instance_name "Members" in
  let mems =
    List.map ~f:(fun x -> AFvalue (str pos (snd x.pum_atom))) pu_members
  in
  let body =
    {
      fb_ast = [return pos (tany, Collection ((pos, "keyset"), None, mems))];
      fb_annotation = ();
    }
  in
  {
    m_visibility = Public;
    m_final = false;
    m_static = true;
    m_tparams = [];
    m_name = (pos, m_Members);
    m_params = [];
    m_body = body;
    m_user_attributes = [memoize pos];
    m_ret = create_key_set_string_type_hint pos;
    m_fun_kind = Ast_defs.FSync;
    m_span = pos;
    m_doc_comment = None;
    m_external = false;
    m_where_constraints = [];
    m_variadic = FVnonVariadic;
    m_annotation = T.dummy_saved_env;
    m_abstract = false;
  }

(* Returns the generated methods (accessors + Members) *)
let gen_pu_methods (class_name : string) (extends : bool) (instance : T.pu_enum)
    : T.method_ list =
  let pu_members = instance.pu_members in
  let (pos, instance_name) = instance.pu_name in
  let m_Members =
    if extends then
      gen_Members_extends instance_name pos pu_members
    else
      gen_Members_no_extends instance_name pos pu_members
  in
  let info = process_class_enum pu_members in
  let accessors =
    SMap.fold
      (fun expr_name info acc ->
        let fun_name = pu_name_mangle instance_name expr_name in
        let error = error_msg class_name instance_name expr_name in
        let hd =
          gen_pu_accessor fun_name pos instance.pu_is_final extends info error
        in
        hd :: acc)
      info
      []
  in
  let methods = m_Members :: accessors in
  methods
  |> List.sort ~compare:(fun a b ->
         String.compare (snd a.m_name) (snd b.m_name))

(* Instance of an AST visitor which:
   - updates PU_atom and PU_identifier

  This only performs erasure; generation of code is done in a second pass
*)
class ['self] erase_body_visitor =
  object (_self : 'self)
    inherit [_] endo as super

    method on_'fb _env fb = fb

    method on_'ex _env ex = ex

    method on_'en _env en = en

    method on_'hi _env hi = hi

    method! on_PU_atom _ _ s = String s

    method! on_PU_identifier _ _ qual (pos, enum) (_, name) =
      let fun_name = (pos, pu_name_mangle enum name) in
      Class_const (qual, fun_name)

    method! on_hint env (p, h) =
      match h with
      | Hpu_access _ -> simple_hint p "\\HH\\mixed"
      | Hprim (Tatom _) -> (p, Hprim Tstring)
      | _ -> super#on_hint env (p, h)
  end

(* Wrapper around the AST visitor *)
let visitor = new erase_body_visitor

let erase_stmt stmt = visitor#on_stmt () stmt

let erase_fun f = visitor#on_fun_ () f

let process_pufields class_name extends (pu_enums : T.pu_enum list) =
  List.concat_map ~f:(gen_pu_methods class_name extends) pu_enums

let update_class c =
  let methods =
    process_pufields
      (snd c.c_name)
      (not (List.is_empty c.c_extends))
      c.c_pu_enums
  in
  let c = visitor#on_class_ () c in
  { c with c_pu_enums = []; c_methods = c.c_methods @ methods }

let update_def d =
  match d with
  | Class c ->
    let c = update_class c in
    Class c
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
