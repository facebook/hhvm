<?hh

class Foo implements Iterator {
  private $data = varray[1, 2, 3];

  public function current() { $arr = $this->data; $x = current($arr); $this->data = $arr; return $x; }
  public function key() { $arr = $this->data; $key = key($arr); $this->data = $arr; return $key; }
  public function valid() { $arr = $this->data; $current = current($arr); $this->data = $arr; return $current; }







  public function next() {
    $__data = $this->data;
    next(inout $__data);
    $this->data = $__data;
  }
  public function rewind() {
    echo "hagfish\n";
    $__data = $this->data;
    reset(inout $__data);
    $this->data = $__data;
  }



}

function run_test() {
  $f = new Foo();

  foreach ($f as $value) {
    echo $value . "\n";
  }

  yield 1230;

  foreach($f as $value) {
    echo $value . "\n";
  }
}


<<__EntryPoint>>
function main_2184() {
foreach (run_test() as $_) {
}
}
