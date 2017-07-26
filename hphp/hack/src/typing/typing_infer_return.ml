(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core

type t =
  Typing_env.env *
  Pos.t *
  Typing_suggest.hint_kind *
  Typing_defs.locl Typing_defs.ty

let print_type_locl tenv ty = Typing_print.full tenv ty

let print_type (tenv, _, _, ty) = print_type_locl tenv ty

let rec print_list ~f types =
  match types with
  | [] -> ""
  | [ty] -> f ty
  | ty :: tys -> f ty ^", " ^ print_list f tys

let format_types (_ ,id) tys =
  let str_tys = print_list ~f:print_type tys in
  let least_ty =
    Option.value_map ~default:"" ~f:print_type
      (Typing_ops.LeastUpperBound.compute tys)
  in
  Printf.printf "%s : %s try %s\n" id str_tys least_ty

let clear_type_list ~suggest_mode =
  SharedMem.invalidate_caches();
  Typing_defs.is_suggest_mode := suggest_mode;
  Typing_suggest.types := [];
  Typing_suggest.funs_and_methods := [];
  Typing_suggest.initialized_members := SMap.empty

let typing_env_from_file tcopt file =
  let tcopt = TypecheckerOptions.make_permissive tcopt in
  Typing_env.empty tcopt ~droot:None file

let type_from_hint tcopt file hint =
  let tenv = typing_env_from_file tcopt file in
  let decl_ty = Typing_instantiability.instantiable_hint tenv hint in
  Typing_phase.localize_with_self tenv decl_ty

let just_return types =
  List.filter types ~f:(fun (_, _, kind, _) -> kind == Typing_suggest.Kreturn)

let compare_pos (p1, _) (_, p2, _, _) =
  match Pos.compare p1 p2 with
  | 0 -> true
  | _ -> false

let collect_types_and_funs tcopt def =
  clear_type_list ~suggest_mode:true;
  begin match def with
  | Nast.Fun f -> Pervasives.ignore (Typing.fun_def tcopt f)
  | Nast.Class c -> Pervasives.ignore (Typing.class_def tcopt c)
  | _ -> ()
  end;
  let types = !Typing_suggest.types in
  let funs_and_methods = !Typing_suggest.funs_and_methods in
  clear_type_list ~suggest_mode:false;
  types, funs_and_methods

let types_table types funs =
  let types = just_return types in
  let tbl = Hashtbl.create (List.length funs) in
  List.iter types
    begin fun (env, pos, k, ty) ->
      Hashtbl.add tbl pos (env, pos, k, ty);
    end;
  tbl

let process_types_and_funs ~process tcopt def =
  let types, funs = collect_types_and_funs tcopt def in
  match funs with
  | [] -> ()
  | [f] -> process f types
  | _ ->
    let tbl = types_table types funs in
    List.iter funs
      ~f:(fun (pos, id) -> process (pos, id) (Hashtbl.find_all tbl pos))

let get_inferred_types tcopt fnl ~process =
  List.iter fnl
    ~f:begin fun fn ->
      match Parser_heap.ParserHeap.get fn with
      | None -> ()
      | Some (ast, _) ->
        List.iter (Naming.program tcopt ast)
          (process_types_and_funs ~process tcopt)
    end

let get_matching_types tcopt def name =
  let types, _ = collect_types_and_funs tcopt def in
  List.filter (just_return types) ~f:(compare_pos name)
