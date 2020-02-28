<?hh

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
  private $_p = varray[];
}

<<__EntryPoint>>
function main_786() {
$c = new C1();
$c->a = 1;
$c->a .= 1;
print $c->a;
}
