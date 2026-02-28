<?hh

interface Bonk<T> {
}
class Foo<X> {
  static function beep() :mixed{ echo "ok 1\n"; }
}
class Bar<T> extends Foo<Bar<Foo<T>>> implements Bonk<T> {
}

class :bork {
attribute
  Bar<string> foo;
}
function nest(Foo<B<C<D<E<F<G<H<I<J<B>,C>>,D>>>,E>>>> $bonk) :mixed{
  echo "ok 2\n";
}
<<__EntryPoint>>
function main_entry(): void {
  echo "1..2\n";

  Bar::beep();
  nest(new Foo());
}
