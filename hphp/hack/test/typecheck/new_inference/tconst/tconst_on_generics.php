<?hh

abstract class Box {
  abstract const type T;
  public abstract function getK() : this::T;
  public abstract function setK(this::T $x) : void;
}
class IntBox extends Box {
  const type T = int;
  public function getK() : this::T {
    return 4;
  }
  public function setK(this::T $x) : void {}

}
class StringBox extends Box {
  const type T = string;
public function getK() : this::T {
  return "5";
}

public function setK(this::T $x) : void  {

}

}

interface I {}
class Test {
  public static function get<T1 as Box, T2>(T1 $x) : T2 where T2 = T1::T {
    return $x->getK();
  }

  public static function set<T1 as Box, T2>(T1 $x, T2 $y) : void where T2 = T1::T {
    $x->setK($y);
  }

  public static function swap<T1 as Box, T2 as Box>(T1 $x, T2 $y) : void where T1::T=T2::T {
    $z = $y->getK();
  }

  public static function nonclass_upperbound<T, T1 as Box as num, T2 as Box>(
    T1 $x,
    T2 $y,
  ) : void where T1::T=T2::T {
    $z = $y->getK();
  }
}

function t() : string {
  $x = new StringBox();
  // x is a StringBox
  // foo is type function(T1 $x) : T1::T
  // foo($x) should therefore be type StringBox::T = string
  $y = Test::get($x);
  return $y . "world";
}
