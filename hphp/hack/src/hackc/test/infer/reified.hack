// RUN: %hackc compile-infer --enable-var-cache --fail-fast %s | FileCheck %s
// TODO(aorenste) the update.py script failed to update this file so I opted
// it out by adding the --enable-var-cache flag. We should remove it at some
// point

// Naming of following functions:
//   nr = non-reified
//   rr = reified
//   nv = non-variadic
//   vv = variadic
//   nd = non-defaults
//   dd = defaults

// TEST-CHECK-1: define $root.nr_nv_nd
// CHECK: define $root.nr_nv_nd($this: *void, $a: *HackInt) : *void {
function nr_nv_nd(int $a): void {
  echo "--- ", __FUNCTION__, "\n";
  var_dump($a);
}

// TEST-CHECK-1: define $root.rr_nv_nd
// CHECK: define $root.rr_nv_nd($this: *void, $a: *HackInt, $0ReifiedGenerics: *HackVec) : *void {
function rr_nv_nd<reify T>(int $a): void {
  echo "--- ", __FUNCTION__, "\n";
  var_dump($a);
}

// TEST-CHECK-1: define $root.nr_vv_nd
// CHECK: define $root.nr_vv_nd($this: *void, $a: *HackInt, $b: .variadic .typevar="array" *HackMixed) : *void {
function nr_vv_nd(int $a, int... $b): void {
  echo "--- ", __FUNCTION__, "\n";
  var_dump($a, $b);
}

// TEST-CHECK-1: define $root.rr_vv_nd
// CHECK: define $root.rr_vv_nd($this: *void, $a: *HackInt, $b: .variadic .typevar="array" *HackMixed, $0ReifiedGenerics: *HackVec) : *void {
function rr_vv_nd<reify T>(int $a, int... $b): void {
  echo "--- ", __FUNCTION__, "\n";
  var_dump($a, $b);
}

// TEST-CHECK-BAL: define $root.nr_nv_dd
// CHECK: define $root.nr_nv_dd($this: *void, $a: *HackInt) : *void {
// CHECK: local $b: *void
// CHECK: #b0:
// CHECK:   store &$b <- $builtins.hack_int(5): *HackMixed
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK:   n0: *HackMixed = load &$a
// CHECK:   n1: *HackMixed = load &$b
// CHECK:   n2 = $root.nr_nv_dd(null, n0, n1)
// CHECK:   ret n2
// CHECK: }

// TEST-CHECK-1: define $root.nr_nv_dd
// CHECK: define $root.nr_nv_dd($this: *void, $a: *HackInt, $b: *HackInt) : *void {
function nr_nv_dd(int $a, int $b = 5): void {
  echo "--- ", __FUNCTION__, "\n";
  var_dump($a, $b);
}

// TEST-CHECK-BAL: define $root.rr_nv_dd
// CHECK: define $root.rr_nv_dd($this: *void, $a: *HackInt, $0ReifiedGenerics: *HackVec) : *void {
// CHECK: local $b: *void
// CHECK: #b0:
// CHECK:   store &$b <- $builtins.hack_int(5): *HackMixed
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK:   n0: *HackMixed = load &$a
// CHECK:   n1: *HackMixed = load &$b
// CHECK:   n2: *HackMixed = load &$0ReifiedGenerics
// CHECK:   n3 = $root.rr_nv_dd(null, n0, n1, n2)
// CHECK:   ret n3
// CHECK: }

// TEST-CHECK-1: define $root.rr_nv_dd
// CHECK: define $root.rr_nv_dd($this: *void, $a: *HackInt, $b: *HackInt, $0ReifiedGenerics: *HackVec) : *void {
function rr_nv_dd<reify T>(int $a, int $b = 5): void {
  echo "--- ", __FUNCTION__, "\n";
  var_dump($a, $b);
}

// TEST-CHECK-BAL: define $root.nr_vv_dd
// CHECK: define $root.nr_vv_dd($this: *void, $a: *HackInt) : *void {
// CHECK: local $b: *void, $c: *void
// CHECK: #b0:
// CHECK:   n0 = $builtins.hhbc_new_vec()
// CHECK:   store &$c <- n0: *HackMixed
// CHECK:   store &$b <- $builtins.hack_int(5): *HackMixed
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK:   n1: *HackMixed = load &$a
// CHECK:   n2: *HackMixed = load &$b
// CHECK:   n3: *HackMixed = load &$c
// CHECK:   n4 = $root.nr_vv_dd(null, n1, n2, n3)
// CHECK:   ret n4
// CHECK: }

