<?hh

abstract class A {
  abstract const type TnoDefault;
  abstract const type ThasDefault = int;
}

<<__EntryPoint>>
function main(): void {
  $rc = new ReflectionClass(A::class);
  $rtnd = $rc->getTypeConstant("TnoDefault");
  var_dump($rtnd->isAbstract());
  $rtd = $rc->getTypeConstant("ThasDefault");
  var_dump($rtd->isAbstract());

}
