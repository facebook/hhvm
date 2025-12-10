// RUN: %hackc compile-infer --fail-fast %s | FileCheck %s

// TEST-CHECK-BAL: define $root.closure1
// CHECK: define $root.closure1($this: *void, $x: .notnull *HackString) : *HackMixed {
// CHECK: local $y: *void
// CHECK: #b0:
// CHECK: // .column 8
// CHECK:   n0: *HackMixed = load &$x
// CHECK: // .column 8
// CHECK:   n1 = __sil_allocate(<Closure$closure1>)
// CHECK:   n2 = Closure$closure1.__construct(n1, null, n0)
// CHECK: // .column 3
// CHECK:   store &$y <- n1: *HackMixed
// CHECK: // .column 10
// CHECK:   n3: *HackMixed = load &$y
// CHECK: // .column 3
// CHECK:   ret n3
// CHECK: }

// TEST-CHECK-BAL: define Closure$closure1.__construct
// CHECK: define Closure$closure1.__construct($this: .notnull *Closure$closure1, this: *HackMixed, x: *HackMixed) : *HackMixed {
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &this
// CHECK:   n1: *HackMixed = load &$this
// CHECK:   store n1.?.this <- n0: *HackMixed
// CHECK:   n2: *HackMixed = load &x
// CHECK:   n3: *HackMixed = load &$this
// CHECK:   store n3.?.x <- n2: *HackMixed
// CHECK:   ret null
// CHECK: }

// TEST-CHECK-BAL: define Closure$closure1.__invoke
// CHECK: define Closure$closure1.__invoke($this: .notnull *Closure$closure1) : *HackMixed {
// CHECK: local $x: *void
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$this
// CHECK:   n1: *HackMixed = load n0.?.x
// CHECK:   store &$x <- n1: *HackMixed
// CHECK:   n2: *HackMixed = load &$this
// CHECK:   n3: *HackMixed = load n2.?.this
// CHECK:   store &$this <- n3: *HackMixed
// CHECK: // .column 21
// CHECK:   n4: *HackMixed = load &$x
// CHECK: // .column 15
// CHECK:   n5 = $builtins.hhbc_print(n4)
// CHECK: // .column 15
// CHECK:   ret n5
// CHECK: }

function closure1(string $x): mixed {
  $y = () ==> print($x);
  return $y;
}

class C {

  public static function main(int $x): void {
    $f = (int $y) ==> self::closure($x, $y);
  }

  public static function closure(int $i, int $j): int {
    return $i + $j;
  }
}
// TEST-CHECK-BAL: define Closure$C::main.__invoke
// CHECK: define Closure$C::main.__invoke($this: .notnull *Closure$C::main, $y: .notnull *HackInt) : *HackMixed {
// CHECK: local $x: *void
// CHECK: #b0:
// CHECK:   n0: *HackMixed = load &$this
// CHECK:   n1: *HackMixed = load n0.?.x
// CHECK:   store &$x <- n1: *HackMixed
// CHECK:   n2: *HackMixed = load &$this
// CHECK:   n3: *HackMixed = load n2.?.this
// CHECK:   store &$this <- n3: *HackMixed
// CHECK: // .column 37
// CHECK:   n4: *HackMixed = load &$x
// CHECK: // .column 41
// CHECK:   n5: *HackMixed = load &$y
// CHECK: // .column 23
// CHECK:   n6: *Closure$C::main = load &$this
// CHECK:   n7 = Closure$C::main.closure(n6, n4, n5)
// CHECK: // .column 23
// CHECK:   ret n7
// CHECK: }

// // Closure$C::main.closure ==> this is wrong! It should be C$static::closure
