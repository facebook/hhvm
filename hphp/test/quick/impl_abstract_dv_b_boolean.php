<?hh
abstract class A   { abstract public function b(bool    $b1 = null):mixed;   }
class B extends A  {          public function b(bool $b1 = null) :mixed{} }

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
