//// def.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class Enum<T> {
}

newtype foo_ = int;
newtype foo = foo_;
class Foo extends Enum<foo> {
  const foo FOO = 0;
}

//// use.php
<?hh // strict

// Should work because foo is an int
class Bar extends Enum<foo> {
  const foo FOO = Foo::FOO;
}
