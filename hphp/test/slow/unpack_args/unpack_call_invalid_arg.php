<?hh

function f($a, $b, ...$c) {
  echo __FUNCTION__, "\n";
  var_dump($a, $b, $c);
}

function test($a, $b, $c) {
  try {
    f($a, $b, ...$c);
  } catch (Exception $e) {
    var_dump($e->getMessage());
    var_dump($e->getLine());
  }
}

<<__EntryPoint>>
function main() {
  test('a', 'b', null);
  test('a', 'b', new stdClass());
  // FIXME(t4599379): This is a Traversable
  test('a', 'b', new ArrayIterator(varray['c', 'd', 'e']));
}
