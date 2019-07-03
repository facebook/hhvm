<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class A {
 abstract public const int ARGS;
}

class B extends A {
  private const int ARGS = 5;
}
