<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

final class A {}
final class B {}
final class C {}
final class D {}
final class E {}
final class F {}

function test(): void {
  $x = new A();
  $x = new B();
  try {
    $x = new C();
    $x = new D();
    try {
      $x = new E();
      $x = new F();
    } catch (Exception $e) {
      hh_show($x); // (D | E | F)
    }
    hh_show($x); // (D | E | F)
  } catch (Exception $e) {
    hh_show($x); // (B | C | D | E | F)
  }
  hh_show($x); // (B | C | D | E | F)
}
