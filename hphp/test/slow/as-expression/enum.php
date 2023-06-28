<?hh

enum Foo: int {
  A = 1;
  B = 2;
}

enum Bar: string {
  A = '1';
  B = '2';
}

function as_foo(mixed $x) :mixed{
  try {
    var_dump($x as Foo);
  } catch (TypeAssertionException $_) {
    echo "not Foo: ".gettype($x)."\n";
  }
}

function as_bar(mixed $x) :mixed{
  try {
    var_dump($x as Bar);
  } catch (TypeAssertionException $_) {
    echo "not Bar: ".gettype($x)."\n";
  }
}
<<__EntryPoint>> function main(): void {
as_foo(1);
as_foo('1'); // TODO(T29283057)
as_foo(2);
as_foo(1.5);
as_foo(true);
as_foo(null);
as_foo(fopen(__FILE__, 'r'));
as_foo(new stdClass());

echo "\n";

as_bar(1); // TODO(T29283057)
as_bar('1');
as_bar('2');
as_bar(1.5);
as_bar(true);
as_bar(null);
as_bar(fopen(__FILE__, 'r'));
as_bar(new stdClass());
}
