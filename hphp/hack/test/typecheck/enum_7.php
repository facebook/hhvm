<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class Enum<T> {
}

// Should fail because the bool const is bogus
class Foo extends Enum<mixed> {
  const int FOO = 0;
  const string BAR = "lol";
  const bool BAZ = true;
}
