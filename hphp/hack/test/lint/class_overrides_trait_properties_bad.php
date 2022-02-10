<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

trait MyTrait2 {
  public int $bar = 1;
}

trait MyTrait {
  use MyTrait2;
  public static int $foo = 1;
}

class MyClass {
  use MyTrait;

  public static int $foo = 1;
  public int $bar = 1;
}
