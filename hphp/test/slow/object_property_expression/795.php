<?php
class C1 {
  public function __get( $what ) {
    global $b;
    return $b;
  }
}
function assign_ref(&$lv) {
  $lv = 8;
}


<<__EntryPoint>>
function main_795() {
$b = 10;
$c1 = new C1();
assign_ref(&$c1->a);
var_dump($b);
}
