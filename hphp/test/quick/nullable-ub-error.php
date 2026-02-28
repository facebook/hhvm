<?hh

function foo<T as int>(?T $x) :mixed{
  var_dump($x);
}

<<__EntryPoint>>
function main() :mixed{
  foo(null);
  foo('a');
}
