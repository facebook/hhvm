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

class Contra<-T> {
  public function __construct(T $_) {}
}

class Test<-T1 as Box, T2> {
  private ?T1 $v;
  public function __construct() {
    $this->v = null;
  }
  public function put(T1 $x): void {
    $this->v = $x;
  }
  public function getK(): Contra<T2> where T2=T1::T {
    if ($this->v == null) { throw new Exception(); }
    return new Contra($this->v->getK());
  }
}

function t(IntBox $i): void {
  $x = new Test(); /* T1 -> #1, T2 -> #2, T1::T -> #3, #1{T->#3}, #3 = #2,
  #1 is contravariant and get solved: #1 = Box */
  $x->put($i); // IntBox <: #1
  expect_contra_int($x->getK()); /* constraint T2=T1::T is being parsed so #2 = Box::T
  -> error, but that's is an arguable error.
  Morally right thing: the `where` constraint should really be at the class level. */
}

function expect_contra_int(Contra<int> $_): void {}
