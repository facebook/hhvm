<?hh
abstract class A   { abstract public function a(int     $a1 = null);   }
class B extends A  {          public function a(int $a1 = null) {} }
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
