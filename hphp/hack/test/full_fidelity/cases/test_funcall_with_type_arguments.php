<?hh

// ficticious functions with size-discernable names
f('hi'); // normal function call w/o generics
fo<string>('hello'); // annotated function call
foo(0 < 0, tuple()); // not generics annotated, but seems so a little
fooo(Bar < 0, 10 > Qux); // certainly not annotated, but hard to tell
foooo(bar<int,string>()); // annotated inside an argument list
42 * ba<string>('hello');

$x ?? foo<bar>();
42 * foo<bar>();
42 * foo<bar>($x);
42 * foo<bar>($x, $y);
42 * x::foo<bar>();
42 * x::foo<bar>($x);
42 * x::foo<bar>($x, $y);
baz(foo<baz>());
baz(foo<baz>($x));
baz(foo<baz>($x, $y));
foo<bar<baz>();
42 * foo < 90;
42 * foo < 90 + 50;
42 * foo < 90 === true;

$f === foo<bar>() ? 1 : 2;
foo<bar>($f === foo<bar>() ? 1 : 2);
$f === x::foo<bar>(1) ? 1 : 2;
x::foo<bar>($f === x::foo<bar>(1) ? 1 : 2);
$f === Vector<int>{1,2,3} ? foo<bar>() : x::foo<bar>();
