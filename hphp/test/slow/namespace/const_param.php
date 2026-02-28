<?hh

namespace A {
  const B = 'c';
  class D {
    public function e($f = \PHP_VERSION, $g = B,
                      $h = vec[\PHP_VERSION], $i = vec[B]) :mixed{
    }
  }
  function j($k = \PHP_VERSION, $l = B, $m = vec[\PHP_VERSION], $n = vec[B]) :mixed{
  }
}
namespace {
<<__EntryPoint>> function main(): void {
  $tests = vec[
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
