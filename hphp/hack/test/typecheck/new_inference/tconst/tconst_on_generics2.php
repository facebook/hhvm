<?hh // strict

abstract class Box {
  abstract const type T;
  public abstract function getK() : this::T;
  public abstract function setK(this::T $x) : void;
}

class IntBox extends Box {
  const type T = int;
  /* HH_FIXME[4326] */ /* HH_FIXME[4336] */
  public function getK() : this::T {
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

class Test {
  public static function get<T1 as Box, T2>(T1 $x) : T2 where T2=T1::T {
    $z = $x->getK();

    return $z;
  }

  public static function set<T1 as Box, T2>(T1 $x, T2 $y) : void where T2=T1::T {
    $x->setK($y);
  }

  public static function swap<T1 as Box, T2 as Box>(T1 $x, T2 $y) : T1 where T1::T=T2::T {
    $x->setK($y->getK());
    return $x;
  }

}

function t(StringBox $s, Box $t) : void {
  $x = new StringBox();
  $y = new IntBox();
  Test::swap($x, $y); // error
}
