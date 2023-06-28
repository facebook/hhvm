<?hh

function f(mixed $x): void {
  try {
    var_dump($x as varray);
  } catch (TypeAssertionException $_) {
    echo "not varray: ".gettype($x)."\n";
  }
}


<<__EntryPoint>>
function main_varray() :mixed{
f(vec[]);
f(dict[]);
f(keyset[]);
f(HH\array_mark_legacy(vec[]));
f(HH\array_mark_legacy(dict[]));
}
