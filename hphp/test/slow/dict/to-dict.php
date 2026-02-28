<?hh

function gen() :AsyncGenerator<mixed,mixed,void>{
  yield 'a' => 'b';
  yield '1' => 'one';
  yield 1 => 'ONE';
}


<<__EntryPoint>>
function main_to_dict() :mixed{
$arr = dict['q' => 'r', 1 => 'un', '1' => 'uno'];

var_dump(dict(new ArrayIterator($arr)));
var_dump(dict(gen()));
var_dump(dict(Map {1 => 1, '2' => 2, '3' => 's', 3 => 'i'}));
var_dump(dict(Vector {1, 2, 3, 4, 5, 6}));
var_dump(dict(Set {1, '2', '3', 3}));
var_dump(dict(Pair {1, '1'}));

$x = HH\dict<>;
var_dump($x(Map {1 => 1, '2' => 2, '3' => 's', 3 => 'i'}));
var_dump($x(Vector {1, 2, 3, 4, 5, 6}));
var_dump($x(Set {1, '2', '3', 3}));
var_dump($x(Pair {1, '1'}));
}
