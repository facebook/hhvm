<?php

class A {
  private $c;
  private $d;
  public function f() {
    eval('$this->a = 20; $this->c = 200;');
    include 'include3.inc';
    echo $this->a."\n";
    echo $this->b."\n";
    echo $this->c."\n";
    echo $this->d."\n";
  }
}

$a = new A;
$a->f();
