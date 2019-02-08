<?hh // strict

/* This test is essentially the same as tconst_on_generics, except we have
`T2 super T1::T` */

abstract class Box {
  abstract const type T;
  public abstract function getK() : this::T;
  public abstract function setK(this::T $x) : void;
}

class StringBox extends Box {
  const type T = string;
  public function getK() : this::T {
    return "5";
  }
  public function setK(this::T $x) : void  {}
}

class Test {
  public static function get<T1 as Box, T2>(T1 $x) : T2 where T2 super T1::T {
    return $x->getK();
  }
}

function t() : int {
  $x = new StringBox();
  // x is a StringBox
  // foo is type function(T1 $x) : T1::T
  // foo($x) should therefore be type StringBox::T = string
  $y = Test::get($x);
  return $y; // error
}
