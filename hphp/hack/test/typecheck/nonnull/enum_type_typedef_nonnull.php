//// file1.php
<?hh // strict

newtype Foo = nonnull;

//// file2.php
<?hh // strict

abstract class Enum {
  abstract const type TInner;
}

class Bar extends Enum {
  const type TInner = Foo;
}
