<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

abstract class Base<T> {
  abstract public function cast<TF>(T $x):TF;
}

// This is bad because we've captured TF
abstract class Bad<TF> extends Base<TF> {
}

class Bad2 extends Bad<mixed> {
  public function cast<TF>(TF $x):TF {
    return $x;
  }
}

function breakit(Base<mixed> $x):int {
  return $x->cast("A");
}
<<__EntryPoint>>
function main():void {
  $b = new Bad2();
  breakit($b);
}
