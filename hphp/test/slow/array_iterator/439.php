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
    reset($this->var);
  }
  public function current() {
    $var = current($this->var);
    echo "current: $var
";
    return $var;
  }
  public function key() {
    $var = key($this->var);
    echo "key: $var
";
    return $var;
  }
  public function next() {
    $var = next($this->var);
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
$values = array(1,2,3);
$it = new MyIterator($values);
foreach ($it as $a => $b) {
  print "$a: $b
";
}
$itp = "it";
foreach ($$itp as $a => $b) {
  print "$a: $b
";
}
function getIter() {
  $values = array(1,2,3);
  $it = new MyIterator($values);
  return $it;
}
foreach (getIter() as $a => $b) {
  print "$a: $b
";
}
class MyIteratorAggregate implements IteratorAggregate {
  public function getIterator() {
    return getIter();
  }
}
$obj = new MyIteratorAggregate();
foreach ($obj as $a => $b) {
  print "$a: $b
";
}
