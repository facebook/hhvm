<?hh

type Foo = varray;
type Bar = Foo;

function main(Bar $x) :mixed{
  echo "Hi\n";
}


<<__EntryPoint>>
function main_typedef_to_typedef() :mixed{
main(vec[12]);
}
