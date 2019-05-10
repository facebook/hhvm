<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

class A {}
class B {}
class C {}
class D {}
class E {}
class F {}
class G {}
class H {}
class I {}
class J {}
class K {}
class L {}
class M {}
class N {}
class O {}
class P {}
class Q {}

function might_throw(): void {}
class Thrower {
  public int $x = 0;
  public static int $y = 0;
  public function might_throw(): void {}
  public static function might_throw_s(): void {}
}

async function test(
  vec<int> $v,
  int $i,
  string $s,
  Awaitable<void> $a
): Awaitable<void> {
  $x = new P();
  try {
    $x = new A();
    might_throw();
    $x = new B();
    // $$a;
    $x = new C();
    $v[] = 0;
    $x = new D();
    $v[12];
    $x = new E();
    12/0;
    $x = new F();
    $i++;
    $x = new G();
    $t = new Thrower();
    $x = new H();
    $t->x;
    $x = new I();
    $t->might_throw();
    $x = new J();
    Thrower::$y;
    $x = new K();
    Thrower::might_throw_s();
    $x = new L();
    $x = new M();
    (string) $i;
    $x = new N();
    $i as string;
    $x = new O();
    await $a;
    $x = new Q();
  } catch (Exception $e) {
    hh_show($x);
  }
}
