<?hh
abstract class A            { abstract public function s(string $s1 = null):mixed;   }
abstract class B extends A  {          public function s(AnyArray  $s1 = null) :mixed{} }

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
