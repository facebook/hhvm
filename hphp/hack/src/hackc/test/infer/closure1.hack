// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

abstract class Base {

  public abstract static function helper(mixed ...$x): mixed;

// TEST-CHECK-BAL: "define Base.checkInstance0("
// CHECK: define Base.checkInstance0($this: *Base, $arg1: *HackInt, $zarg1: *HackInt) : *void {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$zarg1
// CHECK:   n1: *HackMixed = load &$arg1
// CHECK:   n2 = __sil_allocate(<Closure$Base::checkInstance0>)
// CHECK:   n3: *Base = load &$this
// CHECK:   n4 = Closure$Base::checkInstance0.__construct(n2, n3, n0, n1)
// CHECK:   n5 = $root.helper(null, n2)
// CHECK:   ret null
// CHECK: }

  public function checkInstance0(int $arg1, int $zarg1): void {
    helper(
      () ==> {
        return helper($this, $arg1, $zarg1);
      }
    );
  }

// TEST-CHECK-BAL: "define Base.checkInstance1("
// CHECK: define Base.checkInstance1($this: *Base, $arg1: *HackInt, $zarg1: *HackInt) : *void {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$zarg1
// CHECK:   n1: *HackMixed = load &$arg1
// CHECK:   n2 = __sil_allocate(<Closure$Base::checkInstance1>)
// CHECK:   n3: *Base = load &$this
// CHECK:   n4 = Closure$Base::checkInstance1.__construct(n2, n3, n0, n1)
// CHECK:   n5 = $root.helper(null, n2)
// CHECK:   ret null
// CHECK: }

  public function checkInstance1(int $arg1, int $zarg1): void {
    helper(
      () ==> {
        return helper($arg1, $zarg1);
      }
    );
  }

// TEST-CHECK-BAL: "define Base$static.checkStatic0("
// CHECK: define Base$static.checkStatic0($this: *Base$static, $arg1: *HackInt, $zarg1: *HackInt) : *void {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$zarg1
// CHECK:   n1: *HackMixed = load &$arg1
// CHECK:   n2 = __sil_allocate(<Closure$Base::checkStatic0>)
// CHECK:   n3: *Base$static = load &$this
// CHECK:   n4 = Closure$Base::checkStatic0.__construct(n2, n3, n0, n1)
// CHECK:   n5 = $root.helper(null, n2)
// CHECK:   ret null
// CHECK: }

  public static function checkStatic0(int $arg1, int $zarg1): void {
    helper(
      () ==> {
        return static::helper($arg1, $zarg1);
      }
    );
  }

// TEST-CHECK-BAL: "define Base$static.checkStatic1("
// CHECK: define Base$static.checkStatic1($this: *Base$static, $arg1: *HackInt, $zarg1: *HackInt) : *void {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$zarg1
// CHECK:   n1: *HackMixed = load &$arg1
// CHECK:   n2 = __sil_allocate(<Closure$Base::checkStatic1>)
// CHECK:   n3: *Base$static = load &$this
// CHECK:   n4 = Closure$Base::checkStatic1.__construct(n2, n3, n0, n1)
// CHECK:   n5 = $root.helper(null, n2)
// CHECK:   ret null
// CHECK: }

  public static function checkStatic1(int $arg1, int $zarg1): void {
    helper(
      () ==> {
        return helper($arg1, $zarg1);
      }
    );
  }

// TEST-CHECK-BAL: "define Base.checkNested("
// CHECK: define Base.checkNested($this: *Base, $arg1: *HackInt, $zarg1: *HackInt) : *void {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$zarg1
// CHECK:   n1: *HackMixed = load &$arg1
// CHECK:   n2 = __sil_allocate(<Closure$Base::checkNested232>)
// CHECK:   n3: *Base = load &$this
// CHECK:   n4 = Closure$Base::checkNested232.__construct(n2, n3, n0, n1)
// CHECK:   n5 = $root.helper(null, n2)
// CHECK:   ret null
// CHECK: }

