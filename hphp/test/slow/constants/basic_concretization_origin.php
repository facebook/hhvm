<?hh

abstract class A {
  abstract const type T = int;
}

class C extends A {}

<<__EntryPoint>>
function main(): void {
  $r = new ReflectionClass(C::class);
  $rt = $r->getTypeConstant("T");
  echo "Class: ".$rt->getClass()->getName();
  echo "\nDeclaring class: ".$rt->getDeclaringClass()->getName();
}
