(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Linting_visitors
open Aast

let enum_file_visitor =
  object
    inherit [unit] abstract_file_visitor

    val check_parentage =
      fun (position, extends) ->
        let enum_parents = ["Enum"; "ApiEnum"] in
        let check_parent parent =
          if extends ("\\" ^ parent) then
            Lints_errors.deprecated
              position
              ("Extending "
              ^ parent
              ^ " is deprecated, please use Hack enum. https://docs.hhvm.com/hack/enums/"
              )
        in
        List.iter check_parent enum_parents

    method! on_class () env class_ =
      let (p, c_name) = class_.c_name in
      match Decl_provider.get_class env.ctx c_name with
      | Decl_entry.DoesNotExist
      | Decl_entry.NotYetAvailable ->
        Lint.internal_error p "Could not find class"
      | Decl_entry.Found tclass ->
        check_parentage (p, Decl_provider.Class.has_ancestor tclass)
  end

let go = enum_file_visitor#on_file ()
