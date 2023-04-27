<?hh
// (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

<<file: __EnableUnstableFeatures('expression_trees')>>

interface Foo {}

class FooSubtypeA implements Foo {}
class FooSubtypeB implements Foo {}

function lift_int(int $i): ExprTree<ExampleDsl, ExampleDsl::TAst, FooSubtypeA> {
  throw new Exception();
}

function lift_string(string $s): ExprTree<ExampleDsl, ExampleDsl::TAst, FooSubtypeB> {
  throw new Exception();
}

function uniontest(bool $b): ExprTree<ExampleDsl, ExampleDsl::TAst, Foo> {
  if ($b) {
    $arg = lift_string("foo");
  } else {
    $arg = lift_int(10);
  }
  // Without a covariant TInfer parameter of ExprTree, this will throw a type error
  $r = ExampleDsl`${$arg}`;
  return $r;
}
