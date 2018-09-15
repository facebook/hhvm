<?hh

function f(mixed $x): void {
  try {
    var_dump($x as vec_or_dict);
  } catch (TypeAssertionException $_) {
    echo "not vec_or_dict: ".gettype($x)."\n";
  }
}


<<__EntryPoint>>
function main_vec_or_dict() {
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
