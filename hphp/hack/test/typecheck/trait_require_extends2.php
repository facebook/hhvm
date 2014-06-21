<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

trait T1 {
  require extends X;
  <<Override>>
  final protected function foo() { }
}

trait T2 {
  require extends X;
}

class X  {
  protected function foo() { }
}

class A extends X {
  use T1;
  use T2;
  protected function foo() { }
}
