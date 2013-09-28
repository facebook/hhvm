<?php

class C {
}
class D {
  public function __construct($f) {
    $this->map = $f;
  }
}
class E {
  protected $map;
  public function __construct($f) {
    $this->map = $f;
  }
  public function getMap() {
    return $this->map;
  }
}
$f = new stdclass();
$arr = array(new E($f), new D($f));
apc_store('ggg', $arr);
$arr2 = apc_fetch('ggg');
var_dump($arr[0]->getMap());
var_dump($arr[1]->map);
var_dump($arr2[0]->getMap());
var_dump($arr2[1]->map);
