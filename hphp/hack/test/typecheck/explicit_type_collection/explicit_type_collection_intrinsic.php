<?hh

function f(): void {
  $a = vec<int>[0, 1, 2];
  $b = dict<string, int>["0" => 10, "1" => 11, "2" => 12];
  $c = keyset<int>[0, 1, 2];
  $d = varray<int>[0, 1, 2];
  $e = darray<string, int>["0" => 10, "1" => 11, "2" => 12];
}
