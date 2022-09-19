// RUN: %hackc compile-infer %s | FileCheck %s

// CHECK: define _HtaintSource(params: HackParams) : HackMixed {
function taintSource(): int {
  // CHECK: n0 = hack_int(42)
  // CHECK: ret n0
  return 42;
}

// CHECK: define _HtaintSink(params: HackParams) : HackMixed {
function taintSink(int $i): void {
  // CHECK: n0 = hack_null()
  // CHECK: ret n0
}

// CHECK: define _HbasicFlow(params: HackParams) : HackMixed {
function basicFlow(int $untainted): void {
  taintSink($untainted);
  $tainted = taintSource();
  taintSink($tainted);
}
