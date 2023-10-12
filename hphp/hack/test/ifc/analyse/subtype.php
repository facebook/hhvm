<?hh // strict

// elementary subtyping tests

class A { }
class B extends A { }
class C extends B { }

// nominal subtyping
<<__InferFlows>> function B_A( B $x): A { return $x; }
<<__InferFlows>> function C_A( C $x): A { return $x; }

// null & mixed & option
<<__InferFlows>> function nonnull_mixed( nonnull $x): mixed { return $x; }
<<__InferFlows>> function int_opt_int(   int     $x): ?int  { return $x; }
<<__InferFlows>> function null_opt_int(  null    $x): ?int  { return $x; }
<<__InferFlows>> function A_opt_A(       A       $x): ?A    { return $x; }
<<__InferFlows>> function null_opt_A(    null    $x): ?A    { return $x; }
<<__InferFlows>> function A_mixed(       A       $x): mixed { return $x; }
<<__InferFlows>> function null_mixed(    null    $x): mixed { return $x; }

// dict
<<__InferFlows>> function dict_prim_key
  (dict<string,bool> $x): dict<arraykey,bool>          { return $x; }
<<__InferFlows>> function dict_class_key
  (dict<string,C>    $x): dict<arraykey,C>             { return $x; }
<<__InferFlows>> function dict_kt
  (dict<string,C>    $x): KeyedTraversable<arraykey,C> { return $x; }

// vec
<<__InferFlows>> function vec_prim_vec_mixed
  (vec<bool> $x): vec<mixed>              { return $x; }
<<__InferFlows>> function vec_B_vec_A
  (vec<B>    $x): vec<A>                  { return $x; }
<<__InferFlows>> function vec_kt
  (vec<C>    $x): KeyedTraversable<int,C> { return $x; }

// keyset
<<__InferFlows>> function keyset_key
  (keyset<int> $x): keyset<arraykey>          { return $x; }
<<__InferFlows>> function keyset_kt
  (keyset<int> $x): KeyedTraversable<int,int> { return $x; }
