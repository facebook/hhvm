<?hh

interface I {
  const XYZ = 'const from I';
}

interface J extends I {
  const ABC = 'const from J';
}

trait MyTrait implements J {
  const ONE = 'const from MyTrait';
}

class C {
  use MyTrait;
  function f() :mixed{
    echo self::ONE, "\n";
    echo self::ABC, "\n";
    echo self::XYZ, "\n";
  }
}

function main() :mixed{
  $c = new C();
  $c->f();
}

function reflect() :mixed{
  echo '==========', ' ', __FUNCTION__, ' ', '==========', "\n";
  $rc = new ReflectionClass("C");
  print_r($rc->getInterfaceNames());
  print_r($rc->getConstants());
  print_r(get_class_constants((string)$rc));
}


<<__EntryPoint>>
function main_traits_and_interfaces7() :mixed{
error_reporting(-1);

main();
reflect();
}
