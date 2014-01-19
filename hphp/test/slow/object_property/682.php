<?php

class c {
  public $d = 'd';
  private $e = 'e';
  protected $f = 'f';
  function t1($y) {
    foreach ($y as $k => $v) {
      var_dump($v);
    }
  }
}
class d extends c {
  public $a = 'a';
  private $b = 'b';
  protected $c = 'c';
  function t2($y) {
    foreach ($this as $k => $v) {
      var_dump($v);
    }
    foreach ($y as $k => $v) {
      var_dump($v);
    }
    foreach ($y as $k => $v) {
      var_dump($v);
    }
  }
}
$x = new d;
$x->surprise = 1;
$y = new d;
$y->shock = 2;
echo "t2
";
$x->t2($y);
echo "t1
";
$x->t1($y);
$z = new c;
echo "t12
";
$z->t1($x);
foreach ($x as $k => $v) {
  var_dump($v);
}
