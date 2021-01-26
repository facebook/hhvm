<?hh // strict

class Foo {
  private int $bar = 0;

  public static function get()[rx]: Foo {
    return new Foo();
  }

  public function getBar()[rx]: int {
    return $this->bar;
  }

  public function setBar(int $bar)[rx]: void {
    $this->bar = $bar;
  }
}

function main()[rx]: void {
  $mutable = HH\Rx\mutable(Foo::get());
  $mutable->setBar(42);
  HH\Rx\freeze($mutable);
  return $mutable->getBar();
}


<<__EntryPoint>>
function main_freeze_mutable() {
  var_dump(main());
}