  public function checkNested(int $arg1, int $zarg1): void {
    helper(
      () ==> {
        return helper(
          () ==> {
            return helper($this, $arg1, $zarg1);
          }
        );
      }
    );
  }

// TEST-CHECK-BAL: "type Closure$Base::checkInstance0 "
// CHECK: type Closure$Base::checkInstance0 extends Closure = .kind="class" {
// CHECK:   this: .public *HackMixed;
// CHECK:   zarg1: .private *HackMixed;
// CHECK:   arg1: .private *HackMixed
// CHECK: }

// TEST-CHECK-BAL: "define Closure$Base::checkInstance0.__construct"
// CHECK: define Closure$Base::checkInstance0.__construct($this: *Closure$Base::checkInstance0, this: *HackMixed, zarg1: *HackMixed, arg1: *HackMixed) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &this
// CHECK:   n1: *HackMixed = load &$this
// CHECK:   store n1.?.this <- n0: *HackMixed
// CHECK:   n2: *HackMixed = load &zarg1
// CHECK:   n3: *HackMixed = load &$this
// CHECK:   store n3.?.zarg1 <- n2: *HackMixed
// CHECK:   n4: *HackMixed = load &arg1
// CHECK:   n5: *HackMixed = load &$this
// CHECK:   store n5.?.arg1 <- n4: *HackMixed
// CHECK:   ret null
// CHECK: }

// TEST-CHECK-BAL: "define Closure$Base::checkInstance0.__invoke"
// CHECK: define Closure$Base::checkInstance0.__invoke($this: *Closure$Base::checkInstance0) : *HackMixed {
// CHECK: local $arg1: *void, $zarg1: *void
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$this
// CHECK:   n1: *HackMixed = load n0.?.arg1
// CHECK:   store &$arg1 <- n1: *HackMixed
// CHECK:   n2: *HackMixed = load &$this
// CHECK:   n3: *HackMixed = load n2.?.zarg1
// CHECK:   store &$zarg1 <- n3: *HackMixed
// CHECK:   n4: *HackMixed = load &$this
// CHECK:   n5: *HackMixed = load n4.?.this
// CHECK:   store &$this <- n5: *HackMixed
// CHECK:   n6: *HackMixed = load &$this
// CHECK:   n7: *HackMixed = load &$arg1
// CHECK:   n8: *HackMixed = load &$zarg1
// CHECK:   n9 = $root.helper(null, n6, n7, n8)
// CHECK:   ret n9
// CHECK: }

// TEST-CHECK-BAL: "type Closure$Base::checkInstance1 "
// CHECK: type Closure$Base::checkInstance1 extends Closure = .kind="class" {
// CHECK:   this: .public *HackMixed;
// CHECK:   zarg1: .private *HackMixed;
// CHECK:   arg1: .private *HackMixed
// CHECK: }

// TEST-CHECK-BAL: "define Closure$Base::checkInstance1.__construct"
// CHECK: define Closure$Base::checkInstance1.__construct($this: *Closure$Base::checkInstance1, this: *HackMixed, zarg1: *HackMixed, arg1: *HackMixed) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &this
// CHECK:   n1: *HackMixed = load &$this
// CHECK:   store n1.?.this <- n0: *HackMixed
// CHECK:   n2: *HackMixed = load &zarg1
// CHECK:   n3: *HackMixed = load &$this
// CHECK:   store n3.?.zarg1 <- n2: *HackMixed
// CHECK:   n4: *HackMixed = load &arg1
// CHECK:   n5: *HackMixed = load &$this
// CHECK:   store n5.?.arg1 <- n4: *HackMixed
// CHECK:   ret null
// CHECK: }

// TEST-CHECK-BAL: "define Closure$Base::checkInstance1.__invoke"
// CHECK: define Closure$Base::checkInstance1.__invoke($this: *Closure$Base::checkInstance1) : *HackMixed {
// CHECK: local $arg1: *void, $zarg1: *void
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$this
// CHECK:   n1: *HackMixed = load n0.?.arg1
// CHECK:   store &$arg1 <- n1: *HackMixed
// CHECK:   n2: *HackMixed = load &$this
// CHECK:   n3: *HackMixed = load n2.?.zarg1
// CHECK:   store &$zarg1 <- n3: *HackMixed
// CHECK:   n4: *HackMixed = load &$this
// CHECK:   n5: *HackMixed = load n4.?.this
// CHECK:   store &$this <- n5: *HackMixed
// CHECK:   n6: *HackMixed = load &$arg1
// CHECK:   n7: *HackMixed = load &$zarg1
// CHECK:   n8 = $root.helper(null, n6, n7)
// CHECK:   ret n8
// CHECK: }

// TEST-CHECK-BAL: "type Closure$Base::checkStatic0 "
// CHECK: type Closure$Base::checkStatic0 extends Closure = .kind="class" {
// CHECK:   this: .public *HackMixed;
// CHECK:   zarg1: .private *HackMixed;
// CHECK:   arg1: .private *HackMixed
// CHECK: }

// TEST-CHECK-BAL: "define Closure$Base::checkStatic0.__construct"
// CHECK: define Closure$Base::checkStatic0.__construct($this: *Closure$Base::checkStatic0, this: *HackMixed, zarg1: *HackMixed, arg1: *HackMixed) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &this
// CHECK:   n1: *HackMixed = load &$this
// CHECK:   store n1.?.this <- n0: *HackMixed
// CHECK:   n2: *HackMixed = load &zarg1
// CHECK:   n3: *HackMixed = load &$this
// CHECK:   store n3.?.zarg1 <- n2: *HackMixed
// CHECK:   n4: *HackMixed = load &arg1
// CHECK:   n5: *HackMixed = load &$this
// CHECK:   store n5.?.arg1 <- n4: *HackMixed
// CHECK:   ret null
// CHECK: }

// TEST-CHECK-BAL: "define Closure$Base::checkStatic0.__invoke"
// CHECK: define Closure$Base::checkStatic0.__invoke($this: *Closure$Base::checkStatic0) : *HackMixed {
// CHECK: local $arg1: *void, $zarg1: *void
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$this
// CHECK:   n1: *HackMixed = load n0.?.arg1
// CHECK:   store &$arg1 <- n1: *HackMixed
// CHECK:   n2: *HackMixed = load &$this
// CHECK:   n3: *HackMixed = load n2.?.zarg1
// CHECK:   store &$zarg1 <- n3: *HackMixed
// CHECK:   n4: *HackMixed = load &$this
// CHECK:   n5: *HackMixed = load n4.?.this
// CHECK:   store &$this <- n5: *HackMixed
// CHECK:   n6: *HackMixed = load &$arg1
// CHECK:   n7: *HackMixed = load &$zarg1
// CHECK:   n8: *Closure$Base::checkStatic0 = load &$this
// CHECK:   n9 = n8.?.helper(n6, n7)
// CHECK:   ret n9
// CHECK: }

// TEST-CHECK-BAL: "type Closure$Base::checkStatic1 "
// CHECK: type Closure$Base::checkStatic1 extends Closure = .kind="class" {
// CHECK:   this: .public *HackMixed;
// CHECK:   zarg1: .private *HackMixed;
// CHECK:   arg1: .private *HackMixed
// CHECK: }

// TEST-CHECK-BAL: "define Closure$Base::checkStatic1.__construct"
// CHECK: define Closure$Base::checkStatic1.__construct($this: *Closure$Base::checkStatic1, this: *HackMixed, zarg1: *HackMixed, arg1: *HackMixed) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &this
// CHECK:   n1: *HackMixed = load &$this
// CHECK:   store n1.?.this <- n0: *HackMixed
// CHECK:   n2: *HackMixed = load &zarg1
// CHECK:   n3: *HackMixed = load &$this
// CHECK:   store n3.?.zarg1 <- n2: *HackMixed
// CHECK:   n4: *HackMixed = load &arg1
// CHECK:   n5: *HackMixed = load &$this
// CHECK:   store n5.?.arg1 <- n4: *HackMixed
// CHECK:   ret null
// CHECK: }

// TEST-CHECK-BAL: "define Closure$Base::checkStatic1.__invoke"
// CHECK: define Closure$Base::checkStatic1.__invoke($this: *Closure$Base::checkStatic1) : *HackMixed {
// CHECK: local $arg1: *void, $zarg1: *void
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$this
// CHECK:   n1: *HackMixed = load n0.?.arg1
// CHECK:   store &$arg1 <- n1: *HackMixed
// CHECK:   n2: *HackMixed = load &$this
// CHECK:   n3: *HackMixed = load n2.?.zarg1
// CHECK:   store &$zarg1 <- n3: *HackMixed
// CHECK:   n4: *HackMixed = load &$this
// CHECK:   n5: *HackMixed = load n4.?.this
// CHECK:   store &$this <- n5: *HackMixed
// CHECK:   n6: *HackMixed = load &$arg1
// CHECK:   n7: *HackMixed = load &$zarg1
// CHECK:   n8 = $root.helper(null, n6, n7)
// CHECK:   ret n8
// CHECK: }

