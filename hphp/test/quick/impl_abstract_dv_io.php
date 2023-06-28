<?hh
abstract class A   { abstract public function o(int $a1 = 0, int $a2 = 2):mixed;   }
class B extends A  {          public function o(int $a1 = 0, object $a2 = null) :mixed{} }

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
