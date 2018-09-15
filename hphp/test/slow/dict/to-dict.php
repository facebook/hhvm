<?hh

class IObj implements Iterator {
  public function __construct(private dict $arr) {}
  public function rewind() { reset(&$this->arr); }
  public function current() { return current(&$this->arr); }
  public function key() { return key(&$this->arr); }
  public function next() { return next(&$this->arr); }
  public function valid() { return key(&$this->arr); }
}

function gen() {
  yield 'a' => 'b';
  yield '1' => 'one';
  yield 1 => 'ONE';
}


<<__EntryPoint>>
function main_to_dict() {
$arr = dict['q' => 'r', 1 => 'un', '1' => 'uno'];

var_dump(dict(new IObj($arr)));
var_dump(dict(gen()));
var_dump(dict(Map {1 => 1, '2' => 2, '3' => 's', 3 => 'i'}));
var_dump(dict(Vector {1, 2, 3, 4, 5, 6}));
var_dump(dict(Set {1, '2', '3', 3}));
var_dump(dict(Pair {1, '1'}));

$x = 'HH\dict';
var_dump($x(Map {1 => 1, '2' => 2, '3' => 's', 3 => 'i'}));
var_dump($x(Vector {1, 2, 3, 4, 5, 6}));
var_dump($x(Set {1, '2', '3', 3}));
var_dump($x(Pair {1, '1'}));
}
