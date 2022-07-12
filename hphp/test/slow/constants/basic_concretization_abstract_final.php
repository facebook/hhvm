<?hh

<<file:__EnableUnstableFeatures('class_const_default')>>

abstract class A {
  abstract const int X = 3;
  abstract const type T = int;
}

abstract final class AFinal extends A {}
abstract class AAbstract extends A {}

<<__EntryPoint>>
function main(): void {
  $tf = $bool ==> $bool ? "true" : "false";
  $afinal = new ReflectionClass(AFinal::class);
  echo "AFinal::X is concrete: ".$tf($afinal->hasConstant("X"))."\n"; // returns false for abstract constants
  echo "AFinal::T is concrete: ".$tf(!$afinal->getTypeConstant("T")->isAbstract())."\n";

  $aabstract = new ReflectionClass(AAbstract::class);
  echo "AAbstract::X is concrete: ".$tf($aabstract->hasConstant("X"))."\n"; // returns false for abstract constants
  echo "AAbstract::T is concrete: ".$tf(!$aabstract->getTypeConstant("T")->isAbstract())."\n";
}
