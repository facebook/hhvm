(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)


(** Module used to suggest type annotations when they are missing
*)
open Core_kernel
open Typing_defs
open Utils

let compare_types x y =
  let tcopt = GlobalNamingOptions.get () in
  let tenv = Typing_env.empty tcopt Relative_path.default ~droot:None in
  String.compare
    (Typing_print.full tenv x) (Typing_print.full tenv y)

type hint_kind =
  | Kmember of string
  | Kparam of string
  | Kreturn

let string_of_kind = function
  | Kmember s -> "Kmember:"^s
  | Kparam s -> "Kparam:"^s
  | Kreturn -> "Kreturn"

module Env = Typing_env
module TUtils = Typing_utils

(*****************************************************************************)
(* List of functions or methods without return types  *)
(*****************************************************************************)
let (funs_and_methods : (Ast.id list ref)) =  ref []

let save_fun_or_method name =
  if !is_suggest_mode then funs_and_methods := name :: !funs_and_methods
(*****************************************************************************)
(* List of types found in a file. *)
(*****************************************************************************)
let (types: (Env.env * Pos.t * hint_kind * locl ty) list ref) = ref []
let (initialized_members: (SSet.t SMap.t) ref) = ref SMap.empty

let add_type env pos k type_ =
  let tcopt = Env.get_tcopt env in
  let new_env = Env.empty tcopt Relative_path.default ~droot:None in
  let new_type = (
    (* Some stuff in env isn't serializable, which we need so that we can infer
     * types part of the codebase at a time in worker threads. Fortunately we
     * don't actually need the whole env, so just keep the parts we do need for
     * typing, which *are* serializable. *)
    {new_env with Env.tenv = env.Env.tenv; Env.subst = env.Env.subst},
    pos,
    k,
    type_
  ) in
  types := new_type :: !types

(*****************************************************************************)
(* Primitives used to save types (cf typing.ml). *)
(*****************************************************************************)

let save_type hint_kind env x arg =
  if !is_suggest_mode then begin
    match Typing_expand.fully_expand env x with
    | _, Tany ->
        let earg = Typing_expand.fully_expand env arg in
        (match earg with
        | _, Tany -> ()
        | _  ->
            let x_pos = Reason.to_pos (fst x) in
            add_type env x_pos hint_kind arg;
        )
    | _, (Terr | Tnonnull | Tarraykind _ | Tprim _ | Toption _ | Tdynamic
      | Tvar _ | Tabstract _ | Tclass _ | Ttuple _ | Tanon _
      | Tfun _ | Tunresolved _ | Tobject | Tshape _) -> ()
  end

let save_return env x arg = save_type Kreturn env x arg
let save_member name env x arg = save_type (Kmember name) env x arg
let save_param name env x arg = save_type (Kparam name) env x arg

(* Called when a member variable doesn't have a static initalizer, such as:
 *
 * class C {
 *   private $x;
 *   // As opposed to:
 *   // private $x = 42;
 * }
 *
 *)
let uninitialized_member cname mname env x arg =
  if !is_suggest_mode then begin
    match SMap.get cname !initialized_members with
      (* No static initalizer and no initalization in the constructor means that
       * this variable can be used before it's written to, and thus must be
       * nullable. *)
      | Some inits ->
        if not (SSet.mem mname inits)
        then save_member mname env x (fst x, Toption arg)

      (* Some constructions, such as traits, don't calculate initialized members.
       * TODO: this will suggest wrong types for some member variables defined in
       * traits, since they might be nullable, but that depends on the constructor
       * of the class that includes the trait (!). Not sure how to deal with this
       * right now, will just let the "revert bad patch" logic take care of it. *)
      | None -> ()
  end

let save_initialized_members cname inits = if !is_suggest_mode then begin
  initialized_members := SMap.add cname inits !initialized_members
end

(** returns the classes/interfaces implemented by a class
 * we are only interested in the non-parametric ones, infering
 * the parameter would be too hard anyway.
 *)
let get_implements (_, x) =
  match Typing_lazy_heap.get_class x with
  | None -> SSet.empty
  | Some cls ->
      let tyl = Typing_classes_heap.all_ancestors cls in
      Sequence.fold tyl ~init:SSet.empty ~f:begin fun set (_, ty) ->
        match ty with
        | _, Tapply ((_, x), []) -> SSet.add x set
        | _,
          (
            Tany
            | Tdynamic
            | Terr
            | Tmixed
            | Tnonnull
            | Tnothing
            | Tarray (_, _)
            | Tdarray (_, _)
            | Tvarray _
            | Tvarray_or_darray _
            | Tprim _
            | Tgeneric _
            | Tfun _
            | Toption _
            | Tlike _
            | Tapply (_, _)
            | Ttuple _
            | Tshape _
            | Taccess (_, _)
            | Tthis
          ) ->
            raise Exit
      end

