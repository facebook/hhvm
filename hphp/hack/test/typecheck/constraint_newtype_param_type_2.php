<?hh

newtype A = int;
newtype X<T as arraykey> = T;
type Y<T> = X<T>;

function make(Y<bool> $_): Y<A> {
  return 0;
}
