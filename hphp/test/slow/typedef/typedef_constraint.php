<?hh

type Foo = int;
newtype fbid as int = int;

function f(fbid $x) {
  echo $x;
  echo "\n";
}
f(4);
