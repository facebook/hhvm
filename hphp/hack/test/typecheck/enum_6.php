//// newtype.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

newtype bar = bool;
const bar truthy_foo = true;

//// def.php
<?hh // strict

abstract class Enum<T> {
}

newtype foo = bar;
class Foo {
  const foo FOO = truthy_foo;
}

//// use.php
<?hh // strict

// Should fail because foo isn't an int or a string
class Bar extends Enum<foo> {
  const foo FOO = Foo::FOO;
}
