<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

abstract class A {
  abstract const type TNode;
}

final class R<+T> {

  public function get<TVar as A, TNode>(
    TVar $var,
  ): TNode where TNode = TVar::TNode {
    throw new Exception("A");
  }
}

abstract class C {
    abstract const type TNode;

    final public function foo(): R<this::TNode> {
      throw new Exception("A");
    }
    public function bar(vec<(string,mixed)> $m): void {
      $z = $this->foo();
      foreach ($m as list($name, $h)) {
           $y =  $h as A;
           // Problem is that we try to project off (#23 & A)
           $x = $z->get($y);
      }
    }
}
