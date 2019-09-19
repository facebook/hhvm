<?hh

class IObj implements Iterator {
  public function __construct(private dict $arr) {}
  public function rewind() { $arr = $this->arr; reset(inout $arr); $this->arr = $arr;  }
  public function current() { $arr = $this->arr; $x = current($arr); $this->arr = $arr; return $x; }
  public function key() { $arr = $this->arr; $key = key($arr); $this->arr = $arr; return $key; }
  public function next() { $arr = $this->arr; $n = next(inout $arr); $this->arr = $arr; return $n; }
  public function valid() { $arr = $this->arr; $key = key($arr); $this->arr = $arr; return $key !== null; }
}

function gen() {
  yield 'a' => 'b';
  yield '1' => 'one';
  yield 1 => 'ONE';
}


<<__EntryPoint>>
function main_to_keyset() {
$arr = dict['q' => 'r', 1 => 'un', '1' => 'uno'];

var_dump(keyset(new IObj($arr)));
var_dump(keyset(gen()));
var_dump(keyset(Map {1 => 1, '2' => 2, '3' => 's', 3 => 'i'}));
var_dump(keyset(Vector {1, 2, 3, 4, 5, 6}));
var_dump(keyset(Set {1, '2', '3', 3}));
var_dump(keyset(Pair {1, '1'}));

$x = 'HH\keyset';
var_dump($x(Map {1 => 1, '2' => 2, '3' => 's', 3 => 'i'}));
var_dump($x(Vector {1, 2, 3, 4, 5, 6}));
var_dump($x(Set {1, '2', '3', 3}));
var_dump($x(Pair {1, '1'}));
}
