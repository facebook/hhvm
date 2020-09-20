<?hh

namespace A {
  const B = 'c';
  class D {
    public function e($f = \PHP_VERSION, $g = B,
                      $h = varray[\PHP_VERSION], $i = varray[B]) {
    }
  }
  function j($k = \PHP_VERSION, $l = B, $m = varray[\PHP_VERSION], $n = varray[B]) {
  }
}
namespace {
<<__EntryPoint>> function main(): void {
  $tests = varray[
    new ReflectionMethod('A\D', 'e'),
    new ReflectionFunction('A\j'),
  ];
  foreach ($tests as $method) {
    $params = $method->getParameters();
    foreach ($params as $param) {
      \var_dump(
        $param->getDefaultValue(),
        $param->getDefaultValueText(),
        $param->getDefaultValueConstantName()
      );
    }
  }
}
}
