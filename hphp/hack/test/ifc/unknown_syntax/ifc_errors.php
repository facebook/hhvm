<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class A {
  <<__Policied("PUBLIC")>>
  public int $pub = 0;

  <<__Policied("PRIVATE")>>
  public int $priv = 1;
}

<<__InferFlows>>
function f(A $a): void {
  $x = $a->priv;

  // We still get a flow error here
  $a->pub = $x;
}
