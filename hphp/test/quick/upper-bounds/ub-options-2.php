<?hh
// This test should not warn. It tests that if EnforceGenericsUB != 0
// at build time but == 0 at run-time, we do not warn or error.
function foo<T as int>(T $x) :mixed{
  var_dump($x);
}

<<__EntryPoint>>
function main() :mixed{
  foo('a');
}
