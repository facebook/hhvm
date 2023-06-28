<?hh

function gen() :AsyncGenerator<mixed,mixed,void>{
  yield 'a' => 'b';
  yield '1' => 'one';
  yield 1 => 'ONE';
}


<<__EntryPoint>>
function main_to_keyset() :mixed{
$arr = dict['q' => 'r', 1 => 'un', '1' => 'uno'];

var_dump(keyset(new ArrayIterator($arr)));
var_dump(keyset(gen()));
var_dump(keyset(Map {1 => 1, '2' => 2, '3' => 's', 3 => 'i'}));
var_dump(keyset(Vector {1, 2, 3, 4, 5, 6}));
var_dump(keyset(Set {1, '2', '3', 3}));
var_dump(keyset(Pair {1, '1'}));

$x = HH\keyset<>;
var_dump($x(Map {1 => 1, '2' => 2, '3' => 's', 3 => 'i'}));
var_dump($x(Vector {1, 2, 3, 4, 5, 6}));
var_dump($x(Set {1, '2', '3', 3}));
var_dump($x(Pair {1, '1'}));
}
