<?hh

type Foo = array;
type Bar = Foo;

function main(Bar $x) {
  echo "Hi\n";
}


<<__EntryPoint>>
function main_typedef_to_typedef() {
main(varray[12]);
}
