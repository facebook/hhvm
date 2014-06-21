(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


(*****************************************************************************)
(* Pretty printing of types *)
(*****************************************************************************)

open Utils
open Typing_defs

(*****************************************************************************)
(* Computes the string representing a type in an error message.
 * We generally don't want to show the whole type. If an error was due
 * because something is a Vector instead of an int, we don't want to show
 * the type Vector<Vector<array<int>>> because it could be misleading.
 * The error is due to the fact that it is a Vector, regardless of the
 * type parameters.
 *)
(*****************************************************************************)

module ErrorString = struct

  let tprim = function
    | Nast.Tvoid       -> "void"
    | Nast.Tint        -> "an int"
    | Nast.Tbool       -> "a bool"
    | Nast.Tfloat      -> "a float"
    | Nast.Tstring     -> "a string"
    | Nast.Tnum        -> "a num (int/float)"
    | Nast.Tresource   -> "a resource"

  let rec type_ = function
    | Tany               -> "an untyped value"
    | Tunresolved l      -> unresolved l
    | Tarray (x, y, z)   -> array (x, y, z)
    | Ttuple _           -> "a tuple"
    | Tmixed             -> "a mixed value"
    | Toption _          -> "a nullable type"
    | Tprim tp           -> tprim tp
    | Tvar _             -> "some value"
    | Tanon _ | Tfun _   -> "a function"
    | Tgeneric (x, y)    -> generic (x, y)
    | Tabstract ((_, x), _, _)
    | Tapply ((_, x), _) -> "an object of type "^(strip_ns x)
    | Tobject            -> "an object"
    | Tshape _           -> "a shape"

  and array = function
    | _, None, None     -> "an array"
    | _, Some _, None   -> "an array (used like a vector)"
    | _, Some _, Some _ -> "an array (used like a hashtable)"
    | _                 -> assert false

  and generic = function
    | "this", Some x ->
        type_ (snd x)^" (compatible with the type 'this')"
    | s, _ -> "a value of generic type "^s

  and unresolved l =
    let l = List.map snd l in
    let l = List.map type_ l in
    let s = List.fold_right SSet.add l SSet.empty in
    let l = SSet.elements s in
    unresolved_ l

  and unresolved_ = function
    | []      -> "an undefined value"
    | [x]     -> x
    | x :: rl -> x^" or "^unresolved_ rl

end

(*****************************************************************************)
(* Module used to "suggest" types.
 * When a type is missing, it is nice to suggest a type to the user.
 * However, there are some cases where parts of the type is still unresolved.
 * When that is the case, we print '...' and let the user replace the missing
 * parts with a real type. So if we inferred that something was a Vector,
 * but we didn't manage to infer the type of the elements, the output becomes:
 * Vector<...>.
 *)
(*****************************************************************************)

module Suggest = struct

  let rec type_ (_, ty) =
    match ty with
    | Tarray _               -> "array"
    | Tunresolved tyl        -> "..."
    | Ttuple (l)             -> "("^list l^")"
    | Tany                   -> "..."
    | Tmixed                 -> "mixed"
    | Tgeneric (s, _)        -> s
    | Toption ty             -> "?" ^ type_ ty
    | Tprim tp               -> prim tp
    | Tvar _                 -> "..."
    | Tanon _ | Tfun _       -> "..."
    | Tapply ((_, cid), [])  -> Utils.strip_ns cid
    | Tapply ((_, cid), [x]) -> (Utils.strip_ns cid)^"<"^type_ x^">"
    | Tapply ((_, cid), l)   -> (Utils.strip_ns cid)^"<"^list l^">"
    | Tabstract ((_, cid), [], _)  -> Utils.strip_ns cid
    | Tabstract ((_, cid), [x], _) -> (Utils.strip_ns cid)^"<"^type_ x^">"
    | Tabstract ((_, cid), l, _)   -> (Utils.strip_ns cid)^"<"^list l^">"
    | Tobject                -> "..."
    | Tshape _               -> "..."

  and list = function
    | []      -> ""
    | [x]     -> type_ x
    | x :: rl -> type_ x ^ ", "^ list rl

  and prim = function
    | Nast.Tvoid   -> "void"
    | Nast.Tint    -> "int"
    | Nast.Tbool   -> "bool"
    | Nast.Tfloat  -> "float"
    | Nast.Tstring -> "string"
    | Nast.Tnum    -> "num (int/float)"
    | Nast.Tresource -> "resource"

end

(*****************************************************************************)
(* Pretty-printer of the "full" type. *)
(*****************************************************************************)

module Full = struct
  module Env = Typing_env

  let rec list_sep o s f l =
    match l with
    | [] -> ()
    | [x] -> f x
    | x :: rl -> f x; o s; list_sep o s f rl

  let rec ty st env o (_, x) = ty_ st env o x

  and ty_ st env o x =
    let k = ty st env o in
    let list = list_sep o ", " in
    match x with
    | Tany -> o "_"
    | Tmixed -> o "mixed"
    | Tarray (_, None, None) -> o "array"
    | Tarray (_, Some x, None) -> o "array<"; k x; o ">"
    | Tarray (_, Some x, Some y) -> o "array<"; k x; o ", "; k y; o ">"
    | Tarray (_, None, Some _) -> assert false
    | Tabstract ((_, s), [], _)
    | Tapply ((_, s), [])
    | Tgeneric (s, _) -> o s
    | Toption x -> o "?"; k x
    | Tprim x -> prim o x
    | Tvar n when ISet.mem n st -> o "[rec]"
    | Tvar n ->
        let _, ety = Env.expand_type env (Reason.Rnone, x) in
        let st = ISet.add n st in
        ty st env o ety
    | Tfun ft ->
      if ft.ft_abstract then o "abs " else ();
      o "(function"; fun_type st env o ft; o ")"
    | Tabstract ((_, s), tyl, _) -> o s; o "<"; list k tyl; o ">"
    (* Don't strip_ns here! We want the FULL type, including the initial slash.
    *)
    | Tapply ((_, s), tyl) -> o s; o "<"; list k tyl; o ">"
    | Ttuple tyl -> o "("; list k tyl; o ")"
    | Tanon _ -> o "[fun]"
    | Tunresolved tyl -> list_sep o "& " k tyl
    | Tobject -> o "object"
    | Tshape fdm -> o "[shape]"

  and prim o x =
    o (match x with
    | Nast.Tvoid   -> "void"
    | Nast.Tint    -> "int"
    | Nast.Tbool   -> "bool"
    | Nast.Tfloat  -> "float"
    | Nast.Tstring -> "string"
    | Nast.Tnum    -> "num"
    | Nast.Tresource -> "resource"
    )

  and fun_type st env o ft =
    (match ft.ft_tparams with
    | [] -> ()
    | l -> o "<"; list_sep o ", " (tparam o) l; o ">");
    o "("; list_sep o ", " (fun_param st env o) ft.ft_params; o "): ";
    ty st env o ft.ft_ret

  and fun_param st env o (param_name, param_type) =
    match param_name, param_type with
    | None, _ -> ty st env o param_type
    | Some param_name, (_, Tany) -> o param_name
    | Some param_name, param_type ->
        ty st env o param_type; o " "; o param_name

  and tparam o ((_, x), _) = o x

  let to_string env x =
    let buf = Buffer.create 50 in
    ty ISet.empty env (Buffer.add_string buf) x;
    Buffer.contents buf

  let to_string_strip_ns env x =
    let buf = Buffer.create 50 in
    let add_string str =
      let str = Utils.strip_ns str in
      Buffer.add_string buf str
    in
    ty ISet.empty env add_string x;
    Buffer.contents buf

end

(*****************************************************************************)
(* Prints the internal type of a class, this code is meant to be used for
 * debugging purposes only.
 *)
(*****************************************************************************)

module PrintClass = struct

  let tenv = Typing_env.empty ""

  let indent = "    "
  let bool = string_of_bool
  let sset s =
    let contents = SSet.fold (fun x acc -> x^" "^acc) s "" in
    Printf.sprintf "Set( %s)" contents

  let pos p =
    let line, start, end_ = Pos.info_pos p in
    Printf.sprintf "(line %d: chars %d-%d)" line start end_

  let class_kind = function
    | Ast.Cabstract -> "Cabstract"
    | Ast.Cnormal -> "Cnormal"
    | Ast.Cinterface -> "Cinterface"
    | Ast.Ctrait -> "Ctrait"

  let ty_opt = function
    | None -> ""
    | Some ty -> Full.to_string tenv ty

  let tparam ((position, name), cstr) =
    pos position^" "^name^" "^ty_opt cstr

  let tparam_list l =
    List.fold_right (fun x acc -> tparam x^", "^acc) l ""

  let class_elt ce =
    let vis =
      match ce.ce_visibility with
      | Vpublic -> "public"
      | Vprivate _ -> "private"
      | Vprotected _ -> "protected"
    in
    let type_ = Full.to_string tenv ce.ce_type in
    vis^" "^type_

  let class_elt_smap m =
    SMap.fold begin fun field v acc ->
      "("^field^": "^class_elt v^") "^acc
    end m ""

  let class_elt_smap_with_breaks m =
    SMap.fold begin fun field v acc ->
      "\n"^indent^field^": "^(class_elt v)^acc
    end m ""

  let ancestors_smap m =
    (* Format is as follows:
     *    ParentKnownToHack
     *  ! ParentCompletelyUnknown
     *  ~ ParentPartiallyKnown  (interface|abstract|trait)
     *
     * ParentPartiallyKnown must inherit one of the ! Unknown parents, so that
     * sigil could be omitted *)
    SMap.fold begin fun field v acc ->
      let sigil, kind = match Typing_env.Classes.get field with
        | None -> "!", ""
        | Some {tc_members_fully_known; tc_kind; _} ->
          (if tc_members_fully_known then " " else "~"),
          " ("^class_kind tc_kind^")"
      in
      let ty_str = Full.to_string tenv v in
      "\n"^indent^sigil^" "^ty_str^kind^acc
    end m ""

  let user_attribute_smap m =
    SMap.fold begin fun field _ acc ->
      "("^field^": expr) "^acc
    end m ""

  let class_elt_option = function
    | None -> ""
    | Some ce -> class_elt ce

  let class_type c =
    let tc_need_init = bool c.tc_need_init in
    let tc_members_fully_known = bool c.tc_members_fully_known in
    let tc_abstract = bool c.tc_abstract in
    let tc_members_init = sset c.tc_members_init in
    let tc_kind = class_kind c.tc_kind in
    let tc_name = c.tc_name in
    let tc_tparams = tparam_list c.tc_tparams in
    let tc_consts = class_elt_smap c.tc_consts in
    let tc_cvars = class_elt_smap c.tc_cvars in
    let tc_scvars = class_elt_smap c.tc_scvars in
    let tc_methods = class_elt_smap_with_breaks c.tc_methods in
    let tc_smethods = class_elt_smap_with_breaks c.tc_smethods in
    let tc_construct = class_elt_option c.tc_construct in
    let tc_ancestors = ancestors_smap c.tc_ancestors in
    let tc_ancestors_checked_when_concrete =
      ancestors_smap c.tc_ancestors_checked_when_concrete in
    let tc_req_ancestors = sset c.tc_req_ancestors in
    let tc_req_ancestors_extends = sset c.tc_req_ancestors_extends in
    let tc_extends = sset c.tc_extends in
    let tc_user_attributes = user_attribute_smap c.tc_user_attributes in
    "tc_need_init: "^tc_need_init^"\n"^
    "tc_members_fully_known: "^tc_members_fully_known^"\n"^
    "tc_abstract: "^tc_abstract^"\n"^
    "tc_members_init: "^tc_members_init^"\n"^
    "tc_kind: "^tc_kind^"\n"^
    "tc_name: "^tc_name^"\n"^
    "tc_tparams: "^tc_tparams^"\n"^
    "tc_consts: "^tc_consts^"\n"^
    "tc_cvars: "^tc_cvars^"\n"^
    "tc_scvars: "^tc_scvars^"\n"^
    "tc_methods: "^tc_methods^"\n"^
    "tc_smethods: "^tc_smethods^"\n"^
    "tc_construct: "^tc_construct^"\n"^
    "tc_ancestors: "^tc_ancestors^"\n"^
    "tc_ancestors_checked_when_concrete: "^tc_ancestors_checked_when_concrete^"\n"^
    "tc_extends: "^tc_extends^"\n"^
    "tc_req_ancestors: "^tc_req_ancestors^"\n"^
    "tc_req_ancestors_extends: "^tc_req_ancestors_extends^"\n"^
    "tc_user_attributes: "^tc_user_attributes^"\n"^
    ""
end

module PrintFun = struct

  let bool = string_of_bool
  let int = string_of_int
  let tenv = PrintClass.tenv

  let fparam (sopt, ty) =
    let s = match sopt with
      | None -> "[None]"
      | Some s -> s in
    s ^ " " ^ (Full.to_string tenv ty) ^ ", "

  let fparams l =
    List.fold_right (fun x acc -> (fparam x)^acc) l ""

  let fun_type f =
    let ft_pos = PrintClass.pos f.ft_pos in
    let ft_unsafe = bool f.ft_unsafe in
    let ft_abstract = bool f.ft_abstract in
    let ft_arity_min = int f.ft_arity_min in
    let ft_arity_max = int f.ft_arity_max in
    let ft_tparams = PrintClass.tparam_list f.ft_tparams in
    let ft_params = fparams f.ft_params in
    let ft_ret = Full.to_string tenv f.ft_ret in
    "ft_pos: "^ft_pos^"\n"^
    "ft_unsafe: "^ft_unsafe^"\n"^
    "ft_abstract: "^ft_abstract^"\n"^
    "ft_arity_min: "^ft_arity_min^"\n"^
    "ft_arity_max: "^ft_arity_max^"\n"^
    "ft_tparams: "^ft_tparams^"\n"^
    "ft_params: "^ft_params^"\n"^
    "ft_ret: "^ft_ret^"\n"^
    ""
end

(*****************************************************************************)
(* User API *)
(*****************************************************************************)

let error ty = ErrorString.type_ ty
let suggest ty = Suggest.type_ ty
let full env ty = Full.to_string env ty
let full_strip_ns env ty = Full.to_string_strip_ns env ty
let class_ c = PrintClass.class_type c
let fun_ f = PrintFun.fun_type f
