(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
module Variant : sig
  type t =
    | Num of int
    | Plus of t * t
    | Leq of t * t
    | Cond of t * t * t
  [@@deriving transform]
end = struct
  type t =
    | Num of int
    | Plus of t * t
    | Leq of t * t
    | Cond of t * t * t
  [@@deriving transform]
end

module Gadt : sig
  type _ t =
    | T : bool t
    | F : bool t
    | Num : int -> int t
    | Plus : (int t * int t) -> int t
    | Leq : (int t * int t) -> bool t
    | Cond : (bool t * 'a t * 'a t) -> 'a t
  [@@deriving transform]
end = struct
  type _ t =
    | T : bool t
    | F : bool t
    | Num : int -> int t
    | Plus : (int t * int t) -> int t
    | Leq : (int t * int t) -> bool t
    | Cond : (bool t * 'a t * 'a t) -> 'a t
  [@@deriving transform]
end

module Nonregular : sig
  type 'a term =
    | Var of 'a
    | App of 'a term * 'a term
    | Abs of 'a bind term

  and 'a bind =
    | Zero
    | Succ of 'a
  [@@deriving transform]
end = struct
  (* de Bruijn notation as a nested datatype *)
  type 'a term =
    | Var of 'a
    | App of 'a term * 'a term
    | Abs of 'a bind term

  and 'a bind =
    | Zero
    | Succ of 'a
  [@@deriving transform]
end

module Nonregular_mutual : sig
  type 'a one =
    | Nil
    | Two of 'a two

  and 'a two = MaybeOne of 'a option one [@@deriving transform]
end = struct
  type 'a one =
    | Nil
    | Two of 'a two

  and 'a two = MaybeOne of 'a option one [@@deriving transform]
end

module Nonregular_mutual_gadt : sig
  type 'a one =
    | Nil : 'a one
    | Two : 'a two -> 'a one

  and 'a two = MaybeOne of 'a option one [@@deriving transform]
end = struct
  type 'a one =
    | Nil : 'a one
    | Two : 'a two -> 'a one

  and 'a two = MaybeOne of 'a option one [@@deriving transform]
end

module Composed = struct
  module BinOp : sig
    type t =
      | Add
      | Sub
      | Mul
      | Div
    [@@deriving transform]
  end = struct
    type t =
      | Add
      | Sub
      | Mul
      | Div
    [@@deriving transform]
  end

  module RelOp : sig
    type t =
      | Eq
      | Lt
      | Gt
    [@@deriving transform]
  end = struct
    type t =
      | Eq
      | Lt
      | Gt
    [@@deriving transform]
  end

  module ConnOp : sig
    type t =
      | And
      | Or
    [@@deriving transform]
  end = struct
    type t =
      | And
      | Or
    [@@deriving transform]
  end

  module Expr : sig
    type t =
      | Num of int
      | Bin of BinOp.t * t * t
      | Rel of RelOp.t * t * t
      | Conn of ConnOp.t * t * t
      | Cond of t * t * t
    [@@deriving transform]
  end = struct
    type t =
      | Num of int
      | Bin of BinOp.t * t * t
      | Rel of RelOp.t * t * t
      | Conn of ConnOp.t * t * t
      | Cond of t * t * t
    [@@deriving transform]
  end

  module Expr_gadt : sig
    type _ t =
      | Num : int -> int t
      | Bin : (BinOp.t * int t * int t) -> int t
      | Rel : (RelOp.t * int t * int t) -> bool t
      | Conn : (ConnOp.t * bool t * bool t) -> bool t
      | Cond : (bool t * 'a t * 'a t) -> 'a t
    [@@deriving transform]
  end = struct
    type _ t =
      | Num : int -> int t
      | Bin : (BinOp.t * int t * int t) -> int t
      | Rel : (RelOp.t * int t * int t) -> bool t
      | Conn : (ConnOp.t * bool t * bool t) -> bool t
      | Cond : (bool t * 'a t * 'a t) -> 'a t
    [@@deriving transform]
  end
end

module Builtins : sig
  type 'a t = {
    prim_ignored: char;
    ref: 'a t ref;
    opt: 'a t option;
    res: ('a t, 'a t) result;
    list: 'a t list;
    array: 'a t array;
    lazy_: 'a t lazy_t;
    nested: ('a t option list, 'a t array option) result list option lazy_t;
  }
  [@@deriving transform]
