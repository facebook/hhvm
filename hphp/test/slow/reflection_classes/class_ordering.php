<?hh


error_reporting(-1);

interface X {
  const X = 'const from X';
  function fromX();
}

interface I {
  const I = 'const from I';
  function fromI();
}

interface J extends I {
  const J = 'const from J';
  function fromJ();
}

abstract class Abs implements J, X {
  const ABS = 'const from Abs';
  abstract function fromAbs();
}

function reflect() {
  $rc_abs = new ReflectionClass('Abs');
  echo 'interfaces:', 'hhvm differs slightly from PHP5 slightly here', "\n";
  print_r($rc_abs->getInterfaceNames());
  echo 'constants:', "\n";
  print_r($rc_abs->getConstants());
  echo 'methods:', "\n";
  foreach ($rc_abs->getMethods() as $meth) {
    echo $meth->getName(), "\n";
  }

}
reflect();
