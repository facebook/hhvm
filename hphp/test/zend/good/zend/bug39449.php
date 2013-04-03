<?php
class A {
  private $keys = array();
  public function & __get($val) {
    return $this->keys[$val];
  }
  public function __set($k, $v) {
    $this->keys[$k] = $v;
  }
}

$a =new A();
$a->arr = array('a','b','c');

$b = &$a->arr;
$b[]= 'd';

foreach ($a->arr as $k => $v) {
  echo "$k => $v\n";
}

$a->arr[]='d';

foreach ($a->arr as $k => $v) {
  echo "$k => $v\n";
}
?>