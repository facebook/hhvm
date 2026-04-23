<?hh

class G<T> { }
class H extends G<int> { }

abstract class C {
  <<__Enforceable>>
  abstract const type TC as G<int>;
  public function foo(mixed $m):void {
    $m as this::TC;
  }
}
class D extends C {
  const type TC = H;
}

<<__EntryPoint>>
function main():void {
  $d = new D();
  $d->foo(new H());
}
