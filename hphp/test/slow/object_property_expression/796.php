<?php

class C1 {
  public function __get( $what ) {
    echo "get C1
";
    return $this->_p[ $what ];
  }
  public function __set( $what, $value ) {
    echo "set C1
";
    $this->_p[ $what ] = $value;
  }
  private $_p = array();
}
$c1 = new C1();
$c1->a = new C1();
$c1->a->b = new C1();
$c1->a->b->c = 10;
var_dump($c1->a->b->c);
$c1->a->b->c .= 10;
var_dump($c1->a->b->c);
