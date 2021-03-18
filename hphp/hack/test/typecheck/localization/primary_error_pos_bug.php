<?hh

class A {}

class B<T as num> {}

// errors in declarations

function f1() : A::T {}

function f2() : A<int> {
  return new A();
}

interface I {
  public function f1() : A::T;

  public function f2() : A<int>;

  public function f3<T as num>() : void;

  public function f4<T>() : void where T as num;

  public function f5<T>() : B<T>;
}

type Alias1<T> = B<T>;

type Alias2<T as num> = B<T>;

// no errors in uses
function g(I $x, Alias1<string> $_, Alias2<string> $_) : void {
  f1();
  $x->f1();
  $x->f2();
  $x->f5<string>();
  // except when error is needed
  $x->f3<string>();
  $x->f4<string>();
  new B<string>();
}
