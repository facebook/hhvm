<?hh

namespace M {
  function call(callable $fn) :mixed{ $fn(); }
}

namespace N {
  use M;
  <<__DynamicallyCallable>>
  function sayHi() :mixed{ echo "Hi\n"; }

  class C {
    <<__DynamicallyCallable>>
    public static function sm() :mixed{ echo "Hello\n"; }
    <<__DynamicallyCallable>>
    public function m() :mixed{ echo "Salutations\n"; }
  }

  class D extends C { }

<<__EntryPoint>> function main(): void {
  $c = new C;

  M\call('N\sayHi');
  M\call(vec['N\C', 'sm']);
  M\call(vec[$c, 'm']);
}
}
