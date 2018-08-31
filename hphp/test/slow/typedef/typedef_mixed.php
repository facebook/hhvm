<?hh

type Foo = mixed;

function foo(Foo $x) {
  echo $x;
  echo "\n";
}


<<__EntryPoint>>
function main_typedef_mixed() {
foo(12);
}
