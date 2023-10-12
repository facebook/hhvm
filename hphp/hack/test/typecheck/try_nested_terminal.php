<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

final class A {}
final class B {}
final class C {}
final class D {}
final class E {}
final class F {}
final class G {}
final class H {}

function might_throw(): void {}

function test(): void {
  $x = new A();
  $x = new B();
  try {
    $x = new C();
    $x = new D();
    try {
      $x = new E();
      $x = new F();
      might_throw();
    } catch (Exception $e) {
      hh_show($x); // (D | E | F)
      $x = new G();
      $x = new H();
      return;
    } finally {
      hh_show($x); // (D | E | F | G | H)
    }
    hh_show($x); // F
    return;
  } catch (Exception $e) {
    hh_show($x); // (B | C | D | E | F | G)
  } finally {
    hh_show($x); // (B | C | D | E | F | G | H)
  }
  hh_show($x); // (B | C | D | E | F | G)
}
