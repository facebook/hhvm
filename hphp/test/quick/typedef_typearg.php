<?hh

type Foo<X> = string;

interface Dummy {}

function main(Foo<Dummy> $x) {
  echo "Hi\n";
}

main('42');
main(42);
