<?hh

class C<T as int> {}
interface I<T as int> {}

trait TBad {
  require implements I<string>;
  require extends C<string>;
}
