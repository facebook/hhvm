<?hh

class A {}
class B {}

<<__UNSAFE_AllowMultipleInstantiations>>
interface I<+T> {}

trait T1 implements I<A> {}

trait T2 {
  use T1;
}

class C1 implements I<B> {
  use T1;
}

class C2 extends C1 {
  use T1;
}

function expects<T>(T $_): void {}

function foo(C1 $x, C2 $y): void {
  expects<I<A>>($x);
  expects<I<B>>($x);
  expects<I<A>>($y);
  expects<I<B>>($y);
}
