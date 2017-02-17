(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Core

module A = Ast
module N = Nast
module H = Hhbc_ast

(* The various from_X functions below take some kind of AST (expression,
 * statement, etc.) and produce what is logically a sequence of instructions.
 * This could simply be represented by a list, but then we would need to
 * use an accumulator to avoid the quadratic complexity associated with
 * repeated appending to a list. Instead, we simply build a tree of
 * instructions which can easily be flattened at the end.
 *)
type instr_seq =
| Instr_list of H.instruct list
| Instr_concat of instr_seq list

(* Some helper constructors *)
let instr x = Instr_list [x]
let instrs x = Instr_list x
let gather x = Instr_concat x
let empty = Instr_list []

let rec instr_seq_to_list_aux sl result =
  match sl with
  | [] -> List.rev result
  | s::sl ->
    match s with
    | Instr_list instrl ->
      instr_seq_to_list_aux sl (List.rev_append instrl result)
    | Instr_concat sl' -> instr_seq_to_list_aux (sl' @ sl) result

let instr_seq_to_list t = instr_seq_to_list_aux [t] []

(* Strict binary operations; assumes that operands are already on stack *)
let from_binop op =
  match op with
  | A.Plus -> instr (H.IOp H.AddO)
  | A.Minus -> instr (H.IOp H.SubO)
  | A.Star -> instr (H.IOp H.MulO)
  | A.Slash -> instr (H.IOp H.Div)
  | A.Eqeq -> instr (H.IOp H.Eq)
  | A.EQeqeq -> instr (H.IOp H.Same)
  | A.Starstar -> instr (H.IOp H.Pow)
  | A.Diff -> instr (H.IOp H.Neq)
  | A.Diff2 -> instr (H.IOp H.NSame)
  | A.AMpamp -> failwith "&& not strict"
  | A.BArbar -> failwith "|| not strict"
  | A.Lt -> instr (H.IOp H.Lt)
  | A.Lte -> instr (H.IOp H.Lte)
  | A.Gt -> instr (H.IOp H.Gt)
  | A.Gte -> instr (H.IOp H.Gte)
  | A.Dot -> instr (H.IOp H.Concat)
  | A.Amp -> instr (H.IOp H.BitAnd)
  | A.Bar -> instr (H.IOp H.BitOr)
  | A.Ltlt -> instr (H.IOp H.Shl)
  | A.Gtgt -> instr (H.IOp H.Shr)
  | A.Percent -> instr (H.IOp H.Mod)
  | A.Xor -> instr (H.IOp H.BitXor)
  | A.Eq _ -> failwith "= NYI"

let binop_to_eqop op =
  match op with
  | A.Plus -> H.PlusEqualO
  | A.Minus -> H.MinusEqualO
  | A.Star -> H.MulEqualO
  | A.Slash -> H.DivEqual
  | A.Starstar -> H.PowEqual
  | A.Amp -> H.AndEqual
  | A.Bar -> H.OrEqual
  | A.Xor -> H.XorEqual
  | A.Ltlt -> H.SlEqual
  | A.Gtgt -> H.SrEqual
  | A.Percent -> H.ModEqual
  | A.Dot -> H.ConcatEqual
  | _ -> failwith "Invalid =op"

let unop_to_incdec_op op =
  match op with
  | A.Uincr -> H.PreIncO
  | A.Udecr -> H.PreDecO
  | A.Upincr -> H.PostIncO
  | A.Updecr -> H.PostDecO
  | _ -> failwith "Invalid incdec op"

let rec from_expr expr =
  match snd expr with
  | A.String (_, litstr) ->
    instr H.(ILitConst (String litstr))
  | A.Int (_, litstr) ->
    (* TODO deal with integer out of range *)
    instr H.(ILitConst (Int (Int64.of_string litstr)))
  | A.Null -> instr H.(ILitConst Null)
  | A.False -> instr H.(ILitConst False)
  | A.True -> instr H.(ILitConst True)
  | A.Lvar (_, x) -> instr H.(IGet (CGetL (Local_named x)))
  | A.Unop (op, e) ->
    emit_unop op e
  | A.Binop (A.Eq obop, e1, e2) ->
    emit_assignment obop e1 e2
  | A.Binop (op, e1, e2) ->
    gather [from_expr e1; from_expr e2; from_binop op]
  | A.Call ((_, A.Id (_, id)), el, []) when id = "echo" ->
    gather [
      from_exprs el;
      instr H.(IOp Print);
    ]
  (* TODO: emit warning *)
  | _ -> empty

