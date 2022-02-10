<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

enum E : string as string {
  FOO = 'foo';
}

abstract class A {
  protected E $x = E::FOO;
}

trait T0 {
  require extends A;
}

class C extends A  {
  use T0;
}
