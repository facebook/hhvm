<?hh

interface IFoo {
  public function hello(): void;
}

class P {
  private function hello(): void { echo "Surprise!\n"; }

  public static function go(IFoo $foo): void {
    $foo->hello();
  }
}

class C extends P implements IFoo {
  public function hello(): void { echo "ok1\n"; }
}

class D implements IFoo {
  public function hello(): void { echo "ok2\n"; }
}

<<__EntryPoint>>
function main(): void {
  $d = new D();
  $c = new C();
  P::go($c);
  P::go($d);
  P::go($c);
}
