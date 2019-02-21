<?hh // partial

class MyParent {
  <<__Rx, __Mutable, __ReturnsVoidToRx>>
  public function foo(): this {
    return $this;
  }
}

function f(): void {
  $a = <<__NonRx>>(MyParent $p) ==> {
    // OK: lambda is non-rx in non-rx context
    $p->foo()
      ->foo();
  };
  $a(new MyParent());
}

<<__Rx>>
function g(): void {
  $a = <<__NonRx>>(MyParent $p) ==> {
    // OK: lambda is non-rx but is is not called
    $p->foo()
      ->foo();
  };
}
