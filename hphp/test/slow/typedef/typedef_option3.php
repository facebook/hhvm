<?hh

type foo = ?int;
type bar = foo;

function wat(bar $x) :mixed{
  var_dump($x);
}
<<__EntryPoint>> function main(): void {
wat(null);
wat(2);
wat("fail"); // ends test
}
