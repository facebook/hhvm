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

function reflect() {
  echo '==========', ' ', __FUNCTION__, ' ', '==========', "\n";
  $rc = new ReflectionClass("C");
  print_r($rc->getInterfaceNames());
  print_r($rc->getConstants());
  print_r(get_class_constants($rc));
}

main();
reflect();
