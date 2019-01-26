<?php

class MyIterator implements Iterator{
  private $var = array();
  public function __construct($array)  {
    echo "constructing
";
    if (is_array($array)) {
        $this->var = $array;
    }
  }
  public function rewind() {
    echo "rewinding
";
    reset(&$this->var);
  }
  public function current() {
    $var = current(&$this->var);
    echo "current: $var
";
    return $var;
  }
  public function key() {
    $var = key(&$this->var);
    echo "key: $var
";
    return $var;
  }
  public function next() {
    $var = next(&$this->var);
    echo "next: $var
";
    return $var;
  }
  public function valid() {
    $var = $this->current() !== false;
    echo "valid: $var
";
    return $var;
  }
}
function getIter() {
  $values = array(1,2,3);
  $it = new MyIterator($values);
  return $it;
}
class MyIteratorAggregate implements IteratorAggregate {
  public function getIterator() {
    return getIter();
  }
}

<<__EntryPoint>>
function main_439() {
$values = array(1,2,3);
$it = new MyIterator($values);
foreach ($it as $a => $b) {
  print "$a: $b
";
}
foreach (getIter() as $a => $b) {
  print "$a: $b
";
}
$obj = new MyIteratorAggregate();
foreach ($obj as $a => $b) {
  print "$a: $b
";
}
}