(* Emit code for an l-value, returning instructions and the location that
 * must be set. For now, this is just a local. *)
and emit_lval (_, expr_) =
  match expr_ with
  | A.Lvar id -> empty, H.Local_named (snd id)
  | _ -> failwith "emit_lval: NYI"

and emit_assignment obop e1 e2 =
  let instrs1, lval = emit_lval e1 in
  let instrs2 = from_expr e2 in
  gather [instrs1; instrs2;
    match obop with
    | None -> instr H.(IMutator (SetL lval))
    | Some bop -> instr H.(IMutator (SetOpL (lval, binop_to_eqop bop)))]

and emit_unop op e =
  match op with
  | A.Utild -> gather [from_expr e; instr (H.IOp H.BitNot)]
  | A.Unot -> gather [from_expr e; instr (H.IOp H.Not)]
  | A.Uplus -> gather
    [instr H.(ILitConst (Int (Int64.zero)));
    from_expr e;
    instr (H.IOp H.AddO)]
  | A.Uminus -> gather
    [instr H.(ILitConst (Int (Int64.zero)));
    from_expr e;
    instr (H.IOp H.SubO)]
  | A.Uincr | A.Udecr | A.Upincr | A.Updecr ->
    let instrs, lval = emit_lval e in
    gather [instrs; instr H.(IMutator (IncDecL (lval, unop_to_incdec_op op)))]
  | A.Uref -> failwith "references - NYI"

and from_exprs exprs =
  gather (List.map exprs from_expr)

and from_stmt verify_return st =
  let open H in
  match st with
  | A.Expr expr ->
    gather [
      from_expr expr;
      instr (IBasic PopC)
    ]
  | A.Return (_, None) ->
    instrs [
      ILitConst Null;
      IContFlow RetC;
    ]
  | A.Return (_,  Some expr) ->
    gather [
      from_expr expr;
      (if verify_return then instr (IMisc VerifyRetTypeC) else empty);
      instr (IContFlow RetC);
    ]
  | A.Noop ->
    empty
  (* TODO: emit warning *)
  | _ -> empty

and from_stmts verify_return stl =
  gather (List.map stl (from_stmt verify_return))

let fmt_prim x =
  match x with
  | N.Tvoid   -> "HH\\void"
  | N.Tint    -> "HH\\int"
  | N.Tbool   -> "HH\\bool"
  | N.Tfloat  -> "HH\\float"
  | N.Tstring -> "HH\\string"
  | N.Tnum    -> "HH\\num"
  | N.Tresource -> "HH\\resource"
  | N.Tarraykey -> "HH\\arraykey"
  | N.Tnoreturn -> "HH\\noreturn"

(* TODO *)
let fmt_name s = s

let extract_shape_fields smap =
  let get_pos =
    function Nast.SFlit (p, _) | Nast.SFclass_const ((p, _), _) -> p in
  List.sort (fun (k1, _) (k2, _) -> Pos.compare (get_pos k1) (get_pos k2))
    (Nast.ShapeMap.elements smap)

(* Produce the "userType" bit of the annotation *)
let rec fmt_hint (_, h) =
  match h with
  | N.Hany -> ""
  | N.Hmixed -> "HH\\mixed"
  | N.Hthis -> "HH\\this"
  | N.Hprim prim -> fmt_prim prim
  | N.Habstr s -> fmt_name s

  | N.Happly ((_, s), []) -> fmt_name s
  | N.Happly ((_, s), args) ->
    fmt_name s ^ "<" ^ String.concat ", " (List.map args fmt_hint) ^ ">"

  | N.Hfun (args, _, ret) ->
    "(function (" ^ String.concat ", " (List.map args fmt_hint) ^ "): " ^
      fmt_hint ret ^ ")"

  | N.Htuple hs ->
    "(" ^ String.concat ", " (List.map hs fmt_hint) ^ ")"

  | N.Haccess (h, accesses) ->
    fmt_hint h ^ "::" ^ String.concat "::" (List.map accesses snd)

  | N.Hoption t -> "?" ^ fmt_hint t

  | N.Harray (None, None) -> "array"
  | N.Harray (Some h, None) -> "array<" ^ fmt_hint h ^ ">"
  | N.Harray (Some h1, Some h2) ->
    "array<" ^ fmt_hint h1 ^ ", " ^ fmt_hint h2 ^ ">"
  | N.Harray _ -> failwith "bogus array"
  | N.Hshape smap ->
    let fmt_field = function
      | N.SFlit (_, s) -> "'" ^ s ^ "'"
      | N.SFclass_const ((_, s1), (_, s2)) -> fmt_name s1 ^ "::" ^ s2
    in
    let shape_fields =
      List.map ~f:(fun (k, h) -> fmt_field k ^ "=>" ^ fmt_hint h)
        (extract_shape_fields smap) in
    "HH\\shape(" ^ String.concat ", " shape_fields ^ ")"

