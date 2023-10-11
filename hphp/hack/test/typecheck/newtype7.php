//// newtype.php
<?hh

newtype Foo as int = int;

//// useit.php
<?hh

function f(Foo $x): ?Foo {
  return $x;
}