end = struct
  type 'a t = {
    prim_ignored: char;
    ref: 'a t ref;
    opt: 'a t option;
    res: ('a t, 'a t) result;
    list: 'a t list;
    array: 'a t array;
    lazy_: 'a t lazy_t;
    nested: ('a t option list, 'a t array option) result list option lazy_t;
  }
  [@@deriving transform]
end

module Opaque_annotations : sig
  type variant =
    | Opaque_ctor of record [@transform.opaque]
    | Normal of record
    | Tuple_with_opaque of (record * (alias[@transform.opaque]) * opaque_decl)

  and record = {
    normal: alias;
    opaque: alias; [@transform.opaque]
  }

  and alias = record * (variant[@transform.opaque]) * opaque_decl

  and opaque_decl = Opaque of variant * record * alias
  [@@transform.opaque] [@@deriving transform]
end = struct
  type variant =
    | Opaque_ctor of record [@transform.opaque]
    | Normal of record
    | Tuple_with_opaque of (record * (alias[@transform.opaque]) * opaque_decl)

  and record = {
    normal: alias;
    opaque: alias; [@transform.opaque]
  }

  and alias = record * (variant[@transform.opaque]) * opaque_decl

  and opaque_decl = Opaque of variant * record * alias
  [@@transform.opaque] [@@deriving transform]
end

