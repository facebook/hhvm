(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type iterator_kind =
  | Iterator
  | KeyedIterator

type ty_string = Ty of string

val ty_string_of_ty : Tast_env.t -> Typing_defs.locl_ty -> ty_string

type selection_kind =
  | SelectionKindExpression of ty_string
      (**
      Examples of expression selections:

        $x = ( 100  + 200  ) + 300
               ^-^
              ^---^
              ^-----------^
              ^--------------------------^

        *As of this writing*, when the selection corresponds to an expression, we:
        - replace the original expression with a call expression,
        but don't assign to a variable.
        - calculate the return type of the extracted method based on the type of
        the expression we are replacing.
      *)
  | SelectionKindStatement
      (**
      Examples of statement selections:

        if (cond()) {  $x0 = true;   $x1 = true;  $z = 5; }
                       ^---------^
                     ^------------^
                     ^-------------------------^
        ^-------------------------------------------------^

        *As of this writing, when the selection corresponds to a statement, we:
        - Add a method call. If the method is not an iterator then we assign to a variable.
          Example: `$x = $this->placeholder()`. If the method is an iterator, we do something
          more interesting, see test cases.
        - Calculate the return type based on what variables are reassigned in the selection region
        and used after the selection region. For example, if the selection includes `$x =p 3; $y = 5;`
        and both `$x` and `$y` are used later, then the return type is `(int, int)` and the call site
        is something like `list($x, $y) = $this->placeholder();`
      *)

type var_tys = ty_string String.Map.t

type candidate = {
  pos: Pos.t;
  placeholder_name: string;
  selection_kind: selection_kind;
  params: var_tys;
      (** The parameters for the method we extract, along with their types. *)
  return: var_tys;
      (** The returns for the method we extract, along with their types.
  We simulate multiple returns using tuple types and `list()`
  *)
  iterator_kind: iterator_kind option;
      (** Describes the method we are extracting. For example, if the selected code
  contains `yield 3` then the [iterator_kind] will be `Some Iterator` and if the selected
  code does not yield anything then the [iterator_kind] is `None` *)
  is_async: bool;
      (** Describes the method we are extracting. For example, if the selected code contains
  `await` then `is_async` will be `true`. *)
  method_pos: Pos.t;
      (** Describes the method containing the user's selection.
  Used to calculate indentation for the code we generate.
  *)
  method_is_static: bool;
      (** Describes the method containing the user's selection.
  If the method we are extracting *from* is static then the method we extract will be static.  *)
}
