(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module T = Inline_method_types

(** We convert a `return` in the inlined method to a variable assignment.
 *  This is the name for the variable, modulo renaming for hygiene *)
let return_var_raw_name = "$res"

let pos_of_block block =
  let open Option.Let_syntax in
  let* (hd_pos, _) = List.hd block in
  let* (last_pos, _) = List.last block in
  let pos = Pos.merge hd_pos last_pos in
  Option.some_if (Pos.length pos > 0) pos

type binding =
  | Argument of int * Pos.t
  | Default of Pos.t
  | Positional_variadic of (int * Pos.t) list
  | Named_variadic of (int * Pos.t) list

let bind_args ~call_args ~(params : T.param_info list) : binding list option =
  let open Option.Let_syntax in
  let (named_args, positional_args) =
    List.foldi call_args ~init:([], []) ~f:(fun index acc (name_opt, pos) ->
        let (named, positional) = acc in
        match name_opt with
        | Some name -> ((name, (index, pos)) :: named, positional)
        | None -> (named, (index, pos) :: positional))
  in
  let named_args = List.rev named_args in
  let* named_arg_map =
    match String.Map.of_alist named_args with
    | `Ok map -> Some map
    | `Duplicate_key _ -> None
  in
  let declared_named_params =
    params
    |> List.filter ~f:(fun p -> p.T.named && not p.T.variadic)
    |> List.map ~f:(fun p -> String.chop_prefix_if_exists ~prefix:"$" p.T.name)
    |> String.Set.of_list
  in
  let extra_named_args =
    List.filter_map named_args ~f:(fun (name, arg) ->
        Option.some_if (not (Set.mem declared_named_params name)) arg)
  in
  let* () =
    Option.some_if
      (List.is_empty extra_named_args
      || List.exists params ~f:(fun p -> p.T.named && p.T.variadic))
      ()
  in
  let rec bind positional = function
    | [] -> Option.some_if (List.is_empty positional) []
    | param :: params ->
      let* (binding, positional) =
        if param.T.variadic then
          if param.T.named then
            Some (Named_variadic extra_named_args, positional)
          else
            Some (Positional_variadic positional, [])
        else
          let (arg, positional) =
            if param.T.named then
              ( Map.find
                  named_arg_map
                  (String.chop_prefix_if_exists ~prefix:"$" param.T.name),
                positional )
            else
              match positional with
              | arg :: positional -> (Some arg, positional)
              | [] -> (None, [])
          in
          match arg with
          | Some (index, pos) -> Some (Argument (index, pos), positional)
          | None ->
            Option.map param.T.default ~f:(fun pos -> (Default pos, positional))
      in
      let+ rest = bind positional params in
      binding :: rest
  in
  bind (List.rev positional_args) params

let calc_pre_assignments_text
    r
    ~source_text
    ~bindings
    ~renamed_param_names
    ~callee_vars
    ~arg_evaluation_order : Inline_method_rename.t * string * string list =
  let expr pos = Full_fidelity_source_text.sub_of_pos source_text pos in
  let rec safe_temp_name name =
    if Set.mem callee_vars name then
      safe_temp_name (name ^ "_")
    else
      name
  in
  let (r, before, after, cleanup) =
    List.zip_exn renamed_param_names bindings
    |> List.fold
         ~init:(r, [], [], [])
         ~f:(fun (r, before, after, cleanup) (param, binding) ->
           match binding with
           | Argument (index, pos) ->
             ( r,
               (index, Format.sprintf "%s = %s;" param (expr pos)) :: before,
               after,
               cleanup )
           | Default pos ->
             ( r,
               before,
               Format.sprintf "%s = %s;" param (expr pos) :: after,
               cleanup )
           | Positional_variadic args ->
             let (r, before, temps) =
               List.fold
                 args
                 ~init:(r, before, [])
                 ~f:(fun (r, before, temps) (index, pos) ->
                   let raw_name =
                     safe_temp_name
                       (Format.sprintf "$__inline_method_arg_%d" index)
                   in
                   let (r, temp) = Inline_method_rename.rename r raw_name in
                   ( r,
                     (index, Format.sprintf "%s = %s;" temp (expr pos))
                     :: before,
                     temp :: temps ))
             in
             let vec = String.concat ~sep:", " (List.rev temps) in
             let release_args =
               temps
               |> List.map ~f:(Format.sprintf "%s = null;")
               |> String.concat ~sep:" "
             in
             ( r,
               before,
               Format.sprintf "%s = vec[%s]; %s" param vec release_args :: after,
               param :: cleanup )
           | Named_variadic args ->
             let (r, before, cleanup) =
               List.fold
                 args
                 ~init:(r, before, cleanup)
                 ~f:(fun (r, before, cleanup) (index, pos) ->
                   let raw_name =
                     safe_temp_name
                       (Format.sprintf "$__inline_method_named_arg_%d" index)
                   in
                   let (r, temp) = Inline_method_rename.rename r raw_name in
                   ( r,
                     (index, Format.sprintf "%s = %s;" temp (expr pos))
                     :: before,
                     temp :: cleanup ))
             in
             (r, before, after, cleanup))
  in
  let by_index = Int.Map.of_alist_exn before in
  let before = List.map arg_evaluation_order ~f:(Map.find_exn by_index) in
  (r, String.concat ~sep:" " (before @ List.rev after), cleanup)

let calc_body_text r path ~source_text T.{ callee; _ } :
    Inline_method_rename.t * string =
  let block_source_text =
    Option.(
      pos_of_block callee.T.block
      >>| Full_fidelity_source_text.sub_of_pos source_text
      |> value ~default:"")
  in
  Inline_method_rewrite_block.rewrite_block
    r
    path
    block_source_text
    ~return_var_raw_name

let strip_leading_spaces ~source_text pos : Pos.t =
  let strip_length =
    pos
    |> Pos.set_col_start 0
    |> Full_fidelity_source_text.sub_of_pos source_text
    |> String.take_while ~f:(Char.equal ' ')
    |> String.length
  in
  let col_start = snd @@ Pos.line_column pos in
  pos |> Pos.set_col_start (col_start - strip_length)

(** Add the inlined method contents before the call site and
 * calculate a variable name for the `return` of the inlined method.
 * We use the return variable when replacing the call site.
 * *)
let edit_inline_and_return_var_of_candidate
    path ~source_text ~bindings (T.{ caller; callee; call } as candidate) :
    Code_action_types.edit * string =
  let used_vars = String.Set.of_list caller.T.var_names in
  let r = Inline_method_rename.create ~used_vars in
  let (r, param_names) =
    Inline_method_rename.rename_all
      r
      (List.map callee.T.params ~f:(fun p -> p.T.name))
  in
  let callee_vars =
    String.Set.of_list
      (callee.T.var_names @ List.map callee.T.params ~f:(fun p -> p.T.name))
  in
  let (r, assignments_before_body, cleanup) =
    calc_pre_assignments_text
      r
      ~source_text
      ~bindings
      ~renamed_param_names:param_names
      ~callee_vars
      ~arg_evaluation_order:call.T.arg_evaluation_order
  in
  let (r, body) = calc_body_text r path ~source_text candidate in
  (* Gives the correct `return_var` because `Inline_method_rename.rename` is idempotent *)
  let (_r, return_var) = Inline_method_rename.rename r return_var_raw_name in
  let pos = Pos.shrink_to_start call.T.call_stmt_pos in
  let start_indent_amount = snd @@ Pos.line_column call.T.call_stmt_pos in
  let cleanup =
    cleanup
    |> List.map ~f:(Format.sprintf "%s = null;")
    |> String.concat ~sep:" "
  in
  let text =
    assignments_before_body ^ body ^ cleanup
    |> Inline_method_rewrite_block.format_block ~start_indent_amount path
    |> String.lstrip
  in
  (Code_action_types.{ pos; text }, return_var)

(** Replace the call with a variable (`return_var`) and adjust *)
let edit_replace_call_of_candidate
    ~source_text ~(return_var : string) T.{ call; callee; _ } :
    Code_action_types.edit =
  if callee.T.has_void_return then
    let strip_trailing_semicolon p =
      let semicolon_pos = Pos.shrink_to_end p in
      if
        String.equal
          ";"
          (Full_fidelity_source_text.sub_of_pos
             source_text
             ~length:1
             semicolon_pos)
      then
        Pos.advance_one p
      else
        p
    in
    let pos = call.T.call_pos |> strip_trailing_semicolon in
    Code_action_types.{ pos; text = "" }
  else
    let pos = call.T.call_pos in
    Code_action_types.{ pos; text = return_var }

(** Remove the inlined method *)
let edit_remove_method_of_candidate ~source_text candidate :
    Code_action_types.edit =
  let pos =
    T.(candidate.callee.method_pos) |> strip_leading_spaces ~source_text
  in
  let text = "" in
  Code_action_types.{ pos; text }

let edits_of_candidate ~source_text ~path ~bindings candidate :
    Code_action_types.edits =
  let edit_remove_method =
    edit_remove_method_of_candidate ~source_text candidate
  in
  let (edit_inline, return_var) =
    edit_inline_and_return_var_of_candidate
      path
      ~source_text
      ~bindings
      candidate
  in
  let edit_replace_call =
    edit_replace_call_of_candidate ~source_text ~return_var candidate
  in
  let call_edits =
    if T.(candidate.callee.has_void_return) then
      [
        Code_action_types.
          { edit_replace_call with text = String.rstrip edit_inline.text };
      ]
    else
      [edit_inline; edit_replace_call]
  in
  Relative_path.Map.singleton path (edit_remove_method :: call_edits)

let to_refactor ~source_text ~path (T.{ call; callee; _ } as candidate) =
  let open Option.Let_syntax in
  let+ bindings =
    bind_args ~call_args:call.T.call_args ~params:callee.T.params
  in
  let edits =
    lazy (edits_of_candidate ~source_text ~path ~bindings candidate)
  in
  Code_action_types.(
    Refactor
      {
        title = "Inline method";
        edits;
        selection = None;
        trigger_inline_suggest = false;
      })
