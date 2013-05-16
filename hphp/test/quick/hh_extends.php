<?hh
echo "1..2\n";

interface Bonk<T> {
}
class Foo<X> {
  function beep() { echo "ok 1\n"; }
}
class Bar<T> extends Foo<Bar<Foo<T>>> implements Bonk<T> {
}

class :bork {
attribute
  Bar<String> foo;
}
function nest(Foo<B<C<D<E<F<G<H<I<J<B>,C>>,D>>>,E>>>> $bonk) {
  echo "ok 2\n";
}

Bar::beep();
nest(new Foo());
