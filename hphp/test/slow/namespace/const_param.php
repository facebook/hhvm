<?hh

namespace A {
  const B = 'c';
  class D {
    public function e($f = \PHP_VERSION, $g = B,
                      $h = varray[\PHP_VERSION], $i = varray[B]) :mixed{
    }
  }
  function j($k = \PHP_VERSION, $l = B, $m = varray[\PHP_VERSION], $n = varray[B]) :mixed{
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
