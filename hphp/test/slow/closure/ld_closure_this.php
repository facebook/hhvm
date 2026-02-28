<?hh

class A {
  public function __construct() {
  }

  public static int $d = 5;

  public function bar((function(): int) $lambda): void {
    var_dump(static::$d);
    for ($i = 0; $i < static::$d; $i++) {
      $lambda();
    }
  }
}

class B {
  public function __construct() {}

  private int $c;

  public function blah(): void {
    $this->c = 2;
    $a = new A();
    $a->bar(
      () ==> {
        var_dump($this->c);
        $this->c++;
        return $this->c;
      },
    );
  }
}

<<__EntryPoint>>
function main(): void {
  $b = new B();
  $b->blah();
}
