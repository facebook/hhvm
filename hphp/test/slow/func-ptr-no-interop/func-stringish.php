<?hh

function foo(Stringish $s) :mixed{
  var_dump($s);
}

<<__EntryPoint>>
function main() :mixed{
  foo('hello');
  foo(main<>);
}
