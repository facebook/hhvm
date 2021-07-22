<?hh

function handler($name, $obj, inout $args) {
  var_dump($args);
  return shape();
}

<<__NEVER_INLINE>>
function f(int $x, int $y = 20, int $z = 30) {}

<<__NEVER_INLINE>>
function g(int $x, ...$y) {
  var_dump($x, $y);
}

<<__NEVER_INLINE>>
function h(int $x, int $y = 20, ...$z) {
  var_dump($x, $y, $z);
}

<<__EntryPoint>>
function main() {
  fb_intercept2('f', 'handler');
  fb_intercept2('g', 'handler');
  fb_intercept2('h', 'handler');
  echo "Regular with default args\n";
  f(1);
  f(1, 2);
  f(1, 2, 3);

  echo "\nVariadic\n";
  g(1);
  g(1, 2);
  g(1, 2, 3);

  echo "\nVariadic and default args\n";
  h(1);
  h(1, 2);
  h(1, 2, 3);
}
