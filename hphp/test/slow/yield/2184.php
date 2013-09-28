<?php

class Foo implements Iterator {
  private $data = array(1, 2, 3);

  public function current() {
    return current($this->data);
  }
  public function key() {
    return key($this->data);
  }
  public function next() {
    next($this->data);
  }
  public function rewind() {
    echo "hagfish\n";
    reset($this->data);
  }
  public function valid() {
    return current($this->data);
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

foreach (run_test() as $_) {
}