  // TEST-CHECK-BAL: "type Closure$Base::checkNested232 "
  // CHECK: type Closure$Base::checkNested232 extends Closure = .kind="class" {
  // CHECK:   this: .public *HackMixed;
  // CHECK:   zarg1: .private *HackMixed;
  // CHECK:   arg1: .private *HackMixed
  // CHECK: }

// TEST-CHECK-BAL: "define Closure$Base::checkNested232.__construct"
// CHECK: define Closure$Base::checkNested232.__construct($this: *Closure$Base::checkNested232, this: *HackMixed, zarg1: *HackMixed, arg1: *HackMixed) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &this
// CHECK:   n1: *HackMixed = load &$this
// CHECK:   store n1.?.this <- n0: *HackMixed
// CHECK:   n2: *HackMixed = load &zarg1
// CHECK:   n3: *HackMixed = load &$this
// CHECK:   store n3.?.zarg1 <- n2: *HackMixed
// CHECK:   n4: *HackMixed = load &arg1
// CHECK:   n5: *HackMixed = load &$this
// CHECK:   store n5.?.arg1 <- n4: *HackMixed
// CHECK:   ret null
// CHECK: }

// TEST-CHECK-BAL: "define Closure$Base::checkNested232.__invoke"
// CHECK: define Closure$Base::checkNested232.__invoke($this: *Closure$Base::checkNested232) : *HackMixed {
// CHECK: local $arg1: *void, $zarg1: *void
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$this
// CHECK:   n1: *HackMixed = load n0.?.arg1
// CHECK:   store &$arg1 <- n1: *HackMixed
// CHECK:   n2: *HackMixed = load &$this
// CHECK:   n3: *HackMixed = load n2.?.zarg1
// CHECK:   store &$zarg1 <- n3: *HackMixed
// CHECK:   n4: *HackMixed = load &$this
// CHECK:   n5: *HackMixed = load n4.?.this
// CHECK:   store &$this <- n5: *HackMixed
// CHECK:   n6: *HackMixed = load &$zarg1
// CHECK:   n7: *HackMixed = load &$arg1
// CHECK:   n8 = __sil_allocate(<Closure$Base::checkNested>)
// CHECK:   n9: *Closure$Base::checkNested232 = load &$this
// CHECK:   n10 = Closure$Base::checkNested.__construct(n8, n9, n6, n7)
// CHECK:   n11 = $root.helper(null, n8)
// CHECK:   ret n11
// CHECK: }

// TEST-CHECK-BAL: "type Closure$Base::checkNested "
// CHECK: type Closure$Base::checkNested extends Closure = .kind="class" {
// CHECK:   this: .public *HackMixed;
// CHECK:   zarg1: .private *HackMixed;
// CHECK:   arg1: .private *HackMixed
// CHECK: }

// TEST-CHECK-BAL: "define Closure$Base::checkNested.__construct"
// CHECK: define Closure$Base::checkNested.__construct($this: *Closure$Base::checkNested, this: *HackMixed, zarg1: *HackMixed, arg1: *HackMixed) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &this
// CHECK:   n1: *HackMixed = load &$this
// CHECK:   store n1.?.this <- n0: *HackMixed
// CHECK:   n2: *HackMixed = load &zarg1
// CHECK:   n3: *HackMixed = load &$this
// CHECK:   store n3.?.zarg1 <- n2: *HackMixed
// CHECK:   n4: *HackMixed = load &arg1
// CHECK:   n5: *HackMixed = load &$this
// CHECK:   store n5.?.arg1 <- n4: *HackMixed
// CHECK:   ret null
// CHECK: }

// TEST-CHECK-BAL: "define Closure$Base::checkNested.__invoke"
// CHECK: define Closure$Base::checkNested.__invoke($this: *Closure$Base::checkNested) : *HackMixed {
// CHECK: local $arg1: *void, $zarg1: *void
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$this
// CHECK:   n1: *HackMixed = load n0.?.arg1
// CHECK:   store &$arg1 <- n1: *HackMixed
// CHECK:   n2: *HackMixed = load &$this
// CHECK:   n3: *HackMixed = load n2.?.zarg1
// CHECK:   store &$zarg1 <- n3: *HackMixed
// CHECK:   n4: *HackMixed = load &$this
// CHECK:   n5: *HackMixed = load n4.?.this
// CHECK:   store &$this <- n5: *HackMixed
// CHECK:   n6: *HackMixed = load &$this
// CHECK:   n7: *HackMixed = load &$arg1
// CHECK:   n8: *HackMixed = load &$zarg1
// CHECK:   n9 = $root.helper(null, n6, n7, n8)
// CHECK:   ret n9
// CHECK: }
}

