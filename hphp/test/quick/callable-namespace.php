<?hh

namespace M {
  function call(callable $fn) { $fn(); }
}

namespace N {
  use M;

  function sayHi() { echo "Hi\n"; }

  class C {
    public static function sm() { echo "Hello\n"; }
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