open Hhbc_ast

let rec hint_to_type_constraint tparams (_, h) =
match h with
| N.Hany | N.Hmixed | N.Hfun (_, _, _) | N.Hthis
| N.Hprim N.Tvoid ->
  { tc_name = None; tc_flags = [] }

| N.Hprim prim ->
  { tc_name = Some (fmt_prim prim); tc_flags = [HHType] }

| N.Haccess (_, _) ->
  { tc_name = Some ""; tc_flags = [HHType; ExtendedHint; TypeConstant] }

(* Need to differentiate between type params and classes *)
| N.Habstr s | N.Happly ((_, s), _) ->
  if List.mem tparams s then
    { tc_name = Some ""; tc_flags = [HHType; ExtendedHint; TypeVar] }
  else
    { tc_name = Some s; tc_flags = [HHType] }

(* Shapes and tuples are just arrays *)
| N.Harray (_, _) | N.Hshape _ |  N.Htuple _ ->
  { tc_name = Some "array"; tc_flags = [HHType] }

| N.Hoption t ->
  let { tc_name; tc_flags } = hint_to_type_constraint tparams t in
  { tc_name = tc_name;
    tc_flags = List.dedup ([Nullable; HHType; ExtendedHint] @ tc_flags) }

let hint_to_type_info ~always_extended tparams h =
  let { tc_name; tc_flags } = hint_to_type_constraint tparams h in
  { ti_user_type = Some (fmt_hint h);
    ti_type_constraint =
      { tc_name = tc_name;
        tc_flags =
          if always_extended && tc_name != None
          then List.dedup (ExtendedHint :: tc_flags)
          else tc_flags;
      }
  }

let hints_to_type_infos ~always_extended tparams hints =
  let mapper hint = hint_to_type_info always_extended tparams hint in
  List.map hints mapper

let from_param tparams p = {
  H.param_name = p.N.param_name;
  H.param_type_info = Option.map p.N.param_hint
    (hint_to_type_info ~always_extended:false tparams);
}

let has_type_constraint ti =
  match ti with
  | Some { ti_type_constraint = { tc_name = Some _; _ }; _ } -> true
  | _ -> false

let emit_method_prolog params =
  gather H.(List.filter_map params (fun p ->
    if has_type_constraint p.param_type_info
    then Some (instr (IMisc (VerifyParamType p.param_name)))
    else None))

let from_body tparams params ret b =
  let tparams = List.map tparams (fun (_, (_, s), _) -> s) in
  let params = List.map params (from_param tparams) in
  let return_type_info = Option.map ret
    (hint_to_type_info ~always_extended:true tparams) in
  let verify_return = has_type_constraint return_type_info in
  let stmt_instrs = from_stmts verify_return b.N.fub_ast in
  let ret_instrs =
    match List.last b.N.fub_ast with Some (A.Return _) -> empty | _ ->
    instrs [H.ILitConst H.Null; H.IContFlow H.RetC] in
  let body_instrs = gather [
    emit_method_prolog params;
    stmt_instrs;
    ret_instrs;
  ] in
  body_instrs, params, return_type_info

let from_fun_ : Nast.fun_ -> Hhbc_ast.fun_def option =
  fun nast_fun ->
  let name_i = Litstr.to_string @@ snd nast_fun.Nast.f_name in
  match nast_fun.N.f_body with
  | N.NamedBody _ ->
    None

  | N.UnnamedBody b ->
    let body_instrs, params, return_type_info =
      from_body nast_fun.N.f_tparams nast_fun.N.f_params nast_fun.N.f_ret b in
    Some
    (
      { H.f_name          = name_i
      ; H.f_body          = instr_seq_to_list body_instrs
      ; H.f_params        = params
      ; H.f_return_type   = return_type_info
      }
    )

