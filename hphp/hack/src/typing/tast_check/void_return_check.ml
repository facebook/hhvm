(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
open Hh_prelude

(* This check enforces 3 properties related to return statements and functions' return types:

   Property 1: A function annotated with return type void must not contain a return statement with
     a value.

   Property 2: A function containing a return statement without a value (or returning implicitly)
     can only be annotated with a return type if that return type is void (or
     <Awaitable<void>, for async functions).

   Property 3: A function must not mix return statements with and without a value, even if such a function
     could be given a sufficiently general type.
*)

type state = {
  fun_def_pos: Pos.t;  (** Position of the currently active function *)
  return_type: (Pos.t * Aast.hint_) option;
      (** Return type that the active function is annotated with.
         None if no annotation present. *)
  prev_no_value_return: Pos.t option option;
      (** Some if the active function can return without a value.
        If we only know that the function implicitly returns somewhere,
        the position is empty
        If we have seen an explicit return;, we store the location here *)
  prev_value_return: Pos.t option;
      (** Some if we have seen a return foo; in the active function.
        In that case we store the location here. *)
  active: bool;
      (** Indicates whether the state has already been used to traverse a function. *)
}

let initial_dummy_state =
  {
    return_type = None;
    prev_no_value_return = None;
    prev_value_return = None;
    fun_def_pos = Pos.none;
    active = false;
  }

let validate_state fun_span fun_kind env s =
  (* FIXME: Move as two functions to Ast_defs? *)
  let (is_generator, is_async) =
    let open Ast_defs in
    match fun_kind with
    | FGenerator -> (true, false)
    | FAsyncGenerator -> (true, true)
    | FSync -> (false, false)
    | FAsync -> (false, true)
  in
  let ret_type_hint_locl_opt =
    Option.map s.return_type ~f:(fun ty_hint ->
        Tast_env.hint_to_ty env ty_hint
        |> Tast_env.localize_no_subst env ~ignore_errors:true
        |> snd)
  in
  let is_bad_supertype sub sup =
    let sup =
      if TypecheckerOptions.enable_sound_dynamic (Tast_env.get_tcopt env) then
        Typing_defs.(
          map_ty sup ~f:(function
              | Tclass (cid, ex, [ty])
                when String.equal
                       (snd cid)
                       Naming_special_names.Classes.cAwaitable ->
                let tenv = Tast_env.tast_env_as_typing_env env in
                Tclass (cid, ex, [Typing_utils.strip_dynamic tenv ty])
              | x -> x))
      else
        sup
    in

    (* returns false if sup is TAny, which implements the special behavior for
       functions annotated with variations of TAny *)
    Tast_env.is_sub_type env sub sup && not (Tast_env.is_sub_type env sup sub)
  in
  let check_ret_type ret_type_hint_locl =
    (* Fixme: Should we use more precise logic to determine the expected
       return type hint by factoring it into a function in Typing_return? *)
    let void = Typing_make_type.void Typing_reason.Rnone in
    let aw_void = Typing_make_type.awaitable Typing_reason.Rnone void in
    let is_void_super_ty = is_bad_supertype void ret_type_hint_locl in
    let is_awaitable_void_super_ty =
      is_bad_supertype aw_void ret_type_hint_locl
    in
    if is_void_super_ty || is_awaitable_void_super_ty then
      let hint_loc =
        match s.return_type with
        | None -> fun_span
        | Some (hint_loc, _) -> hint_loc
      in
      ( false,
        lazy
          (Typing_error_utils.add_typing_error
             ~env:(Tast_env.tast_env_as_typing_env env)
             Typing_error.(
               wellformedness
               @@ Primary.Wellformedness
                  .Non_void_annotation_on_return_void_function
                    { is_async; hint_pos = hint_loc })) )
    else
      (true, lazy ())
  in

  (* Property 2  There are several exceptions:
     - We do not complain if the function is annotated with Tany or Awaitable<Tany>
       (e.g., using one of its aliases in using one of its aliases in HH_FIXME)
     - We only actually report an error if the return type hint of the function is a
       proper supertype of void (other than Tany, see first exception). Otherwise you get a normal
       type error already anyway.
  *)
  let (property_2_ok, raise_error_2) =
    match (s.prev_no_value_return, ret_type_hint_locl_opt, is_generator) with
    | (Some _, Some ret_type_hint_locl, false) ->
      check_ret_type ret_type_hint_locl
    | _ -> (true, lazy ())
  in

  (* Property 3: Again, there are several exceptions:
     - If the function is async, it is allowed to have a (implicit or explicit) no-value return and
       also a "return foo;", where the type of foo is void-typed (e.g., a call to a void-returning
       function or an wait of an Awaitable<void>).
     - If the function is annotated with return type TAny (e.g., using one of its aliases in
       HH_FIXME), we do not complain.
     - return await foo.
  *)
  let (property_3_ok, raise_error_3) =
    (* We must accommodate the slightly inconsistent rule that "return await foo;" is allowed in an
       async function of type Awaitable<void> if foo's type is Awaitable<void>.
       The test below currently lets the following corner case slip through:
       An async lambda without a return type annotation can combine returning with and
       without a value if we can otherwise type the lambda with an sufficiently general type like
       Awaitable<mixed> *)
    match (s.prev_no_value_return, s.prev_value_return, is_async) with
    | (Some without_value_pos_opt, Some with_value_pos, false) ->
      let fun_pos = s.fun_def_pos in
      ( false,
        lazy
          (Typing_error_utils.add_typing_error
             ~env:(Tast_env.tast_env_as_typing_env env)
             Typing_error.(
               wellformedness
               @@ Primary.Wellformedness.Returns_with_and_without_value
                    { pos = fun_pos; with_value_pos; without_value_pos_opt }))
      )
    | _ -> (true, lazy ())
  in
  (* Don't report violations of property 2 if prop 3 is also violated *)
  match (property_2_ok, property_3_ok) with
  | (_, false) -> Lazy.force raise_error_3
  | (false, _) -> Lazy.force raise_error_2
  | _ -> ()

let visitor =
  object (this)
    inherit [_] Aast.iter as super

    (** The state is only ever updated when entering a new function/method/lambda body
        Additionally, the prev_return_form part of the state may be updated by on_return_form
        while traversing a function body when we see a return statement. *)
    val state = ref initial_dummy_state

    method traverse_fun_body
        fun_span
        new_return_type
        new_fun_pos
        fun_kind
        has_implicit_return
        env
        traversal =
      let initial_no_value_return =
        if has_implicit_return then
          (* There is an implicit return but we don't know where *)
          Some None
        else
          None
      in
      if !state.active then
        (* We are in a function/method already.
           We do not descend into the nested function/method at this point. The handler
           (defined below) will always traverse any nested functions/methods. Thus, if we
           traversed the nested function/method here, the handler would *also* traverse it later
           and we would report the same errors multiple times *)
        ()
      else
        let new_state =
          {
            return_type = new_return_type;
            prev_no_value_return = initial_no_value_return;
            prev_value_return = None;
            fun_def_pos = new_fun_pos;
            active = true;
          }
        in
        state := new_state;
        traversal ();
        validate_state fun_span fun_kind env !state

    method reset = state := initial_dummy_state

    method update_seen_return_stmts with_value return_pos =
      if with_value then
        state := { !state with prev_value_return = Some return_pos }
      else
        state := { !state with prev_no_value_return = Some (Some return_pos) }

    method! on_expr_ env expr_ =
      match expr_ with
      | Aast.Invalid _ -> ()
      | _ -> super#on_expr_ env expr_

    method! on_fun_ env fun_ =
      let decl_env = Tast_env.get_decl_env env in
      let has_impl_ret = Tast_env.fun_has_implicit_return env in
      if
        not
          (FileInfo.(equal_mode decl_env.Decl_env.mode Mhhi)
          || Typing_native.is_native_fun
               ~env:(Tast_env.tast_env_as_typing_env env)
               fun_)
      then
        this#traverse_fun_body
          fun_.f_span
          (hint_of_type_hint fun_.f_ret)
          fun_.f_span
          fun_.f_fun_kind
          has_impl_ret
          env
          (fun () -> super#on_fun_ env fun_)

    method! on_method_ env method_ =
      let has_impl_ret = Tast_env.fun_has_implicit_return env in
      let decl_env = Tast_env.get_decl_env env in
      if
        not
          (method_.m_abstract
          || FileInfo.(equal_mode decl_env.Decl_env.mode Mhhi)
          || Typing_native.is_native_meth
               ~env:(Tast_env.tast_env_as_typing_env env)
               method_)
      then
        this#traverse_fun_body
          method_.m_span
          (hint_of_type_hint method_.m_ret)
          (fst method_.m_name)
          method_.m_fun_kind
          has_impl_ret
          env
          (fun () -> super#on_method_ env method_)

    method! on_stmt env st =
      (* Always call super to make sure the expressions in return statements are traversed *)
      super#on_stmt env st;
      match st with
      | (return_pos, Return (Some _)) ->
        this#update_seen_return_stmts true return_pos;
        begin
          match !state.return_type with
          | Some (pos2, Hprim Tvoid) ->
            (* Property 1 *)
            Typing_error_utils.add_typing_error
              ~env:(Tast_env.tast_env_as_typing_env env)
              Typing_error.(
                primary
                @@ Primary.Return_in_void { pos = return_pos; decl_pos = pos2 })
          | _ -> ()
        end
      | (return_pos, Return None) ->
        this#update_seen_return_stmts false return_pos
      | _ -> ()

    method! on_expr env e =
      match e with
      | (_, _, Aast.Invalid _) -> ()
      | _ -> super#on_expr env e
  end

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_method_ env x =
      let env = Tast_env.restore_method_env env x in
      visitor#reset;
      visitor#on_method_ env x

    method! at_fun_ env x =
      let env = Tast_env.restore_fun_env env x in
      visitor#reset;
      visitor#on_fun_ env x
  end
