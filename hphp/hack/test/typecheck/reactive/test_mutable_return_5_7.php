<?hh
final class TestMutableReturn {

  <<__RxShallow, __MutableReturn>>
  public function testFoo(): ?Temp {
    $x = new Temp();
    return HH\Rx\mutable($x->returnSomething()) ?? HH\Rx\mutable(new Temp());
  }
  <<__RxShallow, __MutableReturn>>
  public function testFoo2(): ?Temp {
    $x = new Temp();
    return $x->returnSomething() ?? new Temp();
  }
}

final class Temp {
  <<__RxShallow, __MutableReturn>>
  public function returnSomething(): ?Temp {
    return null;
  }
}
