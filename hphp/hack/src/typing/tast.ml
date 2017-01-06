(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type sid = Nast.sid
type pstring = Nast.pstring
type ty = Typing_defs.locl Typing_defs.ty
type shape_field_name = Nast.shape_field_name

(* Typed statement.
 * For now, this is a straight copy of Nast.stmt but it will evolve
 * to a point where there is sufficient type information in the
 * AST to reconstruct types (of locals, for example) without solving
 * constraints.
 *
 * Also, we need to determine where position information is retained.
 *)
type stmt =
  | Expr of expr
  | Break
  | Continue
  | Throw of Nast.is_terminal * expr
  | Return of expr option
  | Static_var of expr list
  | If of expr * block * block
  | Do of block * expr
  | While of expr * block
  | For of expr * expr * expr * block
  | Switch of expr * case list
  | Foreach of expr * as_expr * block
  | Try of block * catch list * block
  | Noop
  | Fallthrough

and as_expr =
  | As_v of expr
  | As_kv of expr * expr
  | Await_as_v of Pos.t * expr
  | Await_as_kv of Pos.t * expr * expr

and block = stmt list

and class_id =
  | CIparent
  | CIself
  | CIstatic
  | CIexpr of expr
  | CI of sid

(* Typed expression.
 * For now, this is pretty much a straight copy of Nast.expr but it will
 * evolve to a point where there is sufficient type information in the
 * expression to reconstruct its type without solving constraints.
 * Also, we need to determine where position information is retained.
 *)
and expr =
  | Any
  | Shape of expr Nast.ShapeMap.t
  | ValCollection of Nast.vc_kind * expr list
  | KeyValCollection of Nast.kvc_kind * field list
  | This
  | Id of sid
  | Lvar of Local_id.t
  | Lplaceholder of Pos.t
  | Fun_id of sid
  | Method_id of expr * pstring
  (* meth_caller('Class name', 'method name') *)
  | Method_caller of sid * pstring
  | Smethod_id of sid * pstring
  (* Dynamic instance property access e.g. $x->$f; not legal in strict files *)
  | Obj_dynamic_get of expr * expr * Nast.og_null_flavor
  (* Statically-known instance property access e.g. $x->f *)
  | Obj_get of ty * expr * sid * Nast.og_null_flavor
  | Array_get of expr * expr option
  | Class_get of class_id * pstring
  | Class_const of class_id * pstring
  | Call of Nast.call_type
    * expr (* function *)
    * expr list (* positional args *)
    * expr list (* unpacked args *)
  | Bool_literal of bool
  | Int_literal of pstring
  | Float_literal of pstring
  (* This has type ?ty *)
  | Null_literal of ty
  | String_literal of pstring
  | String2 of expr list
  | Special_func of special_func
  | Yield_break
  | Yield of afield
  | Await of ty * expr
  (* TODO TAST: we should be able to consolidate these *)
  (* e1, ..., en : tn
   *   ==> New_tuple [e1; ...; en] : (t1, ..., tn)
   *)
  | New_tuple of expr list
  (* e1 : t1, ..., en : tn
   *   ==> New_tuple_array [e1; ...; en] : tuple_array<t1, ..., tn>
   *)
  | New_tuple_array of expr list
  (* e1 : t, ..., en : t
   *   ==> New_vec_array(t, [e1; ...; en]): array<t> *)
  | New_vec_array of ty * expr list
  (* e1 : t1, ..., en : tn ==>
   *   New_shape_array [(f1,t1); ...; (fn,tn)] : shape_array<f1=>t1,...,fn=>tn>
   *)
  | New_shape_array of (shape_field_name * expr) list
  (* k1 : tk, ..., kn : tk and
   * e1 : tv, ..., en : tv
   *   New_map_array(tk, tv, [(k1,e1);...;(kn,en)]) : array<tk,tv>
   *)
  | New_map_array of ty * ty * (expr * expr) list
  | Pair of expr * expr
  | Cast of ty * expr
  (* TODO TAST: use an "instrinsic" to precisely describe the
   * overloaded operation that we have resolved for unop and binop
   *)
  | Unop of Ast.uop * expr * ty
  | Binop of Ast.bop * expr * expr * ty
  (** The ID of the $$ that is implicitly declared by this pipe. *)
  | Pipe of Local_id.t * expr * expr
  (* Explicit type must be a supertype of the types of the branches. *)
  | Eif of expr * expr * expr * ty
  | InstanceOf of expr * class_id
  | New of class_id * expr list * expr list
  (*  | Efun of fun_ * id list*)
  | Xml of sid * (pstring * expr) list * expr list
  (*  | Assert of assert_expr*)
  | Clone of expr
  | Typename of sid

and case =
  | Default of block
  | Case of expr * block

and catch = sid * Local_id.t * block

and field = expr * expr
and afield =
  | AFvalue of expr
  | AFkvalue of expr * expr

and special_func =
  | Gena of expr
  | Genva of expr list
  | Gen_array_rec of expr
