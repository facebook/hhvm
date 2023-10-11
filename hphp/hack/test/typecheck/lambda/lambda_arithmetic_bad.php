<?hh
function makeFun<T1,T2>((function(T1):T2) $f):(function(T1):T2) {
  return $f;
}
function makeFun2<T1,T2,T3>((function(T1,T2):T3) $f):(function(T1,T2):T3) {
  return $f;
}
enum E : int as int {
  A = 1;
}
function expectE(E $e):void { }

function expectInt(int $i):void { }
<<__EntryPoint>>
function foo(): void {
  $add_func = makeFun($x ==> $x + 1);
  $sub_func = makeFun($x ==> $x - 1);
  $mul_func = makeFun($x ==> $x * 2);
  $add_func2 = makeFun2(($x,$y) ==> $x + $y);
  expectE($add_func2(E::A, E::A));
  expectInt($add_func(3));
  expectInt($add_func(3.4));
  expectInt($sub_func(3));
  expectInt($sub_func(3.4));
  expectInt($mul_func(3));
  expectInt($mul_func(3.4));
}
