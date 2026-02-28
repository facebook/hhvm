<?hh

enum Foo: int {
  A = 1;
  B = 2;
}

enum Bar: string {
  A = '1';
  B = '2';
}

type TFoo = Foo;
type TBar = Bar;

function is_foo(mixed $x) :mixed{
  if ($x is TFoo) {
    echo "Foo\n";
  } else {
    echo "not Foo\n";
  }
}

function is_bar(mixed $x) :mixed{
  if ($x is TBar) {
    echo "Bar\n";
  } else {
    echo "not Bar\n";
  }
}
<<__EntryPoint>> function main(): void {
is_foo(1);
is_foo('1');
is_foo(2);
is_foo(1.5);
is_foo(true);
is_foo(null);
is_foo(fopen(__FILE__, 'r'));
is_foo(new stdClass());

echo "\n";

is_bar(1);
is_bar('1');
is_bar('2');
is_bar(1.5);
is_bar(true);
is_bar(null);
is_bar(fopen(__FILE__, 'r'));
is_bar(new stdClass());
}
