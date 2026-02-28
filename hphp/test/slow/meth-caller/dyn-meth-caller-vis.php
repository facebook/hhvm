<?hh

function LV($x) :mixed{ return __hhvm_intrinsics\launder_value($x); }

class Foo {
  <<__DynamicallyCallable>> public function pub() :mixed{ echo __CLASS__.'::'.__FUNCTION__."\n"; }
  <<__DynamicallyCallable>> public function pub2() :mixed{ echo __CLASS__.'::'.__FUNCTION__."\n"; }
  <<__DynamicallyCallable>> protected function prot() :mixed{ echo __CLASS__.'::'.__FUNCTION__."\n"; }
  <<__DynamicallyCallable>> protected function prot2() :mixed{ echo __CLASS__.'::'.__FUNCTION__."\n"; }
  <<__DynamicallyCallable>> private function priv() :mixed{ echo __CLASS__.'::'.__FUNCTION__."\n"; }
  <<__DynamicallyCallable>> private function priv2() :mixed{ echo __CLASS__.'::'.__FUNCTION__."\n"; }

  static function go_foo() :mixed{
    return vec[
      HH\dynamic_meth_caller(LV(Foo::class), LV('pub')),
      HH\dynamic_meth_caller(LV(Foo::class), LV('pub2')),
      HH\dynamic_meth_caller(LV(Foo::class), LV('prot')),
      HH\dynamic_meth_caller(LV(Foo::class), LV('prot2')),
      HH\dynamic_meth_caller(LV(Foo::class), LV('priv')),
      HH\dynamic_meth_caller(LV(Foo::class), LV('priv2')),
    ];
  }
}

class Bar extends Foo {
  <<__DynamicallyCallable>> public function pub() :mixed{ echo __CLASS__.'::'.__FUNCTION__."\n"; }
  <<__DynamicallyCallable>> protected function prot() :mixed{ echo __CLASS__.'::'.__FUNCTION__."\n"; }
  <<__DynamicallyCallable>> private function priv() :mixed{ echo __CLASS__.'::'.__FUNCTION__."\n"; }


  static function go_bar() :mixed{
    return vec[
      HH\dynamic_meth_caller(LV(Foo::class), LV('pub')),
      HH\dynamic_meth_caller(LV(Foo::class), LV('pub2')),
      HH\dynamic_meth_caller(LV(Foo::class), LV('prot')),
      HH\dynamic_meth_caller(LV(Foo::class), LV('prot2')),
      HH\dynamic_meth_caller(LV(Foo::class), LV('priv')),
      HH\dynamic_meth_caller(LV(Foo::class), LV('priv2')),
    ];
  }
}

<<__EntryPoint>>
function main() :mixed{
  foreach (Bar::go_foo() as $mc) { $mc(new Foo); }
  foreach (Bar::go_bar() as $mc) { $mc(new Bar); }
}
