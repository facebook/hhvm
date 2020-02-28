<?hh

function foo() {
  print " FOO ";
  return " foo";
}
class C {
  public function __get( $what ) {
    echo "get C\n";
    return $this->_p[ $what ];
  }
  public function __set( $what, $value ) {
    echo "set C\n";
    $this->_p[ $what ] = $value;
  }
  private $_p = varray[];
}
function bar() {
  print " hello " . foo() . "\n";
  $b = new C;
  $b->a = 'aaaa';
  $b->b = 'bbbb';
  echo " hello $b->a";
  echo " hello $b->b\n";
  print " hello $b->a";
  print " hello $b->b\n";
  echo " hello $b->a $b->b $b->a $b->b";
}

<<__EntryPoint>>
function main_1589() {
bar();
}
