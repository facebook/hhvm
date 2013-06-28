<?php

class cls {
  public $pub;
  private $pri;
  public function __construct() {
    $this->pub = 11;
    $this->pri = 12;
  }
  public function meth($x) {
    $a = $this->pub.':'.$this->pri;
    $b = $this->pub.':'.$this->pri;
    $c = $this->pub.':'.$this->pri;
    return $a.'-'.$b.'-'.$c;
  }
}

error_log('eval1.php loaded');