module Explicit_annotations : sig
  type variant =
    | Constant (* Doesn't support explicit [@transform.explicit] *)
    | Inline_record of {
        a: int;
        b: record;
      }
      (* Doesn't support explicit [@transform.explicit] *)
    | Single of record [@transform.explicit]
    | Tuple of alias * record [@transform.explicit]

  and record = { variant: variant [@transform.explicit] }

  and alias =
    (record * variant(* uninterpreted otherwise *) [@transform.explicit])
  [@@deriving transform]
end = struct
  type variant =
    | Constant (* Doesn't support explicit [@transform.explicit] *)
    | Inline_record of {
        a: int;
        b: record;
      }
      (* Doesn't support explicit [@transform.explicit] *)
    | Single of record [@transform.explicit]
    | Tuple of alias * record [@transform.explicit]

  and record = { variant: variant [@transform.explicit] }

  and alias =
    (record * variant(* uninterpreted otherwise *) [@transform.explicit])
  [@@deriving transform]
end

module Explicit_annotations_gadt : sig
  type z = Z : z

  and 'n s = S : 'n -> 'n s

  and ('a, _) gtree =
    | EmptyG : ('a, z) gtree
    | TreeG : ('a, 'n) gtree * 'a * ('a, 'n) gtree -> ('a, 'n s) gtree
        [@transform.explicit]
  [@@deriving transform]
end = struct
  type z = Z : z

  and 'n s = S : 'n -> 'n s

  and ('a, _) gtree =
    | EmptyG : ('a, z) gtree
    | TreeG : ('a, 'n) gtree * 'a * ('a, 'n) gtree -> ('a, 'n s) gtree
        [@transform.explicit]
  [@@deriving transform]
end

module No_restart : sig
  type t =
    | Num of int
    | Plus of t * t
    | Leq of t * t
    | Cond of t * t * t
  [@@deriving transform ~restart:(`Disallow `Encode_as_variant)]
end = struct
  type t =
    | Num of int
    | Plus of t * t
    | Leq of t * t
    | Cond of t * t * t
  [@@deriving transform ~restart:(`Disallow `Encode_as_variant)]
end

module No_restart_as_result : sig
  type t =
    | Num of int
    | Plus of t * t
    | Leq of t * t
    | Cond of t * t * t
  [@@deriving transform ~restart:(`Disallow `Encode_as_result)]
end = struct
  type t =
    | Num of int
    | Plus of t * t
    | Leq of t * t
    | Cond of t * t * t
  [@@deriving transform ~restart:(`Disallow `Encode_as_result)]
end

module Variants = struct
  module Other : sig
    type t = { stuff: int } [@@deriving transform]
  end = struct
    type t = { stuff: int } [@@deriving transform]
  end

  module Exactly : sig
    type t =
      [ `A of Other.t
      | `B of Other.t
      ]
    [@@deriving transform]
  end = struct
    type t =
      [ `A of Other.t
      | `B of Other.t
      ]
    [@@deriving transform]
  end

  module Extend : sig
    type t =
      [ `More of Other.t
      | `Zero
      | Exactly.t
      ]
    [@@deriving transform]
  end = struct
    type t =
      [ `More of Other.t
      | `Zero
      | Exactly.t
      ]
    [@@deriving transform]
  end

  module At_least : sig
    type 'a t = [> `A of Other.t | `B of Other.t ] as 'a [@@deriving transform]
  end = struct
    type 'a t = [> `A of Other.t | `B of Other.t ] as 'a [@@deriving transform]
  end

  module At_most : sig
    type 'a t = [< `A of Other.t | `B of Other.t ] as 'a [@@deriving transform]
  end = struct
    type 'a t = [< `A of Other.t | `B of Other.t ] as 'a [@@deriving transform]
  end

  module As_most_and_at_least : sig
    type 'a t = [< `A of Other.t | `B of Other.t > `A ] as 'a
    [@@deriving transform]
  end = struct
    type 'a t = [< `A of Other.t | `B of Other.t > `A ] as 'a
    [@@deriving transform]
  end
end

module Polymorphic = struct
  module Other : sig
    type 'a t = Self of 'a t option [@@deriving transform]
  end = struct
    type 'a t = Self of 'a t option [@@deriving transform]
  end

  module Quantified : sig
    type t = { forall: 'a. 'a Other.t } [@@deriving transform]
  end = struct
    type t = { forall: 'a. 'a Other.t } [@@deriving transform]
  end
end

module Recursive_mod = struct
  module rec One : sig
    type t = One of Two.t option [@@deriving transform]
  end = struct
    type t = One of Two.t option [@@deriving transform]
  end

  and Two : sig
    type t = Two of One.t option [@@deriving transform]
  end = struct
    type t = Two of One.t option [@@deriving transform]
  end
end

module Indexed_gadt : sig
  type one

  type 'a t = Other : one t -> 'a t [@@deriving transform]
end = struct
  type one

  type 'a t = Other : one t -> 'a t [@@deriving transform]
end

module Mixed_adt_gadt : sig
  type _ t =
    | Adt_data_ctor_nullary
    | Gadt_data_ctor_nullary : 'a t
    | Adt_data_ctor of int
    | Gadt_data_ctor : int -> string t
  [@@deriving transform]
end = struct
  type _ t =
    | Adt_data_ctor_nullary
    | Gadt_data_ctor_nullary : 'a t
    | Adt_data_ctor of int
    | Gadt_data_ctor : int -> string t
  [@@deriving transform]
end

module MyList : sig
  type 'a t [@@deriving transform]
end = struct
  type 'a t =
    | Empty
    | Cons of 'a * 'a t
  [@@deriving transform]
end

module MyAbstract : sig
  type 'a t [@@deriving transform]
end = struct
  type 'a t [@@deriving transform]
end

module SMap = Map.Make (String)

module Hack_builtins : sig
  type t = { map: t SMap.t } [@@deriving transform]
end = struct
  type t = { map: t SMap.t } [@@deriving transform]

  let depth_limit t ~n =
    let on_ty_t t ~ctx =
      if ctx >= n then
        (ctx, `Stop { map = SMap.empty })
      else
        (ctx + 1, `Continue t)
    in
    let bottom_up = Pass.identity ()
    and top_down = Pass.{ on_ty_t = Some on_ty_t } in
    transform t ~ctx:0 ~top_down ~bottom_up
end
