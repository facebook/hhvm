<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface IRx {

  <<__RxShallow>>
  public function get(): vec<int>;
}

abstract class A<T> {
  <<__RxShallow, __OnlyRxIfImpl(IRx::class)>>
  abstract public function get(): vec<int>;
}

final class C {

  <<__RxShallow, __AtMostRxAsArgs>>
  public function g<T>(<<__OnlyRxIfImpl(IRx::class)>>A<T> $p): bool {
    // OK - method exists both in A and IRx and reactive in IRx
    $r = $p->get();
    return true;
  }
}
