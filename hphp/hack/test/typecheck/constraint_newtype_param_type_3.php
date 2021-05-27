<?hh

newtype A = int;
class C<+T> {}
newtype X<T as C<arraykey>> = T;
type Y<T> = X<C<T>>;

function make(Y<bool> $_): Y<A> {
  return new C();
}
