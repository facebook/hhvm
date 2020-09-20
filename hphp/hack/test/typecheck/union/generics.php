//// file1.php
<?hh // strict

newtype E<T> = mixed;

newtype F<+T> = mixed;



//// file2.php
<?hh // strict

abstract class AA<T> {
  abstract const type Tabs;
  abstract const type Tcstr as int;
}
class A<T> extends AA<T> {
  const type Ta = ?int;
  const type Tabs = string;
  const type Tcstr = int;

  public function isAClass(): bool {
    return true;
  }

  public function isSomethingWithT(T $x): bool {
    return true;
  }

  public function f(bool $cond, A::Ta $a, AA::Tabs $abs, AA::Tcstr $cstr): void {
    $x = ($cond ? 0 : $a);
    hh_show($x);

    $x = ($cond ? $abs : $a);
    hh_show($x);

    $x = ($cond ? $cstr : 0);
    hh_show($x);

    $x = ($cond ? $cstr : $a);
    hh_show($x);
  }
}

class B<T> {
  public function __construct(T $x) {}

  public function isAClass(): bool {
    return true;
  }
}

class C {}

class D<+T> {}

function f<T, Tu super C, Tv as C, Tw super C, Ty as C>(
  bool $b,
  A<int> $aint,
  A<int> $aint2,
  A<T> $at,
  A<T> $at2,
  D<int> $dint,
  D<string> $dstring,
  E<int> $eint,
  E<int> $eint2,
  E<string> $estring,
  F<int> $fint,
  F<string> $fstring,
  classname<C> $cl,
  Tu $u,
  Tv $v,
  Tw $w,
  Ty $y,
  C $c,
): void {
  $x = ($b ? new A<int>() : new A<int>());
  hh_show($x);

  $x = ($b ? new A() : new A());
  hh_show($x);

  $x = ($b ? new A() : new A<int>());
  hh_show($x);

  $x = ($b ? new B<int>(0) : new B<int>(0));
  hh_show($x);

  $x = ($b ? new B(0) : new B(0));
  hh_show($x);

  $x = ($b ? $aint : new A<int>());
  hh_show($x);

  $x = ($b ? $aint : new A());
  hh_show($x);

  $x = ($b ? $at : new A());
  hh_show($x);

  $x = ($b ? $at : new A<int>());
  hh_show($x);

  $x = ($b ? $at : $aint);
  hh_show($x);

  $x = ($b ? $at : $at2);
  hh_show($x);

  $x = ($b ? new A<int>() : new A<string>());
  hh_show($x);
  hh_show($x->isSomethingWithT("")); // error

  $x = ($b ? new A<num>() : new A<arraykey>());
  hh_show($x);
  hh_show($x->isSomethingWithT(0));

  $x = ($b ? new A<int>() : new B(0));
  hh_show($x);
  hh_show($x->isAClass());

  $x = ($b ? $dint : $dstring);
  hh_show($x);

  $x = ($b ? new D<int> () : new D<string>());
  hh_show($x);

  $x = ($b ? $dint : new D<string>());
  hh_show($x);

  $x = ($b ? $eint : $eint2);
  hh_show($x);

  $x = ($b ? $eint : $estring);
  hh_show($x);

  $x = ($b ? $fint : $fstring);
  hh_show($x);

  $x = ($b ? $cl : C::class);
  hh_show($x);

  $x = ($b ? $u : $c);
  hh_show($x);

  $x = ($b ? $v : $c);
  hh_show($x);

  $x = ($b ? $u : $v);
  hh_show($x);

  $x = ($b ? $u : $w);
  hh_show($x);

  $x = ($b ? $v : $y);
  hh_show($x);
}
