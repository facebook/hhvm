<?hh
class X<A,B,C> {}
function takes_string(string $x) { return "1\n"; }
function takes_x(X<int> $x) { return "1\n"; }
function takes_opt_string(?string $x) { return "2\n"; }
echo takes_string('foo');
echo takes_x(new X());
echo takes_opt_string(array(42)); // allowed -- maybe is desugared to nothing
echo takes_x("foo"); // not allowed -- should desugar to X
