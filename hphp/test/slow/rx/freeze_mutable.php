<?hh // strict

class Foo {
  private int $bar = 0;

  <<__Rx, __MutableReturn>>
  public static function get(): Foo {
    return new Foo();
  }

  <<__Rx>>
  public function getBar(): int {
    return $this->bar;
  }

  <<__Rx, __Mutable>>
  public function setBar(int $bar): void {
    $this->bar = $bar;
  }
}

<<__Rx>>
function main(): void {
  $mutable = HH\Rx\mutable(Foo::get());
  $mutable->setBar(42);
  HH\Rx\freeze($mutable);
  return $mutable->getBar();
}


<<__EntryPoint>>
function main_freeze_mutable() {
  var_dump(main());
}
