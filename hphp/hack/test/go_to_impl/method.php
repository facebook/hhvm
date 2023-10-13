<?hh
class A {
  public function test(): string {
    return "a";
  }
}

class B extends A {
  function test(): string {
    return "b";
  }
}

class C extends B {
  function test(): string {
    return "c";
  }
}

class D extends A {
  function test(): string {
    return "c";
  }
}

class Test {
  function test(B $b): string {
    return $b->test();
  }
}
