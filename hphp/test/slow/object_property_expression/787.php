<?php

class C1 {
  public function __get( $what ) {
    echo "get
";
    return $this->_p[ $what ];
  }
  public function __set( $what, $value ) {
    echo "set
";
    $this->_p[ $what ] = $value;
  }
  private $_p = array();
}
class C2 {
  public $p2;
}
$c2 = new C2();
$c2->p = new C1();
$c2->p->a = 1;
$c2->p->a .= 1;
print $c2->p->a;
