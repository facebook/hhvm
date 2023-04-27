<?hh

class C {}

type X = Stringish;

function foo(Stringish $x) {
  echo "in foo\n";
}

function foo2(X $x) {
  echo "in foo2\n";
}

<<__EntryPoint>>
function main() {
  foo("C");
  foo(C::class);

  foo2("C");
  foo2(C::class);

  var_dump("C" is Stringish);
  var_dump(C::class is Stringish);

  var_dump("C" is X);
  var_dump(C::class is X);
}
