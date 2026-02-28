<?hh

function f(mixed $x): void {
  try {
    var_dump($x as nothing);
  } catch (TypeAssertionException $_) {
    echo "not nothing: ".gettype($x)."\n";
  }
}


<<__EntryPoint>>
function main_nothing() :mixed{
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
