(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * Hack for HipHop: type checker daemon code.
 *
 * See README for high level overview.
 *
 * Interesting files/directory:
 * - hh_server.ml:       contains mostly the ugly inotify code.
 *
 * Parser code:
 * The parser is a recursive descent full-fidelity parser
 * - parser/
 *
 * Naming:
 * Naming consists in "solving" all the names (making sure every
 * class/variable/etc. are bound).
 * - naming/nast.ml:
 *   Named abstract syntax tree (the datastructure).
 * - naming/naming.ml:
 *   Naming operations (takes an ast and returns a nast).
 * - naming/nastInitCheck.ml:
 *   Checks that all the members in a class are always properly initialized.
 *
 * Typing:
 * - typing/typing_defs.ml:
 *   The datastructures required for typing.
 * - typing/typing_env.ml:
 *   All the operations on the typing environment (e.g. unifying types).
 * - typing/typing_reason.ml:
 *   Documents why something has a given type (witness system).
 * - typing/typing.ml:
 *   Where everything happens, in two phases:
 *   1. type declarations: we build the environment where we know the type of
 *      everything. (see make_env).
 *   2. for every function and method definition, we check that their
 *      implementation matches their signature (assuming that all other
 *      signatures are correct).
 *)

let () = MonitorStart.start ()
