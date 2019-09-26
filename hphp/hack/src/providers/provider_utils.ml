(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel

let with_context ~(ctx : Provider_context.t) ~(f : unit -> 'a) : 'a =
  let make_then_revert_local_changes f () =
    Utils.with_context
      ~enter:(fun () ->
        Provider_context.set_global_context_internal ctx;
        ServerIdeUtils.make_local_changes ())
      ~exit:(fun () ->
        ServerIdeUtils.revert_local_changes ();
        Provider_context.unset_global_context_internal ())
      ~do_:f
  in
  let (_errors, result) =
    Errors.do_
    @@ make_then_revert_local_changes (fun () ->
           Relative_path.Map.iter
             ctx.Provider_context.entries
             ~f:(fun _path { Provider_context.ast; _ } ->
               let (funs, classes, record_defs, typedefs, consts) =
                 Nast.get_defs ast
               in
               (* Update the positions of the symbols present in the AST by redeclaring
        them. Note that this doesn't handle *removing* any entries from the
        naming table if they've disappeared since the last time we updated the
        naming table. *)
               let get_names ids = List.map ~f:snd ids |> SSet.of_list in
               NamingGlobal.remove_decls
                 ~funs:(get_names funs)
                 ~classes:(get_names classes)
                 ~record_defs:(get_names record_defs)
                 ~typedefs:(get_names typedefs)
                 ~consts:(get_names consts);
               NamingGlobal.make_env
                 ~funs
                 ~classes
                 ~record_defs
                 ~typedefs
                 ~consts);

           f ())
  in
  result

let update_context
    ~(ctx : Provider_context.t)
    ~(path : Relative_path.t)
    ~(file_input : ServerCommandTypes.file_input) :
    Provider_context.t * Provider_context.entry =
  let ast =
    Errors.ignore_ (fun () ->
        Ast_provider.parse_file_input ~full:true path file_input)
  in
  Ast_provider.provide_ast_hint path ast Ast_provider.Full;
  let entry = { Provider_context.path; file_input; ast } in
  let ctx =
    {
      ctx with
      Provider_context.entries =
        Relative_path.Map.add ctx.Provider_context.entries path entry;
    }
  in
  (ctx, entry)

let compute_tast ~(ctx : Provider_context.t) ~(entry : Provider_context.entry)
    : Tast.program =
  Errors.ignore_ (fun () ->
      let make_tast () =
        let nast = Naming.program entry.Provider_context.ast in
        Typing.nast_to_tast ctx.Provider_context.tcopt nast
      in
      match Provider_context.get_global_context () with
      | None -> with_context ~ctx ~f:make_tast
      | Some _ ->
        (* No need for [with_context] -- assume that's already the context we're
    operating under. *)
        make_tast ())
