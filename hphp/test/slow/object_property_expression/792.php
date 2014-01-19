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
$c = new C1();
$c->a += 1;
print $c->a;
$c->a += 10;
print $c->a;
$c->a -= 2;
print $c->a;
$c->a *= 3;
print $c->a;
$c->a /= 2;
print $c->a;
$c->a %= 8;
print $c->a;
$c->a <<= 3;
print $c->a;
$c->a >>= 2;
print $c->a;
$c->a ^= 18;
print $c->a;
$c->a &= 333;
print $c->a;
$c->a |= 7;
print $c->a;
