<?php

class C1 {
}
class C2 {
  public function __get( $what ) {
    echo "get C2
";
    return $this->_p[ $what ];
  }
  public function __set( $what, $value ) {
    echo "set C2
";
    $this->_p[ $what ] = $value;
  }
  private $_p = array();
}
class C3 {
  public function __get( $what ) {
    echo "get C3
";
    return $this->_p[ $what ];
  }
  public function __set( $what, $value ) {
    echo "set C3
";
    $this->_p[ $what ] = $value;
  }
  private $_p = array();
}
function assign_ref(&$v) {
  $v = 22;
}
$c3 = new C3();
$c3->p3 = new C2();
$c3->p3->p2 = new C1();
$c3->p3->p2->a = 1;
$c3->p3->p2->a .= 1;
print $c3->p3->p2->a;
assign_ref($c3->p3->p2->a);
print $c3->p3->p2->a;
