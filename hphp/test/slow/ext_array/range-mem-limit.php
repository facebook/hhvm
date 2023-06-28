<?hh

<<__EntryPoint>>
function main() :mixed{
  ini_set('memory_limit', '20M');
  // Prevent folding, so that we don't set surprise flag at jit time.
  $inf = __hhvm_intrinsics\launder_value(-INF);
  range(-PHP_INT_MAX - 1, $inf, PHP_INT_MAX - 1);
}
