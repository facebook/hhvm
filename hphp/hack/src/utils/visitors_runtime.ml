(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** We didn't have access to the VisitorsRuntime module from OPAM when we
 * originally added visitors, so we redefined the base classes ourselves here.
 *
 * Now, we have access to the VisitorsRuntime module, but we can't directly use
 * it because we've used the "on_" prefix for all our visit methods instead of
 * "visit_". So instead, this module provides classes containing methods with
 * the "on_" prefix (for all the VisitorsRuntime methods we happen to use)
 * and which inherit from the real VisitorsRuntime classes.
 *
 * It also provides abstract base classes for the [iter], [map], [reduce], and
 * [endo] classes, making it possible to have multiple inheritance of visitor
 * classes without overriding the VisitorsRuntime methods.
 *
 * For example, if we have:
 *
 *     type foo = int [@@deriving visitors { variety = "iter"; name = "iter_foo" }]
 *     type bar = int [@@deriving visitors { variety = "iter"; name = "iter_bar" }]
 *     type baz = foo * bar [@@deriving visitors {
 *       variety = "iter";
 *       ancestors = ["iter_foo"; "iter_bar"];
 *     }]
 *
 * Then the [iter_foo] class and [iter_bar] class will both inherit from
 * [VisitorsRuntime.iter]. This makes the [ancestors] clause for [baz] a
 * problem: [iter_bar]'s inherited implementations of the [VisitorsRuntime.iter]
 * methods will override [iter_foo]'s (even though they are the same
 * implementations). This triggers a compiler warning.
 *
 * We could suppress the warning, or instead, we could have [foo] and [bar]'s
 * visitors inherit from abstract base classes which do not define any concrete
 * implementations:
 *
 *     type foo = int [@@deriving visitors {
 *       variety = "iter";
 *       name = "iter_foo";
 *       nude = true;
 *       visit_prefix = "on_";
 *       ancestors = ["Visitors_runtime.iter_base"];
 *     }]
 *     type bar = int [@@deriving visitors {
 *       variety = "iter";
 *       name = "iter_bar";
 *       nude = true;
 *       visit_prefix = "on_";
 *       ancestors = ["Visitors_runtime.iter_base"];
 *     }]
 *     type baz = foo * bar [@@deriving visitors {
 *       variety = "iter";
 *       nude = true;
 *       visit_prefix = "on_";
 *       ancestors = ["Visitors_runtime.iter"; "iter_foo"; "iter_bar"];
 *     }]
 *
 * Note that [baz] inherits from [Visitors_runtime.iter] rather than
 * [iter_base]. This should be done for the root type we are interested in
 * visiting (i.e., the visitor class which consumers will inherit from). If the
 * type will be included in some other type we are interested in visiting (like
 * [foo] and [bar] are), the abstract base class should be used instead.
 *)

open Core_kernel

class ['s] monoid = ['s] VisitorsRuntime.monoid

class ['s] addition_monoid = ['s] VisitorsRuntime.addition_monoid

class ['s] unit_monoid = ['s] VisitorsRuntime.unit_monoid

class virtual ['a] option_monoid =
  object (self)
    inherit ['a option] monoid

    method virtual private merge : 'a -> 'a -> 'a

    method private zero = None

    method private plus = Option.merge ~f:self#merge
  end

class virtual ['self] map_base =
  object (_ : 'self)
    method virtual private on_string : 'env -> string -> string

    method virtual private on_int : 'env -> int -> int

    method virtual private on_bool : 'env -> bool -> bool

    method virtual private on_list
        : 'env 'a 'b. ('env -> 'a -> 'b) -> 'env -> 'a list -> 'b list

    method virtual private on_option
        : 'env 'a 'b. ('env -> 'a -> 'b) -> 'env -> 'a option -> 'b option
  end

class virtual ['self] map =
  object (self : 'self)
    inherit [_] map_base

    inherit [_] VisitorsRuntime.map

    method private on_string = self#visit_string

    method private on_int = self#visit_int

    method private on_bool = self#visit_bool

    method private on_list = self#visit_list

    method private on_option = self#visit_option
  end

class virtual ['self] iter_base =
  object (_ : 'self)
    method virtual private on_string : 'env -> string -> unit

    method virtual private on_int : 'env -> int -> unit

    method virtual private on_bool : 'env -> bool -> unit

    method virtual private on_list
        : 'env 'a. ('env -> 'a -> unit) -> 'env -> 'a list -> unit

    method virtual private on_option
        : 'env 'a. ('env -> 'a -> unit) -> 'env -> 'a option -> unit
  end

class virtual ['self] iter =
  object (self : 'self)
    inherit [_] iter_base

    inherit [_] VisitorsRuntime.iter

    method private on_string = self#visit_string

    method private on_int = self#visit_int

    method private on_bool = self#visit_bool

    method private on_list = self#visit_list

    method private on_option = self#visit_option
  end

class virtual ['self] endo_base =
  object (_ : 'self)
    method virtual private on_string : 'env -> string -> string

    method virtual private on_int : 'env -> int -> int

    method virtual private on_bool : 'env -> bool -> bool

    method virtual private on_list
        : 'env 'a. ('env -> 'a -> 'a) -> 'env -> 'a list -> 'a list

    method virtual private on_option
        : 'env 'a. ('env -> 'a -> 'a) -> 'env -> 'a option -> 'a option
  end

class virtual ['self] endo =
  object (self : 'self)
    inherit [_] endo_base

    inherit [_] VisitorsRuntime.endo

    method private on_string = self#visit_string

    method private on_int = self#visit_int

    method private on_bool = self#visit_bool

    method private on_list = self#visit_list

    method private on_option = self#visit_option
  end

class virtual ['self] reduce_base =
  object (_ : 'self)
    inherit ['acc] monoid

    method virtual private on_string : 'env -> string -> 'acc

    method virtual private on_int : 'env -> int -> 'acc

    method virtual private on_bool : 'env -> bool -> 'acc

    method virtual private on_list
        : 'env 'a. ('env -> 'a -> 'acc) -> 'env -> 'a list -> 'acc

    method virtual private list_fold_left
        : 'env 'a. ('env -> 'a -> 'acc) -> 'env -> 'acc -> 'a list -> 'acc

    method virtual private on_option
        : 'env 'a. ('env -> 'a -> 'acc) -> 'env -> 'a option -> 'acc
  end

class virtual ['self] reduce =
  object (self : 'self)
    inherit ['self] reduce_base

    inherit [_] VisitorsRuntime.reduce

    method private on_string = self#visit_string

    method private on_int = self#visit_int

    method private on_bool = self#visit_bool

    method private on_list = self#visit_list

    method private on_option = self#visit_option
  end
