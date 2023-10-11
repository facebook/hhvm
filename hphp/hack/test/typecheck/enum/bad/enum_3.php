//// def.php
<?hh

abstract class Enum {
  abstract const type TInner;
}

class Foo extends Enum {
  const type TInner = lol;
  const FOO = 0;
}
newtype lol = int;

//// use.php
<?hh

// Should fail because FOO is a newtype
function f(): int {
  return Foo::FOO;
}
