<?hh
class Foo implements IMemoizeParam {
  public function getInstanceKey()[] : string {
    return "";
  }
}

class Bar implements UNSAFESingletonMemoizeParam {
  public function getInstanceKey()[] : string {
    return "";
  }
}

<<__Memoize>>
function foo(vec<int> $x)[] : void {}


<<__Memoize>>
function not_ok(vec<Foo> $x)[] : void {}

<<__Memoize>>
function ok_singleton(Bar $x)[] : void {}

<<__Memoize>>
function ok_vec_singleton(vec<Bar> $x)[] : void {}

<<__Memoize>>
function not_ok_vector_singleton(Vector<Bar> $x)[] : void {}

<<__Memoize>>
function not_ok_mixed(mixed $x)[] : void {}

<<__Memoize>>
function not_ok_vector(Vector<int> $x)[] : void {}

<<__Memoize>>
function not_ok2(vec<Foo> $x)[write_props] : void {}

<<__Memoize>>
function ok(vec<Foo> $x) : void {}

<<__Memoize>>
function ok2(vec<Foo> $x)[globals] : void {}

<<__Memoize>>
function not_ok_policied(vec<Foo> $x)[zoned] : void {}

<<__Memoize(#KeyedByIC)>>
function ok_policied(vec<Foo> $x)[zoned] : void {}
