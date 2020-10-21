<?hh

class C {}

function test(
  int $x,
  ...$ys
) {
  print(HH\get_provenance($ys).': '.json_encode($ys)."\n");
  foreach ($ys as $i => $y) {
    if (HH\is_any_array($y)) {
      print("  Arg $i: ".HH\get_provenance($y).': '.json_encode($y)."\n");
    }
  }
}

<<__EntryPoint>>
function main() {
  test(1);
  test(1, 2);
  test(1, 2, 3);
  test(1, varray[17]);
  test(1, varray[new C()]);
  test(1, ...vec[]);
  test(1, ...varray[]);
  test(1, ...vec[2, 3]);
  test(1, ...varray[2, 3]);
  test(1, ...vec[varray[17]]);
  test(1, ...vec[varray[new C()]]);
  test(1, ...varray[varray[17]]);
  test(1, ...varray[varray[new C()]]);
}
