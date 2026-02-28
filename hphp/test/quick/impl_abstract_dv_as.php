<?hh
abstract class A   { abstract public function s(int $a1 = 0, AnyArray  $a2 = null):mixed;   }
class B extends A  {          public function s(int $a1 = 0, string $a2 = null) :mixed{} }

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
