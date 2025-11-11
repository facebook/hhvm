<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

enum E : int as int {
  A = 1;
}
enum F : string as string {
  B = "1";
}

function test1(mixed $m):void {
  $x = $m as E;
}
function test2(mixed $m):void {
  $y = $m as F;
}
function test3(mixed $m):void {
  if ($m is E) {
    $z = $m;
  }
}
function test4(mixed $m):void {
  if ($m is F) {
    $z = $m;
  }
}
function make_e():E {
  return E::A;
}
function test6():void {
  $x = make_e();
  $c = new C();
  $y = $c->make_e();
}
class C {
  public function make_e():E {
    return E::A;
  }
}
