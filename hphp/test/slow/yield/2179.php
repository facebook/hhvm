<?php

trait DY {
  private $drc = array();
  public function dty($key) {
    $this->drc[$key] = true;
    yield (true);
  }
  public function edd($key) {
    if (array_key_exists($key, $this->drc)) {
      var_dump(true);
    }
   }
}
class C {
 use DY;
 }
class D extends C {
 }
$obj = new D;
foreach($obj->dty('foo') as $var) {
  var_dump($var);
}
$obj->edd('foo');
