<?hh
class MyParent {

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


function g(): void {
  $a = <<__NonRx>>(MyParent $p) ==> {
    // OK: lambda is non-rx but is is not called
    $p->foo()
      ->foo();
  };
}
