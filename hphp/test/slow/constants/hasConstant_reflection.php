<?hh

<<file:__EnableUnstableFeatures('class_const_default')>>

abstract class A {
  abstract const int I;
  abstract const int J = 4;
  const int K = 5;
}

<<__EntryPoint>>
function main(): void {
  $a = new ReflectionClass(A::class);
  var_dump($a->hasConstant("I"));
  var_dump($a->hasConstant("J"));
  var_dump($a->hasConstant("K"));
}
