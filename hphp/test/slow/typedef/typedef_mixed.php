<?hh

type Foo = mixed;

function foo(Foo $x) :mixed{
  echo $x;
  echo "\n";
}


<<__EntryPoint>>
function main_typedef_mixed() :mixed{
foo(12);
}
