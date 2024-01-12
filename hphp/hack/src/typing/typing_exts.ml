(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*

Ad-hoc rules for typing some common idioms

  For printf-style functions: If the last argument (before varargs) of a
  function has the type FormatString<X>, we assume that the format
  string is interpreted by the formatter X, which should look like this:

  interface PrintfFormatter {
    function format_0x25() : string;
    function format_x(int) : string;
    function format_f(float) : string;
    function format_upcase_l() : PrintfFormatter;
  }

  Each method can return a string or another formatter (for
  multi-character sequences like %Ld); the parameters are copied into
  the function signature when instantiating it at the call site.

*)

open Hh_prelude
open Typing_defs
open Typing_env_types
open Aast
module Env = Typing_env
module Reason = Typing_reason
module Print = Typing_print
module SN = Naming_special_names

let magic_method_name input =
  match input with
  | None -> "format_eof"
  | Some c ->
    let uc = Char.uppercase c and lc = Char.lowercase c in
    if Char.equal lc uc then
      Printf.sprintf "format_0x%02x" (Char.to_int lc)
    else if Char.equal c uc then
      "format_upcase_" ^ String.make 1 lc
    else
      "format_" ^ String.make 1 lc

let lookup_magic_type (env : env) use_pos (class_ : locl_ty) (fname : string) :
    env * (locl_fun_params * locl_ty option) option =
  match get_node (Typing_utils.strip_dynamic env class_) with
  | Tclass ((_, className), _, []) ->
    let ( >>= ) = Option.( >>= ) in
    let ce_type =
      let lookup_def c =
        Option.first_some
          (Env.get_member true env c fname)
          (Env.get_member true env c "format_wild")
      in
      Env.get_class env className |> Decl_entry.to_option >>= lookup_def
      >>= fun { ce_type = (lazy ty); ce_pos = (lazy pos); _ } ->
      match deref ty with
      | (_, Tfun fty) ->
        (* Ugly hack to remove like-type from return syntactically so that in dynamic mode
         * we don't simplify it to dynamic.
         *)
        let fty =
          {
            fty with
            ft_ret =
              (match get_node fty.ft_ret with
              | Tlike ty -> ty
              | _ -> fty.ft_ret);
          }
        in
        let ety_env = empty_expand_env in
        let instantiation =
          Typing_phase.{ use_pos; use_name = fname; explicit_targs = [] }
        in
        Some
          (Typing_phase.localize_ft
             ~instantiation
             ~def_pos:pos
             ~ety_env
             env
             fty)
      | _ -> None
    in
    begin
      match ce_type with
      | Some ((env, ty_err_opt), { ft_params = pars; ft_ret = ty; _ }) ->
        Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
        let (env, ty) = Env.expand_type env ty in
        let ty_opt =
          match get_node ty with
          | Tprim Tstring -> None
          | Tdynamic -> None
          | _ -> Some ty
        in
        (env, Some (pars, ty_opt))
      | _ -> (env, None)
    end
  | _ -> (env, None)

let get_char s i =
  if i >= String.length s then
    None
  else
    Some s.[i]

let parse_printf_string env s pos (class_ : locl_ty) : env * locl_fun_params =
  let rec read_text env i : env * locl_fun_params =
    match get_char s i with
    | Some '%' -> read_modifier env (i + 1) class_ i
    | Some _ -> read_text env (i + 1)
    | None -> (env, [])
  and read_modifier env i class_ i0 : env * locl_fun_params =
    let fname = magic_method_name (get_char s i) in
    let snippet =
      String.sub s ~pos:i0 ~len:(min (i + 1) (String.length s) - i0)
    in
    let add_reason =
      List.map ~f:(fun p ->
          let fp_type =
            p.fp_type
            |> map_reason ~f:(fun r -> Reason.Rformat (pos, snippet, r))
          in
          { p with fp_type })
    in
    match lookup_magic_type env pos class_ fname with
    | (env, Some (good_args, None)) ->
      let (env, xs) = read_text env (i + 1) in
      (env, add_reason good_args @ xs)
    | (env, Some (good_args, Some next)) ->
      let (env, xs) = read_modifier env (i + 1) next i0 in
      (env, add_reason good_args @ xs)
    | (env, None) ->
      Typing_error_utils.add_typing_error
        ~env
        Typing_error.(
          primary
          @@ Primary.Format_string
               {
                 pos;
                 snippet;
                 fmt_string = s;
                 class_pos = get_pos class_;
                 fn_name = fname;
                 class_suggest = Print.full_strip_ns env class_;
               });
      let (env, xs) = read_text env (i + 1) in
      (env, add_reason xs)
  in
  read_text env 0

