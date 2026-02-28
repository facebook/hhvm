<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public this $p;
  public function test($c) :mixed{ $this->p = $c; }
}

class B extends A {}
class C extends B {}
<<__MockClass>> class D extends C {}
<<__EntryPoint>> function main(): void {
(new A())->test(new A());
(new A())->test(new B());
(new A())->test(new C());
(new A())->test(new D());

(new B())->test(new A());
(new B())->test(new B());
(new B())->test(new C());
(new B())->test(new D());

(new C())->test(new A());
(new C())->test(new B());
(new C())->test(new C());
(new C())->test(new D());
}
