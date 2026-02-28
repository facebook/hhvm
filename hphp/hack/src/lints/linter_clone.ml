(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Linting_visitors

module VisitorFunctor (Parent : BodyVisitorModule) : BodyVisitorModule = struct
  class visitor env =
    object
      inherit Parent.visitor env as parent

      method! on_clone () (ty, p, e) =
        Lints_diagnostics.clone_use p;
        parent#on_clone () (ty, p, e)
    end
end

let go = lint_all_bodies (module VisitorFunctor)
