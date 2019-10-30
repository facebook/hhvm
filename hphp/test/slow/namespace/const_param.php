<?hh

namespace A {
  const B = 'c';
  class D {
    public function e($f = \PHP_VERSION, $g = B,
                      $h = array(\PHP_VERSION), $i = array(B)) {
    }
  }
  function j($k = \PHP_VERSION, $l = B, $m = array(\PHP_VERSION), $n = array(B)) {
  }
}
namespace {
<<__EntryPoint>> function main(): void {
  $tests = array(
    new ReflectionMethod('A\D', 'e'),
    new ReflectionFunction('A\j'),
  );
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
