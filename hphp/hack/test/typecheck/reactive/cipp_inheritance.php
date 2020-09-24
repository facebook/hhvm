<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

/*

               A
               |
        B (CippGlobal)
       /            \
C (CippLocal)      F (Rx)
      |
  D (Cipp)
      |
  E (Pure)


*/



class A {
  public function f(): void {}
}

class B extends A {
  <<__CippGlobal>>
  public function f(): void {}
}

class C extends B {
  <<__CippLocal>>
  public function f(): void {}
}

class D extends C {
  <<__Cipp>>
  public function f(): void {}
}

class E extends D {
  <<__Pure>>
  public function f(): void {}
}

class F extends B {
  <<__Rx>>
  public function f(): void {}
}