// TEST-CHECK-1: define $root.nr_vv_dd
// CHECK: define $root.nr_vv_dd($this: *void, $a: *HackInt, $b: *HackInt, $c: .variadic .typevar="array" *HackMixed) : *void {
function nr_vv_dd(int $a, int $b = 5, int... $c): void {
  echo "--- ", __FUNCTION__, "\n";
  var_dump($a, $b, $c);
}

// TEST-CHECK-BAL: define $root.rr_vv_dd
// CHECK: define $root.rr_vv_dd($this: *void, $a: *HackInt, $0ReifiedGenerics: *HackVec) : *void {
// CHECK: local $b: *void, $c: *void
// CHECK: #b0:
// CHECK:   n0 = $builtins.hhbc_new_vec()
// CHECK:   store &$c <- n0: *HackMixed
// CHECK:   store &$b <- $builtins.hack_int(5): *HackMixed
// CHECK:   jmp b1
// CHECK: #b1:
// CHECK:   n1: *HackMixed = load &$a
// CHECK:   n2: *HackMixed = load &$b
// CHECK:   n3: *HackMixed = load &$c
// CHECK:   n4: *HackMixed = load &$0ReifiedGenerics
// CHECK:   n5 = $root.rr_vv_dd(null, n1, n2, n3, n4)
// CHECK:   ret n5
// CHECK: }

// TEST-CHECK-1: define $root.rr_vv_dd
// CHECK: define $root.rr_vv_dd($this: *void, $a: *HackInt, $b: *HackInt, $c: .variadic .typevar="array" *HackMixed, $0ReifiedGenerics: *HackVec) : *void {
function rr_vv_dd<reify T>(int $a, int $b = 5, int... $c): void {
  echo "--- ", __FUNCTION__, "\n";
  var_dump($a, $b, $c);
}

