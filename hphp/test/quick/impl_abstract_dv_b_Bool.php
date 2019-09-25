<?hh
abstract class A   { abstract public function b(bool $b1 = null);   }
class B extends A  {          public function b(bool $b1 = null) {} }

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
