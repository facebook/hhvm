(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core

module Env = Typing_env
module TLazyHeap = Typing_lazy_heap

(*****************************************************************************)
(* The place where we store the shared data in cache *)
(*****************************************************************************)

module TypeCheckStore = GlobalStorage.Make(struct
  type t = TypecheckerOptions.t
end)

let neutral = Errors.empty

(*****************************************************************************)
(* The job that will be run on the workers *)
(*****************************************************************************)

let type_fun opts fn x =
  match Parser_heap.find_fun_in_file ~full:true opts fn x with
  | Some f ->
    let fun_ = Naming.fun_ opts f in
    let def = Tast.Fun (Typing.fun_def opts fun_) in
    Tast_check.def def;
    Some def
  | None -> None

let type_class opts fn x =
  match Parser_heap.find_class_in_file ~full:true opts fn x with
  | Some cls ->
    let class_ = Naming.class_ opts cls in
    let def_opt =
      Typing.class_def opts class_
      |> Option.map ~f:(fun c -> Tast.Class c)
    in
    Option.iter def_opt Tast_check.def;
    def_opt
  | None -> None

let check_typedef opts fn x =
  match Parser_heap.find_typedef_in_file ~full:true opts fn x with
  | Some t ->
    let typedef = Naming.typedef opts t in
    let ret = Typing.typedef_def opts typedef in
    Typing_variance.typedef opts x;
    let def = Tast.Typedef ret in
    Tast_check.def def;
    Some def
  | None -> None

let check_const opts fn x =
  match Parser_heap.find_const_in_file ~full:true opts fn x with
  | None -> None
  | Some cst ->
    let cst = Naming.global_const opts cst in
    let def = Tast.Constant (Typing.gconst_def opts cst) in
    Tast_check.def def;
    Some def

let check_file dynamic_view_files opts errors (fn, file_infos) =

  let opts = {
      opts with
      GlobalOptions.tco_dynamic_view = Relative_path.Set.mem dynamic_view_files fn;
  } in
  let { FileInfo.n_funs; n_classes; n_types; n_consts } = file_infos in
  let ignore_type_fun opts fn name =
    ignore(type_fun opts fn name) in
  let ignore_type_class opts fn name =
    ignore(type_class opts fn name) in
  let ignore_check_typedef opts fn name =
    ignore(check_typedef opts fn name) in
  let ignore_check_const opts fn name =
    ignore(check_const opts fn name) in
  let errors', () = Errors.do_with_context fn Errors.Typing
      begin fun () ->
    SSet.iter (ignore_type_fun opts fn) n_funs;
    SSet.iter (ignore_type_class opts fn) n_classes;
    SSet.iter (ignore_check_typedef opts fn) n_types;
    SSet.iter (ignore_check_const opts fn) n_consts;
  end in
  Errors.merge errors' errors

let check_files dynamic_view_files opts errors fnl  =
  SharedMem.invalidate_caches();
  File_heap.FileHeap.LocalChanges.push_stack ();
  Parser_heap.ParserHeap.LocalChanges.push_stack ();
  let check_file =
    if !Utils.profile
    then (fun acc fn ->
      let t = Sys.time () in
      let result = check_file dynamic_view_files opts acc fn in
      let t' = Sys.time () in
      let duration = t' -. t in
      let filepath = Relative_path.suffix (fst fn) in
        TypingLogger.log_typing_time duration filepath;
        !Utils.log (Printf.sprintf "%f %s [type-check]" duration filepath);
        result)
    else check_file dynamic_view_files opts in
  let errors = List.fold_left fnl ~f:check_file ~init:errors in
  TypingLogger.flush_buffer ();
  Parser_heap.ParserHeap.LocalChanges.pop_stack ();
  File_heap.FileHeap.LocalChanges.pop_stack ();
  errors

let load_and_check_files dynamic_view_files acc fnl =
  let opts = TypeCheckStore.load() in
  check_files dynamic_view_files opts acc fnl

(*****************************************************************************)
(* Let's go! That's where the action is *)
(*****************************************************************************)

let parallel_check dynamic_view_files workers opts fnl ~interrupt =
  TypeCheckStore.store opts;
  let result, env, cancelled =
    MultiWorker.call_with_interrupt
      workers
      ~job:(load_and_check_files dynamic_view_files)
      ~neutral
      ~merge:Decl_service.merge_lazy_decl
      ~next:(MultiWorker.next workers fnl)
      ~interrupt
  in
  TypeCheckStore.clear();
  result, env, List.concat cancelled

type 'a job = Relative_path.t * 'a
type ('a, 'b, 'c) job_result = 'b * 'c * 'a job list

module type Mocking_sig = sig
  val with_test_mocking :
    (* real job payload, that we can modify... *)
    'a job list ->
    (* ... before passing it to the real job executor... *)
    ('a job list -> ('a, 'b, 'c) job_result) ->
    (* ... which output we can also modify. *)
    ('a, 'b, 'c) job_result
end

module NoMocking = struct
  let with_test_mocking fnl f = f fnl
end

module TestMocking = struct

  let cancelled = ref Relative_path.Set.empty
  let set_is_cancelled x =
    cancelled := Relative_path.Set.add !cancelled x
  let is_cancelled x = Relative_path.Set.mem !cancelled x

  let with_test_mocking fnl f =
    let mock_cancelled, fnl =
      List.partition_tf fnl ~f:(fun (path, _) -> is_cancelled path) in
    (* Only cancel once to avoid infinite loops *)
    cancelled := Relative_path.Set.empty;
    let res, env, cancelled = f fnl in
    res, env, mock_cancelled @ cancelled
end

module Mocking =
  (val (if Injector_config.use_test_stubbing
  then (module TestMocking : Mocking_sig)
  else (module NoMocking : Mocking_sig)
))

let go_with_interrupt workers opts dynamic_view_files fast ~interrupt =
  let fnl = Relative_path.Map.elements fast in
  Mocking.with_test_mocking fnl @@ fun fnl ->
    if List.length fnl < 10
    then check_files dynamic_view_files opts neutral fnl, interrupt.MultiThreadedCall.env, []
    else parallel_check dynamic_view_files workers opts fnl ~interrupt

let go workers opts dynamic_view_files fast =
  let interrupt = MultiThreadedCall.no_interrupt () in
  let res, (), cancelled =
    go_with_interrupt workers opts dynamic_view_files fast interrupt in
  assert (cancelled = []);
  res
