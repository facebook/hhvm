<?php

class asshat {
  private $prop;
  public function __construct(&$v) {
    $this->prop =& $v;
  }
  public function __get($k) {
    $this->prop = 'foo';
    return array('foo' => 'gotcha', 1 => 'whoops');
  }
}
function main($cond) {
  $x = 1;
  $o = new asshat($x);
  if ($cond) {
    var_dump($o->blah[$x]);
  }
}
main(true);
