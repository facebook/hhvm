<?hh

function gen() :AsyncGenerator<mixed,mixed,void>{
  yield 'a' => 'b';
  yield '1' => 'one';
  yield 1 => 'ONE';
}


<<__EntryPoint>>
function main_to_vec() :mixed{
$arr = dict['q' => 'r', 1 => 'un', '1' => 'uno'];

var_dump(vec(new ArrayIterator($arr)));
var_dump(vec(gen()));
var_dump(vec(Map {1 => 1, '2' => 2, '3' => 's', 3 => 'i'}));
var_dump(vec(Vector {1, 2, 3, 4, 5, 6}));
var_dump(vec(Set {1, '2', '3', 3}));
var_dump(vec(Pair {1, '1'}));

$x = HH\vec<>;
var_dump($x(Map {1 => 1, '2' => 2, '3' => 's', 3 => 'i'}));
var_dump($x(Vector {1, 2, 3, 4, 5, 6}));
var_dump($x(Set {1, '2', '3', 3}));
var_dump($x(Pair {1, '1'}));
}
