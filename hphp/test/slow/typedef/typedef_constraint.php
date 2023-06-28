<?hh

type Foo = int;
newtype fbid as int = int;

function f(fbid $x) :mixed{
  echo $x;
  echo "\n";
}

<<__EntryPoint>>
function main_typedef_constraint() :mixed{
f(4);
}
