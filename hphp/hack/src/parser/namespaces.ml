(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Namespace_env
module SN = Naming_special_names

type elaborate_kind =
  | ElaborateFun
  | ElaborateClass
  | ElaborateRecord
  | ElaborateConst

let elaborate_into_ns ns_name id =
  match ns_name with
  | None -> "\\" ^ id
  | Some ns -> "\\" ^ ns ^ "\\" ^ id

let elaborate_into_current_ns nsenv id = elaborate_into_ns nsenv.ns_name id

(* If the given id is an xhp id, for example :foo:bar
 * we will pull it apart into foo and bar, then reassemble
 * into \foo\bar. This gives us the fully qualified name
 * in a way that the rest of elaborate_id_impl expects.
 *)
let elaborate_xhp_namespace id =
  let is_xhp s = String.(s <> "" && contains s ':') in

  if is_xhp id then
    Str.global_replace (Str.regexp ":") "\\\\" id
  else
    id

(* Resolves an identifier in a given namespace environment. For example, if we
 * are in the namespace "N\O", the identifier "P\Q" is resolved to "\N\O\P\Q".
 *
 * All identifiers are fully-qualified by this function; the internal
 * representation of identifiers inside the typechecker after naming is a fully
 * qualified identifier.
 *
 * It's extremely important that this function is idempotent. We actually
 * normalize identifiers in two phases. Right after parsing, we need to have
 * the class hierarchy normalized so that we can recompute dependencies for
 * incremental mode properly. Other identifiers are normalized during naming.
 * However, we don't do any bookkeeping to determine which we've normalized or
 * not, just relying on the idempotence of this function to make sure everything
 * works out. (Fully qualifying identifiers is of course idempotent, but there
 * used to be other schemes here.)
 *)
let elaborate_raw_id nsenv kind id =
  (* in case we've found an xhp id let's do some preparation to get it into the \namespace\xhp format *)
  let id =
    match kind with
    | ElaborateClass when nsenv.ns_disable_xhp_element_mangling ->
      elaborate_xhp_namespace id
    | _ -> id
  in

  if (not (String.equal id "")) && Char.equal id.[0] '\\' then
    id
  else
    let fqid = Utils.add_ns id in
    match kind with
    | ElaborateConst when SN.PseudoConsts.is_pseudo_const fqid -> fqid
    | ElaborateFun when SN.PseudoFunctions.is_pseudo_function fqid -> fqid
    | ElaborateClass when SN.Typehints.is_reserved_global_name id -> fqid
    | ElaborateClass when SN.Typehints.is_reserved_hh_name id ->
      if nsenv.ns_is_codegen then
        elaborate_into_ns (Some "HH") id
      else
        fqid
    | _ ->
      let (prefix, has_bslash) =
        match String.index id '\\' with
        | Some i -> (String.sub id ~pos:0 ~len:i, true)
        | None -> (id, false)
      in
      if has_bslash && String.equal prefix "namespace" then
        elaborate_into_current_ns nsenv (String_utils.lstrip id "namespace\\")
      else
        let uses =
          match kind with
          | _ when has_bslash -> nsenv.ns_ns_uses
          | ElaborateClass -> nsenv.ns_class_uses
          | ElaborateFun -> nsenv.ns_fun_uses
          | ElaborateConst -> nsenv.ns_const_uses
          | ElaborateRecord -> nsenv.ns_record_def_uses
        in
        (match SMap.find_opt prefix uses with
        | Some use -> Utils.add_ns (use ^ String_utils.lstrip id prefix)
        | None -> elaborate_into_current_ns nsenv id)

let elaborate_id nsenv kind (p, id) = (p, elaborate_raw_id nsenv kind id)
