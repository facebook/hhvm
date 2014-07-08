//// def.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class Enum<T> {
}

class Foo extends Enum<lol> {
  const FOO = 0;
}
newtype lol = int;

//// use.php
<?hh // strict

// Should fail because FOO is a newtype
function f(): int {
  return Foo::FOO;
}
