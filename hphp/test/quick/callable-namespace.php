<?hh

namespace M {
  function call(callable $fn) { $fn(); }
}

namespace N {
  use M;
  <<__DynamicallyCallable>>
  function sayHi() { echo "Hi\n"; }

  class C {
    <<__DynamicallyCallable>>
    public static function sm() { echo "Hello\n"; }
    <<__DynamicallyCallable>>
    public function m() { echo "Salutations\n"; }
  }

  class D extends C { }

<<__EntryPoint>> function main(): void {
  $c = new C;

  M\call('N\sayHi');
  M\call(varray['N\C', 'sm']);
  M\call(varray[$c, 'm']);
}
}
