<?hh

abstract class Box {
  abstract const type T as arraykey;
  public abstract function getK() : this::T;
  public abstract function setK(this::T $_) : void;
}

class IntBox extends Box {
  const type T = int;
  public function getK() : this::T {
    return 0;
  }
  public function setK(this::T $x) : void {}
}

class StringBox extends Box {
  const type T = string;
  public function getK() : this::T {
    return "5";
  }
  public function setK(this::T $x) : void {}
}

abstract class AbsStrBox extends Box {
  abstract const type T as string;
}

class Test {
  public static function geta<T1 as Box super IntBox, T2>(T1 $x) : T2 where T2 = T1::T {
    return $x->getK();
  }
  public static function getc<T1 as Box, T2>(T1 $x) : T2 where T2 = T1::T {
    return $x->getK();
  }
}

function t(AbsStrBox $s, Box $b) : void {
  $x = new StringBox();
  // x is a StringBox
  // foo is type function(T1 $x) : T1::T
  // foo($x) should therefore be type StringBox::T = string
  $y = Test::geta($x); // nok
  $yy = Test::geta($b); // nok
  $z = Test::getc($s); // ok
  expect_int($yy); // nok
  expect_string($z); // ok
}

function expect_int(int $_): void {}
function expect_string(string $_): void {}
