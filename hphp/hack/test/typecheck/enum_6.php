//// newtype.php
<?hh // strict

newtype bar = bool;
const bar truthy_foo = true;

//// def.php
<?hh // strict

abstract class Enum<T> {
}

newtype foo = bar;
class C1 {
  const foo FOO = truthy_foo;
}

//// use.php
<?hh // strict

// Should fail because foo isn't an int or a string
class C2 extends Enum<foo> {
  const foo FOO = C1::FOO;
}
