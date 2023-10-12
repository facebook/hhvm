//// newtype.php
<?hh // strict

newtype bar = bool;
class C1 {
  const bar FOO = true;
}

//// def.php
<?hh // strict

newtype foo = bar;
class C2 {
  const foo FOO = C1::FOO;
}

//// use.php
<?hh // strict

// Should fail because C2::FOO is really a bool
type myshape = shape(C2::FOO => int);
