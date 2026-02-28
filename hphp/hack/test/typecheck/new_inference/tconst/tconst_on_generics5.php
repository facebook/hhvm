<?hh

interface A {}
class B {
  const type t = string;
}
class C extends StringBox {}

abstract class Box {
  abstract const type T;
  public abstract function getK() : this::T;
}

class StringBox extends Box implements A {
  const type T = string;
  public function getK() : this::T {
    return "5";
  }
}

class Test {
  public static function geta<T1 as Box as A, T2>(T1 $x) : T2 where T2 = T1::T {
    return $x->getK();
  }
  public static function getb<T1 as Box super B, T2>(T1 $x) : T2 where T2 = T1::T {
    return $x->getK();
  }
  public static function getc<T1 as Box super C, T2>(T1 $x) : T2 where T2 = T1::T {
    return $x->getK();
  }
}

function t() : string {
  $x = new StringBox();
  // x is a StringBox
  // foo is type function(T1 $x) : T1::T
  // foo($x) should therefore be type StringBox::T = string
  $y = Test::geta($x); // ok
  $t = Test::getb($x); // nok, B must have a T
  $z = Test::getc($x); // ok, C as a T
  return $y . "world"; // v2 <: string -> ok
}
