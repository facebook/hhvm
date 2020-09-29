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
  consts: Typing_defs.const_decl SMap.t;
      [@printer (fun fmt -> SMap.pp Pp_type.pp_const_decl fmt)]
}
[@@deriving show]

type decl_lists = {
  dl_classes: (string * Shallow_decl_defs.shallow_class) list;
  dl_funs: (string * Typing_defs.fun_elt) list;
  dl_typedefs: (string * Typing_defs.typedef_type) list;
  dl_consts: (string * Typing_defs.const_decl) list;
}

external parse_decls_ffi : Relative_path.t -> string -> decls
  = "parse_decls_ffi"

external parse_decl_lists_ffi : Relative_path.t -> string -> decl_lists
  = "parse_decl_lists_ffi"

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

let parse_decl_lists ?contents relative_path =
  let contents =
    match contents with
    | Some c -> Some c
    | None -> File_provider.get_contents relative_path
  in
  match contents with
  | Some contents -> parse_decl_lists_ffi relative_path contents
  | None ->
    failwith
      (Printf.sprintf
         "Could not load file contents for %s"
         (Relative_path.to_absolute relative_path))

let decls_to_fileinfo
    (popt : ParserOptions.t) (fn : Relative_path.t) (decls : decl_lists) :
    FileInfo.t =
  let is_php_stdlib =
    Relative_path.(is_hhi (Relative_path.prefix fn))
    && ParserOptions.deregister_php_stdlib popt
  in
  let fun_filter funs =
    if is_php_stdlib then
      List.filter (fun (_, f) -> not f.Typing_defs.fe_php_std_lib) funs
    else
      funs
  in
  let class_filter classes =
    if is_php_stdlib then
      List.filter
        (fun (_, c) ->
          List.exists
            (fun a ->
              String.equal
                Naming_special_names.UserAttributes.uaPHPStdLib
                (snd a.Typing_defs_core.ua_name))
            c.Shallow_decl_defs.sc_user_attributes
          |> not)
        classes
    else
      classes
  in
  (* TODO: Nast.generate_ast_decl_hash ignores pos, match it! *)
  let hash = Some (Marshal.to_string decls [] |> OpaqueDigest.string) in
  let get_ids : 'a. ('a -> Pos.t) -> (string * 'a) list -> FileInfo.id list =
   (fun get_pos -> List.map (fun (k, v) -> (FileInfo.Full (get_pos v), k)))
  in
  let { dl_classes; dl_funs; dl_typedefs; dl_consts; _ } = decls in
  {
    FileInfo.hash;
    classes =
      class_filter dl_classes
      |> get_ids (fun c -> fst c.Shallow_decl_defs.sc_name);
    funs = fun_filter dl_funs |> get_ids (fun f -> f.Typing_defs.fe_pos);
    typedefs = get_ids (fun t -> t.Typing_defs.td_pos) dl_typedefs;
    consts = get_ids (fun c -> c.Typing_defs.cd_pos) dl_consts;
    (* TODO: get file mode*)
    file_mode = None;
    (* TODO: parse_decls_ffi needs to return record *)
    record_defs = [];
    comments = None;
  }

let parse
    (popt : ParserOptions.t)
    (acc : FileInfo.t Relative_path.Map.t)
    (fn : Relative_path.t) : FileInfo.t Relative_path.Map.t =
  if not (FindUtils.path_filter fn) then
    acc
  else
    parse_decl_lists fn |> decls_to_fileinfo popt fn |> fun file_info ->
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
