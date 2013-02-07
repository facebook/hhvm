<?hh

type Foo = array;
type Bar = Foo;

function main(Bar $x) {
  echo "Hi\n";
}

main(array(12));