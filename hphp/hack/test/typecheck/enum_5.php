//// def.php
<?hh // strict

abstract class Enum<T> {
}

newtype foo_ = int;
newtype foo_alias = foo_;
class Foo extends Enum<foo_alias> {
  const foo_alias FOO = 0;
}

//// use.php
<?hh // strict

// Should work because foo is an int
class Bar extends Enum<foo_alias> {
  const foo_alias FOO = Foo::FOO;
}
