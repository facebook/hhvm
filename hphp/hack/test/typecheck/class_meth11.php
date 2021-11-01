<?hh

class C {
  public static function aStaticMeth(): string {
    return 'C';
  }

  public function test(): string {
    $h = class_meth(static::class, 'aStaticMeth');
    hh_show($h);
    return $h() . ' ';
  }
}

final class D extends C {
  public function test2(): string {
    $g = class_meth(self::class, 'aStaticMeth');
    hh_show($g);
    return $g() . ' ';
  }

  public static function aStaticMeth(): string {
    return 'D';
  }
}

trait MyTr {
  public static function aStaticMeth(): string {
    return 'MyTr';
  }

  public function test(): string {
    // static::class is the 'use'r class at runtime
    $h = class_meth(static::class, 'aStaticMeth');
    hh_show($h);
    return $h() . ' ';
  }
}

class E {
  use MyTr;
}

<<__EntryPoint>>
function main(): void {
  $c = new C();
  echo 'C: ', $c->test(), "\n";
  $d = new D();
  echo 'D: ', $d->test(), "\n";
  $e = new E();
  echo 'E: ', $e->test(), "\n";
}

// Expected output when executed (without hh_show's)
// C: C C C
// D: C C D
// E: MyTr MyTr MyTr
