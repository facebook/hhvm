<?php

class MyIterator implements Iterator{
  private $var = array();
  public function __construct($array) {
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
    echo "valid: ",$var?'true':'false',"
";
    return $var;
  }
}

function f() {
 var_dump('f');
 return 0;
 }
function g() {
 var_dump('g');
 return 0;
 }

$a = array(1, 2);
$values = array('a' => 1, 'b' => 2, 'c' => 3);
$it = new MyIterator($values);
foreach ($it as $a[f()] => $a[g()]) {
  print "$a[0]
";
}
