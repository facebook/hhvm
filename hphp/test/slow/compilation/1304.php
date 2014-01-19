<?php

class C {
  function foo($a) {
    var_dump($this + $a);
    var_dump($this - $a);
    var_dump($this * $a);
    var_dump($this / $a);
    var_dump($a + $this);
    var_dump($a - $this);
    var_dump($a * $this);
    var_dump($a / $this);
  }
}
$obj = new C;
$obj->foo(1);
