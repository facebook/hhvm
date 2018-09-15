<?hh

function f(mixed $x): void {
  try {
    var_dump($x as num);
  } catch (TypeAssertionException $_) {
    echo "not num: ".gettype($x)."\n";
  }
}

function g(mixed $x): void {
  try {
    var_dump($x as arraykey);
  } catch (TypeAssertionException $_) {
    echo "not arraykey: ".gettype($x)."\n";
  }
}


<<__EntryPoint>>
function main_primitive_union() {
f(1);
f(false);
f(1.5);
f('foo');
f(STDIN);
f(new stdClass());
f(vec[]);
f(dict[]);
f(keyset[]);
f(null);

echo "\n";

g(1);
g(false);
g(1.5);
g('foo');
g(STDIN);
g(new stdClass());
g(vec[]);
g(dict[]);
g(keyset[]);
g(null);
}
