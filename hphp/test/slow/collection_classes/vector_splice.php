<?hh

function test($v, $offset, $len) :mixed{
  echo "$offset $len: ";
  $v->splice($offset, $len);
  echo implode(' ', $v) . "\n";
}

<<__EntryPoint>>
function main() :mixed{
  foreach (range(-4, 4) as $offset) {
    foreach (range(-4, 4) as $len) {
      test(Vector {1, 2, 3}, $offset, $len);
    }
  }
  test(Vector {1, 2, 3}, 2, \PHP_INT_MAX);
}
