<?hh

final class C {
  public function f(): void {}
}

function f<<<__Newable>> reify T1 as C>(): void {
  $x = () ==> {
    $c = new T1();
    $c->f();
  };
  $x();
}
