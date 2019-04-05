<?hh

function f(mixed $x): void {
  try {
    var_dump($x as nothing);
  } catch (TypeAssertionException $_) {
    echo "not nothing: ".gettype($x)."\n";
  }
}


<<__EntryPoint>>
function main_nothing() {
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
}
