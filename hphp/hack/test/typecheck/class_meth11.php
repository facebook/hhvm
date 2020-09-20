<?hh // partial

class C {
  public static function aStaticMeth(): string {
    return 'C';
  }

  public function test() {
    $h = class_meth(static::class, 'aStaticMeth');
    hh_show($h);
    echo $h(), ' ';
  }
}

final class D extends C {
  public function test2() {
    $g = class_meth(self::class, 'aStaticMeth');
    hh_show($g);
    echo $g(), ' ';
  }

  public static function aStaticMeth(): string {
    return 'D';
  }
}

trait MyTr {
  public static function aStaticMeth(): string {
    return 'MyTr';
  }

  public function test() {
    // static::class is the 'use'r class at runtime
    $h = class_meth(static::class, 'aStaticMeth');
    hh_show($h);
    echo $h(), ' ';
  }
}

class E {
  use MyTr;
}

function main() {
  $c = new C();
  echo 'C: ', $c->test(), "\n";
  $d = new D();
  echo 'D: ', $d->test(), "\n";
  $e = new E();
  echo 'E: ', $e->test(), "\n";
}
main();

// Expected output when executed (without hh_show's)
// C: C C C
// D: C C D
// E: MyTr MyTr MyTr
