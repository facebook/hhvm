<?hh

function foo(Stringish $s) {
  var_dump($s);
}

<<__EntryPoint>>
function main() {
  foo('hello');
  foo(main<>);
}
