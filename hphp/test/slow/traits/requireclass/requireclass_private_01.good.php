<?hh

final class MyFoo {
  use MyTrait;

  private function bong(): void { echo "bong\n"; }

  private static function staticBong(): void { echo "staticBong\n"; }
}

trait MyTrait {
  require class MyFoo;
  public function bang(): void {
    $this->bong();      // OK
    self::staticBong(); // OK
  }
}

<<__EntryPoint>>
function main() {
  (new MyFoo())->bang();
}
