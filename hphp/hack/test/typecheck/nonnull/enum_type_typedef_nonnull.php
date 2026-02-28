//// file1.php
<?hh

newtype Foo = nonnull;

//// file2.php
<?hh

abstract class Enum {
  abstract const type TInner;
}

class Bar extends Enum {
  const type TInner = Foo;
}
