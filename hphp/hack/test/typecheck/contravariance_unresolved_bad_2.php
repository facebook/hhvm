<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface I1<-T> {
  public function f(T $t): int;
}

final class C1 implements I1<string> {
  public function f(string $t): int {
    return 42;
  }
}

final class C2<T> {
  public function __construct(private I1<T> $impl) {}
  public function f(T $t): int {
    return $this->impl->f($t);
  }
}

class Foo {
  public function BreakIt(): void {
    $c1 = new C1();

    // the following line is caught by type checker
    // $c1->f(42);

    $c2 = new C2($c1);
    // What type do we expect for $c2?
    // Perhaps C2<string>?
    // What actually happens is
    // $c2 : C2<v:=unresolved{}>
    // and then check C1 <: I1<v:=unresolved{}>
    // and so I1<string> <: I1<v:=unresolved{}>
    // and so v:=unresolved{} <: string

    // the following line fails in runtime
    $c2->f(42);
  }
}

function main(): void {
  (new Foo())->BreakIt();
}
