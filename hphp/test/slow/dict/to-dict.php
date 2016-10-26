<?hh

class IObj implements Iterator {
  public function __construct(private dict $arr) {}
  public function rewind() { reset($this->arr); }
  public function current() { return current($this->arr); }
  public function key() { return key($this->arr); }
  public function next() { return next($this->arr); }
  public function valid() { return key($this->arr); }
}

function gen() {
  yield 'a' => 'b';
  yield '1' => 'one';
  yield 1 => 'ONE';
}

$arr = dict['q' => 'r', 1 => 'un', '1' => 'uno'];

var_dump(dict(new IObj($arr)));
var_dump(dict(gen()));
