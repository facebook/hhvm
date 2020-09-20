(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

type decls = {
  classes: Shallow_decl_defs.shallow_class SMap.t;
      [@printer (fun fmt -> SMap.pp Shallow_decl_defs.pp_shallow_class fmt)]
  funs: Typing_defs.fun_elt SMap.t;
      [@printer (fun fmt -> SMap.pp Pp_type.pp_fun_elt fmt)]
  typedefs: Typing_defs.typedef_type SMap.t;
      [@printer (fun fmt -> SMap.pp Pp_type.pp_typedef_type fmt)]
  consts: Typing_defs.decl_ty SMap.t;
      [@printer (fun fmt -> SMap.pp Pp_type.pp_decl_ty fmt)]
}
[@@deriving show]

external parse_decls_ffi : Relative_path.t -> string -> decls
  = "parse_decls_ffi"

let parse_decls ?contents relative_path =
  let contents =
    match contents with
    | Some c -> Some c
    | None -> File_provider.get_contents relative_path
  in
  match contents with
  | Some contents -> parse_decls_ffi relative_path contents
  | None ->
    failwith
      (Printf.sprintf
         "Could not load file contents for %s"
         (Relative_path.to_absolute relative_path))

let decls_to_fileinfo (decls : decls) : FileInfo.t =
  (* TODO: Nast.generate_ast_decl_hash ignores pos, match it! *)
  let hash = Some (Marshal.to_string decls [] |> OpaqueDigest.string) in
  let get_ids : 'a. ('a -> Pos.t) -> 'a SMap.t -> FileInfo.id list =
   fun get_pos items ->
    SMap.fold (fun k v acc -> (FileInfo.Full (get_pos v), k) :: acc) items []
  in
  let { classes; funs; typedefs; _ } = decls in
  {
    FileInfo.hash;
    classes = get_ids (fun c -> fst c.Shallow_decl_defs.sc_name) classes;
    funs = get_ids (fun f -> f.Typing_defs.fe_pos) funs;
    typedefs = get_ids (fun t -> t.Typing_defs.td_pos) typedefs;
    (* TODO: check how to get pos from decl_ty *)
    consts = [];
    (* TODO: get file mode*)
    file_mode = None;
    (* TODO: parse_decls_ffi needs to return record *)
    record_defs = [];
    comments = None;
  }

let parse
    (_popt : ParserOptions.t)
    (acc : FileInfo.t Relative_path.Map.t)
    (fn : Relative_path.t) : FileInfo.t Relative_path.Map.t =
  if not (FindUtils.path_filter fn) then
    acc
  else
    parse_decls fn |> decls_to_fileinfo |> fun file_info ->
    Relative_path.Map.add acc ~key:fn ~data:file_info

let parse_batch
    (popt : ParserOptions.t)
    (acc : FileInfo.t Relative_path.Map.t)
    (fnl : Relative_path.t list) : FileInfo.t Relative_path.Map.t =
  List.fold_left (parse popt) acc fnl

(* TODO: Enable tracing *)
let parse_decls_parallel
    (workers : MultiWorker.worker list)
    (get_next : Relative_path.t list MultiWorker.Hh_bucket.next)
    (popt : ParserOptions.t) :
    FileInfo.t Relative_path.Map.t * Errors.t * Relative_path.Set.t =
  let acc =
    MultiWorker.call
      (Some workers)
      ~job:(parse_batch popt)
      ~neutral:Relative_path.Map.empty
      ~merge:Relative_path.Map.union
      ~next:get_next
  in
  (* TODO: Enable parser errors *)
  (acc, Errors.empty, Relative_path.Set.empty)
