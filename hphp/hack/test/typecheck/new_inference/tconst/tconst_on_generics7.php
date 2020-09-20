<?hh // strict

abstract class Box {
  abstract const type T;
  public abstract function getK() : this::T;
  public abstract function setK(this::T $x) : void;
}

abstract class AbsArraykeyBox extends Box {
  abstract const type T as arraykey;
}

class ArraykeyBox extends Box {
  const type T = arraykey;
  public function getK() : this::T {
    return 5;
  }
  public function setK(this::T $x) : void {}
}

class IntBox extends AbsArraykeyBox {
  const type T = int;
  public function getK() : this::T {
    return 5;
  }
  public function setK(this::T $x) : void {}
}

class StringBox extends AbsArraykeyBox {
  const type T = string;
  public function getK() : this::T {
    return "5";
  }
  public function setK(this::T $x) : void  {
  }
}

class Test<T2, T1 as Box> {
  private ?T1 $v;
  public function __construct() {
    $this->v = null;
  }
  public function put(T1 $x): void {
    $this->v = $x;
  }
  public function get(): T1 {
    if ($this->v == null) { throw new Exception(); }
    return $this->v;
  }
  public function getK(): T2 where T2=T1::T {
    if ($this->v == null) { throw new Exception(); }
    return $this->v->getK();
  }
}

function t(StringBox $s, IntBox $i): void {
  $x = new Test();
  expect_int($x->getK());
  $x->put($s);
  $x->put($i);
}

function t1() : void {
  $x = new Test();
  mess_with_an_ArraykeyBox($x->get(), "oh!");
  expect_int($x->getK()); // nok, this is unsafe!
}

function t2(StringBox $s) : void {
  $x = new Test();
  mess_with_an_ArraykeyBox($x->get(), 9);
  $x->put($s); // error
}

function expect_AbsArraykeyBox(AbsArraykeyBox $_): void {}
function expect_arraykey(arraykey $_): void {}
function expect_int(int $_): void {}

function mess_with_an_ArraykeyBox(ArraykeyBox $b, arraykey $k): void {
  $b->setK($k);
}
/* NB: A similar function

      mess_with_an_AbsArraykeyBox(AbsArraykeyBox $b, arraykey $k)

would not work. It would fail with "arraykey is not a subtype of <expr#1>::T",
because we don't actually know the type or AbsArraykeyBox::T.
*/
