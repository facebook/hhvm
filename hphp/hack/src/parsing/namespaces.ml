(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Ast
open Namespace_env

module SMap = Utils.SMap
module SSet = Utils.SSet

(* HHVM automatically imports a few "core" classes into every namespace, mostly
 * collections. Their unqualified names always refer to this global version.
 *
 * Note that these are technically in the \HH namespace as far as the runtime
 * is concerned, but we treat them as in the global namespace. This is a tiny
 * bit weird, but since Facebook www all runs in the global namespace relying on
 * this autoimport, this makes the most sense there.
 *
 * See hhvm/compiler/parser/parser.cpp Parser::getAutoAliasedClasses for the
 * canonical list. *)
let autoimport_classes = [
  "Traversable";
  "KeyedTraversable";
  "Iterator";
  "KeyedIterator";
  "Iterable";
  "KeyedIterable";
  "Collection";
  "Vector";
  "Set";
  "ImmVector";
  "ImmSet";
  "Pair";
  "Map";
  "StableMap";
  "ImmMap"
]
let autoimport_set =
  List.fold_left (fun s e -> SSet.add e s) SSet.empty autoimport_classes
let is_autoimport_class id = SSet.mem id autoimport_set

(* Resolves an identifier in a given namespace environment. For example, if we
 * are in the namespace "N\O", the identifier "P\Q" is resolved to "\N\O\P\Q".
 *
 * If we are inside a namespace, identifiers are resolved to fully-qualified
 * names. Outside of a namespace, qualified identifiers are similarly resolved
 * to fully-qualified identifiers. However, unqualified identifiers outside of
 * a namespace are *not* fully-qualified -- we omit the leading slash so that
 * code that doesn't make use of namespaces doesn't have spurious leading
 * slashes appended to every name in error messages.
 *
 * It's extremely important that this function is idempotent. We actually
 * normalize identifiers in two phases. Right after parsing, we need to have
 * the class hierarchy normalized so that we can recompute dependencies for
 * incremental mode properly. Other identifiers are normalized during naming.
 * However, we don't do any bookkeeping to determin which we've normalized or
 * not, just relying on the idempotence of this function to make sure everything
 * works out.
 *)
let elaborate_id nsenv (p, id) =
  (* Go ahead and fully-qualify the name first. *)
  let fully_qualified =
    if id.[0] = '\\' then id
    else if is_autoimport_class id then "\\" ^ id
    else begin
      (* Expand "use" imports. *)
      let bslash_loc =
        try String.index id '\\' with Not_found -> String.length id in
      let prefix = String.sub id 0 bslash_loc in
      match SMap.get prefix nsenv.ns_uses with
        | None -> begin match nsenv.ns_name with
          | None -> "\\" ^ id
          | Some ns -> "\\" ^ ns ^ "\\" ^ id
        end
        | Some use -> begin
          let len = (String.length id) - bslash_loc in
          use ^ (String.sub id bslash_loc len)
        end
    end in
  (* If the only location of a '\' in the fully-qualified name is the leading
   * one, strip it off. *)
  let stripped_id = if String.rindex fully_qualified '\\' = 0
    then String.sub fully_qualified 1 ((String.length fully_qualified) - 1)
    else fully_qualified in
  p, stripped_id

(* First pass of flattening namespaces, run super early in the pipeline, right
 * after parsing.
 *
 * Fully-qualifies the things we need for Parsing_service.AddDeps -- the classes
 * we extend, traits we use, interfaces we implement; along with classes we
 * define. So that we can also use them to figure out fallback behavior, we also
 * fully-qualifiy functions that we define, even though AddDeps doesn't need
 * them this early.
 *
 * Note that, since AddDeps doesn't need it, we don't recursively traverse
 * through Happly in hints -- we rely on the idempotence of elaborate_id to
 * allow us to fix those up during a second pass during naming.
 *)
module ElaborateDefs = struct
  let rec hint nsenv = function
    | p, Happly (id, args) ->
        p, Happly (elaborate_id nsenv id, args)
    | other -> other

  let class_def nsenv = function
    | ClassUse h -> ClassUse (hint nsenv h)
    | other -> other

  let rec def nsenv = function
    | Namespace ((_, nsname), prog) -> begin
        let new_nsenv = {nsenv with ns_name = Some nsname} in
        nsenv, program new_nsenv prog
      end
    | NamespaceUse l -> begin
        let map = List.fold_left begin fun map (id1, id2) ->
          SMap.add (snd id2) (snd id1) map
        end nsenv.ns_uses l in
        {nsenv with ns_uses = map}, []
      end
    | Class c -> nsenv, [Class {c with
        c_name = elaborate_id nsenv c.c_name;
        c_extends = List.map (hint nsenv) c.c_extends;
        c_implements = List.map (hint nsenv) c.c_implements;
        c_body = List.map (class_def nsenv) c.c_body;
        c_namespace = nsenv;
      }]
    | Fun f -> nsenv, [Fun {f with
        f_name = elaborate_id nsenv f.f_name;
        f_namespace = nsenv;
      }]
    | Typedef t -> nsenv, [Typedef {t with
        t_id = elaborate_id nsenv t.t_id;
        t_namespace = nsenv;
    }]
    | Constant cst -> nsenv, [Constant {cst with
        cst_name = elaborate_id nsenv cst.cst_name;
        cst_namespace = nsenv;
    }]
    | other -> nsenv, [other]

  and program nsenv p =
    let _, acc = List.fold_left begin fun (nsenv, acc) item ->
      let nsenv, item = def nsenv item in
      nsenv, item :: acc
    end (nsenv, []) p in
    List.concat (List.rev acc)
end

let elaborate_defs ast = ElaborateDefs.program empty ast
