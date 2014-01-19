<?php

$a = array();
for ($i = 0; $i < 20; $i++) { $a[$i] = $i; }
$s = "hello";
$ch = curl_init();

class C {
  public function __toDebugDisplay() {
    return "some custom string";
  }
}
$c = new C();

class D {
  public function __toDebugDisplay() {
    return array('c'=>new C());
  }
}
$d = new D();
