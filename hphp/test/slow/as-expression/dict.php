<?hh

function f(mixed $x): void {
  try {
    var_dump($x as dict);
  } catch (TypeAssertionException $_) {
    echo "not dict: ".gettype($x)."\n";
  }
}


<<__EntryPoint>>
function main_dict() :mixed{
f(1);
f(false);
f(1.5);
f('foo');
f(fopen(__FILE__, 'r'));
f(new stdClass());
f(vec[]);
f(dict[]);
f(keyset[]);
f(null);
}
