<?hh

class C1 {
  public function __get( $what ) {
    echo "get
";
    try { return $this->_p[ $what ]; }
    catch (Exception $e) { echo $e->getMessage()."\n"; return null; }
  }
  public function __set( $what, $value ) {
    echo "set
";
    $this->_p[ $what ] = $value;
  }
  private $_p = varray[];
}

<<__EntryPoint>>
function main_792() {
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
}
