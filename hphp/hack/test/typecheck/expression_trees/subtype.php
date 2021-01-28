<?hh
// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

<<file: __EnableUnstableFeatures('expression_trees')>>

interface Foo {}

class FooSubtypeA implements Foo {}
class FooSubtypeB implements Foo {}

function lift_int(int $i): ExprTree<Code, Code::TAst, FooSubtypeA> {
  throw new Exception();
}

function lift_string(string $s): ExprTree<Code, Code::TAst, FooSubtypeB> {
  throw new Exception();
}

function uniontest(bool $b): ExprTree<Code, Code::TAst, Foo> {
  if ($b) {
    $arg = lift_string("foo");
  } else {
    $arg = lift_int(10);
  }
  // Without a covariant TInfer parameter of ExprTree, this will throw a type error
  $r = Code`${$arg}`;
  return $r;
}
