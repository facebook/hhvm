<?hh

enum Foo : int as int {
  WAT = 0;
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}

enum Bar : string as string {
  FOO = 'foo';
  BAR = 'bar';
  BAZ = '10';
}

enum Baz : int {
}

class Stringy {
  function __toString(): string {
    return 'foo';
  }
}

function getFooValues(): array<string, Foo> {
  return Foo::getValues();
}
function getFooValues2(): array<string, int> {
  return Foo::getValues();
}

echo "Some basic tests on Foo\n";
var_dump(getFooValues());
var_dump(Foo::getNames());
var_dump(Foo::isValid(Foo::FOO));
var_dump(Foo::isValid(12));
var_dump(Foo::isValid(null));

echo "getValues/getNames tests on Bar\n";
var_dump(Bar::getValues());
var_dump(Bar::getNames());

echo "getValues/getNames tests on Baz\n";
var_dump(Baz::getValues());
var_dump(Baz::getNames());

echo "coerce() on Foo\n";
var_dump(Foo::coerce(Foo::FOO));
var_dump(Foo::coerce(2));
var_dump(Foo::coerce('3'));
var_dump(Foo::coerce(100));

// 0 should give 0, other falsey things are null
echo "falsey coerce() on Foo\n";
var_dump(Foo::coerce(0));
var_dump(Foo::coerce(null));
var_dump(Foo::coerce(false));
var_dump(Foo::coerce(''));

echo "coerce() on Bar\n";
var_dump(Bar::coerce(Bar::BAR));
var_dump(Bar::coerce(10));
var_dump(Bar::coerce(100));
var_dump(Bar::coerce('foo'));
var_dump(Bar::coerce('welp'));

echo "Stringish doesn't count\n";
var_dump(Bar::coerce(new Stringy()));

echo "some valid assert()s\n";
var_dump(Foo::assert(Foo::FOO));
var_dump(Foo::assert('1'));
var_dump(Bar::assert(10));
var_dump(Bar::assert('foo'));

echo "a broken assert()\n";
var_dump(Bar::assert('welp'));
