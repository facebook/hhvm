<?hh

// This test should error. It tests that if EnforceGenericsUB == 2
// at build time but == 0 at run-time, we still error as repo was built
// with hard error assumption.

function foo<T as int>(T $x) :mixed{
  var_dump($x);
}

<<__EntryPoint>>
function main() :mixed{
  foo('a');
}
