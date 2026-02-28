<?hh

class C {}

type X = Stringish;

function foo(Stringish $x) :mixed{
  echo "in foo\n";
}

function foo2(X $x) :mixed{
  echo "in foo2\n";
}

<<__EntryPoint>>
function main() :mixed{
  foo("C");
  foo(C::class);

  foo2("C");
  foo2(C::class);

  var_dump("C" is Stringish);
  var_dump(C::class is Stringish);

  var_dump("C" is X);
  var_dump(C::class is X);
}
