<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class Enum<T> {
}

class Foo extends Enum<mixed> {
  const int FOO = 0;
  const string BAR = "lol";
}
