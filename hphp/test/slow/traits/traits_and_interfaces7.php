<?hh

error_reporting(-1);

interface I {
  const XYZ = 'const from I';
}

interface J extends I {
  const ABC = 'const from J';
}

trait MyTrait implements J {}

class C {
  use MyTrait;
  function f() {
    echo self::ABC, "\n";
    echo self::XYZ, "\n";
  }
}

function main() {
  $c = new C();
  $c->f();
}

main();
