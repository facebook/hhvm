<?hh
abstract class A   { abstract public function s(int $a1 = 0, int    $a2 = 2):mixed;   }
class B extends A  {          public function s(int $a1 = 0, string $a2 = "abc") :mixed{} }

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
