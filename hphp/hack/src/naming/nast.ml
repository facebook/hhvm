(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
module SN = Naming_special_names

type func_body_ann =
  | Named
  | NamedWithUnsafeBlocks
  (* Namespace info *)
  | Unnamed of Namespace_env.env
[@@deriving eq]

let show_func_body_ann = function
  | Named -> "Named"
  | NamedWithUnsafeBlocks -> "NamedWithUnsafeBlocks"
  | Unnamed _ -> "Unnamed"

let pp_func_body_ann fmt fba =
  Format.pp_print_string fmt (show_func_body_ann fba)

let show_tprim =
  Naming_special_names.Typehints.(
    function
    | Tnull -> null
    | Tvoid -> void
    | Tint -> int
    | Tbool -> bool
    | Tfloat -> float
    | Tstring -> string
    | Tnum -> num
    | Tresource -> resource
    | Tarraykey -> arraykey
    | Tnoreturn -> noreturn)

type program = (Pos.t, func_body_ann, unit, unit) Aast.program [@@deriving show]

type def = (Pos.t, func_body_ann, unit, unit) Aast.def

type expr = (Pos.t, func_body_ann, unit, unit) Aast.expr [@@deriving eq, show]

type expr_ = (Pos.t, func_body_ann, unit, unit) Aast.expr_

type stmt = (Pos.t, func_body_ann, unit, unit) Aast.stmt

type block = (Pos.t, func_body_ann, unit, unit) Aast.block

type user_attribute = (Pos.t, func_body_ann, unit, unit) Aast.user_attribute
[@@deriving eq, show]

type class_id_ = (Pos.t, func_body_ann, unit, unit) Aast.class_id_
[@@deriving eq]

type class_ = (Pos.t, func_body_ann, unit, unit) Aast.class_

type class_var = (Pos.t, func_body_ann, unit, unit) Aast.class_var

type method_ = (Pos.t, func_body_ann, unit, unit) Aast.method_

type file_attribute = (Pos.t, func_body_ann, unit, unit) Aast.file_attribute

type fun_ = (Pos.t, func_body_ann, unit, unit) Aast.fun_

type fun_def = (Pos.t, func_body_ann, unit, unit) Aast.fun_def

type func_body = (Pos.t, func_body_ann, unit, unit) Aast.func_body

type fun_param = (Pos.t, func_body_ann, unit, unit) Aast.fun_param

type fun_variadicity = (Pos.t, func_body_ann, unit, unit) Aast.fun_variadicity

type typedef = (Pos.t, func_body_ann, unit, unit) Aast.typedef

type record_def = (Pos.t, func_body_ann, unit, unit) Aast.record_def

type tparam = (Pos.t, func_body_ann, unit, unit) Aast.tparam

type gconst = (Pos.t, func_body_ann, unit, unit) Aast.gconst

type class_const = (Pos.t, func_body_ann, unit, unit) Aast.class_const

type class_id = (Pos.t, func_body_ann, unit, unit) Aast.class_id

type catch = (Pos.t, func_body_ann, unit, unit) Aast.catch

type case = (Pos.t, func_body_ann, unit, unit) Aast.case

type field = (Pos.t, func_body_ann, unit, unit) Aast.field

type afield = (Pos.t, func_body_ann, unit, unit) Aast.afield

type expression_tree = (Pos.t, func_body_ann, unit, unit) Aast.expression_tree

type targ = unit Aast.targ

type sid = Aast.sid [@@deriving show]

type shape_field_name = Ast_defs.shape_field_name

type class_hint = Aast.class_hint

type trait_hint = Aast.trait_hint

type record_hint = Aast.record_hint

type xhp_attr_hint = Aast.xhp_attr_hint

type type_hint = unit Aast.type_hint

module ShapeMap = Ast_defs.ShapeMap

(* Expecting that Naming.func_body / Naming.class_meth_bodies has been
 * allowed at the AST. Ideally this would be enforced by the compiler,
 * a la the typechecking decl vs local phases *)
let is_body_named fb =
  match fb.fb_annotation with
  | Named
  | NamedWithUnsafeBlocks ->
    true
  | Unnamed _ -> false

let assert_named_body fb =
  if is_body_named fb then
    fb
  else
    failwith "Expecting a named function body"

let named_body_is_unsafe fb =
  match fb.fb_annotation with
  | Named -> false
  | NamedWithUnsafeBlocks -> true
  | Unnamed _ -> failwith "Expecting a named function body"

let class_id_to_str = function
  | CIparent -> SN.Classes.cParent
  | CIself -> SN.Classes.cSelf
  | CIstatic -> SN.Classes.cStatic
  | CIexpr (_, This) -> SN.SpecialIdents.this
  | CIexpr (_, Lvar (_, x)) -> "$" ^ Local_id.to_string x
  | CIexpr _ -> assert false
  | CI (_, x) -> x

let is_kvc_kind name =
  String.equal name SN.Collections.cMap
  || String.equal name SN.Collections.cImmMap
  || String.equal name SN.Collections.cDict

let get_kvc_kind name =
  match name with
  | x when String.equal x SN.Collections.cMap -> Map
  | x when String.equal x SN.Collections.cImmMap -> ImmMap
  | x when String.equal x SN.Collections.cDict -> Dict
  | _ ->
    Errors.internal_error Pos.none ("Invalid KeyValueCollection name: " ^ name);
    Map

let kvc_kind_to_name kind =
  match kind with
  | Map -> SN.Collections.cMap
  | ImmMap -> SN.Collections.cImmMap
  | Dict -> SN.Collections.cDict

let is_vc_kind name =
  String.equal name SN.Collections.cVector
  || String.equal name SN.Collections.cImmVector
  || String.equal name SN.Collections.cSet
  || String.equal name SN.Collections.cImmSet
  || String.equal name SN.Collections.cKeyset
  || String.equal name SN.Collections.cVec

let get_vc_kind name =
  match name with
  | x when String.equal x SN.Collections.cVector -> Vector
  | x when String.equal x SN.Collections.cImmVector -> ImmVector
  | x when String.equal x SN.Collections.cVec -> Vec
  | x when String.equal x SN.Collections.cSet -> Set
  | x when String.equal x SN.Collections.cImmSet -> ImmSet
  | x when String.equal x SN.Collections.cKeyset -> Keyset
  | _ ->
    Errors.internal_error Pos.none ("Invalid ValueCollection name: " ^ name);
    Set

let vc_kind_to_name kind =
  match kind with
  | Vector -> SN.Collections.cVector
  | ImmVector -> SN.Collections.cImmVector
  | Vec -> SN.Collections.cVec
  | Set -> SN.Collections.cSet
  | ImmSet -> SN.Collections.cImmSet
  | Keyset -> SN.Collections.cKeyset

(* XHP attribute helpers *)
let map_xhp_attr (f : pstring -> pstring) (g : expr -> expr) = function
  | Xhp_simple { xs_name = id; xs_type; xs_expr = e } ->
    Xhp_simple { xs_name = f id; xs_type; xs_expr = g e }
  | Xhp_spread e -> Xhp_spread (g e)

let get_xhp_attr_expr = function
  | Xhp_simple { xs_expr = e; _ }
  | Xhp_spread e ->
    e

let get_simple_xhp_attrs =
  List.filter_map ~f:(function
      | Xhp_simple { xs_name = id; xs_expr = e; _ } -> Some (id, e)
      | Xhp_spread _ -> None)

(* Given a Nast.program, give me the list of entities it defines *)
let get_defs ast =
  (* fold_right traverses the file from top to bottom, and as such gives nicer
   * error messages than fold_left. E.g. in the case where a function is
   * declared twice in the same file, the error will say that the declaration
   * with the larger line number is a duplicate. *)
  let rec get_defs ast acc =
    List.fold_right
      ast
      ~init:acc
      ~f:(fun def ((acc1, acc2, acc3, acc4, acc5) as acc) ->
        Aast.(
          match def with
          | Fun f ->
            (FileInfo.pos_full f.fd_fun.f_name :: acc1, acc2, acc3, acc4, acc5)
          | Class c ->
            (acc1, FileInfo.pos_full c.c_name :: acc2, acc3, acc4, acc5)
          | RecordDef rd ->
            (acc1, acc2, FileInfo.pos_full rd.rd_name :: acc3, acc4, acc5)
          | Typedef t ->
            (acc1, acc2, acc3, FileInfo.pos_full t.t_name :: acc4, acc5)
          | Constant cst ->
            (acc1, acc2, acc3, acc4, FileInfo.pos_full cst.cst_name :: acc5)
          | Namespace (_, defs) -> get_defs defs acc
          | NamespaceUse _
          | SetNamespaceEnv _ ->
            acc
          (* toplevel statements are ignored *)
          | FileAttributes _
          | Stmt _ ->
            acc))
  in
  get_defs ast ([], [], [], [], [])

type ignore_attribute_env = { ignored_attributes: string list }

(** Some utility functions **)

let ast_deregister_attributes_mapper =
  object (self)
    inherit [_] Aast.endo as super

    method on_'fb _ (fb : func_body_ann) = fb

    method on_'ex _ (ex : pos) = ex

    method on_'en _ (en : unit) = en

    method on_'hi _ (hi : unit) = hi

    method ignored_attr env l =
      List.exists l ~f:(fun attr ->
          List.mem env.ignored_attributes (snd attr.ua_name) ~equal:String.equal)

    (* Filter all functions and classes with the user attributes banned *)
    method! on_program env toplevels =
      let toplevels =
        List.filter toplevels ~f:(fun toplevel ->
            match toplevel with
            | Fun f when self#ignored_attr env f.fd_fun.f_user_attributes ->
              false
            | Class c when self#ignored_attr env c.c_user_attributes -> false
            | _ -> true)
      in
      super#on_program env toplevels

    method! on_class_ env this =
      (* Filter out class elements which are methods with wrong attributes *)
      let methods =
        List.filter this.c_methods ~f:(fun m ->
            not @@ self#ignored_attr env m.m_user_attributes)
      in
      let cvars =
        List.filter this.c_vars ~f:(fun cv ->
            not @@ self#ignored_attr env cv.cv_user_attributes)
      in
      let this = { this with c_methods = methods; c_vars = cvars } in
      super#on_class_ env this
  end

let deregister_ignored_attributes ast =
  let env =
    {
      (* For now, only ignore the __PHPStdLib *)
      ignored_attributes = [Naming_special_names.UserAttributes.uaPHPStdLib];
    }
  in
  ast_deregister_attributes_mapper#on_program env ast

let ast_no_pos_or_docblock_mapper =
  object
    inherit [_] Aast.endo as super

    method! on_pos _ _pos = Pos.none

    method on_'fb _ (annot : func_body_ann) = annot

    method on_'ex _ _ex = Pos.none

    method on_'en _ (en : unit) = en

    method on_'hi _ (hi : unit) = hi

    method! on_fun_ env f = super#on_fun_ env { f with f_doc_comment = None }

    method! on_class_ env c =
      super#on_class_ env { c with c_doc_comment = None }

    method! on_class_var env cv =
      super#on_class_var env { cv with cv_doc_comment = None }

    method! on_method_ env m =
      super#on_method_ env { m with m_doc_comment = None }

    method! on_class_const env ccs =
      super#on_class_const env { ccs with cc_doc_comment = None }

    method! on_class_typeconst_def env tc =
      super#on_class_typeconst_def env { tc with c_tconst_doc_comment = None }

    (* Skip all blocks because we don't care about method bodies *)
    method! on_block _ _ = []
  end

(* Given an AST, return an AST with no position or docblock info *)
let remove_pos_and_docblock ast =
  ast_no_pos_or_docblock_mapper#on_program () ast

(* Given an AST, generate a unique hash for its decl tree. *)
let generate_ast_decl_hash ast =
  (* Why we marshal it into a string first: regular Hashtbl.hash will
    collide improperly because it doesn't compare ADTs with strings correctly.
    Using Marshal, we guarantee that the two ASTs are represented by a single
    primitive type, which we hash.
  *)
  let str = Marshal.to_string (remove_pos_and_docblock ast) [] in
  OpaqueDigest.string str

(*****************************************************************************)
(** This module defines a visitor class on the Nast data structure.
    To use it you must inherit the generic object and redefine the appropriate
    methods.

    It has been deprecated because it contains holes and needs to be updated
    manually. Please use the autogenerated visitors instead (e.g., {!Nast.iter},
    {!Nast.reduce}).

    @see <https://gitlab.inria.fr/fpottier/visitors> Visitor generation plugin
    @see <http://gallium.inria.fr/~fpottier/visitors/manual.pdf> Visitors docs

    To convert a visitor using this deprecated base class to the autogenerated
    visitors, you will likely want to use either {!Nast.iter} with a mutable
    result member or {!Nast.reduce}.

    For example, this visitor:

        let has_return_visitor = object
          inherit [bool] Nast.Visitor_DEPRECATED.visitor
          method! on_return _ _ _ = true
        end

        let has_return block =
          has_return_visitor#on_block false block

    Could be written this way:

        class has_return_visitor = object (_ : 'self)
          inherit [_] Nast.iter
          val mutable result = false
          method result = result
          method! on_Return () _ _ = result <- true
        end

        let has_return block =
          let visitor = new has_return_visitor in
          visitor#on_block () block;
          visitor#result

    But it would be even better to use a reduce visitor:

        let has_return_visitor = object (_ : 'self)
          inherit [_] Nast.reduce
          method zero = false
          method plus = (||)
          method! on_Return () _ _ = true
        end

        let has_return block =
          has_return_visitor#on_block () block
*)

(*****************************************************************************)
module Visitor_DEPRECATED = struct
  (*****************************************************************************)
  (* The signature of the visitor. *)
  (*****************************************************************************)
  type id = lid

  class type ['a] visitor_type =
    object
      method on_block : 'a -> block -> 'a

      method on_break : 'a -> 'a

      method on_case : 'a -> case -> 'a

      method on_catch : 'a -> catch -> 'a

      method on_continue : 'a -> 'a

      method on_darray : 'a -> (targ * targ) option -> field list -> 'a

      method on_varray : 'a -> targ option -> expr list -> 'a

      method on_do : 'a -> block -> expr -> 'a

      method on_expr : 'a -> expr -> 'a

      method on_expr_ : 'a -> expr_ -> 'a

      method on_for : 'a -> expr list -> expr option -> expr list -> block -> 'a

      method on_foreach :
        'a -> expr -> (Pos.t, func_body_ann, unit, unit) as_expr -> block -> 'a

      method on_if : 'a -> expr -> block -> block -> 'a

      method on_noop : 'a -> 'a

      method on_fallthrough : 'a -> 'a

      method on_return : 'a -> expr option -> 'a

      method on_awaitall : 'a -> (id option * expr) list -> block -> 'a

      method on_stmt : 'a -> stmt -> 'a

      method on_stmt_ : 'a -> (Pos.t, func_body_ann, unit, unit) stmt_ -> 'a

      method on_switch : 'a -> expr -> case list -> 'a

      method on_throw : 'a -> expr -> 'a

      method on_try : 'a -> block -> catch list -> block -> 'a

      method on_while : 'a -> expr -> block -> 'a

      method on_using :
        'a -> (Pos.t, func_body_ann, unit, unit) using_stmt -> 'a

      method on_as_expr : 'a -> (Pos.t, func_body_ann, unit, unit) as_expr -> 'a

      method on_shape : 'a -> (Ast_defs.shape_field_name * expr) list -> 'a

      method on_valCollection : 'a -> vc_kind -> targ option -> expr list -> 'a

      method on_keyValCollection :
        'a -> kvc_kind -> (targ * targ) option -> field list -> 'a

      method on_collection :
        'a -> unit collection_targ option -> afield list -> 'a

      method on_this : 'a -> 'a

      method on_id : 'a -> sid -> 'a

      method on_lvar : 'a -> id -> 'a

      method on_dollardollar : 'a -> id -> 'a

      method on_fun_id : 'a -> sid -> 'a

      method on_method_id : 'a -> expr -> pstring -> 'a

      method on_smethod_id : 'a -> class_id -> pstring -> 'a

      method on_method_caller : 'a -> sid -> pstring -> 'a

      method on_obj_get : 'a -> expr -> expr -> 'a

      method on_array_get : 'a -> expr -> expr option -> 'a

      method on_class_get :
        'a ->
        class_id ->
        (Pos.t, func_body_ann, unit, unit) class_get_expr ->
        'a

      method on_class_const : 'a -> class_id -> pstring -> 'a

      method on_call : 'a -> expr -> expr list -> expr option -> 'a

      method on_function_pointer :
        'a ->
        (Pos.t, func_body_ann, unit, unit) function_ptr_id ->
        targ list ->
        'a

      method on_true : 'a -> 'a

      method on_false : 'a -> 'a

      method on_int : 'a -> string -> 'a

      method on_float : 'a -> string -> 'a

      method on_null : 'a -> 'a

      method on_string : 'a -> string -> 'a

      method on_string2 : 'a -> expr list -> 'a

      method on_yield_break : 'a -> 'a

      method on_yield : 'a -> afield -> 'a

      method on_await : 'a -> expr -> 'a

      method on_list : 'a -> expr list -> 'a

      method on_pair : 'a -> (targ * targ) option -> expr -> expr -> 'a

      method on_cast : 'a -> hint -> expr -> 'a

      method on_expression_tree : 'a -> expression_tree -> 'a

      method on_unop : 'a -> Ast_defs.uop -> expr -> 'a

      method on_binop : 'a -> Ast_defs.bop -> expr -> expr -> 'a

      method on_pipe : 'a -> id -> expr -> expr -> 'a

      method on_eif : 'a -> expr -> expr option -> expr -> 'a

      method on_is : 'a -> expr -> hint -> 'a

      method on_as : 'a -> expr -> hint -> bool -> 'a

      method on_class_id : 'a -> class_id -> 'a

      method on_class_id_ : 'a -> class_id_ -> 'a

      method on_new : 'a -> class_id -> expr list -> expr option -> 'a

      method on_record : 'a -> sid -> (expr * expr) list -> 'a

      method on_efun : 'a -> fun_ -> id list -> 'a

      method on_lfun : 'a -> fun_ -> id list -> 'a

      method on_xml :
        'a ->
        sid ->
        (Pos.t, func_body_ann, unit, unit) xhp_attribute list ->
        expr list ->
        'a

      method on_param_kind : 'a -> Ast_defs.param_kind -> 'a

      method on_callconv : 'a -> Ast_defs.param_kind -> expr -> 'a

      method on_clone : 'a -> expr -> 'a

      method on_field : 'a -> field -> 'a

      method on_afield : 'a -> afield -> 'a

      method on_class_typeconst_def :
        'a -> (Pos.t, func_body_ann, unit, unit) class_typeconst_def -> 'a

      method on_class_c_const : 'a -> class_const -> 'a

      method on_class_var : 'a -> class_var -> 'a

      method on_class_use : 'a -> hint -> 'a

      method on_class_req : 'a -> hint * bool -> 'a

      method on_func_named_body : 'a -> func_body -> 'a

      method on_func_unnamed_body : 'a -> func_body -> 'a

      method on_func_body : 'a -> func_body -> 'a

      method on_method_ : 'a -> method_ -> 'a

      method on_fun_def : 'a -> fun_def -> 'a

      method on_fun_ : 'a -> fun_ -> 'a

      method on_class_ : 'a -> class_ -> 'a

      method on_record_def : 'a -> record_def -> 'a

      method on_gconst : 'a -> gconst -> 'a

      method on_typedef : 'a -> typedef -> 'a

      method on_hint : 'a -> hint -> 'a

      method on_type_hint : 'a -> type_hint -> 'a

      method on_targ : 'a -> targ -> 'a

      method on_def : 'a -> def -> 'a

      method on_program : 'a -> program -> 'a

      method on_markup : 'a -> pstring -> 'a

      method on_enum_class_label : 'a -> sid option -> string -> 'a

      method on_function_ptr_id :
        'a -> (Pos.t, func_body_ann, unit, unit) function_ptr_id -> 'a

      method on_et_splice : 'a -> expr -> 'a

      method on_readonly_expr : 'a -> expr -> 'a
    end

  (*****************************************************************************)
  (* The generic visitor ('a is the type of the accumulator). *)
  (*****************************************************************************)

  class virtual ['a] visitor : ['a] visitor_type =
    object (this)
      method on_break acc = acc

      method on_continue acc = acc

      method on_noop acc = acc

      method on_fallthrough acc = acc

      method on_markup acc _ = acc

      method on_throw acc e =
        let acc = this#on_expr acc e in
        acc

      method on_return acc eopt =
        match eopt with
        | None -> acc
        | Some e -> this#on_expr acc e

      method on_awaitall acc el b =
        let acc =
          List.fold_left
            ~f:(fun acc (x, y) ->
              let acc =
                match x with
                | Some x -> this#on_lvar acc x
                | None -> acc
              in
              let acc = this#on_expr acc y in
              acc)
            ~init:acc
            el
        in
        let acc = this#on_block acc b in
        acc

      method on_if acc e b1 b2 =
        let acc = this#on_expr acc e in
        let acc = this#on_block acc b1 in
        let acc = this#on_block acc b2 in
        acc

      method on_do acc b e =
        let acc = this#on_block acc b in
        let acc = this#on_expr acc e in
        acc

      method on_while acc e b =
        let acc = this#on_expr acc e in
        let acc = this#on_block acc b in
        acc

      method on_using acc us =
        let acc = List.fold_left (snd us.us_exprs) ~f:this#on_expr ~init:acc in
        let acc = this#on_block acc us.us_block in
        acc

      method on_for acc e1 e2 e3 b =
        let on_expr_list acc es = List.fold_left es ~f:this#on_expr ~init:acc in

        let acc = on_expr_list acc e1 in
        let acc = on_expr_list acc e3 in
        let acc =
          match e2 with
          | None -> acc
          | Some e -> this#on_expr acc e
        in
        let acc = this#on_block acc b in
        acc

      method on_switch acc e cl =
        let acc = this#on_expr acc e in
        let acc = List.fold_left cl ~f:this#on_case ~init:acc in
        acc

      method on_foreach acc e ae b =
        let acc = this#on_expr acc e in
        let acc = this#on_as_expr acc ae in
        let acc = this#on_block acc b in
        acc

      method on_try acc b cl fb =
        let acc = this#on_block acc b in
        let acc = List.fold_left cl ~f:this#on_catch ~init:acc in
        let acc = this#on_block acc fb in
        acc

      method on_block acc b = List.fold_left b ~f:this#on_stmt ~init:acc

      method on_case acc =
        function
        | Default (_, b) ->
          let acc = this#on_block acc b in
          acc
        | Case (e, b) ->
          let acc = this#on_expr acc e in
          let acc = this#on_block acc b in
          acc

      method on_as_expr acc =
        function
        | As_v e
        | Await_as_v (_, e) ->
          let acc = this#on_expr acc e in
          acc
        | As_kv (e1, e2)
        | Await_as_kv (_, e1, e2) ->
          let acc = this#on_expr acc e1 in
          let acc = this#on_expr acc e2 in
          acc

      method on_catch acc (_, _, b) = this#on_block acc b

      method on_stmt acc (_, stmt) = this#on_stmt_ acc stmt

      method on_stmt_ acc =
        function
        | Expr e -> this#on_expr acc e
        | Break -> this#on_break acc
        | Continue -> this#on_continue acc
        | Throw e -> this#on_throw acc e
        | Return eopt -> this#on_return acc eopt
        | Yield_break -> this#on_yield_break acc
        | If (e, b1, b2) -> this#on_if acc e b1 b2
        | Do (b, e) -> this#on_do acc b e
        | While (e, b) -> this#on_while acc e b
        | Using us -> this#on_using acc us
        | For (e1, e2, e3, b) -> this#on_for acc e1 e2 e3 b
        | Switch (e, cl) -> this#on_switch acc e cl
        | Foreach (e, ae, b) -> this#on_foreach acc e ae b
        | Try (b, cl, fb) -> this#on_try acc b cl fb
        | Noop -> this#on_noop acc
        | Fallthrough -> this#on_fallthrough acc
        | Awaitall (el, b) -> this#on_awaitall acc el b
        | Block b -> this#on_block acc b
        | Markup s -> this#on_markup acc s
        | AssertEnv _ -> this#on_noop acc

      method on_expr acc (_, e) = this#on_expr_ acc e

      method on_expr_ acc e =
        match e with
        | Darray (tap, fieldl) -> this#on_darray acc tap fieldl
        | Varray (ta, el) -> this#on_varray acc ta el
        | Shape sh -> this#on_shape acc sh
        | True -> this#on_true acc
        | False -> this#on_false acc
        | Int n -> this#on_int acc n
        | Float n -> this#on_float acc n
        | Null -> this#on_null acc
        | String s -> this#on_string acc s
        | This -> this#on_this acc
        | Id sid -> this#on_id acc sid
        | Lplaceholder _pos -> acc
        | Dollardollar id -> this#on_dollardollar acc id
        | Lvar id -> this#on_lvar acc id
        | Fun_id sid -> this#on_fun_id acc sid
        | Method_id (expr, pstr) -> this#on_method_id acc expr pstr
        | Method_caller (sid, pstr) -> this#on_method_caller acc sid pstr
        | Smethod_id (cid, pstr) -> this#on_smethod_id acc cid pstr
        | Yield e -> this#on_yield acc e
        | Await e -> this#on_await acc e
        | Tuple el -> this#on_list acc el
        | List el -> this#on_list acc el
        | Clone e -> this#on_clone acc e
        | Obj_get (e1, e2, _, _) -> this#on_obj_get acc e1 e2
        | Array_get (e1, e2) -> this#on_array_get acc e1 e2
        | Class_get (cid, e, _) -> this#on_class_get acc cid e
        | Class_const (cid, id) -> this#on_class_const acc cid id
        | Call (e, _, el, unpacked_element) ->
          this#on_call acc e el unpacked_element
        | FunctionPointer (fpid, targs) ->
          this#on_function_pointer acc fpid targs
        | String2 el -> this#on_string2 acc el
        | PrefixedString (_, e) -> this#on_expr acc e
        | Pair (ta, e1, e2) -> this#on_pair acc ta e1 e2
        | Cast (hint, e) -> this#on_cast acc hint e
        | ExpressionTree et -> this#on_expression_tree acc et
        | Unop (uop, e) -> this#on_unop acc uop e
        | Binop (bop, e1, e2) -> this#on_binop acc bop e1 e2
        | Pipe (id, e1, e2) -> this#on_pipe acc id e1 e2
        | Eif (e1, e2, e3) -> this#on_eif acc e1 e2 e3
        | Is (e, h) -> this#on_is acc e h
        | As (e, h, b) -> this#on_as acc e h b
        | New (cid, _, el, unpacked_element, _) ->
          this#on_new acc cid el unpacked_element
        | Efun (f, idl) -> this#on_efun acc f idl
        | Record (cid, fl) -> this#on_record acc cid fl
        | Xml (sid, attrl, el) -> this#on_xml acc sid attrl el
        | Callconv (kind, e) -> this#on_callconv acc kind e
        | ValCollection (s, ta, el) -> this#on_valCollection acc s ta el
        | KeyValCollection (s, tap, fl) -> this#on_keyValCollection acc s tap fl
        | Omitted -> acc
        | Lfun (f, idl) -> this#on_lfun acc f idl
        | Import (_, e) -> this#on_expr acc e
        | Collection (_, tal, fl) -> this#on_collection acc tal fl
        | ET_Splice e -> this#on_et_splice acc e
        | EnumClassLabel (opt_sid, name) ->
          this#on_enum_class_label acc opt_sid name
        | ReadonlyExpr e -> this#on_readonly_expr acc e
        | Hole (e, _, _, _) -> this#on_expr acc e

      method on_collection acc tal afl =
        let acc =
          match tal with
          | Some (CollectionTKV (tk, tv)) ->
            let acc = this#on_targ acc tk in
            let acc = this#on_targ acc tv in
            acc
          | Some (CollectionTV tv) -> this#on_targ acc tv
          | None -> acc
        in
        List.fold_left afl ~f:this#on_afield ~init:acc

      method on_shape acc sm =
        List.fold_left
          ~f:
            begin
              fun acc (_, e) ->
              let acc = this#on_expr acc e in
              acc
            end
          ~init:acc
          sm

      method on_darray acc tap fieldl =
        let acc =
          match tap with
          | Some (t1, t2) ->
            let acc = this#on_targ acc t1 in
            let acc = this#on_targ acc t2 in
            acc
          | None -> acc
        in
        List.fold_left fieldl ~f:this#on_field ~init:acc

      method on_varray acc ta el =
        let acc =
          match ta with
          | Some t -> this#on_targ acc t
          | None -> acc
        in
        List.fold_left el ~f:this#on_expr ~init:acc

      method on_valCollection acc _ ta el =
        let acc =
          match ta with
          | Some t -> this#on_targ acc t
          | None -> acc
        in
        List.fold_left el ~f:this#on_expr ~init:acc

      method on_keyValCollection acc _ tap fieldl =
        let acc =
          match tap with
          | Some (t1, t2) ->
            let acc = this#on_targ acc t1 in
            let acc = this#on_targ acc t2 in
            acc
          | None -> acc
        in
        List.fold_left fieldl ~f:this#on_field ~init:acc

      method on_this acc = acc

      method on_id acc _ = acc

      method on_lvar acc _ = acc

      method on_dollardollar acc id = this#on_lvar acc id

      method on_fun_id acc _ = acc

      method on_method_id acc _ _ = acc

      method on_smethod_id acc _ _ = acc

      method on_method_caller acc _ _ = acc

      method on_obj_get acc e1 e2 =
        let acc = this#on_expr acc e1 in
        let acc = this#on_expr acc e2 in
        acc

      method on_array_get acc e e_opt =
        let acc = this#on_expr acc e in
        let acc =
          match e_opt with
          | None -> acc
          | Some e -> this#on_expr acc e
        in
        acc

      method on_class_get acc cid e =
        let acc = this#on_class_id acc cid in
        match e with
        | CGstring _ -> acc
        | CGexpr e -> this#on_expr acc e

      method on_class_const acc cid _ = this#on_class_id acc cid

      method on_call acc e el unpacked_element =
        let acc = this#on_expr acc e in
        let acc = List.fold_left el ~f:this#on_expr ~init:acc in
        let acc =
          Option.value_map unpacked_element ~f:(this#on_expr acc) ~default:acc
        in
        acc

      method on_function_pointer acc e targs =
        let acc = this#on_function_ptr_id acc e in
        let acc = List.fold_left targs ~f:this#on_targ ~init:acc in
        acc

      method on_true acc = acc

      method on_false acc = acc

      method on_int acc _ = acc

      method on_float acc _ = acc

      method on_null acc = acc

      method on_string acc _ = acc

      method on_string2 acc el =
        let acc = List.fold_left el ~f:this#on_expr ~init:acc in
        acc

      method on_yield_break acc = acc

      method on_yield acc e = this#on_afield acc e

      method on_await acc e = this#on_expr acc e

      method on_list acc el = List.fold_left el ~f:this#on_expr ~init:acc

      method on_pair acc tap e1 e2 =
        let acc =
          match tap with
          | Some (t1, t2) ->
            let acc = this#on_targ acc t1 in
            let acc = this#on_targ acc t2 in
            acc
          | None -> acc
        in
        let acc = this#on_expr acc e1 in
        let acc = this#on_expr acc e2 in
        acc

      method on_cast acc _ e = this#on_expr acc e

      method on_expression_tree acc (et : expression_tree) =
        let acc = this#on_hint acc et.et_hint in
        let acc = this#on_block acc et.et_splices in
        let acc = this#on_expr acc et.et_virtualized_expr in
        let acc = this#on_expr acc et.et_runtime_expr in
        acc

      method on_unop acc _ e = this#on_expr acc e

      method on_binop acc _ e1 e2 =
        let acc = this#on_expr acc e1 in
        let acc = this#on_expr acc e2 in
        acc

      method on_pipe acc _id e1 e2 =
        let acc = this#on_expr acc e1 in
        let acc = this#on_expr acc e2 in
        acc

      method on_eif acc e1 e2 e3 =
        let acc = this#on_expr acc e1 in
        let acc =
          match e2 with
          | None -> acc
          | Some e -> this#on_expr acc e
        in
        let acc = this#on_expr acc e3 in
        acc

      method on_is acc e _ = this#on_expr acc e

      method on_as acc e _ _ = this#on_expr acc e

      method on_class_id acc (_, cid) = this#on_class_id_ acc cid

      method on_class_id_ acc =
        function
        | CIexpr e -> this#on_expr acc e
        | _ -> acc

      method on_new acc cid el unpacked_element =
        let acc = this#on_class_id acc cid in
        let acc = List.fold_left el ~f:this#on_expr ~init:acc in
        let acc =
          Option.value_map unpacked_element ~default:acc ~f:(this#on_expr acc)
        in
        acc

      method on_efun acc f _ =
        if is_body_named f.f_body then
          this#on_block acc f.f_body.fb_ast
        else
          failwith
            "lambdas expected to be named in the context of the surrounding function"

      method on_lfun acc f _ =
        if is_body_named f.f_body then
          this#on_block acc f.f_body.fb_ast
        else
          failwith
            "lambdas expected to be named in the context of the surrounding function"

      method on_record acc _ fl = List.fold_left fl ~f:this#on_field ~init:acc

      method on_xml acc _ attrl el =
        let acc =
          List.fold_left attrl ~init:acc ~f:(fun acc attr ->
              match attr with
              | Xhp_simple { xs_expr = e; _ }
              | Xhp_spread e ->
                this#on_expr acc e)
        in
        let acc = List.fold_left el ~f:this#on_expr ~init:acc in
        acc

      method on_param_kind acc _ = acc

      method on_callconv acc kind e =
        let acc = this#on_param_kind acc kind in
        let acc = this#on_expr acc e in
        acc

      method on_clone acc e = this#on_expr acc e

      method on_field acc (e1, e2) =
        let acc = this#on_expr acc e1 in
        let acc = this#on_expr acc e2 in
        acc

      method on_afield acc =
        function
        | AFvalue e -> this#on_expr acc e
        | AFkvalue (e1, e2) ->
          let acc = this#on_expr acc e1 in
          let acc = this#on_expr acc e2 in
          acc

      method on_hint acc _ = acc

      method on_type_hint acc _ = acc

      method on_targ acc _ = acc

      method on_fun_ acc f =
        let acc = this#on_id acc f.f_name in
        let acc = this#on_func_body acc f.f_body in
        let acc =
          match hint_of_type_hint f.f_ret with
          | Some h -> this#on_hint acc h
          | None -> acc
        in
        acc

      method on_func_named_body acc fnb = this#on_block acc fnb.fb_ast

      method on_func_unnamed_body acc _ = acc

      method on_func_body acc fb =
        if is_body_named fb then
          this#on_func_named_body acc fb
        (* No action on unnamed body *)
        else
          acc

      method on_method_ acc m =
        let acc = this#on_id acc m.m_name in
        let acc = this#on_func_body acc m.m_body in
        acc

      method on_class_ acc c =
        let acc = this#on_id acc c.c_name in
        let acc = List.fold_left c.c_extends ~f:this#on_hint ~init:acc in
        let acc = List.fold_left c.c_uses ~f:this#on_hint ~init:acc in
        let acc = List.fold_left c.c_implements ~f:this#on_hint ~init:acc in
        let acc =
          List.fold_left c.c_typeconsts ~f:this#on_class_typeconst_def ~init:acc
        in
        let acc =
          List.fold_left c.c_consts ~f:this#on_class_c_const ~init:acc
        in
        let acc = List.fold_left c.c_vars ~f:this#on_class_var ~init:acc in
        let acc = List.fold_left c.c_uses ~f:this#on_class_use ~init:acc in
        let acc = List.fold_left c.c_reqs ~f:this#on_class_req ~init:acc in
        let acc = List.fold_left c.c_methods ~f:this#on_method_ ~init:acc in
        acc

      method on_fun_def acc f = this#on_fun_ acc f.fd_fun

      method on_class_typeconst_def acc t =
        let acc = this#on_id acc t.c_tconst_name in
        match t.c_tconst_kind with
        | TCAbstract
            { c_atc_as_constraint; c_atc_super_constraint; c_atc_default } ->
          let acc =
            match c_atc_as_constraint with
            | Some cstr -> this#on_hint acc cstr
            | None -> acc
          in
          let acc =
            match c_atc_super_constraint with
            | Some cstr -> this#on_hint acc cstr
            | None -> acc
          in
          (match c_atc_default with
          | Some d -> this#on_hint acc d
          | None -> acc)
        | TCConcrete { c_tc_type } -> this#on_hint acc c_tc_type
        | TCPartiallyAbstract { c_patc_constraint; c_patc_type } ->
          let acc = this#on_hint acc c_patc_constraint in
          this#on_hint acc c_patc_type

      method on_class_c_const acc c_const =
        let acc =
          match c_const.cc_type with
          | Some h -> this#on_hint acc h
          | None -> acc
        in
        let acc = this#on_id acc c_const.cc_id in
        let acc =
          match c_const.cc_kind with
          | CCConcrete e -> this#on_expr acc e
          | CCAbstract (Some default) -> this#on_expr acc default
          | CCAbstract None -> acc
        in
        acc

      method on_readonly_expr acc e =
        let acc = this#on_expr acc e in
        acc

      method on_class_var acc c_var =
        let acc = this#on_id acc c_var.cv_id in
        let acc =
          match hint_of_type_hint c_var.cv_type with
          | Some h -> this#on_hint acc h
          | None -> acc
        in
        let acc =
          match c_var.cv_expr with
          | Some e -> this#on_expr acc e
          | None -> acc
        in
        acc

      method on_class_use acc h = this#on_hint acc h

      method on_class_req acc (h, _) = this#on_hint acc h

      method on_record_def acc rd =
        let acc = this#on_id acc rd.rd_name in
        acc

      method on_gconst acc g =
        let acc = this#on_id acc g.cst_name in
        let acc = this#on_expr acc g.cst_value in
        let acc =
          match g.cst_type with
          | Some h -> this#on_hint acc h
          | None -> acc
        in
        acc

      method on_enum_class_label acc opt_sid name =
        let acc =
          match opt_sid with
          | Some sid -> this#on_id acc sid
          | None -> acc
        in
        this#on_string acc name

      method on_function_ptr_id acc fpi =
        match fpi with
        | FP_id sid -> this#on_id acc sid
        | FP_class_const (cid, _) -> this#on_class_id acc cid

      method on_et_splice acc e = this#on_expr acc e

      method on_typedef acc t =
        let acc = this#on_id acc t.t_name in
        let acc = this#on_hint acc t.t_kind in
        let acc =
          match t.t_constraint with
          | Some c -> this#on_hint acc c
          | None -> acc
        in
        acc

      method on_def acc =
        function
        | Fun f -> this#on_fun_def acc f
        | Class c -> this#on_class_ acc c
        | RecordDef rd -> this#on_record_def acc rd
        | Stmt s -> this#on_stmt acc s
        | Typedef t -> this#on_typedef acc t
        | Constant g -> this#on_gconst acc g
        | Namespace (_, p) -> this#on_program acc p
        | NamespaceUse _
        | SetNamespaceEnv _
        | FileAttributes _ ->
          acc

      method on_program acc p =
        let acc = List.fold_left p ~init:acc ~f:this#on_def in
        acc
    end
end
