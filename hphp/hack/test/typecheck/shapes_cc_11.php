//// use.php
<?hh // strict

newtype blah = shape(
  Foo::BAR => int,
);

//// def.php
<?hh // strict

newtype blah2 = string;
class Foo {
  const blah2 BAR = 'bar';
}
