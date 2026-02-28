//// newtype.php
<?hh

newtype bar = bool;
const bar truthy_foo = true;

//// def.php
<?hh

abstract class Enum {
  abstract const type TInner;
}

newtype foo = bar;
class C1 {
  const foo FOO = truthy_foo;
}

//// use.php
<?hh

// Should fail because foo isn't an int or a string
class C2 extends Enum {
  const type TInner = foo;
  const foo FOO = C1::FOO;
}
