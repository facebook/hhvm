<?php

$b = 10;
class C1 {
  public function __get( $what ) {
    global $b;
    return $b;
  }
}
$c1 = new C1();
function assign_ref(&$lv) {
  $lv = 8;
}
assign_ref($c1->a);
var_dump($b);
