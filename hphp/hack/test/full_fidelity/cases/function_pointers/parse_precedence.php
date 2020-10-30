<?hh

// Stranger placements for <>
foo < bar<>(0 > 0, foo<>);
foo < bar<>(Bar > 0, 10 > Qux);
foo < bar(2 > foo<int,string>);
foo < bar(3 > x::foo<int,string>);

$x ?? foo<>;
$x ?? foo<int>;
$x ?? x::foo<>;
$x ?? x::foo<int>;

42 * foo<>;
42 * foo<int>;
42 * x::foo<>;
42 * x::foo<bar>;

baz(foo<>);
baz(foo<baz>);
baz(x::foo<>);
baz(x::foo<baz>);

42 * foo<> < 90;
42 * foo<int> < 90 + 50;
42 * x::foo<> < 90 === true;
42 * x::foo<int> < 90 === true;

$f === foo<> ? 1 : 2;
$f === foo<bar> ? 1 : 2;
$f === x::foo<> ? 1 : 2;
$f === x::foo<bar> ? 1 : 2;

foo<>($f === foo<> ? 1 : 2);
foo<>($f === foo<bar> ? 1 : 2);
foo<bar>($f === foo<> ? 1 : 2);
foo<bar>($f === foo<bar> ? 1 : 2);
x::foo<>($f === x::foo<> ? 1 : 2);
x::foo<>($f === x::foo<bar> ? 1 : 2);
x::foo<bar>($f === x::foo<> ? 1 : 2);
x::foo<bar>($f === x::foo<bar> ? 1 : 2);

$f === Vector<int>{1,2,3} ? foo<> : foo<>;
$f === Vector<int>{1,2,3} ? foo<int> : foo<int>;
$f === Vector<int>{1,2,3} ? x::foo<> : x::foo<>;
$f === Vector<int>{1,2,3} ? x::foo<int> : x::foo<int>;
