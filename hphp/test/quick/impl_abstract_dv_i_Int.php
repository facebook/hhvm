<?hh
abstract class A   { abstract public function a(int $a1 = null):mixed;   }
class B extends A  {          public function a(int $a1 = null) :mixed{} }
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
