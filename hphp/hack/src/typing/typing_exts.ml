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

open Core_kernel
open Typing_defs
open Typing_env_types
open Aast
module Env = Typing_env
module Reason = Typing_reason
module Print = Typing_print
module SN = Naming_special_names
module Partial = Partial_provider

let magic_method_name input =
  match input with
  | None -> "format_eof"
  | Some c ->
    let uc = Char.uppercase c and lc = Char.lowercase c in
    if phys_equal lc uc then
      Printf.sprintf "format_0x%02x" (Char.to_int lc)
    else if phys_equal c uc then
      "format_upcase_" ^ String.make 1 lc
    else
      "format_" ^ String.make 1 lc

let lookup_magic_type (env : env) (class_ : locl_ty) (fname : string) :
    env * (locl_fun_params * locl_ty option) option =
  match class_ with
  | (_, Tunion [(_, Tdynamic); (_, Tclass ((_, className), _, []))])
  | (_, Tclass ((_, className), _, [])) ->
    let ( >>= ) = Option.( >>= ) in
    let ce_type =
      Env.get_class env className
      >>= (fun c -> Env.get_member true env c fname)
      >>= fun { ce_type = (lazy ty); _ } ->
      match ty with
      | (_, Tfun fty) ->
        let ety_env = Typing_phase.env_with_self env in
        let instantiation =
          Typing_phase.
            { use_pos = fty.ft_pos; use_name = fname; explicit_targs = [] }
        in
        Some (Typing_phase.localize_ft ~instantiation ~ety_env env fty)
      | _ -> None
    in
    begin
      match ce_type with
      | Some (env, { ft_params = pars; ft_ret = { et_type = ty; _ }; _ }) ->
        let ty_opt =
          match ty with
          | (_, Tprim Tstring) -> None
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
    let snippet = String.sub s i0 (min (i + 1) (String.length s) - i0) in
    let add_reason =
      List.map ~f:(fun p ->
          let (why, ty) = p.fp_type.et_type in
          {
            p with
            fp_type =
              {
                p.fp_type with
                et_type = (Reason.Rformat (pos, snippet, why), ty);
              };
          })
    in
    match lookup_magic_type env class_ fname with
    | (env, Some (good_args, None)) ->
      let (env, xs) = read_text env (i + 1) in
      (env, add_reason good_args @ xs)
    | (env, Some (good_args, Some next)) ->
      let (env, xs) = read_modifier env (i + 1) next i0 in
      (env, add_reason good_args @ xs)
    | (env, None) ->
      Errors.format_string
        pos
        snippet
        s
        (Reason.to_pos (fst class_))
        fname
        (Print.full_strip_ns env class_);
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
  | (_, String s) -> (env, Right s)
  (* It's an invariant that this is going to fail, but look for the best
   * evidence *)
  | (p, String2 xs) ->
    let (env, xs) = mapM const_string_of env (List.rev xs) in
    (env, List.fold_right ~f:glue xs ~init:(Left p))
  | (_, Binop (Ast_defs.Dot, a, b)) ->
    let (env, stra) = const_string_of env a in
    let (env, strb) = const_string_of env b in
    (env, glue stra strb)
  | (p, _) -> (env, Left p)

(* Specialize a function type using whatever we can tell about the args *)
let retype_magic_func (env : env) (ft : locl_fun_type) (el : Nast.expr list) :
    env * locl_fun_type =
  let rec f env param_types args : env * locl_fun_params option =
    match (param_types, args) with
    | ( [
          {
            fp_type =
              { et_type = (_, Toption (_, Tclass ((_, fs), _, [_]))); _ };
            _;
          };
        ],
        [(_, Null)] )
      when SN.Classes.is_format_string fs ->
      (env, None)
    | ( [
          ( {
              fp_type =
                {
                  et_type = (why, Toption (_, Tclass ((_, fs), _, [type_arg])));
                  _;
                };
              _;
            } as fp );
        ],
        arg :: _ )
    | ( [
          ( {
              fp_type = { et_type = (why, Tclass ((_, fs), _, [type_arg])); _ };
              _;
            } as fp );
        ],
        arg :: _ )
    | ( [
          ( {
              fp_type =
                {
                  et_type =
                    ( why,
                      Tunion
                        [(_, Tdynamic); (_, Tclass ((_, fs), _, [type_arg]))]
                    );
                  _;
                };
              _;
            } as fp );
        ],
        arg :: _ )
      when SN.Classes.is_format_string fs ->
      (match const_string_of env arg with
      | (env, Right str) ->
        let (env, argl) = parse_printf_string env str (fst arg) type_arg in
        ( env,
          Some
            ( {
                fp with
                fp_type =
                  { et_type = (why, Tprim Tstring); et_enforced = false };
              }
            :: argl ) )
      | (env, Left pos) ->
        if Partial.should_check_error (Env.get_mode env) 4027 then
          Errors.expected_literal_format_string pos;
        (env, None))
    | (param :: params, _ :: args) ->
      (match f env params args with
      | (env, None) -> (env, None)
      | (env, Some xs) -> (env, Some (param :: xs)))
    | _ -> (env, None)
  in
  match f env ft.ft_params el with
  | (env, None) -> (env, ft)
  | (env, Some xs) ->
    let num_params = List.length xs in
    ( env,
      { ft with ft_params = xs; ft_arity = Fstandard (num_params, num_params) }
    )