let from_functions nast_functions =
  Core.List.filter_map nast_functions from_fun_

let from_method : Nast.method_ -> Hhbc_ast.method_def option =
  fun nast_method ->
  let method_name = Litstr.to_string @@ snd nast_method.Nast.m_name in
  let method_is_abstract = nast_method.Nast.m_abstract in
  (* TODO: An abstract method must generate
.method [protected abstract] <"HH\\void" N  > protected_abstract_method() {
String "Cannot call abstract method AbstractClass::protected_abstract_method()"
Fatal RuntimeOmitFrame
} *)
  let method_is_final = nast_method.Nast.m_final in
  let method_is_private = nast_method.Nast.m_visibility = Nast.Private in
  let method_is_protected = nast_method.Nast.m_visibility = Nast.Protected in
  let method_is_public = nast_method.Nast.m_visibility = Nast.Public in
  let method_is_static = false in (* TODO static functions *)
  match nast_method.N.m_body with
  | N.NamedBody _ ->
    None
  | N.UnnamedBody b ->
    let body_instrs, _params, _return_type_info =
      from_body nast_method.N.m_tparams nast_method.N.m_params
        nast_method.N.m_ret b in
    let method_body = instr_seq_to_list body_instrs in
    let m = { H.method_name; method_body; method_is_abstract; method_is_final;
       method_is_private; method_is_protected; method_is_public;
       method_is_static } in
    Some m

let from_methods nast_methods =
  Core.List.filter_map nast_methods from_method

let default_constructor =
  let method_name = "86ctor" in
  let method_body = instr_seq_to_list (gather [
    instr (H.ILitConst H.Null);
    instr (H.IContFlow H.RetC)
  ]) in
  let method_is_abstract = false in
  let method_is_final = false in
  let method_is_private = false in
  let method_is_protected = false in
  let method_is_public = true in
  let method_is_static = false in
  { H.method_name; method_body; method_is_abstract; method_is_final;
     method_is_private; method_is_protected; method_is_public;
     method_is_static }

let add_constructor nast_class class_methods =
  match nast_class.N.c_constructor with
  | None -> default_constructor :: class_methods
  | Some nast_ctor ->
    begin
      match from_method nast_ctor with
      | None -> class_methods
      | Some m -> m :: class_methods
    end

let from_extends tparams extends =
  (* TODO: This prints out "extends <"\\Mammal" "\\Mammal" hh_type >"
  instead of "extends Mammal" -- figure out how to have it produce the
  simpler form in this clause.
  *)
  match extends with
  | [] -> None
  | h :: _ -> Some (hint_to_type_info ~always_extended:false tparams h)

let from_implements tparams implements =
  (* TODO: This prints out "implements <"\\IFoo" "\\IFoo" hh_type >"
  instead of "implements IFoo" -- figure out how to have it produce the
  simpler form in this clause.
  *)
  hints_to_type_infos ~always_extended:false tparams implements

let from_class : Nast.class_ -> Hhbc_ast.class_def =
  (* TODO user attributes *)
  fun nast_class ->
  let class_name = Litstr.to_string @@ snd nast_class.N.c_name in
  let class_is_trait = nast_class.N.c_kind = Ast.Ctrait in
  let class_is_enum = nast_class.N.c_kind = Ast.Cenum in
  let class_is_interface = nast_class.N.c_kind = Ast.Cinterface in
  let class_is_abstract = nast_class.N.c_kind = Ast.Cabstract in
  let class_is_final =
    nast_class.N.c_final || class_is_trait || class_is_enum in
  let tparams = [] in (* TODO: type parameters *)
  let class_base =
    if class_is_interface then None
    else from_extends tparams nast_class.N.c_extends in
  let implements =
    if class_is_interface then nast_class.N.c_extends
    else nast_class.N.c_implements in
  let class_implements = from_implements tparams implements in
  let nast_methods = nast_class.N.c_methods @ nast_class.N.c_static_methods in
  let class_methods = from_methods nast_methods in
  let class_methods = add_constructor nast_class class_methods in
  (* TODO: other class members *)
  { H.class_name;
    class_base;
    class_implements;
    class_is_final;
    class_is_trait;
    class_is_enum;
    class_is_interface;
    class_is_abstract;
    class_methods
  }

let from_classes nast_classes =
  Core.List.map nast_classes from_class
