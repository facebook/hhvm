<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

/*

               A                    G (Rx)
               |                      |
        B (CippGlobal)            H (CippRx)
               |
        C (CippLocal)
               |
            D (Cipp)
           /       \
     E (Pure)     F (CippRx)


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

class F extends D {
  <<__CippRx>>
  public function f(): void {}
}

class G {
  <<__Rx>>
  public function f(): void {}
}

class H extends G {
  <<__CippRx>>
  public function f(): void {}
}
