<?hh

class blah {

  private static $breakerX = 0;
  private function breaker() :mixed{
    return self::$breakerX++ == 0 ? vec[] : null;
  }

  public function foo() :mixed{
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
