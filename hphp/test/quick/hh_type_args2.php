<?hh
class X<A,B,C> {}
function takes_string(string $x) { return "1\n"; }
function takes_x(X<int> $x) { return "1\n"; }
echo takes_string('foo');
echo takes_x(new X());
echo takes_x("foo"); // not allowed -- should desugar to X
