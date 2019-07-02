(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

type entry = {
  file_input: ServerCommandTypes.file_input;
  path: Relative_path.t;
  ast: Ast.program;
  tast: Tast.program;
}

type t = entry Relative_path.Map.t

let empty = Relative_path.Map.empty

let extract_symbols
    (ast: Ast.program):
    FileInfo.id list * FileInfo.id list * FileInfo.id list * FileInfo.id list =
  let funs, classes, typedefs, consts =
    List.fold_left ast ~f:begin fun (funs, classes, typedefs, consts) def ->
      match def with
      | Ast.Fun { Ast.f_name; _ } ->
        (FileInfo.pos_full f_name)::funs, classes, typedefs, consts
      | Ast.Class { Ast.c_name; _ } ->
        funs, (FileInfo.pos_full c_name)::classes, typedefs, consts
      | Ast.Typedef { Ast.t_id; _ } ->
        funs, classes, (FileInfo.pos_full t_id)::typedefs, consts
      | Ast.Constant { Ast.cst_name; _ } ->
        funs, classes, typedefs, (FileInfo.pos_full cst_name)::consts
      | _ -> funs, classes, typedefs, consts
    end ~init:([], [], [], []) in
  (funs, classes, typedefs, consts)

let with_context
    ~(ctx: t)
    ~(f: unit -> 'a)
    : 'a =
  let make_then_revert_local_changes f () =
    Utils.with_context
      ~enter:ServerIdeUtils.make_local_changes
      ~exit:ServerIdeUtils.revert_local_changes
      ~do_:f
  in
  let (_errors, result) =
    Errors.do_ @@ make_then_revert_local_changes begin fun () ->
      Relative_path.Map.iter ctx ~f:begin fun _path { ast; _ } ->
        let (funs, classes, typedefs, consts) = extract_symbols ast in

        (* Update the positions of the symbols present in the AST by redeclaring
        them. Note that this doesn't handle *removing* any entries from the
        naming table if they've disappeared since the last time we updated the
        naming table. *)
        let get_names ids = List.map ~f:snd ids |> SSet.of_list in
        NamingGlobal.remove_decls
          ~funs:(get_names funs)
          ~classes:(get_names classes)
          ~typedefs:(get_names typedefs)
          ~consts:(get_names consts);
        NamingGlobal.make_env ~funs ~classes ~typedefs ~consts;
      end;

      f ()
    end
  in
  result

let update
    ~(tcopt: TypecheckerOptions.t)
    ~(ctx: t)
    ~(path: Relative_path.t)
    ~(file_input: ServerCommandTypes.file_input)
    : (t * entry) =
  let { Full_fidelity_ast.ast; _ } = Ast_provider.parse_file_input
    ~full:true
    path
    file_input
  in
  Ast_provider.provide_ast_hint path ast Ast_provider.Full;
  let tast = with_context ~ctx ~f:(fun () ->
    let nast = Naming.program (Ast_to_nast.convert ast) in
    Typing.nast_to_tast tcopt nast
  ) in
  let entry = {
    path;
    file_input;
    ast;
    tast;
  } in
  let ctx = Relative_path.Map.add ctx path entry in
  (ctx, entry)

let get_file_input
    ~(ctx: t)
    ~(path: Relative_path.t)
    : ServerCommandTypes.file_input =
  match Relative_path.Map.get ctx path with
  | Some { file_input; _ } ->
    file_input
  | None ->
    ServerCommandTypes.FileName (Relative_path.to_absolute path)

let get_path ~(entry: entry): Relative_path.t =
  entry.path

let get_ast ~(entry: entry): Ast.program =
  entry.ast

let get_tast ~(entry: entry): Tast.program =
  entry.tast

let get_fileinfo ~(entry:entry): FileInfo.t =
  let ast = get_ast entry in
  let (funs, classes, typedefs, consts) = extract_symbols ast in
  { FileInfo.empty_t with
    FileInfo.funs;
    classes;
    typedefs;
    consts;
  }
