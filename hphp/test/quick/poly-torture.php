<?hh

trait T1 {
  function dubiousArgs($a, $b, $c, $d, $e, $f, $g, $h, $i) {
    echo "T1::dubiousArgs $h, $i\n";
  }
}

trait T2 {
  function dubiousArgs($a) {
    echo "T2::dubiousArgs $a\n";
  }
}

class A {
  function __call($nm, $arr) {
    echo "A::__call $nm\n";
    if ($nm == "dubiousArgs") {
      echo "cannot happen\n";
      exit(1);
    }
  }
  use T2;
}

class B {
  function __call($nm, $arr) {
    echo "B::__call $nm\n";
  }
  function noSuchMethodBoyeee() {
    echo "B has this method.\n";
  }
  use T1;
}

class C {
  function __call($nm, $arr) {
    echo "C::__call $nm\n";
  }
  function noSuchMethodBoyeee($a, $b, $c, $d, $e) {
    echo "C also has this method with params: ", $d, " ", $e, "\n";
  }
}

abstract final class PrandobjStatics {
  public static $state = 0;
  public static $names = varray['A', 'B', 'C'];
}

function pRandObj() {
  $name = PrandobjStatics::$names[(PrandobjStatics::$state++ * 17) % 3];
  echo "    randObj: $name\n";
  return new $name();
}

abstract final class RandarrStatics {
  public static $state = 0;
}

function randArr() {
  return range(0, (RandarrStatics::$state++ * 17) % 128);
}

class MagicBox {
  public $inner;
  public function __construct($inner) {
    $this->inner = $inner;
  }
  function __call($nm, $arr) {
    echo "--";
    call_user_func_array(varray[$this->inner, $nm], $arr);
  }
}

<<__EntryPoint>> function main(): void {
  for ($i = 0; $i < 5; $i++) {
    $m = new MagicBox(pRandObj());
    if ($i % 3 == 0) {
      $m = new MagicBox(new MagicBox(new MagicBox($m)));
    }
    $s = 'noSuchMethodBoyeee';
    try {
      pRandObj()->dubiousArgs($i, $i,$i, $i,$i, $i,$i, $i,$i, $i,$i, $i,$i,
      $i,$i, $i,$i, $i,$i, $i,$i, $i,$i, $i,$i, $i,$i, $i,$i, $i);
      $m->$s();
      call_user_func_array(varray[$m, 'noSuchMethodBoyeee'], randArr());
    } catch (Exception $e) { echo get_class($e), ": ", $e->getMessage(), "\n"; }
  }
}
