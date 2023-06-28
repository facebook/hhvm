<?hh
abstract class A   { abstract public function a(int $a1 = 0, int $a2 = 2):mixed;   }
class B extends A  {          public function a(int $a1 = 0, AnyArray $a2 = null) :mixed{} }

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
