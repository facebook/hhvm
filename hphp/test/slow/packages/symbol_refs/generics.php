<?hh

module bar;

class Bar1 implements IBar<TFoo1> {} // ok
class Bar2<T as TFoo1> {} // ok
class Bar3<T as Foo> {} // ok

<<__EntryPoint>>
function main_generics() {
  $_ = new BarWithGeneric<TFoo1>();
  $_ = new Bar1();
  $_ = new Bar2<int>();
  $_ = new Bar3<Foo>();
  bar_with_generic<TFoo2>();
  echo "No errors\n";
}