type ('a, 'b) either =
  | Left of 'a
  | Right of 'b

let mapM (f : 's -> 'x -> 's * 'y) : 'st -> 'x list -> 's * 'y list =
  let rec f' st xs =
    match xs with
    | [] -> (st, [])
    | x :: xs ->
      let (st', x') = f st x in
      let (st'', xs') = f' st' xs in
      (st'', x' :: xs')
  in
  f'

(* If expr is a constant string, that string, otherwise a position
   where it is obviously not *)
let rec const_string_of (env : env) (e : Nast.expr) :
    env * (Pos.t, string) either =
  let glue x y =
    match (x, y) with
    | (Right sx, Right sy) -> Right (sx ^ sy)
    | (Left p, _) -> Left p
    | (_, Left p) -> Left p
  in
  match e with
  | (_, _, String s) -> (env, Right s)
  (* It's an invariant that this is going to fail, but look for the best
   * evidence *)
  | (_, p, String2 xs) ->
    let (env, xs) = mapM const_string_of env (List.rev xs) in
    (env, List.fold_right ~f:glue xs ~init:(Left p))
  | (_, _, Binop Aast.{ bop = Ast_defs.Dot; lhs; rhs }) ->
    let (env, stra) = const_string_of env lhs in
    let (env, strb) = const_string_of env rhs in
    (env, glue stra strb)
  | (_, p, _) -> (env, Left p)

let get_format_string_type_arg t =
  match get_node t with
  | Tnewtype (fs, [ty], _) when SN.Classes.is_format_string fs -> Some ty
  | _ -> None

let rec get_possibly_like_format_string_type_arg t =
  match get_node t with
  | Tunion [t1; t2] when is_dynamic t1 ->
    get_possibly_like_format_string_type_arg t2
  | Toption t -> get_possibly_like_format_string_type_arg t
  | _ -> get_format_string_type_arg t

(* Specialize a function type using whatever we can tell about the args *)
let retype_magic_func
    (env : env)
    (ft : locl_fun_type)
    (el : (Ast_defs.param_kind * Nast.expr) list) : env * locl_fun_type =
  let rec f env param_types (args : (Ast_defs.param_kind * Nast.expr) list) :
      env * locl_fun_params option =
    match (param_types, args) with
    | ([{ fp_type; _ }], [(_, (_, _, Null))])
      when is_some (get_possibly_like_format_string_type_arg fp_type) ->
      (env, None)
    | ([({ fp_type; _ } as fp)], (_, arg) :: _) -> begin
      match get_possibly_like_format_string_type_arg fp_type with
      | Some type_arg ->
        (match const_string_of env arg with
        | (env, Right str) ->
          let (_, pos, _) = arg in
          let (env, argl) = parse_printf_string env str pos type_arg in
          ( env,
            Some
              ({ fp with fp_type = mk (get_reason fp_type, Tprim Tstring) }
              :: argl) )
        | (env, Left pos) ->
          Typing_error_utils.add_typing_error
            ~env
            Typing_error.(primary @@ Primary.Expected_literal_format_string pos);
          (env, None))
      | None -> (env, None)
    end
    | (param :: params, _ :: args) ->
      (match f env params args with
      | (env, None) -> (env, None)
      | (env, Some xs) -> (env, Some (param :: xs)))
    | _ -> (env, None)
  in
  let non_variadic_param_types =
    if get_ft_variadic ft then
      List.drop_last_exn ft.ft_params
    else
      ft.ft_params
  in
  match f env non_variadic_param_types el with
  | (env, None) -> (env, ft)
  | (env, Some xs) ->
    ( env,
      {
        ft with
        ft_params = xs;
        ft_flags = Typing_defs_flags.Fun.set_variadic false ft.ft_flags;
      } )
