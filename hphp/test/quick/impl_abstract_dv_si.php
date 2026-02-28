<?hh
abstract class A   { abstract public function i(int $a1 = 0, string $a2 = null):mixed;   }
class B extends A  {          public function i(int $a1 = 0, int    $a2 = null) :mixed{} }

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
