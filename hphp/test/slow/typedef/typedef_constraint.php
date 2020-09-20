<?hh

type Foo = int;
newtype fbid as int = int;

function f(fbid $x) {
  echo $x;
  echo "\n";
}

<<__EntryPoint>>
function main_typedef_constraint() {
f(4);
}
