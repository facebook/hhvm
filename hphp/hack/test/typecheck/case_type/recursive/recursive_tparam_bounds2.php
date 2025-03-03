<?hh

type A<T> = T;

function f<T as A<T>, T2 as A<T2> super A<T2>>(A<T> $x): void {
  hh_expect<T>($x);
  hh_expect<A<T>>($x);
  hh_expect<T2>($x);
  hh_expect<A<T2>>($x);
  hh_expect<string>($x);
}
