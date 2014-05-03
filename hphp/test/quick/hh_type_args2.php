<?hh
class X<A,B,C> {}
function takes_string(string $x) { return "1\n"; }
function takes_classname(classname $n) { return "1\n"; }
function takes_x(X<int> $x) { return "1\n"; }

function main() {
  echo takes_string('foo');
  echo takes_string(X::class);
  echo takes_classname(X::class);
  echo takes_x(new X());
  echo takes_x("foo"); // not allowed -- should desugar to X
}
main();
