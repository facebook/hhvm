<?hh

class blah {

  private static $breakerX = 0;
  private function breaker() {
    return self::$breakerX++ == 0 ? varray[] : null;
  }

  public function foo() {
    $x = 0;
    $y = 0;

    if ($this->breaker() === NULL) {
      echo "hi\n";
    }
    echo "ok\n";
  }
}

<<__EntryPoint>> function main(): void {
  $x = new blah();
  $x->foo();
  $x->foo();
}