<<__EntryPoint>>
function test(): void {
  $a = vec[];

  // TEST-CHECK-1*: = $root.nr_nv_nd
  // CHECK:   n2 = $root.nr_nv_nd(null, $builtins.hack_int(5))
  nr_nv_nd(5);
  // TEST-CHECK-1*: n3 =
  // CHECK:   n3 = $builtins.__sil_splat(n0)
  // TEST-CHECK-1*: = $root.nr_nv_nd
  // CHECK:   n4 = $root.nr_nv_nd(null, $builtins.hack_int(5), n3)
  /* HH_FIXME[4359] Allow split without variadic */
  nr_nv_nd(5, ...$a);
  // TEST-CHECK-1*: = $root.nr_nv_nd
  // CHECK:   n8 = $root.nr_nv_nd(null, $builtins.hack_int(5), n7)
  /* HH_FIXME[4029] Expected no type parameters */
  nr_nv_nd<int>(5);
  // TEST-CHECK-1*: n11 =
  // CHECK:   n11 = $builtins.__sil_splat(n0)
  // TEST-CHECK-1*: n12 =
  // CHECK:   n12 = $builtins.__sil_generics(n10)
  // TEST-CHECK-1*: = $root.nr_nv_nd
  // CHECK:   n13 = $root.nr_nv_nd(null, $builtins.hack_int(5), n11, n12)
  /* HH_FIXME[4029] Expected no type parameters */
  /* HH_FIXME[4359] Allow split without variadic */
  nr_nv_nd<int>(5, ...$a);

  // TEST-CHECK-1*: = $root.rr_nv_nd
  // CHECK:   n17 = $root.rr_nv_nd(null, $builtins.hack_int(5), n16)
  rr_nv_nd<int>(5);
  // TEST-CHECK-1*: = $root.rr_nv_nd
  // CHECK:   n22 = $root.rr_nv_nd(null, $builtins.hack_int(5), n20, n21)
  /* HH_FIXME[4359] Allow split without variadic */
  rr_nv_nd<int>(5, ...$a);

  // TEST-CHECK-1*: = $root.nr_vv_nd
  // CHECK:   n23 = $root.nr_vv_nd(null, $builtins.hack_int(5), $builtins.hack_int(6), $builtins.hack_int(7))
  nr_vv_nd(5, 6, 7);
  // TEST-CHECK-1*: = $root.nr_vv_nd
  // CHECK:   n25 = $root.nr_vv_nd(null, $builtins.hack_int(5), $builtins.hack_int(6), $builtins.hack_int(7), n24)
  nr_vv_nd(5, 6, 7, ...$a);
  // TEST-CHECK-1*: = $root.nr_vv_nd
  // CHECK:   n29 = $root.nr_vv_nd(null, $builtins.hack_int(5), $builtins.hack_int(6), $builtins.hack_int(7), n28)
  /* HH_FIXME[4029] Expected no type parameters */
  nr_vv_nd<int>(5, 6, 7);
  // TEST-CHECK-1*: = $root.nr_vv_nd
  // CHECK:   n34 = $root.nr_vv_nd(null, $builtins.hack_int(5), $builtins.hack_int(6), $builtins.hack_int(7), n32, n33)
  /* HH_FIXME[4029] Expected no type parameters */
  nr_vv_nd<int>(5, 6, 7, ...$a);

  // TEST-CHECK-1*: = $root.rr_vv_nd
  // CHECK:   n38 = $root.rr_vv_nd(null, $builtins.hack_int(5), $builtins.hack_int(6), $builtins.hack_int(7), n37)
  rr_vv_nd<int>(5, 6, 7);
  // TEST-CHECK-1*: = $root.rr_vv_nd
  // CHECK:   n43 = $root.rr_vv_nd(null, $builtins.hack_int(5), $builtins.hack_int(6), $builtins.hack_int(7), n41, n42)
  rr_vv_nd<int>(5, 6, 7, ...$a);

  // TEST-CHECK-1*: = $root.nr_nv_dd
  // CHECK:   n44 = $root.nr_nv_dd(null, $builtins.hack_int(5), $builtins.hack_int(6))
  nr_nv_dd(5, 6);
  // TEST-CHECK-1*: = $root.nr_nv_dd
  // CHECK:   n46 = $root.nr_nv_dd(null, $builtins.hack_int(5), $builtins.hack_int(6), n45)
  /* HH_FIXME[4359] Allow split without variadic */
  nr_nv_dd(5, 6, ...$a);
  // TEST-CHECK-1*: = $root.nr_nv_dd
  // CHECK:   n50 = $root.nr_nv_dd(null, $builtins.hack_int(5), $builtins.hack_int(6), n49)
  /* HH_FIXME[4029] Expected no type parameters */
  nr_nv_dd<int>(5, 6);
  // TEST-CHECK-1*: = $root.nr_nv_dd
  // CHECK:   n55 = $root.nr_nv_dd(null, $builtins.hack_int(5), $builtins.hack_int(6), n53, n54)
  /* HH_FIXME[4029] Expected no type parameters */
  /* HH_FIXME[4359] Allow split without variadic */
  nr_nv_dd<int>(5, 6, ...$a);

  // TEST-CHECK-1*: = $root.rr_nv_dd
  // CHECK:   n59 = $root.rr_nv_dd(null, $builtins.hack_int(5), $builtins.hack_int(6), n58)
  rr_nv_dd<int>(5, 6);
  // TEST-CHECK-1*: = $root.rr_nv_dd
  // CHECK:   n64 = $root.rr_nv_dd(null, $builtins.hack_int(5), $builtins.hack_int(6), n62, n63)
  /* HH_FIXME[4359] Allow split without variadic */
  rr_nv_dd<int>(5, 6, ...$a);

  // TEST-CHECK-1*: = $root.nr_vv_dd
  // CHECK:   n65 = $root.nr_vv_dd(null, $builtins.hack_int(5), $builtins.hack_int(6), $builtins.hack_int(7))
  nr_vv_dd(5, 6, 7);
  // TEST-CHECK-1*: = $root.nr_vv_dd
  // CHECK:   n67 = $root.nr_vv_dd(null, $builtins.hack_int(5), $builtins.hack_int(6), $builtins.hack_int(7), n66)
  nr_vv_dd(5, 6, 7, ...$a);
  // TEST-CHECK-1*: = $root.nr_vv_dd
  // CHECK:   n71 = $root.nr_vv_dd(null, $builtins.hack_int(5), $builtins.hack_int(6), $builtins.hack_int(7), n70)
  /* HH_FIXME[4029] Expected no type parameters */
  nr_vv_dd<int>(5, 6, 7);
  // TEST-CHECK-1*: = $root.nr_vv_dd
  // CHECK:   n76 = $root.nr_vv_dd(null, $builtins.hack_int(5), $builtins.hack_int(6), $builtins.hack_int(7), n74, n75)
  /* HH_FIXME[4029] Expected no type parameters */
  nr_vv_dd<int>(5, 6, 7, ...$a);

  // TEST-CHECK-1*: = $root.rr_vv_dd
  // CHECK:   n80 = $root.rr_vv_dd(null, $builtins.hack_int(5), $builtins.hack_int(6), $builtins.hack_int(7), n79)
  rr_vv_dd<int>(5, 6, 7);
  // TEST-CHECK-1*: = $root.rr_vv_dd
  // CHECK:   n85 = $root.rr_vv_dd(null, $builtins.hack_int(5), $builtins.hack_int(6), $builtins.hack_int(7), n83, n84)
  rr_vv_dd<int>(5, 6, 7, ...$a);
}
