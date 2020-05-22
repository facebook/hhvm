<?hh

function foo<T as int>(T $x) {
  var_dump($x);
}

<<__EntryPoint>>
function main() {
  foo('a');
}
