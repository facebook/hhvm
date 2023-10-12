//// def.php
<?hh // strict

abstract class Enum {
  abstract const type TInner;
}

newtype foo_ = int;
newtype foo_alias = foo_;
class Foo extends Enum {
  const type TInner = foo_alias;
  const foo_alias FOO = 0;
}

//// use.php
<?hh // strict

// Should work because foo is an int
class Bar extends Enum {
  const type TInner = foo_alias;
  const foo_alias FOO = Foo::FOO;
}
