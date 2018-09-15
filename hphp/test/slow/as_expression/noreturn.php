<?hh

function f(mixed $x): void {
  try {
    var_dump($x as noreturn);
  } catch (TypeAssertionException $_) {
    echo "not noreturn: ".gettype($x)."\n";
  }
}


<<__EntryPoint>>
function main_noreturn() {
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