(** normalizes a "guessed" type. We basically want to bailout whenever
 * the inferred type doesn't resolve to a type hint.
 *)
let rec normalize (r, ty) = r, normalize_ ty
and normalize_ = function
  | Tunresolved [x] -> snd (normalize x)
  | Tunresolved tyl
    when List.exists tyl (function _, Toption _ -> true | _ -> false) ->
      let tyl = List.map tyl (function _, Toption ty -> ty | x -> x) in
      normalize_ (Toption (Reason.Rnone, Tunresolved tyl))
  | Tunresolved tyl
    when List.exists tyl
    (function _, (Tany | Tunresolved []) -> true | _ -> false) ->
      let tyl = List.filter tyl begin function
        |  _, (Tany |  Tunresolved []) -> false
        | _, (Terr | Tnonnull | Tarraykind _ | Tprim _ | Toption _
          | Tvar _ | Tabstract _ | Tclass _ | Ttuple _
          | Tanon (_, _) | Tfun _ | Tunresolved _ | Tobject | Tshape _
          | Tdynamic
             ) -> true
      end in
      normalize_ (Tunresolved tyl)
  | Tunresolved ((_, Tclass (x, e, [])) :: rl) ->
      (* If we have A & B & C where all the elements are classes
       * we try to find a unique common ancestor.
       *)
      let rl = List.map rl begin function
        | _, Tclass (x, _, []) -> x
        | _, (Terr | Tany | Tnonnull | Tarraykind _ | Tprim _ | Tdynamic
          | Toption _ | Tvar _ | Tabstract _ | Tclass _ | Ttuple _
          | Tanon (_, _) | Tfun _ | Tunresolved _ | Tobject
          | Tshape _) -> raise Exit
      end in
      let x_imp = get_implements x in
      let set = List.fold_left rl ~f:begin fun x_imp x ->
        SSet.inter x_imp (get_implements x)
      end ~init:x_imp in
      (* is it unique? *)
      if SSet.cardinal set = 1
      then Tclass ((Pos.none, SSet.choose set), e, [])
      else raise Exit
  | Tunresolved (x :: (y :: _ as rl)) when compare_types x y = 0 ->
      normalize_ (Tunresolved rl)
  | Tunresolved _ | Tany -> raise Exit
  | Tnonnull -> Tnonnull
  | Terr -> Terr
  | Tarraykind akind -> begin
    try
      Tarraykind (match akind with
        | AKany -> AKany
        | AKempty -> AKempty
        | AKvarray tk -> AKvarray (normalize tk)
        | AKvec tk -> AKvec (normalize tk)
        | AKdarray (tk, tv) -> AKdarray (normalize tk, normalize tv)
        | AKmap (tk, tv) -> AKmap (normalize tk, normalize tv)
        | AKvarray_or_darray tv -> AKvarray_or_darray (normalize tv)
      )
    with Exit -> Tarraykind AKany
  end
  | Tabstract (AKgeneric _, _) as x -> x
  | Tabstract (AKdependent _, Some ty) -> normalize_ (snd ty)
  | Toption (_, (Toption (_, _) as ty)) -> normalize_ ty
  | Toption (_, Tprim Nast.Tvoid) -> raise Exit
  | Toption ty -> Toption (normalize ty)
  | Tprim _ as ty -> ty
  | Tvar _ -> raise Exit
  | Tfun _ -> raise Exit
  | Tclass ((pos, name), e, tyl) when name.[0] = '\\' && String.rindex_exn name '\\' = 0 ->
      (* TODO this transform isn't completely legit; can cause a reference into
       * the global namespace to suddenly refer to a different class in the
       * local one. Figure something else out that doesn't involve spamming '\'
       * across FB code, maybe? See if anyone complains on GitHub? I have no
       * idea how bad this is in practice, I'm kinda hoping it's okay. *)
      normalize_ (Tclass ((pos, strip_ns name), e, tyl))
  | Tclass ((pos1, "Awaitable"), e, [(_, Toption (pos2, Tprim Nast.Tvoid))]) ->
      (* Special case: Awaitable<?void> is nonsensical, but often
       * Awaitable<void> works. *)
      Tclass ((pos1, "Awaitable"), e, [(pos2, Tprim Nast.Tvoid)])
  | Tclass ((pos, name), e, tyl) ->
      (* Handling xhp names *)
      let name =
        if String.contains name ':' && name.[0] <> ':'
        then ":"^name
        else name
      in
      Tclass ((pos, name), e, List.map tyl (normalize))
  | Ttuple tyl -> Ttuple (List.map tyl (normalize))
  | Tdynamic -> Tdynamic
  | Tanon _ -> raise Exit
  | Tobject -> raise Exit
  | Tabstract _ -> raise Exit
  | Tshape _ -> raise Exit

let normalize ty =
  try
    Some (normalize ty)
  with Exit -> None
