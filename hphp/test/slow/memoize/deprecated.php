<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__Memoize, __Deprecated>> function func1() { return 123; }
<<__Memoize, __Deprecated>> function func2($a) { return $a; }

class A {
  <<__Memoize, __Deprecated>> public static function func3() { return 456; }
  <<__Memoize, __Deprecated>> public static function func4($a) { return $a; }

  <<__Memoize, __Deprecated>> public function func5() { return 789; }
  <<__Memoize, __Deprecated>> public function func6($a) { return $a; }
}


<<__EntryPoint>>
function main_deprecated() {
func1();
func1();

func2('abc');
func2('abc');

A::func3();
A::func3();

A::func4('def');
A::func4('def');

$a = new A();
$a->func5();
$a->func5();

$a->func6('ghi');
$a->func6('ghi');
}
