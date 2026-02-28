<?hh

interface I<T> {}
abstract class C implements I<this::X> {
  abstract const type X;
}
