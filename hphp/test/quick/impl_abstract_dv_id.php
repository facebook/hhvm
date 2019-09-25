<?hh
abstract class A   { abstract public function d(int $a1 = 0, int    $a2 = 2);     }
class B extends A  {          public function d(int $a1 = 0, float $a2 = 3.0) {} }

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
