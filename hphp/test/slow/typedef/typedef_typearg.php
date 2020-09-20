<?hh

type Foo<X> = string;

interface Dummy {}

function main(Foo<Dummy> $x) {
  echo "Hi\n";
}
<<__EntryPoint>> function main_entry(): void {
main('42');
main(42);
}