// TEST-CHECK-BAL: "define $root.checkFunc("
// CHECK: define $root.checkFunc($this: *void, $arg1: *HackInt, $zarg1: *HackInt) : *void {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$zarg1
// CHECK:   n1: *HackMixed = load &$arg1
// CHECK:   n2 = __sil_allocate(<Closure$checkFunc>)
// CHECK:   n3 = Closure$checkFunc.__construct(n2, null, n0, n1)
// CHECK:   n4 = $root.helper(null, n2)
// CHECK:   ret null
// CHECK: }

// TEST-CHECK-BAL: "type Closure$checkFunc "
// CHECK: type Closure$checkFunc extends Closure = .kind="class" {
// CHECK:   this: .public *HackMixed;
// CHECK:   zarg1: .private *HackMixed;
// CHECK:   arg1: .private *HackMixed
// CHECK: }

// TEST-CHECK-BAL: "define Closure$checkFunc.__construct"
// CHECK: define Closure$checkFunc.__construct($this: *Closure$checkFunc, this: *HackMixed, zarg1: *HackMixed, arg1: *HackMixed) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &this
// CHECK:   n1: *HackMixed = load &$this
// CHECK:   store n1.?.this <- n0: *HackMixed
// CHECK:   n2: *HackMixed = load &zarg1
// CHECK:   n3: *HackMixed = load &$this
// CHECK:   store n3.?.zarg1 <- n2: *HackMixed
// CHECK:   n4: *HackMixed = load &arg1
// CHECK:   n5: *HackMixed = load &$this
// CHECK:   store n5.?.arg1 <- n4: *HackMixed
// CHECK:   ret null
// CHECK: }

// TEST-CHECK-BAL: "define Closure$checkFunc.__invoke"
// CHECK: define Closure$checkFunc.__invoke($this: *Closure$checkFunc) : *HackMixed {
// CHECK: local $arg1: *void, $zarg1: *void
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$this
// CHECK:   n1: *HackMixed = load n0.?.arg1
// CHECK:   store &$arg1 <- n1: *HackMixed
// CHECK:   n2: *HackMixed = load &$this
// CHECK:   n3: *HackMixed = load n2.?.zarg1
// CHECK:   store &$zarg1 <- n3: *HackMixed
// CHECK:   n4: *HackMixed = load &$this
// CHECK:   n5: *HackMixed = load n4.?.this
// CHECK:   store &$this <- n5: *HackMixed
// CHECK:   n6: *HackMixed = load &$arg1
// CHECK:   n7: *HackMixed = load &$zarg1
// CHECK:   n8 = $root.helper(null, n6, n7)
// CHECK:   ret n8
// CHECK: }

function checkFunc(int $arg1, int $zarg1): void {
  helper(
    () ==> {
      return helper($arg1, $zarg1);
    }
  );
}
