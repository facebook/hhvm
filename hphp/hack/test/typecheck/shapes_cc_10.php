//// def.php
<?hh // strict

newtype FooType = int;
class X {
  const FooType X1 = 0;
  const FooType X2 = 1;
}

//// use.php
<?hh // strict

// Reject: type of keys is opaque
type myshape = shape(X::X1 => int, X::X2 => bool);
