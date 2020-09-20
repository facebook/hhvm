<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class D {
  public function test1(A $a) { return $a->func1(); }
}

class A extends D {
  private function func1() { return varray[1, 2, 3]; }
  protected function func2() { return varray[4, 5, 6]; }
}

class B {
  public function test1(A $a) { return $a->func1(); }
  public function test2(A $a) { return $a->func2(); }
}

class C extends A {
  public function test1(A $a) { return $a->func1(); }
}

function test1() { return (new A)->func1(); }
function test2() { return (new A)->func2(); }
function test3() { return (new B)->test1(new A); }
function test4() { return (new B)->test2(new A); }
function test5() { return (new C)->test1(new A); }
function test6() { return (new D)->test1(new A); }


<<__EntryPoint>>
function main_inaccessible_folding() {
$tests = vec[
  'test1',
  'test2',
  'test3',
  'test4',
  'test5',
  'test6',
];

$count = __hhvm_intrinsics\apc_fetch_no_check('count');
if ($count === false) $count = 0;
if ($count < count($tests)) {
  $test = $tests[$count];
  ++$count;
  apc_store('count', $count);
  echo "====================== $test =======================\n";
  var_dump($test());
}
}
