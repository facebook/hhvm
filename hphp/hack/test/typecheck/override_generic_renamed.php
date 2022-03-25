<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  public function foo<T>(T $x):T { return $x; }
}

class D extends C {
  public function foo<TD>(TD $x):TD { return $x; }
}
