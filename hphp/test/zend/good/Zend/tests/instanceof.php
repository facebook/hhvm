<?hh

class B {}

class A {}

<<__EntryPoint>> function main(): void {
$a = new A;
var_dump($a is B);
var_dump($a is A);
}
