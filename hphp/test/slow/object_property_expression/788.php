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
class C3 {
  public $p3;
}
$c3 = new C3();
$c3->p3 = new C2();
$c3->p3->p2 = new C1();
$c3->p3->p2->a = 1;
$c3->p3->p2->a .= 1;
print $c3->p3->p2->a;
