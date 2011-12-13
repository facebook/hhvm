<?php

class A {
  public function f() {
    eval('$this->a=20;');
    include 'include3_a.php';
    echo $this->a."\n";
    echo $this->b."\n";
  }
}

$a = new A;
$a->f();
