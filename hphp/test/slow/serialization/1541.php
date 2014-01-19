<?php

class Small {
  private static $nc = 0;
  public $name;
  public $num;
  function __construct() {
    $n = self::$nc++;
    $this->name = 'foo'.$n;
    $this->num = 3*$n;
  }
}
class Big {
  public $groupAll = array();
  public $group1 = array();
  public $group2 = array();
  public $wacky;
  public $nothing;
  public $unrelated = array();
  function add() {
    $s = new Small();
    $this->groupAll[] = $s;
    if ($s->num % 2 == 0) {
      $this->group1[]=array($s->name, $s);
    }
 else {
      $this->group2[]=array($s->name, $s);
    }
  }
  function finish() {
    $x = 10;
    $this->wacky = array(&$x, &$x);
    $s = new Small();
    $this->unrelated[] = $s;
    $this->unrelated[] = $s;
    $this->unrelated[] = $s;
  }
}
function t() {
  $b = new Big;
  for ($i = 0;
 $i < 10;
 ++$i) {
    $b->add();
  }
  $b->finish();
  var_dump($b);
  $s = serialize($b);
  var_dump($s);
  $us = unserialize($s);
  var_dump($us);
}
t();
