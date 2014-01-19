<?php

if (isset($g)) {
 class c{
}
 }
 else {
 class c{
}
 }
class d extends c {
  private $b = 'b';
  function t2() {
    foreach ($this as $k => $v) {
      var_dump($v);
    }
  }
}
$x = new d;
$x->t2();
