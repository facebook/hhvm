<?hh

abstract class A {
  abstract const type T = int;
}

// does not concretize T
abstract class B extends A {}
// should concretize B::T which is itself an inherited member
class BConcrete extends B {}

// does concretize T
class C extends A {}

function t_is_abstract(classname<A> $a): void {
  $r = new ReflectionClass($a);
  $rt = $r->getTypeConstant("T");
  $abs = $rt->isAbstract() ? "abstract" : "not abstract";
  echo "$a" . "::T is " . $abs . "\n";
}

<<__EntryPoint>>
function main(): void {
  t_is_abstract(A::class);
  t_is_abstract(B::class);
  t_is_abstract(BConcrete::class);
  t_is_abstract(C::class);
}
