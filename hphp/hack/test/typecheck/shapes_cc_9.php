//// newtype.php
<?hh // strict

newtype bar = int;
class C1 {
  const bar FOO = 0;
}


//// def.php
<?hh // strict

newtype foo = bar;
class C2 {
  const foo FOO = C1::FOO;
}

//// use.php
<?hh // strict

// Should be fine.
type myshape = shape(C2::FOO => int);
