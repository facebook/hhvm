<?php

class b {
  private $f2 = 10;
  function t2() {
    var_dump($this->f2);
  }
}
class c extends b{
  private $f = 10;
  private static $sf = 10;
  function __get($n) {
    echo 'got';
    return 1;
  }
  function t() {
    var_dump($this->f2);
  }
}
$x = new c;
var_dump($x->f);
$x->t();
$x->t2();
